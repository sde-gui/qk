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

#include "mooedit/mooedit-script.h"


#define VAR_DOC         "doc"
#define DOC_ATTR_FILE   "file"
#define VAR_WINDOW      "window"


static void moo_edit_context_init_api   (MSContext  *ctx);


G_DEFINE_TYPE (MooEditContext, moo_edit_context, MS_TYPE_CONTEXT)

enum {
    PROP_0,
    PROP_DOC
};

static void
moo_edit_context_set_property (GObject        *object,
                               guint           prop_id,
                               const GValue   *value,
                               GParamSpec     *pspec)
{
    MooEditContext *ctx = MOO_EDIT_CONTEXT (object);

    switch (prop_id)
    {
        case PROP_DOC:
            moo_edit_context_set_doc (ctx, g_value_get_object (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_edit_context_get_property (GObject        *object,
                               guint           prop_id,
                               GValue         *value,
                               GParamSpec     *pspec)
{
    MooEditContext *ctx = MOO_EDIT_CONTEXT (object);

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
moo_edit_context_class_init (MooEditContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->set_property = moo_edit_context_set_property;
    gobject_class->get_property = moo_edit_context_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_DOC,
                                     g_param_spec_object ("doc",
                                             "doc",
                                             "doc",
                                             MOO_TYPE_EDIT,
                                             G_PARAM_READWRITE));
}


static void
moo_edit_context_init (MooEditContext *ctx)
{
    moo_edit_context_init_api (MS_CONTEXT (ctx));
}


void
moo_edit_context_set_doc (MooEditContext *ctx,
                          MooEdit        *doc)
{
    MSValue *val;
    GtkWidget *window = NULL;

    g_return_if_fail (MOO_IS_EDIT_CONTEXT (ctx));
    g_return_if_fail (!doc || MOO_IS_EDIT (doc));

    if (doc)
    {
        GtkWidget *toplevel = gtk_widget_get_toplevel (GTK_WIDGET (doc));

        if (GTK_IS_WINDOW (toplevel))
            window = toplevel;
    }

    g_object_set (ctx, "window", window, NULL);

    if (doc)
    {
        val = ms_value_dict ();
        ms_value_dict_set_string (val, DOC_ATTR_FILE,
                                  moo_edit_get_filename (doc));
    }
    else
    {
        val = ms_value_none ();
    }

    ms_context_assign_variable (MS_CONTEXT (ctx), VAR_DOC, val);
    ms_value_unref (val);

    if (moo_python_running ())
    {
        ms_context_assign_py_object (MS_CONTEXT (ctx), VAR_DOC, doc);
        ms_context_assign_py_object (MS_CONTEXT (ctx), VAR_WINDOW, window);
    }

    ctx->doc = doc;
    g_object_notify (G_OBJECT (ctx), "doc");
}


MSContext *
moo_edit_context_new (MooEditWindow *window)
{
    g_return_val_if_fail (!window || MOO_IS_EDIT_WINDOW (window), NULL);

    return g_object_new (MOO_TYPE_EDIT_CONTEXT,
                         "window", window,
                         "doc", window ? moo_edit_window_get_active_doc (window) : NULL,
                         NULL);
}


static void
moo_edit_set_shell_vars (MooCommand     *cmd,
                         MooEditWindow  *window)
{
    MooEdit *doc = window ? moo_edit_window_get_active_doc (window) : NULL;

    if (doc)
        moo_command_set_shell_var (cmd, DOC_ATTR_FILE,
                                   moo_edit_get_filename (doc));
}


void
moo_edit_setup_command (MooCommand     *cmd,
                        MooEditWindow  *window)
{
    MSContext *ctx;

    g_return_if_fail (MOO_IS_COMMAND (cmd));
    g_return_if_fail (!window || MOO_IS_EDIT_WINDOW (window));

    ctx = moo_edit_context_new (window);
    moo_command_set_context (cmd, ctx);
    moo_command_set_py_dict (cmd, ctx->py_dict);
    g_object_unref (ctx);

    moo_edit_set_shell_vars (cmd, window);
}


/******************************************************************/
/* API
 */

enum {
    FUNC_BACKSPACE,
    FUNC_DELETE,

    FUNC_UP,
    FUNC_DOWN,
    FUNC_LEFT,
    FUNC_RIGHT,

    FUNC_INSERT,
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
    "Insert", "NewLine", "Select", "Selection",
    "Cut", "Copy", "Paste"
};

static MSFunc *builtin_funcs[N_BUILTIN_FUNCS];


#define CHECK_DOC(ctx,doc)                          \
G_STMT_START {                                      \
    doc = MOO_EDIT_CONTEXT(ctx)->doc;               \
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
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));

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

    scroll_to_cursor (GTK_TEXT_VIEW (doc));
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
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));

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

    scroll_to_cursor (GTK_TEXT_VIEW (doc));
    return ms_value_none ();
}


static MSValue *
cfunc_newline (MSValue  **args,
               guint      n_args,
               MSContext *ctx)
{
    int n;
    GtkTextBuffer *buffer;
    MooEdit *doc;
    char *freeme = NULL;
    const char *insert;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, TRUE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
    insert = "\n";

    if (n > 1)
        insert = freeme = g_strnfill (n, '\n');

    insert_and_scroll (GTK_TEXT_VIEW (doc), insert, n);

    g_free (freeme);
    return ms_value_none ();
}


static void
get_cursor (MooEdit *doc,
            int     *line,
            int     *col)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    *line = gtk_text_iter_get_line (&iter);
    *col = gtk_text_iter_get_line_offset (&iter);
}


static MSValue *
cfunc_up (MSValue  **args,
          guint      n_args,
          MSContext *ctx)
{
    int line, col, n;
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    get_cursor (doc, &line, &col);

    if (line > 0)
        moo_text_view_move_cursor (MOO_TEXT_VIEW (doc),
                                   MAX (line - n, 0), col,
                                   FALSE);
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
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!check_one_arg (args, n_args, ctx, FALSE, &n, 1))
        return NULL;

    if (!n)
        return ms_value_none ();

    get_cursor (doc, &line, &col);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
    line_count = gtk_text_buffer_get_line_count (buffer);

    moo_text_view_move_cursor (MOO_TEXT_VIEW (doc),
                               MIN (line + n, line_count - 1), col,
                               FALSE);
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
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!ms_value_get_int (arg, &n))
        return ms_context_set_error (MS_CONTEXT (ctx), MS_ERROR_TYPE,
                                     "argument must be integer");

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
    gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                      gtk_text_buffer_get_insert (buffer));
    end = start;
    gtk_text_iter_forward_chars (&end, n);

    gtk_text_buffer_select_range (buffer, &end, &start);
    scroll_to_cursor (GTK_TEXT_VIEW (doc));

    return ms_value_none ();
}


static void
move_cursor (MooEdit *doc,
             int      howmany)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    gtk_text_iter_forward_chars (&iter, howmany);
    gtk_text_buffer_place_cursor (buffer, &iter);
    scroll_to_cursor (GTK_TEXT_VIEW (doc));
}


static MSValue *
cfunc_left (MSValue  **args,
            guint      n_args,
            MSContext *ctx)
{
    int n;
    MooEdit *doc;

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
    MooEdit *doc;

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
    MooEdit *doc;

    CHECK_DOC (ctx, doc);

    if (!n_args)
        return ms_value_none ();

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (doc));

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    for (i = 0; i < n_args; ++i)
    {
        char *s = ms_value_print (args[i]);
        gtk_text_buffer_insert (buffer, &start, s, -1);
        g_free (s);
    }

    scroll_to_cursor (GTK_TEXT_VIEW (doc));
    return ms_value_none ();
}


static MSValue *
cfunc_selection (MSContext *ctx)
{
    MooEdit *doc;
    char *text;

    CHECK_DOC (ctx, doc);

    text = moo_text_view_get_selection (MOO_TEXT_VIEW (doc));
    return ms_value_take_string (text);
}


#define DEFINE_CLIP_FUNC(name,sig)      \
static MSValue *                        \
name (MSContext *ctx)                   \
{                                       \
    MooEdit *doc;                       \
    CHECK_DOC (ctx, doc);               \
    g_signal_emit_by_name (doc, sig);   \
    return ms_value_none ();            \
}

DEFINE_CLIP_FUNC(cfunc_cut, "cut-clipboard")
DEFINE_CLIP_FUNC(cfunc_copy, "copy-clipboard")
DEFINE_CLIP_FUNC(cfunc_paste, "paste-clipboard")


static void
init_api (void)
{
    if (builtin_funcs[0])
        return;

    builtin_funcs[FUNC_BACKSPACE] = ms_cfunc_new_var (cfunc_backspace);
    builtin_funcs[FUNC_DELETE] = ms_cfunc_new_var (cfunc_delete);
    builtin_funcs[FUNC_INSERT] = ms_cfunc_new_var (cfunc_insert);
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
moo_edit_context_init_api (MSContext *ctx)
{
    guint i;

    init_api ();

    for (i = 0; i < N_BUILTIN_FUNCS; ++i)
        ms_context_set_func (ctx, builtin_func_names[i],
                             builtin_funcs[i]);
}
