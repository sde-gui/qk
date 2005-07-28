/*
 *   mooui/mooaccel.c
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

#include "mooui/mooaccel.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moocompat.h"
#include <gtk/gtk.h>
#include <string.h>

#define MOO_ACCEL_PREFS_KEY "Shortcuts"

static GHashTable *moo_accel_map = NULL;            /* char* -> char* */
static GHashTable *moo_default_accel_map = NULL;    /* char* -> char* */


static void watch_gtk_accel_map         (void);
static void block_watch_gtk_accel_map   (void);
static void unblock_watch_gtk_accel_map (void);


static void init_accel_map (void)
{
    static gboolean done = FALSE;

    if (!done)
    {
        done = TRUE;

        moo_accel_map =
                g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) g_free);
        moo_default_accel_map =
                g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) g_free);

        watch_gtk_accel_map ();
    }
}


static char *accel_path_to_prefs_key (const char *accel_path)
{
    GString *key;
    const char *p;

    if (accel_path && accel_path[0] == '<')
    {
        accel_path = strchr (accel_path, '/');
        if (accel_path)
            accel_path++;
    }

    if (!accel_path || !accel_path[0])
        return NULL;

    key = g_string_new (MOO_ACCEL_PREFS_KEY);

    while (accel_path && *accel_path)
    {
        p = strchr (accel_path, '/');

        g_string_append (key, "::");

        if (p)
        {
            g_string_append_len (key, accel_path, p - accel_path);
            accel_path = p + 1;
        }
        else
        {
            g_string_append (key, accel_path);
            accel_path = NULL;
        }
    }

    if (key->len > strlen (MOO_ACCEL_PREFS_KEY))
    {
        return g_string_free (key, FALSE);
    }
    else
    {
        g_string_free (key, TRUE);
        return NULL;
    }
}


void         moo_prefs_set_accel            (const char     *accel_path,
                                             const char     *accel)
{
    char *key = accel_path_to_prefs_key (accel_path);
    g_return_if_fail (key != NULL);
    moo_prefs_set_string (key, accel);
    g_free (key);
}


const char  *moo_prefs_get_accel            (const char     *accel_path)
{
    const char *accel;
    char *key = accel_path_to_prefs_key (accel_path);
    g_return_val_if_fail (key != NULL, NULL);
    accel = moo_prefs_get_string (key);
    g_free (key);
    return accel;
}


void         moo_set_accel                  (const char     *accel_path,
                                             const char     *accel)
{
    guint accel_key = 0;
    GdkModifierType accel_mods = 0;
    GtkAccelKey old;

    g_return_if_fail (accel_path != NULL && accel != NULL);

    init_accel_map ();

    if (*accel)
    {
        gtk_accelerator_parse (accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods) {
            g_hash_table_insert (moo_accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
        }
        else {
            g_warning ("could not parse accelerator '%s'", accel);
            g_hash_table_insert (moo_accel_map,
                                 g_strdup (accel_path),
                                 g_strdup (""));
        }
    }
    else {
        g_hash_table_insert (moo_accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
    }

    block_watch_gtk_accel_map ();

    if (gtk_accel_map_lookup_entry (accel_path, &old))
    {
        if (accel_key != old.accel_key || accel_mods != old.accel_mods)
        {
            if (accel_key || accel_mods)
            {
                if (!gtk_accel_map_change_entry (accel_path, accel_key,
                     accel_mods, TRUE))
                    g_warning ("could not set accel '%s' for accel_path '%s'",
                               accel, accel_path);
            }
            else
            {
                gtk_accel_map_change_entry (accel_path, 0, 0, TRUE);
            }
        }
    }
    else
    {
        if (accel_key || accel_mods)
        {
            gtk_accel_map_add_entry (accel_path,
                                     accel_key,
                                     accel_mods);
        }
    }

    unblock_watch_gtk_accel_map ();
}


void         moo_set_default_accel          (const char     *accel_path,
                                             const char     *accel)
{
    const char *old_accel;

    g_return_if_fail (accel_path != NULL && accel != NULL);

    init_accel_map ();

    old_accel = moo_get_default_accel (accel_path);

    if (old_accel && !strcmp (old_accel, accel))
        return;

    if (*accel)
    {
        guint accel_key = 0;
        GdkModifierType accel_mods = 0;

        gtk_accelerator_parse (accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods)
        {
            g_hash_table_insert (moo_default_accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
        }
        else
        {
            g_warning ("could not parse accelerator '%s'", accel);
        }
    }
    else
    {
        g_hash_table_insert (moo_default_accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
    }
}


const char  *moo_get_accel                  (const char     *accel_path)
{
    g_return_val_if_fail (accel_path != NULL, NULL);
    init_accel_map ();
    return g_hash_table_lookup (moo_accel_map, accel_path);
}


const char  *moo_get_default_accel          (const char     *accel_path)
{
    g_return_val_if_fail (accel_path != NULL, NULL);
    init_accel_map ();
    return g_hash_table_lookup (moo_default_accel_map, accel_path);
}


#if GTK_CHECK_VERSION(2,4,0)
static void sync_accel_prefs            (const char *accel_path)
{
    const char *default_accel, *accel;

    g_return_if_fail (accel_path != NULL);

    init_accel_map ();

    accel = moo_get_accel (accel_path);
    default_accel = moo_get_default_accel (accel_path);
    if (!accel) accel = "";
    if (!default_accel) default_accel = "";

    if (strcmp (accel, default_accel))
        moo_prefs_set_accel (accel_path, accel);
    else
        moo_prefs_set_accel (accel_path, NULL);
}


static void accel_map_changed (G_GNUC_UNUSED GtkAccelMap *object,
                               gchar *accel_path,
                               guint accel_key,
                               GdkModifierType accel_mods)
{
    char *accel;
    const char *old_accel;
    const char *default_accel;

    init_accel_map ();

    old_accel = moo_get_accel (accel_path);
    default_accel = moo_get_default_accel (accel_path);

    if (!old_accel)
        return;

    if (accel_key)
        accel = gtk_accelerator_name (accel_key, accel_mods);
    else
        accel = g_strdup ("");

    g_return_if_fail (accel != NULL);

    if (strcmp (accel, old_accel))
        g_hash_table_insert (moo_accel_map,
                             g_strdup (accel_path),
                             g_strdup (accel));

    sync_accel_prefs (accel_path);

    g_free (accel);
}


static void watch_gtk_accel_map         (void)
{
    GtkAccelMap *accel_map = gtk_accel_map_get ();
    g_return_if_fail (accel_map != NULL);
    g_signal_connect (accel_map, "changed",
                      G_CALLBACK (accel_map_changed), NULL);
}

static void block_watch_gtk_accel_map   (void)
{
    GtkAccelMap *accel_map = gtk_accel_map_get ();
    g_return_if_fail (accel_map != NULL);
    g_signal_handlers_block_by_func (accel_map,
                                     (gpointer) accel_map_changed,
                                     NULL);
}

static void unblock_watch_gtk_accel_map (void)
{
    GtkAccelMap *accel_map = gtk_accel_map_get ();
    g_return_if_fail (accel_map != NULL);
    g_signal_handlers_unblock_by_func (accel_map,
                                       (gpointer) accel_map_changed,
                                       NULL);
}
#else /* !GTK_CHECK_VERSION(2,4,0) */
static void watch_gtk_accel_map         (void)
{
}

static void block_watch_gtk_accel_map   (void)
{
}

static void unblock_watch_gtk_accel_map (void)
{
}
#endif /* !GTK_CHECK_VERSION(2,4,0) */


char        *moo_get_accel_label            (const char     *accel)
{
    guint key;
    GdkModifierType mods;

    g_return_val_if_fail (accel != NULL, g_strdup (""));

    init_accel_map ();

    if (*accel)
    {
        gtk_accelerator_parse (accel, &key, &mods);
        return gtk_accelerator_get_label (key, mods);
    }
    else
    {
        return g_strdup ("");
    }
}
