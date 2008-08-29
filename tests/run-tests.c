#include <mooedit/mooedit-tests.h>
#include <moolua/moolua-tests.h>
#include <mooutils/mooutils-tests.h>
#include <gtk/gtk.h>
#include <stdio.h>

static void
add_tests (void)
{
    moo_test_gobject ();
    moo_test_mooaccel ();
    moo_test_mooutils_fs ();
    moo_test_moo_file_writer ();
    moo_test_mooutils_misc ();

#ifdef __WIN32__
    moo_test_mooutils_win32 ();
#endif

    moo_test_lua ();
    moo_test_mooedit_lua_api ();

    moo_test_key_file ();
    moo_test_editor ();
}

int
main (int argc, char *argv[])
{
    const char *data_dir;
    GOptionContext *ctx;
    GOptionGroup *grp;
    GError *error = NULL;
    MooTestOptions opts = 0;
    gboolean list_only = FALSE;

    GOptionEntry options[] = {
        { "list", 0, 0, G_OPTION_ARG_NONE, &list_only, "List available tests", NULL },
        { NULL, 0, 0, 0, NULL, NULL, NULL }
    };

    g_thread_init (NULL);

#ifdef __WIN32__
    data_dir = "test-data";
#else
    data_dir = SRCDIR "/data";
#endif

    grp = g_option_group_new ("run-tests", "run-tests", "run-tests", NULL, NULL);
    g_option_group_add_entries (grp, options);
    ctx = g_option_context_new ("[TEST_SUITE]");
    g_option_context_set_main_group (ctx, grp);
    g_option_context_add_group (ctx, gtk_get_option_group (TRUE));

    if (!g_option_context_parse (ctx, &argc, &argv, &error))
    {
        g_printerr ("%s\n", error->message);
        exit (EXIT_FAILURE);
    }

    if (argc > 2)
    {
        g_printerr ("invalid arguments\n");
        exit (EXIT_FAILURE);
    }

    if (list_only)
        opts |= MOO_TEST_LIST_ONLY;

    add_tests ();
    moo_test_run_tests (argv[1], data_dir, opts);
    moo_test_cleanup ();

#ifdef __WIN32__
    if (!g_getenv ("WINESERVER"))
    {
        printf ("Done, press Enter...");
        fflush (stdout);
        getchar ();
    }
#endif

    return moo_test_get_result () ? 0 : 1;
}
