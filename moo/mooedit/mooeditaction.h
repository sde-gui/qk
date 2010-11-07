/*
 *   mooeditaction.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_EDIT_ACTION_H
#define MOO_EDIT_ACTION_H

#include <mooutils/mooaction.h>
#include <mooedit/mooedit.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_ACTION                (moo_edit_action_get_type ())
#define MOO_EDIT_ACTION(object)             (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_ACTION, MooEditAction))
#define MOO_EDIT_ACTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_ACTION, MooEditActionClass))
#define MOO_IS_EDIT_ACTION(object)          (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_ACTION))
#define MOO_IS_EDIT_ACTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_ACTION))
#define MOO_EDIT_ACTION_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_ACTION, MooEditActionClass))

typedef struct _MooEditAction        MooEditAction;
typedef struct _MooEditActionPrivate MooEditActionPrivate;
typedef struct _MooEditActionClass   MooEditActionClass;

struct _MooEditAction
{
    MooAction parent;
    MooEditActionPrivate *priv;
};

struct _MooEditActionClass
{
    MooActionClass parent_class;

    void     (*check_state)     (MooEditAction *action);

    gboolean (*check_visible)   (MooEditAction *action);
    gboolean (*check_sensitive) (MooEditAction *action);
};


GType   moo_edit_action_get_type            (void) G_GNUC_CONST;

MooEdit *moo_edit_action_get_doc            (MooEditAction  *action);

/* defined in mooeditwindow.c */
GSList *_moo_edit_parse_langs               (const char     *string);


G_END_DECLS

#endif /* MOO_EDIT_ACTION_H */
