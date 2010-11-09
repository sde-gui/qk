/* gspawn.h - Process launching
 *
 *  Copyright 2000 Red Hat, Inc.
 *
 * GLib is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not, write
 * to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if defined(NOT_DEFINED) && !defined (__GLIB_H_INSIDE__) && !defined (GLIB_COMPILATION)
#error "Only <glib.h> can be included directly."
#endif

#ifndef __M_SPAWN_H__
#define __M_SPAWN_H__

#include <glib.h>

G_BEGIN_DECLS

/* I'm not sure I remember our proposed naming convention here. */
#define M_SPAWN_ERROR m_spawn_error_quark ()

typedef enum
{
  M_SPAWN_ERROR_FORK,   /* fork failed due to lack of memory */
  M_SPAWN_ERROR_READ,   /* read or select on pipes failed */
  M_SPAWN_ERROR_CHDIR,  /* changing to working dir failed */
  M_SPAWN_ERROR_ACCES,  /* execv() returned EACCES */
  M_SPAWN_ERROR_PERM,   /* execv() returned EPERM */
  M_SPAWN_ERROR_2BIG,   /* execv() returned E2BIG */
  M_SPAWN_ERROR_NOEXEC, /* execv() returned ENOEXEC */
  M_SPAWN_ERROR_NAMETOOLONG, /* ""  "" ENAMETOOLONG */
  M_SPAWN_ERROR_NOENT,       /* ""  "" ENOENT */
  M_SPAWN_ERROR_NOMEM,       /* ""  "" ENOMEM */
  M_SPAWN_ERROR_NOTDIR,      /* ""  "" ENOTDIR */
  M_SPAWN_ERROR_LOOP,        /* ""  "" ELOOP   */
  M_SPAWN_ERROR_TXTBUSY,     /* ""  "" ETXTBUSY */
  M_SPAWN_ERROR_IO,          /* ""  "" EIO */
  M_SPAWN_ERROR_NFILE,       /* ""  "" ENFILE */
  M_SPAWN_ERROR_MFILE,       /* ""  "" EMFLE */
  M_SPAWN_ERROR_INVAL,       /* ""  "" EINVAL */
  M_SPAWN_ERROR_ISDIR,       /* ""  "" EISDIR */
  M_SPAWN_ERROR_LIBBAD,      /* ""  "" ELIBBAD */
  M_SPAWN_ERROR_FAILED       /* other fatal failure, error->message
                              * should explain
                              */
} MSpawnError;

typedef void (* MSpawnChildSetupFunc) (gpointer user_data);

typedef enum
{
  M_SPAWN_LEAVE_DESCRIPTORS_OPEN = 1 << 0,
  M_SPAWN_DO_NOT_REAP_CHILD      = 1 << 1,
  /* look for argv[0] in the path i.e. use execvp() */
  M_SPAWN_SEARCH_PATH            = 1 << 2,
  /* Dump output to /dev/null */
  M_SPAWN_STDOUT_TO_DEV_NULL     = 1 << 3,
  M_SPAWN_STDERR_TO_DEV_NULL     = 1 << 4,
  M_SPAWN_CHILD_INHERITS_STDIN   = 1 << 5,
  M_SPAWN_FILE_AND_ARGV_ZERO     = 1 << 6
} MSpawnFlags;

GQuark m_spawn_error_quark (void);

#ifdef G_OS_WIN32
#define m_spawn_async m_spawn_async_utf8
#define m_spawn_async_with_pipes m_spawn_async_with_pipes_utf8
#define m_spawn_sync m_spawn_sync_utf8
#define m_spawn_command_line_sync m_spawn_command_line_sync_utf8
#define m_spawn_command_line_async m_spawn_command_line_async_utf8
#endif

gboolean m_spawn_async (const gchar           *working_directory,
                        gchar                **argv,
                        gchar                **envp,
                        MSpawnFlags            flags,
                        MSpawnChildSetupFunc   child_setup,
                        gpointer               user_data,
                        GPid                  *child_pid,
                        GError               **error);


/* Opens pipes for non-NULL standard_output, standard_input, standard_error,
 * and returns the parent's end of the pipes.
 */
gboolean m_spawn_async_with_pipes (const gchar          *working_directory,
                                   gchar               **argv,
                                   gchar               **envp,
                                   MSpawnFlags           flags,
                                   MSpawnChildSetupFunc  child_setup,
                                   gpointer              user_data,
                                   GPid                 *child_pid,
                                   gint                 *standard_input,
                                   gint                 *standard_output,
                                   gint                 *standard_error,
                                   GError              **error);


/* If standard_output or standard_error are non-NULL, the full
 * standard output or error of the command will be placed there.
 */

gboolean m_spawn_sync         (const gchar          *working_directory,
                               gchar               **argv,
                               gchar               **envp,
                               MSpawnFlags           flags,
                               MSpawnChildSetupFunc  child_setup,
                               gpointer              user_data,
                               gchar               **standard_output,
                               gchar               **standard_error,
                               gint                 *exit_status,
                               GError              **error);

gboolean m_spawn_command_line_sync  (const gchar          *command_line,
                                     gchar               **standard_output,
                                     gchar               **standard_error,
                                     gint                 *exit_status,
                                     GError              **error);
gboolean m_spawn_command_line_async (const gchar          *command_line,
                                     GError              **error);

void m_spawn_close_pid (GPid pid);

G_END_DECLS

#endif /* __M_SPAWN_H__ */
