/*
 *   mooedit-lua.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-lua.h"
#include "mooedit/mooedit-private.h"
#include <string.h>
#include <glib/gprintf.h>

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
static void add_editor_api  (lua_State *L);

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
    add_editor_api (L);
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

    lua_pop (L, 2); // pop the doc table and globals table
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

static void
return_error (lua_State  *L,
              const char *format,
              ...) G_GNUC_PRINTF (2,3);

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
            return_error (L, "in %s: internal error", func_name);
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
                    return_error (L, "in %s: internal error", func_name);
                }
                in_opt_args = TRUE;
                break;

            default:
                g_critical ("%s: unknown argument symbol %c", G_STRFUNC, *format);
                return_error (L, "in %s: internal error", func_name);
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
        return_error (L, "in %s: not enough arguments (%d required, %d given)",
                      func_name, n_args, lua_gettop (L));
    if (lua_gettop (L) > n_args + n_opt_args)
        return_error (L, "in %s: too many arguments (max %d accepted, %d given)",
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
                    return_error (L, "%s: %d-th argument must be a string or nil, not %s",
                                  func_name, i+1,
                                  lua_typename (L, lua_type (L, i+1)));
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
                    return_error (L, "%s: %d-th argument must be a string, not %s",
                                  func_name, i+1,
                                  lua_typename (L, lua_type (L, i+1)));
                }

                s_dest = va_arg (args, char**);
                *s_dest = (char*) lua_tostring (L, i+1);
                break;

            case 'S':
                if (!lua_isstring (L, i+1) && !lua_isnumber (L, i+1))
                {
                    va_end (args);
                    return_error (L, "%s: %d-th argument must be a string or number, not %s",
                                  func_name, i+1,
                                  lua_typename (L, lua_type (L, i+1)));
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
                    return_error (L, "%s: %d-th argument must be a number, not %s",
                                  func_name, i+1,
                                  lua_typename (L, lua_type (L, i+1)));
                }

                num = lua_tonumber (L, i+1);
                switch (arg_format[i])
                {
                    case 'i':
                        i_dest = va_arg (args, int*);
                        *i_dest = num;
                        break;
                    case 'u':
                        if (num < 0)
                            return_error (L, "%s: %d-th argument must be non-negative",
                                          func_name, i+1);
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

static void
return_error (lua_State  *L,
              const char *format,
              ...)
{
    va_list args;
    char *msg = NULL;

    va_start (args, format);
    g_vasprintf (&msg, format, args);
    va_end (args);

    if (!msg)
        g_critical ("%s: oops", G_STRFUNC);

    lua_pushstring (L, msg ? msg : "ERROR");
    g_free (msg);
    lua_error (L);
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

    if (n < 0)
        return_error (L, "in Backspace: argument must be positive");

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


static int
cfunc_delete (lua_State *L)
{
    int n = 1;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (L, doc);
    parse_args (L, "Delete", "|i", &n);

    if (n < 0)
        return_error (L, "in Delete: argument must be positive");

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
        gtk_text_iter_forward_cursor_positions (&end, n);
        gtk_text_buffer_delete (buffer, &start, &end);
    }

    scroll_to_cursor (doc);
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
    int n;
    GtkTextView *doc;
    char *freeme = NULL;
    const char *insert;

    CHECK_DOC (L, doc);
    parse_args (L, "NewLine", "|i", n);

    if (n < 0)
        return_error (L, "in NewLine: argument must be positive");

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
        const char *s = lua_tostring (L, i);
        gtk_text_buffer_insert (buffer, &start, s, -1);
    }

    scroll_to_cursor (doc);
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
        return_error (L, "in InsertPlaceholder: document does not support placeholders");

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

    text = moo_text_view_get_selection (doc);
    if (text && text[0])
        lua_pushstring (L, text);
    else
        lua_pushnil (L);
    g_free (text);
    return 1;
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
}


/******************************************************************/
/* editor API
 */

static int
cfunc_open (lua_State *L)
{
    gpointer doc;
    MooEdit *new_doc;
    MooEditor *editor;
    MooEditWindow *window;
    const char *filename;
    GET_DATA ();

    parse_args (L, "Open", "s", &filename);

    doc = data->doc;
    editor = MOO_IS_EDIT (data->doc) ? MOO_EDIT(data->doc)->priv->editor : moo_editor_instance ();
    g_return_val_if_fail (MOO_IS_EDITOR (editor), 0);

    window = MOO_IS_EDIT_WINDOW (data->window) ? MOO_EDIT_WINDOW (data->window) : NULL;
    new_doc = moo_editor_open_file (editor, window, data->window, filename, NULL);

    L_RETURN_BOOL (new_doc != NULL);
}

static void
add_editor_api (lua_State *L)
{
    lua_register (L, "Open", cfunc_open);
}
