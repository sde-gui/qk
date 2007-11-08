/*
 *   mooeditfiltersettings.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mooeditaction.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-debug.h"
#include <glib/gregex.h>
#include <string.h>
#ifndef __WIN32__
#include <fnmatch.h>
#endif


MOO_DEBUG_INIT(filters, FALSE)

#define ELEMENT_FILTER_SETTINGS MOO_EDIT_PREFS_PREFIX "/filter-settings"
#define ELEMENT_SETTING         "setting"
#define PROP_FILTER             "filter"
#define PROP_CONFIG             "config"

typedef enum {
    MOO_EDIT_FILTER_LANGS,
    MOO_EDIT_FILTER_GLOBS,
    MOO_EDIT_FILTER_REGEX
} MooEditFilterType;

struct _MooEditFilter {
    MooEditFilterType type;
    union {
        GSList *langs;
        GSList *globs;
        GRegex *regex;
    } u;
};

typedef struct {
    GRegex *regex;
    char *config;
} FilterSetting;

typedef struct {
    GSList *settings;
} FilterSettingsStore;

static FilterSettingsStore *settings_store;


static char *filter_settings_store_get_setting (FilterSettingsStore *store,
                                                const char          *filename);


MooEditFilter *
_moo_edit_filter_new (const char *string)
{
    g_return_val_if_fail (string && string[0], NULL);

    if (!strncmp (string, "langs:", strlen ("langs:")))
        return _moo_edit_filter_new_langs (string + strlen ("langs:"));
    if (!strncmp (string, "globs:", strlen ("globs:")))
        return _moo_edit_filter_new_globs (string + strlen ("globs:"));
    if (!strncmp (string, "regex:", strlen ("regex:")))
        return _moo_edit_filter_new_regex (string + strlen ("regex:"));

    return _moo_edit_filter_new_globs (string);
}

MooEditFilter *
_moo_edit_filter_new_langs (const char *string)
{
    MooEditFilter *filt;

    g_return_val_if_fail (string != NULL, NULL);

    filt = g_new0 (MooEditFilter, 1);
    filt->type = MOO_EDIT_FILTER_LANGS;
    filt->u.langs = _moo_edit_parse_langs (string);

    return filt;
}

MooEditFilter *
_moo_edit_filter_new_regex (const char *string)
{
    MooEditFilter *filt;
    GRegex *regex;
    GError *error = NULL;

    g_return_val_if_fail (string != NULL, NULL);

    regex = g_regex_new (string, G_REGEX_OPTIMIZE, 0, &error);

    if (!regex)
    {
        g_warning ("%s: invalid regex '%s': %s", G_STRFUNC,
                   string, error->message);
        g_error_free (NULL);
        return NULL;
    }

    filt = g_new0 (MooEditFilter, 1);
    filt->type = MOO_EDIT_FILTER_REGEX;
    filt->u.regex = regex;

    return filt;
}

static GSList *
parse_globs (const char *string)
{
    char **pieces, **p;
    GSList *list = NULL;

    if (!string)
        return NULL;

    pieces = g_strsplit_set (string, " \t\r\n;,", 0);

    if (!pieces)
        return NULL;

    for (p = pieces; *p != NULL; ++p)
    {
        g_strstrip (*p);

        if (**p)
            list = g_slist_prepend (list, g_strdup (*p));
    }

    g_strfreev (pieces);
    return g_slist_reverse (list);
}

MooEditFilter *
_moo_edit_filter_new_globs (const char *string)
{
    MooEditFilter *filt;

    g_return_val_if_fail (string != NULL, NULL);

    filt = g_new0 (MooEditFilter, 1);
    filt->type = MOO_EDIT_FILTER_GLOBS;
    filt->u.globs = parse_globs (string);

    return filt;
}

void
_moo_edit_filter_free (MooEditFilter *filter)
{
    if (filter)
    {
        switch (filter->type)
        {
            case MOO_EDIT_FILTER_GLOBS:
            case MOO_EDIT_FILTER_LANGS:
                g_slist_foreach (filter->u.langs, (GFunc) g_free, NULL);
                g_slist_free (filter->u.langs);
                break;
            case MOO_EDIT_FILTER_REGEX:
                if (filter->u.regex)
                    g_regex_unref (filter->u.regex);
                break;
        }

        g_free (filter);
    }
}


static gboolean
moo_edit_filter_check_globs (GSList  *globs,
                             MooEdit *doc)
{
    char *name = NULL;

    name = moo_edit_get_filename (doc);

    if (name)
    {
        char *tmp = name;
        name = g_path_get_basename (tmp);
        g_free (tmp);
    }

    while (globs)
    {
        if (name)
        {
            if (fnmatch (globs->data, name, 0) == 0)
            {
                g_free (name);
                return TRUE;
            }
        }
        else
        {
            if (!strcmp (globs->data, "*"))
                return TRUE;
        }

        globs = globs->next;
    }

    g_free (name);
    return FALSE;
}

static gboolean
moo_edit_filter_check_langs (GSList  *langs,
                             MooEdit *doc)
{
    MooLang *lang;
    const char *id;

    lang = moo_text_view_get_lang (MOO_TEXT_VIEW (doc));
    id = _moo_lang_id (lang);

    while (langs)
    {
        if (!strcmp (langs->data, id))
            return TRUE;
        langs = langs->next;
    }

    return FALSE;
}

static gboolean
moo_edit_filter_check_regex (GRegex  *regex,
                             MooEdit *doc)
{
    const char *name = moo_edit_get_display_name (doc);
    g_return_val_if_fail (name != NULL, FALSE);
    return g_regex_match (regex, name, 0, NULL);
}

gboolean
_moo_edit_filter_match (MooEditFilter *filter,
                        MooEdit       *doc)
{
    g_return_val_if_fail (filter != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_EDIT (doc), FALSE);

    switch (filter->type)
    {
        case MOO_EDIT_FILTER_GLOBS:
            return moo_edit_filter_check_globs (filter->u.globs, doc);
        case MOO_EDIT_FILTER_LANGS:
            return moo_edit_filter_check_langs (filter->u.langs, doc);
        case MOO_EDIT_FILTER_REGEX:
            return moo_edit_filter_check_regex (filter->u.regex, doc);
    }

    g_return_val_if_reached (FALSE);
}


static void
filter_setting_free (FilterSetting *setting)
{
    if (setting)
    {
        g_free (setting->config);
        if (setting->regex)
            g_regex_unref (setting->regex);
        g_free (setting);
    }
}


static FilterSetting *
filter_setting_new (const char *filter,
                    const char *config)
{
    FilterSetting *setting;
    GError *error = NULL;

    g_return_val_if_fail (filter != NULL, NULL);
    g_return_val_if_fail (config != NULL, NULL);

    setting = g_new0 (FilterSetting, 1);

    setting->regex = g_regex_new (filter, G_REGEX_DOTALL | G_REGEX_OPTIMIZE, 0, &error);
    setting->config = g_strdup (config);

    if (!setting->regex)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        filter_setting_free (setting);
        setting = NULL;
    }

    return setting;
}


static FilterSettingsStore *
filter_settings_store_new (void)
{
    FilterSettingsStore *store;

    store = g_new0 (FilterSettingsStore, 1);

    return store;
}


static void
filter_settings_store_free (FilterSettingsStore *store)
{
    g_slist_foreach (store->settings, (GFunc) filter_setting_free, NULL);
    g_slist_free (store->settings);
    g_free (store);
}


static void
load_node (FilterSettingsStore *store,
           MooMarkupNode       *node)
{
    const char *filter, *config;
    FilterSetting *setting;

    filter = moo_markup_get_prop (node, PROP_FILTER);
    config = moo_markup_get_prop (node, PROP_CONFIG);
    g_return_if_fail (filter != NULL && config != NULL);

    setting = filter_setting_new (filter, config);
    g_return_if_fail (setting != NULL);

    store->settings = g_slist_prepend (store->settings, setting);
}


static void
filter_settings_store_load (FilterSettingsStore *store)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *node;

    g_return_if_fail (!store->settings);

    xml = moo_prefs_get_markup (MOO_PREFS_RC);
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml),
                                   ELEMENT_FILTER_SETTINGS);

    if (!root)
        return;

    for (node = root->children; node != NULL; node = node->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, ELEMENT_SETTING))
        {
            g_warning ("%s: invalid '%s' element", G_STRLOC, node->name);
            continue;
        }

        load_node (store, node);
    }

    store->settings = g_slist_reverse (store->settings);
}


void
_moo_edit_filter_settings_load (void)
{
    if (settings_store)
        return;

    settings_store = filter_settings_store_new ();
    filter_settings_store_load (settings_store);
}


static void
_moo_edit_filter_settings_reload (void)
{
    if (settings_store)
        filter_settings_store_free (settings_store);
    settings_store = NULL;
    _moo_edit_filter_settings_load ();
}


static char *
_moo_edit_filter_settings_get_for_file_utf8 (const char *filename)
{
    g_return_val_if_fail (settings_store != NULL, NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    g_assert (g_utf8_validate (filename, -1, NULL));

    return filter_settings_store_get_setting (settings_store, filename);
}


char *
_moo_edit_filter_settings_get_for_file (const char *filename)
{
    char *filename_utf8;
    char *result;

    g_return_val_if_fail (filename != NULL, NULL);

    filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
    g_return_val_if_fail (filename_utf8 != NULL, NULL);

    result = _moo_edit_filter_settings_get_for_file_utf8 (filename_utf8);

    g_free (filename_utf8);
    return result;
}


static const char *
filter_setting_match (FilterSetting *setting,
                      const char    *filename)
{
    if (g_regex_match (setting->regex, filename, 0, NULL))
    {
        moo_dmsg ("file '%s' matched pattern '%s': config '%s'",
                  filename, g_regex_get_pattern (setting->regex),
                  setting->config);
        return setting->config;
    }

    return NULL;
}


static char *
filter_settings_store_get_setting (FilterSettingsStore *store,
                                   const char          *filename)
{
    GSList *l;
    GString *result = NULL;

    for (l = store->settings; l != NULL; l = l->next)
    {
        const char *match;

        match = filter_setting_match (l->data, filename);

        if (match)
        {
            if (!result)
                result = g_string_new (match);
            else
                g_string_append_printf (result, ";%s", match);
        }
    }

    return result ? g_string_free (result, FALSE) : NULL;
}


void
_moo_edit_filter_settings_set_strings (GSList *list)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;

    xml = moo_prefs_get_markup (MOO_PREFS_RC);
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml),
                                   ELEMENT_FILTER_SETTINGS);

    if (root)
        moo_markup_delete_node (root);

    if (!list)
    {
        _moo_edit_filter_settings_reload ();
        return;
    }

    root = moo_markup_create_element (MOO_MARKUP_NODE (xml),
                                      ELEMENT_FILTER_SETTINGS);

    while (list)
    {
        MooMarkupNode *node;
        const char *filter, *config;

        g_return_if_fail (list->data && list->next && list->next->data);

        filter = list->data;
        config = list->next->data;

        node = moo_markup_create_element (root, ELEMENT_SETTING);
        moo_markup_set_prop (node, PROP_FILTER, filter);
        moo_markup_set_prop (node, PROP_CONFIG, config);

        list = list->next->next;
    }

    _moo_edit_filter_settings_reload ();
}


GSList *
_moo_edit_filter_settings_get_strings (void)
{
    GSList *strings = NULL, *l;

    g_return_val_if_fail (settings_store != NULL, NULL);

    for (l = settings_store->settings; l != NULL; l = l->next)
    {
        FilterSetting *setting = l->data;
        strings = g_slist_prepend (strings, g_strdup (g_regex_get_pattern (setting->regex)));
        strings = g_slist_prepend (strings, g_strdup (setting->config));
    }

    return g_slist_reverse (strings);
}
