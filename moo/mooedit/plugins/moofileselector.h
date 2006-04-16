/*
 *   moofileselector.h
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

#ifndef __MOO_FILE_SELECTOR_H__
#define __MOO_FILE_SELECTOR_H__

#include "mooedit/mooeditwindow.h"
#include "moofileview/moofileview.h"

G_BEGIN_DECLS


#define MOO_TYPE_FILE_SELECTOR              (moo_file_selector_get_type ())
#define MOO_FILE_SELECTOR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FILE_SELECTOR, MooFileSelector))
#define MOO_FILE_SELECTOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FILE_SELECTOR, MooFileSelectorClass))
#define MOO_IS_FILE_SELECTOR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FILE_SELECTOR))
#define MOO_IS_FILE_SELECTOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FILE_SELECTOR))
#define MOO_FILE_SELECTOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FILE_SELECTOR, MooFileSelectorClass))


typedef struct _MooFileSelector         MooFileSelector;
typedef struct _MooFileSelectorClass    MooFileSelectorClass;

struct _MooFileSelector
{
    MooFileView parent;

    MooEditWindow *window;
    GtkWidget *button;
    guint open_pane_timeout;
    gboolean button_highlight;

    GtkTargetList *targets;
    gboolean waiting_for_tab;
};

struct _MooFileSelectorClass
{
    MooFileViewClass parent_class;
};


GType       moo_file_selector_get_type                      (void) G_GNUC_CONST;
GtkWidget  *moo_file_selector_new                           (void);


G_END_DECLS

#endif /* __MOO_FILE_SELECTOR_H__ */
