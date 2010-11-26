/*
 *   mooeditview-private.h
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

#ifndef MOO_EDIT_VIEW_PRIVATE_H
#define MOO_EDIT_VIEW_PRIVATE_H

#include <mooedit/mooeditview-impl.h>

G_BEGIN_DECLS

#define PROGRESS_TIMEOUT    100
#define PROGRESS_WIDTH      300
#define PROGRESS_HEIGHT     100

struct MooEditViewPrivate
{
    MooEdit *doc;
    MooEditBuffer *buffer;

    /***********************************************************************/
    /* Progress dialog
     */
    guint progress_timeout;
    GtkWidget *progress;
    GtkWidget *progressbar;
    char *progress_text;
    GDestroyNotify cancel_op;
    gpointer cancel_data;
};

G_END_DECLS

#endif /* MOO_EDIT_VIEW_PRIVATE_H */
