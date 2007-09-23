/*
 *   moocobject.h
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_COBJECT_H
#define MOO_COBJECT_H

#include <glib-object.h>
#ifdef MOO_OBJC_USE_FOUNDATION
#import <Foundation/Foundation.h>
#else
#import <objc/Object.h>
#endif

G_BEGIN_DECLS

#define MOO_UNUSED_VAR(var) ((void)var)

#ifndef MOO_OBJC_USE_FOUNDATION

/* stripped down NSObject protocol, to make it possible to substitute
 * MooCObject with NSObject when it's available */
@protocol MooCObject
- (Class) class;
- (Class) superclass;
- (BOOL) isKindOfClass: (Class)aClass;
- (id) performSelector: (SEL)aSelector;
- (BOOL) respondsToSelector: (SEL)aSelector;

- (id) retain;
- (void) release;
- (id) autorelease;
- (guint) retainCount;
- (void) dealloc;
@end

@interface MooCObject : Object <MooCObject>
{
@private
    guint retainCount;
}
@end

#else // MOO_OBJC_USE_FOUNDATION

#define MooCObject NSObject

#endif // MOO_OBJC_USE_FOUNDATION


void    moo_init_objc_api               (void);

void    moo_objc_push_autorelease_pool  (void);
void    moo_objc_pop_autorelease_pool   (void);


#ifdef CSTR
#warning "CSTR defined"
#endif
#undef CSTR
typedef const char *CSTR;


G_END_DECLS

#endif /* MOO_COBJECT_H */
/* -*- objc -*- */
