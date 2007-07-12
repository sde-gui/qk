/*
 *   mooedit-script.h
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

#ifndef MOO_EDIT_SCRIPT_H
#define MOO_EDIT_SCRIPT_H

#include <mooedit/mooedit.h>
#include <mooscript/mooscript-context.h>
#include <gtk/gtkwindow.h>

G_BEGIN_DECLS


@interface MooEditScriptContext : MSContext {
@private
    GtkTextView *doc;
};

+ (MooEditScriptContext*) new:(GtkTextView*) doc
                             :(GtkWindow*) window;
- (void) setDoc:(GtkTextView*) doc;
- (GtkTextView*) doc;
@end


G_END_DECLS

#endif /* MOO_EDIT_SCRIPT_H */
