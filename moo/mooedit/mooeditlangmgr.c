/*
 *   mooedit/mooeditlangmgr.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mooedit/mooeditlangmgr.h"
#include "mooutils/moocompat.h"
#include <string.h>
#include "xdgmime/xdgmime.h"


#define LANG_FILE_SUFFIX        ".lang"
#define LANG_FILE_SUFFIX_LEN    5


struct _MooEditLangMgrPrivate {
    GSList          *lang_dirs;
    GSList          *langs;
    MooEditLang     *default_lang;
};


static void moo_edit_lang_mgr_class_init        (MooEditLangMgrClass    *klass);
static void moo_edit_lang_mgr_init              (MooEditLangMgr         *mgr);
static void moo_edit_lang_mgr_finalize          (GObject                *object);

static gboolean moo_edit_lang_mgr_read_lang_file(MooEditLangMgr         *mgr,
                                                 const char             *filename);


/* MOO_TYPE_EDIT_LANG_MGR */
G_DEFINE_TYPE (MooEditLangMgr, moo_edit_lang_mgr, G_TYPE_OBJECT)


static void moo_edit_lang_mgr_class_init    (MooEditLangMgrClass   *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = moo_edit_lang_mgr_finalize;
}


static void moo_edit_lang_mgr_init          (MooEditLangMgr        *mgr)
{
    mgr->priv = g_new0 (MooEditLangMgrPrivate, 1);
}


static void moo_edit_lang_mgr_finalize      (GObject            *object)
{
    MooEditLangMgr *mgr = MOO_EDIT_LANG_MGR (object);

    g_slist_foreach (mgr->priv->lang_dirs, (GFunc) g_free, NULL);
    g_slist_foreach (mgr->priv->langs, (GFunc) g_object_unref, NULL);
    g_slist_free (mgr->priv->lang_dirs);
    g_slist_free (mgr->priv->langs);
    g_free (mgr->priv);

    G_OBJECT_CLASS (moo_edit_lang_mgr_parent_class)->finalize (object);
}


const GSList    *moo_edit_lang_mgr_get_available_languages      (MooEditLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr), NULL);
    return mgr->priv->langs;
}


void             moo_edit_lang_mgr_add_language                 (MooEditLangMgr *mgr,
                                                                 MooEditLang    *lang)
{
    MooEditLang *old = NULL;
    const char *id;

    g_return_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && MOO_IS_EDIT_LANG (lang));
    g_return_if_fail (!g_slist_find (mgr->priv->langs, lang));

    id = moo_edit_lang_get_id (lang);
    g_assert (id != NULL);
    old = moo_edit_lang_mgr_get_language_by_id (mgr, id);

    if (old)
    {
#ifdef DEBUG_LANGS
        g_message ("%s: loading another instance of language '%s'", G_STRLOC, id);
#endif
        g_slist_remove (mgr->priv->langs, old);
        g_object_unref (G_OBJECT (old));
    }

    mgr->priv->langs = g_slist_prepend (mgr->priv->langs, lang);
    g_object_ref (G_OBJECT (lang));

    if (old && old == mgr->priv->default_lang)
        mgr->priv->default_lang = lang;
}


MooEditLang     *moo_edit_lang_mgr_get_language_by_id           (MooEditLangMgr *mgr,
                                                                 const char     *lang_id)
{
    GSList *l;

    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && lang_id != NULL, NULL);

    for (l = mgr->priv->langs; l != NULL; l = l->next)
        if (!strcmp (moo_edit_lang_get_id (MOO_EDIT_LANG(l->data)), lang_id))
            return MOO_EDIT_LANG (l->data);
    return NULL;
}


MooEditLang     *moo_edit_lang_mgr_get_language_for_file        (MooEditLangMgr *mgr,
                                                                 const char     *filename)
{
    MooEditLang *lang = NULL;
    char *utf8_filename;
    GError *err = NULL;

    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && filename != NULL, NULL);

    utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, &err);
    if (!utf8_filename) {
        g_critical ("%s: could not convert filename to UTF8", G_STRLOC);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }

    if (utf8_filename) {
        GSList *l;
        gboolean found = FALSE;

        for (l = mgr->priv->langs; l != NULL && !found; l = l->next) {
            GSList *globs, *g;
            lang = MOO_EDIT_LANG (l->data);
            globs = moo_edit_lang_get_extensions (lang);
            for (g = globs; g != NULL && !found; g = g->next) {
                if (g_pattern_match_simple ((char*) g->data, utf8_filename))
                    found = TRUE;
            }
            g_slist_foreach (globs, (GFunc) g_free, NULL);
            g_slist_free (globs);
        }

        if (!found)
            lang = NULL;

#ifdef DEBUG_LANGS
        if (found)
            g_message ("%s: found lang '%s' for file '%s' by extension",
                       G_STRLOC, moo_edit_lang_get_id (lang), filename);
#endif
    }

#ifdef USE_XDGMIME
    /* TODO: xdgmime wants utf8-encoded filename here. is it a problem? */
    if (!lang)
    {
        const char *mime_type = xdg_mime_get_mime_type_for_file (filename);
        if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
            lang = moo_edit_lang_mgr_get_language_for_mime_type (mgr, mime_type);
#ifdef DEBUG_LANGS
        if (lang)
            g_message ("%s: found lang '%s' for file '%s' by mime type",
                       G_STRLOC, moo_edit_lang_get_id (lang), filename);
#endif
    }
#endif /* USE_XDGMIME */

    /* check if it's backup file */
    if (!lang && utf8_filename)
    {
        char *base = NULL;
        int len = strlen (utf8_filename);
        guint i;

        static const char *bak_globs[] = {"*~", "*.bak", "*.in"};

        for (i = 0; i < G_N_ELEMENTS (bak_globs); ++i)
        {
            int ext_len = strlen (bak_globs[i]) - 1;
            if (len > ext_len && g_pattern_match_simple (bak_globs[i], utf8_filename))
            {
                base = g_strndup (utf8_filename, len - ext_len);
                break;
            }
        }

        if (base)
        {
            lang = moo_edit_lang_mgr_get_language_for_filename (mgr, base);
            g_free (base);
        }
    }

#ifdef DEBUG_LANGS
    if (!lang)
        g_message ("%s: could not find lang for file '%s'", G_STRLOC, filename);
#endif

    return lang;
}


MooEditLang     *moo_edit_lang_mgr_get_language_for_mime_type   (MooEditLangMgr *mgr,
                                                                 const char     *mime_type)
{
    GSList *l;
    MooEditLang *lang = NULL;
    gboolean found = FALSE;

    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && mime_type != NULL, NULL);

    for (l = mgr->priv->langs; l != NULL && !found; l = l->next) {
        GSList *mime_types;

        lang = MOO_EDIT_LANG (l->data);
        mime_types = moo_edit_lang_get_mime_types (lang);
        if (g_slist_find_custom (mime_types, mime_type, (GCompareFunc) strcmp))
            found = TRUE;
        g_slist_foreach (mime_types, (GFunc) g_free, NULL);
        g_slist_free (mime_types);
    }

    if (found)
        return lang;
    else
        return NULL;
}


MooEditLang     *moo_edit_lang_mgr_get_language_for_filename    (MooEditLangMgr *mgr,
                                                                 const char     *filename)
{
    MooEditLang *lang = NULL;
    char *utf8_filename;
    GError *err = NULL;

    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && filename != NULL, NULL);

    utf8_filename = g_filename_to_utf8 (filename, -1, NULL, NULL, &err);
    if (!utf8_filename) {
        g_critical ("%s: could not convert filename to UTF8", G_STRLOC);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
    }

    if (utf8_filename) {
        GSList *l;
        gboolean found = FALSE;

        for (l = mgr->priv->langs; l != NULL && !found; l = l->next) {
            GSList *globs, *g;
            lang = MOO_EDIT_LANG (l->data);
            globs = moo_edit_lang_get_extensions (lang);
            for (g = globs; g != NULL && !found; g = g->next) {
                if (g_pattern_match_simple ((char*) g->data, utf8_filename))
                    found = TRUE;
            }
            g_slist_foreach (globs, (GFunc) g_free, NULL);
            g_slist_free (globs);
        }

        if (!found)
            lang = NULL;

#ifdef DEBUG_LANGS
        if (found)
            g_message ("%s: found lang '%s' for file '%s' by extension",
                       G_STRLOC, moo_edit_lang_get_id (lang), filename);
#endif
    }

#ifdef USE_XDGMIME
    /* TODO: xdgmime wants utf8-encoded filename here. is it a problem? */
    if (!lang)
    {
        const char *mime_type = xdg_mime_get_mime_type_from_file_name (filename);
        if (!xdg_mime_mime_type_equal (mime_type, XDG_MIME_TYPE_UNKNOWN))
            lang = moo_edit_lang_mgr_get_language_for_mime_type (mgr, mime_type);
#ifdef DEBUG_LANGS
        if (lang)
            g_message ("%s: found lang '%s' for file '%s' by mime type",
                       G_STRLOC, moo_edit_lang_get_id (lang), filename);
#endif
    }
#endif /* USE_XDGMIME */

    if (!lang)
        g_message ("%s: could not find lang for file '%s'", G_STRLOC, filename);

    return lang;
}


const GSList    *moo_edit_lang_mgr_get_lang_files_dirs          (MooEditLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr), NULL);
    return mgr->priv->lang_dirs;
}


void             moo_edit_lang_mgr_add_lang_files_dir           (MooEditLangMgr *mgr,
                                                                 const char     *dirname)
{
    GError *err = NULL;
    GDir *dir;
    const char *entry;

#ifndef USE_XML
    g_warning ("%s: xml support disabled, can't load lang files", G_STRLOC);
    return;
#endif /* USE_XML */

    g_return_if_fail (MOO_IS_EDIT_LANG_MGR (mgr) && dirname != NULL);
    g_return_if_fail (!g_slist_find_custom (mgr->priv->lang_dirs, dirname,
                                            (GCompareFunc)strcmp));

    dir = g_dir_open (dirname, 0, &err);
    if (!dir) {
        g_critical ("%s: could not open directory '%s'", G_STRLOC, dirname);
        if (err) {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }
        return;
    }

    for (entry = g_dir_read_name (dir); entry != NULL; entry = g_dir_read_name (dir))
    {
        char *file = g_build_filename (dirname, entry, NULL);

        if (file && g_file_test (file, G_FILE_TEST_IS_REGULAR | G_FILE_TEST_IS_SYMLINK))
        {
            guint len = strlen (file);
            if (len >= LANG_FILE_SUFFIX_LEN &&
                !strcmp (LANG_FILE_SUFFIX, file + (len - LANG_FILE_SUFFIX_LEN)))
                    moo_edit_lang_mgr_read_lang_file (mgr, file);
        }

        g_free (file);
    }

    g_dir_close (dir);
    g_slist_prepend (mgr->priv->lang_dirs, g_strdup (dirname));
}


static gboolean moo_edit_lang_mgr_read_lang_file(MooEditLangMgr         *mgr,
                                                 const char             *filename)
{
    MooEditLang *lang = moo_edit_lang_new_from_file (filename);
    g_return_val_if_fail (lang != NULL, FALSE);
    moo_edit_lang_mgr_add_language (mgr, lang);
    g_object_unref (G_OBJECT (lang));
    return TRUE;
}


MooEditLangMgr  *moo_edit_lang_mgr_new                          (void)
{
    return MOO_EDIT_LANG_MGR (g_object_new (MOO_TYPE_EDIT_LANG_MGR, NULL));
}


MooEditLang     *moo_edit_lang_mgr_get_default_language         (MooEditLangMgr *mgr)
{
    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr), NULL);
    return mgr->priv->default_lang;
}


void             moo_edit_lang_mgr_set_default_language         (MooEditLangMgr *mgr,
                                                                 MooEditLang    *lang)
{
    g_return_if_fail (MOO_IS_EDIT_LANG_MGR (mgr));
    g_return_if_fail (lang == NULL ||
                      g_slist_find (mgr->priv->langs, lang) != NULL);
    mgr->priv->default_lang = lang;
}


GSList          *moo_edit_lang_mgr_get_sections                 (MooEditLangMgr *mgr)
{
    GSList *list = NULL;
    GSList *l;

    g_return_val_if_fail (MOO_IS_EDIT_LANG_MGR (mgr), NULL);

    for (l = mgr->priv->langs; l != NULL; l = l->next)
    {
        const char *section =
                moo_edit_lang_get_section (MOO_EDIT_LANG (l->data));
        if (!section)
            section = "Others";
        if (!g_slist_find_custom (list, section, (GCompareFunc) strcmp))
             list = g_slist_prepend (list, g_strdup (section));
    }

    return list;
}
