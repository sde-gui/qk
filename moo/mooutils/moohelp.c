#include "config.h"
#include "moohelp.h"
#include "mooutils-misc.h"
#include "moodialogs.h"
#include <gdk/gdkkeysyms.h>

typedef struct {
    MooHelpFunc func;
    gpointer func_data;
    GDestroyNotify notify;
} MooHelpData;

static MooHelpData *moo_help_get_data       (GtkWidget  *widget);


static MooHelpData *
moo_help_get_data (GtkWidget *widget)
{
    return g_object_get_data (G_OBJECT (widget), "moo-help-data");
}


static gboolean
moo_help_callback_id (GtkWidget *widget,
                      gpointer   data)
{
    moo_help_open_id (data, widget);
    return TRUE;
}

void
moo_help_set_id (GtkWidget  *widget,
                 const char *id)
{
    g_return_if_fail (GTK_IS_WIDGET (widget));

    if (id)
        moo_help_set_func_full (widget, moo_help_callback_id, g_strdup (id), g_free);
    else
        moo_help_set_func (widget, NULL);
}


static void
moo_help_data_free (MooHelpData *data)
{
    if (data)
    {
        if (data->notify)
            data->notify (data->func_data);
        g_free (data);
    }
}

void
moo_help_set_func_full (GtkWidget     *widget,
                        MooHelpFunc    func,
                        gpointer       func_data,
                        GDestroyNotify notify)
{
    MooHelpData *data = NULL;

    g_return_if_fail (GTK_IS_WIDGET (widget));

    if (func)
    {
        data = g_new0 (MooHelpData, 1);
        data->func = func;
        data->func_data = func_data;
        data->notify = notify;
    }

    g_object_set_data_full (G_OBJECT (widget), "moo-help-data", data,
                            (GDestroyNotify) moo_help_data_free);
}

void
moo_help_set_func (GtkWidget   *widget,
                   gboolean (*func) (GtkWidget*))
{
    g_return_if_fail (GTK_IS_WIDGET (widget));
    moo_help_set_func_full (widget, (MooHelpFunc) func, NULL, NULL);
}


gboolean
moo_help_open (GtkWidget *widget)
{
    MooHelpData *data;

    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

    if (!(data = moo_help_get_data (widget)))
        return FALSE;

    return data->func (widget, data->func_data);
}

void
moo_help_open_any (GtkWidget *widget)
{
    while (widget)
    {
        if (moo_help_open (widget))
            return;
        widget = widget->parent;
    }

    return moo_help_open_id (MOO_HELP_ID_CONTENTS, widget);
}


static gboolean
moo_help_key_press (GtkWidget   *widget,
                    GdkEventKey *event)
{
    if (!(event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK | GDK_META_MASK)) &&
        event->keyval == GDK_F1)
            return moo_help_open (widget);
    else
        return FALSE;
}

void
moo_help_connect_keys (GtkWidget *widget)
{
    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_signal_handlers_disconnect_by_func (widget, (gpointer) moo_help_key_press, NULL);
    g_signal_connect (widget, "key-press-event", G_CALLBACK (moo_help_key_press), NULL);
}


static gboolean
try_help_dir (const char *dir)
{
    char *filename = g_build_filename (dir, "index.html", NULL);
    gboolean found_html = g_file_test (filename, G_FILE_TEST_IS_REGULAR);
    g_free (filename);
    return found_html;
}

static const char *
find_help_dir (void)
{
    static gboolean been_here = FALSE;
    static char *help_dir;
    const char *const *dirs;
    const char *const *p;

    if (been_here)
        return help_dir;
    been_here = TRUE;

#ifdef MOO_HELP_DIR
    if (try_help_dir (MOO_HELP_DIR))
    {
        help_dir = (char*) MOO_HELP_DIR;
        return help_dir;
    }
#endif

    dirs = g_get_system_data_dirs ();

    for (p = dirs; p && *p; ++p)
    {
        char *d = g_build_filename (*p, "doc", MOO_PACKAGE_NAME, "help", NULL);

        if (try_help_dir (d))
        {
            help_dir = d;
            return help_dir;
        }

        g_free (d);
    }

    return NULL;
}

static void
warn_no_help (GtkWidget *parent)
{
    static gboolean been_here;

    if (!been_here)
    {
        been_here = TRUE;
        moo_error_dialog (parent, "Help files not found",
                          "Could not find help files, most likely "
                          "it means broken installation");
    }
}

static void
warn_no_help_file (const char *basename,
                   GtkWidget  *parent)
{
    char *dir_utf8;
    char *msg;

    dir_utf8 = g_filename_display_name (find_help_dir ());
    msg = g_strdup_printf ("File '%s' is missing in the directory '%s'",
                           basename, dir_utf8);

    moo_error_dialog (parent, "Could not find help file", msg);

    g_free (msg);
    g_free (dir_utf8);
}

#ifndef __WIN32__
static void
open_html_file (const char *path,
                GtkWidget  *parent)
{
    static gboolean been_here;
    static char *script;

    const char *argv[3];
    GError *err = NULL;

    if (!been_here)
    {
        been_here = TRUE;
        script = _moo_find_script ("moo-open-html-help", FALSE);

        if (script && !g_file_test (script, G_FILE_TEST_IS_EXECUTABLE))
        {
            g_warning ("could not find moo-open-html-help script");
            g_free (script);
            script = NULL;
        }
    }

    if (!script)
    {
        moo_open_file (path);
        return;
    }

    argv[0] = script;
    argv[1] = path;
    argv[2] = NULL;

    if (parent && gtk_widget_has_screen (parent))
        gdk_spawn_on_screen (gtk_widget_get_screen (parent),
                             NULL, (char**) argv, NULL, 0, NULL, NULL,
                             NULL, &err);
    else
        g_spawn_async (NULL, (char**) argv, NULL, 0, NULL, NULL,
                       NULL, &err);

    if (err)
    {
        g_warning ("%s: error in g_spawn_async", G_STRLOC);
        g_warning ("%s: %s", G_STRLOC, err->message);
        g_error_free (err);
    }
}
#else /* __WIN32__ */
static void
open_html_file (const char *path,
                G_GNUC_UNUSED GtkWidget *parent)
{
    moo_open_file (path);
}
#endif /* __WIN32__ */

static void
open_file_by_id (const char *id,
                 GtkWidget  *parent)
{
    const char *dir;
    char *filename, *basename;

    g_return_if_fail (id != NULL);

    if (!(dir = find_help_dir ()))
    {
        warn_no_help (parent);
        return;
    }

    if (!strcmp (id, "contents"))
        id = "index";

    basename = g_strdup_printf ("%s.html", id);
    filename = g_build_filename (dir, basename, NULL);

    if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR))
        warn_no_help_file (basename, parent);
    else
        open_html_file (filename, parent);

    g_free (filename);
    g_free (basename);
}

#ifdef MOO_ENABLE_HELP
void
moo_help_open_id (const char *id,
                  GtkWidget  *parent)
{
    g_return_if_fail (id != NULL);
    g_return_if_fail (!parent || GTK_IS_WIDGET (parent));
    open_file_by_id (id, parent);
}
#else
void
moo_help_open_id (G_GNUC_UNUSED const char *id,
                  G_GNUC_UNUSED GtkWidget *parent)
{
}
#endif
