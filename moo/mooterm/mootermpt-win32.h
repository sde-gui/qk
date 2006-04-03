/*
 *   mootermpt-win32.h
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

#ifndef MOOTERM_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_TERM_PT_WIN32_H__
#define __MOO_TERM_PT_WIN32_H__

#include <mooterm/mootermpt.h>

G_BEGIN_DECLS


#define MOO_TERM_PT_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_TERM_PT_WIN, MooTermPtWin))
#define MOO_TERM_PT_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MOO_TYPE_TERM_PT_WIN, MooTermPtWinClass))
#define MOO_IS_TERM_PT_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_TERM_PT_WIN))
#define MOO_IS_TERM_PT_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MOO_TYPE_TERM_PT_WIN))
#define MOO_TERM_PT_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MOO_TYPE_TERM_PT_WIN, MooTermPtWinClass))

typedef struct _MooTermPtWin           MooTermPtWin;
typedef struct _MooTermPtWinClass      MooTermPtWinClass;


struct _MooTermPtWin {
    MooTermPt   parent;

    GPid        pid;
    gulong      process_id;
    int         in;
    int         out;
    GIOChannel *in_io;
    GIOChannel *out_io;
    guint       watch_id;
    guint       out_watch_id;
};


struct _MooTermPtWinClass {
    MooTermPtClass  parent_class;
};


G_END_DECLS

#endif /* __MOO_TERM_PT_WIN32_H__ */
