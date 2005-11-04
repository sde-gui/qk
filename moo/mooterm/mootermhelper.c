/*
 *   mooterm/mootermhelper.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __CYGWIN__
#include <windows.h>
#endif /* __CYGWIN__ */
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
#include "pty.h"
#define MOOTERM_COMPILATION
#include "mootermhelper.h"


#define WRITE_LOG     0
#define WRITE_LOGFILE 1
#define LOGFILE       "helper.log"

#if WRITE_LOG
#if WRITE_LOGFILE
static int firsttime = 1;
#define writelog(...) \
{\
    FILE *logfile;\
    if (firsttime) {\
        logfile = fopen (LOGFILE, "w+");\
        firsttime = 0;\
    }\
    else\
        logfile = fopen (LOGFILE, "a+");\
    if (!logfile) {\
        printf ("could not open logfile");\
        _exit (1);\
    }\
    fprintf (logfile, __VA_ARGS__);\
    fclose (logfile);\
}
#else /* ! WRITE_LOGFILE */
#define writelog(...) \
{\
    fprintf (stderr, __VA_ARGS__);\
}
#endif /* ! WRITE_LOGFILE */
#else /* ! WRITE_LOG */
#define writelog(...)
#endif /* WRITE_LOG */


#define BUFSIZE 1024

int master = -1;

#ifdef HAVE_PID_T
pid_t child_pid = -1;
#else /* ! HAVE_PID_T */
int child_pid = -1;
#endif /* ! HAVE_PID_T */


void doexit (int status)
{
    writelog ("helper: exiting\r\n");
    write (master, "\4", 1);
    _vte_pty_close (master);
    close (master);
    sleep (1);
    if (child_pid != -1)
        kill (child_pid, SIGTERM);
    exit (status);
}

static void sigchld_handler (int sig)
{
    writelog ("helper: child died\r\n");
    doexit (0);
}

static void sigint_handler (int sig)
{
    writelog ("helper: got SIGINT\r\n");
    writelog ("helper: child pid %d\r\n", child_pid);
    if (master != -1)
        _vte_pty_n_write (master, "\3", 1);
}

static void sigany_handler (int num)
{
    writelog ("helper: got signal %d\r\n", num);
    doexit (0);
}


int main (int argc, char **argv)
{
    int i;
    int status, flags;
    unsigned width, height;
    char *working_dir;
    const char *env[] = {"TERM=dumb", NULL};

    if (argc < 4 && (argc != 2 || strcmp (argv[1], "--test"))) {
        printf ("usage: %s <term_width> <term_height> <binary> [args]\n", argv[0]);
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

    working_dir = getenv (MOO_TERM_HELPER_ENV);
    if (working_dir && strlen (working_dir)) {
        writelog ("helper: chdir to '%s'\r\n", working_dir);
        chdir (working_dir);
    }
    else {
        writelog ("helper: no %s environment variable\r\n", MOO_TERM_HELPER_ENV);
    }

    master = _vte_pty_open(&child_pid, (char**)env,
                           argv[3],
                           argv + 3,
                           NULL,
                           width, height,
                           0, 0, 0);
    writelog ("helper: forked child pid %d\r\n", child_pid);
    for (i = 3; argv[i] != NULL; ++i)
        writelog ("%s ", argv[i]);
    writelog ("\r\n");

    if (master == -1) {
        writelog ("helper: could not open pty\r\n");
        doexit (1);
    }

    if (waitpid (child_pid, &status, WNOHANG) == -1) {
        writelog ("error in waitpid\r\n");
    }

    if ((flags = fcntl (master, F_GETFL)) < 0) {
        writelog ("F_GETFL on master\r\n");
    }
    else if (-1 == fcntl (master, F_SETFL, O_NONBLOCK | flags)) {
        writelog ("F_SETFL on master\r\n");
    }
    if ((flags = fcntl (0, F_GETFL)) < 0) {
        writelog ("F_GETFL on stdin\r\n");
    }
    else if (-1 == fcntl (0, F_SETFL, O_NONBLOCK | flags)) {
        writelog ("F_SETFL on stdin\r\n");
    }

    while (1) {
        int res = 0;
        int again = 1;
        char buf[BUFSIZE];
        int num_read;

        struct pollfd fds[] = {
            {master, POLLIN | POLLPRI | POLLHUP | POLLERR, 0},
            {0, POLLIN | POLLPRI | POLLHUP | POLLERR, 0}
        };

        while (again) {
            res = poll (fds, 2, -1);
            if (res != -1 && res != 1 && res != 2) {
                writelog ("helper: poll returned bogus value %d\r\n", res);
                doexit (1);
            }
            if (res == -1) {
                switch (errno) {
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
                again = 0;
        }

        if (fds[0].revents & POLLHUP) {
            writelog ("helper: got POLLHUP from child\r\n");
            doexit (0);
        }
        if (fds[1].revents & POLLHUP) {
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
        if (fds[0].revents & (POLLIN | POLLPRI)) {
            num_read = read (master, buf, BUFSIZE);
            if (num_read < 1 && errno != EAGAIN && errno != EINTR) {
                writelog ("helper: error reading from child\r\n");
                doexit (1);
            }
            if (num_read > 0 && _vte_pty_n_write (1, buf, num_read) != num_read) {
                writelog ("helper: error writing to parent\r\n");
                doexit (1);
            }
        }

        /********** reading parent **********/
        if (fds[1].revents & (POLLIN | POLLPRI)) {
            int count = 0;

            num_read = read (0, buf, BUFSIZE);
            if (num_read < 1 && errno != EAGAIN && errno != EINTR) {
                writelog ("helper: error reading from parent\r\n");
                doexit (1);
            }

            while (count < num_read) {
                int i;
                char cmd;

                for (i = count; i < num_read && buf[i] != HELPER_CMD_CHAR; ++i) ;
                if (i > count &&
                    _vte_pty_n_write (master, buf + count, i - count) != i - count) {
                    writelog ("helper: error writing to child\r\n");
                    doexit (1);
                }
                count = i;

                if (count == num_read) break;

                writelog ("helper: got command\r\n");
                ++count;
                if (count == num_read) {
                    if (_vte_pty_n_read (0, &cmd, 1) != 1) {
                        writelog ("helper: error reading command\r\n");
                        doexit (1);
                    }
                }
                else
                    cmd = buf[count++];

                switch (cmd) {
                    case HELPER_OK:
                        writelog ("helper: HELPER_OK\r\n");
                        break;

                    case HELPER_GOODBYE:
                        writelog ("helper: HELPER_GOODBYE\r\n");
                        doexit (0);
                        break;

                    case HELPER_SET_SIZE:
                        writelog ("helper: HELPER_SET_SIZE\r\n");
                    {
                        char size[4];
                        int width, height, j;

                        for (i = 0; count + i < num_read; ++i)
                            size[i] = buf[count + i];
                        count += i;
                        if (i < 4)
                            for (j = i; j < 4; ++j)
                                if (_vte_pty_n_read (0, size + j, 1) != 1) {
                                    writelog ("helper: could not read size\r\n");
                                    doexit (1);
                                }

                        width = CHARS_TO_UINT (size[0], size[1]);
                        height = CHARS_TO_UINT (size[2], size[3]);
                        writelog ("helper: SET_SIZE %d, %d\r\n", width, height);
                        if (_vte_pty_set_size (master, width, height) == -1) {
                            writelog ("helper: could not set size\r\n");
                        }
                        else {
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
