/*
 *   mooterm/mootermpt-cygwin.c
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

#define MOOTERM_COMPILATION
#include "mooterm/mootermpt-private.h"
#include "mooterm/mootermpt-win32.h"
#include "mooterm/mooterm-private.h"
#include "mooterm/mootermhelper.h"
#include "mooterm/mooterm-private.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>

#define TRY_NUM                 10
#define SLEEP_TIME              10
#define TERM_EMULATION          "xterm"
#define READ_CHUNK_SIZE         1024
#define WRITE_CHUNK_SIZE        4096
#define POLL_TIME               5
#define POLL_NUM                1

static char *HELPER_DIR = NULL;
#define HELPER_BINARY "termhelper.exe"


void        moo_term_set_helper_directory   (const char *dir)
{
    g_free (HELPER_DIR);
    HELPER_DIR = g_strdup (dir);
}


#define TERM_WIDTH(pt__)  (MOO_TERM_PT(pt__)->priv->term->priv->width)
#define TERM_HEIGHT(pt__) (MOO_TERM_PT(pt__)->priv->term->priv->height)


#define MOO_TERM_PT_CYG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT_CYG, MooTermPtCyg))
#define MOO_TERM_PT_CYG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT_CYG, MooTermPtCygClass))
#define MOO_IS_TERM_PT_CYG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT_CYG))
#define MOO_IS_TERM_PT_CYG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT_CYG))
#define MOO_TERM_PT_CYG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT_CYG, MooTermPtCygClass))

typedef struct _MooTermPtCyg           MooTermPtCyg;
typedef struct _MooTermPtCygClass      MooTermPtCygClass;


struct _MooTermPtCyg {
    MooTermPtWin parent;
};


struct _MooTermPtCygClass {
    MooTermPtWinClass parent_class;
};

static void moo_term_pt_cyg_finalize    (GObject    *object);

static void     set_size            (MooTermPt      *pt,
                                     guint           width,
                                     guint           height);
static gboolean fork_command        (MooTermPt      *pt,
                                     const MooTermCommand *cmd,
                                     const char     *working_dir,
                                     char          **envp,
                                     GError        **error);
static void     kill_child          (MooTermPt      *pt);
static char     get_erase_char      (MooTermPt      *pt);
static void     send_intr           (MooTermPt      *pt);


/* MOO_TYPE_TERM_PT_CYG */
G_DEFINE_TYPE (MooTermPtCyg, moo_term_pt_cyg, MOO_TYPE_TERM_PT_WIN)


static void
moo_term_pt_cyg_class_init (MooTermPtCygClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    MooTermPtClass *pt_class = MOO_TERM_PT_CLASS (klass);

    gobject_class->finalize = moo_term_pt_cyg_finalize;

    pt_class->set_size = set_size;
    pt_class->fork_command = fork_command;
    pt_class->kill_child = kill_child;
    pt_class->send_intr = send_intr;
    pt_class->get_erase_char = get_erase_char;
}


static void
moo_term_pt_cyg_init (MooTermPtCyg      *pt)
{
}


static void
moo_term_pt_cyg_finalize (GObject *object)
{
    G_OBJECT_CLASS (moo_term_pt_cyg_parent_class)->finalize (object);
}


static gboolean
fork_command (MooTermPt      *pt_gen,
              const MooTermCommand *cmd,
              const char     *working_dir,
              char          **envp,
              GError        **error)
{
    MooTermPtCyg *pt;
    gboolean result;
    char *cmd_line = NULL;
    GString *helper_binary = NULL;
    char *old_path;
    MooTermCommand *new_cmd;

    g_return_val_if_fail (!pt_gen->priv->child_alive, FALSE);
    g_return_val_if_fail (cmd->cmd_line != NULL, FALSE);

    pt = MOO_TERM_PT_CYG (pt_gen);

    if (!HELPER_DIR || !HELPER_DIR[0])
    {
        g_warning ("%s: helper directory is not set", G_STRLOC);
        g_free (HELPER_DIR);
        HELPER_DIR = NULL;
    }

    if (HELPER_DIR)
    {
        helper_binary = g_string_new (HELPER_DIR);
        g_string_append (helper_binary, "\\" HELPER_BINARY);
    }
    else
    {
        helper_binary = g_string_new (HELPER_BINARY);
    }

    g_message ("%s: helper '%s'", G_STRLOC, helper_binary->str);

    cmd_line = g_strdup_printf ("%s %d %d %s", HELPER_BINARY,
                                TERM_WIDTH (pt_gen), 
                                TERM_HEIGHT (pt_gen), 
                                cmd->cmd_line);

    if (HELPER_DIR)
    {
        char *new_path;

        old_path = g_strdup (g_getenv ("PATH"));
        g_message ("%s: pushing '%s' to %s", G_STRLOC, HELPER_DIR, "PATH");

        if (old_path)
            new_path = g_strdup_printf ("%s;%s", HELPER_DIR, old_path);
        else
            new_path = g_strdup (HELPER_DIR);

        g_setenv ("PATH", new_path, TRUE);
        g_free (new_path);
    }

    if (working_dir)
        g_setenv (MOO_TERM_HELPER_ENV, working_dir, TRUE);

    new_cmd = moo_term_command_new (cmd_line, NULL);
    g_return_val_if_fail (new_cmd != NULL, FALSE);
    g_message ("%s: command line '%s'", G_STRLOC, cmd_line);

    result = MOO_TERM_PT_CLASS(moo_term_pt_cyg_parent_class)->
           fork_command (pt_gen, new_cmd, HELPER_BINARY, envp, error);

    if (old_path)
        g_setenv ("PATH", old_path, TRUE);
    else
        g_unsetenv ("PATH");
    
    g_string_free (helper_binary, TRUE);
    g_free (old_path);
    moo_term_command_free (new_cmd);

    return result;
}


static void
kill_child (MooTermPt *pt)
{
    char cmd[2] = {HELPER_CMD_CHAR, HELPER_GOODBYE};
    _moo_term_pt_write (pt, cmd, sizeof (cmd));
    MOO_TERM_PT_CLASS(moo_term_pt_cyg_parent_class)->kill_child (pt);
}


static void
set_size (MooTermPt      *pt,
          guint           width,
          guint           height)
{
    if (pt->priv->child_alive)
        _moo_term_pt_write (MOO_TERM_PT (pt), 
                            set_size_cmd (width, height), 
                            SIZE_CMD_LEN);
}


static char
get_erase_char (G_GNUC_UNUSED MooTermPt *pt)
{
    return 127;
}


static void
send_intr (MooTermPt *pt)
{
    g_return_if_fail (pt->priv->child_alive);
    pt_discard_pending_write (pt);
    _moo_term_pt_write (pt, "\3", 1);
}
