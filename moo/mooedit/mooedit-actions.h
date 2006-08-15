/*
 *   mooedit-actions.h
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

#ifndef __MOO_EDIT_ACTIONS_H__
#define __MOO_EDIT_ACTIONS_H__

#include <mooutils/mooaction.h>
#include <mooutils/moouixml.h>
#include <mooedit/mooedit.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_ACTION_FLAGS          (moo_edit_action_flags_get_type ())
#define MOO_TYPE_EDIT_ACTION                (moo_edit_action_get_type ())
#define MOO_EDIT_ACTION(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_ACTION, MooEditAction))
#define MOO_EDIT_ACTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_ACTION, MooEditActionClass))
#define MOO_IS_EDIT_ACTION(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_ACTION))
#define MOO_IS_EDIT_ACTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_ACTION))
#define MOO_EDIT_ACTION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_ACTION, MooEditActionClass))

typedef struct _MooEditAction        MooEditAction;
typedef struct _MooEditActionPrivate MooEditActionPrivate;
typedef struct _MooEditActionClass   MooEditActionClass;

typedef enum {
    MOO_EDIT_ACTION_NEED_FILE = 1 << 0
} MooEditActionFlags;

struct _MooEditAction
{
    MooAction parent;
    MooEdit *doc;
    GSList *langs;
    MooEditActionFlags flags;
};

struct _MooEditActionClass
{
    MooActionClass parent_class;

    gboolean (*check_state) (MooEditAction *action);
};


typedef GtkAction *(*MooEditActionFunc)     (MooEdit            *edit,
                                             gpointer            data);

GType   moo_edit_action_get_type            (void) G_GNUC_CONST;
GType   moo_edit_action_flags_get_type      (void) G_GNUC_CONST;

void    moo_edit_class_new_action           (MooEditClass       *klass,
                                             const char         *id,
                                             const char         *first_prop_name,
                                             ...) G_GNUC_NULL_TERMINATED;
void    moo_edit_class_new_actionv          (MooEditClass       *klass,
                                             const char         *id,
                                             const char         *first_prop_name,
                                             va_list             props);
void    moo_edit_class_new_action_custom    (MooEditClass       *klass,
                                             const char         *id,
                                             MooEditActionFunc   func,
                                             gpointer            data,
                                             GDestroyNotify      notify);
void    moo_edit_class_new_action_type      (MooEditClass       *klass,
                                             const char         *id,
                                             GType               type);

void    moo_edit_class_remove_action        (MooEditClass       *klass,
                                             const char         *id);

GtkActionGroup *moo_edit_get_actions        (MooEdit            *edit);
GtkAction  *moo_edit_get_action_by_id       (MooEdit            *edit,
                                             const char         *action_id);

void    moo_edit_action_check_state         (MooEditAction      *action);


G_END_DECLS

#endif /* __MOO_EDIT_ACTIONS_H__ */
