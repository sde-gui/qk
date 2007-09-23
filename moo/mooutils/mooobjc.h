/*
 *   mooobjc.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_OBJC_H
#define MOO_OBJC_H

#include <glib.h>

G_BEGIN_DECLS

#define MOO_OBJC_API_VERSION 1

typedef struct _MooObjCAPI MooObjCAPI;
typedef struct _MooObjCObject MooObjCObject;

struct _MooObjCAPI {
    MooObjCObject  *(*retain)                   (MooObjCObject *obj);
    MooObjCObject  *(*autorelease)              (MooObjCObject *obj);
    void            (*release)                  (MooObjCObject *obj);

    char           *(*get_info)                 (void);

    void            (*send_msg)                 (MooObjCObject *obj,
                                                 const char    *name);

    void            (*push_autorelease_pool)    (void);
    void            (*pop_autorelease_pool)     (void);
};

extern MooObjCAPI *moo_objc_api;

gboolean     moo_objc_init          (guint           version,
                                     MooObjCAPI     *api);

#ifndef __OBJC__

#define moo_objc_present() (moo_objc_api != NULL)

#define moo_objc_get_info               moo_objc_api->get_info
#define moo_objc_retain                 moo_objc_api->retain
#define moo_objc_release                moo_objc_api->release
#define moo_objc_autorelease            moo_objc_api->autorelease
#define moo_objc_send_msg               moo_objc_api->send_msg
#define moo_objc_push_autorelease_pool  moo_objc_api->push_autorelease_pool
#define moo_objc_pop_autorelease_pool   moo_objc_api->pop_autorelease_pool

#endif /* __OBJC__ */


G_END_DECLS

#endif /* MOO_OBJC_H */
