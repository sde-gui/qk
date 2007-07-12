/*
 *   moocobject.h
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

/* Objective-C objects with GObject goodies: qdata and toggle refs */

#ifndef MOO_COBJECT_H
#define MOO_COBJECT_H

#include <glib-object.h>

#ifdef __OBJC__

#import <objc/objc-api.h>
#import <objc/Object.h>

#define MOO_UNUSED_VAR(var) ((void)var)

#ifdef CSTR
#ifdef __GNUC__
#warning "CSTR defined"
#endif
#endif
#undef CSTR
typedef const char *CSTR;

@class MooCObject;

typedef void (*MooToggleNotify) (gpointer    data,
			         MooCObject *object,
			         BOOL        is_last_ref);

@interface MooCObject : Object {
@private
    unsigned moo_c_object_ref_count;
    GData *moo_c_object_qdata;
}

- (void) setQData :(GQuark)key
                  :(gpointer)data;
- (void) setQData :(GQuark)key
                  :(gpointer)data
       withDestroy:(GDestroyNotify)destroy;
- (gpointer) getQData :(GQuark)key;
- (void) setData :(CSTR)key
                 :(gpointer)data;
- (void) setData :(CSTR)key
                 :(gpointer)data
      withDestroy:(GDestroyNotify)destroy;
- (gpointer) getData :(CSTR)key;

- (void) addToggleRef :(MooToggleNotify)notify
                      :(gpointer)data;
- (void) removeToggleRef :(MooToggleNotify)notify
                         :(gpointer)data;

- retain;
- (void) release;
- autorelease;
- (guint) retainCount;
- (void) dealloc;
@end


@interface MooAutoreleasePool : MooCObject
{
@private
    MooAutoreleasePool *parent;
    MooAutoreleasePool *child;
    GPtrArray *objects;
}

+ (void) addObject: (id)anObj;
+ (id) currentPool;

- (void) addObject: (id)anObj;
- (void) emptyPool;
@end


GType       moo_cboxed_type_new     (Class      klass,
                                     gboolean   copy);

#define MOO_DEFINE_CBOXED_TYPE(copy)                    \
+ (GType) get_boxed_type                                \
{                                                       \
    static GType type;                                  \
    if (G_UNLIKELY (!type))                             \
        type = moo_cboxed_type_new ([self class], copy);\
    return type;                                        \
}

id moo_cobject_check_type_cast (id obj, Class klass);
BOOL moo_cobject_check_type (id obj, Class klass);

#ifndef G_DISABLE_CAST_CHECKS
#define MOO_COBJECT_CHECK_TYPE_CAST(obj,ClassName) ((ClassName*) moo_cobject_check_type_cast (obj, [ClassName class]))
#define MOO_COBJECT_CHECK_TYPE(obj,ClassName) (moo_cobject_check_type (obj, [ClassName class]))
#else /* G_DISABLE_CAST_CHECKS */
#define MOO_COBJECT_CHECK_TYPE_CAST(obj,ClassName) ((ClassName*)obj)
#define MOO_COBJECT_CHECK_TYPE(obj,ClassName) (obj != nil)
#endif /* G_DISABLE_CAST_CHECKS */

#endif /* __OBJC__ */

#endif /* MOO_COBJECT_H */
/* -*- objc -*- */
