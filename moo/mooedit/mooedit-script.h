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

#include <mooedit/mooedit.h>
#include <mooscript/mooscript-context.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


#define MOO_TYPE_EDIT_SCRIPT_CONTEXT              (moo_edit_script_context_get_type ())
#define MOO_EDIT_SCRIPT_CONTEXT(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_EDIT_SCRIPT_CONTEXT, MooEditScriptContext))
#define MOO_EDIT_SCRIPT_CONTEXT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_EDIT_SCRIPT_CONTEXT, MooEditScriptContextClass))
#define MOO_IS_EDIT_SCRIPT_CONTEXT(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_EDIT_SCRIPT_CONTEXT))
#define MOO_IS_EDIT_SCRIPT_CONTEXT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_EDIT_SCRIPT_CONTEXT))
#define MOO_EDIT_SCRIPT_CONTEXT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_EDIT_SCRIPT_CONTEXT, MooEditScriptContextClass))

typedef struct _MooEditScriptContext MooEditScriptContext;
typedef struct _MooEditScriptContextClass MooEditScriptContextClass;


struct _MooEditScriptContext {
    MSContext base;
    GtkTextView *doc;
};

struct _MooEditScriptContextClass {
    MSContextClass base_class;
};


GType        moo_edit_script_context_get_type   (void) G_GNUC_CONST;

MSContext   *moo_edit_script_context_new        (GtkTextView    *doc,
                                                 GtkWindow      *window);


// MSContext   *moo_text_context_new       (GtkTextView    *doc);
// MSContext   *moo_edit_script_context_new       (MooEdit        *doc,
//                                          MooEditWindow  *window);
//
// void         moo_edit_setup_command     (MooCommand     *cmd,
//                                          MooEdit        *doc,
//                                          MooEditWindow  *window);


G_END_DECLS

#endif /* __MOO_EDIT_SCRIPT_H__ */
