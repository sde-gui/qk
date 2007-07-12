/*
 *   mooscript-context-private.h
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

#ifndef MOO_SCRIPT_CONTEXT_PRIVATE_H
#define MOO_SCRIPT_CONTEXT_PRIVATE_H

#include "mooscript-context.h"

G_BEGIN_DECLS


@interface MSContext (MSContextPrivate)
- (MSVariable*) lookupVar:(CSTR)name;

- (void) print:(CSTR) string;

- (void) setReturn:(MSValue*) val;
- (MSValue*) getReturn;

- (void) setBreak;
- (void) setContinue;
- (void) unsetReturn;
- (void) unsetBreak;
- (void) unsetContinue;

- (BOOL) returnSet;
- (BOOL) breakSet;
- (BOOL) continueSet;
- (BOOL) errorSet;
@end


void _ms_context_add_builtin (MSContext *ctx);


G_END_DECLS

#endif /* MOO_SCRIPT_CONTEXT_PRIVATE_H */
