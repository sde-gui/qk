/*
 *   as-plugin-script.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __AS_PLUGIN_SCRIPT_H__
#define __AS_PLUGIN_SCRIPT_H__

#include <mooscript/mooscript-context.h>
#include <mooscript/mooscript-node.h>
#include <mooedit/mooedit.h>

G_BEGIN_DECLS


#define _as_plugin_context_exec _moo_as_plugin_context_exec


gboolean     _as_plugin_context_exec        (MSContext      *ctx,
                                             MSNode         *script,
                                             MooEdit        *doc,
                                             GtkTextIter    *insert,
                                             char           *match,
                                             char          **parens,
                                             guint           n_parens);


G_END_DECLS

#endif /* __AS_PLUGIN_SCRIPT_H__ */
