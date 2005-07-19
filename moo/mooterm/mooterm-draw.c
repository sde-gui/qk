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
#include "mooterm/mooterm-selection.h"
#include "mooterm/mootermbuffer-private.h"
#include <string.h>


#define CHARS \
"`1234567890-=~!@#$%^&*()_+qwertyuiop[]\\QWERTYUIOP"\
"{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?"

#define HOW_MANY(x, y) (((x) + (y) - 1) / (y))


static gboolean process_updates     (MooTerm    *term)
{
    gdk_window_process_updates (GTK_WIDGET(term)->window, FALSE);
    term->priv->pending_expose = 0;
    return FALSE;
}

static void     add_update_timeout  (MooTerm    *term)
{
    if (!term->priv->pending_expose)
    {
        term->priv->pending_expose =
                g_timeout_add_full (EXPOSE_PRIORITY,
                                    EXPOSE_TIMEOUT,
                                    (GSourceFunc) process_updates,
                                    term,
                                    NULL);
    }
}

static void     remove_update_timeout   (MooTerm    *term)
{
    if (term->priv->pending_expose)
        g_source_remove (term->priv->pending_expose);

    term->priv->pending_expose = 0;
}


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

    gtk_widget_set_size_request (GTK_WIDGET (term),
                                 term->priv->font_info->width * MIN_TERMINAL_WIDTH,
                                 term->priv->font_info->height * MIN_TERMINAL_HEIGHT);
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


void        moo_term_invalidate_rect    (MooTerm        *term,
                                         GdkRectangle   *rect)
{
    GdkRectangle r = {
        rect->x * term->priv->font_info->width,
        rect->y * term->priv->font_info->height,
        rect->width * term->priv->font_info->width,
        rect->height * term->priv->font_info->height
    };

    gdk_window_invalidate_rect (GTK_WIDGET(term)->window,
                                &r, FALSE);
    moo_term_invalidate_content_rect (term, rect);
    add_update_timeout (term);
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


static void invalidate_screen_cell      (MooTerm        *term,
                                         guint           row,
                                         guint           column)
{
    int scrollback, top_line;
    guint char_width, char_height;
    GdkRectangle rect = {0, 0, 0, 0};
    GdkRectangle small_rect = {0, 0, 1, 1};

    scrollback = buf_scrollback (term->priv->buffer);
    top_line = term_top_line (term);

    char_width = term->priv->font_info->width;
    char_height = term->priv->font_info->height;

    rect.width = char_width;
    rect.height = char_height;

    small_rect.x = column;
    small_rect.y = row + scrollback - top_line;
    rect.x = small_rect.x * char_width;
    rect.y = small_rect.y * char_height;

    moo_term_invalidate_content_rect (term, &small_rect);
    gdk_window_invalidate_rect (GTK_WIDGET(term)->window, &rect, FALSE);

    add_update_timeout (term);
}


void        moo_term_cursor_moved       (MooTerm        *term,
                                         MooTermBuffer  *buf)
{
    int new_row, new_col;

    if (buf != term->priv->buffer || !term->priv->cursor_visible)
        return;

    new_row = buf_cursor_row (buf);
    new_col = buf_cursor_col (buf);

    invalidate_screen_cell (term,
                            term->priv->cursor_row,
                            term->priv->cursor_col);

    if (new_row != (int) term->priv->cursor_row ||
        new_col != (int) term->priv->cursor_col)
    {
        term->priv->cursor_col = new_col;
        term->priv->cursor_row = new_row;
        invalidate_screen_cell (term,
                                term->priv->cursor_row,
                                term->priv->cursor_col);
    }
}


void        moo_term_init_back_pixmap       (MooTerm        *term)
{
    if (term->priv->back_pixmap)
        return;

    term->priv->back_pixmap =
            gdk_pixmap_new (GTK_WIDGET(term)->window,
                            term->priv->font_info->width * term->priv->width,
                            term->priv->font_info->height * term->priv->height,
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
                              term->priv->font_info->width * term->priv->width,
                              term->priv->font_info->height * term->priv->height,
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
        int width =  char_width * term->priv->width;
        int height = char_height * term->priv->height;
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
    int width = term->priv->width;
    int height = term->priv->height;
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
    GdkRectangle rect = {0, 0, term->priv->width, term->priv->height};
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

    g_assert (term_top_line (term) <= buf_scrollback (term->priv->buffer));

    remove_update_timeout (term);

    text_rec.width = term->priv->width * char_width;
    text_rec.height = term->priv->height * char_height;

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
static void term_draw_cursor                (MooTerm        *term);

static void term_draw_range                 (MooTerm        *term,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len)
{
    int selected;
    guint first = start;
    guint last = start + len;

    g_return_if_fail (len != 0);
    g_assert (start + len <= term->priv->width);
    g_assert (abs_row < buf_total_height (term->priv->buffer));

    if (term->priv->cursor_visible &&
        term->priv->cursor_row + buf_scrollback (term->priv->buffer) == abs_row)
    {
        guint cursor = buf_cursor_col (term->priv->buffer);

        if (cursor >= start && cursor < start + len)
        {
            if (cursor > start)
                term_draw_range (term, abs_row,
                                 start, cursor - start);

            term_draw_cursor (term);

            if (cursor < start + len - 1)
                term_draw_range (term, abs_row,
                                 cursor + 1, start + len - 1 - cursor);

            return;
        }
    }

    selected = moo_term_row_selected (term, abs_row);

    switch (selected)
    {
        case FULL_SELECTED:
        case NOT_SELECTED:
            term_draw_range_simple (term, abs_row, first, last, selected);
            break;

        case PART_SELECTED:
        {
            guint l_row, l_col, r_row, r_col;

            moo_term_get_selection_bounds (term, &l_row, &l_col,
                                           &r_row, &r_col);

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
    GdkGC *color;

    g_assert (selected == 0 || selected == 1);

    if (!term->priv->colors_inverted)
    {
        color = term->priv->bg[selected][MOO_TERM_COLOR_MAX];
    }
    else
    {
        color = term->priv->fg[selected][MOO_TERM_COLOR_MAX];
    }

    if (start >= line->len)
    {

        gdk_draw_rectangle (term->priv->back_pixmap,
                            color,
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
                            color,
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

    if (((attr->mask & MOO_TERM_TEXT_REVERSE) && !term->priv->colors_inverted) ||
          (!(attr->mask & MOO_TERM_TEXT_REVERSE) && term->priv->colors_inverted))
    {
        GdkGC *tmp = fg;
        fg = bg;
        bg = tmp;
    }

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

    if (attr->mask & MOO_TERM_TEXT_BOLD && term->priv->settings.allow_bold)
        gdk_draw_layout (term->priv->back_pixmap,
                         fg,
                         start * term->priv->font_info->width + 1,
                         (abs_row - term_top_line (term)) * term->priv->font_info->height,
                         term->priv->layout);

    if (attr->mask & MOO_TERM_TEXT_UNDERLINE)
        gdk_draw_line (term->priv->back_pixmap,
                       fg,
                       start * term->priv->font_info->width,
                       (abs_row - term_top_line (term)) * term->priv->font_info->height + term->priv->font_info->ascent + 1,
                       (start + len) * term->priv->font_info->width + 1,
                       (abs_row - term_top_line (term)) * term->priv->font_info->height + term->priv->font_info->ascent + 1);
}


static void term_draw_cursor                (MooTerm        *term)
{
    guint screen_width = term->priv->width;
    guint abs_row = buf_cursor_row_abs (term->priv->buffer);
    guint col = buf_cursor_col (term->priv->buffer);
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    char ch[6];
    guint ch_len;
    int screen_row = abs_row - term_top_line (term);
    guint char_width = term->priv->font_info->width;
    guint char_height = term->priv->font_info->height;
    gboolean invert = FALSE;
    GdkGC *fg, *bg;
    MooTermTextAttr *attr;

    g_assert (col < screen_width);

    if (col < term_line_len (line))
    {
        ch_len = g_unichar_to_utf8 (line->data[col].ch, ch);
        attr = &line->data[col].attr;
    }
    else
    {
        ch[0] = EMPTY_CHAR;
        ch_len = 1;
        attr = &ZERO_ATTR;
    }

    pango_layout_set_text (term->priv->layout, ch, ch_len);

    if (((attr->mask & MOO_TERM_TEXT_REVERSE) && !term->priv->colors_inverted) ||
          (!(attr->mask & MOO_TERM_TEXT_REVERSE) && term->priv->colors_inverted))
    {
        invert = TRUE;
    }

    switch (term->priv->cursor_shape)
    {
        case CURSOR_BLOCK:
            if (invert)
            {
                fg = term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX];
                bg = term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX];
            }
            else
            {
                fg = term->priv->fg[CURSOR][MOO_TERM_COLOR_MAX];
                bg = term->priv->bg[CURSOR][MOO_TERM_COLOR_MAX];
            }

            if (!moo_term_cell_selected (term, abs_row, col))
            {
                gdk_draw_rectangle (term->priv->back_pixmap,
                                    bg,
                                    TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap,
                                 fg,
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
            }
            else
            {
                gdk_draw_rectangle (term->priv->back_pixmap, bg,
                                    FALSE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_rectangle (term->priv->back_pixmap, fg,
                                    TRUE,
                                    col * char_width,
                                    screen_row * char_height,
                                    char_width,
                                    char_height);
                gdk_draw_layout (term->priv->back_pixmap, bg,
                                 col * char_width,
                                 screen_row * char_height,
                                 term->priv->layout);
            }
            break;

        case CURSOR_UNDERLINE:
            if (!moo_term_cell_selected (term, abs_row, col))
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
                                    (screen_row + 1) * char_height - term->priv->cursor_height,
                                    char_width,
                                    term->priv->cursor_height);
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
                                    (screen_row + 1) * char_height - term->priv->cursor_height,
                                    char_width,
                                    term->priv->cursor_height);
            }
            break;
    }
}


void        moo_term_buf_content_changed(MooTerm        *term,
                                         MooTermBuffer  *buf)
{
    GdkRectangle *rect = NULL;
    GdkRegion *dirty, *changed;
    int n_rect, i;
    guint top_line, scrollback;
    int height;

    changed = buf_get_changed (buf);

    if (buf != term->priv->buffer || !changed || gdk_region_empty (changed))
        return;

    /* TODO TODO TODO*/
    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;

    top_line = term_top_line (term);
    scrollback = buf_scrollback (buf);
    height = term->priv->height;

    g_assert (top_line <= scrollback);

    if (top_line != scrollback)
    {
        GdkRectangle clip = {0, 0, term->priv->width, height};
        GdkRegion *tmp;

        gdk_region_offset (changed, 0, scrollback - top_line);

        tmp = gdk_region_rectangle (&clip);
        gdk_region_intersect (changed, tmp);
        gdk_region_destroy (tmp);
    }

    if (gdk_region_empty (changed))
    {
        gdk_region_destroy (changed);
        return;
    }

    gdk_region_get_rectangles (changed, &rect, &n_rect);
    g_return_if_fail (rect != NULL);

    dirty = gdk_region_new ();

    for (i = 0; i < n_rect; ++i)
    {
        moo_term_invalidate_content_rect (term, &rect[i]);

        rect[i].x *= term->priv->font_info->width;
        rect[i].y *= term->priv->font_info->height;
        rect[i].width *= term->priv->font_info->width;
        rect[i].height *= term->priv->font_info->height;

        gdk_region_union_with_rect (dirty, &rect[i]);
    }

    gdk_window_invalidate_region (GTK_WIDGET(term)->window,
                                  dirty, FALSE);
    add_update_timeout (term);

    g_free (rect);
    gdk_region_destroy (changed);
    gdk_region_destroy (dirty);
}


void        moo_term_force_update           (MooTerm        *term)
{
    GdkRegion *region;
    GdkWindow *window = GTK_WIDGET (term)->window;

    region = gdk_window_get_update_area (window);

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


void        moo_term_invert_colors          (MooTerm    *term,
                                             gboolean    invert)
{
    if (invert != term->priv->colors_inverted)
    {
        moo_term_invalidate_all (term);
        term->priv->colors_inverted = invert;
    }
}


void        moo_term_set_cursor_visible     (MooTerm    *term,
                                             gboolean    visible)
{
    if (term->priv->cursor_visible != visible)
    {
        term->priv->cursor_visible = visible;

        if (visible)
        {
            term->priv->cursor_row = buf_cursor_row (term->priv->buffer);
            term->priv->cursor_col = buf_cursor_col (term->priv->buffer);
        }

        invalidate_screen_cell (term,
                                term->priv->cursor_row,
                                term->priv->cursor_col);
    }
}
