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

    term->priv->layout = pango_layout_new (ctx);

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


// /* interval [first, last) */
// static void draw_range_simple (MooTerm   *term,
//                                guint      abs_row,
//                                guint      first,
//                                guint      last,
//                                gboolean   selected)
// {
//     MooTermLine *line;
//     int screen_row = abs_row - term_top_line (term);
//     int size;
//     static char text[8 * MAX_TERMINAL_WIDTH];
//     PangoLayout *l;
//     GtkWidget *widget = GTK_WIDGET (term);
//
//     g_assert (first <= last);
//     g_return_if_fail (first != last || first == 0);
//     g_assert (last <= term_width (term));
//     g_assert (abs_row < buf_total_height (term->priv->buffer));
//
//     line = buf_line (term->priv->buffer, abs_row);
//     size = term_line_len (line);
//
//     if (size < (int)last)
//     {
//         if ((int)first < size)
//         {
//             guint chars_len = term_line_get_chars (line, text, first, size - first);
//             memset (text + chars_len, EMPTY_CHAR, last - size);
//             size = chars_len + last - size;
//         }
//         else
//         {
//             memset (text, EMPTY_CHAR, last - first);
//             size = last - first;
//         }
//     }
//     else
//     {
//         size = term_line_get_chars (line, text, first, size - first);
//     }
//
//     l = term->priv->pango_lines->line;
//     pango_layout_set_text (l, text, size);
//
//     gdk_draw_rectangle (widget->window,
//                         term->priv->bg[selected ? SELECTED : NORMAL],
//                         TRUE,
//                         first * term->priv->font_info->width,
//                         screen_row * term->priv->font_info->height,
//                         (last - first) * term->priv->font_info->width,
//                         term->priv->font_info->height);
//     gdk_draw_layout (widget->window,
//                      term->priv->fg[selected ? SELECTED : NORMAL],
//                      first * term->priv->font_info->width,
//                      screen_row * term->priv->font_info->height,
//                      l);
// }


// /* interval [first, last) */
// static void draw_range (MooTerm *term, guint abs_row, guint first, guint last)
// {
//     guint screen_width = term_width (term);
//     TermSelection *sel = term->priv->selection;
//     int selected;
//
//     g_assert (first <= last);
//     /* selection bounds may be stupid ??? */
//     g_return_if_fail (first != last || first == 0);
//
//     g_assert (last <= screen_width);
//
//     selected = term_selection_row_selected (sel, abs_row);
//
//     switch (selected)
//     {
//         case FULL_SELECTED:
//         case NOT_SELECTED:
//             draw_range_simple (term, abs_row, first, last, selected);
//             break;
//
//         case PART_SELECTED:
//         {
//             guint l_row = sel->l_row;
//             guint l_col = sel->l_col;
//             guint r_row = sel->r_row;
//             guint r_col = sel->r_col;
//
//             if (l_row == r_row)
//             {
//                 g_assert (abs_row == l_row);
//
//                 if (r_col <= first || last <= l_col)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, FALSE);
//                 }
//                 else if (l_col <= first && last <= r_col)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, TRUE);
//                 }
//                 else if (first < l_col)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, l_col, FALSE);
//                     draw_range_simple (term, abs_row, l_col,
//                                        MIN (last, r_col), TRUE);
//
//                     if (r_col < last)
//                         draw_range_simple (term, abs_row,
//                                            r_col, last, FALSE);
//                 }
//                 else
//                 {
//                     draw_range_simple (term, abs_row, first,
//                                        MIN (last, r_col), TRUE);
//
//                     if (r_col < last)
//                         draw_range_simple (term, abs_row,
//                                            r_col, last, FALSE);
//                 }
//             }
//             else if (l_row == abs_row)
//             {
//                 if (last <= l_col)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, FALSE);
//                 }
//                 else if (l_col <= first)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, TRUE);
//                 }
//                 else
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, l_col, FALSE);
//                     draw_range_simple (term, abs_row,
//                                        l_col, last, TRUE);
//                 }
//             }
//             else
//             {
//                 g_assert (abs_row == r_row);
//
//                 if (last <= r_col)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, TRUE);
//                 }
//                 else if (r_col <= first)
//                 {
//                     draw_range_simple (term, abs_row,
//                                        first, last, FALSE);
//                 }
//                 else {
//                     draw_range_simple (term, abs_row,
//                                        first, r_col, TRUE);
//                     draw_range_simple (term, abs_row,
//                                        r_col, last, FALSE);
//                 }
//             }
//             break;
//         }
//
//         default:
//             g_assert_not_reached ();
//     }
// }


// static void draw_caret (MooTerm *term, guint abs_row, guint col)
// {
//     guint screen_width = term_width (term);
//     MooTermLine *line = buf_line (term->priv->buffer, abs_row);
//     TermSelection *sel = term->priv->selection;
//     char ch[6];
//     guint ch_len;
//     int screen_row = abs_row - term_top_line (term);
//     PangoLayout *l;
//     GdkGC **fg = term->priv->fg;
//     GdkGC **bg = term->priv->bg;
//     guint char_width = term->priv->font_info->width;
//     guint char_height = term->priv->font_info->height;
//     GtkWidget *widget = GTK_WIDGET (term);
//
//     g_assert (col < screen_width);
//
//     if (!buf_cursor_visible (term->priv->buffer))
//         return;
//     /* return draw_range (term, abs_row, col, col + 1); */
//
//     if (col < term_line_len (line))
//     {
//         ch_len = g_unichar_to_utf8 (term_line_get_unichar (line, col), ch);
//     }
//     else
//     {
//         ch[0] = EMPTY_CHAR;
//         ch_len = 1;
//     }
//
//     l = term->priv->pango_lines->line;
//     pango_layout_set_text (l, ch, ch_len);
//
//     switch (term->priv->caret_shape)
//     {
//         case CARET_BLOCK:
//             if (!term_selected (sel, abs_row, col))
//             {
//                 gdk_draw_rectangle (widget->window, bg[CURSOR], TRUE,
//                                     col * char_width,
//                                     screen_row * char_height,
//                                     char_width,
//                                     char_height);
//                 gdk_draw_layout (widget->window, fg[CURSOR],
//                                  col * char_width,
//                                  screen_row * char_height,
//                                  l);
//             }
//             else
//             {
//                 gdk_draw_rectangle (widget->window, bg[CURSOR], FALSE,
//                                     col * char_width,
//                                     screen_row * char_height,
//                                     char_width,
//                                     char_height);
//                 gdk_draw_rectangle (widget->window, fg[CURSOR], TRUE,
//                                     col * char_width,
//                                     screen_row * char_height,
//                                     char_width,
//                                     char_height);
//                 gdk_draw_layout (widget->window, bg[CURSOR],
//                                  col * char_width,
//                                  screen_row * char_height,
//                                  l);
//             }
//             break;
//
//         case CARET_UNDERLINE:
//             if (!term_selected (sel, abs_row, col))
//             {
//                 gdk_draw_rectangle (widget->window, bg[NORMAL], TRUE,
//                                     col * char_width,
//                                     screen_row * char_height,
//                                     char_width,
//                                     char_height);
//                 gdk_draw_layout (widget->window, fg[NORMAL],
//                                  col * char_width,
//                                  screen_row * char_height,
//                                  l);
//                 gdk_draw_rectangle (widget->window, bg[CURSOR],
//                                     TRUE,
//                                     col * char_width,
//                                     (screen_row + 1) * char_height - term->priv->caret_height,
//                                     char_width,
//                                     term->priv->caret_height);
//             }
//             else
//             {
//                 gdk_draw_rectangle (widget->window, bg[SELECTED], TRUE,
//                                     col * char_width,
//                                     screen_row * char_height,
//                                     char_width,
//                                     char_height);
//                 gdk_draw_layout (widget->window, fg[SELECTED],
//                                  col * char_width,
//                                  screen_row * char_height,
//                                  l);
//                 gdk_draw_rectangle (widget->window, fg[CURSOR],
//                                     TRUE,
//                                     col * char_width,
//                                     (screen_row + 1) * char_height - term->priv->caret_height,
//                                     char_width,
//                                     term->priv->caret_height);
//             }
//             break;
//     }
// }


// static void draw_line (MooTerm *term, guint abs_row)
// {
//     MooTermLine *line = buf_line (term->priv->buffer, abs_row);
//     TermSelection *sel = term->priv->selection;
//     GtkWidget *widget = GTK_WIDGET (term);
//     int selected;
//
//     selected = term_selection_row_selected (sel, abs_row);
//
//     switch (selected)
//     {
//         case FULL_SELECTED:
//         case NOT_SELECTED:
//         {
//             int screen_row = abs_row - term_top_line (term);
//             guint char_height = term->priv->font_info->height;
//
//             g_assert (0 <= screen_row);
//             g_assert (screen_row < (int) term_height (term));
//
//             if (!term_pango_lines_valid (term->priv->pango_lines,
//                                          screen_row))
//             {
//                 char buf[8 * MAX_TERMINAL_WIDTH];
//                 guint buf_len = term_line_get_chars (line, buf, 0, -1);
//                 term_pango_lines_set_text (term->priv->pango_lines,
//                                            screen_row,
//                                            buf, buf_len);
//             }
//
//             gdk_draw_rectangle (widget->window,
//                                 term->priv->bg[selected ? SELECTED : NORMAL],
//                                 TRUE,
//                                 0,
//                                 screen_row * char_height,
//                                 widget->allocation.width,
//                                 char_height);
//
//             gdk_draw_layout (widget->window,
//                              term->priv->fg[selected ? SELECTED : NORMAL],
//                              0,
//                              screen_row * char_height,
//                              term_pango_line (term, screen_row));
//
//             break;
//         }
//
//         case PART_SELECTED:
//         {
//             guint l_row = sel->l_row;
//             guint l_col = sel->l_col;
//             guint r_row = sel->r_row;
//             guint r_col = sel->r_col;
//
//             if (l_row == r_row)
//             {
//                 g_assert (abs_row == l_row);
//
//                 if (l_col > 0)
//                     draw_range_simple (term, abs_row, 0, l_col, FALSE);
//
//                 draw_range_simple (term, abs_row, l_col, r_col, TRUE);
//
//                 if (r_col < term_width (term))
//                     draw_range_simple (term, abs_row, r_col,
//                                        term_width (term), FALSE);
//             }
//             else if (l_row == abs_row)
//             {
//                 if (l_col > 0)
//                     draw_range_simple (term, abs_row, 0, l_col, FALSE);
//
//                 draw_range_simple (term, abs_row, l_col,
//                                    term_width (term), TRUE);
//             }
//             else
//             {
//                 g_assert (abs_row == r_row);
//
//                 draw_range_simple (term, abs_row, 0, r_col, TRUE);
//
//                 if (r_col < term_width (term))
//                     draw_range_simple (term, abs_row, r_col,
//                                        term_width (term), FALSE);
//             }
//
//             break;
//         }
//
//         default:
//             g_assert_not_reached ();
//     }
// }


// gboolean    moo_term_expose_event       (GtkWidget      *widget,
//                                          GdkEventExpose *event)
// {
//     GdkRectangle text_rec = {0, 0, 0, 0};
//     GdkRegion *text_reg;
//
//     MooTerm *term = MOO_TERM (widget);
//     MooTermBuffer *buf = term->priv->buffer;
//
//     guint char_width = term->priv->font_info->width;
//     guint char_height = term->priv->font_info->height;
//     guint top_line = term_top_line (term);
//
//     g_assert (top_line <= buf_screen_offset (buf));
//
//     text_rec.width = term_width (term) * char_width;
//     text_rec.height = term_height (term) * char_height;
//
//     if (event->area.x + event->area.width >= text_rec.width)
//     {
//         gdk_draw_rectangle (widget->window,
//                             term->priv->bg[NORMAL], TRUE,
//                             text_rec.width, 0,
//                             char_width,
//                             widget->allocation.height);
//     }
//
//     if (event->area.y + event->area.height >= text_rec.height)
//     {
//         gdk_draw_rectangle (widget->window,
//                             term->priv->bg[NORMAL], TRUE,
//                             0, text_rec.height,
//                             widget->allocation.width,
//                             char_height);
//     }
//
//     text_reg = gdk_region_rectangle (&text_rec);
//     gdk_region_intersect (event->region, text_reg);
//     gdk_region_destroy (text_reg);
//
//     if (!gdk_region_empty (event->region))
//     {
//         GdkRectangle *changed, *rect;
//         int n_changed;
//
//         gdk_region_get_rectangles (event->region,
//                                    &changed, &n_changed);
//         g_assert (changed != NULL);
//
//         for (rect = changed; rect < changed + n_changed; ++rect)
//         {
//             int left    = HOW_MANY (rect->x + 1, char_width) - 1;
//             int top     = HOW_MANY (rect->y + 1, char_height) - 1;
//             int width   = HOW_MANY (rect->x + rect->width, char_width) - left;
//             int height  = HOW_MANY (rect->y + rect->height, char_height) - top;
//
//             int abs_first = top + top_line;
//             int abs_last = abs_first + height;
//
//             if (rect->width == text_rec.width)
//             {
//                 int i;
//                 for (i = abs_first; i < abs_last; ++i)
//                     draw_line (term, i);
//             }
//             else
//             {
//                 int i;
//                 for (i = abs_first; i < abs_last; ++i)
//                     draw_range (term, i, left, left + width);
//             }
//         }
//
//         if (buf_cursor_visible (buf))
//         {
//             int cursor_row_abs = buf_cursor_row_abs (term->priv->buffer);
//             int cursor_col = buf_cursor_col (term->priv->buffer);
//
//             GdkRectangle cursor = {
//                 cursor_col * char_width,
//                 (cursor_row_abs - top_line) * char_height,
//                 char_width, char_height
//             };
//
//             switch (gdk_region_rect_in (event->region, &cursor))
//             {
//                 case GDK_OVERLAP_RECTANGLE_IN:
//                 case GDK_OVERLAP_RECTANGLE_PART:
//                     draw_caret (term, cursor_row_abs, cursor_col);
//                     break;
//
//                 default:
//                     break;
//             }
//         }
//
//         g_free (changed);
//     }
//
//     return TRUE;
// }


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
    int i;
    GtkWidget *widget = GTK_WIDGET (term);
    GdkColormap *colormap;
    GdkGCValues vals;
    GdkColor colors[MOO_TERM_COLOR_MAX];

    g_assert (GTK_WIDGET_REALIZED (widget));

    for (i = 0; i < 3; ++i)
    {
        term->priv->fg[i][MOO_TERM_COLOR_MAX] =
                gdk_gc_new_with_values (widget->window, &vals, 0);
        term->priv->bg[i][MOO_TERM_COLOR_MAX] =
                gdk_gc_new_with_values  (widget->window, &vals, 0);
    }

    gdk_gc_set_foreground (term->priv->fg[NORMAL][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_background (term->priv->fg[NORMAL][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));
    gdk_gc_set_foreground (term->priv->fg[SELECTED][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));
    gdk_gc_set_background (term->priv->fg[SELECTED][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_foreground (term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));
    gdk_gc_set_background (term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_foreground (term->priv->bg[NORMAL][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));
    gdk_gc_set_background (term->priv->bg[NORMAL][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_foreground (term->priv->bg[SELECTED][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_background (term->priv->bg[SELECTED][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));
    gdk_gc_set_foreground (term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                           &(widget->style->black));
    gdk_gc_set_background (term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                           &(widget->style->white));

    gdk_window_set_background (widget->window, &(widget->style->white));

    colormap = gdk_colormap_get_system ();
    g_return_if_fail (colormap != NULL);

    gdk_color_parse ("black", &colors[MOO_TERM_BLACK]);
    gdk_color_parse ("red", &colors[MOO_TERM_RED]);
    gdk_color_parse ("green", &colors[MOO_TERM_GREEN]);
    gdk_color_parse ("yellow", &colors[MOO_TERM_YELLOW]);
    gdk_color_parse ("blue", &colors[MOO_TERM_BLUE]);
    gdk_color_parse ("magenta", &colors[MOO_TERM_MAGENTA]);
    gdk_color_parse ("cyan", &colors[MOO_TERM_CYAN]);
    gdk_color_parse ("white", &colors[MOO_TERM_WHITE]);

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
        gdk_colormap_alloc_color (colormap, &colors[i], TRUE, TRUE);

    for (i = 0; i < MOO_TERM_COLOR_MAX; ++i)
    {
        vals.foreground = colors[i];
        vals.background = colors[i];

        term->priv->fg[NORMAL][i] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_FOREGROUND | GDK_GC_BACKGROUND);

        term->priv->bg[NORMAL][i] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_FOREGROUND | GDK_GC_BACKGROUND);
    }
}


// void        moo_term_buf_content_changed(MooTerm        *term)
// {
//     MooTermBuffer *buf = term->priv->buffer;
//     BufRegion *changed = buf_get_changed (buf);
//
//     if (changed && !buf_region_empty (changed))
//     {
//         GdkRegion *region;
//         GdkRectangle *rect = NULL;
//         int n_rect, i;
//         BufRectangle clip;
//         BufRegion *tmp;
//
//         guint top_line = term_top_line (term);
//         guint screen_offset = buf_screen_offset (buf);
//         int height = term_height (term);
//         g_assert (top_line <= screen_offset);
//
//         changed = buf_region_copy (changed);
//         if (top_line != screen_offset)
//             buf_region_offset (changed, 0, screen_offset - top_line);
//
//         clip.x = 0;
//         clip.width = term_width (term);
//         clip.y = 0;
//         clip.height = height;
//
//         tmp = buf_region_rectangle (&clip);
//         buf_region_intersect (changed, tmp);
//         buf_region_destroy (tmp);
//
//         if (buf_region_empty (changed))
//         {
//             buf_region_destroy (changed);
//             return;
//         }
//
//         gdk_region_get_rectangles (changed, &rect, &n_rect);
//         g_return_if_fail (rect != NULL);
//
//         for (i = 0; i < n_rect; ++i)
//         {
//             rect[i].x       *= term->priv->font_info->width;
//             rect[i].y       *= term->priv->font_info->height;
//             rect[i].width   *= term->priv->font_info->width;
//             rect[i].height  *= term->priv->font_info->height;
//         }
//
//         region = gdk_region_new ();
//         for (i = 0; i < n_rect; ++i)
//             gdk_region_union_with_rect (region, &rect[i]);
//
//         queue_expose (term, region);
//         gdk_region_destroy (region);
//         g_free (rect);
//
//         buf_region_get_clipbox (changed, &clip);
//         g_assert (clip.y + clip.height <= height);
//
//         for (i = clip.y; i < clip.y + clip.height; ++i)
//             term_pango_lines_invalidate (term, i);
//
//         buf_region_destroy (changed);
//     }
// }


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
        GdkRectangle small_rect = {0, 0, 1, 1};

        rect.x = old_col * char_width;
        rect.y = (old_row + screen_offset - top_line) * char_height;

        small_rect.x = old_col;
        small_rect.y = old_row + screen_offset - top_line;
        moo_term_invalidate_content_rect (term, &small_rect);

        if (term->priv->dirty)
            gdk_region_union_with_rect (term->priv->dirty, &rect);
        else
            term->priv->dirty = gdk_region_rectangle (&rect);

        if (new_row != (int) old_row || new_col != (int) old_col)
        {
            rect.x = new_col * char_width;
            rect.y = (new_row + screen_offset - top_line) * char_height;

            gdk_region_union_with_rect (term->priv->dirty, &rect);

            small_rect.x = new_col;
            small_rect.y = new_row + screen_offset - top_line;
            moo_term_invalidate_content_rect (term, &small_rect);
        }

        queue_expose (term, NULL);
    }
}


void        moo_term_init_back_pixmap       (MooTerm        *term)
{
    if (term->priv->back_pixmap)
        return;

    term->priv->back_pixmap =
            gdk_pixmap_new (GTK_WIDGET(term)->window,
                            term->priv->font_info->width * term_width (term),
                            term->priv->font_info->height * term_height (term),
                            -1);

    term->priv->clip = gdk_gc_new (term->priv->back_pixmap);
    gdk_gc_set_clip_origin (term->priv->clip, 0, 0);

    moo_term_invalidate_content_all (term);
}


void        moo_term_resize_back_pixmap     (MooTerm        *term)
{
    if (term->priv->font_changed)
    {
        GdkPixmap *pix;

        term->priv->font_changed = FALSE;

        pix = gdk_pixmap_new (term->priv->back_pixmap,
                              term->priv->font_info->width * term_width (term),
                              term->priv->font_info->height * term_height (term),
                              -1);

        g_object_unref (term->priv->back_pixmap);
        term->priv->back_pixmap = pix;

        moo_term_invalidate_content_all (term);
    }
    else
    {
        GdkPixmap *pix;
        GdkRegion *region;
        guint char_width = term->priv->font_info->width;
        guint char_height = term->priv->font_info->height;
        int old_width, old_height;
        int width =  char_width * term_width (term);
        int height = char_height * term_height (term);
        GdkRectangle rec = {0, 0, width, height};

        pix = gdk_pixmap_new (term->priv->back_pixmap,
                              width, height, -1);

        gdk_drawable_get_size (term->priv->back_pixmap,
                               &old_width, &old_height);

        gdk_gc_set_clip_rectangle (term->priv->clip, &rec);
        gdk_draw_drawable (pix, term->priv->clip,
                           term->priv->back_pixmap,
                           0, 0, 0, 0,
                           MIN (width, old_width),
                           MIN (height, old_height));

        if (width > old_width)
        {
            rec.x = old_width / char_width;
            rec.width = (width - old_width) / char_width;
            rec.y = 0;
            rec.height = height / char_height;
            moo_term_invalidate_content_rect (term, &rec);
        }

        if (height > old_height)
        {
            rec.x = 0;
            rec.width = width / char_width;
            rec.y = old_height / char_height;
            rec.height = (height - old_height) / char_height;
            moo_term_invalidate_content_rect (term, &rec);
        }

        rec.x = rec.y = 0;
        rec.width = width / char_width;
        rec.height = height / char_height;
        if (term->priv->changed_content)
        {
            region = gdk_region_rectangle (&rec);
            gdk_region_intersect (term->priv->changed_content, region);
            gdk_region_destroy (region);
        }

        g_object_unref (term->priv->back_pixmap);
        term->priv->back_pixmap = pix;
    }
}


/* absolute row */
static void term_draw_range                 (MooTerm        *term,
                                             guint           row,
                                             guint           start,
                                             guint           len);

void        moo_term_update_back_pixmap     (MooTerm        *term)
{
    GdkRectangle *rects = NULL;
    int n_rects;
    int i, j;
    int top_line = term_top_line (term);
    int width = term_width (term);
    int height = term_height (term);
    GdkRectangle clip = {0, 0, width, height};

    if (!term->priv->changed_content)
        return;

    gdk_region_get_rectangles (term->priv->changed_content,
                               &rects, &n_rects);

    for (i = 0; i < n_rects; ++i)
    {
        if (gdk_rectangle_intersect (&rects[i], &clip, &rects[i]))
        {
            for (j = 0; j < rects[i].height; ++j)
                term_draw_range (term, top_line + rects[i].y + j,
                                 rects[i].x, rects[i].width);
        }
    }

    g_free (rects);
    gdk_region_destroy (term->priv->changed_content);
    term->priv->changed_content = NULL;
}


void        moo_term_invalidate_content_all (MooTerm        *term)
{
    GdkRectangle rect = {0, 0, term_width (term), term_height (term)};
    moo_term_invalidate_content_rect (term, &rect);
}


void        moo_term_invalidate_content_rect(MooTerm        *term,
                                             GdkRectangle   *rect)
{
    if (term->priv->changed_content)
        gdk_region_union_with_rect (term->priv->changed_content, rect);
    else
        term->priv->changed_content = gdk_region_rectangle (rect);
}


gboolean    moo_term_expose_event       (GtkWidget      *widget,
                                         GdkEventExpose *event)
{
    GdkRectangle text_rec = {0, 0, 0, 0};
    GdkRegion *text_reg;

    MooTerm *term = MOO_TERM (widget);

    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;

    g_assert (term_top_line (term) <= buf_screen_offset (term->priv->buffer));

    text_rec.width = term_width (term) * char_width;
    text_rec.height = term_height (term) * char_height;

    if (event->area.x + event->area.width >= text_rec.width)
    {
        gdk_draw_rectangle (widget->window,
                            term->priv->bg[NORMAL][MOO_TERM_COLOR_MAX], TRUE,
                            text_rec.width, 0,
                            char_width,
                            widget->allocation.height);
    }

    if (event->area.y + event->area.height >= text_rec.height)
    {
        gdk_draw_rectangle (widget->window,
                            term->priv->bg[NORMAL][MOO_TERM_COLOR_MAX], TRUE,
                            0, text_rec.height,
                            widget->allocation.width,
                            char_height);
    }

    text_reg = gdk_region_rectangle (&text_rec);
    gdk_region_intersect (event->region, text_reg);
    gdk_region_destroy (text_reg);

    if (!gdk_region_empty (event->region))
    {
        moo_term_update_back_pixmap (term);
        gdk_gc_set_clip_region (term->priv->clip,
                                event->region);
        gdk_draw_drawable (event->window,
                           term->priv->clip,
                           term->priv->back_pixmap,
                           0, 0, 0, 0,
                           text_rec.width,
                           text_rec.height);
    }

    return TRUE;
}


/****************************************************************************/
/* Drawing
 */

static void term_draw_range_simple          (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             int             selected);
static void term_draw_cells                 (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             MooTermTextAttr *attr,
                                             int             selected);
static void term_draw_caret                 (MooTerm        *term);

static void term_draw_range                 (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len)
{
    TermSelection *sel = term->priv->selection;
    int selected;
    guint first = start;
    guint last = start + len;

    g_return_if_fail (len != 0);
    g_assert (start + len <= term_width (term));
    g_assert (abs_row < buf_total_height (term->priv->buffer));

    if (buf_cursor_visible (term->priv->buffer) &&
        buf_cursor_row_abs (term->priv->buffer) == abs_row)
    {
        guint cursor = buf_cursor_col (term->priv->buffer);

        if (cursor >= start && cursor < start + len)
        {
            if (cursor > start)
                term_draw_range (term, abs_row,
                                 start, cursor - start);

            term_draw_caret (term);

            if (cursor < start + len - 1)
                term_draw_range (term, abs_row,
                                 cursor + 1, start + len - 1 - cursor);

            return;
        }
    }

    selected = term_selection_row_selected (sel, abs_row);

    switch (selected)
    {
        case FULL_SELECTED:
        case NOT_SELECTED:
            term_draw_range_simple (term, abs_row, first, last, selected);
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
                    term_draw_range_simple (term, abs_row,
                                            first, last, FALSE);
                }
                else if (l_col <= first && last <= r_col)
                {
                    term_draw_range_simple (term, abs_row,
                                            first, last, TRUE);
                }
                else if (first < l_col)
                {
                    term_draw_range_simple (term, abs_row,
                                       first, l_col, FALSE);
                    term_draw_range_simple (term, abs_row, l_col,
                                       MIN (last, r_col), TRUE);

                    if (r_col < last)
                        term_draw_range_simple (term, abs_row,
                                           r_col, last, FALSE);
                }
                else
                {
                    term_draw_range_simple (term, abs_row, first,
                                       MIN (last, r_col), TRUE);

                    if (r_col < last)
                        term_draw_range_simple (term, abs_row,
                                           r_col, last, FALSE);
                }
            }
            else if (l_row == abs_row)
            {
                if (last <= l_col)
                {
                    term_draw_range_simple (term, abs_row,
                                       first, last, FALSE);
                }
                else if (l_col <= first)
                {
                    term_draw_range_simple (term, abs_row,
                                       first, last, TRUE);
                }
                else
                {
                    term_draw_range_simple (term, abs_row,
                                       first, l_col, FALSE);
                    term_draw_range_simple (term, abs_row,
                                       l_col, last, TRUE);
                }
            }
            else
            {
                g_assert (abs_row == r_row);

                if (last <= r_col)
                {
                    term_draw_range_simple (term, abs_row,
                                       first, last, TRUE);
                }
                else if (r_col <= first)
                {
                    term_draw_range_simple (term, abs_row,
                                       first, last, FALSE);
                }
                else {
                    term_draw_range_simple (term, abs_row,
                                       first, r_col, TRUE);
                    term_draw_range_simple (term, abs_row,
                                       r_col, last, FALSE);
                }
            }
            break;
        }

        default:
            g_assert_not_reached ();
    }
}


static void term_draw_range_simple          (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             gboolean        selected)
{
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;
    int y = (abs_row - term_top_line (term)) * char_height;

    g_assert (selected == 0 || selected == 1);

    if (start >= line->len)
    {
        gdk_draw_rectangle (term->priv->back_pixmap,
                            term->priv->bg[selected][MOO_TERM_COLOR_MAX],
                            TRUE,
                            start * char_width,
                            y,
                            len * char_width,
                            char_height);

        return;
    }
    else if (start + len > line->len)
    {
        gdk_draw_rectangle (term->priv->back_pixmap,
                            term->priv->bg[selected][MOO_TERM_COLOR_MAX],
                            TRUE,
                            line->len * char_width,
                            y,
                            (start + len - line->len) * char_width,
                            char_height);

        len = line->len - start;
    }

    g_assert (start + len <= line->len);

    while (len)
    {
        guint i;
        MooTermTextAttr *attr = &line->data[start].attr;

        for (i = 1; i < len && !attr_cmp (attr, &line->data[start + i].attr); ++i) ;

        term_draw_cells (term, abs_row, start, i, attr, selected);

        len -= i;
        start += i;
    }
}


static void term_draw_cells                 (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             MooTermTextAttr *attr,
                                             gboolean        selected)
{
    static char buf[8 * MAX_TERMINAL_WIDTH];
    guint buf_len;
    GdkGC *fg = NULL;
    GdkGC *bg = NULL;

    MooTermLine *line = buf_line (term->priv->buffer, abs_row);

    g_assert (len != 0);
    g_assert (start + len <= line->len);

    buf_len = term_line_get_chars (line, buf, start, len);
    g_return_if_fail (buf_len != 0);

    pango_layout_set_text (term->priv->layout, buf, buf_len);

    if (attr->mask & MOO_TERM_TEXT_FOREGROUND)
    {
        g_assert (attr->foreground < MOO_TERM_COLOR_MAX);
        fg = term->priv->fg[selected][attr->foreground];
    }

    if (attr->mask & MOO_TERM_TEXT_BACKGROUND)
    {
        g_assert (attr->background < MOO_TERM_COLOR_MAX);
        bg = term->priv->bg[selected][attr->background];
    }

    if (!fg)
        fg = term->priv->fg[selected][MOO_TERM_COLOR_MAX];
    if (!bg)
        bg = term->priv->bg[selected][MOO_TERM_COLOR_MAX];

    gdk_draw_rectangle (term->priv->back_pixmap,
                        bg,
                        TRUE,
                        start * term->priv->font_info->width,
                        (abs_row - term_top_line (term)) * term->priv->font_info->height,
                        len * term->priv->font_info->width,
                        term->priv->font_info->height);

    gdk_draw_layout (term->priv->back_pixmap,
                     fg,
                     start * term->priv->font_info->width,
                     (abs_row - term_top_line (term)) * term->priv->font_info->height,
                     term->priv->layout);
}


static void term_draw_caret                 (MooTerm        *term)
{
    guint screen_width = term_width (term);
    guint abs_row = buf_cursor_row_abs (term->priv->buffer);
    guint col = buf_cursor_col (term->priv->buffer);
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    TermSelection *sel = term->priv->selection;
    char ch[6];
    guint ch_len;
    int screen_row = abs_row - term_top_line (term);
    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;

    g_assert (col < screen_width);

    if (col < term_line_len (line))
    {
        ch_len = g_unichar_to_utf8 (line->data[col].ch, ch);
    }
    else
    {
        ch[0] = EMPTY_CHAR;
        ch_len = 1;
    }

    pango_layout_set_text (term->priv->layout, ch, ch_len);

    switch (term->priv->caret_shape)
    {
        case CARET_BLOCK:
            if (!term_selected (sel, abs_row, col))
            {
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                                    TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap,
                                 term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX],
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
            }
            else
            {
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                                    FALSE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX],
                                    TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap,
                                 term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
            }
            break;

        case CARET_UNDERLINE:
            if (!term_selected (sel, abs_row, col))
            {
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->bg[NORMAL][MOO_TERM_COLOR_MAX],
                                    TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap,
                                 term->priv->fg[NORMAL][MOO_TERM_COLOR_MAX],
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX],
                                    TRUE,
                                    col * char_width,
                                    (screen_row + 1) * char_height - term->priv->caret_height,
                                    char_width,
                                    term->priv->caret_height);
            }
            else
            {
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->bg[SELECTED][MOO_TERM_COLOR_MAX], TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap,
                                 term->priv->fg[SELECTED][MOO_TERM_COLOR_MAX],
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX],
                                    TRUE,
                                    col * char_width,
                                    (screen_row + 1) * char_height - term->priv->caret_height,
                                    char_width,
                                    term->priv->caret_height);
            }
            break;
    }
}


void        moo_term_buf_content_changed(MooTerm        *term)
{
    MooTermBuffer *buf = term->priv->buffer;
    BufRegion *changed = buf_get_changed (buf);

    if (changed && !buf_region_empty (changed))
    {
        GdkRectangle *rect = NULL;
        int n_rect, i;

        guint top_line = term_top_line (term);
        guint screen_offset = buf_screen_offset (buf);
        int height = term_height (term);
        g_assert (top_line <= screen_offset);

        changed = buf_region_copy (changed);

        if (top_line != screen_offset)
        {
            GdkRectangle clip = {0, 0, term_width (term), height};
            GdkRegion *tmp;

            buf_region_offset (changed, 0, screen_offset - top_line);

            tmp = buf_region_rectangle (&clip);
            buf_region_intersect (changed, tmp);
            buf_region_destroy (tmp);
        }

        if (gdk_region_empty (changed))
        {
            gdk_region_destroy (changed);
            return;
        }

        gdk_region_get_rectangles (changed, &rect, &n_rect);
        g_return_if_fail (rect != NULL);

        if (!term->priv->dirty)
            term->priv->dirty = gdk_region_new ();

        for (i = 0; i < n_rect; ++i)
        {
            moo_term_invalidate_content_rect (term, &rect[i]);

            rect[i].x *= term->priv->font_info->width;
            rect[i].y *= term->priv->font_info->height;
            rect[i].width *= term->priv->font_info->width;
            rect[i].height *= term->priv->font_info->height;

            gdk_region_union_with_rect (term->priv->dirty, &rect[i]);
        }

        queue_expose (term, NULL);
        g_free (rect);

        gdk_region_destroy (changed);
    }
}
