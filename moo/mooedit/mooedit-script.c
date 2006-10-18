/*
 *   mooedit-script.c
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
#include "mooedit/mooedit-script.h"
#include "mooedit/mooedit-private.h"
#include <string.h>


#define MS_VAR_WINDOW       "window"
#define MS_VAR_DOC          "doc"
#define MS_VAR_FILE         "file"
#define MS_VAR_NAME         "name"
#define MS_VAR_BASE         "base"
#define MS_VAR_DIR          "dir"
#define MS_VAR_EXT          "ext"


static void add_text_api    (MSContext  *ctx);
static void add_editor_api  (MSContext  *ctx);


G_DEFINE_TYPE (MooEditScriptContext, moo_edit_script_context, MS_TYPE_CONTEXT)

enum {
    PROP_0,
    PROP_DOC
};


static void
moo_edit_script_context_set_property (GObject        *object,
                                      guint           prop_id,
                                      const GValue   *value,
                                      GParamSpec     *pspec)
{
    MooEditScriptContext *ctx = MOO_EDIT_SCRIPT_CONTEXT (object);

    switch (prop_id)
    {
        case PROP_DOC:
            moo_edit_script_context_set_doc (ctx, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_edit_script_context_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
    MooEditScriptContext *ctx = MOO_EDIT_SCRIPT_CONTEXT (object);

    switch (prop_id)
    {
        case PROP_DOC:
            g_value_set_object (value, ctx->doc);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_edit_script_context_class_init (MooEditScriptContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_edit_script_context_set_property;
    gobject_class->get_property = moo_edit_script_context_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc", "doc", "doc",
                                                          GTK_TYPE_TEXT_VIEW,
                                                          G_PARAM_READWRITE));
}


static void
moo_edit_script_context_init (MooEditScriptContext *ctx)
{
    add_text_api (MS_CONTEXT (ctx));
    add_editor_api (MS_CONTEXT (ctx));
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


void
moo_edit_script_context_set_doc (MooEditScriptContext *ctx,
                                 GtkTextView          *doc)
{
    MSValue *val;
    GtkWidget *window = NULL;

    g_return_if_fail (MOO_IS_EDIT_SCRIPT_CONTEXT (ctx));
    g_return_if_fail (!doc || GTK_IS_TEXT_VIEW (doc));

    if (doc)
    {
        window = gtk_widget_get_toplevel (GTK_WIDGET (doc));
        if (!GTK_IS_WINDOW (window))
            window = NULL;
    }

    g_object_set (ctx, "window", window, NULL);

    val = ms_value_dict ();

    if (MOO_IS_EDIT (doc))
    {
        char *dirname = NULL, *base = NULL, *ext = NULL;

        if (moo_edit_get_basename (MOO_EDIT (doc)))
            get_extension (moo_edit_get_basename (MOO_EDIT (doc)), &base, &ext);

        if (moo_edit_get_filename (MOO_EDIT (doc)))
            dirname = g_path_get_dirname (moo_edit_get_filename (MOO_EDIT (doc)));

        ms_value_dict_set_string (val, MS_VAR_FILE,
                                  moo_edit_get_filename (MOO_EDIT (doc)));
        ms_value_dict_set_string (val, MS_VAR_NAME,
                                  moo_edit_get_basename (MOO_EDIT (doc)));
        ms_value_dict_set_string (val, MS_VAR_BASE, base);
        ms_value_dict_set_string (val, MS_VAR_DIR, dirname);
        ms_value_dict_set_string (val, MS_VAR_EXT, ext);

        g_free (base);
        g_free (ext);
        g_free (dirname);
    }
    else
    {
        ms_value_dict_set_string (val, MS_VAR_FILE, NULL);
        ms_value_dict_set_string (val, MS_VAR_NAME, NULL);
        ms_value_dict_set_string (val, MS_VAR_BASE, NULL);
        ms_value_dict_set_string (val, MS_VAR_DIR, NULL);
        ms_value_dict_set_string (val, MS_VAR_EXT, NULL);
    }

    ms_context_assign_variable (MS_CONTEXT (ctx), MS_VAR_DOC, val);
    ms_value_unref (val);

    ctx->doc = doc;
    g_object_notify (G_OBJECT (ctx), "doc");
}


MSContext *
moo_edit_script_context_new (GtkTextView *doc,
                             GtkWindow   *window)
{
    g_return_val_if_fail (!window || GTK_IS_WINDOW (window), NULL);
    g_return_val_if_fail (!doc || GTK_IS_TEXT_VIEW (doc), NULL);

    if (window)
        return g_object_new (MOO_TYPE_EDIT_SCRIPT_CONTEXT,
                             "window", window,
                             "doc", doc,
                             NULL);
    else
        return g_object_new (MOO_TYPE_EDIT_SCRIPT_CONTEXT,
                             "doc", doc,
                             NULL);
}


#if 0
// MSContext *
// moo_text_context_new (GtkTextView *doc)
// {
//     GtkWidget *window;
//     MSContext *ctx;
//
//     g_return_val_if_fail (GTK_IS_TEXT_VIEW (doc), NULL);
//
//     if (MOO_IS_EDIT (doc))
//         return moo_edit_script_context_new (MOO_EDIT (doc), NULL);
//
//     window = gtk_widget_get_toplevel (GTK_WIDGET (doc));
//
//     ctx = g_object_new (MS_TYPE_CONTEXT,
//                         "window", GTK_IS_WINDOW (window) ? window : NULL,
//                         NULL);
//
//     moo_edit_script_context_init_text_api (ctx);
//
//     return ctx;
// }
//
//
// static void
// moo_edit_set_shell_vars (MooCommand     *cmd,
//                          MooEdit        *doc,
//                          MooEditWindow  *window)
// {
//     if (doc)
//     {
//         char *dirname = NULL, *base = NULL, *ext = NULL;
//
//         if (moo_edit_get_filename (doc))
//             dirname = g_path_get_dirname (moo_edit_get_filename (doc));
//
//         if (moo_edit_get_basename (doc))
//             get_extension (moo_edit_get_basename (doc), &base, &ext);
//
//         moo_command_set_shell_var (cmd, MS_VAR_FILE,
//                                    moo_edit_get_filename (doc));
//         moo_command_set_shell_var (cmd, MS_VAR_NAME,
//                                    moo_edit_get_basename (doc));
//         moo_command_set_shell_var (cmd, MS_VAR_BASE, base);
//         moo_command_set_shell_var (cmd, MS_VAR_EXT, ext);
//         moo_command_set_shell_var (cmd, MS_VAR_DIR, dirname);
//
//         g_free (base);
//         g_free (ext);
//         g_free (dirname);
//     }
//     else
//     {
//         moo_command_set_shell_var (cmd, MS_VAR_FILE, NULL);
//         moo_command_set_shell_var (cmd, MS_VAR_BASE, NULL);
//         moo_command_set_shell_var (cmd, MS_VAR_EXT, NULL);
//         moo_command_set_shell_var (cmd, MS_VAR_DIR, NULL);
//     }
// }
//
//
// void
// moo_edit_setup_command (MooCommand     *cmd,
//                         MooEdit        *doc,
//                         MooEditWindow  *window)
// {
//     MSContext *ctx;
//
//     g_return_if_fail (MOO_IS_COMMAND (cmd));
//     g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));
//     g_return_if_fail (!doc || MOO_IS_EDIT (doc));
//
//     if (!doc && window)
//         doc = moo_edit_window_get_active_doc (window);
//
//     ctx = moo_edit_script_context_new (doc, window);
//
//     moo_command_set_context (cmd, ctx);
//
//     if (ctx->py_dict)
//         moo_command_set_py_dict (cmd, ctx->py_dict);
//
//     g_object_unref (ctx);
//
//     moo_edit_set_shell_vars (cmd, doc, window);
// }
#endif


/******************************************************************/
/* text API
 */

enum {
    FUNC_BACKSPACE,
    FUNC_DELETE,

    FUNC_UP,
    FUNC_DOWN,
    FUNC_LEFT,
    FUNC_RIGHT,

    FUNC_INSERT,
    FUNC_INSERT_PLACEHOLDER,
    FUNC_NEWLINE,
    FUNC_SELECT,
    FUNC_SELECTION,

    FUNC_CUT,
    FUNC_COPY,
    FUNC_PASTE,

    N_BUILTIN_FUNCS
};


static const char *builtin_func_names[N_BUILTIN_FUNCS] = {
    "Backspace", "Delete", "Up", "Down", "Left", "Right",
    "Insert", "InsertPlaceholder", "NewLine", "Select", "Selection",
    "Cut", "Copy", "Paste"
};

static MSFunc *builtin_funcs[N_BUILTIN_FUNCS];


#define CHECK_DOC(ctx,doc)                          \
G_STMT_START {                                      \
    doc = MOO_EDIT_SCRIPT_CONTEXT(ctx)->doc;        \
                                                    \
    if (!doc)                                       \
    {                                               \
        ms_context_set_error (MS_CONTEXT (ctx),     \
                              MS_ERROR_TYPE,        \
                              "Document not set");  \
        return NULL;                                \
    }                                               \
} G_STMT_END


static gboolean
check_one_arg (MSValue          **args,
               guint              n_args,
               MSContext         *ctx,
               gboolean           nonnegative,
               int               *dest,
               int                default_val)
{
    int val;

    if (n_args > 1)
    {
        ms_context_set_error (ctx, MS_ERROR_TYPE,
                              "number of args must be zero or one");
        return FALSE;
    }

    if (!n_args)
    {
        *dest = default_val;
        return TRUE;
    }

    if (!ms_value_get_int (args[0], &val))
    {
        ms_context_set_error (ctx, MS_ERROR_TYPE,
                              "argument must be integer");
        return FALSE;
    }

    if (nonnegative && val < 0)
    {
        ms_context_set_error (ctx, MS_ERROR_VALUE,
                              "argument must be non-negative");
        return FALSE;
    }

    *dest = val;
    return TRUE;
}


static void
scroll_to_cursor (GtkTextView *view)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
    GtkTextMark *insert = gtk_text_buffer_get_insert (buffer);
    gtk_text_view_scroll_mark_onscreen (view, insert);
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


static MSValue *
cfunc_backspace (MSValue  **args,
                 guint      n_args,
                 MSContext *ctx)
{
    int n;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        gtk_text_buffer_delete (buffer, &start, &end);
        n--;
    }

    if (n)
    {
        gtk_text_iter_backward_chars (&start, n);
        gtk_text_buffer_delete (buffer, &start, &end);
    }

    scroll_to_cursor (doc);
    return ms_value_none ();
}


static MSValue *
cfunc_delete (MSValue  **args,
              guint      n_args,
              MSContext *ctx)
{
    int n;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        gtk_text_buffer_delete (buffer, &start, &end);
        n--;
    }

    if (n)
    {
        gtk_text_iter_forward_chars (&end, n);
        gtk_text_buffer_delete (buffer, &start, &end);
    }

    scroll_to_cursor (doc);
    return ms_value_none ();
}


static MSValue *
cfunc_newline (MSValue  **args,
               guint      n_args,
               MSContext *ctx)
{
    int n;
    GtkTextBuffer *buffer;
    GtkTextView *doc;
    char *freeme = NULL;
    const char *insert;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (doc);
    insert = "\n";

    if (n > 1)
        insert = freeme = g_strnfill (n, '\n');

    insert_and_scroll (doc, insert, n);

    g_free (freeme);
    return ms_value_none ();
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


static MSValue *
cfunc_up (MSValue  **args,
          guint      n_args,
          MSContext *ctx)
{
    int line, col, n;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    get_cursor (doc, &line, &col);

    if (line > 0)
        moo_text_view_move_cursor (doc, MAX (line - n, 0), col, TRUE, FALSE);

    scroll_to_cursor (GTK_TEXT_VIEW (doc));

    return ms_value_none ();
}


static MSValue *
cfunc_down (MSValue  **args,
            guint      n_args,
            MSContext *ctx)
{
    int line, col, n, line_count;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    get_cursor (doc, &line, &col);
    buffer = gtk_text_view_get_buffer (doc);
    line_count = gtk_text_buffer_get_line_count (buffer);

    moo_text_view_move_cursor (doc, MIN (line + n, line_count - 1), col, TRUE, FALSE);
    scroll_to_cursor (GTK_TEXT_VIEW (doc));

    return ms_value_none ();
}


static MSValue *
cfunc_select (MSValue   *arg,
              MSContext *ctx)
{
    int n;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!ms_value_get_int (arg, &n))
        return ms_context_set_error (MS_CONTEXT (ctx), MS_ERROR_TYPE,
                                     "argument must be integer");

    buffer = gtk_text_view_get_buffer (doc);
    gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                      gtk_text_buffer_get_insert (buffer));
    end = start;
    gtk_text_iter_forward_chars (&end, n);

    gtk_text_buffer_select_range (buffer, &end, &start);
    scroll_to_cursor (doc);

    return ms_value_none ();
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

    gtk_text_iter_forward_chars (&iter, howmany);
    gtk_text_buffer_place_cursor (buffer, &iter);
    scroll_to_cursor (doc);
}


static MSValue *
cfunc_left (MSValue  **args,
            guint      n_args,
            MSContext *ctx)
{
    int n;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    move_cursor (doc, -n);
    return ms_value_none ();
}


static MSValue *
cfunc_right (MSValue  **args,
             guint      n_args,
             MSContext *ctx)
{
    int n;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    move_cursor (doc, n);
    return ms_value_none ();
}


static MSValue *
cfunc_insert (MSValue  **args,
              guint      n_args,
              MSContext *ctx)
{
    guint i;
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;

    CHECK_DOC (ctx, doc);

    if (!n_args)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    for (i = 0; i < n_args; ++i)
    {
        char *s = ms_value_print (args[i]);
        gtk_text_buffer_insert (buffer, &start, s, -1);
        g_free (s);
    }

    scroll_to_cursor (doc);
    return ms_value_none ();
}


static MSValue *
cfunc_insert_placeholder (MSValue  **args,
                          guint      n_args,
                          MSContext *ctx)
{
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    GtkTextView *doc;
    char *text = NULL;

    CHECK_DOC (ctx, doc);

    if (n_args > 1)
        return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                     "number of args must be zero or one");

    if (!MOO_IS_TEXT_VIEW (doc))
        return ms_context_set_error (ctx, MS_ERROR_TYPE,
                                     "document does not support placeholders");

    if (n_args)
        text = ms_value_print (args[0]);

    buffer = gtk_text_view_get_buffer (doc);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    moo_text_view_insert_placeholder (MOO_TEXT_VIEW (doc), &start, text);
    scroll_to_cursor (doc);

    g_free (text);
    return ms_value_none ();
}


static MSValue *
cfunc_selection (MSContext *ctx)
{
    GtkTextView *doc;
    char *text;

    CHECK_DOC (ctx, doc);

    text = moo_text_view_get_selection (doc);
    return ms_value_take_string (text);
}


#define DEFINE_CLIP_FUNC(name,sig)      \
static MSValue *                        \
name (MSContext *ctx)                   \
{                                       \
    GtkTextView *doc;                   \
    CHECK_DOC (ctx, doc);               \
    g_signal_emit_by_name (doc, sig);   \
    return ms_value_none ();            \
}

DEFINE_CLIP_FUNC(cfunc_cut, "cut-clipboard")
DEFINE_CLIP_FUNC(cfunc_copy, "copy-clipboard")
DEFINE_CLIP_FUNC(cfunc_paste, "paste-clipboard")

#undef DEFINE_CLIP_FUNC


static void
init_text_api (void)
{
    if (builtin_funcs[0])
        return;

    builtin_funcs[FUNC_BACKSPACE] = ms_cfunc_new_var (cfunc_backspace);
    builtin_funcs[FUNC_DELETE] = ms_cfunc_new_var (cfunc_delete);
    builtin_funcs[FUNC_INSERT] = ms_cfunc_new_var (cfunc_insert);
    builtin_funcs[FUNC_INSERT_PLACEHOLDER] = ms_cfunc_new_var (cfunc_insert_placeholder);
    builtin_funcs[FUNC_UP] = ms_cfunc_new_var (cfunc_up);
    builtin_funcs[FUNC_DOWN] = ms_cfunc_new_var (cfunc_down);
    builtin_funcs[FUNC_LEFT] = ms_cfunc_new_var (cfunc_left);
    builtin_funcs[FUNC_RIGHT] = ms_cfunc_new_var (cfunc_right);
    builtin_funcs[FUNC_SELECT] = ms_cfunc_new_1 (cfunc_select);
    builtin_funcs[FUNC_SELECTION] = ms_cfunc_new_0 (cfunc_selection);
    builtin_funcs[FUNC_NEWLINE] = ms_cfunc_new_var (cfunc_newline);
    builtin_funcs[FUNC_CUT] = ms_cfunc_new_0 (cfunc_cut);
    builtin_funcs[FUNC_COPY] = ms_cfunc_new_0 (cfunc_copy);
    builtin_funcs[FUNC_PASTE] = ms_cfunc_new_0 (cfunc_paste);
}


static void
add_text_api (MSContext *ctx)
{
    guint i;

    init_text_api ();

    for (i = 0; i < N_BUILTIN_FUNCS; ++i)
        ms_context_set_func (ctx, builtin_func_names[i],
                             builtin_funcs[i]);
}


/******************************************************************/
/* editor API
 */

enum {
    FUNC_OPEN,
    N_BUILTIN_EDITOR_FUNCS
};


static const char *builtin_editor_func_names[N_BUILTIN_EDITOR_FUNCS] = {
    "Open"
};

static MSFunc *builtin_editor_funcs[N_BUILTIN_EDITOR_FUNCS];


static MSValue *
cfunc_open (MSValue   *arg,
            MSContext *ctx)
{
    gpointer doc;
    MooEdit *new_doc;
    MooEditor *editor;
    MooEditWindow *window;
    char *filename;

    g_return_val_if_fail (MOO_IS_EDIT_SCRIPT_CONTEXT (ctx), NULL);

    doc = MOO_EDIT_SCRIPT_CONTEXT(ctx)->doc;
    editor = MOO_IS_EDIT (doc) ? MOO_EDIT (doc)->priv->editor : moo_editor_instance ();
    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    window = MOO_IS_EDIT_WINDOW (ctx->window) ? MOO_EDIT_WINDOW (ctx->window) : NULL;
    filename = ms_value_print (arg);

    new_doc = moo_editor_open_file (editor, window, ctx->window, filename, NULL);

    g_free (filename);
    return ms_value_bool (new_doc != NULL);
}


static void
init_editor_api (void)
{
    if (builtin_editor_funcs[0])
        return;

    builtin_editor_funcs[FUNC_OPEN] = ms_cfunc_new_1 (cfunc_open);
}


static void
add_editor_api (MSContext *ctx)
{
    guint i;

    init_editor_api ();

    for (i = 0; i < N_BUILTIN_EDITOR_FUNCS; ++i)
        ms_context_set_func (ctx, builtin_editor_func_names[i],
                             builtin_editor_funcs[i]);
}
