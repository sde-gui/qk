/*
 *   mooterm/mootermdraw.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mooterm-private.h"
#include "mooterm/mooterm-selection.h"
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mootermline-private.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>

#define CHAR_WIDTH(term__)    ((term__)->priv->font->width)
#define CHAR_HEIGHT(term__)   ((term__)->priv->font->height)
#define CHAR_ASCENT(term__)   ((term__)->priv->font->ascent)
#define PIXEL_WIDTH(term__)   (CHAR_WIDTH (term__) * (term__)->priv->width)
#define PIXEL_HEIGHT(term__)  (CHAR_HEIGHT (term__) * (term__)->priv->height)

#define CHARS \
"`1234567890-=~!@#$%^&*()_+qwertyuiop[]\\QWERTYUIOP"\
"{}|asdfghjkl;'ASDFGHJKL:\"zxcvbnm,./ZXCVBNM<>?"

#define HOW_MANY(x__,y__) (((x__) + (y__) - 1) / (y__))


static void add_update_timeout  (MooTerm    *term);


static void
font_calculate (MooTermFont *font)
{
    PangoRectangle logical;
    PangoLayout *layout;
    PangoLayoutIter *iter;

    g_assert (font->ctx != NULL);

    layout = pango_layout_new (font->ctx);
    pango_layout_set_text (layout, CHARS, strlen(CHARS));
    pango_layout_get_extents (layout, NULL, &logical);

    font->width = HOW_MANY (logical.width, strlen (CHARS));
    font->width = PANGO_PIXELS ((int)font->width);

    iter = pango_layout_get_iter (layout);
    font->height = PANGO_PIXELS (logical.height);
    font->ascent = PANGO_PIXELS (pango_layout_iter_get_baseline (iter));

    pango_layout_iter_free (iter);
    g_object_unref (layout);
}


static MooTermFont*
moo_term_font_new  (PangoContext   *ctx)
{
    MooTermFont *font = g_new0 (MooTermFont, 1);

    font->ctx = ctx;
    g_object_ref (ctx);

    font_calculate (font);

    return font;
}


static void
moo_term_update_font (MooTerm *term)
{
    PangoFontDescription *font;
    GtkWidget *widget = GTK_WIDGET (term);

    _moo_term_init_font_stuff (term);
    font = widget->style->font_desc;

    if (!pango_font_description_get_size (font))
    {
        g_return_if_reached ();
    }

    pango_context_set_font_description (term->priv->font->ctx, font);
    font_calculate (term->priv->font);

    if (GTK_WIDGET_REALIZED (term))
        _moo_term_update_size (term, FALSE);
}


void
moo_term_set_font_from_string (MooTerm        *term,
                               const char     *fontname)
{
    PangoFontDescription *font;

    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (fontname != NULL);

    font = pango_font_description_from_string (fontname);
    g_return_if_fail (font != NULL);

    gtk_widget_modify_font (GTK_WIDGET (term), font);
    pango_font_description_free (font);
}


void
_moo_term_init_font_stuff (MooTerm *term)
{
    PangoContext *ctx;

    if (term->priv->font)
        _moo_term_font_free (term->priv->font);
    if (term->priv->layout)
        g_object_unref (term->priv->layout);

    gtk_widget_ensure_style (GTK_WIDGET (term));

    ctx = gtk_widget_create_pango_context (GTK_WIDGET (term));
    g_return_if_fail (ctx != NULL);

    term->priv->font = moo_term_font_new (ctx);
    term->priv->layout = pango_layout_new (ctx);

    g_object_unref (ctx);

    gtk_widget_set_size_request (GTK_WIDGET (term),
                                 term->priv->font->width * MIN_TERMINAL_WIDTH,
                                 term->priv->font->height * MIN_TERMINAL_HEIGHT);
}


void
_moo_term_font_free (MooTermFont    *font)
{
    if (font)
    {
        g_object_unref (font->ctx);
        g_free (font);
    }
}


void
_moo_term_invalidate (MooTerm *term)
{
    GdkRectangle rec = {0, 0, 0, 0};
    rec.width = term->priv->width;
    rec.height = term->priv->height;
    _moo_term_invalidate_screen_rect (term, &rec);
}


void
_moo_term_invalidate_screen_rect (MooTerm      *term,
                                  GdkRectangle *rect)
{
    if (GTK_WIDGET_REALIZED (term))
    {
        GdkRectangle r;

        r.x = rect->x * CHAR_WIDTH(term);
        r.y = rect->y * CHAR_HEIGHT(term);
        r.width = rect->width * CHAR_WIDTH(term);
        r.height = rect->height * CHAR_HEIGHT(term);

        gdk_window_invalidate_rect (GTK_WIDGET(term)->window,
                                    &r, FALSE);
    }
}


void
moo_term_set_colors (MooTerm  *term,
                     GdkColor *colors,
                     guint     n_colors)
{
    guint i;

    g_return_if_fail (MOO_IS_TERM (term));
    g_return_if_fail (colors != NULL);
    g_return_if_fail (n_colors == MOO_TERM_NUM_COLORS ||
                      n_colors == 2 * MOO_TERM_NUM_COLORS);

    for (i = 0; i < MOO_TERM_NUM_COLORS; ++i)
    {
        term->priv->palette[i] = colors[i];

        if (n_colors > MOO_TERM_NUM_COLORS)
            term->priv->palette[i + MOO_TERM_NUM_COLORS] = colors[i + MOO_TERM_NUM_COLORS];
        else
            term->priv->palette[ + MOO_TERM_NUM_COLORS] = colors[i];
    }

    if (GTK_WIDGET_REALIZED (term))
        _moo_term_update_palette (term);
}


static void
moo_term_update_text_colors (MooTerm *term)
{
    GtkWidget *widget = GTK_WIDGET (term);
    GdkColormap *colormap;
    GdkGCValues vals;

    if (!GTK_WIDGET_REALIZED (widget))
        return;

    colormap = gdk_colormap_get_system ();
    g_return_if_fail (colormap != NULL);

    gdk_colormap_alloc_color (colormap, &term->priv->fg_color[0],
                              TRUE, TRUE);
    gdk_colormap_alloc_color (colormap, &term->priv->fg_color[1],
                              TRUE, TRUE);
    gdk_colormap_alloc_color (colormap, &term->priv->bg_color,
                              TRUE, TRUE);

    if (term->priv->fg[0])
    {
        g_object_unref (term->priv->fg[0]);
        g_object_unref (term->priv->fg[1]);
        g_object_unref (term->priv->bg);
    }

    term->priv->fg[0] =
            gdk_gc_new_with_values (widget->window, &vals, 0);
    term->priv->fg[1] =
            gdk_gc_new_with_values (widget->window, &vals, 0);
    term->priv->bg =
            gdk_gc_new_with_values (widget->window, &vals, 0);

    gdk_gc_set_foreground (term->priv->fg[0], &term->priv->fg_color[0]);
    gdk_gc_set_foreground (term->priv->fg[1], &term->priv->fg_color[1]);
    gdk_gc_set_foreground (term->priv->bg, &term->priv->bg_color);

    gdk_window_set_background (widget->window, &term->priv->bg_color);

    _moo_term_invalidate (term);
}


static void
moo_term_set_text_colors (MooTerm  *term,
                          GdkColor *fg,
                          GdkColor *fg_bold,
                          GdkColor *bg)
{
    g_return_if_fail (MOO_IS_TERM (term));

    if (fg)
        term->priv->fg_color[0] = *fg;

    if (fg_bold)
        term->priv->fg_color[1] = *fg_bold;
    else if (fg)
        term->priv->fg_color[1] = *fg;

    if (bg)
        term->priv->bg_color = *bg;

    if (GTK_WIDGET_REALIZED (term))
        moo_term_update_text_colors (term);
}


void
_moo_term_init_palette (MooTerm        *term)
{
    int i;
    GdkColor colors[2 * MOO_TERM_NUM_COLORS];
    GtkWidget *widget = GTK_WIDGET (term);

    gtk_widget_ensure_style (widget);

    moo_term_set_text_colors (term,
                              &widget->style->text[GTK_STATE_NORMAL],
                              &widget->style->text[GTK_STATE_NORMAL],
                              &widget->style->base[GTK_STATE_NORMAL]);

    for (i = 0; i < 2; ++i)
    {
        gdk_color_parse ("black", &colors[8 * i + MOO_TERM_BLACK]);
        gdk_color_parse ("red", &colors[8 * i + MOO_TERM_RED]);
        gdk_color_parse ("green", &colors[8 * i + MOO_TERM_GREEN]);
        gdk_color_parse ("yellow", &colors[8 * i + MOO_TERM_YELLOW]);
        gdk_color_parse ("blue", &colors[8 * i + MOO_TERM_BLUE]);
        gdk_color_parse ("magenta", &colors[8 * i + MOO_TERM_MAGENTA]);
        gdk_color_parse ("cyan", &colors[8 * i + MOO_TERM_CYAN]);
        gdk_color_parse ("white", &colors[8 * i + MOO_TERM_WHITE]);
    }

    moo_term_set_colors (term, colors, 2 * MOO_TERM_NUM_COLORS);
}


void
_moo_term_update_palette (MooTerm *term)
{
    int i, j;
    GtkWidget *widget = GTK_WIDGET (term);
    GdkColormap *colormap;
    GdkGCValues vals;

    g_return_if_fail (GTK_WIDGET_REALIZED (widget));

    colormap = gdk_colormap_get_system ();
    g_return_if_fail (colormap != NULL);

    gdk_colormap_alloc_color (colormap, &term->priv->fg_color[0],
                              TRUE, TRUE);
    gdk_colormap_alloc_color (colormap, &term->priv->fg_color[1],
                              TRUE, TRUE);
    gdk_colormap_alloc_color (colormap, &term->priv->bg_color,
                              TRUE, TRUE);

    if (term->priv->color[0])
        for (i = 0; i < 2 * MOO_TERM_NUM_COLORS; ++i)
            g_object_unref (term->priv->color[i]);

    for (i = 0; i < MOO_TERM_NUM_COLORS; ++i)
        for (j = 0; j < 2; ++j)
            gdk_colormap_alloc_color (colormap,
                                      &term->priv->palette[8*j + i],
                                      TRUE, TRUE);

    for (i = 0; i < MOO_TERM_NUM_COLORS; ++i)
        for (j = 0; j < 2; ++j)
    {
        vals.foreground = term->priv->palette[8*j + i];
        term->priv->color[8*j + i] =
                gdk_gc_new_with_values (widget->window,
                                        &vals,
                                        GDK_GC_FOREGROUND);
    }

    moo_term_update_text_colors (term);
}


void
_moo_term_style_set (GtkWidget *widget,
                     G_GNUC_UNUSED GtkStyle *previous_style)
{
    MooTerm *term = MOO_TERM (widget);

    g_return_if_fail (widget->style != NULL);

    moo_term_set_text_colors (term,
                              &widget->style->text[GTK_STATE_NORMAL],
                              &widget->style->text[GTK_STATE_NORMAL],
                              &widget->style->base[GTK_STATE_NORMAL]);
    moo_term_update_text_colors (term);
    moo_term_update_font (term);
}


static void
invalidate_screen_cell (MooTerm        *term,
                        guint           row,
                        guint           column)
{
    int scrollback, top_line;
    GdkRectangle small_rect = {0, 0, 1, 1};

    scrollback = buf_scrollback (term->priv->buffer);
    top_line = term_top_line (term);

    small_rect.x = column;
    small_rect.y = row + scrollback - top_line;

    _moo_term_invalidate_screen_rect (term, &small_rect);
}


void
_moo_term_cursor_moved (MooTerm        *term,
                        MooTermBuffer  *buf)
{
    if (buf == term->priv->buffer)
    {
        term->priv->cursor_row = buf_cursor_row (buf);
        term->priv->cursor_col = buf_cursor_col_display (buf);
        add_update_timeout (term);
    }
}


/* absolute row */
static void term_draw_range (MooTerm        *term,
                             GdkDrawable    *drawable,
                             guint           row,
                             guint           start,
                             guint           len);


inline static void
rect_window_to_screen (MooTerm      *term,
                       GdkRectangle *window,
                       GdkRectangle *screen)
{
    int cw = CHAR_WIDTH (term);
    int ch = CHAR_HEIGHT (term);
    screen->width = (window->x + window->width -1)/cw - window->x/cw + 1;
    screen->x = window->x/cw;
    screen->height = (window->y + window->height -1)/ch - window->y/ch + 1;
    screen->y = window->y/ch;
}


static void
moo_term_draw (MooTerm     *term,
               GdkDrawable *drawable,
               GdkRegion   *region)
{
    GdkRectangle *rects = NULL;
    int n_rects;
    int i, j;
    int top_line = term_top_line (term);
    int width = term->priv->width;
    int height = term->priv->height;
    GdkRectangle clip = {0, 0, 0, 0};

    g_return_if_fail (region != NULL);

    clip.width = width;
    clip.height = height;
    gdk_region_get_rectangles (region, &rects, &n_rects);

    for (i = 0; i < n_rects; ++i)
    {
        GdkRectangle *r = &rects[i];

        rect_window_to_screen (term, r, r);

        if (gdk_rectangle_intersect (r, &clip, r))
        {
            for (j = 0; j < r->height; ++j)
                term_draw_range (term, drawable,
                                 top_line + r->y + j,
                                 r->x, r->width);
        }
    }

    g_free (rects);
}


inline static void
rect_screen_to_window (MooTerm      *term,
                       GdkRectangle *screen,
                       GdkRectangle *window)
{
    int cw = CHAR_WIDTH (term);
    int ch = CHAR_HEIGHT (term);
    window->x = screen->x * cw;
    window->width = screen->width * cw;
    window->y = screen->y * ch;
    window->height = screen->height * ch;
}


static GdkRegion *
region_rectangle (int x,
                  int y,
                  int width,
                  int height)
{
    GdkRectangle rect;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    return gdk_region_rectangle (&rect);
}

static void
region_union_with_rect (GdkRegion **region,
                        int         x,
                        int         y,
                        int         width,
                        int         height)
{
    GdkRectangle rect;

    rect.x = x;
    rect.y = y;
    rect.width = width;
    rect.height = height;

    if (*region)
        gdk_region_union_with_rect (*region, &rect);
    else
        *region = gdk_region_rectangle (&rect);
}


gboolean
_moo_term_expose_event (GtkWidget      *widget,
                        GdkEventExpose *event)
{
    GdkRegion *text_region;
    MooTerm *term = MOO_TERM (widget);

    if (widget->window != event->window)
        return FALSE;

    g_assert (term_top_line (term) <= buf_scrollback (term->priv->buffer));

    text_region = region_rectangle (0, 0, PIXEL_WIDTH (term), PIXEL_HEIGHT (term));
    gdk_region_intersect (text_region, event->region);

    if (!gdk_region_empty (text_region))
        moo_term_draw (term, event->window, text_region);

    gdk_region_destroy (text_region);
    return FALSE;
}


/****************************************************************************/
/* Drawing
 */

static void term_draw_range_simple          (MooTerm        *term,
                                             GdkDrawable    *drawable,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             int             selected);
static void term_draw_cells                 (MooTerm        *term,
                                             GdkDrawable    *drawable,
                                             guint           abs_row,
                                             guint           start,
                                             guint           len,
                                             MooTermTextAttr attr,
                                             int             selected);
static void term_draw_cursor                (MooTerm        *term,
                                             GdkDrawable    *drawable);

static void
term_draw_range (MooTerm        *term,
                 GdkDrawable    *drawable,
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

    if (term->priv->cursor_visible && term->priv->blink_cursor_visible &&
        term->priv->cursor_row + buf_scrollback (term->priv->buffer) == abs_row)
    {
        guint cursor = buf_cursor_col_display (term->priv->buffer);

        if (cursor >= start && cursor < start + len)
        {
            if (cursor > start)
                term_draw_range (term, drawable, abs_row,
                                 start, cursor - start);

            term_draw_cursor (term, drawable);

            if (cursor < start + len - 1)
                term_draw_range (term, drawable, abs_row,
                                 cursor + 1, start + len - 1 - cursor);

            return;
        }
    }

    selected = _moo_term_row_selected (term, abs_row);

    switch (selected)
    {
        case FULL_SELECTED:
        case NOT_SELECTED:
            term_draw_range_simple (term, drawable, abs_row, first, len, selected);
            break;

        case PART_SELECTED:
        {
            guint l_row, l_col, r_row, r_col;

            _moo_term_get_selected_cells (term, &l_row, &l_col,
                                          &r_row, &r_col);

            if (l_row == r_row)
            {
                g_assert (abs_row == l_row);

                if (r_col <= first || last <= l_col)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, FALSE);
                }
                else if (l_col <= first && last <= r_col)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, TRUE);
                }
                else if (first < l_col)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, l_col - first, FALSE);
                    term_draw_range_simple (term, drawable, abs_row, l_col,
                                            MIN (last, r_col) - l_col, TRUE);

                    if (r_col < last)
                        term_draw_range_simple (term, drawable, abs_row,
                                                r_col, last - r_col, FALSE);
                }
                else
                {
                    term_draw_range_simple (term, drawable, abs_row, first,
                                            MIN (last, r_col) - first, TRUE);

                    if (r_col < last)
                        term_draw_range_simple (term, drawable, abs_row,
                                                r_col, last - r_col, FALSE);
                }
            }
            else if (l_row == abs_row)
            {
                if (last <= l_col)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, FALSE);
                }
                else if (l_col <= first)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, TRUE);
                }
                else
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, l_col - first, FALSE);
                    term_draw_range_simple (term, drawable, abs_row,
                                            l_col, last - l_col, TRUE);
                }
            }
            else
            {
                g_assert (abs_row == r_row);

                if (last <= r_col)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, TRUE);
                }
                else if (r_col <= first)
                {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, len, FALSE);
                }
                else {
                    term_draw_range_simple (term, drawable, abs_row,
                                            first, r_col - first, TRUE);
                    term_draw_range_simple (term, drawable, abs_row,
                                            r_col, last - r_col, FALSE);
                }
            }
            break;
        }

        default:
            g_assert_not_reached ();
    }
}


static void
term_draw_range_simple (MooTerm        *term,
                        GdkDrawable    *drawable,
                        guint           abs_row,
                        guint           start,
                        guint           len,
                        gboolean        selected)
{
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);
    int y = (abs_row - term_top_line (term)) * CHAR_HEIGHT(term);
    GdkGC *bg = NULL;
    guint invert;

    g_assert (selected == 0 || selected == 1);

    invert = (selected ? 1 : 0) + (term->priv->colors_inverted ? 1 : 0);
    invert %= 2;

    if (invert)
        bg = term->priv->fg[COLOR_NORMAL];

    if (start >= _moo_term_line_width (line))
    {
        if (bg)
            gdk_draw_rectangle (drawable, bg, TRUE,
                                start * CHAR_WIDTH(term),
                                y,
                                len * CHAR_WIDTH(term),
                                CHAR_HEIGHT(term));

        return;
    }
    else if (start + len > _moo_term_line_width (line))
    {
        if (bg)
            gdk_draw_rectangle (drawable, bg, TRUE,
                                _moo_term_line_width (line) * CHAR_WIDTH(term),
                                y,
                                (start + len - _moo_term_line_width (line)) * CHAR_WIDTH(term),
                                CHAR_HEIGHT(term));

        len = _moo_term_line_width (line) - start;
    }

    g_assert (start + len <= _moo_term_line_width (line));

    while (len)
    {
        guint n = 1;
        MooTermTextAttr attr = _moo_term_line_get_attr (line, start);

        while (n < len)
            if (MOO_TERM_TEXT_ATTR_EQUAL (attr, _moo_term_line_get_attr (line, start + n)))
                n++;
            else
                break;

        term_draw_cells (term, drawable, abs_row, start, n, attr, selected);

        len -= n;
        start += n;
    }
}


static void
term_draw_cells (MooTerm        *term,
                 GdkDrawable    *drawable,
                 guint           abs_row,
                 guint           start,
                 guint           len,
                 MooTermTextAttr attr,
                 gboolean        selected)
{
    static char buf[8 * MAX_TERMINAL_WIDTH];
    guint buf_len;
    GdkGC *fg = NULL;
    GdkGC *bg = NULL;
    guint bold;
    guint invert;
    PangoLayout *layout = term->priv->layout;
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);

    g_assert (len != 0);
    g_assert (start + len <= _moo_term_line_width (line));

    buf_len = _moo_term_line_get_chars (line, buf, start, len);
    g_return_if_fail (buf_len != 0);

    pango_layout_set_text (term->priv->layout, buf, buf_len);

    bold = (attr.mask & MOO_TERM_TEXT_BOLD) ? COLOR_BOLD : COLOR_NORMAL;

    if (attr.mask & MOO_TERM_TEXT_FOREGROUND)
    {
        g_return_if_fail (attr.foreground < MOO_TERM_NUM_COLORS);
        fg = term->priv->color[8 * bold + attr.foreground];
    }
    else
    {
        fg = term->priv->fg[bold];
    }

    if (attr.mask & MOO_TERM_TEXT_BACKGROUND)
    {
        g_return_if_fail (attr.foreground < MOO_TERM_NUM_COLORS);
        bg = term->priv->color[8 * bold + attr.background];
    }
    else
    {
        bg = term->priv->bg;
    }

    invert = (selected ? 1 : 0) + (term->priv->colors_inverted ? 1 : 0) +
            (attr.mask & MOO_TERM_TEXT_REVERSE ? 1 : 0);
    invert %= 2;

    if (invert)
    {
        GdkGC *tmp = fg;
        fg = bg;
        bg = tmp;
    }

    if (bg == term->priv->bg)
        bg = NULL;

    if (bg)
        gdk_draw_rectangle (drawable, bg, TRUE,
                            start * CHAR_WIDTH(term),
                            (abs_row - term_top_line (term)) * CHAR_HEIGHT(term),
                            len * CHAR_WIDTH(term),
                            CHAR_HEIGHT(term));

    if ((attr.mask & MOO_TERM_TEXT_BOLD) && term->priv->settings.bold_pango)
    {
        PangoAttrList *list = pango_attr_list_new ();
        PangoAttribute *a = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
        a->start_index = 0;
        a->end_index = buf_len;
        pango_attr_list_insert (list, a);
        pango_layout_set_attributes (layout, list);
        pango_attr_list_unref (list);
    }

    gdk_draw_layout (drawable,
                     fg,
                     start * CHAR_WIDTH(term),
                     (abs_row - term_top_line (term)) * CHAR_HEIGHT(term),
                     term->priv->layout);

    if ((attr.mask & MOO_TERM_TEXT_BOLD) && term->priv->settings.bold_pango)
        pango_layout_set_attributes (layout, NULL);

    if ((attr.mask & MOO_TERM_TEXT_BOLD) && term->priv->settings.bold_offset)
        gdk_draw_layout (drawable,
                         fg,
                         start * CHAR_WIDTH(term) + 1,
                         (abs_row - term_top_line (term)) * CHAR_HEIGHT(term),
                         term->priv->layout);

    if (attr.mask & MOO_TERM_TEXT_UNDERLINE)
        gdk_draw_line (drawable,
                       fg,
                       start * CHAR_WIDTH(term),
                       (abs_row - term_top_line (term)) * CHAR_HEIGHT(term) + CHAR_ASCENT(term) + 1,
                       (start + len) * CHAR_WIDTH(term) + 1,
                       (abs_row - term_top_line (term)) * CHAR_HEIGHT(term) + CHAR_ASCENT(term) + 1);
}


static void
term_draw_cursor (MooTerm     *term,
                  GdkDrawable *drawable)
{
    guint scrollback = buf_scrollback (term->priv->buffer);
    guint abs_row = term->priv->cursor_row + scrollback;
    guint column = term->priv->cursor_col;
    MooTermLine *line = buf_line (term->priv->buffer, abs_row);

    if (_moo_term_line_width (line) > column)
    {
        term_draw_cells (term, drawable, abs_row, column, 1,
                         _moo_term_line_get_attr (line, column),
                         !_moo_term_cell_selected (term, abs_row, column));
        return;
    }
    else
    {
        GdkGC *color;
        guint invert;

        invert = 1 + (_moo_term_cell_selected (term, abs_row, column) ? 1 : 0) +
                (term->priv->colors_inverted ? 1 : 0);
        invert %= 2;

        if (invert)
            color = term->priv->fg[COLOR_NORMAL];
        else
            color = term->priv->bg;

        gdk_draw_rectangle (drawable,
                            color,
                            TRUE,
                            column * CHAR_WIDTH(term),
                            (abs_row - term_top_line (term)) * CHAR_HEIGHT(term),
                            CHAR_WIDTH(term),
                            CHAR_HEIGHT(term));
    }
}


static void
add_buffer_changed (MooTerm *term)
{
    GdkRegion *changed;
    guint top_line, scrollback;
    int height;
    MooTermBuffer *buf;

    if (!GTK_WIDGET_DRAWABLE (term))
        return;

    buf = term->priv->buffer;
    changed = buf->priv->changed;

    if (!changed || gdk_region_empty (changed))
        return;

    buf->priv->changed = NULL;

    top_line = term_top_line (term);
    scrollback = buf_scrollback (buf);
    height = term->priv->height;

    g_assert (top_line <= scrollback);

    if (top_line != scrollback)
    {
        GdkRegion *tmp;

        gdk_region_offset (changed, 0, scrollback - top_line);

        tmp = region_rectangle (0, 0, term->priv->width, height);
        gdk_region_intersect (changed, tmp);
        gdk_region_destroy (tmp);
    }

    if (gdk_region_empty (changed))
    {
        gdk_region_destroy (changed);
        return;
    }

    if (term->priv->changed)
    {
        gdk_region_union (term->priv->changed, changed);
        gdk_region_destroy (changed);
    }
    else
    {
        term->priv->changed = changed;
    }
}


static gboolean
invalidate_window (MooTerm *term)
{
    GdkRegion *region, *changed, *clip_region;
    GdkRectangle *rectangles;
    int n_rectangles, i;
    int top_line = term_top_line (term);
    int scrollback = buf_scrollback (term->priv->buffer);
    GdkWindow *window = GTK_WIDGET(term)->window;
    gboolean need_redraw = FALSE;

    if (!GTK_WIDGET_DRAWABLE (term))
        return FALSE;

    add_buffer_changed (term);

    if (!term->priv->changed &&
         term->priv->cursor_col == term->priv->cursor_col_old &&
         term->priv->cursor_row == term->priv->cursor_row_old)
            goto out;

    if (term->priv->changed)
    {
        changed = term->priv->changed;
        term->priv->changed = NULL;
    }
    else
    {
        changed = gdk_region_new ();
    }

    if (term->priv->cursor_col != term->priv->cursor_col_old ||
        term->priv->cursor_row != term->priv->cursor_row_old)
    {
        int row;

        row = scrollback + term->priv->cursor_row - top_line;
        if (term->priv->cursor_col < term->priv->width &&
            row >= 0 && row < (int) term->priv->height)
        {
            region_union_with_rect (&changed,
                                    term->priv->cursor_col,
                                    row, 1, 1);
        }

        row = scrollback + term->priv->cursor_row_old - top_line;
        if (term->priv->cursor_col_old < term->priv->width &&
            row >= 0 && row < (int) term->priv->height)
        {
            region_union_with_rect (&changed,
                                    term->priv->cursor_col_old,
                                    row, 1, 1);
        }
    }

    clip_region = region_rectangle (0, 0, term->priv->width, term->priv->height);
    gdk_region_intersect (changed, clip_region);
    gdk_region_destroy (clip_region);

    if (gdk_region_empty (changed))
    {
        gdk_region_destroy (changed);
        goto out;
    }

    gdk_region_get_rectangles (changed, &rectangles, &n_rectangles);
    g_return_val_if_fail (n_rectangles > 0, TRUE);

    for (i = 0; i < n_rectangles; ++i)
    {
        rect_screen_to_window (term, &rectangles[i], &rectangles[i]);
    }

    region = gdk_region_rectangle (&rectangles[0]);

    for (i = 1; i < n_rectangles; ++i)
        gdk_region_union_with_rect (region, &rectangles[i]);

    gdk_window_invalidate_region (window, region, FALSE);
    need_redraw = TRUE;

    g_free (rectangles);
    gdk_region_destroy (region);
    gdk_region_destroy (changed);

out:
    term->priv->cursor_col_old = term->priv->cursor_col;
    term->priv->cursor_row_old = term->priv->cursor_row;

    return need_redraw;
}

static gboolean
update_delay_timeout (MooTerm *term)
{
    /* We only stop the timer if no update request was received in this
     * past cycle.
     */
    if (invalidate_window (term))
        return TRUE;

    term->priv->update_timer = 0;
    return FALSE;
}

static gboolean
update_timeout (MooTerm *term)
{
    invalidate_window (term);

    /* Set a timer such that we do not invalidate for a while. */
    /* This limits the number of times we draw to ~40fps. */
    term->priv->update_timer =
            _moo_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                   UPDATE_REPEAT_TIMEOUT,
                                   (GSourceFunc) update_delay_timeout,
                                   term, NULL);

    return FALSE;
}

static void
add_update_timeout (MooTerm *term)
{
    if (!term->priv->update_timer)
        term->priv->update_timer =
                _moo_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                       UPDATE_TIMEOUT,
                                       (GSourceFunc) update_timeout,
                                       term, NULL);
}

void
_moo_term_buf_content_changed (MooTerm        *term,
                               MooTermBuffer  *buf)
{
    if (GTK_WIDGET_DRAWABLE (term) &&
        buf == term->priv->buffer &&
        buf->priv->changed &&
        !gdk_region_empty (buf->priv->changed))
    {
        add_update_timeout (term);
    }
}


void
_moo_term_buffer_scrolled (MooTermBuffer  *buf,
                           guint           lines,
                           MooTerm        *term)
{
    if (lines && term->priv->buffer == buf && GTK_WIDGET_DRAWABLE (term))
    {
        region_union_with_rect (&term->priv->changed, 0, 0,
                                term->priv->width,
                                term->priv->height);
        add_update_timeout (term);
    }
}


void
_moo_term_invert_colors (MooTerm    *term,
                         gboolean    invert)
{
    if (invert != term->priv->colors_inverted)
    {
        _moo_term_invalidate (term);
        term->priv->colors_inverted = invert;
    }
}


void
_moo_term_set_cursor_visible (MooTerm    *term,
                              gboolean    visible)
{
    term->priv->cursor_visible = visible;
    invalidate_screen_cell (term,
                            term->priv->cursor_row,
                            term->priv->cursor_col);
}


static gboolean
blink (MooTerm *term)
{
    term->priv->blink_cursor_visible =
            !term->priv->blink_cursor_visible;
    invalidate_screen_cell (term,
                            term->priv->cursor_row,
                            term->priv->cursor_col);
    return TRUE;
}


static void
start_cursor_blinking (MooTerm        *term)
{
    if (!term->priv->cursor_blink_timeout_id && term->priv->cursor_blinks)
        term->priv->cursor_blink_timeout_id =
                _moo_timeout_add (term->priv->cursor_blink_time,
                                  (GSourceFunc) blink,
                                  term);
}


static void
stop_cursor_blinking (MooTerm        *term)
{
    if (term->priv->cursor_blink_timeout_id)
    {
        g_source_remove (term->priv->cursor_blink_timeout_id);
        term->priv->cursor_blink_timeout_id = 0;
        term->priv->blink_cursor_visible = TRUE;
        invalidate_screen_cell (term,
                                term->priv->cursor_row,
                                term->priv->cursor_col);
    }
}


void
_moo_term_pause_cursor_blinking (MooTerm        *term)
{
    if (term->priv->cursor_blinks)
    {
        stop_cursor_blinking (term);
        start_cursor_blinking (term);
    }
}


void
_moo_term_set_cursor_blinks (MooTerm        *term,
                             gboolean        blinks)
{
    term->priv->cursor_blinks = blinks;
    if (blinks)
        start_cursor_blinking (term);
    else
        stop_cursor_blinking (term);
    g_object_notify (G_OBJECT (term), "cursor-blinks");
}


void
moo_term_set_cursor_blink_time (MooTerm        *term,
                                guint           ms)
{
    if (ms)
    {
        term->priv->cursor_blink_time = ms;
        _moo_term_set_cursor_blinks (term, TRUE);
        _moo_term_pause_cursor_blinking (term);
    }
    else
    {
        _moo_term_set_cursor_blinks (term, FALSE);
    }
}
