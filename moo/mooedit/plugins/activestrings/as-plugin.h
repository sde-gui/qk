/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin.h
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

#ifndef __AS_PLUGIN_H__
#define __AS_PLUGIN_H__

G_BEGIN_DECLS


#define AS_XML_ROOT         "ActiveStrings"
#define AS_XML_ITEM         "item"
#define AS_XML_PROP_PATTERN "pattern"
#define AS_XML_PROP_LANG    "lang"
#define AS_XML_PROP_ENABLED "enabled"


typedef void (*ASLoadFunc) (const char   *pattern,
                            const char   *script,
                            const char   *lang,
                            gboolean      enabled,
                            gpointer      data);


GtkWidget *_as_plugin_prefs_page    (MooPlugin  *plugin);
void       _as_plugin_reload        (MooPlugin  *plugin);
void       _as_plugin_load          (MooPlugin  *plugin,
                                     ASLoadFunc  func,
                                     gpointer    data);


G_END_DECLS

#endif /* __AS_PLUGIN_H__ */
