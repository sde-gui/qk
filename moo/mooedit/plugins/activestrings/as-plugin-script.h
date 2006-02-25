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

#ifndef __MS_PLUGIN_SCRIPT_H__
#define __MS_PLUGIN_SCRIPT_H__

#include "mooutils/mooscript/mooscript-context.h"
#include "mooutils/mooscript/mooscript-node.h"
#include "mooedit/mooedit.h"

G_BEGIN_DECLS


#define MS_TYPE_PLUGIN_CONTEXT              (_ms_plugin_context_get_type ())
#define MS_PLUGIN_CONTEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MS_TYPE_PLUGIN_CONTEXT, MSPluginContext))
#define MS_PLUGIN_CONTEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MS_TYPE_PLUGIN_CONTEXT, MSPluginContextClass))
#define MS_IS_PLUGIN_CONTEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MS_TYPE_PLUGIN_CONTEXT))
#define MS_IS_PLUGIN_CONTEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MS_TYPE_PLUGIN_CONTEXT))
#define MS_PLUGIN_CONTEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MS_TYPE_PLUGIN_CONTEXT, MSPluginContextClass))

typedef struct _MSPluginContext MSPluginContext;
typedef struct _MSPluginContextClass MSPluginContextClass;


struct _MSPluginContext {
    MSContext context;
    MooEdit *doc;
};

struct _MSPluginContextClass {
    MSContextClass context_class;
};


GType        _ms_plugin_context_get_type    (void) G_GNUC_CONST;

MSContext   *_ms_plugin_context_new         (void);

gboolean     _ms_plugin_context_exec        (MSContext      *ctx,
                                             MSNode         *script,
                                             MooEdit        *doc,
                                             GtkTextIter    *insert,
                                             char           *match,
                                             char          **parens,
                                             guint           n_parens);


G_END_DECLS

#endif /* __MS_PLUGIN_SCRIPT_H__ */
