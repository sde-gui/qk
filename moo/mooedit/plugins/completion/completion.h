/*
 *   completion.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooedit/mooplugin.h"
#include "mooedit/mootextcompletion.h"

#ifndef __COMPLETION_H__
#define __COMPLETION_H__

G_BEGIN_DECLS


#define CMPL_PLUGIN_ID "Completion"
#define CMPL_PREFS_ROOT MOO_PLUGIN_PREFS_ROOT "/" CMPL_PLUGIN_ID
#define CMPL_DIR "completion"

#define CMPL_FILE_SUFFIX_LIST   ".lst"
#define CMPL_FILE_SUFFIX_CONFIG ".cfg"
#define CMPL_FILE_SUFFIX_PYTHON ".py"

#define DEFAULT_POPUP_TIMEOUT 100

#define _completion_complete                _moo_completion_plugin_complete
#define _cmpl_plugin_load                   _moo_completion_plugin_load
#define _cmpl_plugin_clear                  _moo_completion_plugin_clear
#define _cmpl_plugin_set_lang_completion    _moo_completion_plugin_set_lang_completion
#define _cmpl_plugin_set_doc_completion     _moo_completion_plugin_set_doc_completion
#define _cmpl_plugin_set_focused_doc        _moo_completion_plugin_set_focused_doc
#define _cmpl_plugin_set_auto_complete      _moo_completion_plugin_set_auto_complete


typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
    GQuark cmpl_quark;
    GHashTable *data; /* char* -> CompletionData* */

    guint prefs_notify;
    gboolean auto_complete;
    MooEdit *focused_doc;
    guint popup_timeout;
    int popup_interval;
    gboolean working;
} CmplPlugin;


void        _completion_complete                    (CmplPlugin         *plugin,
                                                     MooEdit            *doc,
                                                     gboolean            automatic);

void        _cmpl_plugin_load                       (CmplPlugin         *plugin);
void        _cmpl_plugin_clear                      (CmplPlugin         *plugin);

void        _cmpl_plugin_set_focused_doc            (CmplPlugin         *plugin,
                                                     MooEdit            *doc);

void        _cmpl_plugin_set_lang_completion        (CmplPlugin         *plugin,
                                                     const char         *lang,
                                                     MooTextCompletion  *cmpl);
void        _cmpl_plugin_set_doc_completion         (CmplPlugin         *plugin,
                                                     MooEdit            *doc,
                                                     MooTextCompletion  *cmpl);


G_END_DECLS

#endif /* __COMPLETION_H__ */
