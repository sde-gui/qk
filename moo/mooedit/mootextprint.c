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


static GtkPageSetup *page_setup;
static GtkPrintSettings *print_settings;

static void load_default_settings   (void);

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
                                             int                 page_nr);
static void moo_print_operation_end_print   (GtkPrintOperation  *operation,
                                             GtkPrintContext    *context);


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
moo_print_operation_finalize (GObject *object)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (object);

    if (print->doc)
        g_object_unref (print->doc);
    if (print->buffer)
        g_object_unref (print->buffer);

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
            print->wrap = g_value_get_boolean (value) != 0;
            g_object_notify (object, "wrap");
            break;

        case PROP_WRAP_MODE:
            print->wrap_mode = g_value_get_enum (value);
            g_object_notify (object, "wrap-mode");
            break;

        case PROP_ELLIPSIZE:
            print->ellipsize = g_value_get_boolean (value) != 0;
            g_object_notify (object, "ellipsize");
            break;

        case PROP_FONT:
            g_free (print->font);
            print->font = g_value_dup_string (value);
            g_object_notify (object, "font");
            break;

        case PROP_USE_STYLES:
            print->use_styles = g_value_get_boolean (value);
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
            g_value_set_boolean (value, print->wrap);
            break;

        case PROP_WRAP_MODE:
            g_value_set_enum (value, print->wrap_mode);
            break;

        case PROP_ELLIPSIZE:
            g_value_set_boolean (value, print->ellipsize);
            break;

        case PROP_FONT:
            g_value_set_string (value, print->font);
            break;

        case PROP_USE_STYLES:
            g_value_set_boolean (value, print->use_styles);
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
                                             TRUE,
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
    print->wrap = TRUE;
    print->wrap_mode = PANGO_WRAP_WORD_CHAR;
    print->ellipsize = FALSE;
    print->use_styles = TRUE;
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

    if (parent && GTK_WIDGET_TOPLEVEL (parent))
        parent_window = GTK_WINDOW (parent);

    new_page_setup = gtk_print_run_page_setup_dialog (parent_window,
                                                      page_setup,
                                                      print_settings);

    if (page_setup)
        g_object_unref (page_setup);

    page_setup = new_page_setup;
}


static void
moo_print_operation_begin_print (GtkPrintOperation  *operation,
                                 GtkPrintContext    *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    PangoFontDescription *font = NULL;
    double page_height;
    GtkTextIter iter, print_end;
    GTimer *timer;

    g_return_if_fail (print->buffer != NULL);
    g_return_if_fail (print->first_line >= 0);
    g_return_if_fail (print->last_line < 0 || print->last_line >= print->first_line);
    g_return_if_fail (print->first_line < gtk_text_buffer_get_line_count (print->buffer));
    g_return_if_fail (print->last_line < gtk_text_buffer_get_line_count (print->buffer));

    timer = g_timer_new ();

    if (print->last_line < 0)
        print->last_line = gtk_text_buffer_get_line_count (print->buffer) - 1;

    print->page.x = print->page.y = 0;
    print->page.width = gtk_print_context_get_width (context);
    print->page.height = gtk_print_context_get_height (context);

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

    print->layout = gtk_print_context_create_layout (context);

    if (print->wrap)
    {
        pango_layout_set_width (print->layout, print->page.width * PANGO_SCALE);
        pango_layout_set_wrap (print->layout, print->wrap_mode);
    }
    else if (print->ellipsize)
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

        text = gtk_text_buffer_get_text (print->buffer, &iter, &end, FALSE);
        pango_layout_set_text (print->layout, text, -1);
        g_free (text);

        pango_layout_get_pixel_extents (print->layout, NULL, &line_rect);
        line_height = line_rect.height;

        gtk_text_iter_forward_line (&iter);

        if (page_height + line_height > print->page.height)
        {
            gboolean part = FALSE;
            PangoLayoutIter *layout_iter;

            layout_iter = pango_layout_get_iter (print->layout);

            if (print->wrap && pango_layout_get_line_count (print->layout) > 1)
            {
                double part_height = 0;

                do
                {
                    PangoLayoutLine *layout_line;

                    layout_line = pango_layout_iter_get_line (layout_iter);
                    pango_layout_line_get_pixel_extents (layout_line, NULL, &line_rect);

                    if (page_height + part_height + line_rect.height > print->page.height)
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

        page_height += line_height;
    }

    gtk_print_operation_set_nr_of_pages (operation, print->pages->len);

    g_message ("begin_print: %d pages in %f s", print->pages->len,
               g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
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
print_page (MooPrintOperation *print,
            const GtkTextIter *start,
            const GtkTextIter *end,
            cairo_t           *cr)
{
    char *text;
    GtkTextIter line_start, line_end;
    double offset;

    cairo_set_source_rgb (cr, 0, 0, 0);

    if (!print->use_styles)
    {
        text = gtk_text_buffer_get_text (print->buffer, start, end, FALSE);
        pango_layout_set_text (print->layout, text, -1);
        g_free (text);

        cairo_move_to (cr, print->page.x, print->page.y);
        pango_cairo_show_layout (cr, print->layout);

        return;
    }

    line_start = *start;
    offset = 0;

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

        cairo_move_to (cr, 0, offset);
        pango_cairo_show_layout (cr, print->layout);

        pango_layout_get_pixel_extents (print->layout, NULL, &line_rect);
        offset += line_rect.height;
        gtk_text_iter_forward_line (&line_start);
    }
}


static void
moo_print_operation_draw_page (GtkPrintOperation  *operation,
                               GtkPrintContext    *context,
                               int                 page_nr)
{
    cairo_t *cr;
    GtkTextIter start, end;
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);
    GTimer *timer;

    g_return_if_fail (print->buffer != NULL);
    g_return_if_fail (print->pages != NULL);
    g_return_if_fail (print->layout != NULL);
    g_return_if_fail (page_nr < (int) print->pages->len);

    timer = g_timer_new ();

    cr = gtk_print_context_get_cairo (context);

    start = g_array_index (print->pages, GtkTextIter, page_nr);

    if (page_nr + 1 < (int) print->pages->len)
        end = g_array_index (print->pages, GtkTextIter, page_nr + 1);
    else
        gtk_text_buffer_get_end_iter (print->buffer, &end);

    print_page (print, &start, &end, cr);

    g_message ("page %d: %f s", page_nr, g_timer_elapsed (timer, NULL));
    g_timer_destroy (timer);
}


static void
moo_print_operation_end_print (GtkPrintOperation  *operation,
                               G_GNUC_UNUSED GtkPrintContext *context)
{
    MooPrintOperation *print = MOO_PRINT_OPERATION (operation);

    g_return_if_fail (print->buffer != NULL);

    g_object_unref (print->layout);
    g_array_free (print->pages, TRUE);

    print->layout = NULL;
    print->pages = NULL;
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

    print = g_object_new (MOO_TYPE_PRINT_OPERATION, "doc", view, NULL);

    if (!parent)
        parent = GTK_WIDGET (view);

    parent = gtk_widget_get_toplevel (parent);

    if (GTK_WIDGET_TOPLEVEL (parent))
        parent_window = GTK_WINDOW (parent);

    res = gtk_print_operation_run (print, parent_window, &error);

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
            break;
    }

    g_object_unref (print);
}
