/*
 *   mooterm/mootermdraw.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"


#define CHARS \
"`1234567890-=~!@#$%^&*()_+qwertyuiop[]\\QWERTYUIOP"\
"{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?"

#define HOW_MANY(x, y) (((x) + (y) - 1) / (y))


void        moo_term_init_font_stuff    (MooTerm        *term)
{
    PangoContext *ctx;
    PangoFontDescription *font;

    if (term->priv->pango_lines)
        term_pango_lines_free (term->priv->pango_lines);
    if (term->priv->font_info)
        term_font_info_free (term->priv->font_info);

    ctx = gtk_widget_create_pango_context (GTK_WIDGET (term));
    g_return_if_fail (ctx != NULL);

    font = pango_font_description_from_string (DEFAULT_MONOSPACE_FONT);

    if (!font)
        font = pango_font_description_from_string (DEFAULT_MONOSPACE_FONT2);

    if (font)
    {
        pango_context_set_font_description (ctx, font);
        pango_font_description_free (font);
    }

    term->priv->font_info = term_font_info_new (ctx);
    term->priv->pango_lines = term_pango_lines_new (ctx);

    g_object_unref (ctx);
}


void             term_font_info_calculate   (TermFontInfo           *info)
{
    PangoRectangle logical;
    PangoLayout *layout;
    PangoLayoutIter *iter;

    g_assert (info->ctx != NULL);

    layout = pango_layout_new (info->ctx);
    pango_layout_set_text (layout, CHARS, strlen(CHARS));
    pango_layout_get_extents (layout, NULL, &logical);

    info->width = HOW_MANY (logical.width, strlen (CHARS));
    info->width = PANGO_PIXELS (info->width);

    iter = pango_layout_get_iter (layout);
    info->height = PANGO_PIXELS (logical.height);
    info->ascent = PANGO_PIXELS (pango_layout_iter_get_baseline (iter));

    pango_layout_iter_free (iter);
    g_object_unref (layout);
}


void             term_font_info_set_font    (TermFontInfo           *info,
                                             PangoFontDescription   *font_desc)
{
    g_return_if_fail (font_desc != NULL);

    pango_context_set_font_description (info->ctx, font_desc);

    term_font_info_calculate (info);
}


TermFontInfo    *term_font_info_new         (PangoContext           *ctx)
{
    TermFontInfo *info = g_new0 (TermFontInfo, 1);

    info->ctx = ctx;
    g_object_ref (ctx);

    term_font_info_calculate (info);

    return info;
}


void             term_font_info_free        (TermFontInfo           *info)
{
    if (info)
    {
        g_object_unref (info->ctx);
        g_free (info);
    }
}


TermPangoLines *term_pango_lines_new        (PangoContext   *ctx)
{
    TermPangoLines *l = g_new0 (TermPangoLines, 1);

    l->size = 0;
    l->screen = g_ptr_array_new ();
    l->valid = g_byte_array_new ();
    l->line = pango_layout_new (ctx);
    l->ctx = g_object_ref (ctx);

    return l;
}


void        term_pango_lines_free           (TermPangoLines *lines)
{
    if (lines)
    {
        g_ptr_array_foreach (lines->screen,
                             (GFunc) g_object_unref,
                             NULL);

        g_ptr_array_free (lines->screen, TRUE);
        g_byte_array_free (lines->valid, TRUE);
        g_object_unref (lines->ctx);
        g_object_unref (lines->line);

        g_free (lines);
    }
}


void        term_pango_lines_resize         (MooTerm        *term,
                                             guint           size)
{
    guint i;
    TermPangoLines *lines = term->priv->pango_lines;

    if (size == lines->size)
        return;

    if (size > lines->size)
    {
        g_ptr_array_set_size (lines->screen, size);

        for (i = lines->size; i < size; ++i)
        {
            lines->screen->pdata[i] = pango_layout_new (lines->ctx);
            g_assert (lines->screen->pdata[i] != NULL);
        }
    }
    else
    {
        for (i = size; i < lines->size; ++i)
            g_object_unref (lines->screen->pdata[i]);

        g_ptr_array_set_size (lines->screen, size);
    }

    lines->size = size;
    g_byte_array_set_size (lines->valid, size);
    term_pango_lines_invalidate_all (term);
}


void        term_pango_lines_set_font       (TermPangoLines *lines,
                                             PangoFontDescription *font)
{
    guint i;

    for (i = 0; i < lines->size; ++i)
        pango_layout_set_font_description (lines->screen->pdata[i], font);

    pango_layout_set_font_description (lines->line, font);
}


gboolean    term_pango_lines_valid          (TermPangoLines *lines,
                                             guint           i)
{
    g_assert (i < lines->size);
    return lines->valid->data[i];
}


void        term_pango_lines_invalidate     (MooTerm    *term,
                                             guint       i)
{
    TermPangoLines *lines = term->priv->pango_lines;
    g_assert (i < lines->size);
    lines->valid->data[i] = FALSE;
}


void        term_pango_lines_invalidate_all (MooTerm    *term)
{
    guint i;
    TermPangoLines *lines = term->priv->pango_lines;
    for (i = 0; i < lines->size; ++i)
        lines->valid->data[i] = FALSE;
}


void        term_pango_lines_set_text       (TermPangoLines *lines,
                                             guint           i,
                                             const char     *text,
                                             int             len)
{
    g_assert (i < lines->size);
    pango_layout_set_text (lines->screen->pdata[i], text, len);
    lines->valid->data[i] = TRUE;
}


void        moo_term_force_update           (MooTerm        *term)
{
    GdkRegion *region;
    GdkWindow *window = GTK_WIDGET (term)->window;

    if (term->priv->pending_expose)
    {
        g_source_remove (term->priv->pending_expose);
        term->priv->pending_expose = 0;
    }

    region = gdk_window_get_update_area (window);

    if (term->priv->dirty)
    {
        if (region)
        {
            gdk_region_union (region, term->priv->dirty);
            gdk_region_destroy (term->priv->dirty);
        }
        else
        {
            region = term->priv->dirty;
        }

        term->priv->dirty = NULL;
    }

    if (region)
    {
        GdkEvent *event = gdk_event_new (GDK_EXPOSE);
        event->expose.window = g_object_ref (window);
        event->expose.send_event = TRUE;
        gdk_region_get_clipbox (region, &event->expose.area);
        event->expose.region = region;
        gtk_main_do_event (event);
        gdk_event_free (event);
    }
}


static gboolean emit_expose                 (MooTerm        *term)
{
    term->priv->pending_expose = 0;

    if (term->priv->dirty)
    {
        gdk_window_invalidate_region (GTK_WIDGET (term)->window,
                                      term->priv->dirty,
                                      FALSE);
        gdk_region_destroy (term->priv->dirty);
        term->priv->dirty = NULL;
    }

    return FALSE;
}


static void queue_expose                    (MooTerm        *term,
                                             GdkRegion      *region)
{
    if (region)
    {
        if (term->priv->dirty)
            gdk_region_union (term->priv->dirty, region);
        else
            term->priv->dirty = gdk_region_copy (region);
    }

    if (!term->priv->pending_expose)
    {
        term->priv->pending_expose =
                g_idle_add_full (EXPOSE_PRIORITY,
                                 (GSourceFunc) emit_expose,
                                 term,
                                 NULL);
    }
}


/* interval [first, last) */
inline static void draw_range_simple (MooTerm   *term,
                                      guint      abs_row,
                                      guint      first,
                                      guint      last,
                                      gboolean   selected)
{
    MooTermLine *line;
    int screen_row = abs_row - term_top_line (term);
    int size;
    static char text[8 * MAX_TERMINAL_WIDTH];
    PangoLayout *l;
    GtkWidget *widget = GTK_WIDGET (term);

    g_assert (first <= last);
    g_return_if_fail (first != last || first == 0);
    g_assert (last <= term_width (term));
    g_assert (abs_row < buf_total_height (term->priv->buffer));

    line = buf_line (term->priv->buffer, abs_row);
    size = term_line_len (line);

    if (size < (int)last)
    {
        if ((int)first < size)
        {
            guint chars_len = term_line_get_chars (line, text, first, size - first);
            memset (text + chars_len, EMPTY_CHAR, last - size);
            size = chars_len + last - size;
        }
        else
        {
            memset (text, EMPTY_CHAR, last - first);
            size = last - first;
        }
    }
    else
    {
        size = term_line_get_chars (line, text, first, size - first);
    }

    l = term->priv->pango_lines->line;
    pango_layout_set_text (l, text, size);

    gdk_draw_rectangle (widget->window,
                        term->priv->bg[selected ? SELECTED : NORMAL],
                        TRUE,
                        first * term->priv->font_info->width,
                        screen_row * term->priv->font_info->height,
                        (last - first) * term->priv->font_info->width,
                        term->priv->font_info->height);
    gdk_draw_layout (widget->window,
                     term->priv->fg[selected ? SELECTED : NORMAL],
                     first * term->priv->font_info->width,
                     screen_row * term->priv->font_info->height,
                     l);
}


/* interval [first, last) */
inline static void draw_range (MooTerm *term, guint abs_row, guint first, guint last)
{
    guint screen_width = term_width (term);
    TermSelection *sel = term->priv->selection;
    int selected;

    g_assert (first <= last);
    /* selection bounds may be stupid ??? */
    g_return_if_fail (first != last || first == 0);

    g_assert (last <= screen_width);

    selected = term_selection_row_selected (sel, abs_row);

    switch (selected)
    {
        case FULL_SELECTED:
        case NOT_SELECTED:
            draw_range_simple (term, abs_row, first, last, selected);
            break;

        case PART_SELECTED:
        {
            guint l_row = sel->l_row;
            guint l_col = sel->l_col;
            guint r_row = sel->r_row;
            guint r_col = sel->r_col;

            if (l_row == r_row)
            {
                g_assert (abs_row == l_row);

                if (r_col <= first || last <= l_col)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, FALSE);
                }
                else if (l_col <= first && last <= r_col)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, TRUE);
                }
                else if (first < l_col)
                {
                    draw_range_simple (term, abs_row,
                                       first, l_col, FALSE);
                    draw_range_simple (term, abs_row, l_col,
                                       MIN (last, r_col), TRUE);

                    if (r_col < last)
                        draw_range_simple (term, abs_row,
                                           r_col, last, FALSE);
                }
                else
                {
                    draw_range_simple (term, abs_row, first,
                                       MIN (last, r_col), TRUE);

                    if (r_col < last)
                        draw_range_simple (term, abs_row,
                                           r_col, last, FALSE);
                }
            }
            else if (l_row == abs_row)
            {
                if (last <= l_col)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, FALSE);
                }
                else if (l_col <= first)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, TRUE);
                }
                else
                {
                    draw_range_simple (term, abs_row,
                                       first, l_col, FALSE);
                    draw_range_simple (term, abs_row,
                                       l_col, last, TRUE);
                }
            }
            else
            {
                g_assert (abs_row == r_row);

                if (last <= r_col)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, TRUE);
                }
                else if (r_col <= first)
                {
                    draw_range_simple (term, abs_row,
                                       first, last, FALSE);
                }
                else {
                    draw_range_simple (term, abs_row,
                                       first, r_col, TRUE);
                    draw_range_simple (term, abs_row,
                                       r_col, last, FALSE);
                }
            }
            break;
        }

        default:
            g_assert_not_reached ();
    }
}


inline static void draw_caret (MooTerm *term, guint abs_row, guint col)
{
    guint screen_width = term_width (term);
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    TermSelection *sel = term->priv->selection;
    char ch[6];
    guint ch_len;
    int screen_row = abs_row - term_top_line (term);
    PangoLayout *l;
    GdkGC **fg = term->priv->fg;
    GdkGC **bg = term->priv->bg;
    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;
    GtkWidget *widget = GTK_WIDGET (term);

    g_assert (col < screen_width);

    if (!buf_cursor_visible (term->priv->buffer))
        return;
    /* return draw_range (term, abs_row, col, col + 1); */

    if (col < term_line_len (line))
    {
        ch_len = g_unichar_to_utf8 (term_line_get_unichar (line, col), ch);
    }
    else
    {
        ch[0] = EMPTY_CHAR;
        ch_len = 1;
    }

    l = term->priv->pango_lines->line;
    pango_layout_set_text (l, ch, ch_len);

    switch (term->priv->caret_shape)
    {
        case CARET_BLOCK:
            if (!term_selected (sel, abs_row, col))
            {
                gdk_draw_rectangle (widget->window, bg[CURSOR], TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (widget->window, fg[CURSOR],
                                 col * char_width,
                                 screen_row * char_height,
                                 l);
            }
            else
            {
                gdk_draw_rectangle (widget->window, bg[CURSOR], FALSE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_rectangle (widget->window, fg[CURSOR], TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (widget->window, bg[CURSOR],
                                 col * char_width,
                                 screen_row * char_height,
                                 l);
            }
            break;

        case CARET_UNDERLINE:
            if (!term_selected (sel, abs_row, col))
            {
                gdk_draw_rectangle (widget->window, bg[NORMAL], TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (widget->window, fg[NORMAL],
                                 col * char_width,
                                 screen_row * char_height,
                                 l);
                gdk_draw_rectangle (widget->window, bg[CURSOR],
                                    TRUE,
                                    col * char_width,
                                    (screen_row + 1) * char_height - term->priv->caret_height,
                                    char_width,
                                    term->priv->caret_height);
            }
            else
            {
                gdk_draw_rectangle (widget->window, bg[SELECTED], TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (widget->window, fg[SELECTED],
                                 col * char_width,
                                 screen_row * char_height,
                                 l);
                gdk_draw_rectangle (widget->window, fg[CURSOR],
                                    TRUE,
                                    col * char_width,
                                    (screen_row + 1) * char_height - term->priv->caret_height,
                                    char_width,
                                    term->priv->caret_height);
            }
            break;
    }
}


inline static void draw_line (MooTerm *term, guint abs_row)
{
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    TermSelection *sel = term->priv->selection;
    GtkWidget *widget = GTK_WIDGET (term);
    int selected;

    selected = term_selection_row_selected (sel, abs_row);

    switch (selected)
    {
        case FULL_SELECTED:
        case NOT_SELECTED:
        {
            int screen_row = abs_row - term_top_line (term);
            guint char_height = term->priv->font_info->height;

            g_assert (0 <= screen_row);
            g_assert (screen_row < (int) term_height (term));

            if (!term_pango_lines_valid (term->priv->pango_lines,
                                         screen_row))
            {
                char buf[8 * MAX_TERMINAL_WIDTH];
                guint buf_len = term_line_get_chars (line, buf, 0, -1);
                term_pango_lines_set_text (term->priv->pango_lines,
                                           screen_row,
                                           buf, buf_len);
            }

            gdk_draw_rectangle (widget->window,
                                term->priv->bg[selected ? SELECTED : NORMAL],
                                TRUE,
                                0,
                                screen_row * char_height,
                                widget->allocation.width,
                                char_height);

            gdk_draw_layout (widget->window,
                             term->priv->fg[selected ? SELECTED : NORMAL],
                             0,
                             screen_row * char_height,
                             term_pango_line (term, screen_row));

            break;
        }

        case PART_SELECTED:
        {
            guint l_row = sel->l_row;
            guint l_col = sel->l_col;
            guint r_row = sel->r_row;
            guint r_col = sel->r_col;

            if (l_row == r_row)
            {
                g_assert (abs_row == l_row);

                if (l_col > 0)
                    draw_range_simple (term, abs_row, 0, l_col, FALSE);

                draw_range_simple (term, abs_row, l_col, r_col, TRUE);

                if (r_col < term_width (term))
                    draw_range_simple (term, abs_row, r_col,
                                       term_width (term), FALSE);
            }
            else if (l_row == abs_row)
            {
                if (l_col > 0)
                    draw_range_simple (term, abs_row, 0, l_col, FALSE);

                draw_range_simple (term, abs_row, l_col,
                                   term_width (term), TRUE);
            }
            else
            {
                g_assert (abs_row == r_row);

                draw_range_simple (term, abs_row, 0, r_col, TRUE);

                if (r_col < term_width (term))
                    draw_range_simple (term, abs_row, r_col,
                                       term_width (term), FALSE);
            }

            break;
        }

        default:
            g_assert_not_reached ();
    }
}


gboolean    moo_term_expose_event       (GtkWidget      *widget,
                                         GdkEventExpose *event)
{
    GdkRectangle text_rec = {0, 0, 0, 0};
    GdkRegion *text_reg;

    MooTerm *term = MOO_TERM (widget);
    MooTermBuffer *buf = term->priv->buffer;

    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;
    guint top_line = term_top_line (term);

    g_assert (top_line <= buf_screen_offset (buf));

    text_rec.width = term_width (term) * char_width;
    text_rec.height = term_height (term) * char_height;

    if (event->area.x + event->area.width >= text_rec.width)
    {
        gdk_draw_rectangle (widget->window,
                            term->priv->bg[NORMAL], TRUE,
                            text_rec.width, 0,
                            char_width,
                            widget->allocation.height);
    }

    if (event->area.y + event->area.height >= text_rec.height)
    {
        gdk_draw_rectangle (widget->window,
                            term->priv->bg[NORMAL], TRUE,
                            0, text_rec.height,
                            widget->allocation.width,
                            char_height);
    }

    text_reg = gdk_region_rectangle (&text_rec);
    gdk_region_intersect (event->region, text_reg);
    gdk_region_destroy (text_reg);

    if (!gdk_region_empty (event->region))
    {
        GdkRectangle *changed, *rect;
        int n_changed;

        gdk_region_get_rectangles (event->region,
                                   &changed, &n_changed);
        g_assert (changed != NULL);

        for (rect = changed; rect < changed + n_changed; ++rect)
        {
            int left    = HOW_MANY (rect->x + 1, char_width) - 1;
            int top     = HOW_MANY (rect->y + 1, char_height) - 1;
            int width   = HOW_MANY (rect->x + rect->width, char_width) - left;
            int height  = HOW_MANY (rect->y + rect->height, char_height) - top;

            int abs_first = top + top_line;
            int abs_last = abs_first + height;

            if (rect->width == text_rec.width)
            {
                int i;
                for (i = abs_first; i < abs_last; ++i)
                    draw_line (term, i);
            }
            else
            {
                int i;
                for (i = abs_first; i < abs_last; ++i)
                    draw_range (term, i, left, left + width);
            }
        }

        if (buf_cursor_visible (buf))
        {
            int cursor_row_abs = buf_cursor_row_abs (term->priv->buffer);
            int cursor_col = buf_cursor_col (term->priv->buffer);

            GdkRectangle cursor = {
                cursor_col * char_width,
                (cursor_row_abs - top_line) * char_height,
                char_width, char_height
            };

            switch (gdk_region_rect_in (event->region, &cursor))
            {
                case GDK_OVERLAP_RECTANGLE_IN:
                case GDK_OVERLAP_RECTANGLE_PART:
                    draw_caret (term, cursor_row_abs, cursor_col);
                    break;

                default:
                    break;
            }
        }

        g_free (changed);
    }

    return TRUE;
}


void        moo_term_invalidate_rect    (MooTerm        *term,
                                         BufRectangle   *rect)
{
    GdkRectangle r = {
        rect->x * term->priv->font_info->width,
        rect->y * term->priv->font_info->height,
        rect->width * term->priv->font_info->width,
        rect->height * term->priv->font_info->height
    };

    GdkRegion *region = gdk_region_rectangle (&r);
    queue_expose (term, region);
    gdk_region_destroy (region);
}


void        moo_term_setup_palette      (MooTerm        *term)
{
    int i, j;
    GtkWidget *widget = GTK_WIDGET (term);
    GdkColormap *colormap;
    GdkGCValues vals;

    g_assert (GTK_WIDGET_REALIZED (widget));

    for (i = 0; i < 3; ++i)
    {
        term->priv->fg[i] = gdk_gc_new (widget->window);
        term->priv->bg[i] = gdk_gc_new (widget->window);
    }

    gdk_gc_set_foreground (term->priv->fg[NORMAL],   &(widget->style->black));
    gdk_gc_set_background (term->priv->fg[NORMAL],   &(widget->style->white));
    gdk_gc_set_foreground (term->priv->fg[SELECTED], &(widget->style->white));
    gdk_gc_set_background (term->priv->fg[SELECTED], &(widget->style->black));
    gdk_gc_set_foreground (term->priv->fg[CURSOR],   &(widget->style->white));
    gdk_gc_set_background (term->priv->fg[CURSOR],   &(widget->style->black));
    gdk_gc_set_foreground (term->priv->bg[NORMAL],   &(widget->style->white));
    gdk_gc_set_background (term->priv->bg[NORMAL],   &(widget->style->black));
    gdk_gc_set_foreground (term->priv->bg[SELECTED], &(widget->style->black));
    gdk_gc_set_background (term->priv->bg[SELECTED], &(widget->style->white));
    gdk_gc_set_foreground (term->priv->bg[CURSOR],   &(widget->style->black));
    gdk_gc_set_background (term->priv->bg[CURSOR],   &(widget->style->white));

    gdk_window_set_background (widget->window, &(widget->style->white));

    colormap = gdk_colormap_get_system ();
    g_return_if_fail (colormap != NULL);

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
        term->priv->color[i] = g_new (GdkColor, 1);
    term->priv->color[MOO_TERM_COLOR_MAX] = NULL;

    gdk_color_parse ("black", term->priv->color[MOO_TERM_BLACK]);
    gdk_color_parse ("red", term->priv->color[MOO_TERM_RED]);
    gdk_color_parse ("green", term->priv->color[MOO_TERM_GREEN]);
    gdk_color_parse ("yellow", term->priv->color[MOO_TERM_YELLOW]);
    gdk_color_parse ("blue", term->priv->color[MOO_TERM_BLUE]);
    gdk_color_parse ("magenta", term->priv->color[MOO_TERM_MAGENTA]);
    gdk_color_parse ("cyan", term->priv->color[MOO_TERM_CYAN]);
    gdk_color_parse ("white", term->priv->color[MOO_TERM_WHITE]);

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
        gdk_colormap_alloc_color (colormap,
                                  term->priv->color[i],
                                  TRUE, TRUE);

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
        for (j = 0; j < MOO_TERM_COLOR_MAX; ++j)
    {
        vals.foreground = *term->priv->color[i];
        vals.background = *term->priv->color[j];

        term->priv->pair[i][j] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_FOREGROUND | GDK_GC_BACKGROUND);
    }

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
    {
        vals.foreground = *term->priv->color[i];
        vals.background = *term->priv->color[i];

        term->priv->pair[i][MOO_TERM_COLOR_MAX] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_FOREGROUND);

        term->priv->pair[MOO_TERM_COLOR_MAX][i] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_BACKGROUND);
    }

    term->priv->pair[MOO_TERM_COLOR_MAX][MOO_TERM_COLOR_MAX] =
            gdk_gc_new_with_values (widget->window,
                                    &vals, 0);
}


void        moo_term_buf_content_changed(MooTerm        *term)
{
    MooTermBuffer *buf = term->priv->buffer;
    BufRegion *changed = buf_get_changed (buf);

    if (changed && !buf_region_empty (changed))
    {
        GdkRegion *region;
        GdkRectangle *rect = NULL;
        int n_rect, i;
        BufRectangle clip;
        BufRegion *tmp;

        guint top_line = term_top_line (term);
        guint screen_offset = buf_screen_offset (buf);
        int height = term_height (term);
        g_assert (top_line <= screen_offset);

        changed = buf_region_copy (changed);
        if (top_line != screen_offset)
            buf_region_offset (changed, 0, screen_offset - top_line);

        clip.x = 0;
        clip.width = term_width (term);
        clip.y = 0;
        clip.height = height;

        tmp = buf_region_rectangle (&clip);
        buf_region_intersect (changed, tmp);
        buf_region_destroy (tmp);

        if (buf_region_empty (changed))
        {
            buf_region_destroy (changed);
            return;
        }

        gdk_region_get_rectangles (changed, &rect, &n_rect);
        g_return_if_fail (rect != NULL);

        for (i = 0; i < n_rect; ++i)
        {
            rect[i].x       *= term->priv->font_info->width;
            rect[i].y       *= term->priv->font_info->height;
            rect[i].width   *= term->priv->font_info->width;
            rect[i].height  *= term->priv->font_info->height;
        }

        region = gdk_region_new ();
        for (i = 0; i < n_rect; ++i)
            gdk_region_union_with_rect (region, &rect[i]);

        queue_expose (term, region);
        gdk_region_destroy (region);
        g_free (rect);

        buf_region_get_clipbox (changed, &clip);
        g_assert (clip.y + clip.height <= height);

        for (i = clip.y; i < clip.y + clip.height; ++i)
            term_pango_lines_invalidate (term, i);

        buf_region_destroy (changed);
    }
}


void        moo_term_cursor_moved       (MooTerm        *term,
                                         guint           old_row,
                                         guint           old_col)
{
    MooTermBuffer *buf = term->priv->buffer;

    if (buf_cursor_visible (buf))
    {
        int screen_offset = buf_screen_offset (buf);
        int top_line = term_top_line (term);

        int new_row = buf_cursor_row (buf);
        int new_col = buf_cursor_col (buf);
        guint char_width = term->priv->font_info->width;
        guint char_height = term->priv->font_info->height;

        GdkRectangle rect = {0, 0, char_width, char_height};

        rect.x = old_col * char_width;
        rect.y = (old_row + screen_offset - top_line) * char_height;

        if (term->priv->dirty)
            gdk_region_union_with_rect (term->priv->dirty, &rect);
        else
            term->priv->dirty = gdk_region_rectangle (&rect);

        if (new_row != (int) old_row || new_col != (int) old_col)
        {
            rect.x = new_col * char_width;
            rect.y = (new_row + screen_offset - top_line) * char_height;

            gdk_region_union_with_rect (term->priv->dirty, &rect);
        }

        queue_expose (term, NULL);
    }
}
