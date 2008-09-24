/*
 *   mooedit-lua.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-lua.h"
#include "mooedit/mooeditor.h"
#include "mooedit/moocommand-exe.h"
#include "mooedit/moousertools.h"
#include "mooutils/moohistorycombo.h"
#include <string.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#define VAR_WINDOW       "window"
#define VAR_DOC          "doc"
#define VAR_FILE         "file"
#define VAR_NAME         "name"
#define VAR_BASE         "base"
#define VAR_DIR          "dir"
#define VAR_EXT          "ext"

typedef struct {
    gpointer doc;
    GtkTextBuffer *buffer;
    GtkWidget *window;
} MooEditLuaData;

static gpointer data_key;

static void add_text_api    (lua_State *L);
static void add_medit_api   (lua_State *L);

static void
unset_data (lua_State *L)
{
    MooEditLuaData *data = NULL;

    lua_pushlightuserdata (L, &data_key);
    lua_gettable (L, LUA_REGISTRYINDEX);

    if (lua_isuserdata (L, -1))
    {
        data = lua_touserdata (L, -1);
        g_free (data);
        lua_pop (L, 1);

        lua_pushlightuserdata (L, &data_key);
        lua_pushnil (L);
        lua_settable (L, LUA_REGISTRYINDEX);
    }
    else
    {
        lua_pop (L, 1);
    }
}

void
_moo_edit_lua_add_api (lua_State *L)
{
    g_return_if_fail (L != NULL);
    add_text_api (L);
    add_medit_api (L);
}


static void
get_extension (const char *string,
               char      **base,
               char      **ext)
{
    char *dot;

    g_return_if_fail (string != NULL);

    dot = strrchr (string, '.');

    if (dot)
    {
        *base = g_strndup (string, dot - string);
        *ext = g_strdup (dot);
    }
    else
    {
        *base = g_strdup (string);
        *ext = g_strdup ("");
    }
}

static void
set_string (lua_State  *L,
            const char *key,
            const char *value)
{
    if (value)
    {
        lua_pushstring (L, value);
        lua_setfield (L, -2, key);
    }
}

static void
set_global_vars (lua_State   *L,
                 GtkTextView *doc)
{
    char *filename = NULL, *basename = NULL;
    char *dirname = NULL, *base = NULL, *ext = NULL;
    int gl_idx;

    lua_getfenv (L, -1);
    gl_idx = lua_gettop (L);
    lua_newtable (L);
    lua_pushvalue (L, -1);
    lua_setfield (L, gl_idx, VAR_DOC);

    if (MOO_IS_EDIT (doc))
    {
        filename = moo_edit_get_filename (MOO_EDIT (doc));

        if (filename)
        {
            basename = g_path_get_basename (filename);

            if (basename)
                get_extension (basename, &base, &ext);

            dirname = g_path_get_dirname (filename);
        }

        set_string (L, VAR_FILE, filename);
        set_string (L, VAR_NAME, basename);
        set_string (L, VAR_BASE, base);
        set_string (L, VAR_DIR, dirname);
        set_string (L, VAR_EXT, ext);

        g_free (base);
        g_free (ext);
        g_free (dirname);
        g_free (basename);
        g_free (filename);
    }

    lua_pop (L, 2); /* pop the doc table and globals table */
}


void
_moo_edit_lua_set_doc (lua_State   *L,
                       GtkTextView *doc)
{
    MooEditLuaData *data;

    g_return_if_fail (L != NULL);
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    data = g_new0 (MooEditLuaData, 1);
    data->doc = doc;
    data->buffer = doc ? gtk_text_view_get_buffer (doc) : NULL;
    data->window = doc ? gtk_widget_get_toplevel (GTK_WIDGET (doc)) : NULL;
    if (!GTK_IS_WINDOW (data->window))
        data->window = NULL;

    lua_pushlightuserdata (L, &data_key);
    lua_pushlightuserdata (L, data);
    lua_settable (L, LUA_REGISTRYINDEX);

    set_global_vars (L, doc);
}

void
_moo_edit_lua_cleanup (lua_State *L)
{
    g_return_if_fail (L != NULL);
    unset_data (L);
}


static MooEditLuaData *
get_data (lua_State *L)
{
    MooEditLuaData *data = NULL;

    lua_pushlightuserdata (L, &data_key);
    lua_gettable (L, LUA_REGISTRYINDEX);

    if (lua_isuserdata (L, -1))
        data = lua_touserdata (L, -1);

    lua_pop (L, 1);
    return data;
}

#define GET_DATA()                          \
    MooEditLuaData *data = get_data (L);    \
    g_return_val_if_fail (data != NULL, 0);

#define MAX_ARGS 16

static void
parse_args_format (lua_State  *L,
                   const char *func_name,
                   const char *format,
                   char       *args,
                   int        *n_argsp,
                   int        *n_opt_argsp)
{
    gboolean in_opt_args = FALSE;
    int n_args = 0, n_opt_args = 0;
    guint i;

    i = 0;
    while (*format)
    {
        if (i > MAX_ARGS)
        {
            g_critical ("%s: too many arguments", G_STRFUNC);
            luaL_error (L, "in %s: internal error", func_name);
        }

        switch (*format)
        {
            case 's':
            case 'S':
            case 'z':
            case 'i':
            case 'u':
            case 'f':
                args[i++] = *format;
                if (in_opt_args)
                    n_opt_args++;
                else
                    n_args++;
                break;

            case '|':
                if (in_opt_args)
                {
                    g_critical ("%s: duplicated | symbol", G_STRFUNC);
                    luaL_error (L, "in %s: internal error", func_name);
                }
                in_opt_args = TRUE;
                break;

            default:
                g_critical ("%s: unknown argument symbol %c", G_STRFUNC, *format);
                luaL_error (L, "in %s: internal error", func_name);
                break;
        }

        format++;
    }

    *n_argsp = n_args;
    *n_opt_argsp = n_opt_args;
}

static void
parse_args (lua_State  *L,
            const char *func_name,
            const char *format,
            ...)
{
    va_list args;
    char arg_format[MAX_ARGS];
    int n_args, n_opt_args;
    int i;

    parse_args_format (L, func_name, format, arg_format, &n_args, &n_opt_args);

    if (lua_gettop (L) < n_args)
        luaL_error (L, "in %s: not enough arguments (%d required, %d given)",
                    func_name, n_args, lua_gettop (L));
    if (lua_gettop (L) > n_args + n_opt_args)
        luaL_error (L, "in %s: too many arguments (max %d accepted, %d given)",
                    func_name, n_args + n_opt_args, lua_gettop (L));

    va_start (args, format);
    i = 0;

    for (i = 0; i < lua_gettop (L); ++i)
    {
        char **s_dest;
        int *i_dest;
        guint *u_dest;
        double *d_dest;
        lua_Number num;

        switch (arg_format[i])
        {
            case 'z':
                if (!lua_isstring (L, i+1) && !lua_isnil (L, i+1))
                {
                    va_end (args);
                    luaL_typerror (L, i+1, "string or nil");
                }

                s_dest = va_arg (args, char**);
                if (lua_isstring (L, i+1))
                    *s_dest = (char*) lua_tostring (L, i+1);
                else
                    *s_dest = NULL;
                break;

            case 's':
                if (!lua_isstring (L, i+1))
                {
                    va_end (args);
                    luaL_typerror (L, i+1, "string");
                }

                s_dest = va_arg (args, char**);
                *s_dest = (char*) lua_tostring (L, i+1);
                break;

            case 'S':
                if (!lua_isstring (L, i+1) && !lua_isnumber (L, i+1))
                {
                    va_end (args);
                    luaL_typerror (L, i+1, "string or number");
                }

                s_dest = va_arg (args, char**);
                *s_dest = (char*) lua_tostring (L, i+1);
                break;

            case 'i':
            case 'u':
            case 'f':
                if (!lua_isnumber (L, i+1))
                {
                    va_end (args);
                    luaL_typerror (L, i+1, "number");
                }

                num = lua_tonumber (L, i+1);
                switch (arg_format[i])
                {
                    case 'i':
                        i_dest = va_arg (args, int*);
                        *i_dest = num;
                        break;
                    case 'u':
                        luaL_argcheck (L, num >= 0, i+1, "must be non-negative");
                        u_dest = va_arg (args, guint*);
                        *u_dest = num;
                        break;
                    case 'f':
                        d_dest = va_arg (args, double*);
                        *d_dest = num;
                        break;
                }
                break;
        }
    }
}


/******************************************************************/
/* text API
 */

#define CHECK_DOC(L, doc)                           \
G_STMT_START {                                      \
    GET_DATA ();                                    \
    if (!data->doc)                                 \
    {                                               \
        lua_pushstring (L, "Document not set");     \
        lua_error (L);                              \
    }                                               \
    doc = data->doc;                                \
} G_STMT_END


static void
scroll_to_cursor (GtkTextView *view)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
    GtkTextMark *insert = gtk_text_buffer_get_insert (buffer);
    gtk_text_view_scroll_mark_onscreen (view, insert);
}


static int
cfunc_backspace (lua_State *L)
{
    int n = 1;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Backspace", "|i", &n);

    luaL_argcheck (L, n >= 0, 1, "must be positive");

    if (!n)
        return 0;

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        gtk_text_buffer_delete (buffer, &start, &end);
        n--;
    }

    if (n)
    {
        gtk_text_iter_backward_cursor_positions (&start, n);
        gtk_text_buffer_delete (buffer, &start, &end);
    }

    scroll_to_cursor (doc);
    return 0;
}


static void
delete_from_cursor (GtkTextView *doc,
                    int          n)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;

    if (!n)
        return;

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        gtk_text_buffer_delete (buffer, &start, &end);
        n--;
    }

    if (n)
    {
        gtk_text_iter_forward_cursor_positions (&end, n);
        gtk_text_buffer_delete (buffer, &start, &end);
    }

    scroll_to_cursor (doc);
}

static int
cfunc_delete (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Delete", "|i", &n);

    luaL_argcheck (L, n > 0, 1, "must be positive");
    delete_from_cursor (doc, n);

    return 0;
}


static void
insert_text (GtkTextView *view,
             const char  *text,
             int          len)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
    gtk_text_buffer_insert_at_cursor (buffer, text, len);
}

static void
insert_and_scroll (GtkTextView *view,
                   const char  *text,
                   int          len)
{
    insert_text (view, text, len);
    scroll_to_cursor (view);
}

static int
cfunc_new_line (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;
    char *freeme = NULL;
    const char *insert;

    CHECK_DOC (L, doc);
    parse_args (L, "NewLine", "|i", &n);

    luaL_argcheck (L, n >= 0, 1, "must be positive");

    if (!n)
        return 0;

    insert = "\n";

    if (n > 1)
        insert = freeme = g_strnfill (n, '\n');

    insert_and_scroll (doc, insert, n);

    g_free (freeme);
    return 0;
}


static void
get_cursor (GtkTextView *doc,
            int         *line,
            int         *col)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    *line = gtk_text_iter_get_line (&iter);
    *col = moo_text_iter_get_visual_line_offset (&iter, 8);
}

static void
move_cursor_vert (GtkTextView *doc,
                  int          n)
{
    int line, col;

    if (!n)
        return;

    get_cursor (doc, &line, &col);

    if (n < 0)
    {
        if (line > 0)
            moo_text_view_move_cursor (doc, MAX (line + n, 0), col, TRUE, FALSE);
    }
    else
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer (doc);
        int line_count = gtk_text_buffer_get_line_count (buffer);
        moo_text_view_move_cursor (doc, MIN (line + n, line_count - 1), col, TRUE, FALSE);
    }

    scroll_to_cursor (GTK_TEXT_VIEW (doc));
}

static int
cfunc_up (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Up", "|i", &n);

    move_cursor_vert (doc, -n);
    return 0;
}

static int
cfunc_down (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Down", "|i", &n);

    move_cursor_vert (doc, n);
    return 0;
}

static int
cfunc_select (lua_State *L)
{
    int n;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Select", "i", &n);

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                      gtk_text_buffer_get_insert (buffer));
    end = start;
    gtk_text_iter_forward_cursor_positions (&end, n);

    gtk_text_buffer_select_range (buffer, &end, &start);
    scroll_to_cursor (doc);

    return 0;
}


static void
move_cursor (GtkTextView *doc,
             int          howmany)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    gtk_text_iter_forward_cursor_positions (&iter, howmany);
    gtk_text_buffer_place_cursor (buffer, &iter);
    scroll_to_cursor (doc);
}

static int
cfunc_left (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Left", "|i", &n);

    move_cursor (doc, -n);
    return 0;
}

static int
cfunc_right (lua_State *L)
{
    int n = 1;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Right", "|i", &n);

    move_cursor (doc, n);
    return 0;
}


static int
cfunc_insert (lua_State *L)
{
    int i;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (L, doc);

    if (!lua_gettop (L))
        return 0;

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    for (i = 1; i <= lua_gettop (L); ++i)
    {
        size_t len;
        const char *s = lua_check_utf8string (L, i, &len);
        gtk_text_buffer_insert (buffer, &start, s, len);
    }

    scroll_to_cursor (doc);
    return 0;
}

static int
cfunc_insert_text (lua_State *L)
{
    int i, n_args;
    int offset;
    GtkTextIter iter;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (L, doc);

    n_args = lua_gettop (L);
    if (n_args < 2)
        return 0;

    offset = lua_tonumber (L, 1);
    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, offset - 1);

    for (i = 2; i <= n_args; ++i)
    {
        size_t len;
        const char *s = lua_check_utf8string (L, i, &len);
        gtk_text_buffer_insert (buffer, &iter, s, len);
    }

    return 0;
}


static int
cfunc_insert_placeholder (lua_State *L)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;
    const char *text = NULL;

    CHECK_DOC (L, doc);
    parse_args (L, "InsertPlaceholder", "|S", &text);

    if (!MOO_IS_TEXT_VIEW (doc))
        luaL_error (L, "in InsertPlaceholder: the document does not support placeholders");

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    moo_text_view_insert_placeholder (MOO_TEXT_VIEW (doc), &start, text);
    scroll_to_cursor (doc);

    return 0;
}


static int
cfunc_selection (lua_State *L)
{
    GtkTextView *doc;
    char *text;

    CHECK_DOC (L, doc);
    parse_args (L, "Selection", "");

    if ((text = moo_text_view_get_selection (doc)) && text[0])
    {
        lua_take_utf8string (L, text);
    }
    else
    {
        lua_pushnil (L);
        g_free (text);
    }

    return 1;
}


static int
cfunc_get_insert (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    CHECK_DOC (L, doc);
    parse_args (L, "GetInsert", "");

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    return 1;
}

static int
cfunc_get_selection_bounds (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    CHECK_DOC (L, doc);
    parse_args (L, "GetSelectionBounds", "");

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_selection_bounds (buffer, &start, &end);

    lua_pushnumber (L, gtk_text_iter_get_offset (&start) + 1);
    lua_pushnumber (L, gtk_text_iter_get_offset (&end) + 1);
    return 2;
}

static int
cfunc_get_line (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int pos = G_MAXINT;

    CHECK_DOC (L, doc);
    parse_args (L, "GetLine", "|i", &pos);

    buffer = gtk_text_view_get_buffer (doc);

    if (pos == G_MAXINT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos - 1);

    lua_pushnumber (L, gtk_text_iter_get_line (&iter) + 1);
    return 1;
}

static int
cfunc_get_pos_at_line (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int line;

    CHECK_DOC (L, doc);
    parse_args (L, "GetPosAtLine", "i", &line);
    luaL_argcheck (L, line > 0, 1, "must be positive");

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_line (buffer, &iter, line - 1);

    lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    return 1;
}

static int
cfunc_line_start (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int pos = G_MAXINT;

    CHECK_DOC (L, doc);
    parse_args (L, "LineStart", "|i", &pos);

    buffer = gtk_text_view_get_buffer (doc);

    if (pos == G_MAXINT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos - 1);

    gtk_text_iter_set_line_offset (&iter, 0);

    lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    return 1;
}

static int
cfunc_line_end (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int pos = G_MAXINT;

    CHECK_DOC (L, doc);
    parse_args (L, "LineEnd", "|i", &pos);

    buffer = gtk_text_view_get_buffer (doc);

    if (pos == G_MAXINT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos - 1);

    if (!gtk_text_iter_ends_line (&iter))
        gtk_text_iter_forward_to_line_end (&iter);

    lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    return 1;
}

static int
cfunc_forward_line (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int pos = G_MAXINT;
    int n_lines = 1;

    CHECK_DOC (L, doc);
    parse_args (L, "ForwardLine", "|ii", &pos, &n_lines);
    luaL_argcheck (L, n_lines > 0, 2, "must be positive");

    buffer = gtk_text_view_get_buffer (doc);

    if (pos == G_MAXINT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos - 1);

    if (gtk_text_iter_forward_line (&iter))
    {
        gtk_text_iter_forward_lines (&iter, n_lines - 1);
        lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    }
    else
    {
        lua_pushnil (L);
    }

    return 1;
}

static int
cfunc_backward_line (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    int pos = G_MAXINT;
    int n_lines = 1;

    CHECK_DOC (L, doc);
    parse_args (L, "BackwardLine", "|ii", &pos, &n_lines);
    luaL_argcheck (L, n_lines > 0, 2, "must be positive");

    buffer = gtk_text_view_get_buffer (doc);

    if (pos == G_MAXINT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                          gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos - 1);

    if (gtk_text_iter_get_line (&iter) != 0)
    {
        gtk_text_iter_backward_lines (&iter, n_lines);
        lua_pushnumber (L, gtk_text_iter_get_offset (&iter) + 1);
    }
    else
    {
        lua_pushnil (L);
    }

    return 1;
}

static int
cfunc_get_text (lua_State *L)
{
    GtkTextView *doc;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    int start_offset, end_offset;
    char *text;

    CHECK_DOC (L, doc);
    parse_args (L, "GetText", "ii", &start_offset, &end_offset);

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_offset (buffer, &start, start_offset - 1);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, end_offset - 1);
    gtk_text_iter_order (&start, &end);

    text = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
    lua_take_utf8string (L, text);

    return 1;
}

static void
delete_range (GtkTextView *doc,
              int          start_offset,
              int          end_offset)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_offset (buffer, &start, start_offset - 1);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, end_offset - 1);
    gtk_text_buffer_delete (buffer, &start, &end);
}

static int
cfunc_delete_text (lua_State *L)
{
    int start, end;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "DeleteText", "ii", &start, &end);

    delete_range (doc, start, end);

    return 0;
}


#define DEFINE_SIGNAL_FUNC(name,sig,Name)   \
static int                                  \
name (lua_State *L)                         \
{                                           \
    GtkTextView *doc;                       \
    CHECK_DOC (L, doc);                     \
    parse_args (L, Name, "");               \
    g_signal_emit_by_name (doc, sig);       \
    return 0;                               \
}

DEFINE_SIGNAL_FUNC(cfunc_cut, "cut-clipboard", "Cut")
DEFINE_SIGNAL_FUNC(cfunc_copy, "copy-clipboard", "Copy")
DEFINE_SIGNAL_FUNC(cfunc_paste, "paste-clipboard", "Paste")

#undef DEFINE_SIGNAL_FUNC

static void
add_text_api (lua_State *L)
{
    lua_register (L, "Cut", cfunc_cut);
    lua_register (L, "Copy", cfunc_copy);
    lua_register (L, "Paste", cfunc_paste);
    lua_register (L, "Backspace", cfunc_backspace);
    lua_register (L, "Delete", cfunc_delete);
    lua_register (L, "Up", cfunc_up);
    lua_register (L, "Down", cfunc_down);
    lua_register (L, "Left", cfunc_left);
    lua_register (L, "Right", cfunc_right);
    lua_register (L, "Selection", cfunc_selection);
    lua_register (L, "Select", cfunc_select);
    lua_register (L, "Insert", cfunc_insert);
    lua_register (L, "InsertPlaceholder", cfunc_insert_placeholder);
    lua_register (L, "NewLine", cfunc_new_line);
    lua_register (L, "InsertText", cfunc_insert_text);
    lua_register (L, "DeleteText", cfunc_delete_text);
    lua_register (L, "GetInsert", cfunc_get_insert);
    lua_register (L, "GetLine", cfunc_get_line);
    lua_register (L, "GetPosAtLine", cfunc_get_pos_at_line);
    lua_register (L, "GetSelectionBounds", cfunc_get_selection_bounds);
    lua_register (L, "GetText", cfunc_get_text);
    lua_register (L, "LineStart", cfunc_line_start);
    lua_register (L, "LineEnd", cfunc_line_end);
    lua_register (L, "ForwardLine", cfunc_forward_line);
    lua_register (L, "BackwardLine", cfunc_backward_line);
}


/******************************************************************/
/* medit module
 */

static int
cfunc_open (lua_State *L)
{
    MooEdit *new_doc;
    MooEditor *editor;
    MooEditWindow *window;
    const char *filename;
    GET_DATA ();

    parse_args (L, "_medit.open", "s", &filename);

    editor = MOO_IS_EDIT (data->doc) ? moo_edit_get_editor (data->doc) : moo_editor_instance ();
    g_return_val_if_fail (MOO_IS_EDITOR (editor), 0);

    window = MOO_IS_EDIT_WINDOW (data->window) ? MOO_EDIT_WINDOW (data->window) : NULL;
    new_doc = moo_editor_open_file (editor, window, data->window, filename, NULL);

    L_RETURN_BOOL (new_doc != NULL);
}

#if 0
static int
zfunc_history_entry (lua_State *L)
{
    int response;
    const char *dialog_text = NULL, *entry_text = NULL, *user_id = NULL;
    GtkWidget *dialog, *entry;

    GET_DATA ();
    parse_args (L, "medit.entry", "|zzz", &entry_text, &user_id, &dialog_text);

    dialog = gtk_dialog_new_with_buttons (NULL,
                                          data->window ? GTK_WINDOW (data->window) : NULL,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                                          NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

    if (dialog_text)
    {
        GtkWidget *label;
        label = gtk_label_new (dialog_text);
        gtk_widget_show (label);
        gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE, FALSE, 0);
    }

    entry = moo_history_combo_new (user_id);
    moo_combo_set_use_button (MOO_COMBO (entry), FALSE);
    gtk_widget_show (entry);
    moo_combo_entry_set_text (MOO_COMBO (entry), entry_text ? entry_text : "");
    moo_combo_entry_set_activates_default (MOO_COMBO (entry), TRUE);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), entry, FALSE, FALSE, 0);

    response = gtk_dialog_run (GTK_DIALOG (dialog));

    if (response == GTK_RESPONSE_OK)
    {
        const char *text = moo_combo_entry_get_text (MOO_COMBO (entry));

        if (text[0])
            moo_history_combo_commit (MOO_HISTORY_COMBO (entry));

        lua_pushstring (L, text);
    }
    else
    {
        lua_pushnil (L);
    }

    gtk_widget_destroy (dialog);

    return 1;
}
#endif

static int
cfunc_run_in_pane (lua_State *L)
{
    const char *cmd_line;
    const char *working_dir;
    GET_DATA ();

    parse_args (L, "_medit.run_in_pane", "sz", &cmd_line, &working_dir);

    _moo_edit_run_in_pane (cmd_line, working_dir, NULL,
                           MOO_EDIT_WINDOW (data->window),
                           MOO_IS_EDIT (data->doc) ? MOO_EDIT (data->doc) : NULL);

    return 0;
}

static int
cfunc_run_async (lua_State *L)
{
    const char *cmd_line, *working_dir;
    GET_DATA ();

    parse_args (L, "_medit.run_async", "sz", &cmd_line, &working_dir);

    _moo_edit_run_async (cmd_line, working_dir, NULL,
                         MOO_EDIT_WINDOW (data->window),
                         MOO_IS_EDIT (data->doc) ? MOO_EDIT (data->doc) : NULL);

    return 0;
}

static int
cfunc_run_sync (lua_State *L)
{
    const char *cmd_line, *working_dir;
    const char *input;
    char *out = NULL, *err = NULL;
    GET_DATA ();

    parse_args (L, "_medit.run_sync", "szz", &cmd_line, &working_dir, &input);

    _moo_edit_run_sync (cmd_line, working_dir, NULL,
                        MOO_EDIT_WINDOW (data->window),
                        MOO_IS_EDIT (data->doc) ? MOO_EDIT (data->doc) : NULL,
                        input, NULL, &out, &err);

    lua_pushstring (L, out ? out : "");
    lua_pushstring (L, err ? err : "");
    g_free (err);
    g_free (out);
    return 2;
}

static int
cfunc_reload_user_tools (lua_State *L)
{
    parse_args (L, "_medit.reload_user_tools", "");
    _moo_edit_load_user_tools ();
    return 0;
}

static void
add_medit_api (lua_State *L)
{
    static const struct luaL_reg meditlib[] = {
        {"open", cfunc_open},
        {"run_in_pane", cfunc_run_in_pane},
        {"run_async", cfunc_run_async},
        {"run_sync", cfunc_run_sync},
        {"reload_user_tools", cfunc_reload_user_tools},
        {NULL, NULL}
    };

    luaL_openlib (L, "_medit", meditlib, 0);
}
