/*
 *   mooeditwindowpane.h
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

#ifndef __MOO_EDIT_WINDOW_PANE_H__
#define __MOO_EDIT_WINDOW_PANE_H__

#include "mooedit/mooeditwindow.h"
#include "mooedit/moobigpaned.h"

G_BEGIN_DECLS


typedef struct _MooEditWindowPaneInfo   MooEditWindowPaneInfo;
typedef struct _MooEditWindowPaneParams MooEditWindowPaneParams;


typedef gboolean (*MooPaneFactoryInitFunc)  (MooEditWindowPaneInfo  *info,
                                             gpointer                data);
typedef void     (*MooPaneFactoryDeinitFunc)(MooEditWindowPaneInfo  *info,
                                             gpointer                data);
typedef gboolean (*MooPaneCreateFunc)       (MooEditWindow          *window,
                                             MooEditWindowPaneInfo  *info,
                                             MooPaneLabel          **label,
                                             GtkWidget             **widget,
                                             gpointer                data);
typedef void     (*MooPaneDestroyFunc)      (MooEditWindow          *window,
                                             MooEditWindowPaneInfo  *info,
                                             gpointer                data);


struct _MooEditWindowPaneInfo
{
    const char *id;
    const char *name;
    const char *description;

    MooPaneFactoryInitFunc init;
    MooPaneFactoryDeinitFunc deinit;
    MooPaneCreateFunc create;
    MooPaneDestroyFunc destroy;

    MooEditWindowPaneParams *params;
};

struct _MooEditWindowPaneParams
{
    MooPanePosition position;
    gboolean enabled;
};


void         moo_edit_window_register_pane  (MooEditWindowPaneInfo  *info,
                                             gpointer                data);
void         moo_edit_window_unregister_pane(const char             *id);

GtkWidget   *moo_edit_window_lookup_pane    (MooEditWindow          *window,
                                             const char             *id);
MooBigPaned *moo_edit_window_get_paned      (MooEditWindow          *window);


G_END_DECLS

#endif /* __MOO_EDIT_WINDOW_PANE_H__ */
