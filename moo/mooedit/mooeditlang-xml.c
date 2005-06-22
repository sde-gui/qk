/*
 *   mooedit/mooeditlang-xml.c
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

/*
 * Pretty big part of this file is taken from/based on gtksourceview/gtksourcelang.c
 *  Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditlang-private.h"
#include "mooedit/mooeditprefs.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>

/****************************************************************************/
/*
 * lang files are the same as gtksourceview's with some additions:
 *
 * lang file looks like the following:

<!-- header is ignored -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE language SYSTEM "language.dtd">

<!-- version is ignored -->
<!-- 'extensions' are really globs, but word 'extensions' sounds better -->
<!-- it accepts both 'name' and '_name'; and both 'section' and '_section' -->
<language   name="name"
            version="1.0"
            section="section"
            mimetypes="text/x-c;text/x-chdr;text/x-csrc"
            extensions="*.c"
            author="Copyright by Some Guy or whatever">

    <brackets><!-- in some format --></brackets>
    <word-chars><!-- in some format --></word-chars>
    <comments><!-- in some format --></comments>

    <!-- then the same stuff as in gtksourceview lang files -->

    <styles>
        <style  name="The style name used in tag definition" default_style="Default style name"
                bold="0" italic="TRUE" underline="FALSE" strikethrough="no"
                foreground="blue" background="#000" />
        <!-- etc. -->
    </styles>

</language>

 */
/****************************************************************************/

#define MIME_TYPES_DELIMITER ";"


typedef enum {
    BOLD            = 1 << 0,
    ITALIC          = 1 << 1,
    UNDERLINE       = 1 << 2,
    STRIKETHROUGH   = 1 << 3
} StyleAttrMask;

typedef struct {
    char *name;
    char *default_style;
    guint bold              : 1;
    guint italic            : 1;
    guint underline         : 1;
    guint strikethrough     : 1;
    StyleAttrMask attr_mask;
    char *foreground;
    char *background;
} Style;

typedef struct {
    gboolean     error;
    const char  *filename;
    char        *name;
    char        *section;
    char        *mimetypes;
    char        *extensions;
    char        *author;
    GSList      *style_names;
    GPtrArray   *styles;        /* Style* */
} LangDescription;


static void                  free_string_list   (GSList     *list);
static void                  free_xmlstring_list(GSList     *list);
static GSList               *split_mime_types   (const char *str);
static GSList               *split_extensions   (const char *str);
static Style                *style_new          (void);
static void                  style_free         (Style      *style);
static gboolean              get_bool           (const char *str);
static GtkSourceTagStyle    *create_style       (const char *lang_id,
                                                 const char *style_name,
                                                 GPtrArray  *styles);


#define assign_string(var,string)   \
    g_free (var);                   \
    var = g_strdup (string);


static LangDescription *lang_description_new (const char *filename);
static void lang_description_free (LangDescription *desc);
static void load_desc_start_element (LangDescription    *desc,
                                     const char         *name,
                                     const char        **attrs);

gboolean    moo_edit_lang_load_description  (MooEditLang *lang)
{
    xmlSAXHandler sax;
    LangDescription *desc;
    GSList *l;

    if (lang->priv->description_loaded) return TRUE;

    g_return_val_if_fail (lang->priv->filename != NULL, FALSE);

    memset (&sax, 0, sizeof(sax));
    sax.startElement = (startElementSAXFunc)load_desc_start_element;
    desc = lang_description_new (lang->priv->filename);

    if (xmlSAXUserParseFile (&sax, desc, lang->priv->filename)) {
        g_critical ("%s: error in xmlSAXUserParseFile (%s)",
                    G_STRLOC, lang->priv->filename);
        lang_description_free (desc);
        return FALSE;
    }

    if (desc->error) {
        g_critical ("%s: error in loading language description from file '%s'",
                    G_STRLOC, lang->priv->filename);
        lang_description_free (desc);
        return FALSE;
    }

    if (!desc->name) {
        g_critical ("%s: language doesn't have 'name' attribute in file '%s'",
                    G_STRLOC, lang->priv->filename);
        lang_description_free (desc);
        return FALSE;
    }

    if (!desc->section) {
        g_warning ("%s: language '%s' doesn't have 'section' attribute in file '%s'",
                   G_STRLOC, desc->name, lang->priv->filename);
        desc->section = g_strdup (MOO_EDIT_LANG_SECTION_OTHERS);
    }

    free_string_list (lang->priv->mime_types);
    lang->priv->mime_types = split_mime_types (desc->mimetypes);
    free_string_list (lang->priv->extensions);
    lang->priv->extensions = split_extensions (desc->extensions);

    assign_string (lang->priv->id, desc->name);
    assign_string (lang->priv->name, desc->name);
    assign_string (lang->priv->section, desc->section);
    assign_string (lang->priv->author, desc->author);

    for (l = desc->style_names; l; l = l->next) {
        const char *style_name;
        GtkSourceTagStyle *style;

        style_name = (const char*)l->data;
        style = create_style (lang->priv->id, style_name, desc->styles);

        g_hash_table_insert (lang->priv->style_id_to_style,
                             g_strdup (style_name), style);
        g_hash_table_insert (lang->priv->style_id_to_style_name,
                             g_strdup (style_name), g_strdup (style_name));
    }

    lang_description_free (desc);

    lang->priv->description_loaded = TRUE;
    return TRUE;
}


static LangDescription *lang_description_new (const char *filename)
{
    LangDescription *desc = g_new0 (LangDescription, 1);
    desc->styles = g_ptr_array_new ();
    desc->filename = filename;
    return desc;
}

static void lang_description_free (LangDescription *desc)
{
    guint i;

    g_return_if_fail (desc != NULL);

    g_free (desc->name);
    g_free (desc->section);
    g_free (desc->mimetypes);
    g_free (desc->extensions);
    g_free (desc->author);

    free_string_list (desc->style_names);

    for (i = 0; i < desc->styles->len; ++i)
        style_free ((Style*) desc->styles->pdata[i]);
    g_ptr_array_free (desc->styles, TRUE);

    g_free (desc);
}


static void load_desc_start_element (LangDescription    *desc,
                                     const char         *elm_name,
                                     const char        **attrs)
{
    const char **attr;

    if (desc->error) return;

    g_return_if_fail (elm_name != NULL);

    if (!strcmp (elm_name, "line-comment") ||
        !strcmp (elm_name, "string") ||
        !strcmp (elm_name, "keyword-list") ||
        !strcmp (elm_name, "pattern-item") ||
        !strcmp (elm_name, "syntax-item") ||
        !strcmp (elm_name, "block-comment"))
    {
        const char *tag_name = NULL;
        const char *style = NULL;

        for (attr = attrs; *attr != NULL; ++attr) {
            if (!strcmp (*attr, "name") || !strcmp (*attr, "_name"))
            {
                ++attr;
                tag_name = *attr;
            }
            else if (!strcmp (*attr, "style") || !strcmp (*attr, "_style"))
            {
                ++attr;
                style = *attr;
                break;
            }
            else
                ++attr;
        }

        if (style)
        {
            desc->style_names =
                g_slist_prepend (desc->style_names, g_strdup (style));
        }
        else if (tag_name)
        {
            desc->style_names =
                g_slist_prepend (desc->style_names, g_strdup (tag_name));
        }
        else
        {
            g_critical ("%s: syntax tag without name or style in file '%s'",
                        G_STRLOC, desc->filename ? desc->filename : "???");
            desc->error = FALSE;
        }

        return;
    }

    if (!strcmp (elm_name, "styles"))
        return;

    if (!strcmp (elm_name, "style"))
    {
        Style *style = style_new ();
        g_ptr_array_add (desc->styles, style);

        for (attr = attrs; *attr != NULL; ++attr) {
            if (!strcmp (*attr, "name"))
            {
                ++attr;
                style->name = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "default_style"))
            {
                ++attr;
                style->default_style = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "foreground")) {
                ++attr;
                style->foreground = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "background")) {
                ++attr;
                style->background = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "bold")) {
                ++attr;
                style->bold = get_bool (*attr);
                style->attr_mask |= BOLD;
            }
            else if (!strcmp (*attr, "italic")) {
                ++attr;
                style->italic = get_bool (*attr);
                style->attr_mask |= ITALIC;
            }
            else if (!strcmp (*attr, "underline")) {
                ++attr;
                style->underline = get_bool (*attr);
                style->attr_mask |= UNDERLINE;
            }
            else if (!strcmp (*attr, "strikethrough")) {
                ++attr;
                style->strikethrough = get_bool (*attr);
                style->attr_mask |= STRIKETHROUGH;
            }
            else {
                g_critical ("%s: unknown style attribute '%s' in file '%s'",
                            G_STRLOC, *attr,
                            desc->filename ? desc->filename : "???");
                ++attr;
            }
        }

        return;
    }

    if (!strcmp (elm_name, "brackets") ||
        !strcmp (elm_name, "word-chars") ||
        !strcmp (elm_name, "comments"))
            return;

    if (!strcmp (elm_name, "language"))
    {
        g_return_if_fail (attrs != NULL);
        for (attr = attrs; *attr != NULL; ++attr) {
            if (!strcmp (*attr, "_name") ||
                !strcmp (*attr, "name"))
            {
                ++attr;
                if (desc->name) {
                    g_warning ("%s: both 'name' and '_name' attributes specified", G_STRLOC);
                    g_free (desc->name);
                }
                desc->name = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "_section") ||
                    !strcmp (*attr, "section"))
            {
                ++attr;
                if (desc->section) {
                    g_warning ("%s: both 'section' and '_section' attributes specified", G_STRLOC);
                    g_free (desc->section);
                }
                desc->section = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "mimetypes")) {
                ++attr;
                desc->mimetypes = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "extensions")) {
                ++attr;
                desc->extensions = g_strdup (*attr);
            }
            else if (!strcmp (*attr, "version")) {
                ++attr;
            }
            else {
                g_warning ("%s: unknown language attribute %s", G_STRLOC, *attr);
                ++attr;
            }
        }
    }
}


static GtkSourceTagStyle    *create_style       (const char *lang_id,
                                                 const char *style_id,
                                                 GPtrArray  *styles)
{
    Style *s = NULL;
    GtkSourceTagStyle *style;
    guint i;

    g_return_val_if_fail (style_id != NULL,
                          gtk_source_tag_style_new());

    style = moo_edit_lang_get_default_style (style_id);
    if (!style) style = gtk_source_tag_style_new();

    for (i = 0; i < styles->len; ++i) {
        Style *s = g_ptr_array_index (styles, i);
        if (s->name && !strcmp (style_id, s->name))
            break;
    }
    if (i < styles->len) s = g_ptr_array_index (styles, i);

    if (s) {
        GtkSourceTagStyle *default_style =
            moo_edit_lang_get_default_style (s->default_style);

        if (default_style) {
            *style = *default_style;
            gtk_source_tag_style_free (default_style);
        }
    }

    if (s)
    {
        GdkColor color;

        if (s->foreground) {
            if (gdk_color_parse (s->foreground, &color)) {
                style->foreground = color;
                style->mask |= GTK_SOURCE_TAG_STYLE_USE_FOREGROUND;
            }
            else {
                g_warning ("could not parse color '%s' for tag style '%s'",
                           s->foreground, style_id);
            }
        }

        if (s->background) {
            if (gdk_color_parse (s->background, &color)) {
                style->background = color;
                style->mask |= GTK_SOURCE_TAG_STYLE_USE_BACKGROUND;
            }
            else {
                g_warning ("could not parse color '%s' for tag style '%s'",
                           s->background, style_id);
            }
        }

        if (s->attr_mask & BOLD)
            style->bold = s->bold;
        if (s->attr_mask & ITALIC)
            style->italic = s->italic;
        if (s->attr_mask & UNDERLINE)
            style->underline = s->underline;
        if (s->attr_mask & STRIKETHROUGH)
            style->strikethrough = s->strikethrough;
    }

    moo_edit_style_load (lang_id, style_id, style);

    return style;
}


/****************************************************************************/
/* Full loading                                                             */
/***/

#define KEYWORDS_NUM_LIMIT 1000

static gchar   *strconvescape (gchar *source);
static gboolean parse_brackets              (const char     *string,
                                             gunichar      **left,
                                             gunichar      **right,
                                             guint          *num);
static gboolean parse_word_chars            (const char     *string,
                                             gunichar      **chars,
                                             guint          *num);

static void     add_tag                     (MooEditLang    *lang,
                                             GtkSourceTag   *tag,
                                             const char     *style_id);

static gboolean process_lang_element_node   (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_brackets_node       (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_word_chars_node     (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_comments_node       (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_escape_char_node    (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_line_comment_node   (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_syntax_item_node    (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_string_node         (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_pattern_item_node   (MooEditLang    *lang,
                                             xmlNode        *node);
static gboolean process_keyword_list_node   (MooEditLang    *lang,
                                             xmlNode        *node);


gboolean    moo_edit_lang_load_full         (MooEditLang *lang)
{
    xmlDoc *doc;
    xmlNode *root, *elm;

    g_return_val_if_fail (MOO_IS_EDIT_LANG (lang) &&
                          lang->priv->filename != NULL, FALSE);

    if (lang->priv->loaded) return TRUE;
    if (!moo_edit_lang_load_description (lang)) return FALSE;
    lang->priv->loaded = TRUE;

    LIBXML_TEST_VERSION

#ifdef HAVE_XMLREADFILE
    doc = xmlReadFile (lang->priv->filename, NULL, 0);
#else /* ! HAVE_XMLREADFILE */
    doc = xmlParseFile (lang->priv->filename);
#endif /* ! HAVE_XMLREADFILE */

    if (doc == NULL) {
        g_critical ("%s: could not parse file '%s'",
                    G_STRLOC, lang->priv->filename);
        xmlCleanupParser();
        lang->priv->loaded = FALSE;
        return FALSE;
    }

    root = xmlDocGetRootElement (doc);
    if (xmlStrcmp (root->name, (const xmlChar*)"language")) {
        g_critical ("%s: root element in file '%s'"
                    "is not 'language'", G_STRLOC,
                    lang->priv->filename);
        xmlFreeDoc (doc);
        xmlCleanupParser();
        lang->priv->loaded = FALSE;
        return FALSE;
    }

    for (elm = root->children; elm != NULL; elm = elm->next)
        process_lang_element_node (lang, elm);

    lang->priv->tags = g_slist_reverse (lang->priv->tags);

    xmlFreeDoc (doc);
    xmlCleanupParser();

    return TRUE;
}


static void     add_tag                     (MooEditLang    *lang,
                                             GtkSourceTag   *tag,
                                             const char     *style_id)
{
    char *tag_id;
    GtkSourceTagStyle *style;

    g_return_if_fail (tag != NULL);

    lang->priv->tags = g_slist_prepend (lang->priv->tags, tag);

    tag_id = gtk_source_tag_get_id (tag);

    if (!style_id)
        style_id = tag_id;

    g_hash_table_insert (lang->priv->tag_id_to_style_id,
                         g_strdup (tag_id), g_strdup (style_id));

    style = g_hash_table_lookup (lang->priv->style_id_to_style, style_id);
    if (!style) {
        g_warning ("%s: no style with id '%s'", G_STRLOC, style_id);
        style = gtk_source_tag_style_new ();
        g_hash_table_insert (lang->priv->style_id_to_style,
                            g_strdup (style_id), style);

        g_hash_table_insert (lang->priv->style_id_to_style_name,
                            g_strdup (style_id), g_strdup (style_id));
    }

    gtk_source_tag_set_style (tag, style);
    g_free (tag_id);
}


static gchar *strconvescape (gchar *source)
{
	gchar cur_char;
	gchar last_char = '\0';
	gint iterations = 0;
	gint max_chars;
	gchar *dest;

	if (source == NULL)
		return NULL;

	max_chars = strlen (source);
	dest = source;

	for (iterations = 0; iterations < max_chars; iterations++) {
		cur_char = source[iterations];
		*dest = cur_char;
		if (last_char == '\\' && cur_char == 'n') {
			dest--;
			*dest = '\n';
		} else if (last_char == '\\' && cur_char == 't') {
			dest--;
			*dest = '\t';
		}
		last_char = cur_char;
		dest++;
	}
	*dest = '\0';

	return source;
}


static gboolean process_lang_element_node   (MooEditLang    *lang,
                                             xmlNode        *node)
{
    if (node->type == XML_ELEMENT_NODE)
    {
        if (!xmlStrcmp (node->name, (const xmlChar*)"escape-char"))
            return process_escape_char_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"brackets"))
            return process_brackets_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"comments"))
            return process_comments_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"word-chars"))
            return process_word_chars_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"line-comment"))
            return process_line_comment_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"block-comment"))
            return process_syntax_item_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"string"))
            return process_string_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"syntax-item"))
            return process_syntax_item_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"keyword-list"))
            return process_keyword_list_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"pattern-item"))
            return process_pattern_item_node (lang, node);

        else if (!xmlStrcmp (node->name, (const xmlChar*)"styles"))
            return TRUE;

        else {
#ifdef HAVE_XMLNODE_LINE
            g_critical ("%s: unknown tag '%s' at line %d in file '%s'",
                        G_STRLOC, node->name, node->line,
                        lang->priv->filename ? lang->priv->filename : "???");
#else /* !HAVE_XMLNODE_LINE */
            g_critical ("%s: unknown tag '%s' in file '%s'",
                        G_STRLOC, node->name,
                        lang->priv->filename ? lang->priv->filename : "???");
#endif /* !HAVE_XMLNODE_LINE */
            return TRUE;
        }
    }
    else return TRUE;
}


static gboolean process_brackets_node       (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *val;

    val = xmlNodeGetContent (node);

    if (!val) {
        g_critical ("%s: no content", G_STRLOC);
        return FALSE;
    }

    if (!parse_brackets ((char*) val, NULL, NULL, NULL)) {
        g_critical ("%s: could not parse brackets in '%s'",
                    G_STRLOC, (char*) val);
        xmlFree (val);
        return FALSE;
    }

    g_free (lang->priv->brackets);
    lang->priv->brackets = g_strdup ((char*)val);

    xmlFree (val);
    return TRUE;
}


static gboolean process_word_chars_node     (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *val;
    gunichar *chars = NULL;
    guint n = 0;

    val = xmlNodeGetContent (node);

    if (!val) {
        g_critical ("%s: no content", G_STRLOC);
        return FALSE;
    }

    if (!parse_word_chars ((char*) val, &chars, &n)) {
        g_critical ("%s: could not parse word_chars in '%s'",
                    G_STRLOC, (char*) val);
        xmlFree (val);
        return FALSE;
    }

    g_free (lang->priv->word_chars);
    lang->priv->word_chars = chars;
    lang->priv->num_word_chars = n;

    xmlFree (val);
    return TRUE;
}


static gboolean process_comments_node       (G_GNUC_UNUSED MooEditLang    *lang,
                                             G_GNUC_UNUSED xmlNode        *node)
{
    g_warning ("%s: implement me", G_STRLOC);
    return FALSE;
}


static gboolean process_escape_char_node    (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *val = xmlNodeGetContent ((xmlNode*)node);
    gunichar esc_char;

    if (!val) {
        g_critical ("%s: no content", G_STRLOC);
        return FALSE;
    }

    esc_char = g_utf8_get_char_validated ((const char*)val, -1);
    xmlFree (val);

    if (esc_char == (gunichar)-1 || esc_char == (gunichar)-2)
    {
        g_critical ("%s: invalid unicode char read", G_STRLOC);
        return FALSE;
    }

    lang->priv->escape_char = esc_char;
    return TRUE;
}


static gboolean process_line_comment_node   (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *name = NULL, *style = NULL;
    xmlChar *start_regex = NULL;
    xmlNode *n;
    GtkTextTag *tag = NULL;
    gboolean success = TRUE;

    name = xmlGetProp (node, (const xmlChar*)"_name");
    if (!name) name = xmlGetProp (node, (const xmlChar*)"name");
    style = xmlGetProp (node, (const xmlChar*)"_style");
    if (!style) style = xmlGetProp (node, (const xmlChar*)"style");

    for (n = node->children; n; n = n->next) {
        if (n->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp (n->name, (const xmlChar*)"start-regex"))
                start_regex = xmlNodeGetContent (n);
            else
                g_critical ("%s: unexpected element '%s'", G_STRLOC,
                            n->name);
            break;
        }
    }

    tag = NULL;
    if (name && start_regex) {
        tag = gtk_line_comment_tag_new ((char*)name, (char*)name,
                                        strconvescape((char*)start_regex));
        if (tag) {
            add_tag (lang, GTK_SOURCE_TAG (tag), style);
        }
        else {
            g_critical ("%s: could not create tag", G_STRLOC);
            success = FALSE;
        }
    }
    else {
        if (!name)
            g_critical ("%s: no name attribute", G_STRLOC);
        if (!start_regex)
            g_critical ("%s: no start_regex attribute", G_STRLOC);
        success = FALSE;
    }

    if (name) xmlFree (name);
    if (style) xmlFree (style);
    if (start_regex) xmlFree (start_regex);
    return success;
}


static gboolean process_syntax_item_node    (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *name, *style;
    xmlChar *start_regex = NULL;
    xmlChar *end_regex = NULL;
    GtkTextTag *tag = NULL;
    gboolean success = TRUE;
    xmlNode *n;

    name = xmlGetProp (node, (const xmlChar*)"_name");
    if (!name) name = xmlGetProp (node, (const xmlChar*)"name");
    style = xmlGetProp (node, (const xmlChar*)"_style");
    if (!style) style = xmlGetProp (node, (const xmlChar*)"style");

    for (n = node->children; n != NULL; n = n->next)
    {
        if (n->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp (n->name, (const xmlChar*)"start-regex"))
                start_regex = xmlNodeGetContent (n);
            else if (!xmlStrcmp (n->name, (const xmlChar*)"end-regex"))
                end_regex = xmlNodeGetContent (n);
            else {
                g_critical ("%s: pattern node has wrong name '%s'",
                            G_STRLOC, n->name);
                success = FALSE;
                break;
            }
        }
    }

    if (success && name && start_regex && end_regex) {
        tag = gtk_block_comment_tag_new ((char*)name, (char*)name,
                                         strconvescape((char*)start_regex),
                                         strconvescape((char*)end_regex));
        if (tag) {
            add_tag (lang, GTK_SOURCE_TAG (tag), style);
        }
        else {
            g_critical ("%s: could not create tag '%s'", G_STRLOC, (char*)name);
            success = FALSE;
        }
    }
    else {
        if (!name)
            g_critical ("%s: no name attribute", G_STRLOC);
        if (!start_regex)
            g_critical ("%s: no start_regex attribute", G_STRLOC);
        if (!end_regex)
            g_critical ("%s: no end_regex attribute", G_STRLOC);
        success = FALSE;
    }

    if (name) xmlFree (name);
    if (style) xmlFree (style);
    if (start_regex) xmlFree (start_regex);
    if (end_regex) xmlFree (end_regex);

    return success;
}


static gboolean process_string_node         (MooEditLang    *lang,
                                             xmlNode        *node)
{
    xmlChar *name, *style;
    xmlChar *start_regex = NULL;
    xmlChar *end_regex = NULL;
    gboolean end_at_line_end = TRUE;
    xmlChar *prop = NULL;
    gboolean success = TRUE;
    GtkTextTag *tag = NULL;
    xmlNode *n;

    name = xmlGetProp (node, (const xmlChar*)"_name");
    if (!name) name = xmlGetProp (node, (const xmlChar*)"name");
    style = xmlGetProp (node, (const xmlChar*)"_style");
    if (!style) style = xmlGetProp (node, (const xmlChar*)"style");

    prop = xmlGetProp (node, (const xmlChar*)"end-at-line-end");
    if (!prop)
        g_critical ("%s: no 'end-at-line-end' property", G_STRLOC);
    else {
        end_at_line_end = get_bool (prop);
        xmlFree (prop);
    }

    for (n = node->children; n != NULL; n = n->next)
    {
        if (n->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp (n->name, (const xmlChar*)"start-regex"))
                start_regex = xmlNodeGetContent (n);
            else if (!xmlStrcmp (n->name, (const xmlChar*)"end-regex"))
                end_regex = xmlNodeGetContent (n);
            else {
                g_critical ("%s: pattern node has wrong name '%s'", G_STRLOC, n->name);
                success = FALSE;
                break;
            }
        }
    }

    if (success && name && start_regex && end_regex) {
        tag = gtk_string_tag_new ((char*)name, (char*)name,
                                  strconvescape((char*)start_regex),
                                  strconvescape((char*)end_regex),
                                  end_at_line_end);
        if (tag) {
            add_tag (lang, GTK_SOURCE_TAG (tag), style);
        }
        else {
            g_critical ("%s: could not create tag", G_STRLOC);
            success = FALSE;
        }
    }
    else {
        if (!name)
            g_critical ("%s: no name attribute", G_STRLOC);
        if (!style)
            g_critical ("%s: no style attribute", G_STRLOC);
        if (!start_regex)
            g_critical ("%s: no start_regex attribute", G_STRLOC);
        if (!end_regex)
            g_critical ("%s: no end_regex attribute", G_STRLOC);
        success = FALSE;
    }

    if (name) xmlFree (name);
    if (style) xmlFree (style);
    if (start_regex) xmlFree (start_regex);
    if (end_regex) xmlFree (end_regex);

    return success;
}


static gboolean process_pattern_item_node   (MooEditLang    *lang,
                                             xmlNode        *node)
{
    gboolean success = TRUE;
    xmlChar *name, *style;
    xmlChar *pattern = NULL;
    xmlNode *n;

    name = xmlGetProp (node, (const xmlChar*)"_name");
    if (!name) name = xmlGetProp (node, (const xmlChar*)"name");
    style = xmlGetProp (node, (const xmlChar*)"_style");
    if (!style) style = xmlGetProp (node, (const xmlChar*)"style");

    for (n = node->children; n; n = n->next) {
        if (n->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp (n->name, (const xmlChar*)"regex")) {
                pattern = xmlNodeGetContent (n);
                break;
            }
            else {
                g_critical ("%s: unexpected element %s", G_STRLOC, n->name);
                break;
            }
        }
    }

    if (name && pattern) {
        GtkTextTag *tag = gtk_pattern_tag_new ((char*)name, (char*)name,
                                               strconvescape((char*)pattern));
        if (tag) {
            add_tag (lang, GTK_SOURCE_TAG (tag), style);
        }
        else {
            g_warning ("%s: could not create tag", G_STRLOC);
            success = FALSE;
        }
    }
    else {
        if (!name)
            g_critical ("%s: no name attribute", G_STRLOC);
        if (!pattern)
            g_critical ("%s: no pattern attribute", G_STRLOC);
        success = FALSE;
    }

    if (name) xmlFree (name);
    if (style) xmlFree (style);
    if (pattern) xmlFree (pattern);

    return success;
}


static gboolean process_keyword_list_node   (MooEditLang    *lang,
                                             xmlNode        *node)
{
    gboolean success = TRUE;
    gboolean case_sensitive = TRUE;
    gboolean match_empty_string_at_beginning = TRUE;
    gboolean match_empty_string_at_end = TRUE;
    xmlChar *prop = NULL;
    xmlChar *name, *style;
    xmlChar *start_regex, *end_regex;
    guint keywords_num = 0;
    GSList *keywords_list = NULL;
    guint keywords_group = 0;
    xmlNode *kw_node;
    GtkTextTag *tag = NULL;

    name = xmlGetProp (node, (const xmlChar*)"_name");
    if (!name) name = xmlGetProp (node, (const xmlChar*)"name");
    if (!name)
    {
        g_critical ("%s: no 'name' atribute in file '%s'",
                    G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
        success = FALSE;
    }

    style = xmlGetProp (node, (const xmlChar*)"_style");
    if (!style) style = xmlGetProp (node, (const xmlChar*)"style");
    if (!style)
    {
        g_critical ("%s: no 'style' atribute in file '%s'",
                    G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
        success = FALSE;
    }

    prop = xmlGetProp (node, (const xmlChar*)"case-sensitive");
    if (!prop)
    {
        /* not a warning because it happens in gtksourceview files */
        g_message ("%s: no 'case-sensitive' atribute in file '%s'",
                   G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
    }
    else
    {
        case_sensitive = get_bool (prop);
        xmlFree (prop);
    }

    prop = xmlGetProp (node, (const xmlChar*)"match-empty-string-at-beginning");
    if (prop)
    {
        match_empty_string_at_beginning = get_bool (prop);
        xmlFree (prop);
    }

    prop = xmlGetProp (node, (const xmlChar*)"match-empty-string-at-end");
    if (prop) {
        match_empty_string_at_end = get_bool (prop);
        xmlFree (prop);
    }

    start_regex = xmlGetProp (node, (const xmlChar*)"beginning-regex");
    end_regex = xmlGetProp (node, (const xmlChar*)"end-regex");

    for (kw_node = node->children; success && kw_node != NULL; kw_node = kw_node->next)
    {
        if (kw_node->type == XML_ELEMENT_NODE) {
            xmlChar *keyword;

            if (xmlStrcmp (kw_node->name, (const xmlChar*)"keyword")) {
                g_critical ("%s: unexpected node '%s' in file '%s'",
                            G_STRLOC, kw_node->name,
                            lang->priv->filename ? lang->priv->filename : "???");
                success = FALSE;
                free_xmlstring_list (keywords_list);
                break;
            }

            keyword = xmlNodeGetContent (kw_node);
            if (!keyword) {
                g_critical ("%s: empty node in file '%s'",
                            G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
                success = FALSE;
                free_xmlstring_list (keywords_list);
                break;
            }

            keywords_list = g_slist_prepend (keywords_list,
                                             strconvescape((char*)keyword));
            ++keywords_num;

            if (keywords_num == KEYWORDS_NUM_LIMIT - 1)
            {
                char *group_name = g_strdup_printf ("%s%d", name, keywords_group);
                keywords_list = g_slist_reverse (keywords_list);
                tag = gtk_keyword_list_tag_new (
                        (char*) name,
                        group_name,
                        keywords_list,
                        case_sensitive,
                        match_empty_string_at_beginning,
                        match_empty_string_at_end,
                        strconvescape((char*)start_regex),
                        strconvescape((char*)end_regex));
                g_free (group_name);

                if (tag) {
                    add_tag (lang, GTK_SOURCE_TAG (tag), style);
                }
                else {
                    g_critical ("%s: could not create tag in file '%s'",
                                G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
                    success = FALSE;
                }

                free_xmlstring_list (keywords_list);
                keywords_list = NULL;
                keywords_num = 0;
                ++keywords_group;
            }
        }
    }

    if (keywords_list) {
        char *group_name = g_strdup_printf ("%s%d", name, keywords_group);
        keywords_list = g_slist_reverse (keywords_list);
        tag = gtk_keyword_list_tag_new (
                (char*) name,
                group_name,
                keywords_list,
                case_sensitive,
                match_empty_string_at_beginning,
                match_empty_string_at_end,
                strconvescape((char*)start_regex),
                strconvescape((char*)end_regex));
        g_free (group_name);

        if (tag)
        {
            add_tag (lang, GTK_SOURCE_TAG (tag), style);
        }
        else
        {
            g_critical ("%s: could not create tag in file '%s'",
                        G_STRLOC, lang->priv->filename ? lang->priv->filename : "???");
            success = FALSE;
        }

        free_xmlstring_list (keywords_list);
        keywords_list = NULL;
        keywords_num = 0;
        ++keywords_group;
    }

    if (name) xmlFree (name);
    if (style) xmlFree (style);
    if (start_regex) xmlFree (start_regex);
    if (end_regex) xmlFree (end_regex);

    return success;
}


#define utf8_next_char(p) (const char *)((p) + g_utf8_skip[*(const guchar *)(p)])

static gboolean parse_brackets              (const char     *string,
                                             gunichar      **left_brackets,
                                             gunichar      **right_brackets,
                                             guint          *num)
{
    long len, i;
    const char *p;
    gunichar *left, *right;

    g_return_val_if_fail (g_utf8_validate (string, -1, NULL), FALSE);
    len = g_utf8_strlen (string, -1);
    g_return_val_if_fail (len > 0 && (len / 2) * 2 == len, FALSE);

    len /= 2;
    p = string;
    left = g_new (gunichar, len);
    right = g_new (gunichar, len);

    for (i = 0; i < len; ++i) {
        left[i] = g_utf8_get_char (p);
        p = utf8_next_char (p);
        right[i] = g_utf8_get_char (p);
        p = utf8_next_char (p);
    }

    if (left_brackets)
        *left_brackets = left;
    else
        g_free (left);
    if (right_brackets)
        *right_brackets = right;
    else
        g_free (right);
    if (num)
        *num = len;

    return TRUE;
}


static gboolean parse_word_chars            (const char     *string,
                                             gunichar      **ch,
                                             guint          *num)
{
    long len, i;
    const char *p;
    gunichar *chars;

    g_return_val_if_fail (g_utf8_validate (string, -1, NULL), FALSE);
    len = g_utf8_strlen (string, -1);
    g_return_val_if_fail (len > 0, FALSE);

    p = string;
    chars = g_new (gunichar, len);

    for (i = 0; i < len; ++i) {
        chars[i] = g_utf8_get_char (p);
        p = utf8_next_char (p);
    }

    *ch = chars;
    *num = len;

    return TRUE;
}


/****************************************************************************/
/* Aux functions                                                            */
/***/

static void free_string_list (GSList *list)
{
    if (!list) return;
    g_slist_foreach (list, (GFunc) g_free, NULL);
    g_slist_free (list);
}

static void free_xmlstring_list (GSList *list)
{
    if (!list) return;
    g_slist_foreach (list, (GFunc) xmlFree, NULL);
    g_slist_free (list);
}


static GSList *split_mime_types (const char *str)
{
    GSList *result;
    char **mime_types, **s;

    if (!str) return NULL;

    mime_types = g_strsplit (str, MIME_TYPES_DELIMITER, 0);
    if (!mime_types) return NULL;

    result = NULL;
    for (s = mime_types; *s; ++s)
        result = g_slist_prepend (result, g_strdup (*s));
    g_strfreev (mime_types);

    result = g_slist_reverse (result);
    return result;
}


static GSList *split_extensions (const char *str)
{
    return split_mime_types (str);
}


static Style                *style_new          (void)
{
    return g_new0 (Style, 1);
}


static void                  style_free         (Style      *style)
{
    if (!style) return;
    g_free (style->name);
    g_free (style->default_style);
    g_free (style->foreground);
    g_free (style->background);
    g_free (style);
}


static gboolean              get_bool           (const char *str)
{
    g_return_val_if_fail (str != NULL, FALSE);

    if (!g_ascii_strcasecmp (str, "TRUE") ||
        !g_ascii_strcasecmp (str, "yes") ||
        !g_ascii_strcasecmp (str, "1"))
            return TRUE;
    else
        return FALSE;
}
