/*
 *   mooapp/moopythonconsole.c
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

#include <Python.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "mooapp/moopython.h"
#include "mooapp/moopythonconsole.h"
#include "mooutils/moostock.h"
#include "mooutils/moocompat.h"
#include "mooutils/moowin.h"
#include "mooutils/mooentry.h"


#define MAX_HISTORY 500

static void     moo_python_console_class_init   (MooPythonConsoleClass *klass);
static GObject *moo_python_console_constructor  (GType               type,
                                                 guint               n_construct_properties,
                                                 GObjectConstructParam  *construct_properties);
static void     moo_python_console_set_property (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);
static void     moo_python_console_init         (MooPythonConsole   *console);
static void     moo_python_console_finalize     (GObject            *object);

static void     create_gui      (MooPythonConsole   *self);
static void     entry_activate  (MooPythonConsole   *self);
static gboolean key_press_event (GtkEntry           *entry,
                                 GdkEventKey        *event,
                                 MooPythonConsole   *self);

static void     entry_commit    (MooPythonConsole   *self);
static void     history_next    (MooPythonConsole   *self);
static void     history_prev    (MooPythonConsole   *self);

static void     write_in        (const char         *text,
                                 int                 len,
                                 MooPythonConsole   *self);
static void     write_out       (const char         *text,
                                 int                 len,
                                 MooPythonConsole   *self);
static void     write_err       (const char         *text,
                                 int                 len,
                                 MooPythonConsole   *self);

static void     queue_free      (GQueue             *que);
static guint    queue_len       (GQueue             *que);
static gpointer queue_nth       (GQueue             *que,
                                 guint               n);


/* MOO_TYPE_PYTHON_CONSOLE */
G_DEFINE_TYPE (MooPythonConsole, moo_python_console, GTK_TYPE_WINDOW);

enum {
    PROP_0,
    PROP_PYTHON
};

static void moo_python_console_class_init (MooPythonConsoleClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->constructor = moo_python_console_constructor;
    gobject_class->finalize = moo_python_console_finalize;
    gobject_class->set_property = moo_python_console_set_property;

    g_object_class_install_property (gobject_class,
                                     PROP_PYTHON,
                                     g_param_spec_object
                                             ("python",
                                              "python",
                                              "python",
                                              MOO_TYPE_PYTHON,
                                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}


static void moo_python_console_init (MooPythonConsole *console)
{
    console->history = g_queue_new ();
    console->current = 0;
}


static GObject *moo_python_console_constructor (GType                   type,
                                                 guint                   n_props,
                                                 GObjectConstructParam  *props)
{
    MooPythonConsole *console;
    PangoFontDescription *font;
    GtkTextIter iter;

    GObject *obj =
            G_OBJECT_CLASS (moo_python_console_parent_class)->constructor (type, n_props, props);

    console = MOO_PYTHON_CONSOLE (obj);

    create_gui (console);
    moo_window_set_icon_from_stock (GTK_WINDOW (console), MOO_STOCK_APP);

    g_signal_connect (console, "delete-event",
                      G_CALLBACK (gtk_widget_hide_on_delete),
                      NULL);

    moo_python_set_log_func (console->python,
                             (MooPythonLogFunc) write_in,
                             (MooPythonLogFunc) write_out,
                             (MooPythonLogFunc) write_err,
                             console);

    g_signal_connect_swapped (console->entry, "activate",
                              G_CALLBACK (entry_activate),
                              console);
    g_signal_connect (console->entry, "key-press-event",
                      G_CALLBACK (key_press_event),
                      console);

    console->buf = gtk_text_view_get_buffer (console->textview);
    gtk_text_buffer_get_end_iter (console->buf, &iter);
    console->end = gtk_text_buffer_create_mark (console->buf, NULL, &iter, FALSE);
    console->in_tag = gtk_text_buffer_create_tag (console->buf, "input", "foreground", "blue", NULL);
    console->out_tag = gtk_text_buffer_create_tag (console->buf, "output", "foreground", "black", NULL);
    console->err_tag = gtk_text_buffer_create_tag (console->buf, "error", "foreground", "red", NULL);

    font = pango_font_description_from_string ("Courier New 10");
    if (font) {
        gtk_widget_modify_font (GTK_WIDGET (console->textview), font);
        gtk_widget_modify_font (console->entry, font);
        pango_font_description_free (font);
    }

    console->current = 0;

    return obj;
}


static void moo_python_console_finalize       (GObject      *object)
{
    MooPythonConsole *console = MOO_PYTHON_CONSOLE (object);
    queue_free (console->history);
    G_OBJECT_CLASS (moo_python_console_parent_class)->finalize (object);
}


static void moo_python_console_set_property    (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec)
{
    MooPythonConsole *console = MOO_PYTHON_CONSOLE (object);
    switch (prop_id)
    {
        case PROP_PYTHON:
            console->python = g_value_get_object (value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void     entry_activate  (MooPythonConsole  *self)
{
    return entry_commit (self);
}


static void     entry_commit    (MooPythonConsole  *self)
{
    char *s;
    PyObject *res;

    s = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->entry)));
    gtk_entry_set_text (GTK_ENTRY (self->entry), "");

    if (!s || !s[0])
    {
        g_free (s);
        return;
    }

    res = (PyObject*) moo_python_run_string (self->python, s, FALSE);

    if (res && res != Py_None)
    {
        PyObject* str = PyObject_Str (res);

        if (str)
        {
            write_out (PyString_AsString(str), -1, self);
            Py_DECREF (str);
        }
        else
        {
            PyErr_Print ();
        }
    }
    else if (!res)
    {
        PyErr_Print ();
    }

    Py_XDECREF (res);

    if (g_queue_is_empty (self->history) || strcmp (s, g_queue_peek_head (self->history)))
        g_queue_push_tail (self->history, s);
    else
        g_free (s);

    if (queue_len (self->history) >= MAX_HISTORY)
        g_free (g_queue_pop_head (self->history));

    self->current = queue_len (self->history);
}


static void     write_in        (const char         *text_to_write,
                                 int                 len,
                                 MooPythonConsole  *self)
{
    char *text;
    char **lines, **l;
    GString *s;
    GtkTextIter end;

    if (!text_to_write || !text_to_write[0] || len == 0)
        return;

    if (len > 0)
        text = g_strndup (text_to_write, len);
    else
        text = (char*)text_to_write;

    s = g_string_new ("");
    lines = g_strsplit (text, "\n", 0);
    for (l = lines; *l != NULL; ++l)
        if (**l)
            g_string_append_printf (s, ">>> %s\n", *l);

    gtk_text_buffer_get_end_iter (self->buf, &end);
    gtk_text_buffer_insert_with_tags (self->buf, &end,
                                      s->str, s->len,
                                      self->in_tag, NULL);
    gtk_text_buffer_get_end_iter (self->buf, &end);
    gtk_text_buffer_place_cursor (self->buf, &end);
    gtk_text_view_scroll_mark_onscreen (self->textview, self->end);

    g_strfreev (lines);
    g_string_free (s, TRUE);
    if (text != text_to_write)
        g_free (text);
}


static void     write_out       (const char         *text,
                                 int                 len,
                                 MooPythonConsole  *self)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter (self->buf, &end);
    gtk_text_buffer_insert_with_tags (self->buf, &end, text, len, self->out_tag, NULL);
    gtk_text_view_scroll_mark_onscreen (self->textview, self->end);
}

static void     write_err       (const char         *text,
                                 int                 len,
                                 MooPythonConsole  *self)
{
    GtkTextIter end;
    gtk_text_buffer_get_end_iter (self->buf, &end);
    gtk_text_buffer_insert_with_tags (self->buf, &end, text, len, self->err_tag, NULL);
    gtk_text_view_scroll_mark_onscreen (self->textview, self->end);
}


static gboolean key_press_event (G_GNUC_UNUSED GtkEntry           *entry,
                                 GdkEventKey        *event,
                                 MooPythonConsole  *self)
{
    if (event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_MOD1_MASK))
        return FALSE;

    switch (event->keyval) {
        case GDK_Tab:
            return TRUE;

        case GDK_Down:
            history_next (self);
            return TRUE;

        case GDK_Up:
            history_prev (self);
            return TRUE;

        case GDK_Return:
            entry_commit (self);
            return TRUE;

        default:
            return FALSE;
    }
}


static void     history_prev    (MooPythonConsole  *self)
{
    const char *s = "";
    guint hist_size = queue_len (self->history);

    self->current -= 1;
    if (self->current == (guint)-1)
        self->current = hist_size;
    if (self->current != hist_size)
        s = queue_nth (self->history, self->current);

    gtk_entry_set_text (GTK_ENTRY (self->entry), s);
}

static void     history_next    (MooPythonConsole  *self)
{
    const char *s = "";
    guint hist_size = queue_len (self->history);

    self->current += 1;
    if (self->current == hist_size + 1)
        self->current = 0;
    if (self->current != hist_size)
        s = queue_nth (self->history, self->current);

    gtk_entry_set_text (GTK_ENTRY (self->entry), s);
}


static void     create_gui      (MooPythonConsole  *self)
{
    GtkWidget *vbox1;
    GtkWidget *scrolledwindow1;
    GtkWidget *alignment1;
    GtkWidget *hseparator1;

    gtk_window_set_title (GTK_WINDOW (self), "Python");
    gtk_window_set_default_size (GTK_WINDOW (self), 400, 300);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox1);
    gtk_container_add (GTK_CONTAINER (self), vbox1);

    scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolledwindow1);
    gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
    GTK_WIDGET_UNSET_FLAGS (scrolledwindow1, GTK_CAN_FOCUS);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_ETCHED_IN);

    self->textview = GTK_TEXT_VIEW (gtk_text_view_new ());
    gtk_widget_show (GTK_WIDGET (self->textview));
    gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET (self->textview));
    GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (self->textview), GTK_CAN_FOCUS);
    gtk_text_view_set_editable (self->textview, FALSE);
    gtk_text_view_set_wrap_mode (self->textview, GTK_WRAP_WORD);

    alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_widget_show (alignment1);
    gtk_box_pack_start (GTK_BOX (vbox1), alignment1, FALSE, FALSE, 0);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 3, 3, 0, 0);

    hseparator1 = gtk_hseparator_new ();
    gtk_widget_show (hseparator1);
    gtk_container_add (GTK_CONTAINER (alignment1), hseparator1);

    self->entry = moo_entry_new ();
    gtk_widget_show (self->entry);
    gtk_box_pack_start (GTK_BOX (vbox1), self->entry, FALSE, FALSE, 0);
    GTK_WIDGET_SET_FLAGS (self->entry, GTK_CAN_DEFAULT);

    gtk_widget_grab_focus (self->entry);
    gtk_widget_grab_default (self->entry);
}


MooPythonConsole   *moo_python_console_new        (struct _MooPython *python)
{
    return MOO_PYTHON_CONSOLE (g_object_new (MOO_TYPE_PYTHON_CONSOLE,
                                              "python", python,
                                              NULL));
}


static void     queue_free      (GQueue             *que)
{
    GList *l;
    if (!que) return;
    for (l = que->head; l != NULL; l = l->next)
        g_free (l->data);
    g_queue_free (que);
}


static guint    queue_len       (GQueue             *que)
{
    GList *l;
    guint len = 0;
    g_return_val_if_fail (que != NULL, 0);
    for (l = que->head; l != NULL; l = l->next)
        len++;
    return len;
}


static gpointer queue_nth       (GQueue             *que,
                                 guint               n)
{
    GList *l;
    guint i;
    for (i = 0, l = que->head; i < n; ++i, l = l->next) ;
    return l->data;
}
