#include "manager.h"
#include "project.h"
#include "project-plugin.h"
#include "mpfile.h"
#include <mooedit/mooeditor.h>
#include <mooutils/moofiledialog.h>


@interface MPManager(Private)
- (void) doOpenProject: (CSTR) file
                 error: (GError**) error;
- (void) doCloseProject;
@end


@implementation MPManager

- (void) dealloc
{
    g_free (filename);
    filename = NULL;
    [super dealloc];
}

- (void) deinit
{
    if (project)
        [self doCloseProject];
}

- (void) attachWindow: (MooEditWindow*) win
{
    MooEditor *editor = moo_editor_instance ();
    GtkAction *action;

    window = win;

    moo_editor_set_app_name (editor, project ? [project name] : NULL);
    moo_objc_signal_connect (window, "close", self, @selector(onCloseWindow));

    if ((action = moo_window_get_action (MOO_WINDOW (window), "CloseProject")))
        g_object_set (action, "sensitive", project != NULL, NULL);
    if ((action = moo_window_get_action (MOO_WINDOW (window), "ProjectOptions")))
        g_object_set (action, "visible", project != NULL, NULL);

    if (!project)
    {
        const char *path = moo_prefs_get_string ("Plugins/Project/last");

        if (path && g_file_test (path, G_FILE_TEST_EXISTS))
            [self doOpenProject:path error:NULL];
    }
}

- (void) detachWindow: (MooEditWindow*) win
{
    g_return_if_fail (win == window);
    window = NULL;
}

- (void) openProject: (CSTR) file
{
    if (!window)
    {
        g_free (filename);
        filename = g_strdup (file);
    }

    if (!file)
        file = moo_file_dialogp (window ? GTK_WIDGET (window) : NULL,
                                 MOO_FILE_DIALOG_OPEN,
                                 NULL, "Open Project",
                                 "Plugins/Project/last_dir", NULL);

    if (!file)
        return;

    if (project)
        [self closeProject];
    if (project)
        return;

    [self doOpenProject:file error:NULL];
}

- (void) closeProject
{
}

- (void) projectOptions
{
}

@end

@implementation MPManager(Private)

- (void) initProjectTypes
{
}

- (void) deinitProjectTypes
{
}

- (void) doOpenProject: (CSTR) file
                 error: (GError**) error
{
    g_return_if_fail (project == nil);
    g_return_if_fail (file != NULL);
    g_return_if_fail (!error || !*error);

    MPFile *pf = [MPFile open:file error:error];
    if (!pf)
        return;

    if ((project = [MPProject loadFile:pf error:error]))
    {
        moo_editor_set_app_name (moo_editor_instance (), [project name]);
        moo_history_list_add_filename (moo_history_list_get (MP_RECENT_LIST_ID), file);
        moo_prefs_set_filename ("Plugins/Project/last", file);

        if (window)
        {
            GtkAction *close = moo_window_get_action (MOO_WINDOW (window), MP_ACTION_CLOSE_PROJECT);
            GtkAction *options = moo_window_get_action (MOO_WINDOW (window), MP_ACTION_PROJECT_OPTIONS);
            if (close)
                g_object_set (close, "sensitive", TRUE, NULL);
            if (options)
                g_object_set (options, "visible", TRUE, NULL);
        }
    }
    else
    {
        moo_history_list_remove (moo_history_list_get (MP_RECENT_LIST_ID), file);
    }

    [pf release];
}

// - (void) doCloseProject;

@end


// -*- objc -*-
