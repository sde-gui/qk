/*
 *   mooeditwindow.h
 *
 *   Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#ifndef MOO_EDIT_WINDOW_H
#define MOO_EDIT_WINDOW_H

#include <mooedit/mooedittypes.h>
#include <mooedit/mooedit.h>
#include <mooedit/mooedit-enums.h>
#include <mooutils/moowindow.h>
#include <mooutils/moobigpaned.h>
#include <mooutils/mooatom.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_WINDOW            (moo_edit_window_get_type ())
#define MOO_EDIT_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOO_TYPE_EDIT_WINDOW, MooEditWindow))
#define MOO_EDIT_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_WINDOW, MooEditWindowClass))
#define MOO_IS_EDIT_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOO_TYPE_EDIT_WINDOW))
#define MOO_IS_EDIT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_WINDOW))
#define MOO_EDIT_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_WINDOW, MooEditWindowClass))

typedef struct MooEditWindowPrivate MooEditWindowPrivate;
typedef struct MooEditWindowClass   MooEditWindowClass;

struct MooEditWindow
{
    MooWindow               parent;
    MooEditWindowPrivate   *priv;
    MooBigPaned            *paned;
};

struct MooEditWindowClass
{
    MooWindowClass          parent_class;

    /* these do not open or close document */
    void (*new_doc)         (MooEditWindow  *window,
                             MooEdit        *doc);
    void (*close_doc)       (MooEditWindow  *window,
                             MooEdit        *doc);
    void (*close_doc_after) (MooEditWindow  *window);
};

#define MOO_EDIT_TAB_ATOM_NAME "MOO_EDIT_TAB"
#define MOO_EDIT_TAB_ATOM (moo_edit_tab_atom ())
MOO_DECLARE_ATOM_GLOBAL (MOO_EDIT_TAB, moo_edit_tab)

GType            moo_edit_window_get_type               (void) G_GNUC_CONST;

gboolean         moo_edit_window_close_all              (MooEditWindow  *window);

typedef gboolean (*MooActionCheckFunc) (GtkAction      *action,
                                        MooEditWindow  *window,
                                        MooEdit        *doc,
                                        gpointer        data);
void             moo_edit_window_set_action_check       (const char     *action_id,
                                                         MooActionCheckType type,
                                                         MooActionCheckFunc func,
                                                         gpointer        data,
                                                         GDestroyNotify  notify);
void             moo_edit_window_set_action_filter      (const char     *action_id,
                                                         MooActionCheckType type,
                                                         const char     *filter);

MooEdit         *moo_edit_window_get_active_doc         (MooEditWindow  *window);
void             moo_edit_window_set_active_doc         (MooEditWindow  *window,
                                                         MooEdit        *edit);
MooEditView     *moo_edit_window_get_active_view        (MooEditWindow  *window);
void             moo_edit_window_set_active_view        (MooEditWindow  *window,
                                                         MooEditView    *view);

MooEditor       *moo_edit_window_get_editor             (MooEditWindow  *window);

MooEdit         *moo_edit_window_get_nth_doc            (MooEditWindow  *window,
                                                         guint           n);
MooEditView     *moo_edit_window_get_nth_view           (MooEditWindow  *window,
                                                         guint           n);
MooEditArray    *moo_edit_window_get_docs               (MooEditWindow  *window);
MooEditViewArray*moo_edit_window_get_views              (MooEditWindow  *window);
int              moo_edit_window_n_docs                 (MooEditWindow  *window);

/* sinks widget */
MooPane         *moo_edit_window_add_pane               (MooEditWindow  *window,
                                                         const char     *user_id,
                                                         GtkWidget      *widget,
                                                         MooPaneLabel   *label,
                                                         MooPanePosition position);
gboolean         moo_edit_window_remove_pane            (MooEditWindow  *window,
                                                         const char     *user_id);
GtkWidget       *moo_edit_window_get_pane               (MooEditWindow  *window,
                                                         const char     *user_id);

typedef void (*MooAbortJobFunc) (gpointer job);

void             moo_edit_window_add_stop_client        (MooEditWindow  *window,
                                                         GObject        *client);
void             moo_edit_window_remove_stop_client     (MooEditWindow  *window,
                                                         GObject        *client);
void             moo_edit_window_abort_jobs             (MooEditWindow  *window);


G_END_DECLS

#endif /* MOO_EDIT_WINDOW_H */
