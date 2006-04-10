/*
 *   moocompletion.h
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

#include "mooedit/mooplugin.h"

#ifndef __MOO_COMPLETION_H__
#define __MOO_COMPLETION_H__

G_BEGIN_DECLS


#define CMPL_PLUGIN_ID "Completion"
#define CMPL_PREFS_ROOT MOO_PLUGIN_PREFS_ROOT "/" CMPL_PLUGIN_ID
#define CMPL_DIR "completion"
#define CMPL_FILE_NONE "none"
#define CMPL_FILE_SUFFIX ".lst"


typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
    GHashTable *data; /* char* -> CompletionData* */
} CmplPlugin;


GtkWidget  *_cmpl_plugin_prefs_page     (MooPlugin      *plugin);

void        _moo_completion_callback    (MooEditWindow  *window);
void        _moo_completion_complete    (CmplPlugin     *plugin,
                                         MooEdit        *doc);

void        _cmpl_plugin_load           (CmplPlugin     *plugin);
void        _cmpl_plugin_clear          (CmplPlugin     *plugin);


G_END_DECLS

#endif /* __AS_PLUGIN_H__ */
