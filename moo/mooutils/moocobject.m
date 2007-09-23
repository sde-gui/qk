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
#import "moocobject.h"
#import "mooobjc.h"
#import <objc/objc-api.h>


static GSList *autorelease_pools;


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

#else

typedef NSAutoreleasePool MooAutoreleasePool;

#endif


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
    MooAutoreleasePool *pool = [[MooAutoreleasePool alloc] init];
    g_return_if_fail (pool != nil);
    autorelease_pools = g_slist_prepend (autorelease_pools, pool);
}

void
moo_objc_pop_autorelease_pool (void)
{
    MooAutoreleasePool *pool;

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

#if 0

#ifdef MOO_OS_DARWIN
#define class_get_class_name(klass) (klass->name)
#define sel_get_name sel_getName
#endif

#define OBJECT_HAS_TOGGLE_REF_FLAG 1
#define OBJECT_HAS_TOGGLE_REF() ((g_datalist_get_flags (&moo_c_object_qdata) & OBJECT_HAS_TOGGLE_REF_FLAG) != 0)

static GQuark quark_toggle_refs = 0;

typedef struct {
  guint n_toggle_refs;
  struct {
    MooToggleNotify notify;
    gpointer    data;
  } toggle_refs[1];  /* flexible array */
} ToggleRefStack;

static void
toggle_refs_notify (MooCObject *object,
                    GData     **qdata,
		    gboolean    is_last_ref)
{
    ToggleRefStack *tstack = g_datalist_id_get_data (qdata, quark_toggle_refs);
    g_assert (tstack->n_toggle_refs == 1);
    tstack->toggle_refs[0].notify (tstack->toggle_refs[0].data, object, is_last_ref);
}

@implementation MooCObject : Object

+ initialize
{
    quark_toggle_refs = g_quark_from_static_string ("moo-object-toggle-refs");
    return self;
}

#ifndef MOO_OS_DARWIN
- (retval_t) forward :(SEL)aSel :(arglist_t)argFrame
{
    MOO_UNUSED_VAR (argFrame);
    g_critical ("no method `%s' found in <%s at %p>",
                sel_get_name (aSel), [self name], (gpointer) self);
    return NULL;
}
#else
- forward: (SEL)aSel :(marg_list)args
{
    MOO_UNUSED_VAR (args);
    g_critical ("no method `%s' found in <%s at %p>",
                sel_get_name (aSel), [self name], (gpointer) self);
    return nil;
}
#endif

- init
{
    moo_c_object_ref_count = 1;
    moo_c_object_qdata = NULL;
    return [super init];
}

- (void) dealloc
{
    if (moo_c_object_qdata)
        g_datalist_clear (&moo_c_object_qdata);
    [self free];
}

- free
{
    return [super free];
}

- retain
{
    g_assert (moo_c_object_ref_count > 0);

    moo_c_object_ref_count++;

    if (moo_c_object_ref_count == 2 && OBJECT_HAS_TOGGLE_REF ())
        toggle_refs_notify (self, &moo_c_object_qdata, FALSE);

    return self;
}

- (void) release
{
    gboolean need_dealloc;

    g_assert (moo_c_object_ref_count > 0);

    moo_c_object_ref_count--;
    need_dealloc = !moo_c_object_ref_count;

    if (moo_c_object_ref_count == 1 && OBJECT_HAS_TOGGLE_REF ())
        toggle_refs_notify (self, &moo_c_object_qdata, TRUE);

    if (need_dealloc)
        [self dealloc];
}

- autorelease
{
    [MooAutoreleasePool addObject: self];
    return self;
}

- (guint) retainCount
{
    return moo_c_object_ref_count;
}

- (void) setQData :(GQuark)key
                  :(gpointer)data
{
    g_datalist_id_set_data (&moo_c_object_qdata, key, data);
}

- (void) setQData :(GQuark)key :(gpointer)data withDestroy:(GDestroyNotify)destroy
{
    g_datalist_id_set_data_full (&moo_c_object_qdata, key, data, destroy);
}

- (gpointer) getQData :(GQuark)key
{
    return g_datalist_id_get_data (&moo_c_object_qdata, key);
}

- (void) setData :(CSTR)key :(gpointer)data
{
    g_datalist_set_data (&moo_c_object_qdata, key, data);
}

- (void) setData :(CSTR)key :(gpointer)data withDestroy:(GDestroyNotify)destroy
{
    g_datalist_set_data_full (&moo_c_object_qdata, key, data, destroy);
}

- (gpointer) getData: (CSTR)key
{
    return g_datalist_get_data (&moo_c_object_qdata, key);
}

- (void) addToggleRef :(MooToggleNotify)notify
                      :(gpointer)data
{
    ToggleRefStack *tstack;
    guint i;

    g_return_if_fail (notify != NULL);
    g_return_if_fail (moo_c_object_ref_count >= 1);

    [self retain];

    tstack = g_datalist_id_remove_no_notify (&moo_c_object_qdata, quark_toggle_refs);
    if (tstack)
    {
        i = tstack->n_toggle_refs++;
        /* allocate i = tstate->n_toggle_refs - 1 positions beyond the 1 declared
        * in tstate->toggle_refs */
        tstack = g_realloc (tstack, sizeof (*tstack) + sizeof (tstack->toggle_refs[0]) * i);
    }
    else
    {
        tstack = g_renew (ToggleRefStack, NULL, 1);
        tstack->n_toggle_refs = 1;
        i = 0;
    }

    /* Set a flag for fast lookup after adding the first toggle reference */
    if (tstack->n_toggle_refs == 1)
        g_datalist_set_flags (&moo_c_object_qdata, OBJECT_HAS_TOGGLE_REF_FLAG);

    tstack->toggle_refs[i].notify = notify;
    tstack->toggle_refs[i].data = data;
    g_datalist_id_set_data_full (&moo_c_object_qdata, quark_toggle_refs, tstack,
                                 (GDestroyNotify) g_free);
}

- (void) removeToggleRef :(MooToggleNotify)notify
                         :(gpointer)data
{
    ToggleRefStack *tstack;
    gboolean found_one = FALSE;

    g_return_if_fail (notify != NULL);

    tstack = g_datalist_id_get_data (&moo_c_object_qdata, quark_toggle_refs);

    if (tstack)
    {
        guint i;

        for (i = 0; i < tstack->n_toggle_refs; i++)
            if (tstack->toggle_refs[i].notify == notify &&
                tstack->toggle_refs[i].data == data)
            {
                found_one = TRUE;
                tstack->n_toggle_refs -= 1;

                if (i != tstack->n_toggle_refs)
                    tstack->toggle_refs[i] = tstack->toggle_refs[tstack->n_toggle_refs];

                if (tstack->n_toggle_refs == 0)
                    g_datalist_unset_flags (&moo_c_object_qdata, OBJECT_HAS_TOGGLE_REF_FLAG);

                [self release];

                break;
            }
    }

    if (!found_one)
        g_warning ("%s: couldn't find toggle ref %p(%p)", G_STRFUNC, notify, data);
}

@end


@implementation MooAutoreleasePool : MooCObject

static MooAutoreleasePool *current_pool;

+ (void) addObject: (id)anObj
{
    g_return_if_fail (current_pool != nil);
    [current_pool addObject: anObj];
}

+ (id) currentPool
{
    return current_pool;
}

- (void) addObject: (id)anObj
{
    if (G_UNLIKELY (!objects))
        objects = g_ptr_array_new ();
    g_ptr_array_add (objects, anObj);
}

- (void) emptyPool
{
    [self retain];

    if (child)
        [child emptyPool];

    if (objects)
    {
        id *objs;
        guint n_objs, i;

        n_objs = objects->len;
        objs = (id*) g_ptr_array_free (objects, FALSE);
        objects = NULL;

        for (i = 0; i < n_objs; ++i)
            [objs[i] release];
        g_free (objs);
    }

    [self release];
}

- (id) autorelease
{
    g_return_val_if_reached (self);
}

- init
{
    [super init];

    if (current_pool)
    {
        current_pool->child = self;
        parent = current_pool;
        current_pool = self;
    }

    current_pool = self;

    return self;
}

- (void) free
{
    current_pool = parent;

    if (current_pool)
        current_pool->child = nil;

    [self emptyPool];
    [super free];
}

@end


static gpointer
moo_object_retain (gpointer obj)
{
    g_return_val_if_fail (obj != NULL, NULL);
    return [(id)obj retain];
}

static gpointer
moo_object_copy (gpointer obj)
{
    g_return_val_if_fail (obj != NULL, NULL);
    return [(id)obj copy];
}

static void
moo_object_release (gpointer obj)
{
    g_return_if_fail (obj != NULL);
    [(id)obj release];
}


GType
moo_cboxed_type_new (Class    klass,
                     gboolean copy)
{
    g_return_val_if_fail (klass != Nil, 0);

    return g_boxed_type_register_static (class_get_class_name (klass),
                                         copy ? moo_object_copy : moo_object_retain,
                                         moo_object_release);
}


id
moo_cobject_check_type_cast (id obj, Class klass)
{
    if (!obj)
        g_warning ("invalid cast of (nil) to `%s'",
                   class_get_class_name (klass));
    else if (![obj isKindOf :klass])
        g_warning ("invalid cast from `%s' to `%s'",
                   [obj name], class_get_class_name (klass));

    return obj;
}

BOOL
moo_cobject_check_type (id obj, Class klass)
{
    return obj != nil && [obj isKindOf :klass];
}

#endif


/* -*- objc -*- */
