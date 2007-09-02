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
} MooEditLuaData;

static gpointer data_key;

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

static int
do_thing (lua_State *L)
{
    GET_DATA ();
    g_print ("doc: %p\n", data->doc);
    lua_pushnumber (L, 3.14);
    return 1;
}

void
_moo_edit_lua_add_api (lua_State *L)
{
    g_return_if_fail (L != NULL);

    lua_register (L, "do_thing", do_thing);
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


// /******************************************************************/
// /* text API
//  */
//
// enum {
//     FUNC_BACKSPACE,
//     FUNC_DELETE,
//
//     FUNC_UP,
//     FUNC_DOWN,
//     FUNC_LEFT,
//     FUNC_RIGHT,
//
//     FUNC_INSERT,
//     FUNC_INSERT_PLACEHOLDER,
//     FUNC_NEWLINE,
//     FUNC_SELECT,
//     FUNC_SELECTION,
//
//     FUNC_CUT,
//     FUNC_COPY,
//     FUNC_PASTE,
//
//     N_BUILTIN_FUNCS
// };
//
//
// static const char *builtin_func_names[N_BUILTIN_FUNCS] = {
//     "Backspace", "Delete", "Up", "Down", "Left", "Right",
//     "Insert", "InsertPlaceholder", "NewLine", "Select", "Selection",
//     "Cut", "Copy", "Paste"
// };
//
// static MSFunc *builtin_funcs[N_BUILTIN_FUNCS];
//
//
// #define CHECK_DOC(ctx,doc)                          \
// G_STMT_START {                                      \
//     doc = [(MooEditScriptContext*)ctx doc];         \
//                                                     \
//     if (!doc)                                       \
//     {                                               \
//         [ctx setError:MS_ERROR_TYPE                 \
//                      :"Document not set"];          \
//         return NULL;                                \
//     }                                               \
// } G_STMT_END
//
//
// static gboolean
// check_one_arg (MSValue          **args,
//                guint              n_args,
//                MSContext         *ctx,
//                gboolean           nonnegative,
//                int               *dest,
//                int                default_val)
// {
//     int val;
//
//     if (n_args > 1)
//     {
//         [ctx setError:MS_ERROR_TYPE
//                      :"number of args must be zero or one"];
//         return FALSE;
//     }
//
//     if (!n_args)
//     {
//         *dest = default_val;
//         return TRUE;
//     }
//
//     if (!ms_value_get_int (args[0], &val))
//     {
//         [ctx setError:MS_ERROR_TYPE
//                      :"argument must be integer"];
//         return FALSE;
//     }
//
//     if (nonnegative && val < 0)
//     {
//         [ctx setError:MS_ERROR_TYPE
//                      :"argument must be non-negative"];
//         return FALSE;
//     }
//
//     *dest = val;
//     return TRUE;
// }
//
//
// static void
// scroll_to_cursor (GtkTextView *view)
// {
//     GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
//     GtkTextMark *insert = gtk_text_buffer_get_insert (buffer);
//     gtk_text_view_scroll_mark_onscreen (view, insert);
// }
//
//
// static void
// insert_text (GtkTextView *view,
//              const char  *text,
//              int          len)
// {
//     GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
//     gtk_text_buffer_insert_at_cursor (buffer, text, len);
// }
//
//
// static void
// insert_and_scroll (GtkTextView *view,
//                    const char  *text,
//                    int          len)
// {
//     insert_text (view, text, len);
//     scroll_to_cursor (view);
// }
//
//
// static MSValue *
// cfunc_backspace (MSValue  **args,
//                  guint      n_args,
//                  MSContext *ctx)
// {
//     int n;
//     GtkTextIter start, end;
//     GtkTextBuffer *buffer;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
//         return NULL;
//
//     if (!n)
//         return ms_value_none ();
//
//     buffer = gtk_text_view_get_buffer (doc);
//
//     if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
//     {
//         gtk_text_buffer_delete (buffer, &start, &end);
//         n--;
//     }
//
//     if (n)
//     {
//         gtk_text_iter_backward_chars (&start, n);
//         gtk_text_buffer_delete (buffer, &start, &end);
//     }
//
//     scroll_to_cursor (doc);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_delete (MSValue  **args,
//               guint      n_args,
//               MSContext *ctx)
// {
//     int n;
//     GtkTextIter start, end;
//     GtkTextBuffer *buffer;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
//         return NULL;
//
//     if (!n)
//         return ms_value_none ();
//
//     buffer = gtk_text_view_get_buffer (doc);
//
//     if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
//     {
//         gtk_text_buffer_delete (buffer, &start, &end);
//         n--;
//     }
//
//     if (n)
//     {
//         gtk_text_iter_forward_chars (&end, n);
//         gtk_text_buffer_delete (buffer, &start, &end);
//     }
//
//     scroll_to_cursor (doc);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_newline (MSValue  **args,
//                guint      n_args,
//                MSContext *ctx)
// {
//     int n;
//     GtkTextView *doc;
//     char *freeme = NULL;
//     const char *insert;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
//         return NULL;
//
//     if (!n)
//         return ms_value_none ();
//
//     insert = "\n";
//
//     if (n > 1)
//         insert = freeme = g_strnfill (n, '\n');
//
//     insert_and_scroll (doc, insert, n);
//
//     g_free (freeme);
//     return ms_value_none ();
// }
//
//
// static void
// get_cursor (GtkTextView *doc,
//             int         *line,
//             int         *col)
// {
//     GtkTextBuffer *buffer;
//     GtkTextIter iter;
//
//     buffer = gtk_text_view_get_buffer (doc);
//     gtk_text_buffer_get_iter_at_mark (buffer, &iter,
//                                       gtk_text_buffer_get_insert (buffer));
//
//     *line = gtk_text_iter_get_line (&iter);
//     *col = moo_text_iter_get_visual_line_offset (&iter, 8);
// }
//
//
// static MSValue *
// cfunc_up (MSValue  **args,
//           guint      n_args,
//           MSContext *ctx)
// {
//     int line, col, n;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
//         return NULL;
//
//     if (!n)
//         return ms_value_none ();
//
//     get_cursor (doc, &line, &col);
//
//     if (line > 0)
//         moo_text_view_move_cursor (doc, MAX (line - n, 0), col, TRUE, FALSE);
//
//     scroll_to_cursor (GTK_TEXT_VIEW (doc));
//
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_down (MSValue  **args,
//             guint      n_args,
//             MSContext *ctx)
// {
//     int line, col, n, line_count;
//     GtkTextBuffer *buffer;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
//         return NULL;
//
//     if (!n)
//         return ms_value_none ();
//
//     get_cursor (doc, &line, &col);
//     buffer = gtk_text_view_get_buffer (doc);
//     line_count = gtk_text_buffer_get_line_count (buffer);
//
//     moo_text_view_move_cursor (doc, MIN (line + n, line_count - 1), col, TRUE, FALSE);
//     scroll_to_cursor (GTK_TEXT_VIEW (doc));
//
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_select (MSValue   *arg,
//               MSContext *ctx)
// {
//     int n;
//     GtkTextBuffer *buffer;
//     GtkTextIter start, end;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!ms_value_get_int (arg, &n))
//         return [ctx setError:MS_ERROR_TYPE
//                             :"argument must be an integer"];
//
//     buffer = gtk_text_view_get_buffer (doc);
//     gtk_text_buffer_get_iter_at_mark (buffer, &start,
//                                       gtk_text_buffer_get_insert (buffer));
//     end = start;
//     gtk_text_iter_forward_chars (&end, n);
//
//     gtk_text_buffer_select_range (buffer, &end, &start);
//     scroll_to_cursor (doc);
//
//     return ms_value_none ();
// }
//
//
// static void
// move_cursor (GtkTextView *doc,
//              int          howmany)
// {
//     GtkTextBuffer *buffer;
//     GtkTextIter iter;
//
//     buffer = gtk_text_view_get_buffer (doc);
//     gtk_text_buffer_get_iter_at_mark (buffer, &iter,
//                                       gtk_text_buffer_get_insert (buffer));
//
//     gtk_text_iter_forward_chars (&iter, howmany);
//     gtk_text_buffer_place_cursor (buffer, &iter);
//     scroll_to_cursor (doc);
// }
//
//
// static MSValue *
// cfunc_left (MSValue  **args,
//             guint      n_args,
//             MSContext *ctx)
// {
//     int n;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
//         return NULL;
//
//     move_cursor (doc, -n);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_right (MSValue  **args,
//              guint      n_args,
//              MSContext *ctx)
// {
//     int n;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
//         return NULL;
//
//     move_cursor (doc, n);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_insert (MSValue  **args,
//               guint      n_args,
//               MSContext *ctx)
// {
//     guint i;
//     GtkTextIter start, end;
//     GtkTextBuffer *buffer;
//     GtkTextView *doc;
//
//     CHECK_DOC (ctx, doc);
//
//     if (!n_args)
//         return ms_value_none ();
//
//     buffer = gtk_text_view_get_buffer (doc);
//
//     if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
//         gtk_text_buffer_delete (buffer, &start, &end);
//
//     for (i = 0; i < n_args; ++i)
//     {
//         char *s = ms_value_print (args[i]);
//         gtk_text_buffer_insert (buffer, &start, s, -1);
//         g_free (s);
//     }
//
//     scroll_to_cursor (doc);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_insert_placeholder (MSValue  **args,
//                           guint      n_args,
//                           MSContext *ctx)
// {
//     GtkTextIter start, end;
//     GtkTextBuffer *buffer;
//     GtkTextView *doc;
//     char *text = NULL;
//
//     CHECK_DOC (ctx, doc);
//
//     if (n_args > 1)
//         return [ctx setError:MS_ERROR_TYPE
//                             :"number of args must be zero or one"];
//
//     if (!MOO_IS_TEXT_VIEW (doc))
//         return [ctx setError:MS_ERROR_TYPE
//                             :"document does not support placeholders"];
//
//     if (n_args)
//         text = ms_value_print (args[0]);
//
//     buffer = gtk_text_view_get_buffer (doc);
//
//     if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
//         gtk_text_buffer_delete (buffer, &start, &end);
//
//     moo_text_view_insert_placeholder (MOO_TEXT_VIEW (doc), &start, text);
//     scroll_to_cursor (doc);
//
//     g_free (text);
//     return ms_value_none ();
// }
//
//
// static MSValue *
// cfunc_selection (MSContext *ctx)
// {
//     GtkTextView *doc;
//     char *text;
//
//     CHECK_DOC (ctx, doc);
//
//     text = moo_text_view_get_selection (doc);
//     return ms_value_take_string (text);
// }
//
//
// #define DEFINE_CLIP_FUNC(name,sig)      \
// static MSValue *                        \
// name (MSContext *ctx)                   \
// {                                       \
//     GtkTextView *doc;                   \
//     CHECK_DOC (ctx, doc);               \
//     g_signal_emit_by_name (doc, sig);   \
//     return ms_value_none ();            \
// }
//
// DEFINE_CLIP_FUNC(cfunc_cut, "cut-clipboard")
// DEFINE_CLIP_FUNC(cfunc_copy, "copy-clipboard")
// DEFINE_CLIP_FUNC(cfunc_paste, "paste-clipboard")
//
// #undef DEFINE_CLIP_FUNC
//
//
// static void
// init_text_api (void)
// {
//     if (builtin_funcs[0])
//         return;
//
//     builtin_funcs[FUNC_BACKSPACE] = [MSCFunc newVar:cfunc_backspace];
//     builtin_funcs[FUNC_DELETE] = [MSCFunc newVar:cfunc_delete];
//     builtin_funcs[FUNC_INSERT] = [MSCFunc newVar:cfunc_insert];
//     builtin_funcs[FUNC_INSERT_PLACEHOLDER] = [MSCFunc newVar:cfunc_insert_placeholder];
//     builtin_funcs[FUNC_UP] = [MSCFunc newVar:cfunc_up];
//     builtin_funcs[FUNC_DOWN] = [MSCFunc newVar:cfunc_down];
//     builtin_funcs[FUNC_LEFT] = [MSCFunc newVar:cfunc_left];
//     builtin_funcs[FUNC_RIGHT] = [MSCFunc newVar:cfunc_right];
//     builtin_funcs[FUNC_SELECT] = [MSCFunc new1:cfunc_select];
//     builtin_funcs[FUNC_SELECTION] = [MSCFunc new0:cfunc_selection];
//     builtin_funcs[FUNC_NEWLINE] = [MSCFunc newVar:cfunc_newline];
//     builtin_funcs[FUNC_CUT] = [MSCFunc new0:cfunc_cut];
//     builtin_funcs[FUNC_COPY] = [MSCFunc new0:cfunc_copy];
//     builtin_funcs[FUNC_PASTE] = [MSCFunc new0:cfunc_paste];
// }
//
//
// /******************************************************************/
// /* editor API
//  */
//
// enum {
//     FUNC_OPEN,
//     N_BUILTIN_EDITOR_FUNCS
// };
//
//
// static const char *builtin_editor_func_names[N_BUILTIN_EDITOR_FUNCS] = {
//     "Open"
// };
//
// static MSFunc *builtin_editor_funcs[N_BUILTIN_EDITOR_FUNCS];
//
//
// static MSValue *
// cfunc_open (MSValue   *arg,
//             MSContext *ctx)
// {
//     gpointer doc;
//     MooEdit *new_doc;
//     MooEditor *editor;
//     MooEditWindow *window;
//     char *filename;
//
//     g_return_val_if_fail (ctx != nil, NULL);
//
//     doc = [(MooEditScriptContext*)ctx doc];
//     editor = MOO_IS_EDIT (doc) ? MOO_EDIT (doc)->priv->editor : moo_editor_instance ();
//     g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);
//
//     window = MOO_IS_EDIT_WINDOW ([ctx window]) ? MOO_EDIT_WINDOW ([ctx window]) : NULL;
//     filename = ms_value_print (arg);
//
//     new_doc = moo_editor_open_file (editor, window, [ctx window], filename, NULL);
//
//     g_free (filename);
//     return ms_value_bool (new_doc != NULL);
// }
//
//
// static void
// init_editor_api (void)
// {
//     if (builtin_editor_funcs[0])
//         return;
//
//     builtin_editor_funcs[FUNC_OPEN] = [MSCFunc new1:cfunc_open];
// }
//
//
// @implementation MooEditScriptContext (MooEditScriptContextPrivate)
//
// - (void) addTextApi
// {
//     guint i;
//
//     init_text_api ();
//
//     for (i = 0; i < N_BUILTIN_FUNCS; ++i)
//         [self setFunc:builtin_func_names[i] :builtin_funcs[i]];
// }
//
// - (void) addEditorApi
// {
//     guint i;
//
//     init_editor_api ();
//
//     for (i = 0; i < N_BUILTIN_EDITOR_FUNCS; ++i)
//         [self setFunc:builtin_editor_func_names[i] :builtin_editor_funcs[i]];
// }
//
// @end
//
//
// /* -*- objc -*- */
