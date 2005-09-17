/*
 *   mooutils/bind.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/bind.h"


static void button_clicked (GtkButton *btn, GtkEntry *entry)
{
    MooQueryTextFunc func = (MooQueryTextFunc)g_object_get_data (G_OBJECT (btn), "moo_query_text_func");
    MooTransformTextFunc text_func = (MooTransformTextFunc)g_object_get_data (G_OBJECT (btn), "moo_transform_text_func");
    void *data = g_object_get_data (G_OBJECT (btn), "moo_bind_button_data");
    const char *text = func (data, GTK_WIDGET (btn));
    if (!text) return;
    if (text_func) {
        char *transformed = text_func (text, data);
        gtk_entry_set_text (entry, transformed);
        g_free (transformed);
    }
    else
        gtk_entry_set_text (entry, text);
}


void        moo_bind_button     (GtkButton                  *button,
                                 GtkEntry                   *entry,
                                 MooQueryTextFunc            func,
                                 MooTransformTextFunc        text_func,
                                 gpointer                    data)
{
    g_return_if_fail (button != NULL && entry != NULL && func != NULL);
    g_object_set_data (G_OBJECT (button), "moo_query_text_func", (gpointer)func);
    g_object_set_data (G_OBJECT (button), "moo_transform_text_func", (gpointer)text_func);
    g_object_set_data (G_OBJECT (button), "moo_bind_button_data", data);
    g_signal_connect (button, "clicked", G_CALLBACK (button_clicked), entry);
}


char       *moo_quote_text      (const char *text,
                                 G_GNUC_UNUSED gpointer data)
{
    return text ? g_strdup_printf ("\"%s\"", text) : NULL;
}


/****************************************************************************/
/* BindProperty
 */

typedef struct {
    GObject *source;
    GObject *target;
    GParamSpec *source_prop;
    GParamSpec *target_prop;
    gboolean alive;
    MooTransformPropFunc transform;
    gpointer data;
    GDestroyNotify destroy_notify;
} PropWatcher;


static void source_died         (PropWatcher *watcher);
static void target_died         (PropWatcher *watcher);
static void watcher_check       (PropWatcher *watcher);
static void watcher_die         (PropWatcher *watcher);


static PropWatcher*
prop_watcher_new (GObject            *target,
                  const char         *target_prop,
                  GObject            *source,
                  const char         *source_prop,
                  MooTransformPropFunc transform,
                  gpointer            transform_data,
                  GDestroyNotify      destroy_notify)
{
    PropWatcher *watcher;
    GObjectClass *target_class, *source_class;
    char *signal_name;

    g_return_val_if_fail (G_IS_OBJECT (target), NULL);
    g_return_val_if_fail (G_IS_OBJECT (source), NULL);
    g_return_val_if_fail (target_prop != NULL, NULL);
    g_return_val_if_fail (source_prop != NULL, NULL);
    g_return_val_if_fail (transform != NULL, NULL);

    target_class = g_type_class_peek (G_OBJECT_TYPE (target));
    source_class = g_type_class_peek (G_OBJECT_TYPE (source));

    watcher = g_new0 (PropWatcher, 1);

    watcher->source = source;
    watcher->target = target;

    watcher->source_prop = g_object_class_find_property (source_class, source_prop);
    if (!watcher->source_prop)
    {
        g_warning ("%s: could not find property '%s' in class '%s'",
                   G_STRLOC, source_prop, g_type_name (G_OBJECT_TYPE (source)));
        g_free (watcher);
        return NULL;
    }

    watcher->target_prop = g_object_class_find_property (target_class, target_prop);
    if (!watcher->target_prop)
    {
        g_warning ("%s: could not find property '%s' in class '%s'",
                   G_STRLOC, target_prop, g_type_name (G_OBJECT_TYPE (target)));
        g_free (watcher);
        return NULL;
    }

    watcher->alive = TRUE;

    watcher->transform = transform;
    watcher->data = transform_data;
    watcher->destroy_notify = destroy_notify;

    signal_name = g_strdup_printf ("notify::%s", source_prop);
    g_signal_connect_swapped (source, signal_name,
                              G_CALLBACK (watcher_check), watcher);
    g_object_weak_ref (source, (GWeakNotify) source_died, watcher);
    g_object_weak_ref (target, (GWeakNotify) target_died, watcher);

    g_free (signal_name);
    return watcher;
}


static void
watcher_die (PropWatcher *watcher)
{
    if (watcher->source)
    {
        g_signal_handlers_disconnect_by_func (watcher->source,
                                              (gpointer) watcher_check,
                                              watcher);
        g_object_weak_unref (watcher->source, (GWeakNotify) source_died, watcher);
    }

    if (watcher->target)
        g_object_weak_unref (watcher->target, (GWeakNotify) target_died, watcher);

    if (watcher->destroy_notify)
        watcher->destroy_notify (watcher->data);

    g_free (watcher);
}


void
moo_bind_property_full (GObject            *target,
                        const char         *target_prop,
                        GObject            *source,
                        const char         *source_prop,
                        MooTransformPropFunc transform,
                        gpointer            transform_data,
                        GDestroyNotify      destroy_notify)
{
    PropWatcher *watcher;

    g_return_if_fail (G_IS_OBJECT (target));
    g_return_if_fail (G_IS_OBJECT (source));
    g_return_if_fail (target_prop != NULL);
    g_return_if_fail (source_prop != NULL);
    g_return_if_fail (transform != NULL);

    watcher = prop_watcher_new (target, target_prop,
                                source, source_prop,
                                transform, transform_data,
                                destroy_notify);

    if (!watcher)
        return;

    watcher_check (watcher);
}


static void
source_died (PropWatcher *watcher)
{
    watcher->source = NULL;
    watcher_die (watcher);
}


static void
target_died (PropWatcher *watcher)
{
    watcher->target = NULL;
    watcher_die (watcher);
}


static void
watcher_check (PropWatcher *watcher)
{
    GValue source_val, target_val;

    if (!G_IS_OBJECT (watcher->source) ||
         !G_IS_OBJECT (watcher->target))
    {
        watcher_die (watcher);
        g_return_if_reached ();
    }

    source_val.g_type = 0;
    target_val.g_type = 0;

    g_value_init (&source_val, watcher->source_prop->value_type);
    g_value_init (&target_val, watcher->target_prop->value_type);

    g_object_get_property (watcher->source,
                           watcher->source_prop->name,
                           &source_val);
    watcher->transform (&target_val, &source_val, watcher->data);
    g_object_set_property (watcher->target,
                           watcher->target_prop->name,
                           &target_val);

    g_value_unset (&source_val);
    g_value_unset (&target_val);
}


static void
bool_transform (GValue        *target,
                const GValue  *source,
                gpointer       invert)
{
    if (invert)
        g_value_set_boolean (target, !g_value_get_boolean (source));
    else
        g_value_set_boolean (target, g_value_get_boolean (source));
}

void
moo_bind_bool_property (GObject            *target,
                        const char         *target_prop,
                        GObject            *source,
                        const char         *source_prop,
                        gboolean            invert)
{
    moo_bind_property_full (target, target_prop, source, source_prop,
                            bool_transform, GINT_TO_POINTER (invert), NULL);
}


void
moo_bind_sensitive (GtkToggleButton    *btn,
                    GtkWidget         **dependent,
                    int                 num_dependent,
                    gboolean            invert)
{
    int i;
    for (i = 0; i < num_dependent; ++i)
        moo_bind_bool_property (G_OBJECT (dependent[i]), "sensitive",
                                G_OBJECT (btn), "active", invert);
}
