/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooeditprefspage.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditprefs-glade.h"
#include "mooedit/mooeditcolorsprefs-glade.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooglade.h"
#include <string.h>


GtkWidget*
moo_edit_prefs_page_new (MooEditor *editor)
{
    GtkWidget *page;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    _moo_edit_set_default_settings ();

    page = moo_prefs_dialog_page_new_from_xml ("Editor",
                                               GTK_STOCK_EDIT,
                                               MOO_EDIT_PREFS_GLADE_UI,
                                               -1,
                                               "page",
                                               MOO_EDIT_PREFS_PREFIX);
    return page;
}


GtkWidget*
moo_edit_colors_prefs_page_new (MooEditor      *editor)
{
    GtkWidget *page;

    g_return_val_if_fail (MOO_IS_EDITOR (editor), NULL);

    page = moo_prefs_dialog_page_new_from_xml ("Editor Colors",
                                               GTK_STOCK_EDIT,
                                               MOO_EDIT_COLORS_PREFS_GLADE_UI,
                                               -1,
                                               "page",
                                               MOO_EDIT_PREFS_PREFIX);
    return page;
}
