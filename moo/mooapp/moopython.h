/*
 *   mooapp/moopython.h
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

#ifndef MOOPYTHON_H
#define MOOPYTHON_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MOO_TYPE_PYTHON              (moo_python_get_type ())
#define MOO_PYTHON(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PYTHON, MooPython))
#define MOO_PYTHON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PYTHON, MooPythonClass))
#define MOO_IS_PYTHON(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PYTHON))
#define MOO_IS_PYTHON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PYTHON))
#define MOO_PYTHON_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PYTHON, MooPythonClass))


typedef struct _MooPython        MooPython;
typedef struct _MooPythonClass   MooPythonClass;

typedef void    (*MooPythonLogFunc) (const char *text,
                                     int         len,
                                     gpointer    data);

struct _MooPython
{
    GObject              parent;

    gboolean             running;
    void                *main_mod;
    MooPythonLogFunc     log_in_func;
    MooPythonLogFunc     log_out_func;
    MooPythonLogFunc     log_err_func;
    void                *log_data;

    gboolean             use_console;
    struct _MooPythonConsole *_console;
};

struct _MooPythonClass
{
    GObjectClass parent_class;
};

GType       moo_python_get_type         (void) G_GNUC_CONST;

MooPython  *moo_python_get_instance     (void);
MooPython  *moo_python_new              (gboolean        use_python_console);

void        moo_python_start            (MooPython      *python,
                                         int             argc,
                                         char          **argv);
void        moo_python_shutdown         (MooPython      *python);
gboolean    moo_python_running          (MooPython      *python);

void        moo_python_set_log_func     (MooPython      *python,
                                         MooPythonLogFunc  in,
                                         MooPythonLogFunc  out,
                                         MooPythonLogFunc  err,
                                         gpointer        data);
void        moo_python_write_log        (MooPython      *python,
                                         int             kind,
                                         const char     *text,
                                         int             len);

int         moo_python_run_simple_string(MooPython      *python,
                                         const char     *cmd);
int         moo_python_run_simple_file  (MooPython      *python,
                                         gpointer        fp,
                                         const char     *filename);

gpointer    moo_python_run_string       (MooPython      *python,
                                         const char     *str,
                                         gboolean        silent);
gpointer    moo_python_run_file         (MooPython      *python,
                                         gpointer        fp,
                                         const char     *filename);

gpointer    moo_python_get_console      (MooPython      *python);


G_END_DECLS

#endif /* MOOPYTHON_H */
