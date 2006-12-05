/*
 *   completion.h
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

#ifndef __COMPLETION_H__
#define __COMPLETION_H__

G_BEGIN_DECLS


#define CMPL_PLUGIN_ID "Completion"
#define CMPL_PREFS_ROOT MOO_PLUGIN_PREFS_ROOT "/" CMPL_PLUGIN_ID
#define CMPL_DIR "completion"
#define CMPL_FILE_NONE "none"

#define CMPL_FILE_SUFFIX_LIST   ".lst"
#define CMPL_FILE_SUFFIX_CONFIG ".cfg"
#define CMPL_FILE_SUFFIX_PYTHON ".py"

#define _completion_callback        _moo_completion_plugin_callback
#define _completion_complete        _moo_completion_plugin_complete
#define _cmpl_plugin_load           _moo_completion_plugin_load
#define _cmpl_plugin_clear          _moo_completion_plugin_clear
#define _cmpl_plugin_prefs_page     _moo_completion_plugin_prefs_page


typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
    GHashTable *data; /* char* -> CompletionData* */
} CmplPlugin;


GtkWidget  *_cmpl_plugin_prefs_page     (MooPlugin      *plugin);

void        _completion_callback        (MooEditWindow  *window);
void        _completion_complete        (CmplPlugin     *plugin,
                                         MooEdit        *doc);

void        _cmpl_plugin_load           (CmplPlugin     *plugin);
void        _cmpl_plugin_clear          (CmplPlugin     *plugin);


G_END_DECLS

#endif /* __COMPLETION_H__ */
