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

#include "mooutils/mooscript/mooscript-context.h"
#include "mooutils/mooscript/mooscript-node.h"
#include "mooedit/mooedit.h"

G_BEGIN_DECLS


#define AS_TYPE_PLUGIN_CONTEXT              (_as_plugin_context_get_type ())
#define AS_PLUGIN_CONTEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), AS_TYPE_PLUGIN_CONTEXT, ASPluginContext))
#define AS_PLUGIN_CONTEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), AS_TYPE_PLUGIN_CONTEXT, ASPluginContextClass))
#define AS_IS_PLUGIN_CONTEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), AS_TYPE_PLUGIN_CONTEXT))
#define AS_IS_PLUGIN_CONTEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), AS_TYPE_PLUGIN_CONTEXT))
#define AS_PLUGIN_CONTEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), AS_TYPE_PLUGIN_CONTEXT, ASPluginContextClass))

typedef struct _ASPluginContext ASPluginContext;
typedef struct _ASPluginContextClass ASPluginContextClass;


struct _ASPluginContext {
    ASContext context;
    MooEdit *doc;
};

struct _ASPluginContextClass {
    ASContextClass context_class;
};


GType        _as_plugin_context_get_type    (void) G_GNUC_CONST;

ASContext   *_as_plugin_context_new         (void);

gboolean     _as_plugin_context_exec        (ASContext      *ctx,
                                             ASNode         *script,
                                             MooEdit        *doc,
                                             GtkTextIter    *insert,
                                             char           *match,
                                             char          **parens,
                                             guint           n_parens);


G_END_DECLS

#endif /* __AS_PLUGIN_SCRIPT_H__ */
