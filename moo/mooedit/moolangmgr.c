/*
 *   moolangmgr.c
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
#include "config.h"
#include "mooedit/moolangmgr-private.h"
#include "mooedit/moolang-private.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/xdgmime/xdgmime.h"
#include <string.h>

#define LANGUAGE_DIR            "language-specs"
#define ELEMENT_LANG_CONFIG     MOO_EDIT_PREFS_PREFIX "/langs"
#define ELEMENT_LANG            "lang"
#define ELEMENT_EXTENSIONS      "extensions"
#define ELEMENT_MIME_TYPES      "mime-types"
#define ELEMENT_CONFIG          "config"
#define PROP_LANG_ID            "id"
#define SCHEME_DEFAULT          "default"


static void     string_list_free        (GSList     *list);
static GSList  *string_list_copy        (GSList     *list);
static void     read_langs              (MooLangMgr *mgr);
static void     read_schemes            (MooLangMgr *mgr);
static void     load_config             (MooLangMgr *mgr);
static MooLang *get_lang_for_filename   (MooLangMgr *mgr,
                                         const char *filename);
static MooLang *get_lang_for_mime_type  (MooLangMgr *mgr,
                                         const char *mime_type);


G_DEFINE_TYPE (MooLangMgr, moo_lang_mgr, GTK_TYPE_SOURCE_LANGUAGES_MANAGER)


static void
moo_lang_mgr_init (MooLangMgr *mgr)
{
    char **dirs;
    guint n_dirs, i;
    GSList *list = NULL;

    mgr->schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);

    mgr->langs = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        g_free, g_object_unref);
    mgr->extensions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                             (GDestroyNotify) string_list_free);
    mgr->mime_types = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                             (GDestroyNotify) string_list_free);
    mgr->config = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, g_free);

    dirs = moo_get_data_subdirs (LANGUAGE_DIR, MOO_DATA_SHARE, &n_dirs);

    for (i = 0; i < n_dirs; ++i)
        list = g_slist_prepend (list, dirs[i]);

    g_object_set (mgr, "lang-files-dirs", list, "style-schemes-dirs", list, NULL);

    load_config (mgr);

    g_slist_free (list);
    g_strfreev (dirs);
}


static void
moo_lang_mgr_dispose (GObject *object)
{
    MooLangMgr *mgr = MOO_LANG_MGR (object);

    if (mgr->langs)
    {
        g_hash_table_destroy (mgr->langs);
        g_hash_table_destroy (mgr->config);
        g_hash_table_destroy (mgr->extensions);
        g_hash_table_destroy (mgr->mime_types);
        g_hash_table_destroy (mgr->schemes);
        mgr->langs = NULL;
        mgr->config = NULL;
        mgr->extensions = NULL;
        mgr->mime_types = NULL;
        mgr->active_scheme = NULL;
        mgr->schemes = NULL;
    }

    G_OBJECT_CLASS (moo_lang_mgr_parent_class)->dispose (object);
}


static void
moo_lang_mgr_class_init (MooLangMgrClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_lang_mgr_dispose;

    _moo_signal_new_cb ("loaded",
                        G_OBJECT_CLASS_TYPE (klass),
                        G_SIGNAL_RUN_LAST,
                        NULL, NULL, NULL,
                        _moo_marshal_VOID__VOID,
                        G_TYPE_NONE, 0);
}


MooLangMgr *
moo_lang_mgr_new (void)
{
    return g_object_new (MOO_TYPE_LANG_MGR, NULL);
}


static GSList *
string_list_copy (GSList *list)
{
    GSList *copy = NULL;

    while (list)
    {
        copy = g_slist_prepend (copy, g_strdup (list->data));
        list = list->next;
    }

    return g_slist_reverse (list);
}

static void
string_list_free (GSList *list)
{
    g_slist_foreach (list, (GFunc) g_free, NULL);
    g_slist_free (list);
}


MooLang *
moo_lang_mgr_get_lang (MooLangMgr *mgr,
                       const char *name)
{
    char *id;
    MooLang *lang;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    if (!name || !strcmp (name, MOO_LANG_NONE))
        return NULL;

    read_langs (mgr);
    id = _moo_lang_id_from_name (name);
    lang = g_hash_table_lookup (mgr->langs, id);

    if (!lang)
        g_warning ("could not find language '%s'", name);

    g_free (id);
    return lang;
}


static MooLang *
get_lang_by_extension (MooLangMgr *mgr,
                       const char *filename)
{
    MooLang *lang = NULL;
    char *basename, *utf8_basename;
    GSList *langs, *l;
    gboolean found = FALSE;

    g_return_val_if_fail (filename != NULL, NULL);

    /* TODO: is this right? */
    basename = g_path_get_basename (filename);
    g_return_val_if_fail (basename != NULL, NULL);
    utf8_basename = g_filename_display_name (basename);
    g_return_val_if_fail (utf8_basename != NULL, NULL);

    langs = moo_lang_mgr_get_available_langs (mgr);

    for (l = langs; !found && l != NULL; l = l->next)
    {
        GSList *g, *extensions;

        lang = l->data;
        extensions = _moo_lang_mgr_get_extensions (mgr, _moo_lang_id (lang));

        for (g = extensions; !found && g != NULL; g = g->next)
        {
            if (g_pattern_match_simple ((char*) g->data, utf8_basename))
            {
                found = TRUE;
                break;
            }
        }

        string_list_free (extensions);
    }

    if (!found)
        lang = NULL;

    g_slist_foreach (langs, (GFunc) g_object_unref, NULL);
    g_slist_free (langs);
    g_free (utf8_basename);
    g_free (basename);
    return lang;
}


static MooLang *
lang_mgr_get_lang_for_bak_filename (MooLangMgr *mgr,
                                    const char *filename)
{
    MooLang *lang = NULL;
    char *utf8_name, *utf8_base = NULL;
    int len;
    guint i;

    static const char *bak_globs[] = {"*~", "*.bak", "*.in", "*.orig"};

    utf8_name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
    g_return_val_if_fail (utf8_name != NULL, NULL);

    len = strlen (utf8_name);

    for (i = 0; i < G_N_ELEMENTS (bak_globs); ++i)
    {
        int ext_len = strlen (bak_globs[i]) - 1;

        if (len > ext_len && g_pattern_match_simple (bak_globs[i], utf8_name))
        {
            utf8_base = g_strndup (utf8_name, len - ext_len);
            break;
        }
    }

    if (utf8_base)
    {
        char *base = g_filename_from_utf8 (utf8_base, -1, NULL, NULL, NULL);

        if (base)
            lang = get_lang_for_filename (mgr, base);

        g_free (base);
        g_free (utf8_base);
    }

    g_free (utf8_name);
    return lang;
}


static gboolean
filename_blacklisted (MooLangMgr *mgr,
                      const char *filename)
{
    /* XXX bak files */
    char *basename, *utf8_basename;
    gboolean result = FALSE;
    GSList *extensions;

    basename = g_path_get_basename (filename);
    g_return_val_if_fail (basename != NULL, FALSE);
    utf8_basename = g_filename_display_name (basename);
    g_return_val_if_fail (utf8_basename != NULL, FALSE);

    extensions = g_hash_table_lookup (mgr->extensions, MOO_LANG_NONE);

    while (extensions)
    {
        if (g_pattern_match_simple ((char*) extensions->data, utf8_basename))
        {
            result = TRUE;
            break;
        }

        extensions = extensions->next;
    }

    g_free (utf8_basename);
    g_free (basename);
    return result;
}

static gboolean
file_blacklisted (MooLangMgr *mgr,
                  const char *filename)
{
    /* XXX mime type */
    return filename_blacklisted (mgr, filename);
}


MooLang *
moo_lang_mgr_get_lang_for_file (MooLangMgr *mgr,
                                const char *filename)
{
    MooLang *lang = NULL;
    const char *mime_type;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    read_langs (mgr);

    if (file_blacklisted (mgr, filename))
        return NULL;

    lang = get_lang_by_extension (mgr, filename);

    if (lang)
        return lang;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */
    /* It's a big problem! */
    mime_type = xdg_mime_get_mime_type_for_file (filename, NULL);

    if (mime_type != XDG_MIME_TYPE_UNKNOWN)
        lang = get_lang_for_mime_type (mgr, mime_type);

    if (lang)
        return lang;
#else
#ifdef __GNUC__
#warning "Implement moo_lang_mgr_get_lang_for_file()"
#endif
#endif /* MOO_USE_XDGMIME */

    lang = lang_mgr_get_lang_for_bak_filename (mgr, filename);

    if (lang)
        return lang;

    return NULL;
}


static MooLang *
get_lang_for_filename (MooLangMgr *mgr,
                       const char *filename)
{
    MooLang *lang = NULL;
    const char *mime_type;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (filename != NULL, NULL);

    if (filename_blacklisted (mgr, filename))
        return NULL;

    lang = get_lang_by_extension (mgr, filename);

    if (lang)
        return lang;

#ifdef MOO_USE_XDGMIME
    /* XXX: xdgmime wants utf8-encoded filename here. is it a problem? */
    /* It's a big problem! */

    mime_type = xdg_mime_get_mime_type_from_file_name (filename);

    if (mime_type != XDG_MIME_TYPE_UNKNOWN)
        lang = get_lang_for_mime_type (mgr, mime_type);

    if (lang)
        return lang;
#else
#ifdef __GNUC__
#warning "Implement moo_lang_mgr_get_lang_for_filename()"
#endif
#endif /* MOO_USE_XDGMIME */

    lang = lang_mgr_get_lang_for_bak_filename (mgr, filename);

    if (lang)
        return lang;

    return NULL;
}


#ifdef MOO_USE_XDGMIME
static int
check_mime_subclass (const char *base,
                     const char *mime)
{
    return !xdg_mime_mime_type_subclass (mime, base);
}


static MooLang *
get_lang_for_mime_type (MooLangMgr *mgr,
                        const char *mime)
{
    GSList *l, *langs;
    MooLang *lang = NULL;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (mime != NULL, NULL);

    langs = moo_lang_mgr_get_available_langs (mgr);

    for (l = langs; !found && l != NULL; l = l->next)
    {
        GSList *mimetypes;

        lang = l->data;
        mimetypes = _moo_lang_mgr_get_mime_types (mgr, _moo_lang_id (lang));

        if (g_slist_find_custom (mimetypes, mime, (GCompareFunc) strcmp))
            found = TRUE;

        string_list_free (mimetypes);
    }

    for (l = langs; !found && l != NULL; l = l->next)
    {
        GSList *mimetypes;

        lang = l->data;
        mimetypes = _moo_lang_mgr_get_mime_types (mgr, _moo_lang_id (lang));

        if (g_slist_find_custom (mimetypes, mime, (GCompareFunc) check_mime_subclass))
            found = TRUE;

        string_list_free (mimetypes);
    }

    g_slist_foreach (langs, (GFunc) g_object_unref, NULL);
    g_slist_free (langs);
    return found ? lang : NULL;
}
#else /* MOO_USE_XDGMIME */
MooLang*
moo_lang_mgr_get_lang_for_mime_type (MooLangMgr *mgr,
                                     const char *mime)
{
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);
    g_return_val_if_fail (mime != NULL, NULL);
    g_warning ("%s: implement me?", G_STRLOC);
    return NULL;
}
#endif /* MOO_USE_XDGMIME */


static void
read_langs (MooLangMgr *mgr)
{
    const GSList *langs;

    if (mgr->got_langs)
        return;

    read_schemes (mgr);

    mgr->got_langs = TRUE;
    langs = gtk_source_languages_manager_get_available_languages (GTK_SOURCE_LANGUAGES_MANAGER (mgr));

    while (langs)
    {
        MooLang *lang = langs->data;
        const char *id = _moo_lang_id (lang);

        g_hash_table_insert (mgr->langs, g_strdup (id), g_object_ref (lang));
        _moo_lang_parse_file (lang);

        langs = langs->next;
    }

    g_signal_emit_by_name (mgr, "loaded");
}


static GSList *
parse_string_list (const char *string)
{
    char *copy;
    GSList *list = NULL;
    char **pieces, **p;

    if (!string || !string[0])
        return NULL;

    copy = g_strstrip (g_strdup (string));

    pieces = g_strsplit_set (copy, ",;", 0);
    g_return_val_if_fail (pieces != NULL, NULL);

    for (p = pieces; *p; p++)
        if (**p)
            list = g_slist_prepend (list, g_strdup (*p));

    g_strfreev (pieces);
    g_free (copy);

    return g_slist_reverse (list);
}


static void
load_lang_node (MooLangMgr    *mgr,
                MooMarkupNode *lang_node)
{
    const char *lang_id;
    MooMarkupNode *node;

    lang_id = moo_markup_get_prop (lang_node, PROP_LANG_ID);
    g_return_if_fail (lang_id != NULL);

    for (node = lang_node->children; node != NULL; node = node->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (!strcmp (node->name, ELEMENT_EXTENSIONS))
        {
            const char *string = moo_markup_get_content (node);
            GSList *list = parse_string_list (string);
            g_hash_table_insert (mgr->extensions, g_strdup (lang_id), list);
        }
        else if (!strcmp (node->name, ELEMENT_MIME_TYPES))
        {
            const char *string = moo_markup_get_content (node);
            GSList *list = parse_string_list (string);
            g_hash_table_insert (mgr->mime_types, g_strdup (lang_id), list);
        }
        else if (!strcmp (node->name, ELEMENT_CONFIG))
        {
            const char *string = moo_markup_get_content (node);
            g_hash_table_insert (mgr->config, g_strdup (lang_id), g_strdup (string));
        }
        else
        {
            g_warning ("%s: invalid node '%s'", G_STRLOC, node->name);
        }
    }
}


static void
set_default_config (MooLangMgr *mgr)
{
    _moo_lang_mgr_set_config (mgr, "makefile", "use-tabs: true");
}

static void
load_config (MooLangMgr *mgr)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *node;

    set_default_config (mgr);

    xml = moo_prefs_get_markup ();
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml),
                                   ELEMENT_LANG_CONFIG);

    if (!root)
        return;

    for (node = root->children; node != NULL; node = node->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (strcmp (node->name, ELEMENT_LANG))
        {
            g_warning ("%s: invalid '%s' element", G_STRLOC, node->name);
            continue;
        }

        load_lang_node (mgr, node);
    }
}


MooTextStyleScheme *
_moo_lang_mgr_get_active_scheme (MooLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    read_schemes (mgr);

    return mgr->active_scheme;
}


void
_moo_lang_mgr_set_active_scheme (MooLangMgr *mgr,
                                 const char *name)
{
    MooTextStyleScheme *scheme;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));

    read_schemes (mgr);

    if (!name)
        name = SCHEME_DEFAULT;

    scheme = g_hash_table_lookup (mgr->schemes, name);

    if (!scheme)
        scheme = g_hash_table_lookup (mgr->schemes, SCHEME_DEFAULT);

    g_return_if_fail (scheme != NULL);
    mgr->active_scheme = scheme;
}


static void
prepend_scheme (G_GNUC_UNUSED const char *name,
                MooTextStyleScheme *scheme,
                GSList            **list)
{
    *list = g_slist_prepend (*list, g_object_ref (scheme));
}

GSList *
_moo_lang_mgr_list_schemes (MooLangMgr *mgr)
{
    GSList *list = NULL;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    read_schemes (mgr);
    g_hash_table_foreach (mgr->schemes, (GHFunc) prepend_scheme, &list);

    return list;
}


static void
read_schemes (MooLangMgr *mgr)
{
    const GSList *list;

    if (mgr->got_schemes)
        return;

    mgr->got_schemes = TRUE;

    list = gtk_source_languages_manager_get_available_style_schemes (GTK_SOURCE_LANGUAGES_MANAGER (mgr));

    while (list)
    {
        MooTextStyleScheme *scheme = list->data;

        if (!mgr->active_scheme || !strcmp (moo_text_style_scheme_get_id (scheme), SCHEME_DEFAULT))
            mgr->active_scheme = scheme;

        g_hash_table_insert (mgr->schemes, g_strdup (moo_text_style_scheme_get_id (scheme)), g_object_ref (scheme));

        list = list->next;
    }
}


GSList *
moo_lang_mgr_get_sections (MooLangMgr *mgr)
{
    GSList *sections = NULL;
    const GSList *list;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    read_langs (mgr);

    list = gtk_source_languages_manager_get_available_languages (GTK_SOURCE_LANGUAGES_MANAGER (mgr));

    while (list)
    {
        const char *section = _moo_lang_get_section (list->data);
        if (section && !g_slist_find_custom (sections, section, (GCompareFunc) strcmp))
            sections = g_slist_prepend (sections, g_strdup (section));
        list = list->next;
    }

    return sections;
}


GSList *
moo_lang_mgr_get_available_langs (MooLangMgr *mgr)
{
    GSList *langs = NULL;
    const GSList *list;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    read_langs (mgr);

    list = gtk_source_languages_manager_get_available_languages (GTK_SOURCE_LANGUAGES_MANAGER (mgr));

    while (list)
    {
        langs = g_slist_prepend (langs, g_object_ref (list->data));
        list = list->next;
    }

    return g_slist_reverse (langs);
}


static GSList *
get_string_list (MooLangMgr *mgr,
                 const char *lang_id,
                 GHashTable *hash,
                 gpointer    func)
{
    char *id;
    MooLang *lang;
    GSList *extensions;
    gpointer orig_key;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    id = _moo_lang_id_from_name (lang_id);

    if (g_hash_table_lookup_extended (hash, id, &orig_key, (gpointer*) &extensions))
    {
        g_free (id);
        return string_list_copy (extensions);
    }

    lang = moo_lang_mgr_get_lang (mgr, id);
    g_free (id);

    if (lang)
    {
        GSList *(*get_stuff) (GtkSourceLanguage*) = func;
        return get_stuff (GTK_SOURCE_LANGUAGE (lang));
    }

    return NULL;
}


GSList *
_moo_lang_mgr_get_extensions (MooLangMgr *mgr,
                              const char *lang_id)
{
    return get_string_list (mgr, lang_id, mgr->mime_types, gtk_source_language_get_globs);
}


GSList *
_moo_lang_mgr_get_mime_types (MooLangMgr *mgr,
                              const char *lang_id)
{
    return get_string_list (mgr, lang_id, mgr->mime_types, gtk_source_language_get_mime_types);
}


static gboolean
string_list_equal (GSList *list1,
                   GSList *list2)
{
    while (list1 && list2)
    {
        if (strcmp (list1->data, list2->data) != 0)
            return FALSE;

        list1 = list1->next;
        list2 = list2->next;
    }

    return list1 == list2;
}

static void
set_strings_list (MooLangMgr *mgr,
                  const char *lang_id,
                  const char *string,
                  GHashTable *hash,
                  gpointer    func)
{
    GSList *list;
    GSList *old_list = NULL;
    MooLang *lang;
    char *id;

    id = _moo_lang_id_from_name (lang_id);
    lang = g_hash_table_lookup (mgr->langs, id);

    if (lang)
    {
        GSList *(*get_stuff) (GtkSourceLanguage*) = func;
        old_list = get_stuff (GTK_SOURCE_LANGUAGE (lang));
    }

    list = parse_string_list (string);

    if (string_list_equal (old_list, list))
    {
        g_hash_table_remove (hash, id);
        string_list_free (list);
    }
    else
    {
        g_hash_table_insert (hash, g_strdup (id), list);
    }

    g_free (id);
    string_list_free (old_list);
}

void
_moo_lang_mgr_set_mime_types (MooLangMgr *mgr,
                              const char *lang_id,
                              const char *mime)
{
    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    set_strings_list (mgr, lang_id, mime, mgr->mime_types,
                      gtk_source_language_get_mime_types);
}


void
_moo_lang_mgr_set_extensions (MooLangMgr *mgr,
                              const char *lang_id,
                              const char *extensions)
{
    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    set_strings_list (mgr, lang_id, extensions, mgr->extensions,
                      gtk_source_language_get_globs);
}


const char *
_moo_lang_mgr_get_config (MooLangMgr *mgr,
                          const char *lang_id)
{
    char *id;
    const char *config;

    g_return_val_if_fail (MOO_IS_LANG_MGR (mgr), NULL);

    id = _moo_lang_id_from_name (lang_id);
    config = g_hash_table_lookup (mgr->config, id);

    g_free (id);
    return config;
}


void
_moo_lang_mgr_set_config (MooLangMgr *mgr,
                          const char *lang_id,
                          const char *config)
{
    char *norm = NULL;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));

    if (config && config[0])
    {
        norm = g_strstrip (g_strdup (config));

        if (!norm[0])
        {
            g_free (norm);
            norm = NULL;
        }
    }

    g_hash_table_insert (mgr->config, _moo_lang_id_from_name (lang_id), norm);
}


void
_moo_lang_mgr_update_config (MooLangMgr     *mgr,
                             MooEditConfig  *config,
                             const char     *lang_id)
{
    const char *lang_config;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));
    g_return_if_fail (MOO_IS_EDIT_CONFIG (config));

    g_object_freeze_notify (G_OBJECT (config));

    moo_edit_config_unset_by_source (config, MOO_EDIT_CONFIG_SOURCE_LANG);

    lang_config = _moo_lang_mgr_get_config (mgr, lang_id);

    if (lang_config)
        moo_edit_config_parse (config, lang_config,
                               MOO_EDIT_CONFIG_SOURCE_LANG);

    g_object_thaw_notify (G_OBJECT (config));
}


static MooMarkupNode *
create_lang_node (MooMarkupNode *root,
                  MooMarkupNode *lang_node,
                  const char    *lang_id)
{
    if (!lang_node)
    {
        lang_node = moo_markup_create_element (root, ELEMENT_LANG);
        moo_markup_set_prop (lang_node, PROP_LANG_ID, lang_id);
    }

    return lang_node;
}

static char *
list_to_string (GSList *list)
{
    GString *string = g_string_new (NULL);

    while (list)
    {
        g_string_append (string, list->data);

        if ((list = list->next))
            g_string_append_c (string, ';');
    }

    return g_string_free (string, FALSE);
}

static void
save_one_lang (MooMarkupNode *root,
               const char    *lang_id,
               const char    *config,
               GSList        *extensions,
               gboolean       save_extensions,
               GSList        *mimetypes,
               gboolean       save_mimetypes)
{
    MooMarkupNode *lang_node = NULL;

    if (save_extensions)
    {
        char *string = list_to_string (extensions);
        lang_node = create_lang_node (root, lang_node, lang_id);
        moo_markup_create_text_element (lang_node, ELEMENT_EXTENSIONS, string);
        g_free (string);
    }

    if (save_mimetypes)
    {
        char *string = list_to_string (mimetypes);
        lang_node = create_lang_node (root, lang_node, lang_id);
        moo_markup_create_text_element (lang_node, ELEMENT_MIME_TYPES, string);
        g_free (string);
    }

    if (config)
    {
        lang_node = create_lang_node (root, lang_node, lang_id);
        moo_markup_create_text_element (lang_node, ELEMENT_CONFIG, config);
    }
}

static void
save_one (MooLangMgr     *mgr,
          MooLang        *lang,
          const char     *id,
          MooMarkupDoc   *xml,
          MooMarkupNode **root)
{
    const char *config;
    GSList *extensions = NULL, *mimetypes = NULL;
    gboolean has_extensions, has_mimetypes;
    gpointer dummy;

    id = lang ? _moo_lang_id (lang) : id;
    g_return_if_fail (id != NULL);

    config = g_hash_table_lookup (mgr->config, id);
    has_extensions = g_hash_table_lookup_extended (mgr->extensions, id, &dummy, (gpointer*) &extensions);
    has_mimetypes = g_hash_table_lookup_extended (mgr->mime_types, id, &dummy, (gpointer*) &mimetypes);

    if (!config && !has_extensions && !has_mimetypes)
        return;

    if (!*root)
        *root = moo_markup_create_element (MOO_MARKUP_NODE (xml), ELEMENT_LANG_CONFIG);

    g_return_if_fail (*root != NULL);

    save_one_lang (*root, id, config,
                   extensions, has_extensions,
                   mimetypes, has_mimetypes);
}

void
_moo_lang_mgr_save_config (MooLangMgr *mgr)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root;
    GSList *langs;

    g_return_if_fail (MOO_IS_LANG_MGR (mgr));

    xml = moo_prefs_get_markup ();
    g_return_if_fail (xml != NULL);

    root = moo_markup_get_element (MOO_MARKUP_NODE (xml), ELEMENT_LANG_CONFIG);

    if (root)
        moo_markup_delete_node (root);

    root = NULL;

    save_one (mgr, NULL, MOO_LANG_NONE, xml, &root);

    langs = moo_lang_mgr_get_available_langs (mgr);

    while (langs)
    {
        MooLang *lang = langs->data;
        save_one (mgr, lang, _moo_lang_id (lang), xml, &root);
        g_object_unref (lang);
        langs = g_slist_delete_link (langs, langs);
    }
}
