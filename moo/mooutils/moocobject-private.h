/*
 *   moocobject-private.h
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_COBJECT_PRIVATE_H
#define MOO_COBJECT_PRIVATE_H

#import "moocobject.h"

G_BEGIN_DECLS


#ifndef MOO_OBJC_USE_FOUNDATION

@interface MooAutoreleasePool : MooCObject
{
@private
    MooAutoreleasePool *parent;
    MooAutoreleasePool *child;
    GSList *objects;
}

+ (void) addObject: (id)anObj;

- (void) addObject: (id)anObj;
- (void) emptyPool;
@end

#define NSAutoreleasePool MooAutoreleasePool

#endif


G_END_DECLS

#endif /* MOO_COBJECT_PRIVATE_H */
/* -*- objc -*- */
