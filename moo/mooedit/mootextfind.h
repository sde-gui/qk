/*
 *   mootextfind.h
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

#ifndef __MOO_TEXT_FIND_H__
#define __MOO_TEXT_FIND_H__

#include <gtk/gtktextview.h>
#include <gtk/gtkdialog.h>
#include "mooutils/moohistorylist.h"
#include "mooutils/mooglade.h"
#include "mooutils/eggregex.h"

G_BEGIN_DECLS


#define MOO_TYPE_FIND_FLAGS         (moo_find_flags_get_type ())

#define MOO_TYPE_FIND               (moo_find_get_type ())
#define MOO_FIND(object)            (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FIND, MooFind))
#define MOO_FIND_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FIND, MooFindClass))
#define MOO_IS_FIND(object)         (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FIND))
#define MOO_IS_FIND_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FIND))
#define MOO_FIND_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FIND, MooFindClass))


typedef struct _MooFind       MooFind;
typedef struct _MooFindClass  MooFindClass;

typedef enum {
    MOO_FIND_REGEX              = 1 << 0,
    MOO_FIND_CASELESS           = 1 << 1,
    MOO_FIND_IN_SELECTED        = 1 << 2,
    MOO_FIND_BACKWARDS          = 1 << 3,
    MOO_FIND_WHOLE_WORDS        = 1 << 4,
    MOO_FIND_FROM_CURSOR        = 1 << 5,
    MOO_FIND_DONT_PROMPT        = 1 << 6,
    MOO_FIND_REPL_LITERAL       = 1 << 7
} MooFindFlags;

struct _MooFind
{
    GtkDialog base;
    MooGladeXML *xml;
    EggRegex *regex;
    guint replace : 1;
};

struct _MooFindClass
{
    GtkDialogClass base_class;
};

typedef void (*MooFindMsgFunc) (const char *msg,
                                gpointer    data);


GType           moo_find_get_type           (void) G_GNUC_CONST;
GType           moo_find_flags_get_type     (void) G_GNUC_CONST;

GtkWidget      *moo_find_new                (gboolean        replace);

void            moo_find_setup              (MooFind        *find,
                                             GtkTextView    *view);
gboolean        moo_find_run                (MooFind        *find,
                                             MooFindMsgFunc  msg_func,
                                             gpointer        data);

void            moo_find_set_flags          (MooFind        *find,
                                             MooFindFlags    flags);
MooFindFlags    moo_find_get_flags          (MooFind        *find);
/* returned string/regex must be freed/unrefed */
char           *moo_find_get_text           (MooFind        *find);
EggRegex       *moo_find_get_regex          (MooFind        *find);
char           *moo_find_get_replacement    (MooFind        *find);

void            moo_text_view_run_find      (GtkTextView    *view,
                                             MooFindMsgFunc  msg_func,
                                             gpointer        data);
void            moo_text_view_run_replace   (GtkTextView    *view,
                                             MooFindMsgFunc  msg_func,
                                             gpointer        data);
void            moo_text_view_run_find_next (GtkTextView    *view,
                                             MooFindMsgFunc  msg_func,
                                             gpointer        data);
void            moo_text_view_run_find_prev (GtkTextView    *view,
                                             MooFindMsgFunc  msg_func,
                                             gpointer        data);
void            moo_text_view_run_goto_line (GtkTextView    *view);


G_END_DECLS

#endif /* __MOO_TEXT_FIND_H__ */
