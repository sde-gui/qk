/*
 *   mooaccel.c
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

#include "mooutils/mooaccel.h"
#include "mooutils/mooprefs.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <string.h>

#define MOO_ACCEL_PREFS_KEY "Shortcuts"


static void          accel_map_changed      (GtkAccelMap        *map,
                                             gchar              *accel_path,
                                             guint               accel_key,
                                             GdkModifierType     accel_mods);

static const char   *get_accel              (const char         *accel_path);
static void          prefs_new_accel        (const char         *accel_path,
                                             const char         *default_accel);
static const char   *prefs_get_accel        (const char         *accel_path);
static void          prefs_set_accel        (const char         *accel_path,
                                             const char         *accel);

static gboolean      parse_accel            (const char         *accel,
                                             guint              *key,
                                             GdkModifierType    *mods);

static char         *_moo_accel_normalize   (const char         *accel);


static GHashTable *moo_accel_map = NULL;            /* char* -> char* */
static GHashTable *moo_default_accel_map = NULL;    /* char* -> char* */


static void
init_accel_map (void)
{
    if (!moo_accel_map)
    {
        moo_accel_map =
                g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) g_free);
        moo_default_accel_map =
                g_hash_table_new_full (g_str_hash, g_str_equal,
                                       (GDestroyNotify) g_free,
                                       (GDestroyNotify) g_free);

        g_signal_connect (gtk_accel_map_get (), "changed",
                          G_CALLBACK (accel_map_changed),
                          NULL);
    }
}


static void
accel_map_changed (G_GNUC_UNUSED GtkAccelMap *map,
                   gchar            *accel_path,
                   guint             accel_key,
                   GdkModifierType   accel_mods)
{
    char *new_accel;

    if (accel_key && accel_mods)
        new_accel = gtk_accelerator_name (accel_key, accel_mods);
    else
        new_accel = NULL;

    _moo_modify_accel (accel_path, new_accel);

    g_free (new_accel);
}


static char *
accel_path_to_prefs_key (const char *accel_path)
{
    if (accel_path && accel_path[0] == '<')
    {
        accel_path = strchr (accel_path, '/');
        if (accel_path)
            accel_path++;
    }

    if (!accel_path || !accel_path[0])
        return NULL;

    return g_strdup_printf (MOO_ACCEL_PREFS_KEY "/%s", accel_path);
}


static void
prefs_set_accel (const char *accel_path,
                 const char *accel)
{
    char *key;

    g_return_if_fail (accel != NULL);

    key = accel_path_to_prefs_key (accel_path);
    g_return_if_fail (key != NULL);

    g_return_if_fail (moo_prefs_key_registered (key));
    moo_prefs_set_string (key, accel);

    g_free (key);
}


static void
prefs_new_accel (const char *accel_path,
                 const char *default_accel)
{
    char *key;

    g_return_if_fail (accel_path != NULL && default_accel != NULL);

    key = accel_path_to_prefs_key (accel_path);
    g_return_if_fail (key != NULL);

    if (!moo_prefs_key_registered (key))
        moo_prefs_new_key_string(key, default_accel);

    g_free (key);
}


static const char *
prefs_get_accel (const char *accel_path)
{
    const char *accel;
    char *key;

    key = accel_path_to_prefs_key (accel_path);
    g_return_val_if_fail (key != NULL, NULL);

    accel = moo_prefs_get_string (key);

    g_free (key);
    return accel;
}


static void
set_accel (const char *accel_path,
           const char *accel)
{
    guint accel_key = 0;
    GdkModifierType accel_mods = 0;
    const char *old_accel;

    g_return_if_fail (accel_path != NULL && accel != NULL);

    init_accel_map ();

    old_accel = get_accel (accel_path);

    if (old_accel && !strcmp (old_accel, accel))
        return;

    if (*accel)
    {
        gtk_accelerator_parse (accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods)
        {
            g_hash_table_insert (moo_accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
        }
        else
        {
            g_warning ("could not parse accelerator '%s'", accel);
            g_hash_table_insert (moo_accel_map,
                                 g_strdup (accel_path),
                                 g_strdup (""));
        }
    }
    else
    {
        g_hash_table_insert (moo_accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
    }

    g_signal_handlers_block_by_func (gtk_accel_map_get (),
                                     (gpointer) accel_map_changed,
                                     NULL);

    if (!gtk_accel_map_change_entry (accel_path, accel_key, accel_mods, TRUE))
        g_warning ("could not set accel '%s' for accel_path '%s'",
                   accel, accel_path);

    g_signal_handlers_unblock_by_func (gtk_accel_map_get (),
                                       (gpointer) accel_map_changed,
                                       NULL);
}


static const char *
get_accel (const char *accel_path)
{
    g_return_val_if_fail (accel_path != NULL, NULL);
    init_accel_map ();
    return g_hash_table_lookup (moo_accel_map, accel_path);
}


static const char *
get_default_accel (const char *accel_path)
{
    g_return_val_if_fail (accel_path != NULL, NULL);
    init_accel_map ();
    return g_hash_table_lookup (moo_default_accel_map, accel_path);
}


const char *
_moo_get_accel (const char *accel_path)
{
    const char *accel = get_accel (accel_path);
    g_return_val_if_fail (accel != NULL, "");
    return accel;
}


const char *
_moo_get_default_accel (const char *accel_path)
{
    const char *accel = get_default_accel (accel_path);
    g_return_val_if_fail (accel != NULL, "");
    return accel;
}


void
_moo_accel_register (const char *accel_path,
                     const char *default_accel)
{
    char *freeme = NULL;

    g_return_if_fail (accel_path != NULL && default_accel != NULL);

    init_accel_map ();

    if (get_default_accel (accel_path))
        return;

    if (default_accel[0])
    {
        freeme = _moo_accel_normalize (default_accel);
        g_return_if_fail (freeme != NULL);
        default_accel = freeme;
    }

    if (default_accel[0])
    {
        guint accel_key = 0;
        GdkModifierType accel_mods = 0;

        gtk_accelerator_parse (default_accel, &accel_key, &accel_mods);

        if (accel_key || accel_mods)
        {
            g_hash_table_insert (moo_default_accel_map,
                                 g_strdup (accel_path),
                                 gtk_accelerator_name (accel_key, accel_mods));
        }
        else
        {
            g_warning ("could not parse accelerator '%s'", default_accel);
        }
    }
    else
    {
        g_hash_table_insert (moo_default_accel_map,
                             g_strdup (accel_path),
                             g_strdup (""));
    }

    prefs_new_accel (accel_path, default_accel);
    set_accel (accel_path, prefs_get_accel (accel_path));

    g_free (freeme);
}


void
_moo_modify_accel (const char *accel_path,
                   const char *new_accel)
{
    char *freeme = NULL;

    g_return_if_fail (accel_path != NULL);
    g_return_if_fail (get_accel (accel_path) != NULL);

    if (!new_accel)
        new_accel = "";

    if (new_accel[0])
    {
        freeme = _moo_accel_normalize (new_accel);
        new_accel = freeme;
        g_return_if_fail (new_accel != NULL);
    }

    set_accel (accel_path, new_accel);
    prefs_set_accel (accel_path, new_accel);

    g_free (freeme);
}


char *
_moo_get_accel_label (const char *accel)
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


/*****************************************************************************/
/* Parsing accelerator strings
 */

/* TODO: multibyte symbols? */
static guint
keyval_from_symbol (char sym)
{
    switch (sym)
    {
        case ' ': return GDK_space;
        case '!': return GDK_exclam;
        case '"': return GDK_quotedbl;
        case '#': return GDK_numbersign;
        case '$': return GDK_dollar;
        case '%': return GDK_percent;
        case '&': return GDK_ampersand;
        case '\'': return GDK_apostrophe;
#if 0
        case '\'': return GDK_quoteright;
#define GDK_asciicircum 0x05e
#define GDK_grave 0x060
#endif
        case '(': return GDK_parenleft;
        case ')': return GDK_parenright;
        case '*': return GDK_asterisk;
        case '+': return GDK_plus;
        case ',': return GDK_comma;
        case '-': return GDK_minus;
        case '.': return GDK_period;
        case '/': return GDK_slash;
        case ':': return GDK_colon;
        case ';': return GDK_semicolon;
        case '<': return GDK_less;
        case '=': return GDK_equal;
        case '>': return GDK_greater;
        case '?': return GDK_question;
        case '@': return GDK_at;
        case '[': return GDK_bracketleft;
        case '\\': return GDK_backslash;
        case ']': return GDK_bracketright;
        case '_': return GDK_underscore;
        case '`': return GDK_quoteleft;
        case '{': return GDK_braceleft;
        case '|': return GDK_bar;
        case '}': return GDK_braceright;
        case '~': return GDK_asciitilde;
    }

    return 0;
}


static guint
parse_key (const char *string)
{
    char *stripped = g_strstrip (g_strdup (string));
    guint key = gdk_keyval_from_name (stripped);

    if (!key)
        key = keyval_from_symbol (stripped[0]);

    g_free (stripped);
    return key;
}


static GdkModifierType
parse_mod (const char *string)
{
    GdkModifierType mod = 0;
    char *stripped;

    stripped = g_strstrip (g_ascii_strdown (string, -1));

    if (!strcmp (stripped, "alt"))
        mod = GDK_MOD1_MASK;
    else if (!strcmp (stripped, "ctl") ||
              !strcmp (stripped, "ctrl") ||
              !strcmp (stripped, "control"))
        mod = GDK_CONTROL_MASK;
    else if (!strncmp (stripped, "mod", 3) &&
              1 <= stripped[3] && stripped[3] <= 5 && !stripped[4])
    {
        switch (stripped[3])
        {
            case '1': mod = GDK_MOD1_MASK; break;
            case '2': mod = GDK_MOD2_MASK; break;
            case '3': mod = GDK_MOD3_MASK; break;
            case '4': mod = GDK_MOD4_MASK; break;
            case '5': mod = GDK_MOD5_MASK; break;
        }
    }
    else if (!strcmp (stripped, "shift") ||
              !strcmp (stripped, "shft"))
        mod = GDK_SHIFT_MASK;
    else if (!strcmp (stripped, "release"))
        mod = GDK_RELEASE_MASK;

    g_free (stripped);
    return mod;
}


static gboolean
accel_parse_sep (const char     *accel,
                 const char     *sep,
                 guint          *keyval,
                 GdkModifierType *modifiers)
{
    char **pieces;
    guint n_pieces, i;
    GdkModifierType mod = 0;
    guint key = 0;

    pieces = g_strsplit (accel, sep, 0);
    n_pieces = g_strv_length (pieces);
    g_return_val_if_fail (n_pieces > 1, FALSE);

    for (i = 0; i < n_pieces - 1; ++i)
    {
        GdkModifierType m = parse_mod (pieces[i]);

        if (!m)
            goto out;

        mod |= m;
    }

    key = parse_key (pieces[n_pieces-1]);

out:
    g_strfreev (pieces);

    if (!key)
        mod = 0;

    if (keyval)
        *keyval = key;
    if (modifiers)
        *modifiers = mod;
    return key != 0;
}


static gboolean
parse_accel (const char      *accel,
             guint           *keyval,
             GdkModifierType *modifiers)
{
    guint key = 0;
    guint len;
    GdkModifierType mods = 0;
    char *p;

    g_return_val_if_fail (accel && accel[0], FALSE);

    len = strlen (accel);

    if (len == 1)
    {
        key = parse_key (accel);
        goto out;
    }

    if (accel[0] == '<')
    {
        gtk_accelerator_parse (accel, &key, &mods);
        goto out;
    }

    if ((p = strchr (accel, '+')) && p != accel + len - 1)
        return accel_parse_sep (accel, "+", keyval, modifiers);
    else if ((p = strchr (accel, '-')) && p != accel + len - 1)
        return accel_parse_sep (accel, "-", keyval, modifiers);

    key = parse_key (accel);

out:
    if (keyval)
        *keyval = gdk_keyval_to_lower (key);
    if (modifiers)
        *modifiers = mods;
    return key != 0;
}


static char *
_moo_accel_normalize (const char *accel)
{
    guint key;
    GdkModifierType mods;

    if (!accel || !accel[0])
        return NULL;

    if (parse_accel (accel, &key, &mods))
    {
        return gtk_accelerator_name (key, mods);
    }
    else
    {
        g_warning ("could not parse accelerator '%s'", accel);
        return NULL;
    }
}
