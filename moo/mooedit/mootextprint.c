/*
 *   mootextprint.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mootextprint.h"
#include "mooedit/mooedit.h"
#include "mooedit/mooedit-private.h"
#include "mooedit/mooprint-glade.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/mooglade.h"
#include "mooutils/moodialogs.h"
#include <time.h>
#include <errno.h>
#include <string.h>


#define PREFS_FONT                      MOO_EDIT_PREFS_PREFIX "/print/font"
#define PREFS_USE_STYLES                MOO_EDIT_PREFS_PREFIX "/print/use_styles"
#define PREFS_USE_CUSTOM_FONT           MOO_EDIT_PREFS_PREFIX "/print/use_font"
#define PREFS_WRAP                      MOO_EDIT_PREFS_PREFIX "/print/wrap"
#define PREFS_ELLIPSIZE                 MOO_EDIT_PREFS_PREFIX "/print/ellipsize"

#define PREFS_PRINT_HEADER              MOO_EDIT_PREFS_PREFIX "/print/header/print"
#define PREFS_PRINT_HEADER_SEPARATOR    MOO_EDIT_PREFS_PREFIX "/print/header/separator"
#define PREFS_HEADER_LEFT               MOO_EDIT_PREFS_PREFIX "/print/header/left"
#define PREFS_HEADER_CENTER             MOO_EDIT_PREFS_PREFIX "/print/header/center"
#define PREFS_HEADER_RIGHT              MOO_EDIT_PREFS_PREFIX "/print/header/right"
#define PREFS_PRINT_FOOTER              MOO_EDIT_PREFS_PREFIX "/print/footer/print"
#define PREFS_PRINT_FOOTER_SEPARATOR    MOO_EDIT_PREFS_PREFIX "/print/footer/separator"
#define PREFS_FOOTER_LEFT               MOO_EDIT_PREFS_PREFIX "/print/footer/left"
#define PREFS_FOOTER_CENTER             MOO_EDIT_PREFS_PREFIX "/print/footer/center"
#define PREFS_FOOTER_RIGHT              MOO_EDIT_PREFS_PREFIX "/print/footer/right"


#define SET_OPTION(print, opt, val)     \
G_STMT_START {                          \
    if (val)                            \
        (print)->options |= (opt);      \
    else                                \
        (print)->options &= (~(opt));   \
} G_STMT_END

#define GET_OPTION(print, opt)      ((print->options & opt) != 0)


typedef struct _HFFormat HFFormat;
static HFFormat *hf_format_parse    (const char *strformat);
static void      hf_format_free     (HFFormat   *format);
static char     *hf_format_eval     (HFFormat   *format,
                                     struct tm  *tm,
                                     int         page,
                                     int         total_pages,
                                     const char *filename,
                                     const char *basename);


static GtkPageSetup *page_setup;
static GtkPrintSettings *print_settings;

static void load_default_settings   (void);
static void moo_print_init_prefs    (void);

static void moo_print_operation_finalize    (GObject            *object);
static void moo_print_operation_set_property(GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void moo_print_operation_get_property(GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void moo_print_operation_begin_print (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context);
static void moo_print_operation_draw_page   (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context,
                                             int                 page);
static void moo_print_operation_end_print   (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context);
static void moo_print_operation_status_changed (GtkPrintOperation *operation);
static GtkWidget *moo_print_operation_create_custom_widget (GtkPrintOperation *operation);
static void moo_print_operation_custom_widget_apply (GtkPrintOperation *operation,
                                                     GtkWidget *widget);
static void moo_print_operation_load_prefs  (MooPrintOperation  *print);
static void update_progress                 (GtkPrintOperation  *operation,
                                             int                 page);


G_DEFINE_TYPE(MooPrintOperation, _moo_print_operation, GTK_TYPE_PRINT_OPERATION)

enum {
    PROP_0,
    PROP_DOC,
    PROP_BUFFER,
    PROP_WRAP,
    PROP_WRAP_MODE,
    PROP_ELLIPSIZE,
    PROP_FONT,
    PROP_USE_STYLES
};


static void
header_footer_destroy (MooPrintHeaderFooter *hf)
{
    int i;

    if (hf->font)
        g_object_unref (hf->font);
    if (hf->layout)
        g_object_unref (hf->layout);

    for (i = 0; i < 3; ++i)
    {
        g_free (hf->format[i]);
        hf_format_free (hf->parsed_format[i]);
    }
}


static void
moo_print_operation_finalize (GObject *object)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);
    header_footer_destroy (&print->header);
    header_footer_destroy (&print->footer);
    g_free (print->tm);
    g_free (print->filename);
    g_free (print->basename);

    G_OBJECT_CLASS(_moo_print_operation_parent_class)->finalize (object);
}


static void
moo_print_operation_set_property (GObject            *object,
                                  guint               prop_id,
                                  const GValue       *value,
                                  GParamSpec         *pspec)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            _moo_print_operation_set_doc (print, g_value_get_object (value));
            break;

        case PROP_BUFFER:
            _moo_print_operation_set_buffer (print, g_value_get_object (value));
            break;

        case PROP_WRAP:
            SET_OPTION (print, MOO_PRINT_WRAP, g_value_get_boolean (value));
            g_object_notify (object, "wrap");
            break;

        case PROP_WRAP_MODE:
            print->wrap_mode = g_value_get_enum (value);
            g_object_notify (object, "wrap-mode");
            break;

        case PROP_ELLIPSIZE:
            SET_OPTION (print, MOO_PRINT_ELLIPSIZE, g_value_get_boolean (value));
            g_object_notify (object, "ellipsize");
            break;

        case PROP_FONT:
            g_free (print->font);
            print->font = g_value_dup_string (value);
            g_object_notify (object, "font");
            break;

        case PROP_USE_STYLES:
            SET_OPTION (print, MOO_PRINT_USE_STYLES, g_value_get_boolean (value));
            g_object_notify (object, "use-styles");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
moo_print_operation_get_property (GObject            *object,
                                  guint               prop_id,
                                  GValue             *value,
                                  GParamSpec         *pspec)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    switch (prop_id)
    {
        case PROP_DOC:
            g_value_set_object (value, print->doc);
            break;

        case PROP_BUFFER:
            g_value_set_object (value, print->buffer);
            break;

        case PROP_WRAP:
            g_value_set_boolean (value, GET_OPTION (print, MOO_PRINT_WRAP));
            break;

        case PROP_WRAP_MODE:
            g_value_set_enum (value, print->wrap_mode);
            break;

        case PROP_ELLIPSIZE:
            g_value_set_boolean (value, GET_OPTION (print, MOO_PRINT_ELLIPSIZE));
            break;

        case PROP_FONT:
            g_value_set_string (value, print->font);
            break;

        case PROP_USE_STYLES:
            g_value_set_boolean (value, GET_OPTION (print, MOO_PRINT_USE_STYLES));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
_moo_print_operation_class_init (MooPrintOperationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkPrintOperationClass *print_class = GTK_PRINT_OPERATION_CLASS (klass);

    object_class->finalize = moo_print_operation_finalize;
    object_class->set_property = moo_print_operation_set_property;
    object_class->get_property = moo_print_operation_get_property;

    print_class->begin_print = moo_print_operation_begin_print;
    print_class->draw_page = moo_print_operation_draw_page;
    print_class->end_print = moo_print_operation_end_print;
    print_class->create_custom_widget = moo_print_operation_create_custom_widget;
    print_class->custom_widget_apply = moo_print_operation_custom_widget_apply;
    print_class->status_changed = moo_print_operation_status_changed;

    g_object_class_install_property (object_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             GTK_TYPE_TEXT_VIEW,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("buffer",
                                             "buffer",
                                             "buffer",
                                             GTK_TYPE_TEXT_BUFFER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_FONT,
                                     g_param_spec_string ("font",
                                             "font",
                                             "font",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_WRAP,
                                     g_param_spec_boolean ("wrap",
                                             "wrap",
                                             "wrap",
                                             TRUE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_WRAP_MODE,
                                     g_param_spec_enum ("wrap-mode",
                                             "wrap-mode",
                                             "wrap-mode",
                                             PANGO_TYPE_WRAP_MODE,
                                             PANGO_WRAP_WORD_CHAR,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_ELLIPSIZE,
                                     g_param_spec_boolean ("ellipsize",
                                             "ellipsize",
                                             "ellipsize",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (object_class,
                                     PROP_USE_STYLES,
                                     g_param_spec_boolean ("use-styles",
                                             "use-styles",
                                             "use-styles",
                                             FALSE,
                                             G_PARAM_READWRITE));
}


static void
_moo_print_operation_init (MooPrintOperation *print)
{
    load_default_settings ();

    gtk_print_operation_set_print_settings (GTK_PRINT_OPERATION (print),
                                            print_settings);

    if (page_setup)
        gtk_print_operation_set_default_page_setup (GTK_PRINT_OPERATION (print),
                                                    page_setup);

    print->last_line = -1;
    print->options = MOO_PRINT_HEADER | MOO_PRINT_FOOTER;
    print->wrap_mode = PANGO_WRAP_WORD_CHAR;
    print->header.separator = TRUE;
    print->footer.separator = TRUE;
}


void
_moo_print_operation_set_doc (MooPrintOperation  *print,
                              GtkTextView        *doc)
{
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (print->doc == doc)
        return;

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);

    print->doc = doc;

    if (print->doc)
    {
        g_object_ref (print->doc);
        print->buffer = gtk_text_view_get_buffer (print->doc);
        g_object_ref (print->buffer);
    }
    else
    {
        print->buffer = NULL;
    }

    g_object_freeze_notify (G_OBJECT (print));
    g_object_notify (G_OBJECT (print), "doc");
    g_object_notify (G_OBJECT (print), "buffer");
    g_object_thaw_notify (G_OBJECT (print));
}


void
_moo_print_operation_set_buffer (MooPrintOperation *print,
                                 GtkTextBuffer     *buffer)
{
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    g_return_if_fail (!buffer || GTK_IS_TEXT_BUFFER (buffer));

    if (print->buffer == buffer)
        return;

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);

    print->doc = NULL;
    print->buffer = buffer;

    if (print->buffer)
        g_object_ref (print->buffer);

    g_object_notify (G_OBJECT (print), "buffer");
}


static void
load_default_settings (void)
{
    if (!print_settings)
        print_settings = gtk_print_settings_new ();
}


void
_moo_edit_page_setup (GtkTextView    *view,
                      GtkWidget      *parent)
{
    GtkPageSetup *new_page_setup;
    GtkWindow *parent_window = NULL;

    g_return_if_fail (!view || GTK_IS_TEXT_VIEW (view));

    load_default_settings ();

    if (!parent && view)
        parent = GTK_WIDGET (view);

    if (parent)
        parent = gtk_widget_get_toplevel (parent);

    if (parent && GTK_IS_WINDOW (parent))
        parent_window = GTK_WINDOW (parent);

    new_page_setup = gtk_print_run_page_setup_dialog (parent_window,
                                                      page_setup,
                                                      print_settings);

    if (page_setup)
        g_object_unref (page_setup);

    page_setup = new_page_setup;
}


static void
header_footer_get_size (MooPrintHeaderFooter *hf,
                        MooPrintOperation    *print,
                        GtkPrintContext      *context,
                        PangoFontDescription *default_font)
{
    PangoFontDescription *font = default_font;
    PangoRectangle rect;

    hf->do_print = GET_OPTION (print, MOO_PRINT_HEADER) &&
            (hf->parsed_format[0] || hf->parsed_format[1] || hf->parsed_format[2]);

    if (hf->layout)
        g_object_unref (hf->layout);
    hf->layout = NULL;

    if (!hf->do_print)
        return;

    if (hf->font)
        font = hf->font;

    hf->layout = gtk_print_context_create_pango_layout (context);
    pango_layout_set_text (hf->layout, "AAAyyy", -1);

    if (font)
        pango_layout_set_font_description (hf->layout, font);

    pango_layout_get_pixel_extents (hf->layout, NULL, &rect);
    hf->text_height = rect.height;

    if (hf->separator)
    {
        hf->separator_height = 1.;
        hf->separator_before = hf->text_height / 2;
        hf->separator_after = hf->text_height / 2;
    }
    else
    {
        hf->separator_height = .0;
        hf->separator_before = 0.;
        hf->separator_after = 0.;
    }
}


static void
moo_print_operation_calc_page_size (MooPrintOperation  *print,
                                    GtkPrintContext    *context,
                                    PangoFontDescription *default_font)
{
    print->page.x = 0.;
    print->page.y = 0.;
    print->page.width = gtk_print_context_get_width (context);
    print->page.height = gtk_print_context_get_height (context);

    header_footer_get_size (&print->header, print, context, default_font);
    header_footer_get_size (&print->footer, print, context, default_font);

    if (print->header.do_print)
    {
        double delta = print->header.text_height + print->header.separator_before +
                    print->header.separator_after + print->header.separator_height;
        print->page.y += delta;
        print->page.height -= delta;
    }

    if (print->footer.do_print)
    {
        double delta = print->footer.text_height + print->footer.separator_before +
                    print->footer.separator_after + print->footer.separator_height;
        print->page.height -= delta;
    }

    if (print->page.height < 0)
    {
        g_critical ("%s: page messed up", G_STRLOC);
        print->page.y = 0.;
        print->page.height = gtk_print_context_get_height (context);
    }
}


#ifdef __WIN32__
static struct tm *
localtime_r (const time_t *timep,
             struct tm *result)
{
    struct tm *res;
    res = localtime (timep);
    if (res)
        *result = *res;
    return res;
}
#endif

static void
moo_print_operation_begin_print (GtkPrintOperation  *operation,
                                 GtkPrintContext    *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    PangoFontDescription *font = NULL;
    double page_height;
    GtkTextIter iter, print_end;
    GTimer *timer;
    time_t t;

    g_return_if_fail (print->buffer != NULL);
    g_return_if_fail (print->first_line >= 0);
    g_return_if_fail (print->last_line < 0 || print->last_line >= print->first_line);
    g_return_if_fail (print->first_line < gtk_text_buffer_get_line_count (print->buffer));
    g_return_if_fail (print->last_line < gtk_text_buffer_get_line_count (print->buffer));

    moo_print_operation_load_prefs (MOO_PRINT_OPERATION (operation));

    timer = g_timer_new ();

    if (MOO_IS_EDIT (print->doc))
        _moo_edit_set_state (MOO_EDIT (print->doc), MOO_EDIT_STATE_PRINTING, "Printing");

    if (print->last_line < 0)
        print->last_line = gtk_text_buffer_get_line_count (print->buffer) - 1;

    if (print->font)
        font = pango_font_description_from_string (print->font);

    if (!font && print->doc)
    {
        PangoContext *widget_ctx;

        widget_ctx = gtk_widget_get_pango_context (GTK_WIDGET (print->doc));

        if (widget_ctx)
        {
            font = pango_context_get_font_description (widget_ctx);

            if (font)
                font = pango_font_description_copy_static (font);
        }
    }

    moo_print_operation_calc_page_size (print, context, font);
    print->layout = gtk_print_context_create_pango_layout (context);

    if (GET_OPTION (print, MOO_PRINT_WRAP))
    {
        pango_layout_set_width (print->layout, print->page.width * PANGO_SCALE);
        pango_layout_set_wrap (print->layout, print->wrap_mode);
    }
    else if (GET_OPTION (print, MOO_PRINT_ELLIPSIZE))
    {
        pango_layout_set_width (print->layout, print->page.width * PANGO_SCALE);
        pango_layout_set_ellipsize (print->layout, PANGO_ELLIPSIZE_END);
    }

    if (font)
    {
        pango_layout_set_font_description (print->layout, font);
        pango_font_description_free (font);
    }

    print->pages = g_array_new (FALSE, FALSE, sizeof (GtkTextIter));
    gtk_text_buffer_get_iter_at_line (print->buffer, &iter,
                                      print->first_line);
    gtk_text_buffer_get_iter_at_line (print->buffer, &print_end,
                                      print->last_line);
    gtk_text_iter_forward_line (&print_end);
    g_array_append_val (print->pages, iter);
    page_height = 0;

    while (gtk_text_iter_compare (&iter, &print_end) < 0)
    {
        GtkTextIter end;
        PangoRectangle line_rect;
        double line_height;
        char *text;

        end = iter;

        if (!gtk_text_iter_ends_line (&end))
            gtk_text_iter_forward_to_line_end (&end);

        text = gtk_text_buffer_get_slice (print->buffer, &iter, &end, TRUE);
        pango_layout_set_text (print->layout, text, -1);
        g_free (text);

        pango_layout_get_pixel_extents (print->layout, NULL, &line_rect);
        line_height = line_rect.height;

        gtk_text_iter_forward_line (&iter);

        if (page_height + line_height > print->page.height + .1)
        {
            gboolean part = FALSE;
            PangoLayoutIter *layout_iter;

            layout_iter = pango_layout_get_iter (print->layout);

            if (GET_OPTION (print, MOO_PRINT_WRAP) && pango_layout_get_line_count (print->layout) > 1)
            {
                double part_height = 0;

                do
                {
                    PangoLayoutLine *layout_line;

                    layout_line = pango_layout_iter_get_line (layout_iter);
                    pango_layout_line_get_pixel_extents (layout_line, NULL, &line_rect);

                    if (page_height + part_height + line_rect.height > print->page.height + .1)
                        break;

                    part_height += line_rect.height;
                    part = TRUE;
                }
                while (pango_layout_iter_next_line (layout_iter));
            }

            if (part)
            {
                int index = pango_layout_iter_get_index (layout_iter);
                iter = end;
                gtk_text_iter_set_line_index (&iter, index);
                line_height = 0;
            }

            pango_layout_iter_free (layout_iter);

            g_array_append_val (print->pages, iter);
            page_height = 0;
        }
        else
        {
            page_height += line_height;
        }
    }

    gtk_print_operation_set_n_pages (operation, print->pages->len);

//     g_message ("begin_print: %d pages in %f s", print->pages->len,
//                g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);

    if (!print->tm)
        print->tm = g_new (struct tm, 1);

    errno = 0;
    time (&t);

    if (errno)
    {
        int err = errno;
        g_critical ("time: %s", g_strerror (err));
        g_free (print->tm);
        print->tm = NULL;
    }
    else if (!localtime_r (&t, print->tm))
    {
        int err = errno;
        g_critical ("time: %s", g_strerror (err));
        g_free (print->tm);
        print->tm = NULL;
    }
}


static GSList *
iter_get_attrs (GtkTextIter       *iter,
                const GtkTextIter *limit)
{
    GSList *attrs = NULL, *tags;
    PangoAttribute *bg = NULL, *fg = NULL, *style = NULL, *ul = NULL;
    PangoAttribute *weight = NULL, *st = NULL;

    tags = gtk_text_iter_get_tags (iter);
    gtk_text_iter_forward_to_tag_toggle (iter, NULL);

    if (gtk_text_iter_compare (iter, limit) > 0)
        *iter = *limit;

    while (tags)
    {
        GtkTextTag *tag = tags->data;
        gboolean bg_set, fg_set, style_set, ul_set, weight_set, st_set;

        g_object_get (tag,
                      "background-set", &bg_set,
                      "foreground-set", &fg_set,
                      "style-set", &style_set,
                      "underline-set", &ul_set,
                      "weight-set", &weight_set,
                      "strikethrough-set", &st_set,
                      NULL);

        if (bg_set)
        {
            GdkColor *color = NULL;
            if (bg) pango_attribute_destroy (bg);
            g_object_get (tag, "background-gdk", &color, NULL);
            bg = pango_attr_background_new (color->red, color->green, color->blue);
            gdk_color_free (color);
        }

        if (fg_set)
        {
            GdkColor *color = NULL;
            if (fg) pango_attribute_destroy (fg);
            g_object_get (tag, "foreground-gdk", &color, NULL);
            fg = pango_attr_foreground_new (color->red, color->green, color->blue);
            gdk_color_free (color);
        }

        if (style_set)
        {
            PangoStyle style_value;
            if (style) pango_attribute_destroy (style);
            g_object_get (tag, "style", &style_value, NULL);
            style = pango_attr_style_new (style_value);
        }

        if (ul_set)
        {
            PangoUnderline underline;
            if (ul) pango_attribute_destroy (ul);
            g_object_get (tag, "underline", &underline, NULL);
            ul = pango_attr_underline_new (underline);
        }

        if (weight_set)
        {
            PangoWeight weight_value;
            if (weight) pango_attribute_destroy (weight);
            g_object_get (tag, "weight", &weight_value, NULL);
            weight = pango_attr_weight_new (weight_value);
        }

        if (st_set)
        {
            gboolean strikethrough;
            if (st) pango_attribute_destroy (st);
            g_object_get (tag, "strikethrough", &strikethrough, NULL);
            st = pango_attr_strikethrough_new (strikethrough);
        }

        tags = g_slist_delete_link (tags, tags);
    }

    if (bg)
        attrs = g_slist_prepend (attrs, bg);
    if (fg)
        attrs = g_slist_prepend (attrs, fg);
    if (style)
        attrs = g_slist_prepend (attrs, style);
    if (ul)
        attrs = g_slist_prepend (attrs, ul);
    if (weight)
        attrs = g_slist_prepend (attrs, weight);
    if (st)
        attrs = g_slist_prepend (attrs, st);

    return attrs;
}


static void
fill_layout (PangoLayout       *layout,
             const GtkTextIter *start,
             const GtkTextIter *end)
{
    char *text;
    PangoAttrList *attr_list;
    GtkTextIter segm_start, segm_end;
    int start_index;

    text = gtk_text_iter_get_text (start, end);
    pango_layout_set_text (layout, text, -1);

    attr_list = NULL;
    segm_start = *start;
    start_index = gtk_text_iter_get_line_index (start);

    while (gtk_text_iter_compare (&segm_start, end) < 0)
    {
        GSList *attrs;

        segm_end = segm_start;
        attrs = iter_get_attrs (&segm_end, end);

        if (attrs)
        {
            int si, ei;

            si = gtk_text_iter_get_line_index (&segm_start) - start_index;
            ei = gtk_text_iter_get_line_index (&segm_end) - start_index;

            while (attrs)
            {
                PangoAttribute *a = attrs->data;

                a->start_index = si;
                a->end_index = ei;

                if (!attr_list)
                    attr_list = pango_attr_list_new ();

                pango_attr_list_insert (attr_list, a);

                attrs = g_slist_delete_link (attrs, attrs);
            }
        }

        segm_start = segm_end;
    }

    pango_layout_set_attributes (layout, attr_list);

    if (attr_list)
        pango_attr_list_unref (attr_list);
}


static void
print_header_footer (MooPrintOperation    *print,
                     cairo_t              *cr,
                     MooPrintHeaderFooter *hf,
                     int                   page,
                     gboolean              header)
{
    double y;
    char *text;
    PangoRectangle rect;
    int total_pages;

    if (header)
        y = 0;
    else
        y = print->page.y + print->page.height + hf->separator_before +
                hf->separator_after + hf->separator_height;

    g_object_get (print, "n-pages", &total_pages, NULL);

    if (hf->parsed_format[0] &&
        (text = hf_format_eval (hf->parsed_format[0], print->tm, page,
                                total_pages, print->filename, print->basename)))
    {
        pango_layout_set_text (hf->layout, text, -1);
        cairo_move_to (cr, print->page.x, y);
        pango_cairo_show_layout (cr, hf->layout);
        g_free (text);
    }

    if (hf->parsed_format[1] &&
        (text = hf_format_eval (hf->parsed_format[1], print->tm, page,
                                total_pages, print->filename, print->basename)))
    {
        pango_layout_set_text (hf->layout, text, -1);
        pango_layout_get_pixel_extents (hf->layout, NULL, &rect);
        cairo_move_to (cr, print->page.x + print->page.width/2 - rect.width/2, y);
        pango_cairo_show_layout (cr, hf->layout);
        g_free (text);
    }

    if (hf->parsed_format[2] &&
        (text = hf_format_eval (hf->parsed_format[2], print->tm, page,
                                total_pages, print->filename, print->basename)))
    {
        pango_layout_set_text (hf->layout, text, -1);
        pango_layout_get_pixel_extents (hf->layout, NULL, &rect);
        cairo_move_to (cr, print->page.x + print->page.width - rect.width, y);
        pango_cairo_show_layout (cr, hf->layout);
        g_free (text);
    }

    if (hf->separator)
    {
        if (header)
            y = hf->text_height + hf->separator_before + hf->separator_height/2;
        else
            y = print->page.y + print->page.height +
                    hf->separator_after + hf->separator_height/2;

        cairo_move_to (cr, print->page.x, y);
        cairo_set_line_width (cr, hf->separator_height);
        cairo_line_to (cr, print->page.x + print->page.width, y);
        cairo_stroke (cr);
    }
}


static void
print_page (MooPrintOperation *print,
            const GtkTextIter *start,
            const GtkTextIter *end,
            int                page,
            cairo_t           *cr)
{
    char *text;
    GtkTextIter line_start, line_end;
    double offset;

    cairo_set_source_rgb (cr, 0, 0, 0);

    if (print->header.do_print)
        print_header_footer (print, cr, &print->header, page, TRUE);
    if (print->footer.do_print)
        print_header_footer (print, cr, &print->footer, page, FALSE);

    if (!GET_OPTION (print, MOO_PRINT_USE_STYLES))
    {
        text = gtk_text_buffer_get_slice (print->buffer, start, end, TRUE);
        pango_layout_set_text (print->layout, text, -1);
        g_free (text);

        cairo_move_to (cr, print->page.x, print->page.y);
        pango_cairo_show_layout (cr, print->layout);

        return;
    }

    line_start = *start;
    offset = print->page.y;

    while (gtk_text_iter_compare (&line_start, end) < 0)
    {
        PangoRectangle line_rect;

        if (gtk_text_iter_ends_line (&line_start))
        {
            pango_layout_set_text (print->layout, "", 0);
            pango_layout_set_attributes (print->layout, NULL);
        }
        else
        {
            line_end = line_start;
            gtk_text_iter_forward_to_line_end (&line_end);

            if (gtk_text_iter_compare (&line_end, end) > 0)
                line_end = *end;

            fill_layout (print->layout,&line_start, &line_end);
        }

        cairo_move_to (cr, print->page.x, offset);
        pango_cairo_show_layout (cr, print->layout);

        pango_layout_get_pixel_extents (print->layout, NULL, &line_rect);
        offset += line_rect.height;
        gtk_text_iter_forward_line (&line_start);
    }
}


static void
moo_print_operation_draw_page (GtkPrintOperation  *operation,
                               GtkPrintContext    *context,
                               int                 page)
{
    cairo_t *cr;
    GtkTextIter start, end;
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    GTimer *timer;

    g_return_if_fail (print->buffer != NULL);
    g_return_if_fail (print->pages != NULL);
    g_return_if_fail (print->layout != NULL);
    g_return_if_fail (page < (int) print->pages->len);

    timer = g_timer_new ();
    update_progress (operation, page);

    cr = gtk_print_context_get_cairo_context (context);

    start = g_array_index (print->pages, GtkTextIter, page);

    if (page + 1 < (int) print->pages->len)
        end = g_array_index (print->pages, GtkTextIter, page + 1);
    else
        gtk_text_buffer_get_end_iter (print->buffer, &end);

    print_page (print, &start, &end, page, cr);

//     g_message ("page %d: %f s", page, g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
}


static void
moo_print_operation_end_print (GtkPrintOperation  *operation,
                               G_GNUC_UNUSED GtkPrintContext *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);

    g_return_if_fail (print->buffer != NULL);

    g_array_free (print->pages, TRUE);

    if (MOO_IS_EDIT (print->doc))
        _moo_edit_set_state (MOO_EDIT (print->doc), MOO_EDIT_STATE_NORMAL, NULL);

    print->layout = NULL;
    print->pages = NULL;
}


static void
update_progress (GtkPrintOperation *op,
                 int                page)
{
    char *text = NULL;
    MooPrintOperation *print = MOO_PRINT_OPERATION (op);
    GtkPrintStatus status;
    MooEditWindow *window;
    MooEdit *doc;

    if (!MOO_IS_EDIT (print->doc))
        return;

    doc = MOO_EDIT (print->doc);
    window = moo_edit_get_window (doc);
    status = gtk_print_operation_get_status (op);

    if (status == GTK_PRINT_STATUS_FINISHED)
    {
        text = g_strdup ("Finished printing");
    }
    else if (status == GTK_PRINT_STATUS_GENERATING_DATA && page >= 0)
    {
        int n_pages;
        g_object_get (print, "n-pages", &n_pages, NULL);
        text = g_strdup_printf ("Printing page %d of %d", page, n_pages);
    }
    else
    {
        text = g_strdup (gtk_print_operation_get_status_string (op));
    }

    if (window)
        moo_edit_window_message (window, NULL);

    if (moo_edit_get_state (doc) == MOO_EDIT_STATE_PRINTING)
        _moo_edit_set_progress_text (doc, text);
    else if (window)
        moo_edit_window_message (window, text);

    g_free (text);
}


static void
moo_print_operation_status_changed (GtkPrintOperation *op)
{
    update_progress (op, -1);
}


static void
moo_print_operation_load_prefs (MooPrintOperation *print)
{
    char *formats[3];

    moo_print_init_prefs ();

    formats[0] = g_strdup (moo_prefs_get_string (PREFS_HEADER_LEFT));
    formats[1] = g_strdup (moo_prefs_get_string (PREFS_HEADER_CENTER));
    formats[2] = g_strdup (moo_prefs_get_string (PREFS_HEADER_RIGHT));

    _moo_print_operation_set_header_format (MOO_PRINT_OPERATION (print),
                                            formats[0], formats[1], formats[2]);

    g_free (formats[0]);
    g_free (formats[1]);
    g_free (formats[2]);

    formats[0] = g_strdup (moo_prefs_get_string (PREFS_FOOTER_LEFT));
    formats[1] = g_strdup (moo_prefs_get_string (PREFS_FOOTER_CENTER));
    formats[2] = g_strdup (moo_prefs_get_string (PREFS_FOOTER_RIGHT));

    _moo_print_operation_set_footer_format (MOO_PRINT_OPERATION (print),
                                            formats[0], formats[1], formats[2]);

    g_free (formats[0]);
    g_free (formats[1]);
    g_free (formats[2]);

    if (moo_prefs_get_bool (PREFS_USE_CUSTOM_FONT))
    {
        const char *font = moo_prefs_get_string (PREFS_FONT);
        g_object_set (print, "font", font, NULL);
    }

    g_object_set (print, "use-styles", moo_prefs_get_bool (PREFS_USE_STYLES),
                  "wrap", moo_prefs_get_bool (PREFS_WRAP),
                  "ellipsize", moo_prefs_get_bool (PREFS_ELLIPSIZE),
                  NULL);
}


void
_moo_edit_print (GtkTextView *view,
                 GtkWidget   *parent)
{
    GtkWindow *parent_window = NULL;
    GtkWidget *error_dialog;
    GtkPrintOperation *print;
    GtkPrintOperationResult res;
    GError *error = NULL;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));

    print = g_object_new (MOO_TYPE_PRINT_OPERATION,
                          "doc", view, NULL);

    if (MOO_IS_EDIT (view))
        _moo_print_operation_set_filename (MOO_PRINT_OPERATION (print),
                                           moo_edit_get_display_filename (MOO_EDIT (view)),
                                           moo_edit_get_display_basename (MOO_EDIT (view)));

    if (!parent)
        parent = GTK_WIDGET (view);

    parent = gtk_widget_get_toplevel (parent);

    if (GTK_IS_WINDOW (parent))
        parent_window = GTK_WINDOW (parent);

    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                   parent_window, &error);

    switch (res)
    {
        case GTK_PRINT_OPERATION_RESULT_ERROR:
            error_dialog = gtk_message_dialog_new (parent_window,
                                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "Error printing file:\n%s",
                                                   error ? error->message : "");
            gtk_dialog_run (GTK_DIALOG (error_dialog));
            gtk_widget_destroy (error_dialog);
            g_error_free (error);
            break;

        case GTK_PRINT_OPERATION_RESULT_APPLY:
            if (print_settings)
                g_object_unref (print_settings);
            print_settings = gtk_print_operation_get_print_settings (print);
            g_object_ref (print_settings);
            break;

        case GTK_PRINT_OPERATION_RESULT_CANCEL:
        case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:
            break;
    }

    g_object_unref (print);
}


void
_moo_print_operation_set_filename (MooPrintOperation  *print,
                                   const char         *filename,
                                   const char         *basename)
{
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    g_free (print->filename);
    g_free (print->basename);
    print->filename = g_strdup (filename);
    print->basename = g_strdup (basename);
}


static void
set_hf_format (MooPrintHeaderFooter *hf,
               const char          **formats)
{
    int i;
    HFFormat *parsed[3] = {NULL, NULL, NULL};

    for (i = 0; i < 3; ++i)
    {
        if (formats[i])
            parsed[i] = hf_format_parse (formats[i]);

        if (formats[i] && !parsed[i])
        {
            for (i = 0; i < 3; ++i)
                hf_format_free (parsed[i]);
            return;
        }
    }

    for (i = 0; i < 3; ++i)
    {
        g_free (hf->format[i]);
        hf_format_free (hf->parsed_format[i]);
        hf->format[i] = g_strdup (formats[i]);
        hf->parsed_format[i] = parsed[i];
    }
}


void
_moo_print_operation_set_header_format (MooPrintOperation  *print,
                                        const char         *left,
                                        const char         *center,
                                        const char         *right)
{
    const char *formats[] = {left, center, right};
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    set_hf_format (&print->header, formats);
}


void
_moo_print_operation_set_footer_format (MooPrintOperation  *print,
                                        const char         *left,
                                        const char         *center,
                                        const char         *right)
{
    const char *formats[] = {left, center, right};
    g_return_if_fail (MOO_IS_PRINT_OPERATION (print));
    set_hf_format (&print->footer, formats);
}


/*****************************************************************************/
/* Header/footer gui
 */

#define SET_TEXT(wid,setting)                                           \
G_STMT_START {                                                          \
    GtkEntry *entry__ = moo_glade_xml_get_widget (xml, wid);            \
    const char *s__ = moo_prefs_get_string (setting);                   \
    gtk_entry_set_text (entry__, s__ ? s__ : "");                       \
} G_STMT_END

#define GET_TEXT(wid,setting)                                           \
G_STMT_START {                                                          \
    GtkEntry *entry__ = moo_glade_xml_get_widget (xml, wid);            \
    const char *s__ = gtk_entry_get_text (entry__);                     \
    moo_prefs_set_string (setting, s__ && s__[0] ? s__ : NULL);         \
} G_STMT_END

#define SET_BOOL(wid,setting)                                           \
G_STMT_START {                                                          \
    gtk_toggle_button_set_active (moo_glade_xml_get_widget (xml, wid),  \
                                  moo_prefs_get_bool (setting));        \
} G_STMT_END

#define GET_BOOL(wid,setting)                                           \
G_STMT_START {                                                          \
    gpointer btn__ = moo_glade_xml_get_widget (xml, wid);               \
    moo_prefs_set_bool (setting, gtk_toggle_button_get_active (btn__)); \
} G_STMT_END


static void
moo_print_init_prefs (void)
{
    moo_prefs_new_key_bool (PREFS_PRINT_HEADER, TRUE);
    moo_prefs_new_key_bool (PREFS_PRINT_FOOTER, TRUE);
    moo_prefs_new_key_bool (PREFS_PRINT_HEADER_SEPARATOR, TRUE);
    moo_prefs_new_key_bool (PREFS_PRINT_FOOTER_SEPARATOR, TRUE);
    moo_prefs_new_key_string (PREFS_HEADER_LEFT, "%Ef");
    moo_prefs_new_key_string (PREFS_HEADER_CENTER, NULL);
    moo_prefs_new_key_string (PREFS_HEADER_RIGHT, NULL);
    moo_prefs_new_key_string (PREFS_FOOTER_LEFT, NULL);
    moo_prefs_new_key_string (PREFS_FOOTER_CENTER, "Page %Ep of %EP");
    moo_prefs_new_key_string (PREFS_FOOTER_RIGHT, NULL);

    moo_prefs_new_key_string (PREFS_FONT, NULL);
    moo_prefs_new_key_bool (PREFS_USE_STYLES, FALSE);
    moo_prefs_new_key_bool (PREFS_USE_CUSTOM_FONT, FALSE);
    moo_prefs_new_key_bool (PREFS_WRAP, TRUE);
    moo_prefs_new_key_bool (PREFS_ELLIPSIZE, FALSE);
}


static void
set_options (MooGladeXML *xml)
{
    const char *s;

    moo_print_init_prefs ();

    SET_BOOL ("print_header", PREFS_PRINT_HEADER);
    SET_BOOL ("header_separator", PREFS_PRINT_HEADER_SEPARATOR);

    SET_TEXT ("header_left", PREFS_HEADER_LEFT);
    SET_TEXT ("header_center", PREFS_HEADER_CENTER);
    SET_TEXT ("header_right", PREFS_HEADER_RIGHT);

    SET_BOOL ("print_footer", PREFS_PRINT_FOOTER);
    SET_BOOL ("footer_separator", PREFS_PRINT_FOOTER_SEPARATOR);

    SET_TEXT ("footer_left", PREFS_FOOTER_LEFT);
    SET_TEXT ("footer_center", PREFS_FOOTER_CENTER);
    SET_TEXT ("footer_right", PREFS_FOOTER_RIGHT);

    SET_BOOL ("use_styles", PREFS_USE_STYLES);
    SET_BOOL ("use_custom_font", PREFS_USE_CUSTOM_FONT);
    SET_BOOL ("wrap", PREFS_WRAP);
    SET_BOOL ("ellipsize", PREFS_ELLIPSIZE);

    if ((s = moo_prefs_get_string (PREFS_FONT)))
        gtk_font_button_set_font_name (moo_glade_xml_get_widget (xml, "font"), s);
}


static void
get_options (MooGladeXML *xml)
{
    GET_BOOL ("print_header", PREFS_PRINT_HEADER);
    GET_BOOL ("header_separator", PREFS_PRINT_HEADER_SEPARATOR);
    GET_TEXT ("header_left", PREFS_HEADER_LEFT);
    GET_TEXT ("header_center", PREFS_HEADER_CENTER);
    GET_TEXT ("header_right", PREFS_HEADER_RIGHT);

    GET_BOOL ("print_footer", PREFS_PRINT_FOOTER);
    GET_BOOL ("footer_separator", PREFS_PRINT_FOOTER_SEPARATOR);
    GET_TEXT ("footer_left", PREFS_FOOTER_LEFT);
    GET_TEXT ("footer_center", PREFS_FOOTER_CENTER);
    GET_TEXT ("footer_right", PREFS_FOOTER_RIGHT);

    GET_BOOL ("use_styles", PREFS_USE_STYLES);
    GET_BOOL ("use_custom_font", PREFS_USE_CUSTOM_FONT);
    GET_BOOL ("wrap", PREFS_WRAP);
    GET_BOOL ("ellipsize", PREFS_ELLIPSIZE);

    if (gtk_toggle_button_get_active (moo_glade_xml_get_widget (xml, "use_custom_font")))
        moo_prefs_set_string (PREFS_FONT,
                              gtk_font_button_get_font_name (moo_glade_xml_get_widget (xml, "font")));
}


void
_moo_edit_print_options_dialog (GtkWidget *parent)
{
    GtkWidget *dialog;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_from_buf (MOO_PRINT_GLADE_XML, -1, NULL);
    g_return_if_fail (xml != NULL);

    dialog = moo_glade_xml_get_widget (xml, "dialog");
    g_return_if_fail (dialog != NULL);

    moo_window_set_parent (dialog, parent);

    set_options (xml);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
        get_options (xml);

    g_object_unref (xml);
    gtk_widget_destroy (dialog);
}


static GtkWidget *
moo_print_operation_create_custom_widget (G_GNUC_UNUSED GtkPrintOperation *operation)
{
    GtkWidget *page;
    MooGladeXML *xml;

    xml = moo_glade_xml_new_from_buf (MOO_PRINT_GLADE_XML, -1, "page");
    g_return_val_if_fail (xml != NULL, NULL);

    page = moo_glade_xml_get_widget (xml, "page");
    g_return_val_if_fail (page != NULL, NULL);

    g_object_set_data_full (G_OBJECT (page), "moo-glade-xml",
                            xml, g_object_unref);
    set_options (xml);
    return page;
}


static void
moo_print_operation_custom_widget_apply (G_GNUC_UNUSED GtkPrintOperation *print,
                                         GtkWidget *widget)
{
    MooGladeXML *xml;

    xml = g_object_get_data (G_OBJECT (widget), "moo-glade-xml");
    g_return_if_fail (xml != NULL);

    get_options (xml);
}


/*****************************************************************************/
/* Format string
 */

typedef enum {
    HF_FORMAT_TIME,
    HF_FORMAT_PAGE,
    HF_FORMAT_TOTAL_PAGES,
    HF_FORMAT_FILENAME,
    HF_FORMAT_BASENAME
} HFFormatType;

typedef struct {
    HFFormatType type;
    char *string;
} HFFormatChunk;

struct _HFFormat {
    GSList *chunks;
};


static HFFormatChunk *
hf_format_chunk_new (HFFormatType type,
                     const char  *string,
                     int          len)
{
    HFFormatChunk *chunk = g_slice_new0 (HFFormatChunk);

    chunk->type = type;

    if (string)
        chunk->string = g_strndup (string, len >= 0 ? len : (int) strlen (string));

    return chunk;
}


static void
hf_format_chunk_free (HFFormatChunk *chunk)
{
    if (chunk)
    {
        g_free (chunk->string);
        g_slice_free (HFFormatChunk, chunk);
    }
}


static void
hf_format_free (HFFormat *format)
{
    if (format)
    {
        g_slist_foreach (format->chunks, (GFunc) hf_format_chunk_free, NULL);
        g_slist_free (format->chunks);
        g_slice_free (HFFormat, format);
    }
}


#define ADD_CHUNK(format,type,string,len)                           \
    format->chunks =                                                \
        g_slist_prepend (format->chunks,                            \
                         hf_format_chunk_new (type, string, len))


static HFFormat *
hf_format_parse (const char *format_string)
{
    HFFormat *format;
    const char *p, *str;

    g_return_val_if_fail (format_string != NULL, NULL);

    format = g_slice_new0 (HFFormat);
    p = str = format_string;

    while (*p && (p = strchr (p, '%')))
    {
        switch (p[1])
        {
            case 'E':
                if (p != str)
                    ADD_CHUNK (format, HF_FORMAT_TIME, str, p - str);
                switch (p[2])
                {
                    case 0:
                        g_warning ("%s: trailing '%%E' in %s",
                                   G_STRLOC, format_string);
                        goto error;
                    case 'f':
                        ADD_CHUNK (format, HF_FORMAT_BASENAME, NULL, 0);
                        str = p = p + 3;
                        break;
                    case 'F':
                        ADD_CHUNK (format, HF_FORMAT_FILENAME, NULL, 0);
                        str = p = p + 3;
                        break;
                    case 'p':
                        ADD_CHUNK (format, HF_FORMAT_PAGE, NULL, 0);
                        str = p = p + 3;
                        break;
                    case 'P':
                        ADD_CHUNK (format, HF_FORMAT_TOTAL_PAGES, NULL, 0);
                        str = p = p + 3;
                        break;
                    default:
                        g_warning ("%s: unknown format '%%E%c' in %s",
                                   G_STRLOC, p[2], format_string);
                        goto error;
                }
                break;
            default:
                p = p + 2;
        }
    }

    if (*str)
        ADD_CHUNK (format, HF_FORMAT_TIME, str, -1);

    format->chunks = g_slist_reverse (format->chunks);
    return format;

error:
    hf_format_free (format);
    return NULL;
}


static void
eval_strftime (GString    *dest,
               const char *format,
               struct tm  *tm)
{
    gsize result;
    char buf[1024];
    char *retval;

    g_return_if_fail (format != NULL);
    g_return_if_fail (tm != NULL);

    if (!format[0])
        return;

    result = strftime (buf, 1024, format, tm);

    if (!result)
    {
        g_warning ("%s: strftime failed", G_STRLOC);
        return;
    }

    retval = g_locale_to_utf8 (buf, -1, NULL, NULL, NULL);

    if (!retval)
    {
        g_warning ("%s: could not convert result of strftime to UTF8", G_STRLOC);
        return;
    }

    g_string_append (dest, retval);
    g_free (retval);
}


static char *
hf_format_eval (HFFormat   *format,
                struct tm  *tm,
                int         page,
                int         total_pages,
                const char *filename,
                const char *basename)
{
    GString *string;
    GSList *l;

    g_return_val_if_fail (format != NULL, NULL);

    string = g_string_new (NULL);

    for (l = format->chunks; l != NULL; l = l->next)
    {
        HFFormatChunk *chunk = l->data;

        switch (chunk->type)
        {
            case HF_FORMAT_TIME:
                eval_strftime (string, chunk->string, tm);
                break;
            case HF_FORMAT_PAGE:
                g_string_append_printf (string, "%d", page + 1);
                break;
            case HF_FORMAT_TOTAL_PAGES:
                g_string_append_printf (string, "%d", total_pages);
                break;
            case HF_FORMAT_FILENAME:
                if (!filename)
                    g_critical ("%s: filename is NULL", G_STRLOC);
                else
                    g_string_append (string, filename);
                break;
            case HF_FORMAT_BASENAME:
                if (!basename)
                    g_critical ("%s: basename is NULL", G_STRLOC);
                else
                    g_string_append (string, basename);
                break;
        }
    }

    return g_string_free (string, FALSE);
}
