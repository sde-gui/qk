/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   as-plugin-script.h
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

#ifndef __AS_PLUGIN_SCRIPT_H__
#define __AS_PLUGIN_SCRIPT_H__

#include <mooscript/mooscript-context.h>
#include <mooscript/mooscript-node.h>
#include <mooedit/mooedit.h>

G_BEGIN_DECLS


MSContext   *_as_plugin_context_new         (void);
gboolean     _as_plugin_context_exec        (MSContext      *ctx,
                                             MSNode         *script,
                                             MooEdit        *doc,
                                             GtkTextIter    *insert,
                                             char           *match,
                                             char          **parens,
                                             guint           n_parens);


G_END_DECLS

#endif /* __AS_PLUGIN_SCRIPT_H__ */
