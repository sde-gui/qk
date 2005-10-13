/*
 * Copyright (C) 2001,2002 Red Hat, Inc.
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Modified by muntyan */
/* mooterm/pty.c */

/* macro from glib/gmacros.h */
#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_PRINTF( format_idx, arg_idx )    \
  __attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define G_GNUC_SCANF( format_idx, arg_idx )     \
  __attribute__((__format__ (__scanf__, format_idx, arg_idx)))
#define G_GNUC_FORMAT( arg_idx )                \
  __attribute__((__format_arg__ (arg_idx)))
#define G_GNUC_NORETURN                         \
  __attribute__((__noreturn__))
#define G_GNUC_CONST                            \
  __attribute__((__const__))
#define G_GNUC_UNUSED                           \
  __attribute__((__unused__))
#define G_GNUC_NO_INSTRUMENT			\
  __attribute__((__no_instrument_function__))
#else   /* !__GNUC__ */
#define G_GNUC_PRINTF( format_idx, arg_idx )
#define G_GNUC_SCANF( format_idx, arg_idx )
#define G_GNUC_FORMAT( arg_idx )
#define G_GNUC_NORETURN
#define G_GNUC_CONST
#define G_GNUC_UNUSED
#define G_GNUC_NO_INSTRUMENT
#endif  /* !__GNUC__ */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /*  HAVE_CONFIG_H */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_TERMIOS_H
#include <sys/termios.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#include <sys/uio.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
/* for darwin */
#ifdef HAVE_UTIL_H
#include <util.h>
#endif
#include <assert.h>
#include "mooterm/pty.h"

#ifdef MSG_NOSIGNAL
#define PTY_RECVMSG_FLAGS MSG_NOSIGNAL
#else
#define PTY_RECVMSG_FLAGS 0
#endif


/* Reset the handlers for all known signals to their defaults.  The parent
 * (or one of the libraries it links to) may have changed one to be ignored. */
static void
_vte_pty_reset_signal_handlers(void)
{
	signal(SIGHUP,  SIG_DFL);
	signal(SIGINT,  SIG_DFL);
	signal(SIGILL,  SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGFPE,  SIG_DFL);
	signal(SIGKILL, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGCONT, SIG_DFL);
	signal(SIGSTOP, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
#ifdef SIGBUS
	signal(SIGBUS,  SIG_DFL);
#endif
#ifdef SIGPOLL
	signal(SIGPOLL, SIG_DFL);
#endif
#ifdef SIGPROF
	signal(SIGPROF, SIG_DFL);
#endif
#ifdef SIGSYS
	signal(SIGSYS,  SIG_DFL);
#endif
#ifdef SIGTRAP
	signal(SIGTRAP, SIG_DFL);
#endif
#ifdef SIGURG
	signal(SIGURG,  SIG_DFL);
#endif
#ifdef SIGVTALARM
	signal(SIGVTALARM, SIG_DFL);
#endif
#ifdef SIGXCPU
	signal(SIGXCPU, SIG_DFL);
#endif
#ifdef SIGXFSZ
	signal(SIGXFSZ, SIG_DFL);
#endif
#ifdef SIGIOT
	signal(SIGIOT,  SIG_DFL);
#endif
#ifdef SIGEMT
	signal(SIGEMT,  SIG_DFL);
#endif
#ifdef SIGSTKFLT
	signal(SIGSTKFLT, SIG_DFL);
#endif
#ifdef SIGIO
	signal(SIGIO,   SIG_DFL);
#endif
#ifdef SIGCLD
	signal(SIGCLD,  SIG_DFL);
#endif
#ifdef SIGPWR
	signal(SIGPWR,  SIG_DFL);
#endif
#ifdef SIGINFO
	signal(SIGINFO, SIG_DFL);
#endif
#ifdef SIGLOST
	signal(SIGLOST, SIG_DFL);
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, SIG_DFL);
#endif
#ifdef SIGUNUSED
	signal(SIGUNUSED, SIG_DFL);
#endif
}

#ifdef HAVE_SOCKETPAIR
static int
_vte_pty_pipe_open(int *a, int *b)
{
	int p[2], ret = -1;
#ifdef PF_UNIX
#ifdef SOCK_STREAM
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, p);
#else
#ifdef SOCK_DGRAM
	ret = socketpair(PF_UNIX, SOCK_DGRAM, 0, p);
#endif
#endif
	if (ret == 0) {
		*a = p[0];
		*b = p[1];
		return 0;
	}
#endif
	return ret;
}
#else
static int
_vte_pty_pipe_open(int *a, int *b)
{
	int p[2], ret = -1;

	ret = pipe(p);

	if (ret == 0) {
		*a = p[0];
		*b = p[1];
	}
	return ret;
}
#endif

static int
_vte_pty_pipe_open_bi(int *a, int *b, int *c, int *d)
{
	int ret;
	ret = _vte_pty_pipe_open(a, b);
	if (ret != 0) {
		return ret;
	}
	ret = _vte_pty_pipe_open(c, d);
	if (ret != 0) {
		close(*a);
		close(*b);
	}
	return ret;
}

/* Like read, but hide EINTR and EAGAIN. */
ssize_t _vte_pty_n_read (int fd, void *buffer, size_t count)
{
	size_t n = 0;
	char *buf = (char*)buffer;
	int i;
	while (n < count) {
		i = read(fd, buf + n, count - n);
		switch (i) {
		case 0:
			return n;
			break;
		case -1:
			switch (errno) {
			case EINTR:
			case EAGAIN:
#ifdef ERESTART
			case ERESTART:
#endif
				break;
			default:
				return -1;
			}
			break;
		default:
			n += i;
			break;
		}
	}
	return n;
}

/* Like write, but hide EINTR and EAGAIN. */
ssize_t _vte_pty_n_write(int fd, const void *buffer, size_t count)
{
	size_t n = 0;
	const char *buf = (const char*)buffer;
	int i;
	while (n < count) {
		i = write(fd, buf + n, count - n);
		switch (i) {
		case 0:
			return n;
			break;
		case -1:
			switch (errno) {
			case EINTR:
			case EAGAIN:
#ifdef ERESTART
			case ERESTART:
#endif
				break;
			default:
				return -1;
			}
			break;
		default:
			n += i;
			break;
		}
	}
	return n;
}

/* Run the given command (if specified), using the given descriptor as the
 * controlling terminal. */
static int
_vte_pty_run_on_pty(int fd, int ready_reader, int ready_writer,
		    char **env_add, const char *command, char **argv,
		    const char *directory)
{
	int i;
	char c = 'a'; /* to make valgrind happy */
	char **args, *arg;

	if (fd != STDIN_FILENO) {
		dup2(fd, STDIN_FILENO);
	}
	if (fd != STDOUT_FILENO) {
		dup2(fd, STDOUT_FILENO);
	}
	if (fd != STDERR_FILENO) {
		dup2(fd, STDERR_FILENO);
	}

	/* Close the original slave descriptor, unless it's one of the stdio
	 * descriptors. */
	if ((fd != STDIN_FILENO) &&
	    (fd != STDOUT_FILENO) &&
	    (fd != STDERR_FILENO)) {
		close(fd);
	}

#ifdef HAVE_STROPTS_H
	if ((ioctl(fd, I_FIND, "ptem") == 0) &&
	    (ioctl(fd, I_PUSH, "ptem") == -1)) {
		close (fd);
		_exit (0);
		return -1;
	}

	if ((ioctl(fd, I_FIND, "ldterm") == 0) &&
	    (ioctl(fd, I_PUSH, "ldterm") == -1)) {
		close (fd);
		_exit (0);
		return -1;
	}

	if ((ioctl(fd, I_FIND, "ttcompat") == 0) &&
	    (ioctl(fd, I_PUSH, "ttcompat") == -1)) {
		perror ("ioctl (fd, I_PUSH, \"ttcompat\")");
		close (fd);
		_exit (0);
		return -1;
	}
#endif /* HAVE_STROPTS_H */

	/* Set any environment variables. */
	for (i = 0; (env_add != NULL) && (env_add[i] != NULL); i++) {
		if (putenv (strdup (env_add[i])) != 0) {
#if 0
// 			g_warning(_("Error adding `%s' to environment, "
// 				    "continuing."), env_add[i]);
#endif
		}
	}

	/* Reset our signals -- our parent may have done any number of
	 * weird things to them. */
	_vte_pty_reset_signal_handlers();

	/* Change to the requested directory. */
	if (directory != NULL) {
		chdir(directory);
	}

	/* Signal to the parent that we've finished setting things up by
	 * sending an arbitrary byte over the status pipe and waiting for
	 * a response.  This synchronization step ensures that the pty is
	 * fully initialized before the parent process attempts to do anything
	 * with it, and is required on systems where additional setup, beyond
	 * merely opening the device, is required.  This is at least the case
	 * on Solaris. */
	_vte_pty_n_write(ready_writer, &c, 1);
	fsync(ready_writer);
	_vte_pty_n_read(ready_reader, &c, 1);
	close(ready_writer);
	if (ready_writer != ready_reader) {
		close(ready_reader);
	}

	/* If the caller provided a command, we can't go back, ever. */
	if (command != NULL) {
		/* Outta here. */
		if (argv != NULL) {
			for (i = 0; (argv[i] != NULL); i++) ;
			args = (char**)calloc (i + 1, sizeof(char*));
			for (i = 0; (argv[i] != NULL); i++) {
				args[i] = strdup (argv[i]);
			}
			execvp(command, args);
		} else {
			arg = strdup (command);
			execlp(command, arg, NULL);
		}

		/* Avoid calling any atexit() code. */
		_exit(0);
		assert (0);
	}

	return 0;
}

/* Open the named PTY slave, fork off a child (storing its PID in child),
 * and exec the named command in its own session as a process group leader */
static int
_vte_pty_fork_on_pty_name(const char *path, int parent_fd, char **env_add,
			  const char *command, char **argv,
			  const char *directory,
			  int columns, int rows, pid_t *child)
{
	int fd, i;
	char c;
	int ready_a[2], ready_b[2];
	pid_t pid;

	/* Open pipes for synchronizing between parent and child. */
	if (_vte_pty_pipe_open_bi(&ready_a[0], &ready_a[1],
				  &ready_b[0], &ready_b[1]) == -1) {
		/* Error setting up pipes.  Bail. */
		*child = -1;
		return -1;
	}

	/* Start up a child. */
	pid = fork();
	switch (pid) {
	case -1:
		/* Error fork()ing.  Bail. */
		*child = -1;
		return -1;
		break;
	case 0:
		/* Child. Close the parent's ends of the pipes. */
		close(ready_a[0]);
		close(ready_b[1]);

		/* Start a new session and become process-group leader. */
		setsid();
		setpgid(0, 0);

		/* Close most descriptors. */
		for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
			if ((i != ready_b[0]) && (i != ready_a[1])) {
				close(i);
			}
		}

		/* Open the slave PTY, acquiring it as the controlling terminal
		 * for this process and its children. */
		fd = open(path, O_RDWR);
		if (fd == -1) {
			return -1;
		}
#ifdef TIOCSCTTY
		/* TIOCSCTTY is defined?  Let's try that, too. */
		ioctl(fd, TIOCSCTTY, fd);
#endif
		/* Store 0 as the "child"'s ID to indicate to the caller that
		 * it is now the child. */
		*child = 0;
		return _vte_pty_run_on_pty(fd, ready_b[0], ready_a[1],
					   env_add, command, argv, directory);
		break;
	default:
		/* Parent.  Close the child's ends of the pipes, do the ready
		 * handshake, and return the child's PID. */
		close(ready_b[0]);
		close(ready_a[1]);

		/* Wait for the child to be ready, set the window size, then
		 * signal that we're ready.  We need to synchronize here to
		 * avoid possible races when the child has to do more setup
		 * of the terminal than just opening it. */
		_vte_pty_n_read(ready_a[0], &c, 1);
		_vte_pty_set_size(parent_fd, columns, rows);
		_vte_pty_n_write(ready_b[1], &c, 1);
		close(ready_a[0]);
		close(ready_b[1]);

		*child = pid;
		return 0;
		break;
	}
	assert (0);
	return -1;
}

#if 0
// /* Fork off a child (storing its PID in child), and exec the named command
//  * in its own session as a process group leader using the given terminal. */
// static int
// _vte_pty_fork_on_pty_fd(int fd, char **env_add,
// 			const char *command, char **argv,
// 			const char *directory,
// 			int columns, int rows, pid_t *child)
// {
// 	int i;
// 	char *tty;
// 	char c;
// 	int ready_a[2], ready_b[2];
// 	pid_t pid;
//
// 	/* Open pipes for synchronizing between parent and child. */
// 	if (_vte_pty_pipe_open_bi(&ready_a[0], &ready_a[1],
// 				  &ready_b[0], &ready_b[1]) == -1) {
// 		/* Error setting up pipes.  Bail. */
// 		*child = -1;
// 		return -1;
// 	}
//
// 	/* Start up a child. */
// 	pid = fork();
// 	switch (pid) {
// 	case -1:
// 		/* Error fork()ing.  Bail. */
// 		*child = -1;
// 		return -1;
// 		break;
// 	case 0:
// 		/* Child.  CLose the parent's ends of the pipes. */
// 		close(ready_a[0]);
// 		close(ready_b[1]);
//
// 		/* Save the name of the pty -- we'll need it later to acquire
// 		 * it as our controlling terminal. */
// 		tty = ttyname(fd);
//
// 		/* Start a new session and become process-group leader. */
// 		setsid();
// 		setpgid(0, 0);
//
// 		/* Close all other descriptors. */
// 		for (i = 0; i < sysconf(_SC_OPEN_MAX); i++) {
// 			if ((i != fd) &&
// 			    (i != ready_b[0]) &&
// 			    (i != ready_a[1])) {
// 				close(i);
// 			}
// 		}
//
// 		/* Try to reopen the pty to acquire it as our controlling
// 		 * terminal. */
// 		if (tty != NULL) {
// 			i = open(tty, O_RDWR);
// 			if (i != -1) {
// 				close(fd);
// 				fd = i;
// 			}
// 	#ifdef TIOCSCTTY
// 			/* TIOCSCTTY is defined?  Let's try that, too. */
// 			ioctl(fd, TIOCSCTTY, fd);
// 	#endif
// 		}
//
// 		/* Store 0 as the "child"'s ID to indicate to the caller that
// 		 * it is now the child. */
// 		*child = 0;
// 		return _vte_pty_run_on_pty(fd, ready_b[0], ready_a[1],
// 					   env_add, command, argv, directory);
// 		break;
// 	default:
// 		/* Parent.  Close the child's ends of the pipes, do the ready
// 		 * handshake, and return the child's PID. */
// 		close(ready_b[0]);
// 		close(ready_a[1]);
//
// 		/* Wait for the child to be ready, set the window size, then
// 		 * signal that we're ready.  We need to synchronize here to
// 		 * avoid possible races when the child has to do more setup
// 		 * of the terminal than just opening it. */
// 		_vte_pty_n_read(ready_a[0], &c, 1);
// 		_vte_pty_set_size(fd, columns, rows);
// 		_vte_pty_n_write(ready_b[1], &c, 1);
// 		close(ready_a[0]);
// 		close(ready_b[1]);
//
// 		*child = pid;
// 		return 0;
// 	}
// 	assert (0);
// 	return -1;
// }
#endif

/**
 * vte_pty_set_size:
 * @master: the file descriptor of the pty master
 * @columns: the desired number of columns
 * @rows: the desired number of rows
 *
 * Attempts to resize the pseudo terminal's window size.  If successful, the
 * OS kernel will send #SIGWINCH to the child process group.
 *
 * Returns: 0 on success, -1 on failure.
 */
int
_vte_pty_set_size(int master, int columns, int rows)
{
	struct winsize size;
	int ret;
	memset(&size, 0, sizeof(size));
	size.ws_row = rows ? rows : 24;
	size.ws_col = columns ? columns : 80;
	ret = ioctl(master, TIOCSWINSZ, &size);
	return ret;
}

/**
 * vte_pty_get_size:
 * @master: the file descriptor of the pty master
 * @columns: a place to store the number of columns
 * @rows: a place to store the number of rows
 *
 * Attempts to read the pseudo terminal's window size.
 *
 * Returns: 0 on success, -1 on failure.
 */
int
_vte_pty_get_size(int master, int *columns, int *rows)
{
	struct winsize size;
	int ret;
	memset(&size, 0, sizeof(size));
	ret = ioctl(master, TIOCGWINSZ, &size);
	if (ret == 0) {
		if (columns != NULL) {
			*columns = size.ws_col;
		}
		if (rows != NULL) {
			*rows = size.ws_row;
		}
	} else {
	}
	return ret;
}


#ifndef MOO_OS_DARWIN
static char *
_vte_pty_ptsname(int master)
{
#if defined(HAVE_PTSNAME_R)
	size_t len = 1024;
	char *buf = NULL;
	int i;
	do {
		buf = (char*)calloc (sizeof(char), len);
		i = ptsname_r(master, buf, len - 1);
		switch (i) {
		case 0:
			/* Return the allocated buffer with the name in it. */
			return buf;
			break;
		default:
			free(buf);
			buf = NULL;
			break;
		}
		len *= 2;
	} while ((i != 0) && (errno == ERANGE));
#elif defined(HAVE_PTSNAME)
	char *p;
	if ((p = ptsname(master)) != NULL) {
		return strdup (p);
	}
#elif defined(TIOCGPTN)
	int pty = 0;
	if (ioctl(master, TIOCGPTN, &pty) == 0) {
		return g_strdup_printf("/dev/pts/%d", pty);
	}
#endif
	return NULL;
}
#endif /* MOO_OS_DARWIN */


#ifndef MOO_OS_DARWIN
static int
_vte_pty_getpt()
{
	int fd, flags;
#ifdef HAVE_GETPT
	/* Call the system's function for allocating a pty. */
	fd = getpt();
#elif defined(HAVE_POSIX_OPENPT)
	/* taken from freebsd port */
	fd = posix_openpt(O_RDWR | O_NOCTTY);
#else
	/* Try to allocate a pty by accessing the pty master multiplex. */
	fd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
	if ((fd == -1) && (errno == ENOENT)) {
		fd = open("/dev/ptc", O_RDWR | O_NOCTTY); /* AIX */
	}
#endif
	/* Set it to blocking. */
	flags = fcntl(fd, F_GETFL);
	flags &= ~(O_NONBLOCK);
	fcntl(fd, F_SETFL, flags);
	return fd;
}
#endif /* !MOO_OS_DARWIN */

static int
_vte_pty_grantpt(int master)
{
#ifdef HAVE_GRANTPT
	return grantpt(master);
#else
	return 0;
#endif
}

static int
_vte_pty_unlockpt(int fd)
{
#ifdef HAVE_UNLOCKPT
	return unlockpt(fd);
#elif defined(TIOCSPTLCK)
	int zero = 0;
	return ioctl(fd, TIOCSPTLCK, &zero);
#else
#ifndef MOO_OS_DARWIN
	return -1;
#else /* !MOO_OS_DARWIN */
	return 0;
#endif /* !MOO_OS_DARWIN */
#endif
}


#ifndef MOO_OS_DARWIN

static int
_vte_pty_open_unix98(pid_t *child, char **env_add,
		     const char *command, char **argv,
		     const char *directory, int columns, int rows)
{
	int fd;
	char *buf;

	/* Attempt to open the master. */
	fd = _vte_pty_getpt();
	if (fd != -1) {
		/* Read the slave number and unlock it. */
		if (((buf = _vte_pty_ptsname(fd)) == NULL) ||
		    (_vte_pty_grantpt(fd) != 0) ||
		    (_vte_pty_unlockpt(fd) != 0)) {
			close(fd);
			fd = -1;
		} else {
			/* Start up a child process with the given command. */
			if (_vte_pty_fork_on_pty_name(buf, fd, env_add, command,
						      argv, directory,
						      columns, rows,
						      child) != 0) {
				close(fd);
				fd = -1;
			}
			free(buf);
		}
	}
	return fd;
}

#else /* MOO_OS_DARWIN */

static int
_vte_pty_open_bsd   (pid_t *child, char **env_add,
		     const char *command, char **argv,
		     const char *directory, int columns, int rows)
{
	int master, slave;
	char slave_name[80];

	if (openpty (&master, &slave, slave_name, NULL, NULL) != -1) {
		close (slave);
		/* Read the slave number and unlock it. */
		if ((_vte_pty_grantpt(master) != 0) ||
		    (_vte_pty_unlockpt(master) != 0)) {
			close (master);
			master = -1;
		} else {
			/* Start up a child process with the given command. */
			if (_vte_pty_fork_on_pty_name(slave_name, master, env_add, command,
						      argv, directory,
						      columns, rows,
						      child) != 0)
			{
				close(master);
				master = -1;
			}
		}
	}
	return master;
}

#endif /* MOO_OS_DARWIN */


#if 0
//TODO: learn about this stuff
// // it doesn't work on cygwin
// // #ifdef HAVE_RECVMSG
// #if defined(HAVE_RECVMSG) && defined(LINE_MAX)
// static void
// _vte_pty_read_ptypair(int tunnel, int *parentfd, int *childfd)
// {
// 	int i, ret;
// 	char control[LINE_MAX], iobuf[LINE_MAX];
// 	struct cmsghdr *cmsg;
// 	struct msghdr msg;
// 	struct iovec vec;
//
// 	for (i = 0; i < 2; i++) {
// 		vec.iov_base = iobuf;
// 		vec.iov_len = sizeof(iobuf);
// 		msg.msg_name = NULL;
// 		msg.msg_namelen = 0;
// 		msg.msg_iov = &vec;
// 		msg.msg_iovlen = 1;
// 		msg.msg_control = control;
// 		msg.msg_controllen = sizeof(control);
// 		ret = recvmsg(tunnel, &msg, PTY_RECVMSG_FLAGS);
// 		if (ret == -1) {
// 			return;
// 		}
// 		for (cmsg = CMSG_FIRSTHDR(&msg);
// 		     cmsg != NULL;
// 		     cmsg = CMSG_NXTHDR(&msg, cmsg)) {
// 			if (cmsg->cmsg_type == SCM_RIGHTS) {
// 				memcpy(&ret, CMSG_DATA(cmsg), sizeof(ret));
// 				switch (i) {
// 					case 0:
// 						*parentfd = ret;
// 						break;
// 					case 1:
// 						*childfd = ret;
// 						break;
// 					default:
// 						assert (0);
// 						break;
// 				}
// 			}
// 		}
// 	}
// }
// #else
// #ifdef I_RECVFD
// static void
// _vte_pty_read_ptypair(int tunnel, int *parentfd, int *childfd)
// {
// 	int i;
// 	if (ioctl(tunnel, I_RECVFD, &i) == -1) {
// 		return;
// 	}
// 	*parentfd = i;
// 	if (ioctl(tunnel, I_RECVFD, &i) == -1) {
// 		return;
// 	}
// 	*childfd = i;
// }
// #endif
// #endif
#endif


/**
 * _vte_pty_open:
 * @child: location to store the new process's ID
 * @env_add: a list of environment variables to add to the child's environment
 * @command: name of the binary to run
 * @argv: arguments to pass to @command
 * @directory: directory to start the new command in, or NULL
 * @columns: desired window columns
 * @rows: desired window rows
 * @lastlog: TRUE if the lastlog should be updated
 * @utmp: TRUE if the utmp or utmpx log should be updated
 * @wtmp: TRUE if the wtmp or wtmpx log should be updated
 *
 * Starts a new copy of @command running under a psuedo-terminal, optionally in
 * the supplied @directory, with window size set to @rows x @columns
 * and variables in @env_add added to its environment.  If any combination of
 * @lastlog, @utmp, and @wtmp is set, then the session is logged in the
 * corresponding system files.
 *
 * Returns: an open file descriptor for the pty master, -1 on failure
 */
int
_vte_pty_open(pid_t *child, char **env_add,
	      const char *command, char **argv, const char *directory,
	      int columns, int rows,
              G_GNUC_UNUSED int lastlog,
              G_GNUC_UNUSED int utmp,
              G_GNUC_UNUSED int wtmp)
{
#ifndef MOO_OS_DARWIN
	int ret = -1;
	if (ret == -1) {
		ret = _vte_pty_open_unix98(child, env_add, command, argv,
					   directory, columns, rows);
	}
	return ret;
#else /* MOO_OS_DARWIN */
	return _vte_pty_open_bsd (child, env_add, command, argv,
				  directory, columns, rows);
#endif /* MOO_OS_DARWIN */
}

/**
 * _vte_pty_close:
 * @pty: the pty master descriptor.
 *
 * Cleans up the PTY associated with the descriptor, specifically any logging
 * performed for the session.  The descriptor itself remains open.
 */
void
_vte_pty_close(G_GNUC_UNUSED int p)
{
}

#ifdef PTY_MAIN
int fd;

static void
sigchld_handler(int signum)
{
	/* This is very unsafe.  Never do it in production code. */
	_vte_pty_close(fd);
}

int
main(int argc, char **argv)
{
	pid_t child = 0;
	char c;
	int ret;
	signal(SIGCHLD, sigchld_handler);
	fd = _vte_pty_open(&child, NULL,
			   (argc > 1) ? argv[1] : NULL,
			   (argc > 1) ? argv + 1 : NULL,
			   NULL,
			   0, 0,
			   TRUE, TRUE, TRUE);
	if (child == 0) {
		int i;
		for (i = 0; ; i++) {
			switch (i % 3) {
			case 0:
			case 1:
				fprintf(stdout, "%d\n", i);
				break;
			case 2:
				fprintf(stderr, "%d\n", i);
				break;
			default:
    assert (0);
				break;
			}
			sleep(1);
		}
	}
	g_print("Child pid is %d.\n", (int)child);
	do {
		ret = _vte_pty_n_read(fd, &c, 1);
		if (ret == 0) {
			break;
		}
		if ((ret == -1) && (errno != EAGAIN) && (errno != EINTR)) {
			break;
		}
		if (argc < 2) {
			_vte_pty_n_write(STDOUT_FILENO, "[", 1);
		}
		_vte_pty_n_write(STDOUT_FILENO, &c, 1);
		if (argc < 2) {
			_vte_pty_n_write(STDOUT_FILENO, "]", 1);
		}
	} while (TRUE);
	return 0;
}
#endif
