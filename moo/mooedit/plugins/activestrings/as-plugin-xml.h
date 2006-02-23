/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-xml.h
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

#ifndef __AS_PLUGIN_XML_H__
#define __AS_PLUGIN_XML_H__

#include <glib.h>

G_BEGIN_DECLS


typedef struct _ASInfo ASInfo;

struct _ASInfo {
    char *pattern;
    char *script;
    char *lang;
};


ASInfo     *_as_info_new    (const char *pattern,
                             const char *script,
                             const char *lang);
void        _as_info_free   (ASInfo     *info);

gboolean    _as_load_file   (const char *filename,
                             GSList    **info);
gboolean    _as_save        (const char *filename,
                             GSList     *info);

char       *_as_format_xml  (GSList     *info);


G_END_DECLS

#endif /* __AS_PLUGIN_XML_H__ */
