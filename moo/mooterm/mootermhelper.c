/*
 *   mooterm/mootermhelper.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pty.h"
#define MOOTERM_COMPILATION
#include "mootermhelper.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_UNUSED __attribute__((__unused__))
#else   /* !__GNUC__ */
#define G_GNUC_UNUSED
#endif  /* !__GNUC__ */

#define WRITE_LOG     0
#define WRITE_LOGFILE 1
#define LOGFILE       "c:\\helper.log"

#if WRITE_LOG
#if WRITE_LOGFILE
static void
writelog (const char *format, ...)
{
    FILE *logfile;
    static int been_here;
    va_list args;

    if (been_here)
    {
        logfile = fopen (LOGFILE, "a+");
    }
    else
    {
        logfile = fopen (LOGFILE, "w+");
        been_here = 1;
    }

    if (!logfile)
    {
        printf ("could not open logfile");
        _exit (1);
    }

    va_start (args, format);
    vfprintf (logfile, format, args);
    va_end (args);

    fclose (logfile);
}
#else /* ! WRITE_LOGFILE */
static void
writelog (const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vfprintf (stderr, format, args);
    va_end (args);
}
#endif /* ! WRITE_LOGFILE */
#else /* ! WRITE_LOG */
static void
writelog (G_GNUC_UNUSED const char *format, ...)
{
}
#endif /* WRITE_LOG */


#define BUFSIZE 1024

int master = -1;

#ifdef HAVE_PID_T
pid_t child_pid = -1;
#else /* ! HAVE_PID_T */
int child_pid = -1;
#endif /* ! HAVE_PID_T */


void
doexit (int status)
{
    /* XXX man, it's all stupid (and race conditions, and infinite loops, and crash)! */
    writelog ("helper: exiting\r\n");
    write (master, "\4", 1);
    _vte_pty_close (master);
    close (master);
    sleep (1);
    if (child_pid != -1)
        kill (child_pid, SIGTERM);
    exit (status);
}

static void
sigchld_handler (G_GNUC_UNUSED int sig)
{
    writelog ("helper: child died\r\n");
    doexit (0);
}

static void
sigint_handler (G_GNUC_UNUSED int sig)
{
    writelog ("helper: got SIGINT\r\n");
    writelog ("helper: child pid %d\r\n", child_pid);
    if (master != -1)
        _vte_pty_n_write (master, "\3", 1);
}

static void
sigany_handler (G_GNUC_UNUSED int num)
{
    writelog ("helper: got signal %d\r\n", num);
    doexit (0);
}


#define GAP_ARGS_OFFSET 4

int
main (int argc, char **argv)
{
    int i;
    int status, flags;
    unsigned width, height;
    int echo;
    char *working_dir;

    if (argc < GAP_ARGS_OFFSET + 1 && (argc != 2 || strcmp (argv[1], "--test")))
    {
        printf ("usage: %s <term_width> <term_height> <echo> <binary> [args]\n", argv[0]);
        exit (1);
    }

    writelog ("helper: hi there\r\n");

    for (i = 3; i < 10000; ++i)
        close (i);

    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);

    signal(SIGHUP,  sigany_handler);
    signal(SIGABRT, sigany_handler);
    signal(SIGPIPE, sigany_handler);
    signal(SIGTERM, sigany_handler);

    width = strtol (argv[1], NULL, 10);
    height = strtol (argv[2], NULL, 10);
    echo = !strcmp (argv[3], "y");

    working_dir = getenv (MOO_TERM_HELPER_ENV);

    if (working_dir && strlen (working_dir))
    {
        writelog ("helper: chdir to '%s'\r\n", working_dir);
        chdir (working_dir);
    }
    else
    {
        writelog ("helper: no %s environment variable\r\n", MOO_TERM_HELPER_ENV);
    }

    master = _vte_pty_open (&child_pid, NULL,
                            argv[GAP_ARGS_OFFSET],
                            argv + GAP_ARGS_OFFSET,
                            NULL,
                            width, height,
                            0, 0, 0);

    writelog ("helper: forked child pid %d\r\n", child_pid);
    for (i = GAP_ARGS_OFFSET; argv[i] != NULL; ++i)
        writelog ("%s ", argv[i]);
    writelog ("\r\n");

    if (master == -1)
    {
        writelog ("helper: could not open pty\r\n");
        doexit (1);
    }

    if (_vte_pty_set_echo_input (master, echo) == -1)
        writelog ("error in _vte_pty_set_echo_input\r\n");

    if (waitpid (child_pid, &status, WNOHANG) == -1)
        writelog ("error in waitpid\r\n");

    if ((flags = fcntl (master, F_GETFL)) < 0)
        writelog ("F_GETFL on master\r\n");
    else if (-1 == fcntl (master, F_SETFL, O_NONBLOCK | flags))
        writelog ("F_SETFL on master\r\n");

    if ((flags = fcntl (0, F_GETFL)) < 0)
        writelog ("F_GETFL on stdin\r\n");
    else if (-1 == fcntl (0, F_SETFL, O_NONBLOCK | flags))
        writelog ("F_SETFL on stdin\r\n");

    while (1)
    {
        int res = 0;
        int again = 1;
        char buf[BUFSIZE];
        int num_read;

        struct pollfd fds[] =
        {
            {master, POLLIN | POLLPRI | POLLHUP | POLLERR, 0},
            {0, POLLIN | POLLPRI | POLLHUP | POLLERR, 0}
        };

        while (again)
        {
            res = poll (fds, 2, -1);

            if (res != -1 && res != 1 && res != 2)
            {
                writelog ("helper: poll returned bogus value %d\r\n", res);
                doexit (1);
            }

            if (res == -1)
            {
                switch (errno)
                {
                    case EAGAIN:
                        writelog ("helper: poll error: EAGAIN\r\n");
                        break;
                    case EINTR:
                        writelog ("helper: poll error: EINTR\r\n");
                        break;
                    default:
                        writelog ("helper: poll error\r\n");
                        writelog ("helper: %s\r\n", strerror (errno));
                        doexit (1);
                }
            }
            else
            {
                again = 0;
            }
        }

        if (fds[0].revents & POLLHUP)
        {
            writelog ("helper: got POLLHUP from child\r\n");
            doexit (0);
        }
        if (fds[1].revents & POLLHUP)
        {
            writelog ("helper: got POLLHUP from parent\r\n");
            doexit (0);
        }
        if ((fds[0].revents & POLLERR) &&
            (!errno || (errno != EAGAIN && errno != EINTR)))
        {
            writelog ("helper: got POLLERR from child\r\n");
            doexit (0);
        }
        if ((fds[1].revents & POLLERR) &&
            (!errno || (errno != EAGAIN && errno != EINTR)))
        {
            writelog ("helper: got POLLERR from parent\r\n");
            doexit (0);
        }

        /********** reading child **********/
        if (fds[0].revents & (POLLIN | POLLPRI))
        {
            num_read = read (master, buf, BUFSIZE);

            if (num_read < 1 && errno != EAGAIN && errno != EINTR)
            {
                writelog ("helper: error reading from child\r\n");
                doexit (1);
            }

            if (num_read > 0 && _vte_pty_n_write (1, buf, num_read) != num_read)
            {
                writelog ("helper: error writing to parent\r\n");
                doexit (1);
            }
        }

        /********** reading parent **********/
        if (fds[1].revents & (POLLIN | POLLPRI))
        {
            int count = 0;

            num_read = read (0, buf, BUFSIZE);

            if (num_read < 1 && errno != EAGAIN && errno != EINTR)
            {
                writelog ("helper: error reading from parent\r\n");
                doexit (1);
            }

            while (count < num_read)
            {
                int i;
                char cmd[HELPER_CMD_SIZE];

                for (i = count; i < num_read && buf[i] != HELPER_CHAR_CMD; ++i) ;

                if (i > count &&
                    _vte_pty_n_write (master, buf + count, i - count) != i - count)
                {
                    writelog ("helper: error writing to child\r\n");
                    doexit (1);
                }

                count = i;

                if (count == num_read)
                    break;

                writelog ("helper: got command\r\n");

                if (num_read > count)
                    memcpy (cmd, buf + count, MIN (HELPER_CMD_SIZE, num_read - count));

                if (count + HELPER_CMD_SIZE > num_read)
                {
                    if (_vte_pty_n_read (0, cmd + num_read - count,
                                         HELPER_CMD_SIZE - (num_read - count)) !=
                            HELPER_CMD_SIZE - (num_read - count))
                    {
                        writelog ("helper: error reading command\r\n");
                        doexit (1);
                    }
                }

                count += HELPER_CMD_SIZE;
                assert (cmd[0] == HELPER_CHAR_CMD);

                switch (cmd[1])
                {
                    case HELPER_CMD_OK:
                        writelog ("helper: HELPER_OK\r\n");
                        break;

                    case HELPER_CMD_GOODBYE:
                        writelog ("helper: HELPER_GOODBYE\r\n");
                        doexit (0);
                        break;

                    case HELPER_CMD_SET_SIZE:
                        writelog ("helper: HELPER_SET_SIZE\r\n");
                        {
                            int width, height;

                            width = HELPER_CMD_GET_WIDTH (cmd);
                            height = HELPER_CMD_GET_HEIGHT (cmd);

                            writelog ("helper: SET_SIZE %d, %d\r\n", width, height);

                            if (_vte_pty_set_size (master, width, height) == -1)
                            {
                                writelog ("helper: could not set size\r\n");
                            }
                            else
                            {
                                writelog ("helper: success\r\n", width, height);
                            }
                        }
                        break;

                    case HELPER_CMD_SET_ECHO:
                        writelog ("helper: HELPER_SET_ECHO\r\n");
                        {
                            int echo;

                            echo = HELPER_CMD_GET_ECHO(cmd);

                            writelog ("helper: SET_ECHO %s\r\n", echo ? "true" : "false");

                            if (_vte_pty_set_echo_input (master, echo) == -1)
                            {
                                writelog ("helper: could not set echo mode\r\n");
                            }
                            else
                            {
                                writelog ("helper: success\r\n", width, height);
                            }
                        }
                        break;

                    default:
                        writelog ("helper: unknown command\r\n");
                        break;
                }
            }
        }
    }

    _vte_pty_close (master);
    close (master);
    master = -1;
    writelog ("helper: exit\r\n");
    doexit (0);
}
