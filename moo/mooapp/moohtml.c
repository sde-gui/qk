/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moohtml.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooapp/moohtml.h"
#include "mooutils/moomarshals.h"
#include "mooutils/eggregex.h"
#include "mooutils/mooutils-misc.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libxml/HTMLtree.h>
#include <string.h>
#include <errno.h>


#define DEFAULT_PAR_SPACING 6


struct _MooHtmlPrivate {
    GHashTable *root_tags; /* char* -> MooHtmlTag* */
    GSList *href_tags;

    char *title;
    htmlDocPtr doc;
    GHashTable *anchors; /* char* -> GtkTextMark* */
    char *hover_link;

    double font_sizes[7];
    int par_spacing[7];
    double heading_sizes[6];
    int heading_spacing[6];
    char *heading_faces[6];
    char *monospace;
    GHashTable *font_faces;

    char *filename;
    char *basename;
    char *dirname;

    gboolean new_line;
    gboolean space;

    gboolean button_pressed;
    gboolean in_drag;

    GSList *rulers;
};

typedef enum {
    MOO_HTML_FG                 = 1 << 0,
    MOO_HTML_BG                 = 1 << 1,
    MOO_HTML_BOLD               = 1 << 2,
    MOO_HTML_ITALIC             = 1 << 3,
    MOO_HTML_UNDERLINE          = 1 << 4,
    MOO_HTML_STRIKETHROUGH      = 1 << 5,
    MOO_HTML_LINK               = 1 << 6,
    MOO_HTML_SUB                = 1 << 7,
    MOO_HTML_SUP                = 1 << 8,
    MOO_HTML_LEFT_MARGIN        = 1 << 9,

    MOO_HTML_PRE                = 1 << 10,

    MOO_HTML_MONOSPACE          = 1 << 11,
    MOO_HTML_LARGER             = 1 << 12,
    MOO_HTML_SMALLER            = 1 << 13,
    MOO_HTML_HEADING            = 1 << 14,
    MOO_HTML_FONT_SIZE          = 1 << 15,
    MOO_HTML_FONT_PT_SIZE       = 1 << 16,
    MOO_HTML_FONT_FACE          = 1 << 17,
} MooHtmlAttrMask;

struct _MooHtmlAttr
{
    MooHtmlAttrMask mask;
    char *fg;
    char *bg;
    char *link;
    int left_margin;

    guint heading;
    guint font_size;
    guint font_pt_size;
    guint scale;
    char *font_face;
};


static MooHtmlAttr *moo_html_attr_copy      (const MooHtmlAttr *src);
static void     moo_html_attr_free          (MooHtmlAttr    *attr);

static void     moo_html_finalize           (GObject        *object);
static void     moo_html_set_property       (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_html_get_property       (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_html_tag_finalize       (GObject        *object);
static void     moo_html_tag_set_property   (GObject        *object,
                                             guint           prop_id,
                                             const GValue   *value,
                                             GParamSpec     *pspec);
static void     moo_html_tag_get_property   (GObject        *object,
                                             guint           prop_id,
                                             GValue         *value,
                                             GParamSpec     *pspec);

static void     moo_html_size_allocate      (GtkWidget      *widget,
                                             GtkAllocation  *allocation);
static gboolean moo_html_button_press       (GtkWidget      *widget,
                                             GdkEventButton *event);
static gboolean moo_html_button_release     (GtkWidget      *widget,
                                             GdkEventButton *event);
// static gboolean moo_html_key_press          (GtkWidget      *widget,
//                                              GdkEventKey    *event);
static gboolean moo_html_motion             (GtkWidget      *widget,
                                             GdkEventMotion *event);

static gboolean moo_html_load_url_real      (MooHtml        *html,
                                             const char     *url);
static void     moo_html_clear              (MooHtml        *html);
static void     moo_html_set_doc            (MooHtml        *html,
                                             htmlDocPtr      doc);
static void     moo_html_load_doc           (MooHtml        *html,
                                             htmlDocPtr      doc);

static MooHtmlTag *moo_html_create_tag      (MooHtml        *html,
                                             const MooHtmlAttr *attr,
                                             MooHtmlTag     *parent,
                                             gboolean        force);
static void     moo_html_create_anchor      (MooHtml        *html,
                                             GtkTextBuffer  *buffer,
                                             GtkTextIter    *iter,
                                             const char     *name);
static MooHtmlTag *moo_html_get_link_tag    (MooHtml        *html,
                                             GtkTextIter    *iter);
static MooHtmlTag *moo_html_get_tag         (MooHtml        *html,
                                             GtkTextIter    *iter);

static gboolean moo_html_parse_url          (const char     *url,
                                             char          **scheme,
                                             char          **base,
                                             char          **anchor);
static gboolean moo_html_goto_anchor        (MooHtml        *html,
                                             const char     *anchor);
static void     moo_html_make_heading_tag   (MooHtml        *html,
                                             MooHtmlTag     *tag,
                                             guint           heading);

static void     init_funcs                  (void);


G_DEFINE_TYPE (MooHtml, moo_html, GTK_TYPE_TEXT_VIEW)
G_DEFINE_TYPE (MooHtmlTag, moo_html_tag, GTK_TYPE_TEXT_TAG)

enum {
    HTML_PROP_0,
    HTML_PROP_TITLE
};

enum {
    LOAD_URL = 0,
    HOVER_LINK,
    NUM_HTML_SIGNALS
};

enum {
    TAG_PROP_0,
    TAG_PROP_HREF
};


static guint html_signals[NUM_HTML_SIGNALS];


/************************************************************************/
/* MooHtmlTag
 */

static void
moo_html_tag_class_init (MooHtmlTagClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_html_tag_finalize;
    gobject_class->set_property = moo_html_tag_set_property;
    gobject_class->get_property = moo_html_tag_get_property;

    g_object_class_install_property (gobject_class,
                                     TAG_PROP_HREF,
                                     g_param_spec_string ("href",
                                             "href",
                                             "href",
                                             NULL,
                                             G_PARAM_READWRITE));
}


static void
moo_html_tag_init (MooHtmlTag *tag)
{
    tag->href = NULL;
    tag->parent = NULL;
    tag->attr = NULL;
}


static void
moo_html_tag_set_property (GObject        *object,
                           guint           prop_id,
                           const GValue   *value,
                           GParamSpec     *pspec)
{
    MooHtmlTag *tag = MOO_HTML_TAG (object);

    switch (prop_id)
    {
        case TAG_PROP_HREF:
            g_free (tag->href);
            tag->href = g_strdup (g_value_get_string (value));
            g_object_notify (object, "href");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_html_tag_get_property (GObject        *object,
                           guint           prop_id,
                           GValue         *value,
                           GParamSpec     *pspec)
{
    MooHtmlTag *tag = MOO_HTML_TAG (object);

    switch (prop_id)
    {
        case TAG_PROP_HREF:
            g_value_set_string (value, tag->href);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_html_tag_finalize (GObject        *object)
{
    MooHtmlTag *tag = MOO_HTML_TAG (object);

    moo_html_attr_free (tag->attr);
    g_free (tag->href);

    G_OBJECT_CLASS(moo_html_tag_parent_class)->finalize (object);
}


/************************************************************************/
/* MooHtml
 */

static void
moo_html_class_init (MooHtmlClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->finalize = moo_html_finalize;
    gobject_class->set_property = moo_html_set_property;
    gobject_class->get_property = moo_html_get_property;

    widget_class->size_allocate = moo_html_size_allocate;
    widget_class->button_press_event = moo_html_button_press;
    widget_class->button_release_event = moo_html_button_release;
//     widget_class->key_press_event = moo_html_key_press;
    widget_class->motion_notify_event = moo_html_motion;

    klass->load_url = moo_html_load_url_real;

    init_funcs ();

    g_object_class_install_property (gobject_class,
                                     HTML_PROP_TITLE,
                                     g_param_spec_string ("title",
                                             "title",
                                             "title",
                                             NULL,
                                             G_PARAM_READWRITE));

    html_signals[LOAD_URL] =
            g_signal_new ("load-url",
                          G_TYPE_FROM_CLASS (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooHtmlClass, load_url),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__STRING,
                          G_TYPE_BOOLEAN, 1,
                          G_TYPE_STRING);

    html_signals[HOVER_LINK] =
            g_signal_new ("hover-link",
                          G_TYPE_FROM_CLASS (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooHtmlClass, hover_link),
                          NULL, NULL,
                          _moo_marshal_VOID__STRING,
                          G_TYPE_NONE, 1,
                          G_TYPE_STRING);
}


static void
moo_html_init (MooHtml *html)
{
    html->priv = g_new0 (MooHtmlPrivate, 1);

    html->priv->anchors = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                 g_free, NULL);
    html->priv->root_tags = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                   g_free, NULL);
    html->priv->href_tags = NULL;
    html->priv->doc = NULL;

    html->priv->filename = NULL;
    html->priv->basename = NULL;
    html->priv->dirname = NULL;

    g_object_set (html,
                  "cursor-visible", FALSE,
                  "editable", FALSE,
                  "wrap-mode", GTK_WRAP_WORD,
                  "pixels-below-lines", DEFAULT_PAR_SPACING,
                  NULL);

    html->priv->font_sizes[0] = PANGO_SCALE_X_SMALL;
    html->priv->font_sizes[1] = PANGO_SCALE_SMALL;
    html->priv->font_sizes[2] = PANGO_SCALE_MEDIUM;
    html->priv->font_sizes[3] = PANGO_SCALE_LARGE;
    html->priv->font_sizes[4] = PANGO_SCALE_X_LARGE;
    html->priv->font_sizes[5] = PANGO_SCALE_XX_LARGE;
    html->priv->font_sizes[6] = PANGO_SCALE_XX_LARGE * PANGO_SCALE_LARGE;

    html->priv->heading_sizes[0] = PANGO_SCALE_XX_LARGE * PANGO_SCALE_LARGE;
    html->priv->heading_sizes[1] = PANGO_SCALE_XX_LARGE;
    html->priv->heading_sizes[2] = PANGO_SCALE_X_LARGE;
    html->priv->heading_sizes[3] = PANGO_SCALE_LARGE;
    html->priv->heading_sizes[4] = PANGO_SCALE_MEDIUM;
    html->priv->heading_sizes[5] = PANGO_SCALE_SMALL;

    html->priv->monospace = g_strdup ("Monospace");
    html->priv->font_faces = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}


static void
moo_html_set_property (GObject        *object,
                       guint           prop_id,
                       const GValue   *value,
                       GParamSpec     *pspec)
{
    MooHtml *html = MOO_HTML (object);

    switch (prop_id)
    {
        case HTML_PROP_TITLE:
            g_free (html->priv->title);
            html->priv->title = g_strdup (g_value_get_string (value));
            g_object_notify (object, "title");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_html_get_property (GObject        *object,
                       guint           prop_id,
                       GValue         *value,
                       GParamSpec     *pspec)
{
    MooHtml *html = MOO_HTML (object);

    switch (prop_id)
    {
        case HTML_PROP_TITLE:
            g_value_set_string (value, html->priv->title);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_html_finalize (GObject      *object)
{
    int i;
    MooHtml *html = MOO_HTML (object);

    g_hash_table_destroy (html->priv->anchors);
    g_hash_table_destroy (html->priv->root_tags);
    g_slist_free (html->priv->href_tags);

    g_free (html->priv->title);
    g_free (html->priv->hover_link);

    if (html->priv->doc)
        xmlFreeDoc (html->priv->doc);

    g_hash_table_destroy (html->priv->font_faces);
    g_free (html->priv->monospace);
    for (i = 0; i < 6; ++i)
        g_free (html->priv->heading_faces[i]);

    g_free (html->priv->filename);
    g_free (html->priv->basename);
    g_free (html->priv->dirname);

    g_free (html->priv);

    G_OBJECT_CLASS (moo_html_parent_class)->finalize (object);
}


GtkWidget*
moo_html_new (void)
{
    return GTK_WIDGET (g_object_new (MOO_TYPE_HTML, NULL));
}


static gboolean
moo_html_load_url_real (MooHtml        *html,
                        const char     *url)
{
    char *scheme, *base, *anchor;
    gboolean result = FALSE;

    g_return_val_if_fail (MOO_IS_HTML (html), FALSE);
    g_return_val_if_fail (url != NULL, FALSE);

    if (!moo_html_parse_url (url, &scheme, &base, &anchor))
        g_return_val_if_reached (FALSE);

    if (!scheme)
        scheme = g_strdup ("file://");

    if (!strcmp (scheme, "mailto://"))
    {
        result = moo_open_email (base, NULL, NULL);
        goto out;
    }

    if (strcmp (scheme, "file://"))
        goto out;

    if (!base || (html->priv->basename && !strcmp (html->priv->basename, base)))
    {
        if (anchor)
            result = moo_html_goto_anchor (html, anchor);
        else
            result = TRUE;
    }
    else if (!g_path_is_absolute (base))
    {
        if (html->priv->dirname)
        {
            char *filename = g_build_filename (html->priv->dirname, base, NULL);

            result = moo_html_load_file (html, filename, NULL);

            if (result && anchor)
                moo_html_goto_anchor (html, anchor);

            g_free (filename);
        }
    }
    else
    {
        result = moo_html_load_file (html, base, NULL);

        if (result && anchor)
            moo_html_goto_anchor (html, anchor);
    }

out:
    g_free (scheme);
    g_free (base);
    g_free (anchor);
    return result;
}


static void
remove_tag (GtkTextTag      *tag,
            GtkTextTagTable *table)
{
    gtk_text_tag_table_remove (table, tag);
}


static void
moo_html_clear (MooHtml        *html)
{
    GtkTextBuffer *buffer;
    GtkTextTagTable *table;
    GtkTextIter start, end;

    g_hash_table_destroy (html->priv->anchors);
    html->priv->anchors = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (html));
    table = gtk_text_buffer_get_tag_table (buffer);
    g_slist_foreach (html->priv->href_tags, (GFunc) remove_tag, table);
    g_slist_free (html->priv->href_tags);
    html->priv->href_tags = NULL;

    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_delete (buffer, &start, &end);

    html->priv->new_line = TRUE;
    html->priv->space = TRUE;

    if (html->priv->doc)
        xmlFreeDoc (html->priv->doc);
    html->priv->doc = NULL;

    g_slist_free (html->priv->rulers);
    html->priv->rulers = NULL;
}


gboolean
moo_html_load_memory (MooHtml            *html,
                      const char         *buffer,
                      int                 size,
                      const char         *url,
                      const char         *encoding)
{
    htmlDocPtr doc;

    g_return_val_if_fail (MOO_IS_HTML (html), FALSE);
    g_return_val_if_fail (buffer != NULL, FALSE);

    if (size < 0)
        size = strlen (buffer);

    doc = htmlReadMemory (buffer, size, url, encoding,
                          HTML_PARSE_NONET);

    if (!doc)
        return FALSE;

    g_free (html->priv->filename);
    g_free (html->priv->basename);
    g_free (html->priv->dirname);
    html->priv->filename = NULL;
    html->priv->basename = NULL;
    html->priv->dirname = NULL;

    moo_html_set_doc (html, doc);

    xmlCleanupParser ();
    return TRUE;
}


gboolean
moo_html_load_file (MooHtml            *html,
                    const char         *file,
                    const char         *encoding)
{
    htmlDocPtr doc;

    g_return_val_if_fail (MOO_IS_HTML (html), FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    doc = htmlReadFile (file, encoding, HTML_PARSE_NONET);

    if (!doc)
        return FALSE;

    g_free (html->priv->filename);
    g_free (html->priv->basename);
    g_free (html->priv->dirname);

    html->priv->filename = g_strdup (file);
    html->priv->basename = g_path_get_basename (file);
    html->priv->dirname = g_path_get_dirname (file);

    moo_html_set_doc (html, doc);

    xmlCleanupParser ();
    return TRUE;
}


static void
moo_html_set_doc (MooHtml        *html,
                  htmlDocPtr      doc)
{
    g_return_if_fail (doc != NULL);
    g_return_if_fail (doc != html->priv->doc);

    moo_html_clear (html);

    html->priv->doc = doc;
    moo_html_load_doc (html, doc);
}


static void
attr_compose (MooHtmlAttr       *dest,
              const MooHtmlAttr *src)
{
    static MooHtmlAttrMask simple =
            MOO_HTML_BOLD | MOO_HTML_ITALIC | MOO_HTML_UNDERLINE |
            MOO_HTML_STRIKETHROUGH | MOO_HTML_MONOSPACE | MOO_HTML_SUB |
            MOO_HTML_SUP | MOO_HTML_PRE;
    static MooHtmlAttrMask font_size_mask =
            MOO_HTML_LARGER | MOO_HTML_SMALLER | MOO_HTML_HEADING |
            MOO_HTML_FONT_SIZE | MOO_HTML_FONT_PT_SIZE;
    static MooHtmlAttrMask font_face_mask =
            MOO_HTML_FONT_FACE | MOO_HTML_MONOSPACE;

    g_return_if_fail (dest != NULL);

    if (!src)
        return;

    dest->mask |= (src->mask & simple);

    if (src->mask & MOO_HTML_FG)
    {
        dest->mask |= MOO_HTML_FG;
        dest->fg = src->fg;
    }

    if (src->mask & MOO_HTML_BG)
    {
        dest->mask |= MOO_HTML_BG;
        dest->bg = src->bg;
    }

    if (src->mask & MOO_HTML_LINK)
    {
        dest->mask |= MOO_HTML_LINK;
        dest->link = src->link;
    }

    if (src->mask & MOO_HTML_LEFT_MARGIN)
    {
        if (!(dest->mask & MOO_HTML_LEFT_MARGIN))
        {
            dest->mask |= MOO_HTML_LEFT_MARGIN;
            dest->left_margin = src->left_margin;
        }
        else
        {
            dest->left_margin += src->left_margin;
        }
    }

    if ((src->mask & font_face_mask) && !(dest->mask & font_face_mask))
    {
        dest->mask |= (src->mask & font_face_mask);
        dest->font_face = src->font_face;
    }

    if (dest->mask & MOO_HTML_HEADING)
    {
        g_assert (1 <= dest->heading && dest->heading <= 6);
    }
    else if (src->mask & MOO_HTML_HEADING)
    {
        dest->mask &= ~font_size_mask;
        dest->mask |= MOO_HTML_HEADING;
        dest->heading = src->heading;
        g_assert (1 <= dest->heading && dest->heading <= 6);
    }
    else if (dest->mask & (MOO_HTML_LARGER | MOO_HTML_SMALLER))
    {
        int size = 3;
        int scale = (dest->mask & MOO_HTML_LARGER) ? dest->scale : -dest->scale;

        if (src->mask & (MOO_HTML_LARGER | MOO_HTML_SMALLER))
        {
            if (src->mask & MOO_HTML_LARGER)
                scale += src->scale;
            else
                scale -= src->scale;
        }
        else if (src->mask & MOO_HTML_FONT_SIZE)
        {
            size = src->font_size;
        }
        else if (src->mask & MOO_HTML_FONT_PT_SIZE)
        {
            /* XXX ??? */
        }

        size += scale;
        size = CLAMP (size, 1, 7);

        dest->mask &= ~font_size_mask;
        dest->mask |= MOO_HTML_FONT_SIZE;
        dest->font_size = size;
    }
    else if (dest->mask & MOO_HTML_FONT_SIZE)
    {
        dest->mask &= ~font_size_mask;
        dest->mask |= MOO_HTML_FONT_SIZE;
    }
    else if (dest->mask & MOO_HTML_FONT_PT_SIZE)
    {
        dest->mask &= ~font_size_mask;
        dest->mask |= MOO_HTML_FONT_PT_SIZE;
    }
    else
    {
        dest->mask &= ~font_size_mask;
        dest->mask |= (src->mask & font_size_mask);
        dest->heading = src->heading;
        dest->font_size = src->font_size;
        dest->font_pt_size = src->font_pt_size;
        dest->scale = src->scale;
    }
}


static void
attr_apply (const MooHtmlAttr *attr,
            MooHtmlTag        *tag,
            MooHtml           *html)
{
    g_return_if_fail (attr != NULL && tag != NULL);

    moo_html_attr_free (tag->attr);
    tag->attr = moo_html_attr_copy (attr);

    if (attr->mask & MOO_HTML_FG)
        g_object_set (tag, "foreground", attr->fg, NULL);
    if (attr->mask & MOO_HTML_BG)
        g_object_set (tag, "background", attr->bg, NULL);
    if (attr->mask & MOO_HTML_BOLD)
        g_object_set (tag, "weight", PANGO_WEIGHT_BOLD, NULL);
    if (attr->mask & MOO_HTML_ITALIC)
        g_object_set (tag, "style", PANGO_STYLE_ITALIC, NULL);
    if (attr->mask & MOO_HTML_UNDERLINE)
        g_object_set (tag, "underline", PANGO_UNDERLINE_SINGLE, NULL);
    if (attr->mask & MOO_HTML_STRIKETHROUGH)
        g_object_set (tag, "strikethrough", TRUE, NULL);

    if (attr->mask & MOO_HTML_LEFT_MARGIN)
        g_object_set (tag, "left-margin", attr->left_margin, NULL);

    if (attr->mask & MOO_HTML_LINK)
    {
        g_free (tag->href);
        tag->href = g_strdup (attr->link);
        g_object_set (tag, "foreground", "blue", NULL);
    }

    if (attr->mask & MOO_HTML_SUP)
        g_object_set (tag,
                      "rise", 8 * PANGO_SCALE,
                      "size", 8 * PANGO_SCALE,
                      NULL);
    if (attr->mask & MOO_HTML_SUB)
        g_object_set (tag,
                      "rise", -8 * PANGO_SCALE,
                      "size", 8 * PANGO_SCALE,
                      NULL);

    if (attr->mask & MOO_HTML_MONOSPACE)
        g_object_set (tag, "font", html->priv->monospace, NULL);
    else if (attr->mask & MOO_HTML_FONT_FACE)
        g_object_set (tag, "font", attr->font_face, NULL);

    if (attr->mask & MOO_HTML_HEADING)
    {
        moo_html_make_heading_tag (html, tag, attr->heading);
    }
    else if (attr->mask & MOO_HTML_LARGER)
    {
        double scale;
        int space;
        int size = 3 + attr->scale;
        size = CLAMP (size, 1, 7);
        scale = html->priv->font_sizes[size - 1];
        space = html->priv->par_spacing[size - 1];
        g_object_set (tag,
                      "scale", scale,
                      "pixels-below-lines", DEFAULT_PAR_SPACING + space,
                      NULL);
    }
    else if (attr->mask & MOO_HTML_SMALLER)
    {
        double scale;
        int space;
        int size = 3 - attr->scale;
        size = CLAMP (size, 1, 7);
        scale = html->priv->font_sizes[size - 1];
        space = html->priv->par_spacing[size - 1];
        g_object_set (tag,
                      "scale", scale,
                      "pixels-below-lines", DEFAULT_PAR_SPACING + space,
                      NULL);
    }
    else if (attr->mask & MOO_HTML_FONT_SIZE)
    {
        g_assert (1 <= attr->font_size && attr->font_size <= 7);
        g_object_set (tag,
                      "scale", html->priv->font_sizes[attr->font_size - 1],
                      "pixels-below-lines",
                      DEFAULT_PAR_SPACING + html->priv->par_spacing[attr->font_size - 1],
                      NULL);
    }
    else if (attr->mask & MOO_HTML_FONT_PT_SIZE)
    {
        g_object_set (tag, "size-points", (double) attr->font_pt_size, NULL);
    }
}


static void
moo_html_make_heading_tag (MooHtml        *html,
                           MooHtmlTag     *tag,
                           guint           heading)
{
    g_assert (1 <= heading && heading <= 6);

    g_object_set (tag,
                  "pixels-below-lines",
                  DEFAULT_PAR_SPACING + html->priv->heading_spacing[heading - 1],
                  "scale", html->priv->heading_sizes[heading - 1],
                  "weight", PANGO_WEIGHT_BOLD, NULL);

    if (html->priv->heading_faces[heading - 1])
        g_object_set (tag, "family",
                      html->priv->heading_faces[heading - 1], NULL);
}


static MooHtmlTag*
moo_html_create_tag (MooHtml           *html,
                     const MooHtmlAttr *attr,
                     MooHtmlTag        *parent,
                     gboolean           force)
{
    MooHtmlTag *tag;
    MooHtmlAttr real_attr;

    g_return_val_if_fail (attr != NULL, NULL);

    if (!attr->mask && !force)
        return parent;

    if (parent && parent->attr)
    {
        real_attr = *parent->attr;
        attr_compose (&real_attr, attr);
    }
    else
    {
        real_attr = *attr;
    }

    tag = g_object_new (MOO_TYPE_HTML_TAG, NULL);
    gtk_text_tag_table_add (gtk_text_buffer_get_tag_table (gtk_text_view_get_buffer (GTK_TEXT_VIEW (html))),
                            GTK_TEXT_TAG (tag));
    g_object_unref (tag);

    if (tag->href)
        html->priv->href_tags = g_slist_prepend (html->priv->href_tags, tag);

    attr_apply (&real_attr, tag, html);

    return tag;
}


static MooHtmlAttr*
moo_html_attr_copy (const MooHtmlAttr *src)
{
    MooHtmlAttr *attr;

    g_return_val_if_fail (src != NULL, NULL);

    attr = g_new (MooHtmlAttr, 1);
    memcpy (attr, src, sizeof (MooHtmlAttr));

    attr->fg = g_strdup (src->fg);
    attr->bg = g_strdup (src->bg);
    attr->link = g_strdup (src->link);
    attr->font_face = g_strdup (src->font_face);

    return attr;
}


static void
moo_html_attr_free (MooHtmlAttr        *attr)
{
    if (attr)
    {
        g_free (attr->fg);
        g_free (attr->bg);
        g_free (attr->link);
        g_free (attr->font_face);
        g_free (attr);
    }
}


static void
moo_html_create_anchor (MooHtml        *html,
                        GtkTextBuffer  *buffer,
                        GtkTextIter    *iter,
                        const char     *name)
{
    GtkTextMark *mark;
    char *alt_name;

    g_return_if_fail (name != NULL && (name[0] != '#' || name[1]));

    mark = gtk_text_buffer_create_mark (buffer, NULL, iter, TRUE);

    if (name[0] == '#')
        alt_name = g_strdup (name + 1);
    else
        alt_name = g_strdup_printf ("#%s", name);

    g_hash_table_insert (html->priv->anchors, g_strdup (name), mark);
    g_hash_table_insert (html->priv->anchors, alt_name, mark);
}


static gboolean
moo_html_motion (GtkWidget      *widget,
                 GdkEventMotion *event)
{
    GtkTextView *textview = GTK_TEXT_VIEW (widget);
    MooHtml *html = MOO_HTML (widget);
    GtkTextIter iter;
    int buf_x, buf_y, x, y, dummy;
    GdkModifierType state;
    MooHtmlTag *tag;

    if (html->priv->button_pressed)
        html->priv->in_drag = TRUE;

    if (event->window != gtk_text_view_get_window (textview, GTK_TEXT_WINDOW_TEXT))
        goto out;

    if (event->is_hint)
    {
        gdk_window_get_pointer (event->window, &x, &y, &state);
    }
    else
    {
        x = event->x;
        y = event->y;
    }

    if (state & (GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK))
        goto out;

    gtk_text_view_window_to_buffer_coords (textview, GTK_TEXT_WINDOW_TEXT,
                                           x, y, &buf_x, &buf_y);
    gtk_text_view_get_iter_at_position (textview, &iter, &dummy, buf_x, buf_y);

    tag = moo_html_get_link_tag (html, &iter);

    if (tag)
    {
        g_return_val_if_fail (tag->href != NULL, FALSE);

        if (!html->priv->hover_link || strcmp (html->priv->hover_link, tag->href))
        {
            GdkCursor *cursor;

            g_free (html->priv->hover_link);
            html->priv->hover_link = g_strdup (tag->href);

            cursor = gdk_cursor_new (GDK_HAND2);
            gdk_window_set_cursor (event->window, cursor);
            gdk_cursor_unref (cursor);

            g_signal_emit (html, html_signals[HOVER_LINK], 0, tag->href);
        }
    }
    else if (html->priv->hover_link)
    {
        GdkCursor *cursor;

        g_free (html->priv->hover_link);
        html->priv->hover_link = NULL;

        cursor = gdk_cursor_new (GDK_XTERM);
        gdk_window_set_cursor (event->window, cursor);
        gdk_cursor_unref (cursor);

        g_signal_emit (html, html_signals[HOVER_LINK], 0, NULL);
    }

out:
    return GTK_WIDGET_CLASS(moo_html_parent_class)->motion_notify_event (widget, event);
}


static MooHtmlTag*
moo_html_get_link_tag (MooHtml        *html,
                       GtkTextIter    *iter)
{
    MooHtmlTag *tag = moo_html_get_tag (html, iter);
    return (tag && tag->href) ? tag : NULL;
}


static MooHtmlTag*
moo_html_get_tag (G_GNUC_UNUSED MooHtml *html,
                  GtkTextIter    *iter)
{
    MooHtmlTag *tag = NULL;
    GSList *l;
    GSList *list = gtk_text_iter_get_tags (iter);

    for (l = list; l != NULL; l = l->next)
    {
        if (MOO_IS_HTML_TAG (l->data))
        {
            tag = l->data;
            break;
        }
    }

    g_slist_free (list);
    return tag;
}


static gboolean
moo_html_button_press (GtkWidget      *widget,
                       GdkEventButton *event)
{
    MooHtml *html = MOO_HTML (widget);
    html->priv->button_pressed = TRUE;
    html->priv->in_drag = FALSE;
    return GTK_WIDGET_CLASS(moo_html_parent_class)->button_press_event (widget, event);
}


static gboolean
moo_html_button_release (GtkWidget      *widget,
                         GdkEventButton *event)
{
    GtkTextView *textview = GTK_TEXT_VIEW (widget);
    MooHtml *html = MOO_HTML (widget);
    GtkTextIter iter;
    int buf_x, buf_y, dummy;
    MooHtmlTag *tag;

    html->priv->button_pressed = FALSE;

    if (html->priv->in_drag)
    {
        html->priv->in_drag = FALSE;
        goto out;
    }

    if (event->window != gtk_text_view_get_window (textview, GTK_TEXT_WINDOW_TEXT))
        goto out;

    gtk_text_view_window_to_buffer_coords (textview, GTK_TEXT_WINDOW_TEXT,
                                           event->x, event->y, &buf_x, &buf_y);
    gtk_text_view_get_iter_at_position (textview, &iter, &dummy, buf_x, buf_y);

    tag = moo_html_get_link_tag (html, &iter);

    if (tag)
    {
        gboolean result;
        g_assert (tag->href != NULL);
        g_signal_emit (html, html_signals[LOAD_URL], 0, tag->href, &result);
    }

out:
    return GTK_WIDGET_CLASS(moo_html_parent_class)->button_release_event (widget, event);
}


static gboolean
moo_html_parse_url (const char     *url,
                    char          **scheme,
                    char          **base,
                    char          **anchor)
{
    EggRegex *regex;

    g_return_val_if_fail (url != NULL, FALSE);
    g_return_val_if_fail (scheme && base && anchor, FALSE);

    regex = egg_regex_new ("^([a-zA-Z]+://)?([^#]*)(#(.*))?$", 0, 0, NULL);
    g_return_val_if_fail (regex != NULL, FALSE);

    if (egg_regex_match (regex, url, -1, 0) < 1)
    {
        egg_regex_unref (regex);
        return FALSE;
    }

    *scheme = egg_regex_fetch (regex, url, 1);
    *base = egg_regex_fetch (regex, url, 2);
    *anchor = egg_regex_fetch (regex, url, 4);

    if (!*scheme || !**scheme) {g_free (*scheme); *scheme = NULL;}
    if (!*base || !**base) {g_free (*base); *base = NULL;}
    if (!*anchor || !**anchor) {g_free (*anchor); *anchor = NULL;}

    egg_regex_unref (regex);
    return TRUE;
}


static gboolean
moo_html_goto_anchor (MooHtml        *html,
                      const char     *anchor)
{
    GtkTextMark *mark;

    g_return_val_if_fail (anchor != NULL, FALSE);

    mark = g_hash_table_lookup (html->priv->anchors, anchor);

    if (!mark)
    {
        g_warning ("%s: could not find anchor '%s'",
                   G_STRLOC, anchor);
        return FALSE;
    }
    else
    {
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (html), mark,
                                      0.1, TRUE, 0, 0);
        return TRUE;
    }
}


void
moo_html_set_font (MooHtml            *html,
                   const char         *string)
{
    PangoFontDescription *font;

    g_return_if_fail (MOO_IS_HTML (html));
    g_return_if_fail (string != NULL);

    font = pango_font_description_from_string (string);
    g_return_if_fail (font != NULL);

    gtk_widget_modify_font (GTK_WIDGET (html), font);
    pango_font_description_free (font);
}


static void
moo_html_size_allocate (GtkWidget      *widget,
                        GtkAllocation  *allocation)
{
    MooHtml *html = MOO_HTML (widget);
    int border_width, child_width, height;
    GSList *l;
    GdkWindow *window;

    GTK_WIDGET_CLASS(moo_html_parent_class)->size_allocate (widget, allocation);

    if (!GTK_WIDGET_REALIZED (widget))
        return;

    window = gtk_text_view_get_window (GTK_TEXT_VIEW (widget), GTK_TEXT_WINDOW_TEXT);
    g_return_if_fail (window != NULL);

    gdk_drawable_get_size (window, &child_width, &height);
    border_width = gtk_container_get_border_width (GTK_CONTAINER (widget));
    child_width -= 2 * border_width + 2 * widget->style->xthickness;
    child_width = MAX (child_width, 0);

    for (l = html->priv->rulers; l != NULL; l = l->next)
    {
        GtkWidget *ruler = l->data;
        gtk_widget_set_size_request (ruler, child_width, -1);
    }
}


/***********************************************************************/
/* Loading into text buffer
 */

#define IS_ELEMENT(node_)           (node_ && node_->type == XML_ELEMENT_NODE)
#define NODE_NAME_IS_(node_,name_)  (node_->name && !strcmp ((char*)node_->name, name_))
#define IS_NAMED_ELM_(node_,name_)  (IS_ELEMENT(node_) && NODE_NAME_IS_(node_, name_))

#define IS_HEAD_ELEMENT(node_)      IS_NAMED_ELM_(node_, "head")
#define IS_BODY_ELEMENT(node_)      IS_NAMED_ELM_(node_, "body")
#define IS_TITLE_ELEMENT(node_)     IS_NAMED_ELM_(node_, "title")
#define IS_META_ELEMENT(node_)      IS_NAMED_ELM_(node_, "meta")
#define IS_LINK_ELEMENT(node_)      IS_NAMED_ELM_(node_, "link")
#define IS_LI_ELEMENT(node_)        IS_NAMED_ELM_(node_, "li")
#define IS_IMG_ELEMENT(node_)       IS_NAMED_ELM_(node_, "img")

#define IS_HEADING_ELEMENT(node_)   (IS_ELEMENT(node_) && node_->name &&                    \
                                     node_->name[0] == 'h' &&                               \
                                     ('1' <= node_->name[1] && node_->name[1] <= '6') &&    \
                                     !node_->name[2])

#define IS_TEXT(node_)              (node_ && node_->type == XML_TEXT_NODE)
#define IS_COMMENT(node_)           (node_ && node_->type == XML_COMMENT_NODE)

#define STR_FREE(s__) if (s__) xmlFree (s__)


static void moo_html_load_head      (MooHtml        *html,
                                     xmlNode        *node);
static void moo_html_load_body      (MooHtml        *html,
                                     xmlNode        *node);
static void moo_html_new_line       (MooHtml        *html,
                                     GtkTextBuffer  *buffer,
                                     GtkTextIter    *iter,
                                     MooHtmlTag     *tag,
                                     gboolean        force);
static void moo_html_insert_text    (MooHtml        *html,
                                     GtkTextBuffer  *buffer,
                                     GtkTextIter    *iter,
                                     MooHtmlTag     *tag,
                                     const char     *text);
static void moo_html_insert_verbatim(MooHtml        *html,
                                     GtkTextBuffer  *buffer,
                                     GtkTextIter    *iter,
                                     MooHtmlTag     *tag,
                                     const char     *text);


static void
moo_html_load_doc (MooHtml        *html,
                   htmlDocPtr      doc)
{
    xmlNode *root, *node;

    root = xmlDocGetRootElement (doc);

    if (!root)
        return;

    for (node = root->children; node != NULL; node = node->next)
    {
        if (IS_HEAD_ELEMENT (node))
        {
            moo_html_load_head (html, node);
        }
        else if (IS_BODY_ELEMENT (node))
        {
            moo_html_load_body (html, node);
        }
        else
        {
            g_warning ("%s: unknown node '%s'", G_STRLOC, node->name);
        }
    }
}


static void
moo_html_load_head (MooHtml        *html,
                    xmlNode        *node)
{
    xmlNode *child;

    for (child = node->children; child != NULL; child = child->next)
    {
        if (IS_TITLE_ELEMENT (child))
        {
            xmlChar *title = xmlNodeGetContent (child);
            g_object_set (html, "title", title, NULL);
            STR_FREE (title);
        }
        else if (IS_META_ELEMENT (child))
        {
        }
        else if (IS_LINK_ELEMENT (child))
        {
        }
        else
        {
            g_message ("%s: unknown node '%s'", G_STRLOC, child->name);
        }
    }
}


static void
moo_html_new_line (MooHtml        *html,
                   GtkTextBuffer  *buffer,
                   GtkTextIter    *iter,
                   MooHtmlTag     *tag,
                   gboolean        force)
{
    if (!html->priv->new_line || force)
    {
        if (tag)
            gtk_text_buffer_insert_with_tags (buffer, iter, "\n", 1,
                                              GTK_TEXT_TAG (tag), NULL);
        else
            gtk_text_buffer_insert (buffer, iter, "\n", 1);
    }

    html->priv->new_line = TRUE;
    html->priv->space = TRUE;
}


static const char*
str_find_separator (const char *str)
{
    const char *p;

    for (p = str; *p; ++p)
    {
        if (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t')
            return p;
    }

    return NULL;
}


static void
moo_html_insert_text (MooHtml        *html,
                      GtkTextBuffer  *buffer,
                      GtkTextIter    *iter,
                      MooHtmlTag     *tag,
                      const char     *text)
{
    const char *p;

    if (tag && tag->attr && (tag->attr->mask & MOO_HTML_PRE))
        return moo_html_insert_verbatim (html, buffer, iter, tag, text);

    while (*text)
    {
        p = str_find_separator (text);

        if (p)
        {
            if (p != text)
            {
                if (tag)
                {
                    gtk_text_buffer_insert_with_tags (buffer, iter,
                                                      text, p - text,
                                                      GTK_TEXT_TAG (tag), NULL);
                    gtk_text_buffer_insert_with_tags (buffer, iter, " ", 1,
                                                      GTK_TEXT_TAG (tag), NULL);
                }
                else
                {
                    gtk_text_buffer_insert (buffer, iter, text, p - text);
                    gtk_text_buffer_insert (buffer, iter, " ", 1);
                }

                html->priv->space = TRUE;
                html->priv->new_line = FALSE;
                text = ++p;
            }
            else
            {
                if (!html->priv->space)
                {
                    gtk_text_buffer_insert (buffer, iter, " ", 1);
                    html->priv->space = TRUE;
                }

                text++;
            }
        }
        else
        {
            if (tag)
                gtk_text_buffer_insert_with_tags (buffer, iter,
                                                  text, -1,
                                                  GTK_TEXT_TAG (tag), NULL);
            else
                gtk_text_buffer_insert (buffer, iter, text, -1);
            html->priv->new_line = FALSE;
            html->priv->space = FALSE;
            break;
        }
    }
}


static gboolean
str_has_trailing_nl (const char *text, int len)
{
    g_assert (len > 0);

    for (len = len - 1; len >= 0; --len)
    {
        if (text[len] == '\n' || text[len] == '\r')
            return TRUE;
        else if (text[len] != ' ' && text[len] != '\t')
            return FALSE;
    }

    return FALSE;
}


static gboolean
str_has_trailing_space (const char *text, int len)
{
    g_assert (len > 0);

    if (text[len-1] == '\n' || text[len-1] == '\r' || text[len-1] == ' ' || text[len-1] == '\t')
        return TRUE;
    else
        return FALSE;
}


static void
moo_html_insert_verbatim (MooHtml        *html,
                          GtkTextBuffer  *buffer,
                          GtkTextIter    *iter,
                          MooHtmlTag     *tag,
                          const char     *text)
{
    guint len;

    g_return_if_fail (text != NULL);

    if (text[0] == '\n' && html->priv->new_line)
        text++;

    len = strlen (text);

    if (!len)
        return;

    if (tag)
        gtk_text_buffer_insert_with_tags (buffer, iter,
                                          text, len,
                                          GTK_TEXT_TAG (tag), NULL);
    else
        gtk_text_buffer_insert (buffer, iter, text, len);

    html->priv->new_line = str_has_trailing_nl (text, len);

    if (html->priv->new_line)
        html->priv->space = TRUE;
    else
        html->priv->space = str_has_trailing_space (text, len);
}


static void process_elm_body    (MooHtml        *html,
                                 GtkTextBuffer  *buffer,
                                 xmlNode        *elm,
                                 MooHtmlTag     *current,
                                 GtkTextIter    *iter);
static void process_text_node   (MooHtml        *html,
                                 GtkTextBuffer  *buffer,
                                 xmlNode        *node,
                                 MooHtmlTag     *current,
                                 GtkTextIter    *iter);
static void process_heading_elm (MooHtml        *html,
                                 GtkTextBuffer  *buffer,
                                 xmlNode        *elm,
                                 MooHtmlTag     *current,
                                 GtkTextIter    *iter);
static void process_format_elm  (MooHtml        *html,
                                 MooHtmlAttr    *attr,
                                 GtkTextBuffer  *buffer,
                                 xmlNode        *elm,
                                 MooHtmlTag     *current,
                                 GtkTextIter    *iter);

static void process_img_elm     (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *current, GtkTextIter *iter);
static void process_p_elm       (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_a_elm       (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_pre_elm     (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_ol_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_ul_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_font_elm    (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_cite_elm    (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_li_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_dt_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_dl_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_dd_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_br_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_div_elm     (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_span_elm    (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_hr_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);

static void process_table_elm   (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
static void process_tr_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                                 MooHtmlTag *parent, GtkTextIter *iter);
// static void process_td_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
//                                  MooHtmlTag *parent, GtkTextIter *iter);
// static void process_th_elm      (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
//                                  MooHtmlTag *parent, GtkTextIter *iter);
// static void process_tbody_elm   (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
//                                  MooHtmlTag *parent, GtkTextIter *iter);


static void
moo_html_load_body (MooHtml        *html,
                    xmlNode        *node)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer;

    html->priv->new_line = TRUE;
    html->priv->space = TRUE;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (html));
    gtk_text_buffer_get_end_iter (buffer, &iter);

    process_elm_body (html, buffer, node, NULL, &iter);
}


typedef struct {
    MooHtmlAttrMask mask;
    const char *name;
} MaskNamePair;

static MooHtmlAttr*
get_format_elm_attr (xmlNode *node)
{
    static GHashTable *elms = NULL;
    MooHtmlAttr *attr;

    if (!IS_ELEMENT (node))
        return NULL;

    if (!elms)
    {
        guint i;
        static MaskNamePair attrs[] = {
            { MOO_HTML_BOLD, "strong" },
            { MOO_HTML_BOLD, "b" },
            { MOO_HTML_ITALIC, "em" },
            { MOO_HTML_ITALIC, "i" },
            { MOO_HTML_ITALIC, "address" },
            { MOO_HTML_UNDERLINE, "ins" },
            { MOO_HTML_UNDERLINE, "u" },
            { MOO_HTML_STRIKETHROUGH, "del" },
            { MOO_HTML_STRIKETHROUGH, "s" },
            { MOO_HTML_STRIKETHROUGH, "strike" },
            { MOO_HTML_MONOSPACE, "code" },
            { MOO_HTML_MONOSPACE, "dfn" },
            { MOO_HTML_MONOSPACE, "samp" },
            { MOO_HTML_MONOSPACE, "kbd" },
            { MOO_HTML_MONOSPACE, "var" },
            { MOO_HTML_MONOSPACE, "tt" },
            { MOO_HTML_SUB, "sub" },
            { MOO_HTML_SUP, "sup" }
        };

        elms = g_hash_table_new (g_str_hash, g_str_equal);

        for (i = 0; i < G_N_ELEMENTS (attrs); ++i)
        {
            attr = g_new0 (MooHtmlAttr, 1);
            attr->mask = attrs[i].mask;
            g_hash_table_insert (elms, (char*) attrs[i].name, attr);
        }

        attr = g_new0 (MooHtmlAttr, 1);
        attr->mask = MOO_HTML_LARGER;
        attr->scale = 1;
        g_hash_table_insert (elms, (char*) "big", attr);

        attr = g_new0 (MooHtmlAttr, 1);
        attr->mask = MOO_HTML_SMALLER;
        attr->scale = 1;
        g_hash_table_insert (elms, (char*) "small", attr);
    }

    return g_hash_table_lookup (elms, node->name);
}


typedef void (*ProcessElm) (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                            MooHtmlTag *parent, GtkTextIter *iter);

static GHashTable *proc_elm_funcs__ = NULL;

static void
add_func__ (const char *static_elm_name,
            ProcessElm  func)
{
    g_hash_table_insert (proc_elm_funcs__, (char*) static_elm_name, func);
}

static void
init_funcs (void)
{
    if (!proc_elm_funcs__)
    {
        proc_elm_funcs__ = g_hash_table_new (g_str_hash, g_str_equal);

        add_func__ ("p", process_p_elm);
        add_func__ ("a", process_a_elm);
        add_func__ ("ol", process_ol_elm);
        add_func__ ("ul", process_ul_elm);
        add_func__ ("font", process_font_elm);
        add_func__ ("pre", process_pre_elm);
        add_func__ ("cite", process_cite_elm);
        add_func__ ("ul", process_ul_elm);
        add_func__ ("li", process_li_elm);
        add_func__ ("hr", process_hr_elm);
        add_func__ ("img", process_img_elm);
        add_func__ ("dt", process_dt_elm);
        add_func__ ("dl", process_dl_elm);
        add_func__ ("dd", process_dd_elm);
        add_func__ ("br", process_br_elm);
        add_func__ ("div", process_div_elm);
        add_func__ ("span", process_span_elm);
    }
}

static ProcessElm
get_proc_elm_func (xmlNode *node)
{
    if (IS_ELEMENT (node))
        return g_hash_table_lookup (proc_elm_funcs__, node->name);
    else
        return NULL;
}


static void
process_elm_body (MooHtml        *html,
                  GtkTextBuffer  *buffer,
                  xmlNode        *elm,
                  MooHtmlTag     *current,
                  GtkTextIter    *iter)
{
    xmlNode *child;
    MooHtmlAttr *attr;
    ProcessElm func;

    for (child = elm->children; child != NULL; child = child->next)
    {

        if (IS_TEXT (child))
            process_text_node (html, buffer, child, current, iter);
        else if (IS_HEADING_ELEMENT (child))
            process_heading_elm (html, buffer, child, current, iter);
        else if ((func = get_proc_elm_func (child)))
            func (html, buffer, child, current, iter);
        else if ((attr = get_format_elm_attr (child)))
            process_format_elm (html, attr, buffer, child, current, iter);

        else if (IS_NAMED_ELM_ (child, "table"))
            process_table_elm (html, buffer, child, current, iter);
        else if (IS_NAMED_ELM_ (child, "tr"))
            process_tr_elm (html, buffer, child, current, iter);

        else if (IS_NAMED_ELM_ (child, "td") ||
                 IS_NAMED_ELM_ (child, "th") ||
                 IS_NAMED_ELM_ (child, "tbody") ||
                 IS_NAMED_ELM_ (child, "col"))
        {
            process_elm_body (html, buffer, child, current, iter);
        }

        else if (IS_ELEMENT (child))
        {
            g_message ("%s: unknown node '%s'", G_STRLOC, child->name);
            process_elm_body (html, buffer, child, current, iter);
        }
        else if (IS_COMMENT (child))
        {
            /* ignore */
        }
        else
        {
            g_warning ("%s: unknown node", G_STRLOC);
        }
    }
}


static void
process_p_elm (MooHtml        *html,
               GtkTextBuffer  *buffer,
               xmlNode        *elm,
               MooHtmlTag     *current,
               GtkTextIter    *iter)
{
    moo_html_new_line (html, buffer, iter, current, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
    moo_html_new_line (html, buffer, iter, current, FALSE);
}


static void
process_heading_elm (MooHtml        *html,
                     GtkTextBuffer  *buffer,
                     xmlNode        *elm,
                     MooHtmlTag     *parent,
                     GtkTextIter    *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    int n;

    g_return_if_fail (elm->name[0] && elm->name[1]);

    n = elm->name[1] - '0';
    g_return_if_fail (1 <= n && n <= 6);

    attr.mask = MOO_HTML_HEADING;
    attr.heading = n;
    current = moo_html_create_tag (html, &attr, parent, FALSE);

    moo_html_new_line (html, buffer, iter, current, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
    moo_html_new_line (html, buffer, iter, current, FALSE);
}


static void
process_text_node (MooHtml        *html,
                   GtkTextBuffer  *buffer,
                   xmlNode        *node,
                   MooHtmlTag     *current,
                   GtkTextIter    *iter)
{
    moo_html_insert_text (html, buffer, iter, current, (char*) node->content);
}


#define GET_PROP(elm__,prop__) (xmlGetProp (elm__, (const guchar*) prop__))
#define STRCMP(xm__,normal__) (strcmp ((char*) xm__, normal__))


static void
process_a_elm (MooHtml        *html,
               GtkTextBuffer  *buffer,
               xmlNode        *elm,
               MooHtmlTag     *parent,
               GtkTextIter    *iter)
{
    xmlChar *href, *name;

    href = GET_PROP (elm, "href");
    name = GET_PROP (elm, "name");

    if (!name)
        name = GET_PROP (elm, "id");

    if (href)
    {
        static MooHtmlAttr attr;
        MooHtmlTag *current;

        attr.mask = MOO_HTML_LINK;
        attr.link = (char*) href;

        current = moo_html_create_tag (html, &attr, parent, FALSE);
        process_elm_body (html, buffer, elm, current, iter);
    }
    else if (name)
    {
        moo_html_create_anchor (html, buffer, iter, (char*) name);
    }

    STR_FREE (href);
    STR_FREE (name);
}


static gboolean
parse_int (const char *str,
           int        *dest)
{
    long num;

    if (!str)
        return FALSE;

    errno = 0;
    num = strtol (str, NULL, 10);

    if (errno)
        return FALSE;
    if (num < G_MININT || num > G_MAXINT)
        return FALSE;

    if (dest)
        *dest = num;

    return TRUE;
}


typedef enum {
    OL_NUM = 0,
    OL_LOWER_ALPHA,
    OL_UPPER_ALPHA,
    OL_LOWER_ROMAN,
    OL_UPPER_ROMAN
} OLType;

static char*
make_li_number (int     count,
                OLType  type)
{
    g_return_val_if_fail (count > 0, NULL);

    switch (type)
    {
        case OL_UPPER_ROMAN:
        case OL_LOWER_ROMAN:
            g_warning ("%s: implement me", G_STRLOC);
        case OL_NUM:
            return g_strdup_printf (" %d. ", count);
        case OL_LOWER_ALPHA:
            g_return_val_if_fail (count <= 26, NULL);
            return g_strdup_printf (" %c. ", count - 1 + 'a');
        case OL_UPPER_ALPHA:
            g_return_val_if_fail (count <= 26, NULL);
            return g_strdup_printf (" %c. ", count - 1 + 'A');
    }

    g_return_val_if_reached (NULL);
}

static void
process_ol_elm (MooHtml        *html,
                GtkTextBuffer  *buffer,
                xmlNode        *elm,
                MooHtmlTag     *current,
                GtkTextIter    *iter)
{
    int count;
    OLType list_type = OL_NUM;
    xmlNode *child;
    xmlChar *start = NULL, *type = NULL;

    count = 1;
    start = GET_PROP (elm, "start");
    parse_int ((char*) start, &count);

    if ((type = GET_PROP (elm, "type")))
    {
        if (!STRCMP (type, "1"))
            list_type = OL_NUM;
        else if (!STRCMP (type, "a"))
            list_type = OL_LOWER_ALPHA;
        else if (!STRCMP (type, "A"))
            list_type = OL_UPPER_ALPHA;
        else if (!STRCMP (type, "i"))
            list_type = OL_LOWER_ROMAN;
        else if (!STRCMP (type, "I"))
            list_type = OL_UPPER_ROMAN;
        else
        {
            g_warning ("%s: invalid type attribute '%s'",
                       G_STRLOC, type);
        }
    }

    moo_html_new_line (html, buffer, iter, current, FALSE);

    for (child = elm->children; child != NULL; child = child->next)
    {
        if (IS_LI_ELEMENT (child))
        {
            char *number;
            gboolean had_new_line;
            xmlChar *value;

            value = GET_PROP (child, "value");
            parse_int ((char*) value, &count);

            number = make_li_number (count, list_type);
            had_new_line = html->priv->new_line;

            moo_html_insert_verbatim (html, buffer, iter, current, number);
            html->priv->new_line = had_new_line;
            process_elm_body (html, buffer, child, current, iter);
            moo_html_new_line (html, buffer, iter, current, FALSE);
            count++;

            g_free (number);
            STR_FREE (value);
        }
        else
        {
            g_message ("%s: unknown node '%s'", G_STRLOC, child->name);
            process_elm_body (html, buffer, child, current, iter);
        }
    }

    STR_FREE (start);
    STR_FREE (type);
}


static void
process_ul_elm (MooHtml        *html,
                GtkTextBuffer  *buffer,
                xmlNode        *elm,
                MooHtmlTag     *current,
                GtkTextIter    *iter)
{
    xmlNode *child;
    for (child = elm->children; child != NULL; child = child->next)
        process_elm_body (html, buffer, child, current, iter);
}


static void
process_li_elm (MooHtml        *html,
                GtkTextBuffer  *buffer,
                xmlNode        *elm,
                MooHtmlTag     *current,
                GtkTextIter    *iter)
{
    gboolean had_new_line;

    moo_html_new_line (html, buffer, iter, current, FALSE);

    had_new_line = html->priv->new_line;
    moo_html_insert_verbatim (html, buffer, iter, current, " * ");
    html->priv->new_line = had_new_line;

    process_elm_body (html, buffer, elm, current, iter);
    moo_html_new_line (html, buffer, iter, current, FALSE);
}


static void
process_pre_elm (MooHtml        *html,
                 GtkTextBuffer  *buffer,
                 xmlNode        *elm,
                 MooHtmlTag     *parent,
                 GtkTextIter    *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;

    attr.mask = MOO_HTML_MONOSPACE | MOO_HTML_PRE;
    current = moo_html_create_tag (html, &attr, parent, FALSE);

    process_elm_body (html, buffer, elm, current, iter);
}


static void
process_font_elm (MooHtml       *html,
                  GtkTextBuffer *buffer,
                  xmlNode       *elm,
                  MooHtmlTag    *parent,
                  GtkTextIter   *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    xmlChar *size__, *color, *face;
    int scale = 0;
    guint size_val;
    const xmlChar *size;

    size__ = GET_PROP (elm, "size");
    color = GET_PROP (elm, "color");
    face = GET_PROP (elm, "face");

    attr.mask = 0;
    attr.font_face = NULL;

    if (size__)
    {
        size = size__;

        if (size[0] == '+')
        {
            scale = 1;
            size++;
        }
        else if (size[0] == '-')
        {
            scale = -1;
            size++;
        }

        if (!size[0] || size[0] < '1' || size[0] > '7')
        {
            g_warning ("%s: invalid size '%s'", G_STRLOC, size);
        }
        else
        {
            size_val = size[0] - '1' + 1;

            if (scale == 1)
            {
                attr.mask |= MOO_HTML_LARGER;
                attr.scale = size_val;
            }
            else if (scale == -1)
            {
                attr.mask |= MOO_HTML_SMALLER;
                attr.scale = size_val;
            }
            else
            {
                attr.mask |= MOO_HTML_FONT_SIZE;
                attr.font_size = size_val;
            }
        }
    }

    if (color)
    {
        attr.mask |= MOO_HTML_FG;
        attr.fg = (char*) color;
    }

    if (face && face[0])
    {
        attr.mask |= MOO_HTML_FONT_FACE;
        attr.font_face = (char*) face;
    }

    current = moo_html_create_tag (html, &attr, parent, FALSE);
    process_elm_body (html, buffer, elm, current, iter);

    STR_FREE (size__);
    STR_FREE (color);
    STR_FREE (face);
}


static void
process_cite_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                  MooHtmlTag *parent, GtkTextIter *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    current = moo_html_create_tag (html, &attr, parent, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
}


static void
process_format_elm (MooHtml        *html,
                    MooHtmlAttr    *attr,
                    GtkTextBuffer  *buffer,
                    xmlNode        *elm,
                    MooHtmlTag     *parent,
                    GtkTextIter    *iter)
{
    MooHtmlTag *current;
    current = moo_html_create_tag (html, attr, parent, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
}


static void
process_dt_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                MooHtmlTag *parent, GtkTextIter *iter)
{
    moo_html_new_line (html, buffer, iter, parent, FALSE);
    process_elm_body (html, buffer, elm, parent, iter);
    moo_html_new_line (html, buffer, iter, parent, FALSE);
}


static void
process_dl_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                MooHtmlTag *parent, GtkTextIter *iter)
{
    moo_html_new_line (html, buffer, iter, parent, FALSE);
    process_elm_body (html, buffer, elm, parent, iter);
    moo_html_new_line (html, buffer, iter, parent, FALSE);
}


static void
process_dd_elm (MooHtml         *html,
                GtkTextBuffer   *buffer,
                xmlNode         *elm,
                MooHtmlTag      *parent,
                GtkTextIter     *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    attr.mask = MOO_HTML_LEFT_MARGIN;
    attr.left_margin = 20;
    current = moo_html_create_tag (html, &attr, parent, FALSE);
    moo_html_new_line (html, buffer, iter, current, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
    moo_html_new_line (html, buffer, iter, current, FALSE);
}


static void
process_br_elm (MooHtml *html,
                GtkTextBuffer *buffer,
                G_GNUC_UNUSED xmlNode *elm,
                MooHtmlTag *parent,
                GtkTextIter *iter)
{
    moo_html_new_line (html, buffer, iter, parent, TRUE);
}


static void
process_div_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                 MooHtmlTag *parent, GtkTextIter *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    current = moo_html_create_tag (html, &attr, parent, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
}


static void
process_span_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                  MooHtmlTag *parent, GtkTextIter *iter)
{
    static MooHtmlAttr attr;
    MooHtmlTag *current;
    current = moo_html_create_tag (html, &attr, parent, FALSE);
    process_elm_body (html, buffer, elm, current, iter);
}


static void
process_hr_elm (MooHtml *html,
                GtkTextBuffer *buffer,
                G_GNUC_UNUSED xmlNode *elm,
                MooHtmlTag *parent,
                GtkTextIter *iter)
{
    GtkTextChildAnchor *anchor;
    GtkWidget *line;

    line = gtk_hseparator_new ();
    gtk_widget_show (line);

    moo_html_new_line (html, buffer, iter, parent, FALSE);

    anchor = gtk_text_buffer_create_child_anchor (buffer, iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (html), line, anchor);
    html->priv->rulers = g_slist_prepend (html->priv->rulers, line);

    moo_html_new_line (html, buffer, iter, parent, TRUE);
}


static void
process_img_elm (MooHtml        *html,
                 GtkTextBuffer  *buffer,
                 xmlNode        *elm,
                 MooHtmlTag     *current,
                 GtkTextIter    *iter)
{
    xmlChar *src;
    xmlChar *alt;
    char *path = NULL;
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    int offset;
    GtkTextIter before;

    src = GET_PROP (elm, "src");
    alt = GET_PROP (elm, "alt");

    g_return_if_fail (src != NULL);

    if (!html->priv->dirname)
        goto try_alt;

    path = g_build_filename (html->priv->dirname, src, NULL);
    g_return_if_fail (path != NULL);

    pixbuf = gdk_pixbuf_new_from_file (path, &error);

    if (!pixbuf)
    {
        g_message ("%s: could not load image '%s'",
                   G_STRLOC, path);
        g_message ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        goto try_alt;
    }

    offset = gtk_text_iter_get_offset (iter);
    gtk_text_buffer_insert_pixbuf (buffer, iter, pixbuf);
    gtk_text_buffer_get_iter_at_offset (buffer, &before, offset);

    if (current)
        gtk_text_buffer_apply_tag (buffer, GTK_TEXT_TAG (current), &before, iter);

    g_object_unref (pixbuf);
    goto out;

try_alt:
    if (alt)
    {
        char *text = g_strdup_printf ("[%s]", alt);
        moo_html_insert_text (html, buffer, iter, current, text);
        g_free (text);
    }

out:
    STR_FREE (src);
    STR_FREE (alt);
    g_free (path);
}


static void
process_table_elm (MooHtml *html,
                   GtkTextBuffer *buffer,
                   xmlNode *elm,
                   MooHtmlTag *parent,
                   GtkTextIter *iter)
{
    process_elm_body (html, buffer, elm, parent, iter);
}

static void
process_tr_elm (MooHtml *html, GtkTextBuffer *buffer, xmlNode *elm,
                MooHtmlTag *parent, GtkTextIter *iter)
{
    moo_html_new_line (html, buffer, iter, parent, FALSE);
    process_elm_body (html, buffer, elm, parent, iter);
    moo_html_new_line (html, buffer, iter, parent, FALSE);
}
