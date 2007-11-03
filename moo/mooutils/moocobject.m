/*
 *   moocobject.m
 *
 *   Copyright (C) 2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include <config.h>
#import "moocobject-private.h"
#import "mooobjc.h"
#import "mooobjcmarshal.h"
#import <objc/objc-api.h>


static GSList *autorelease_pools;


static void
moo_objc_send_msg (MooObjCObject *m_obj,
                   const char    *name)
{
    SEL aSelector;
    id<MooCObject> obj = (id) m_obj;

    g_return_if_fail (obj != NULL);
    g_return_if_fail (name != NULL);

    aSelector = sel_get_any_uid (name);
    g_return_if_fail (aSelector != NULL);
    g_return_if_fail ([obj respondsToSelector:aSelector]);

    [obj performSelector:aSelector];
}

static MooObjCObject *
moo_objc_retain (MooObjCObject *m_obj)
{
    id<MooCObject> obj = (id) m_obj;
    g_return_val_if_fail (obj != NULL, NULL);
    return (MooObjCObject*)[obj retain];
}

static MooObjCObject *
moo_objc_autorelease (MooObjCObject *m_obj)
{
    id<MooCObject> obj = (id) m_obj;
    g_return_val_if_fail (obj != NULL, NULL);
    return (MooObjCObject*)[obj autorelease];
}

static void
moo_objc_release (MooObjCObject *m_obj)
{
    id<MooCObject> obj = (id) m_obj;
    g_return_if_fail (obj != NULL);
    [obj release];
}

static char *
moo_objc_get_info (void)
{
    return g_strdup ("Objective-C");
}

void
moo_init_objc_api (void)
{
    static MooObjCAPI api = {
        moo_objc_retain,
        moo_objc_autorelease,
        moo_objc_release,
        moo_objc_get_info,
        moo_objc_send_msg,
        moo_objc_push_autorelease_pool,
        moo_objc_pop_autorelease_pool
    };

    static gboolean been_here = FALSE;

    if (!been_here)
    {
        been_here = TRUE;
        moo_objc_init (MOO_OBJC_API_VERSION, &api);
    }
}


void
moo_objc_push_autorelease_pool (void)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    g_return_if_fail (pool != nil);
    autorelease_pools = g_slist_prepend (autorelease_pools, pool);
}

void
moo_objc_pop_autorelease_pool (void)
{
    NSAutoreleasePool *pool;

    g_return_if_fail (autorelease_pools != NULL);

    pool = autorelease_pools->data;
    autorelease_pools = g_slist_delete_link (autorelease_pools, autorelease_pools);
    [pool release];
}


#ifndef MOO_OBJC_USE_FOUNDATION

@implementation MooCObject

+ initialize
{
    moo_init_objc_api ();
    return self;
}

- init
{
    [super init];
    retainCount = 1;
    return self;
}

- (Class) class
{
    return [super class];
}

- (Class) superclass
{
    return [super superClass];
}

- (BOOL) isKindOfClass: (Class)aClass
{
    return [super isKindOf:aClass];
}

- (id) performSelector: (SEL)aSelector
{
    return [super perform:aSelector];
}

- (BOOL) respondsToSelector: (SEL)aSelector
{
    return [super respondsTo:aSelector];
}

- (id) retain
{
    retainCount += 1;
    return self;
}

- (void) release
{
    if (!--retainCount)
        [self dealloc];
}

- (id) autorelease
{
    [MooAutoreleasePool addObject:self];
    return self;
}

- (guint) retainCount
{
    return retainCount;
}

- (void) dealloc
{
    [self free];
}

@end


@implementation MooAutoreleasePool

static MooAutoreleasePool *currentPool;

+ (void) addObject: (id)anObj
{
    g_return_if_fail (currentPool != nil);
    [currentPool addObject: anObj];
}

- (void) addObject: (id)anObj
{
    objects = g_slist_prepend (objects, anObj);
}

- (void) emptyPool
{
    GSList *list, *l;

    list = objects;
    objects = NULL;

    for (l = list; l != NULL; l = l->next)
    {
        id<MooCObject> obj = l->data;
        [obj release];
    }

    g_slist_free (list);
}

- (id) autorelease
{
    g_return_val_if_reached (self);
}

- init
{
    [super init];

    if (currentPool)
    {
        currentPool->child = self;
        parent = currentPool;
    }

    currentPool = self;

    return self;
}

- (void) dealloc
{
    currentPool = parent;

    if (currentPool)
        currentPool->child = nil;

    [self emptyPool];

    [super dealloc];
}

@end

#endif // !MOO_OBJC_USE_FOUNDATION


typedef enum {
    CG_SIGNAL_RETAIN_DATA   = 1 << 0,
    CG_SIGNAL_SWAP_DATA     = 1 << 1,
    CG_SIGNAL_CONNECT_AFTER = 1 << 2
} CGSignalFlags;

typedef struct {
    GClosure closure;
    MooCObject *obj;
    SEL sel;
    MooCObject *data;
    guint has_data : 1;
    guint owns_data : 1;
    guint swap : 1;
} CGClosureSel;

static void
cg_closure_sel_marshal (CGClosureSel *closure,
                        GValue       *return_value,
                        guint         n_params,
                        const GValue *params)
{
    IMP callback;

    /* TODO: use NSInvocation to enable forwarding */

    callback = get_imp ([closure->obj class], closure->sel);
    g_return_if_fail (callback != NULL);

    _moo_objc_marshal (G_CALLBACK (callback), closure->obj, closure->sel,
                       return_value, n_params, params,
                       closure->data, closure->has_data, closure->swap);
}

static void
cg_closure_sel_invalidate (G_GNUC_UNUSED gpointer data,
                           CGClosureSel *cg)
{
    if (cg->owns_data)
    {
        [cg->data release];
        cg->data = nil;
    }
}

static GClosure *
_cg_closure_new_sel (MooCObject   *target,
                     SEL           sel,
                     MooCObject   *data,
                     CGSignalFlags sig_flags,
                     gboolean      use_data)
{
    GClosure *closure;
    CGClosureSel *cg;

    g_return_val_if_fail (target != nil, NULL);
    g_return_val_if_fail (sel != NULL, NULL);
    g_return_val_if_fail (data == nil || use_data, NULL);
    g_return_val_if_fail (data != nil || !(sig_flags & CG_SIGNAL_RETAIN_DATA), NULL);

    if (![target respondsToSelector:sel])
    {
#ifndef MOO_OBJC_USE_FOUNDATION
        const char *desc = "<unknown>";
#else
        const char *desc = [[target description] cString];
#endif
        g_critical ("in %s: object %s does not respond to selector '%s'",
                    G_STRLOC, desc ? desc : "<NULL>", sel_get_name (sel));
        return NULL;
    }

    closure = g_closure_new_simple (sizeof (CGClosureSel), NULL);
    g_closure_add_invalidate_notifier (closure, NULL,
                                       (GClosureNotify) cg_closure_sel_invalidate);
    g_closure_set_marshal(closure, (GClosureMarshal) cg_closure_sel_marshal);

    cg = (CGClosureSel*) closure;
    cg->obj = target;
    cg->sel = sel;
    cg->data = data;
    cg->has_data = use_data ? 1 : 0;
    cg->swap = (sig_flags & CG_SIGNAL_SWAP_DATA) ? 1 : 0;

    if (sig_flags & CG_SIGNAL_RETAIN_DATA)
    {
        [cg->data retain];
        cg->owns_data = TRUE;
    }

    return closure;
}

static gulong
_cg_signal_connect_sel (GObject       *obj,
                        const char    *signal,
                        MooCObject    *target,
                        SEL            sel,
                        CGSignalFlags  flags)
{
    gulong cb_id = 0;
    GClosure *closure;

    closure = _cg_closure_new_sel (target, sel, nil, flags, FALSE);

    if (closure)
    {
        g_closure_sink (g_closure_ref (closure));
        cb_id = g_signal_connect_closure (obj, signal, closure,
                                          (flags & CG_SIGNAL_CONNECT_AFTER) ? 1 : 0);
        g_closure_unref (closure);
    }

    return cb_id;
}

gulong
moo_objc_signal_connect (gpointer    instance,
                         const char *signal,
                         MooCObject *target,
                         SEL         sel)
{
    g_return_val_if_fail (G_IS_OBJECT (instance), 0);
    g_return_val_if_fail (signal != NULL, 0);
    g_return_val_if_fail (target != nil, 0);
    g_return_val_if_fail (sel != 0, 0);

    return _cg_signal_connect_sel (instance, signal, target, sel, 0);
}


/* -*- objc -*- */
