/* This file has been generated with opag 0.8.0.  */
/*
 *   medit-app.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#line 15 "../../../medit/medit-app.opag"

#include "medit-credits.h"
#include <moo.h>
#include <mooutils/mooi18n.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mem-debug.h"

#if GTK_CHECK_VERSION(2,8,0) && defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#define TIMESTAMP() (gdk_x11_display_get_user_time (gdk_display_get_default ()))
#else
#define TIMESTAMP() (0)
#endif


typedef enum {
    MODE_SIMPLE,
    MODE_PROJECT
} AppMode;


int _medit_parse_options (const char *const program_name,
                          const int         argc,
                          char **const      argv);

/********************************************************
 * command line parsing code generated by Opag
 * http://www.zero-based.org/software/opag/
 */
#line 122 "medit-app.c"
#ifndef STR_ERR_UNKNOWN_LONG_OPT
# define STR_ERR_UNKNOWN_LONG_OPT   "%s: unrecognized option `--%s'\n"
#endif

#ifndef STR_ERR_LONG_OPT_AMBIGUOUS
# define STR_ERR_LONG_OPT_AMBIGUOUS "%s: option `--%s' is ambiguous\n"
#endif

#ifndef STR_ERR_MISSING_ARG_LONG
# define STR_ERR_MISSING_ARG_LONG   "%s: option `--%s' requires an argument\n"
#endif

#ifndef STR_ERR_UNEXPEC_ARG_LONG
# define STR_ERR_UNEXPEC_ARG_LONG   "%s: option `--%s' doesn't allow an argument\n"
#endif

#ifndef STR_ERR_UNKNOWN_SHORT_OPT
# define STR_ERR_UNKNOWN_SHORT_OPT  "%s: unrecognized option `-%c'\n"
#endif

#ifndef STR_ERR_MISSING_ARG_SHORT
# define STR_ERR_MISSING_ARG_SHORT  "%s: option `-%c' requires an argument\n"
#endif

#define STR_HELP_NEW_APP "\
  -n, --new-app                  Run new instance of application\n"

#define STR_HELP_PID "\
      --pid=PID                  Use existing instance with process id PID\n"

#define STR_HELP_MODE "\
  -m, --mode=[simple|project]    Use specified mode\n"

#define STR_HELP_PROJECT "\
  -p, --project=PROJECT          Open project file PROJECT\n"

#define STR_HELP_LINE "\
  -l, --line=LINE                Open file and position cursor on line LINE\n"

#define STR_HELP_LOG "\
      --log[=FILE]               Show debug output or write it to FILE\n"

#define STR_HELP_DEBUG "\
      --debug                    Run in debug mode\n"

#define STR_HELP_SEND "\
      --send=STRING              Send string to existing instance to execute\n"

#define STR_HELP_SEND_FILE "\
      --send-file=FILE           Send file to existing instance to execute\n"

#define STR_HELP_VERSION "\
      --version                  Display version information and exit\n"

#define STR_HELP_HELP "\
  -h, --help                     Display this help text and exit\n"

#define STR_HELP "\
  -n, --new-app                  Run new instance of application\n\
      --pid=PID                  Use existing instance with process id PID\n\
  -m, --mode=[simple|project]    Use specified mode\n\
  -p, --project=PROJECT          Open project file PROJECT\n\
  -l, --line=LINE                Open file and position cursor on line LINE\n\
      --log[=FILE]               Show debug output or write it to FILE\n\
      --debug                    Run in debug mode\n\
      --send=STRING              Send string to existing instance to execute\n\
      --send-file=FILE           Send file to existing instance to execute\n\
      --version                  Display version information and exit\n\
  -h, --help                     Display this help text and exit\n"

/* Set to 1 if option --new-app (-n) has been specified.  */
char _medit_opt_new_app;

/* Set to 1 if option --pid has been specified.  */
char _medit_opt_pid;

/* Set to 1 if option --mode (-m) has been specified.  */
char _medit_opt_mode;

/* Set to 1 if option --project (-p) has been specified.  */
char _medit_opt_project;

/* Set to 1 if option --line (-l) has been specified.  */
char _medit_opt_line;

/* Set to 1 if option --log has been specified.  */
char _medit_opt_log;

/* Set to 1 if option --debug has been specified.  */
char _medit_opt_debug;

/* Set to 1 if option --send has been specified.  */
char _medit_opt_send;

/* Set to 1 if option --send-file has been specified.  */
char _medit_opt_send_file;

/* Set to 1 if option --version has been specified.  */
char _medit_opt_version;

/* Set to 1 if option --help (-h) has been specified.  */
char _medit_opt_help;

/* Argument to option --pid.  */
const char *_medit_arg_pid;

/* Argument to option --mode (-m).  */
const char *_medit_arg_mode;

/* Argument to option --project (-p).  */
const char *_medit_arg_project;

/* Argument to option --line (-l).  */
const char *_medit_arg_line;

/* Argument to option --log, or a null pointer if no argument.  */
const char *_medit_arg_log;

/* Argument to option --send.  */
const char *_medit_arg_send;

/* Argument to option --send-file.  */
const char *_medit_arg_send_file;

/* Parse command line options.  Return index of first non-option argument,
   or -1 if an error is encountered.  */
int _medit_parse_options (const char *const program_name, const int argc, char **const argv)
{
  static const char *const optstr__new_app = "new-app";
  static const char *const optstr__pid = "pid";
  static const char *const optstr__mode = "mode";
  static const char *const optstr__project = "project";
  static const char *const optstr__line = "line";
  static const char *const optstr__debug = "debug";
  static const char *const optstr__send = "send";
  static const char *const optstr__send_file = "send-file";
  static const char *const optstr__version = "version";
  static const char *const optstr__help = "help";
  int i = 0;
  _medit_opt_new_app = 0;
  _medit_opt_pid = 0;
  _medit_opt_mode = 0;
  _medit_opt_project = 0;
  _medit_opt_line = 0;
  _medit_opt_log = 0;
  _medit_opt_debug = 0;
  _medit_opt_send = 0;
  _medit_opt_send_file = 0;
  _medit_opt_version = 0;
  _medit_opt_help = 0;
  _medit_arg_pid = 0;
  _medit_arg_mode = 0;
  _medit_arg_project = 0;
  _medit_arg_line = 0;
  _medit_arg_log = 0;
  _medit_arg_send = 0;
  _medit_arg_send_file = 0;
  while (++i < argc)
  {
    const char *option = argv [i];
    if (*option != '-')
      return i;
    else if (*++option == '\0')
      return i;
    else if (*option == '-')
    {
      const char *argument;
      size_t option_len;
      ++option;
      if ((argument = strchr (option, '=')) == option)
        goto error_unknown_long_opt;
      else if (argument == 0)
        option_len = strlen (option);
      else
        option_len = argument++ - option;
      switch (*option)
      {
       case '\0':
        return i + 1;
       case 'd':
        if (strncmp (option + 1, optstr__debug + 1, option_len - 1) == 0)
        {
          if (argument != 0)
          {
            option = optstr__debug;
            goto error_unexpec_arg_long;
          }
          _medit_opt_debug = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 'h':
        if (strncmp (option + 1, optstr__help + 1, option_len - 1) == 0)
        {
          if (argument != 0)
          {
            option = optstr__help;
            goto error_unexpec_arg_long;
          }
          _medit_opt_help = 1;
          return i + 1;
        }
        goto error_unknown_long_opt;
       case 'l':
        if (strncmp (option + 1, optstr__line + 1, option_len - 1) == 0)
        {
          if (option_len <= 1)
            goto error_long_opt_ambiguous;
          if (argument != 0)
            _medit_arg_line = argument;
          else if (++i < argc)
            _medit_arg_line = argv [i];
          else
          {
            option = optstr__line;
            goto error_missing_arg_long;
          }
          _medit_opt_line = 1;
          break;
        }
        if (strncmp (option + 1, "og", option_len - 1) == 0)
        {
          if (option_len <= 1)
            goto error_long_opt_ambiguous;
          _medit_arg_log = argument;
          _medit_opt_log = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 'm':
        if (strncmp (option + 1, optstr__mode + 1, option_len - 1) == 0)
        {
          if (argument != 0)
            _medit_arg_mode = argument;
          else if (++i < argc)
            _medit_arg_mode = argv [i];
          else
          {
            option = optstr__mode;
            goto error_missing_arg_long;
          }
          _medit_opt_mode = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 'n':
        if (strncmp (option + 1, optstr__new_app + 1, option_len - 1) == 0)
        {
          if (argument != 0)
          {
            option = optstr__new_app;
            goto error_unexpec_arg_long;
          }
          _medit_opt_new_app = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 'p':
        if (strncmp (option + 1, optstr__pid + 1, option_len - 1) == 0)
        {
          if (option_len <= 1)
            goto error_long_opt_ambiguous;
          if (argument != 0)
            _medit_arg_pid = argument;
          else if (++i < argc)
            _medit_arg_pid = argv [i];
          else
          {
            option = optstr__pid;
            goto error_missing_arg_long;
          }
          _medit_opt_pid = 1;
          break;
        }
        if (strncmp (option + 1, optstr__project + 1, option_len - 1) == 0)
        {
          if (option_len <= 1)
            goto error_long_opt_ambiguous;
          if (argument != 0)
            _medit_arg_project = argument;
          else if (++i < argc)
            _medit_arg_project = argv [i];
          else
          {
            option = optstr__project;
            goto error_missing_arg_long;
          }
          _medit_opt_project = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 's':
        if (strncmp (option + 1, optstr__send + 1, option_len - 1) == 0)
        {
          if (option_len < 4)
            goto error_long_opt_ambiguous;
          if (argument != 0)
            _medit_arg_send = argument;
          else if (++i < argc)
            _medit_arg_send = argv [i];
          else
          {
            option = optstr__send;
            goto error_missing_arg_long;
          }
          _medit_opt_send = 1;
          break;
        }
        if (strncmp (option + 1, optstr__send_file + 1, option_len - 1) == 0)
        {
          if (option_len <= 4)
            goto error_long_opt_ambiguous;
          if (argument != 0)
            _medit_arg_send_file = argument;
          else if (++i < argc)
            _medit_arg_send_file = argv [i];
          else
          {
            option = optstr__send_file;
            goto error_missing_arg_long;
          }
          _medit_opt_send_file = 1;
          break;
        }
        goto error_unknown_long_opt;
       case 'v':
        if (strncmp (option + 1, optstr__version + 1, option_len - 1) == 0)
        {
          if (argument != 0)
          {
            option = optstr__version;
            goto error_unexpec_arg_long;
          }
          _medit_opt_version = 1;
          return i + 1;
        }
       default:
       error_unknown_long_opt:
        fprintf (stderr, STR_ERR_UNKNOWN_LONG_OPT, program_name, option);
        return -1;
       error_long_opt_ambiguous:
        fprintf (stderr, STR_ERR_LONG_OPT_AMBIGUOUS, program_name, option);
        return -1;
       error_missing_arg_long:
        fprintf (stderr, STR_ERR_MISSING_ARG_LONG, program_name, option);
        return -1;
       error_unexpec_arg_long:
        fprintf (stderr, STR_ERR_UNEXPEC_ARG_LONG, program_name, option);
        return -1;
      }
    }
    else
      do
      {
        switch (*option)
        {
         case 'h':
          _medit_opt_help = 1;
          return i + 1;
         case 'l':
          if (option [1] != '\0')
            _medit_arg_line = option + 1;
          else if (++i < argc)
            _medit_arg_line = argv [i];
          else
            goto error_missing_arg_short;
          option = "\0";
          _medit_opt_line = 1;
          break;
         case 'm':
          if (option [1] != '\0')
            _medit_arg_mode = option + 1;
          else if (++i < argc)
            _medit_arg_mode = argv [i];
          else
            goto error_missing_arg_short;
          option = "\0";
          _medit_opt_mode = 1;
          break;
         case 'n':
          _medit_opt_new_app = 1;
          break;
         case 'p':
          if (option [1] != '\0')
            _medit_arg_project = option + 1;
          else if (++i < argc)
            _medit_arg_project = argv [i];
          else
            goto error_missing_arg_short;
          option = "\0";
          _medit_opt_project = 1;
          break;
         default:
          fprintf (stderr, STR_ERR_UNKNOWN_SHORT_OPT, program_name, *option);
          return -1;
         error_missing_arg_short:
          fprintf (stderr, STR_ERR_MISSING_ARG_SHORT, program_name, *option);
          return -1;
        }
      } while (*++option != '\0');
  }
  return i;
}
#line 135 "../../../medit/medit-app.opag"

#undef STR_HELP
#define STR_HELP        \
    STR_HELP_NEW_APP    \
    STR_HELP_PID        \
    STR_HELP_LINE       \
    STR_HELP_LOG        \
    STR_HELP_DEBUG      \
    STR_HELP_VERSION    \
    STR_HELP_HELP

/* end of generated code
 ********************************************************/


static void
usage (void)
{
    g_print ("Usage: %s [OPTIONS] [FILES]\n", g_get_prgname ());
    g_print ("Options:\n%s", STR_HELP);
}

static void
version (void)
{
    g_print ("medit %s\n", MOO_VERSION);
}


static void
check_args (int opt_remain)
{
    if (_medit_opt_help)
    {
        usage ();
        exit (0);
    }

    if (opt_remain < 0)
    {
        usage ();
        exit (1);
    }

    if (_medit_opt_pid)
    {
        if (_medit_opt_mode)
        {
            g_print ("--mode can't be used together with --pid\n");
            exit (1);
        }

        if (_medit_opt_project)
        {
            g_print ("--project can't be used together with --pid\n");
            exit (1);
        }

        if (_medit_opt_new_app)
        {
            g_print ("--new-app can't be used together with --pid\n");
            exit (1);
        }

        if (!_medit_arg_pid || !_medit_arg_pid[0])
        {
            usage ();
            exit (1);
        }
    }

    if (_medit_opt_mode)
    {
        if (!_medit_arg_mode ||
            (strcmp (_medit_arg_mode, "simple") &&
             strcmp (_medit_arg_mode, "project")))
        {
            usage ();
            exit (1);
        }
    }

    if (_medit_opt_project && _medit_arg_mode &&
        strcmp (_medit_arg_mode, "project"))
    {
        usage ();
        exit (1);
    }

    if (_medit_opt_version)
    {
        version ();
        exit (0);
    }
}


static void
push_appdir_to_path (void)
{
#ifdef __WIN32__
    char *appdir;
    const char *path;
    char *new_path;

    appdir = moo_win32_get_app_dir ();
    g_return_if_fail (appdir != NULL);

    path = g_getenv ("Path");

    if (path)
        new_path = g_strdup_printf ("%s;%s", appdir, path);
    else
        new_path = g_strdup (appdir);

    g_setenv ("Path", new_path, TRUE);

    g_free (new_path);
    g_free (appdir);
#endif
}


int
main (int argc, char *argv[])
{
    MooApp *app;
    int opt_remain;
    MooEditor *editor;
    char **files;
    gpointer window;
    int retval;
    gboolean new_instance = FALSE;
    gboolean run_input = TRUE;
    AppMode mode = MODE_SIMPLE;
    guint32 stamp;
    guint32 line = 0;
    const char *pid_string = NULL;

    init_mem_stuff ();

    g_thread_init (NULL);
    gdk_threads_init ();

    gdk_threads_enter ();

    gtk_init (&argc, &argv);
    stamp = TIMESTAMP ();

#if 0
    gdk_window_set_debug_updates (TRUE);
#endif

    opt_remain = _medit_parse_options (g_get_prgname (), argc, argv);
    check_args (opt_remain);

    if (_medit_opt_debug)
        g_setenv ("MOO_DEBUG", "yes", FALSE);

    if (_medit_opt_log)
    {
        if (_medit_arg_log)
            moo_set_log_func_file (_medit_arg_log);
        else
            moo_set_log_func_window (TRUE);
    }

    push_appdir_to_path ();

    if (_medit_opt_mode)
    {
        if (!strcmp (_medit_arg_mode, "simple"))
            mode = MODE_SIMPLE;
        else
            mode = MODE_PROJECT;
    }

    if (_medit_opt_project)
        mode = MODE_PROJECT;

    if (_medit_opt_new_app || mode == MODE_PROJECT)
        new_instance = TRUE;

    files = moo_filenames_from_locale (argv + opt_remain);

    app = g_object_new (MOO_TYPE_APP,
                        "argv", argv,
                        "run-input", run_input,
                        "short-name", "medit",
                        "full-name", "medit",
                        "description", _("medit is a text editor"),
                        "website", "http://mooedit.sourceforge.net/",
                        "website-label", "http://mooedit.sourceforge.net/",
                        "logo", "medit",
                        "credits", THANKS,
                        NULL);

    if (_medit_arg_line)
        line = strtol (_medit_arg_line, NULL, 10);

    if (_medit_opt_pid)
        pid_string = _medit_arg_pid;
    else if (!_medit_opt_new_app)
        pid_string = g_getenv ("MEDIT_PID");

    if (_medit_opt_send || _medit_opt_send_file)
    {
        GString *msg;
        msg = g_string_new (_medit_opt_send ? "p" : "P");
        g_string_append (msg, _medit_opt_send ? _medit_arg_send : _medit_arg_send_file);
        moo_app_send_msg (app, pid_string, msg->str, msg->len + 1);
        exit (0);
    }

    if (pid_string)
    {
        if (moo_app_send_files (app, files, line, stamp, pid_string))
            exit (0);

        g_print ("Could not send files to pid %s\n", pid_string);
        exit (1);
    }

    if ((!new_instance && moo_app_send_files (app, files, line, stamp, NULL)) ||
         !moo_app_init (app))
    {
        gdk_notify_startup_complete ();
        g_strfreev (files);
        g_object_unref (app);
        return 0;
    }

    if (mode == MODE_PROJECT)
    {
        MooPlugin *plugin;

        plugin = moo_plugin_lookup ("ProjectManager");

        if (!plugin)
        {
            g_printerr ("Could not initialize project manager plugin\n");
            exit (1);
        }

        if (_medit_arg_project && *_medit_arg_project)
        {
            char *project = moo_filename_from_locale (_medit_arg_project);
            g_object_set (plugin, "project", _medit_arg_project, NULL);
            g_free (project);
        }

        moo_plugin_set_enabled (plugin, TRUE);
    }

    editor = moo_app_get_editor (app);
    window = moo_editor_new_window (editor);

    if (files && *files)
        moo_app_open_files (app, files, line, stamp);

    g_strfreev (files);

    g_signal_connect_swapped (editor, "all-windows-closed",
                              G_CALLBACK (moo_app_quit), app);

    retval = moo_app_run (app);

    g_object_unref (app);

    gdk_threads_leave ();
    return retval;
}


#if defined(__WIN32__) && !defined(__GNUC__)

#include <windows.h>

int __stdcall
WinMain (HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         char     *lpszCmdLine,
         int       nCmdShow)
{
	return main (__argc, __argv);
}

#endif
