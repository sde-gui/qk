/*
 *   mooeditfiltersettings.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditfiltersettings.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/mooprefs.h"
#include "mooutils/eggregex.h"
#include <string.h>


#define ELEMENT_FILTER_SETTINGS MOO_EDIT_PREFS_PREFIX "/filter-settings"
#define ELEMENT_SETTING         "setting"
#define PROP_FILTER             "filter"
#define PROP_CONFIG             "config"


typedef struct {
    EggRegex *regex;
    char *config;
} FilterSetting;

typedef struct {
    GSList *settings;
} FilterSettingsStore;

static FilterSettingsStore *settings_store;


static void
filter_setting_free (FilterSetting *setting)
{
    if (setting)
    {
        g_free (setting->config);
        egg_regex_unref (setting->regex);
        g_free (setting);
    }
}


static FilterSetting *
filter_setting_new (const char *filter,
                    const char *config)
{
    FilterSetting *setting;

    setting = g_new0 (FilterSetting, 1);

    setting->regex = egg_regex_new (filter, EGG_REGEX_DOTALL, 0, NULL);
    setting->config = g_strdup (config);

    if (!setting->regex || !setting->config)
    {
        filter_setting_free (setting);
        setting = NULL;
    }
    else
    {
        egg_regex_optimize (setting->regex, NULL);
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

    xml = moo_prefs_get_markup ();
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


void
_moo_edit_filter_settings_reload (void)
{
    if (settings_store)
        filter_settings_store_free (settings_store);
    settings_store = NULL;
    _moo_edit_filter_settings_load ();
}


const char *
_moo_edit_filter_settings_get_for_file (const char *filename)
{
    char *filename_utf8;
    const char *result;

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
    if (egg_regex_match (setting->regex, filename, 0))
    {
        g_message ("file '%s' matched pattern '%s': config '%s'",
                   filename, egg_regex_get_pattern (setting->regex),
                   setting->config);
        return setting->config;
    }

    return NULL;
}


static const char *
filter_settings_store_get_setting (FilterSettingsStore *store,
                                   const char          *filename)
{
    GSList *l;

    for (l = store->settings; l != NULL; l = l->next)
    {
        const char *result;

        result = filter_setting_match (l->data, filename);

        if (result)
            return result;
    }

    return NULL;
}


const char *
_moo_edit_filter_settings_get_for_file_utf8 (const char *filename)
{
    g_return_val_if_fail (settings_store != NULL, NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    g_assert (g_utf8_validate (filename, -1, NULL));

    return filter_settings_store_get_setting (settings_store, filename);
}


void
_moo_edit_filter_settings_set_strings (GSList *list)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;

    xml = moo_prefs_get_markup ();
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
        strings = g_slist_prepend (strings, g_strdup (egg_regex_get_pattern (setting->regex)));
        strings = g_slist_prepend (strings, g_strdup (setting->config));
    }

    return g_slist_reverse (strings);
}
