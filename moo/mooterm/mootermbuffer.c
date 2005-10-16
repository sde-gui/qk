/*
 *   mooterm/mootermbuffer.c
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
#include "mooterm/mootermbuffer-private.h"
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermbuffer-graph.h"
#include "mooterm/mootermline-private.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include <string.h>


static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);
static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_construct_properties,
                                                 GObjectConstructParam *construct_param);
static void     moo_term_buffer_finalize        (GObject        *object);

static void     set_defaults        (MooTermBuffer  *buf);

static void     set_screen_width    (MooTermBuffer  *buf,
                                     guint           columns);
static void     set_screen_height   (MooTermBuffer  *buf,
                                     guint           rows);
static void     reset_tab_stops     (MooTermBuffer  *buf);


/* MOO_TYPE_TERM_BUFFER */
G_DEFINE_TYPE (MooTermBuffer, moo_term_buffer, G_TYPE_OBJECT)

enum {
    CHANGED,
    CURSOR_MOVED,
    FEED_CHILD,
    FULL_RESET,
    TABS_CHANGED,
    SCREEN_SIZE_CHANGED,
    NEW_LINE,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_SCREEN_WIDTH,
    PROP_SCREEN_HEIGHT,
    PROP_SCROLLBACK,
    PROP_SCROLLING_REGION_SET
};

static guint signals[LAST_SIGNAL];


static void moo_term_buffer_class_init (MooTermBufferClass *klass)
{
    GObjectClass   *gobject_class = G_OBJECT_CLASS (klass);

    init_drawing_sets ();

    gobject_class->set_property = moo_term_buffer_set_property;
    gobject_class->get_property = moo_term_buffer_get_property;
    gobject_class->constructor = moo_term_buffer_constructor;
    gobject_class->finalize = moo_term_buffer_finalize;

    signals[CHANGED] =
            moo_signal_new_cb ("changed",
                               G_OBJECT_CLASS_TYPE (gobject_class),
                               G_SIGNAL_RUN_LAST,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[CURSOR_MOVED] =
            moo_signal_new_cb ("cursor-moved",
                               G_OBJECT_CLASS_TYPE (gobject_class),
                               G_SIGNAL_RUN_LAST,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[FEED_CHILD] =
            moo_signal_new_cb ("feed-child",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          NULL,
                          NULL, NULL,
                          _moo_marshal_VOID__STRING_INT,
                          G_TYPE_NONE, 2,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_INT);

    signals[FULL_RESET] =
            g_signal_new ("full-reset",
                          G_OBJECT_CLASS_TYPE (gobject_class),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTermBufferClass, full_reset),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[TABS_CHANGED] =
            moo_signal_new_cb ("tabs-changed",
                               G_OBJECT_CLASS_TYPE (gobject_class),
                               G_SIGNAL_RUN_LAST,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[SCREEN_SIZE_CHANGED] =
            moo_signal_new_cb ("screen-size-changed",
                               G_OBJECT_CLASS_TYPE (gobject_class),
                               G_SIGNAL_RUN_LAST,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[NEW_LINE] =
            moo_signal_new_cb ("new-line",
                               G_OBJECT_CLASS_TYPE (gobject_class),
                               G_SIGNAL_RUN_LAST,
                               NULL,
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_WIDTH,
                                     g_param_spec_uint ("screen-width",
                                             "screen-width",
                                             "screen-width",
                                             0, 1000000, 80,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCREEN_HEIGHT,
                                     g_param_spec_uint ("screen-height",
                                             "screen-height",
                                             "screen-height",
                                             0, 1000000, 24,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLBACK,
                                     g_param_spec_uint ("scrollback",
                                             "scrollback",
                                             "scrollback",
                                             0, 1000000, 0,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_SCROLLING_REGION_SET,
                                     g_param_spec_boolean ("scrolling-region-set",
                                             "scrolling-region-set",
                                             "scrolling-region-set",
                                             FALSE,
                                             G_PARAM_READABLE));
}


static void     moo_term_buffer_init            (MooTermBuffer      *buf)
{
    _moo_term_line_mem_init ();

    buf->priv = g_new0 (MooTermBufferPrivate, 1);

    buf->priv->lines = g_ptr_array_new ();
    buf->priv->changed = NULL;
    buf->priv->changed_all = FALSE;

    buf->priv->tag_table = _moo_term_tag_table_new ();
    buf->priv->data_sets = g_hash_table_new (g_direct_hash, g_direct_equal);

    set_defaults (buf);
}


inline static void
delete_line (MooTermBuffer *buf,
             MooTermLine   *line,
             gboolean       remove_tags,
             gboolean       destroy_data)
{
    if (destroy_data && g_hash_table_lookup (buf->priv->data_sets, line))
        g_dataset_destroy (line);
    _moo_term_line_free (line, remove_tags);
}


static void     moo_term_buffer_finalize        (GObject            *object)
{
    guint i;
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    g_hash_table_foreach (buf->priv->data_sets, (GHFunc) g_dataset_destroy, NULL);
    g_hash_table_destroy (buf->priv->data_sets);

    for (i = 0; i < buf->priv->lines->len; ++i)
        delete_line (buf, g_ptr_array_index (buf->priv->lines, i), FALSE, FALSE);
    g_ptr_array_free (buf->priv->lines, TRUE);

    g_list_free (buf->priv->tab_stops);

    if (buf->priv->changed)
        gdk_region_destroy (buf->priv->changed);

    _moo_term_tag_table_free (buf->priv->tag_table);

    g_free (buf->priv);

    G_OBJECT_CLASS (moo_term_buffer_parent_class)->finalize (object);
}


static void     moo_term_buffer_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            if (buf->priv->constructed)
                set_screen_width (buf, g_value_get_uint (value));
            else
                buf->priv->screen_width = g_value_get_uint (value);
            break;

        case PROP_SCREEN_HEIGHT:
            if (buf->priv->constructed)
                set_screen_height (buf, g_value_get_uint (value));
            else
                buf->priv->screen_height = g_value_get_uint (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void     moo_term_buffer_get_property    (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec)
{
    MooTermBuffer *buf = MOO_TERM_BUFFER (object);

    switch (prop_id) {
        case PROP_SCREEN_WIDTH:
            g_value_set_uint (value, buf->priv->screen_width);
            break;

        case PROP_SCREEN_HEIGHT:
            g_value_set_uint (value, buf->priv->screen_height);
            break;

        case PROP_SCROLLBACK:
            g_value_set_uint (value, buf_scrollback (buf));
            break;

        case PROP_SCROLLING_REGION_SET:
            g_value_set_boolean (value, buf->priv->scrolling_region_set);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static GObject *moo_term_buffer_constructor     (GType                  type,
                                                 guint                  n_props,
                                                 GObjectConstructParam *props)
{
    MooTermBuffer *buf;
    GObject *object;
    guint i;

    object = G_OBJECT_CLASS(moo_term_buffer_parent_class)->constructor (type, n_props, props);
    buf = MOO_TERM_BUFFER (object);

    buf->priv->constructed = TRUE;

    buf->priv->current_attr.mask = 0;

    if (buf->priv->screen_width < MIN_TERMINAL_WIDTH)
        buf->priv->screen_width = MIN_TERMINAL_WIDTH;
    if (buf->priv->screen_height < MIN_TERMINAL_HEIGHT)
        buf->priv->screen_height = MIN_TERMINAL_HEIGHT;
    buf->priv->screen_offset = 0;

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;

    for (i = 0; i < buf->priv->screen_height; ++i)
    {
        g_ptr_array_add (buf->priv->lines,
                         _moo_term_line_new (buf->priv->screen_width,
                                             buf->priv->current_attr));
    }

    return object;
}


void
_moo_term_buffer_set_line_data (MooTermBuffer  *buf,
                                MooTermLine    *line,
                                const char     *key,
                                gpointer        data,
                                GDestroyNotify  destroy)
{
    g_dataset_set_data_full (line, key, data, destroy);
    g_hash_table_insert (buf->priv->data_sets, line, line);
}


gpointer
_moo_term_buffer_get_line_data (G_GNUC_UNUSED MooTermBuffer *buf,
                                MooTermLine    *line,
                                const char     *key)
{
    return g_dataset_get_data (line, key);
}


void
_moo_term_buffer_changed (MooTermBuffer  *buf)
{
    if (!buf->priv->freeze_changed_notify)
        g_signal_emit (buf, signals[CHANGED], 0);
}

void
_moo_term_buffer_scrollback_changed (MooTermBuffer  *buf)
{
    g_object_notify (G_OBJECT (buf), "scrollback");
}


static void
set_screen_width (MooTermBuffer  *buf,
                  guint           width)
{
    guint old_width, height, i;

    g_return_if_fail (width >= MIN_TERMINAL_WIDTH);

    old_width = buf->priv->screen_width;
    buf->priv->screen_width = width;
    height = buf->priv->screen_height;

    if (old_width != width)
    {
        if (buf->priv->cursor_col >= width)
            _moo_term_buffer_cursor_move_to (buf, -1, width - 1);

        if (old_width < width)
        {
            GdkRectangle changed = {
                0, old_width,
                buf->priv->screen_height,
                width - old_width
            };

            buf_changed_add_rect (buf, changed);
            _moo_term_buffer_changed (buf);
        }
        else
        {
            reset_tab_stops (buf);
        }

        for (i = 0; i < height; ++i)
            _moo_term_line_resize (buf_screen_line (buf, i),
                                   width, buf->priv->current_attr);

        g_object_notify (G_OBJECT (buf), "screen-width");
        g_signal_emit (buf, signals[SCREEN_SIZE_CHANGED], 0);
    }
}


static void     set_screen_height   (MooTermBuffer  *buf,
                                     guint           height)
{
    guint old_height = buf->priv->screen_height;
    guint width = buf->priv->screen_width;
    gboolean scrollback_changed = FALSE;
    gboolean content_changed = FALSE;
    gboolean cursor_moved = FALSE;

    if (old_height == height)
        return;

    if (buf_get_mode (MODE_CA))
    {
        /* in CA mode do not scroll or something, just resize the screeen */

        guint i;

        if (height > old_height)
        {
            guint add = height - old_height;
            GdkRectangle changed = {0, old_height, width, add};

            for (i = 0; i < height - old_height; ++i)
                g_ptr_array_add (buf->priv->lines,
                                 _moo_term_line_new (width, buf->priv->current_attr));

            buf_changed_add_rect (buf, changed);
            content_changed = TRUE;
        }
        else /* height < old_height */
        {
            guint remove = old_height - height;

            for (i = 1; i <= remove; ++i)
                delete_line (buf, g_ptr_array_index (buf->priv->lines, buf->priv->lines->len - i),
                             TRUE, TRUE);
            g_ptr_array_remove_range (buf->priv->lines,
                                      buf->priv->lines->len - remove,
                                      remove);

            if (buf->priv->cursor_row >= height)
            {
                buf->priv->cursor_row = height - 1;
                cursor_moved = TRUE;
            }
        }
    }
    else /* ! MODE_CA */
    {
        /* if height increases, just add new lines;
           othewise remove lines below cursor, then increment scrollback */

        guint i;

        if (height > old_height)
        {
            guint add = height - old_height;
            GdkRectangle changed = {0, old_height, width, add};

            for (i = 0; i < height - old_height; ++i)
                g_ptr_array_add (buf->priv->lines,
                                 _moo_term_line_new (width, buf->priv->current_attr));

            buf_changed_add_rect (buf, changed);
            content_changed = TRUE;
        }
        else /* height < old_height */
        {
            guint remove = old_height - height;

            if (buf->priv->cursor_row < height)
            {
                for (i = height; i < old_height; ++i)
                    delete_line (buf, buf_screen_line (buf, i), TRUE, TRUE);
                g_ptr_array_remove_range (buf->priv->lines,
                                          buf->priv->lines->len - remove,
                                          remove);
            }
            else
            {
                guint del = old_height - 1 - buf->priv->cursor_row;

                if (del)
                {
                    for (i = buf->priv->cursor_row + 1; i < old_height; ++i)
                        delete_line (buf, buf_screen_line (buf, i), TRUE, TRUE);
                    g_ptr_array_remove_range (buf->priv->lines,
                                              buf->priv->lines->len - del,
                                              del);
                }

                remove -= del;

                buf->priv->screen_offset += remove;
                buf_changed_set_all (buf);
                scrollback_changed = TRUE;
                content_changed = TRUE;
            }

            if (buf->priv->cursor_row >= height)
            {
                buf->priv->cursor_row = height - 1;
                cursor_moved = TRUE;
            }
        }
    }

    buf->priv->screen_height = height;

    _moo_term_buffer_set_scrolling_region (buf, 0, height - 1);

    if (scrollback_changed)
        _moo_term_buffer_scrollback_changed (buf);
    g_object_notify (G_OBJECT (buf), "screen-height");
    g_signal_emit (buf, signals[SCREEN_SIZE_CHANGED], 0);
    if (content_changed)
        _moo_term_buffer_changed (buf);
    if (cursor_moved)
        _moo_term_buffer_cursor_moved (buf);
}


void
moo_term_buffer_set_screen_size (MooTermBuffer  *buf,
                                 guint           columns,
                                 guint           rows)
{
    set_screen_height (buf, rows);
    set_screen_width (buf, columns);
}


void
_moo_term_buffer_cursor_move (MooTermBuffer  *buf,
                              int             rows,
                              int             cols)
{
    int width = buf_screen_width (buf);
    int height = buf_screen_height (buf);
    int cursor_row = buf_cursor_row (buf);
    int cursor_col = buf_cursor_col (buf);

    cursor_col += cols;
    cursor_row += rows;

    if (cursor_row < 0)
        cursor_row = 0;
    else if (cursor_row >= height)
        cursor_row = height - 1;

    if (cursor_col < 0)
        cursor_col = 0;
    else if (cursor_col >= width)
        cursor_col = width - 1;

    _moo_term_buffer_cursor_move_to (buf, cursor_row, cursor_col);
}


void    _moo_term_buffer_cursor_moved (MooTermBuffer  *buf)
{
    if (!buf->priv->freeze_cursor_notify)
        g_signal_emit (buf, signals[CURSOR_MOVED], 0);
}


void    _moo_term_buffer_cursor_move_to  (MooTermBuffer  *buf,
                                         int             row,
                                         int             col)
{
    int width = buf_screen_width (buf);
    int height = buf_screen_height (buf);
    guint old_row = buf_cursor_row (buf);
    guint old_col = buf_cursor_col (buf);

    if (row < 0)
        row = old_row;
    else if (row >= height)
        row = height - 1;

    if (col < 0)
        col = old_col;
    else if (col >= width)
        col = width - 1;

    buf->priv->cursor_row = row;
    buf->priv->cursor_col = col;

    _moo_term_buffer_cursor_moved (buf);
}


/* chars must be valid unicode string */
static void buf_print_unichar_real  (MooTermBuffer  *buf,
                                     gunichar        c)
{
    MooTermLine *line;
    guint width = buf_screen_width (buf);
    guint cursor_row = buf_cursor_row (buf);

    if (c <= MAX_GRAPH)
    {
        if (buf->priv->single_shift >= 0)
        {
            g_assert (buf->priv->single_shift < 4);

            if (graph_sets[buf->priv->GL[buf->priv->single_shift]])
            {
                if (graph_sets[buf->priv->GL[buf->priv->single_shift]][c])
                    c = graph_sets[buf->priv->GL[buf->priv->single_shift]][c];
                else
                    g_warning ("%s: using regular character while in "
                            "graphics mode", G_STRLOC);
            }

            buf->priv->single_shift = -1;
        }
        else if (graph_sets[buf->priv->current_graph_set])
        {
            if (graph_sets[buf->priv->current_graph_set][c])
                c = graph_sets[buf->priv->current_graph_set][c];
            else
                g_warning ("%s: using regular character while in "
                           "graphics mode", G_STRLOC);
        }
    }

    line = buf_screen_line (buf, cursor_row);
    g_assert (_moo_term_line_len (line) == width);

    if (buf_get_mode (MODE_IRM))
    {
        _moo_term_line_insert_unichar (line, buf->priv->cursor_col++,
                                       c, 1, buf->priv->current_attr);
        buf_changed_add_range (buf, cursor_row,
                               buf->priv->cursor_col - 1,
                               width - buf->priv->cursor_col + 1);
    }
    else
    {
        _moo_term_line_set_unichar (line, buf->priv->cursor_col++,
                                    c, 1, buf->priv->current_attr);
        buf_changed_add_range (buf, cursor_row,
                               buf->priv->cursor_col - 1, 1);
    }

    if (buf->priv->cursor_col == width)
    {
        buf->priv->cursor_col--;
        if (buf_get_mode (MODE_DECAWM))
        {
            _moo_term_buffer_new_line (buf);
        }
    }
}


/* chars must be valid unicode string */
void
moo_term_buffer_print_chars (MooTermBuffer  *buf,
                             const char     *chars,
                             int             len)
{
    const char *p = chars;
    const char *s;

    g_return_if_fail (len != 0 && chars != NULL);

    _moo_term_buffer_freeze_changed_notify (buf);
    _moo_term_buffer_freeze_cursor_notify (buf);

    while ((len > 0 && p != chars + len) || (len < 0 && *p != 0))
    {
        for (s = p; ((len > 0 && s != chars + len) || (len < 0 && *s != 0))
             && *s && (*s & 0x80 || (' ' <= *s && *s <= '~')); ++s) ;

        if (s != p)
        {
            const char *i = p;

            for (i = p; i < s; i = g_utf8_next_char (i))
                buf_print_unichar_real (buf, g_utf8_get_char (i));

            p = s;
        }
        else if (!*s)
        {
            ++p;
        }
        else if (*s == 0x7F)
        {
            g_warning ("%s: got DEL", G_STRLOC);
            ++p;
        }
        else
        {
            buf_print_unichar_real (buf, '^');
            buf_print_unichar_real (buf, *s + 0x40);
        }
    }

    _moo_term_buffer_thaw_changed_notify (buf);
    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_changed (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
moo_term_buffer_print_unichar (MooTermBuffer  *buf,
                               gunichar        c)
{
    _moo_term_buffer_freeze_changed_notify (buf);
    _moo_term_buffer_freeze_cursor_notify (buf);

    buf_print_unichar_real (buf, c);

    _moo_term_buffer_thaw_changed_notify (buf);
    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_changed (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_feed_child (MooTermBuffer  *buf,
                             const char     *string,
                             int             len)
{
    g_signal_emit (buf, signals[FEED_CHILD], 0, string, len);
}


static void
reset_tab_stops (MooTermBuffer  *buf)
{
    guint i;
    guint width = buf_screen_width (buf);

    g_list_free (buf->priv->tab_stops);
    buf->priv->tab_stops = NULL;

    for (i = 7; i < width - 1; i += 8)
        buf->priv->tab_stops = g_list_append (buf->priv->tab_stops,
                                              GUINT_TO_POINTER (i));
    g_signal_emit (buf, signals[TABS_CHANGED], 0);
}

guint
_moo_term_buffer_next_tab_stop (MooTermBuffer  *buf,
                                guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) <= current;
         l = l->next) ;

    if (l && GPOINTER_TO_UINT (l->data) > current)
        return GPOINTER_TO_UINT (l->data);
    else
        return buf_screen_width (buf) - 1;
}

guint
_moo_term_buffer_prev_tab_stop (MooTermBuffer  *buf,
                                guint           current)
{
    GList *l;

    for (l = buf->priv->tab_stops;
         l != NULL && GPOINTER_TO_UINT (l->data) < current;
         l = l->next) ;

    if (l && l->prev && GPOINTER_TO_UINT (l->prev->data) < current)
        return GPOINTER_TO_UINT (l->prev->data);
    else
        return 0;
}

void
_moo_term_buffer_clear_tab_stop (MooTermBuffer  *buf,
                                 ClearTabType    what)
{
    switch (what)
    {
        case CLEAR_TAB_AT_CURSOR:
            buf->priv->tab_stops =
                    g_list_remove (buf->priv->tab_stops,
                                   GUINT_TO_POINTER (buf_cursor_col (buf)));

        case CLEAR_ALL_TABS:
            g_list_free (buf->priv->tab_stops);
            buf->priv->tab_stops = NULL;
    }

    g_signal_emit (buf, signals[TABS_CHANGED], 0);
}

static int
cmp_guints (gconstpointer a, gconstpointer b)
{
    if (GPOINTER_TO_UINT (a) < GPOINTER_TO_UINT (b))
        return -1;
    else if (GPOINTER_TO_UINT (a) == GPOINTER_TO_UINT (b))
        return 0;
    else
        return 1;
}

void
_moo_term_buffer_set_tab_stop (MooTermBuffer  *buf)
{
    guint cursor = buf_cursor_col (buf);

    if (!g_list_find (buf->priv->tab_stops, GUINT_TO_POINTER (cursor)))
        buf->priv->tab_stops =
                g_list_insert_sorted (buf->priv->tab_stops,
                                      GUINT_TO_POINTER (cursor),
                                      cmp_guints);

    g_signal_emit (buf, signals[TABS_CHANGED], 0);
}


void
_moo_term_buffer_select_charset (MooTermBuffer  *buf,
                                 guint           set_num,
                                 CharsetType     charset)
{
    g_return_if_fail (set_num < 4 && charset < 5);

    switch (charset)
    {
        case CHARSET_DRAWING:
            buf->priv->GL[set_num] = CHARSET_DRAWING;
            break;

        case CHARSET_ACRSPS:
            g_warning ("%s: choosing graphics instead of"
                       "Alternate Character ROM Special Set", G_STRLOC);
            buf->priv->GL[set_num] = CHARSET_DRAWING;
            break;

        case CHARSET_ACRSSS:
            g_warning ("%s: choosing regular charset instead of"
                    "Alternate Character ROM Standard Set", G_STRLOC);
            buf->priv->GL[set_num] = CHARSET_ASCII;
            break;

        case CHARSET_UK:
            g_warning ("%s: choosing regular charset instead of"
                       "United Kingdom", G_STRLOC);
            buf->priv->GL[set_num] = CHARSET_ASCII;
            break;

        case CHARSET_ASCII:
            buf->priv->GL[set_num] = CHARSET_ASCII;
            break;
    }
}


void
_moo_term_buffer_shift (MooTermBuffer  *buf,
                        guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->current_graph_set = buf->priv->GL[set];
}


void
_moo_term_buffer_single_shift (MooTermBuffer  *buf,
                               guint           set)
{
    g_return_if_fail (set < 4);
    buf->priv->single_shift = set;
}


MooTermBuffer*
moo_term_buffer_new (guint width,
                     guint height)
{
    return g_object_new (MOO_TYPE_TERM_BUFFER,
                         "screen-width", width,
                         "screen-height", height,
                         NULL);
}


void
_moo_term_buffer_set_scrolling_region (MooTermBuffer  *buf,
                                       guint           top_margin,
                                       guint           bottom_margin)
{
    if (top_margin >= bottom_margin ||
        top_margin >= buf->priv->screen_height ||
        bottom_margin >= buf->priv->screen_height)
    {
        top_margin = 0;
        bottom_margin = buf->priv->screen_height - 1;
    }

    buf->priv->top_margin = top_margin;
    buf->priv->bottom_margin = bottom_margin;
    buf->priv->scrolling_region_set =
            (top_margin != 0 || bottom_margin != buf->priv->screen_height - 1);

    g_object_notify (G_OBJECT (buf), "scrolling-region-set");
}


void
_moo_term_buffer_freeze_changed_notify (MooTermBuffer  *buf)
{
    buf->priv->freeze_changed_notify++;
}

void
_moo_term_buffer_thaw_changed_notify (MooTermBuffer  *buf)
{
    if (buf->priv->freeze_changed_notify)
        buf->priv->freeze_changed_notify--;
}

void
_moo_term_buffer_freeze_cursor_notify (MooTermBuffer  *buf)
{
    buf->priv->freeze_cursor_notify++;
}

void
_moo_term_buffer_thaw_cursor_notify (MooTermBuffer  *buf)
{
    if (buf->priv->freeze_cursor_notify)
        buf->priv->freeze_cursor_notify--;
}


#define FREEZE_NOTIFY                               \
G_STMT_START {                                      \
    _moo_term_buffer_freeze_changed_notify (buf);    \
    _moo_term_buffer_freeze_cursor_notify (buf);     \
} G_STMT_END

#define NOTIFY                                      \
G_STMT_START {                                      \
    _moo_term_buffer_changed (buf);                  \
    _moo_term_buffer_cursor_moved (buf);             \
} G_STMT_END

#define THAW_NOTIFY                                 \
G_STMT_START {                                      \
    _moo_term_buffer_thaw_changed_notify (buf);      \
    _moo_term_buffer_thaw_cursor_notify (buf);       \
} G_STMT_END

#define THAW_AND_NOTIFY                             \
G_STMT_START {                                      \
    THAW_NOTIFY;                                    \
    NOTIFY;                                         \
} G_STMT_END

#define NOTIFY_CHANGED  _moo_term_buffer_changed (buf)
#define FREEZE_CHANGED  _moo_term_buffer_freeze_changed_notify (buf)

#define THAW_AND_NOTIFY_CHANGED                     \
G_STMT_START {                                      \
    _moo_term_buffer_thaw_changed_notify (buf);      \
    _moo_term_buffer_changed (buf);                  \
} G_STMT_END


/*****************************************************************************/
/* Terminal stuff
 */

void
_moo_term_buffer_cuu (MooTermBuffer  *buf,
                      guint           n)
{
    guint i;
    guint top = buf->priv->top_margin;

    if (buf->priv->cursor_row < top)
        top = 0;

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n && buf->priv->cursor_row > top; ++i)
        _moo_term_buffer_cursor_move (buf, -1, 0);

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_cud (MooTermBuffer  *buf,
                      guint           n)
{
    guint i;
    guint bottom = buf->priv->bottom_margin;

    if (buf->priv->cursor_row > bottom)
        bottom = buf_screen_height (buf) - 1;

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n && buf->priv->cursor_row < bottom; ++i)
        _moo_term_buffer_cursor_move (buf, 1, 0);

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_new_line (MooTermBuffer  *buf)
{
    FREEZE_NOTIFY;
    _moo_term_buffer_index (buf);
    _moo_term_buffer_cursor_move_to (buf, -1, 0);
    THAW_AND_NOTIFY;
}


void
_moo_term_buffer_index (MooTermBuffer  *buf)
{
    guint cursor_row = buf_cursor_row (buf);
    guint screen_height = buf_screen_height (buf);
    guint width = buf_screen_width (buf);

    g_assert (cursor_row < screen_height);

    if (buf_get_mode (MODE_CA) || buf->priv->scrolling_region_set)
    {
        guint top = buf->priv->top_margin + buf->priv->screen_offset;
        guint bottom = buf->priv->bottom_margin + buf->priv->screen_offset;
        guint cursor = cursor_row + buf->priv->screen_offset;

        if (cursor > bottom || cursor < top)
        {
            g_warning ("got IND outside of scrolling region");
            _moo_term_buffer_cursor_move_to (buf, buf->priv->top_margin, 0);
            cursor_row = buf_cursor_row (buf);
            cursor = cursor_row + buf->priv->screen_offset;
        }

        if (cursor == bottom)
        {
            GdkRectangle changed = {
                0, buf->priv->top_margin, width, bottom - top + 1
            };

            delete_line (buf, g_ptr_array_index (buf->priv->lines, top), TRUE, TRUE);

            memmove (&buf->priv->lines->pdata[top],
                     &buf->priv->lines->pdata[top+1],
                     (bottom - top) * sizeof(gpointer));

            buf->priv->lines->pdata[bottom] =
                    _moo_term_line_new (width, buf->priv->current_attr);

            buf_changed_add_rect (buf, changed);
        }
        else
        {
            buf->priv->cursor_row += 1;
        }
    }
    else
    {
        g_assert (cursor_row < screen_height);

        if (cursor_row == screen_height - 1)
        {
            g_ptr_array_add (buf->priv->lines,
                             _moo_term_line_new (width, buf->priv->current_attr));

            buf->priv->screen_offset += 1;
            _moo_term_buffer_scrollback_changed (buf);

            buf_changed_set_all (buf);
        }
        else
        {
            buf->priv->cursor_row += 1;
        }
    }

    NOTIFY;
    g_signal_emit (buf, signals[NEW_LINE], 0);
}


void
_moo_term_buffer_reverse_index (MooTermBuffer  *buf)
{
    guint width = buf_screen_width (buf);
    guint top = buf->priv->top_margin + buf->priv->screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->screen_offset;
    guint cursor = buf_cursor_row (buf) + buf->priv->screen_offset;

    g_assert (cursor <= bottom && cursor >= top);

    if (cursor == top)
    {
        GdkRectangle changed = {
            0, buf->priv->top_margin, width, bottom - top + 1
        };

        delete_line (buf, g_ptr_array_index (buf->priv->lines, bottom), TRUE, TRUE);

        memmove (&buf->priv->lines->pdata[top+1],
                 &buf->priv->lines->pdata[top],
                 (bottom - top) * sizeof(gpointer));

        buf->priv->lines->pdata[top] =
                _moo_term_line_new (width, buf->priv->current_attr);

        buf_changed_add_rect (buf, changed);
    }
    else
    {
        buf->priv->cursor_row -= 1;
    }

    NOTIFY;
}


void
_moo_term_buffer_backspace (MooTermBuffer  *buf)
{
    _moo_term_buffer_cursor_move (buf, 0, -1);
}


void
_moo_term_buffer_tab (MooTermBuffer  *buf,
                      guint           n)
{
    guint i;
    guint width = buf_screen_width (buf);

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n && buf->priv->cursor_col < width; ++i)
    {
        _moo_term_buffer_cursor_move_to (buf, -1,
            _moo_term_buffer_next_tab_stop (buf, buf->priv->cursor_col));
    }

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_back_tab (MooTermBuffer  *buf,
                           guint           n)
{
    guint i;

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n && buf->priv->cursor_col > 0; ++i)
    {
        _moo_term_buffer_cursor_move_to (buf, -1,
            _moo_term_buffer_prev_tab_stop (buf, buf->priv->cursor_col));
    }

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_linefeed (MooTermBuffer  *buf)
{
    if (buf_get_mode (MODE_LNM))
        _moo_term_buffer_new_line (buf);
    else
        _moo_term_buffer_index (buf);
}


void
_moo_term_buffer_carriage_return (MooTermBuffer  *buf)
{
    _moo_term_buffer_cursor_move_to (buf, -1, 0);
}


static void
set_ansi_foreground (MooTermBuffer *buf,
                     guint          color)
{
    if ((color) < MOO_TERM_COLOR_MAX)
    {
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_FOREGROUND;
        buf->priv->current_attr.foreground = color;
    }
    else
    {
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_FOREGROUND;
    }
}

static void
set_ansi_background (MooTermBuffer *buf,
                     guint          color)
{
    if (color < MOO_TERM_COLOR_MAX)
    {
        buf->priv->current_attr.mask |= MOO_TERM_TEXT_BACKGROUND;
        buf->priv->current_attr.background = color;
    }
    else
    {
        buf->priv->current_attr.mask &= ~MOO_TERM_TEXT_BACKGROUND;
    }
}


void
_moo_term_buffer_sgr (MooTermBuffer  *buf,
                      int            *params,
                      guint           num_params)
{
    guint i;

    if (!num_params)
    {
        buf_set_attrs_mask (0);
        return;
    }

    for (i = 0; i < num_params; ++i)
    {
        int mode = params[i];

        if (mode == -1)
            mode = ANSI_ALL_ATTRIBUTES_OFF;

        switch (mode)
        {
            case ANSI_ALL_ATTRIBUTES_OFF:
                buf_set_attrs_mask (0);
                break;
            case ANSI_BOLD:
                buf_add_attrs_mask (MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERLINE:
                buf_add_attrs_mask (MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINKING:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_NEGATIVE:
                buf_add_attrs_mask (MOO_TERM_TEXT_REVERSE);
                break;
            case ANSI_BOLD_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_BOLD);
                break;
            case ANSI_UNDERLINE_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_UNDERLINE);
                break;
            case ANSI_BLINKING_OFF:
                g_warning ("%s: ignoring blink", G_STRLOC);
                break;
            case ANSI_NEGATIVE_OFF:
                buf_remove_attrs_mask (MOO_TERM_TEXT_REVERSE);
                break;

            default:
                if (30 <= params[i] && params[i] <= 37)
                    set_ansi_foreground (buf, params[i] - 30);
                else if (40 <= params[i] && params[i] <= 47)
                    set_ansi_background (buf, params[i] - 40);
                else if (39 == params[i])
                    set_ansi_foreground (buf, 8);
                else if (49 == params[i])
                    set_ansi_background (buf, 8);
                else
                    g_warning ("%s: unknown text attribute %d",
                               G_STRLOC, params[i]);
        }
    }
}


void
_moo_term_buffer_delete_char (MooTermBuffer  *buf,
                              guint           n)
{
    MooTermLine *line;
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    if (!n || cursor_row > buf->priv->bottom_margin ||
         cursor_row < buf->priv->top_margin)
    {
        return;
    }

    line = buf_screen_line (buf, cursor_row);
    g_assert (_moo_term_line_len (line) == buf_screen_width (buf));
    _moo_term_line_delete_range (line, cursor_col, n, buf->priv->current_attr);
    buf_changed_add_range(buf, cursor_row, cursor_col,
                          buf_screen_width (buf) - cursor_col);
    NOTIFY_CHANGED;
}


void
_moo_term_buffer_erase_range (MooTermBuffer  *buf,
                              guint           row,
                              guint           col,
                              guint           len)
{
    if (row >= buf_screen_height (buf) ||
        col >= buf_screen_width (buf) ||
        !len)
    {
        return;
    }

    _moo_term_line_erase_range (buf_screen_line (buf, row),
                                col, len, buf->priv->current_attr);
    buf_changed_add_range (buf, row, col, len);
    NOTIFY_CHANGED;
}


void
_moo_term_buffer_erase_char (MooTermBuffer  *buf,
                             guint           n)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    _moo_term_buffer_erase_range (buf, cursor_row,
                                  cursor_col, n);
}


void
_moo_term_buffer_erase_in_display (MooTermBuffer  *buf,
                                   EraseType       what)
{
    guint i;
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint width = buf_screen_width (buf);
    guint height = buf_screen_height (buf);

    g_return_if_fail (what == 0 || what == 1 || what == 2);

    FREEZE_CHANGED;

    switch (what)
    {
        case ERASE_FROM_CURSOR:
            _moo_term_buffer_erase_range (buf, cursor_row,
                                          cursor_col, width);
            for (i = cursor_row + 1; i < height; ++i)
                _moo_term_buffer_erase_range (buf, i, 0, width);
            break;

        case ERASE_TO_CURSOR:
            for (i = 0; i < cursor_row; ++i)
                _moo_term_buffer_erase_range (buf, i, 0, width);
            _moo_term_buffer_erase_range (buf, cursor_row,
                                          0, cursor_col + 1);
            break;

        case ERASE_ALL:
            for (i = 0; i < height; ++i)
                _moo_term_buffer_erase_range (buf, i, 0, width);
            break;
    }

    THAW_AND_NOTIFY_CHANGED;
}


void
_moo_term_buffer_erase_in_line (MooTermBuffer  *buf,
                                EraseType       what)
{
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);
    guint width = buf_screen_width (buf);

    g_return_if_fail (what == 0 || what == 1 || what == 2);

    switch (what)
    {
        case ERASE_FROM_CURSOR:
            _moo_term_buffer_erase_range (buf, cursor_row,
                                          cursor_col, width);
            break;

        case ERASE_TO_CURSOR:
            _moo_term_buffer_erase_range (buf, cursor_row,
                                          0, cursor_col + 1);
            break;

        case ERASE_ALL:
            _moo_term_buffer_erase_range (buf, cursor_row,
                                          0, width);
            break;
    }
}


void
_moo_term_buffer_insert_char (MooTermBuffer  *buf,
                              guint           n)
{
    MooTermLine *line;
    guint cursor_col = buf_cursor_col (buf);
    guint cursor_row = buf_cursor_row (buf);

    g_assert (cursor_col < buf_screen_width (buf));

    if (!n || cursor_row > buf->priv->bottom_margin ||
         cursor_row < buf->priv->top_margin)
    {
        return;
    }

    line = buf_screen_line (buf, cursor_row);
    g_assert (_moo_term_line_len (line) == buf_screen_width (buf));

    _moo_term_line_insert_unichar (line, cursor_col, 0, n,
                                   buf->priv->current_attr);
    buf_changed_add_range (buf, cursor_row, cursor_col,
                           buf_screen_width (buf) - cursor_col);
    NOTIFY_CHANGED;
}


void
_moo_term_buffer_cup (MooTermBuffer  *buf,
                      guint           row,
                      guint           col)
{
    if (col >= buf_screen_width (buf))
        col = buf_screen_width (buf) - 1;

    if (buf_get_mode (MODE_DECOM))
    {
        row = CLAMP (row + buf->priv->top_margin,
                     buf->priv->top_margin,
                     buf->priv->bottom_margin);
        _moo_term_buffer_cursor_move_to (buf, row, col);
    }
    else
    {
        _moo_term_buffer_cursor_move_to (buf, row, col);
    }
}


void
_moo_term_buffer_delete_line (MooTermBuffer  *buf,
                              guint           n)
{
    guint cursor = buf->priv->cursor_row + buf->priv->screen_offset;
    guint top = buf->priv->top_margin + buf->priv->screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->screen_offset;
    guint i;

    GdkRectangle changed = {
        0, buf->priv->cursor_row,
        buf->priv->screen_width,
        bottom - cursor + 1
    };

    g_return_if_fail (n != 0);
    g_return_if_fail (top <= cursor && cursor <= bottom);

    /* TODO: scroll the window */

    if (n > bottom - cursor + 1)
        n = bottom - cursor + 1;

    for (i = cursor; i < cursor + n; ++i)
        delete_line (buf, g_ptr_array_index (buf->priv->lines, i), TRUE, TRUE);

    if (n < bottom - cursor + 1)
        memmove (&g_ptr_array_index (buf->priv->lines, cursor),
                  &g_ptr_array_index (buf->priv->lines, cursor + n),
                  (bottom - cursor + 1 - n) * sizeof(gpointer));

    for (i = bottom + 1 - n; i <= bottom; ++i)
        g_ptr_array_index (buf->priv->lines, i) =
                _moo_term_line_new (buf->priv->screen_width,
                                    buf->priv->current_attr);

    buf_changed_add_rect (buf, changed);
    _moo_term_buffer_changed (buf);
}


void
_moo_term_buffer_insert_line (MooTermBuffer  *buf,
                              guint           n)
{
    guint cursor = buf->priv->cursor_row + buf->priv->screen_offset;
    guint top = buf->priv->top_margin + buf->priv->screen_offset;
    guint bottom = buf->priv->bottom_margin + buf->priv->screen_offset;
    guint i;

    GdkRectangle changed = {
        0, buf->priv->cursor_row,
        buf->priv->screen_width,
        buf->priv->bottom_margin - buf->priv->cursor_row + 1
    };

    g_return_if_fail (n != 0);
    g_return_if_fail (top <= cursor && cursor <= bottom);

    /* TODO: scroll the window */

    if (n > bottom - cursor + 1)
        n = bottom - cursor + 1;

    for (i = bottom - n + 1; i <= bottom; ++i)
        delete_line (buf, g_ptr_array_index (buf->priv->lines, i), TRUE, TRUE);

    if (n < bottom - cursor + 1)
        memmove (&g_ptr_array_index (buf->priv->lines, cursor + n),
                 &g_ptr_array_index (buf->priv->lines, cursor),
                 (bottom - cursor + 1 - n) * sizeof(gpointer));

    for (i = cursor; i < cursor + n; ++i)
        g_ptr_array_index (buf->priv->lines, i) =
                _moo_term_line_new (buf->priv->screen_width,
                                    buf->priv->current_attr);

    buf_changed_add_rect (buf, changed);
    _moo_term_buffer_changed (buf);
}


void    moo_term_buffer_set_mode                (MooTermBuffer  *buf,
                                                 guint           mode,
                                                 gboolean        set)
{
    switch (mode)
    {
        case MODE_CA:
            _moo_term_buffer_set_ca_mode (buf, set);
            break;

        /* xterm does this */
        case MODE_DECANM:
            if (set)
            {
                buf->priv->GL[0] = buf->priv->GL[1] = CHARSET_ASCII;
                buf->priv->GL[2] = buf->priv->GL[3] = CHARSET_ASCII;
            }
            buf->priv->modes[mode] = set;
            break;

        /* these do not require anything special, just record the value */
        case MODE_IRM:
        case MODE_LNM:
        case MODE_DECOM:
        case MODE_DECAWM:
        case MODE_REVERSE_WRAPAROUND:   /* TODO*/
            buf->priv->modes[mode] = set;
            break;

        /* these are ignored in the buffer */
        case MODE_SRM:
        case MODE_DECCKM:
        case MODE_DECSCNM:
        case MODE_DECTCEM:
        case MODE_DECNKM:
        case MODE_DECBKM:
        case MODE_DECKPM:
        case MODE_PRESS_TRACKING:
        case MODE_PRESS_AND_RELEASE_TRACKING:
        case MODE_HILITE_MOUSE_TRACKING:
            buf->priv->modes[mode] = set;
            break;

        default:
            g_warning ("%s: unknown mode %d", G_STRLOC, mode);
    }
}


void
_moo_term_buffer_set_ca_mode (MooTermBuffer  *buf,
                              gboolean        set)
{
    buf->priv->modes[MODE_CA] = set;
    _moo_term_buffer_scrollback_changed (buf);
}


static void
set_defaults (MooTermBuffer  *buf)
{
    buf->priv->cursor_col = buf->priv->cursor_row = 0;

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;

    set_default_modes (buf->priv->modes);
    buf->priv->current_attr.mask = 0;
    buf->priv->single_shift = -1;
    buf->priv->GL[0] = buf->priv->GL[2] = buf->priv->GL[3] = CHARSET_ASCII;
    buf->priv->GL[1] = CHARSET_DRAWING;
    buf->priv->current_graph_set = CHARSET_ASCII;
}


void
_moo_term_buffer_reset (MooTermBuffer  *buf)
{
    guint i;

    FREEZE_NOTIFY;

    for (i = 0; i < buf->priv->lines->len; ++i)
        delete_line (buf, g_ptr_array_index (buf->priv->lines, i), TRUE, TRUE);
    g_ptr_array_free (buf->priv->lines, TRUE);

    buf->priv->screen_offset = 0;
    buf->priv->lines = g_ptr_array_sized_new (buf->priv->screen_height);
    for (i = 0; i < buf->priv->screen_height; ++i)
        g_ptr_array_add (buf->priv->lines,
                         _moo_term_line_new (buf->priv->screen_width,
                                             buf->priv->current_attr));

    set_defaults (buf);

    buf_changed_set_all (buf);
    THAW_AND_NOTIFY;
    _moo_term_buffer_scrollback_changed (buf);
}


void
_moo_term_buffer_soft_reset (MooTermBuffer  *buf)
{
    set_default_modes (buf->priv->modes);

    buf->priv->top_margin = 0;
    buf->priv->bottom_margin = buf->priv->screen_height - 1;
    buf->priv->scrolling_region_set = FALSE;

    buf->priv->current_attr.mask = 0;
    buf->priv->single_shift = -1;
    buf->priv->GL[0] = buf->priv->GL[2] = buf->priv->GL[3] = CHARSET_ASCII;
    buf->priv->GL[1] = CHARSET_DRAWING;
    buf->priv->current_graph_set = CHARSET_ASCII;

    buf_changed_set_all (buf);
    _moo_term_buffer_changed (buf);
}


void
_moo_term_buffer_cursor_next_line (MooTermBuffer  *buf,
                                   guint           n)
{
    guint i;

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n ; ++i)
    {
        if (buf->priv->cursor_row + 1 < buf->priv->screen_height)
            _moo_term_buffer_cursor_move_to (buf, buf->priv->cursor_row + 1, 0);
        else
            break;
    }

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_cursor_prev_line (MooTermBuffer  *buf,
                                   guint           n)
{
    guint i;

    _moo_term_buffer_freeze_cursor_notify (buf);

    for (i = 0; i < n ; ++i)
    {
        if (buf->priv->cursor_row > 0)
            _moo_term_buffer_cursor_move_to (buf, buf->priv->cursor_row - 1, 0);
        else
            break;
    }

    _moo_term_buffer_thaw_cursor_notify (buf);
    _moo_term_buffer_cursor_moved (buf);
}


void
_moo_term_buffer_decaln (MooTermBuffer  *buf)
{
    guint i;
    guint width = buf_screen_width (buf);
    guint height = buf_screen_height (buf);

    FREEZE_CHANGED;

    for (i = 0; i < height; ++i)
    {
        MooTermLine *line = buf_screen_line (buf, i);
        g_assert (_moo_term_line_len (line) == width);
        _moo_term_line_set_unichar (line, 0, MOO_TERM_DECALN_CHAR, width,
                                    buf->priv->current_attr);
    }

    buf_changed_set_all (buf);

    THAW_AND_NOTIFY_CHANGED;
}


MooTermLine *
_moo_term_buffer_get_line (MooTermBuffer  *buf,
                           guint           n)
{
    if (buf_get_mode (MODE_CA))
    {
        g_assert (n < buf->priv->screen_height);
        return g_ptr_array_index (buf->priv->lines,
                                  n + buf->priv->screen_offset);
    }
    else
    {
        g_assert (n < buf->priv->screen_offset + buf->priv->screen_height);
        return g_ptr_array_index (buf->priv->lines, n);
    }
}
