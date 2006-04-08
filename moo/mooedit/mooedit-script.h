/*
 *   mooedit-script.h
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

#ifndef __MOO_EDIT_SCRIPT_H__
#define __MOO_EDIT_SCRIPT_H__

#include <mooscript/mooscript-context.h>
#include <mooedit/mooeditor.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_CONTEXT              (moo_edit_context_get_type ())
#define MOO_EDIT_CONTEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_CONTEXT, MooEditContext))
#define MOO_EDIT_CONTEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_CONTEXT, MooEditContextClass))
#define MOO_IS_EDIT_CONTEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_CONTEXT))
#define MOO_IS_EDIT_CONTEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_CONTEXT))
#define MOO_EDIT_CONTEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_CONTEXT, MooEditContextClass))

typedef struct _MooEditContext MooEditContext;
typedef struct _MooEditContextClass MooEditContextClass;


struct _MooEditContext {
    MSContext context;
    MooEdit *doc;
};

struct _MooEditContextClass {
    MSContextClass context_class;
};


GType        moo_edit_context_get_type  (void) G_GNUC_CONST;

MSContext   *moo_edit_context_new       (MooEditWindow  *window);

void         moo_edit_context_set_doc   (MooEditContext *ctx,
                                         MooEdit        *doc);


G_END_DECLS

#endif /* __MOO_EDIT_SCRIPT_H__ */
