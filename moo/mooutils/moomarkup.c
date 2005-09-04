/*
 *   mooutils/moomarkup.c
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

#include "mooutils/moomarkup.h"
#include "mooutils/moofileutils.h"
#include <string.h>
#include <glib.h>


GType moo_markup_doc_get_type (void)
{
    static GType type = 0;
    if (type == 0)
        type = g_boxed_type_register_static ("MooMarkupDoc",
                                             (GBoxedCopyFunc)moo_markup_doc_ref,
                                             (GBoxedFreeFunc)moo_markup_doc_unref);
    return type;
}


typedef void (*markup_start_element_func)   (GMarkupParseContext    *context,
                                             const gchar            *element_name,
                                             const gchar           **attribute_names,
                                             const gchar           **attribute_values,
                                             gpointer                user_data,
                                             GError                **error);
typedef void (*markup_end_element_func)     (GMarkupParseContext    *context,
                                             const gchar            *element_name,
                                             gpointer                user_data,
                                             GError                **error);
typedef void (*markup_text_func)            (GMarkupParseContext    *context,
                                             const gchar            *text,
                                             gsize                   text_len,
                                             gpointer                user_data,
                                             GError                **error);
typedef void (*markup_passthrough_func)     (GMarkupParseContext    *context,
                                             const gchar            *passthrough_text,
                                             gsize                   text_len,
                                             gpointer                user_data,
                                             GError                **error);


typedef struct {
    MooMarkupDoc   *doc;
    MooMarkupNode  *current;
} ParserState;


#define BUFSIZE 1024


static MooMarkupDoc     *moo_markup_doc_new_priv    (const char         *name);
static MooMarkupNode    *moo_markup_element_new     (MooMarkupDoc       *doc,
                                                     MooMarkupNode      *parent,
                                                     const char         *name,
                                                     const char        **attribute_names,
                                                     const char        **attribute_values);
static MooMarkupNode    *moo_markup_text_node_new   (MooMarkupNodeType   type,
                                                     MooMarkupDoc       *doc,
                                                     MooMarkupNode      *parent,
                                                     const char         *text,
                                                     gsize               text_len);
static void              add_node                   (MooMarkupDoc       *doc,
                                                     MooMarkupNode      *parent,
                                                     MooMarkupNode      *node);


static void moo_markup_doc_unref_private        (MooMarkupDoc       *doc);
static void moo_markup_element_free             (MooMarkupElement   *node);
static void moo_markup_text_node_free           (MooMarkupNode      *node);
static void moo_markup_node_free                (MooMarkupNode      *node);

static void collect_text_content                (MooMarkupElement   *node);
static void moo_markup_text_node_add_text       (MooMarkupText      *node,
                                                 const char         *text,
                                                 gsize               text_len);

static void moo_markup_element_print            (MooMarkupElement   *node,
                                                 GString            *dest);
static void moo_markup_text_node_print          (MooMarkupNode      *node,
                                                 GString            *dest);


static void start_element   (GMarkupParseContext    *context,
                             const gchar            *element_name,
                             const gchar           **attribute_names,
                             const gchar           **attribute_values,
                             ParserState            *state,
                             GError **error);
static void end_element     (GMarkupParseContext    *context,
                             const gchar            *element_name,
                             ParserState            *state,
                             GError                **error);
static void text            (GMarkupParseContext    *context,
                             const gchar            *text,
                             gsize                   text_len,
                             ParserState            *state,
                             GError                **error);
static void passthrough     (GMarkupParseContext    *context,
                             const gchar            *passthrough_text,
                             gsize                   text_len,
                             ParserState            *state,
                             GError                **error);


MooMarkupDoc   *moo_markup_parse_memory    (const char     *buffer,
                                            int             size,
                                            GError        **error)
{
    GError *err = NULL;

    GMarkupParser parser = {(markup_start_element_func)start_element,
                            (markup_end_element_func)end_element,
                            (markup_text_func)text,
                            (markup_passthrough_func)passthrough,
                            NULL};

    MooMarkupDoc *doc;
    ParserState state;
    GMarkupParseContext *context;

    if (size < 0) size = strlen (buffer);

    doc = moo_markup_doc_new_priv (NULL);
    state.doc = doc;
    state.current = MOO_MARKUP_NODE (doc);
    context = g_markup_parse_context_new (&parser, (GMarkupParseFlags)0, &state, NULL);

    if (!g_markup_parse_context_parse (context, buffer, size, &err) ||
        !g_markup_parse_context_end_parse (context, &err))
    {
        if (err)
        {
            if (error) *error = err;
            else g_error_free (err);
        }
        g_markup_parse_context_free (context);
        moo_markup_doc_unref (doc);
        return NULL;
    }

    g_markup_parse_context_free (context);
    return doc;
}


MooMarkupDoc   *moo_markup_parse_file      (const char     *filename,
                                            GError        **error)
{
    char *content;
    MooMarkupDoc *doc;

    g_return_val_if_fail (filename != NULL, NULL);

    if (!g_file_get_contents (filename, &content, NULL, error))
        return NULL;

    doc = moo_markup_parse_memory (content, -1, error);

    g_free (content);
    return doc;
}


static void start_element   (G_GNUC_UNUSED GMarkupParseContext    *ctx,
                             const gchar            *element_name,
                             const gchar           **attribute_names,
                             const gchar           **attribute_values,
                             ParserState            *state,
                             G_GNUC_UNUSED GError                **error)
{
    MooMarkupNode *elm = moo_markup_element_new (state->doc,
                                                 state->current,
                                                 element_name,
                                                 attribute_names,
                                                 attribute_values);
    g_assert (elm->parent == state->current);
    state->current = elm;
}


static void end_element     (G_GNUC_UNUSED GMarkupParseContext    *ctx,
                             G_GNUC_UNUSED const gchar            *elm,
                             ParserState            *state,
                             G_GNUC_UNUSED GError                **error)
{
    g_assert (state->current->type == MOO_MARKUP_ELEMENT_NODE);
    collect_text_content (MOO_MARKUP_ELEMENT (state->current));
    g_assert (state->current->parent != NULL);
    state->current = state->current->parent;
}


static void text            (G_GNUC_UNUSED GMarkupParseContext    *ctx,
                             const gchar            *text,
                             gsize                   text_len,
                             ParserState            *state,
                             G_GNUC_UNUSED GError                **error)
{
    if (MOO_MARKUP_IS_TEXT (state->current->last))
        moo_markup_text_node_add_text (MOO_MARKUP_TEXT (state->current->last),
                                       text, text_len);
    else
        moo_markup_text_node_new (MOO_MARKUP_TEXT_NODE,
                                  state->doc, state->current,
                                  text, text_len);
}


static void passthrough     (G_GNUC_UNUSED GMarkupParseContext    *ctx,
                             const gchar            *passthrough_text,
                             gsize                   text_len,
                             ParserState            *state,
                             G_GNUC_UNUSED GError                **error)
{
    if (MOO_MARKUP_IS_COMMENT (state->current->last))
        moo_markup_text_node_add_text (MOO_MARKUP_COMMENT (state->current->last),
                                       passthrough_text, text_len);
    else
        moo_markup_text_node_new (MOO_MARKUP_COMMENT_NODE,
                                  state->doc, state->current,
                                  passthrough_text, text_len);
}


static MooMarkupDoc        *moo_markup_doc_new_priv      (const char    *name)
{
    MooMarkupDoc *doc = g_new0 (MooMarkupDoc, 1);

    doc->type = MOO_MARKUP_DOC_NODE;
    doc->name = g_strdup (name);
    doc->doc = doc;
    doc->ref_count = 1;

    return doc;
}


MooMarkupDoc    *moo_markup_doc_new         (const char *name)
{
    return moo_markup_doc_new_priv (name);
}


static MooMarkupNode       *moo_markup_element_new  (MooMarkupDoc   *doc,
                                                     MooMarkupNode  *parent,
                                                     const char     *name,
                                                     const char    **attribute_names,
                                                     const char    **attribute_values)
{
    MooMarkupElement *elm = g_new0 (MooMarkupElement, 1);
    add_node (doc, parent, MOO_MARKUP_NODE (elm));

    elm->type = MOO_MARKUP_ELEMENT_NODE;
    elm->name = g_strdup (name);

    elm->attr_names = g_strdupv ((char**)attribute_names);
    elm->attr_vals = g_strdupv ((char**)attribute_values);
    if (elm->attr_names)
        for (elm->n_attrs = 0; attribute_names[elm->n_attrs]; ++elm->n_attrs) ;
    else
        elm->n_attrs = 0;

    return MOO_MARKUP_NODE (elm);
}


static MooMarkupNode       *moo_markup_text_node_new(MooMarkupNodeType   type,
                                                     MooMarkupDoc       *doc,
                                                     MooMarkupNode      *parent,
                                                     const char         *text,
                                                     gsize               text_len)
{
    MooMarkupText *node;

    g_assert (type == MOO_MARKUP_TEXT_NODE || type == MOO_MARKUP_COMMENT_NODE);
    node = g_new0 (MooMarkupText, 1);
    add_node (doc, parent, MOO_MARKUP_NODE (node));

    node->type = type;
    if (type == MOO_MARKUP_TEXT_NODE)
        node->name = g_strdup ("TEXT");
    else
        node->name = g_strdup ("COMMENT");

    node->text = g_strndup (text, text_len);
    node->size = text_len;

    return MOO_MARKUP_NODE (node);
}


static void              add_node            (MooMarkupDoc     *doc,
                                              MooMarkupNode    *parent,
                                              MooMarkupNode    *node)
{
    node->doc = doc;
    node->parent = parent;
    if (parent->last)
    {
        parent->last->next = node;
        node->prev = parent->last;
        parent->last = node;
    }
    else
    {
        g_assert (parent->children == NULL);
        g_assert (parent->last == NULL);
        parent->children = node;
        parent->last = node;
    }
}


static void moo_markup_text_node_add_text   (MooMarkupText  *node,
                                             const char     *text,
                                             gsize           text_len)
{
    char *tmp = (char*)g_memdup (node->text, node->size + text_len + 1);
    g_memmove (tmp + node->size, text, text_len);
    tmp[node->size + text_len] = 0;
    g_free (node->text);
    node->text = tmp;
    node->size += text_len;
}


static void collect_text_content        (MooMarkupElement  *node)
{
    MooMarkupNode *child;
    GString *text = NULL;

    for (child = node->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_TEXT (child))
        {
            if (text)
            {
                g_string_append_len (text,
                                     MOO_MARKUP_TEXT(child)->text,
                                     MOO_MARKUP_TEXT(child)->size);
            }
            else
            {
                text = g_string_new_len (MOO_MARKUP_TEXT(child)->text,
                                         MOO_MARKUP_TEXT(child)->size);
            }
        }
    }

    g_free (node->content);

    if (text)
        node->content = g_string_free (text, FALSE);
    else
        node->content = NULL;
}


static void moo_markup_node_free    (MooMarkupNode     *node)
{
    MooMarkupNode *child;
    GSList *children = NULL, *l;

    g_return_if_fail (node != NULL);

    for (child = node->children; child != NULL; child = child->next)
        children = g_slist_prepend (children, child);

    for (l = children; l != NULL; l = l->next)
        moo_markup_node_free (l->data);

    g_free (node->name);

    switch (node->type)
    {
        case MOO_MARKUP_DOC_NODE:
            moo_markup_doc_unref_private (MOO_MARKUP_DOC (node));
            break;
        case MOO_MARKUP_ELEMENT_NODE:
            moo_markup_element_free (MOO_MARKUP_ELEMENT (node));
            break;
        case MOO_MARKUP_TEXT_NODE:
        case MOO_MARKUP_COMMENT_NODE:
            moo_markup_text_node_free (node);
            break;
        default:
            g_assert_not_reached ();
    }

    g_slist_free (children);
    g_free (node);
}


static void moo_markup_element_free         (MooMarkupElement  *node)
{
    g_free (node->content);
    g_strfreev (node->attr_names);
    g_strfreev (node->attr_vals);
}


static void moo_markup_text_node_free       (MooMarkupNode     *node)
{
    g_assert (node->type == MOO_MARKUP_TEXT_NODE || node->type == MOO_MARKUP_COMMENT_NODE);
    g_free (((MooMarkupText*)node)->text);
}


void moo_markup_doc_unref (MooMarkupDoc *doc)
{
    g_return_if_fail (MOO_MARKUP_IS_DOC (doc));
    if (--(doc->ref_count)) return;
    moo_markup_node_free (MOO_MARKUP_NODE (doc));
}


MooMarkupDoc *moo_markup_doc_ref (MooMarkupDoc *doc)
{
    g_return_val_if_fail (MOO_MARKUP_IS_DOC (doc), NULL);
    ++(doc->ref_count);
    return doc;
}


static void moo_markup_doc_unref_private     (G_GNUC_UNUSED MooMarkupDoc      *doc)
{
}


char *moo_markup_node_get_string (MooMarkupNode *node)
{
    MooMarkupNode *child;
    GString *str = g_string_new ("");
    for (child = node->children; child != NULL; child = child->next)
        switch (child->type)
        {
            case MOO_MARKUP_ELEMENT_NODE:
                moo_markup_element_print (MOO_MARKUP_ELEMENT (child), str);
                break;
            case MOO_MARKUP_TEXT_NODE:
            case MOO_MARKUP_COMMENT_NODE:
                moo_markup_text_node_print (child, str);
                break;
            default:
                g_assert_not_reached ();
        }

    return g_string_free (str, FALSE);
}


static void moo_markup_element_print        (MooMarkupElement   *node,
                                             GString            *str)
{
    guint i;

    g_string_append_printf (str, "<%s", node->name);
    for (i = 0; i < node->n_attrs; ++i)
        g_string_append_printf (str, " %s=\"%s\"",
                                node->attr_names[i],
                                node->attr_vals[i]);

    if (node->children)
    {
        MooMarkupNode *child;

        g_string_append (str, ">");
        for (child = node->children; child != NULL; child = child->next)
            switch (child->type)
            {
                case MOO_MARKUP_ELEMENT_NODE:
                    moo_markup_element_print (MOO_MARKUP_ELEMENT (child), str);
                    break;
                case MOO_MARKUP_TEXT_NODE:
                case MOO_MARKUP_COMMENT_NODE:
                    moo_markup_text_node_print (child, str);
                    break;
                default:
                    g_assert_not_reached ();
            }
        g_string_append_printf (str, "</%s>", node->name);
    }
    else
        g_string_append (str, "/>");
}


static void moo_markup_text_node_print      (MooMarkupNode  *node,
                                             GString        *str)
{
    char *p, *escaped;
    MooMarkupText *text = (MooMarkupText*) node;

    if (!g_utf8_validate (text->text, -1, (const char**) &p))
    {
        g_critical ("%s: invalid UTF8", G_STRLOC);
        *p = 0;
    }

    escaped = g_markup_escape_text (text->text, -1);
    g_string_append (str, escaped);
    g_free (escaped);
}


static MooMarkupElement *get_element (MooMarkupNode *node,
                                      char **path)
{
    MooMarkupNode *child;

    if (!path || !path[0] || !path[0][0]) return MOO_MARKUP_ELEMENT (node);

    /* TODO: fix this */
    for (child = node->children; child; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child)) {
            const char *name = moo_markup_get_prop (MOO_MARKUP_ELEMENT (child),
                                                    "name");
            if (name && !strcmp (path[0], name))
                return get_element (child, ++path);
        }
    }

    return NULL;
}


MooMarkupElement *moo_markup_get_element     (MooMarkupNode  *node,
                                              const char     *path)
{
    char **p;
    MooMarkupElement *elm;

    g_return_val_if_fail (path != NULL, NULL);

    p = g_strsplit (path, "/", 0);
    elm = get_element (node, p);
    g_strfreev (p);
    return elm;
}


const char      *moo_markup_get_prop        (MooMarkupElement   *node,
                                             const char         *prop_name)
{
    guint i;
    g_return_val_if_fail (node != NULL && prop_name != NULL, NULL);
    for (i = 0 ; i < node->n_attrs; ++i)
        if (!strcmp (prop_name, node->attr_names[i]))
            return node->attr_vals[i];
    return NULL;
}


MooMarkupElement*moo_markup_get_root_element(MooMarkupDoc       *doc,
                                             const char         *name)
{
    MooMarkupNode *child;
    for (child = doc->children; child; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
        {
            if (!name || !strcmp (child->name, name))
                return MOO_MARKUP_ELEMENT (child);
        }
    }

    return NULL;
}


char            *moo_markup_element_get_path(MooMarkupElement   *elm)
{
    GString *path;
    MooMarkupNode *node;

    g_return_val_if_fail (elm != NULL, NULL);

    path = g_string_new (elm->name);

    for (node = elm->parent;
         node != NULL && MOO_MARKUP_IS_ELEMENT (node);
         node = node->parent)
    {
        g_string_prepend (path, "/");
        g_string_prepend (path, node->name);
    }

    return g_string_free (path, FALSE);
}


void             moo_markup_delete_node     (MooMarkupNode      *node)
{
    MooMarkupNode *parent, *child, *next, *prev;

    g_return_if_fail (node != NULL);
    g_return_if_fail (node->parent != NULL);

    parent = node->parent;

    for (child = parent->children; child != NULL && child != node;
         child = child->next) ;

    g_return_if_fail (child == node);

    next = node->next;
    prev = node->prev;

    if (parent->children == node)
    {
        g_assert (node->prev == NULL);
        parent->children = next;
    }

    if (parent->last == node)
    {
        g_assert (node->next == NULL);
        parent->last = prev;
    }

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    moo_markup_node_free (node);
}


MooMarkupElement*moo_markup_create_root_element
                                            (MooMarkupDoc       *doc,
                                             const char         *name)
{
    MooMarkupElement *elm;

    g_return_val_if_fail (MOO_MARKUP_IS_DOC (doc), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    elm = moo_markup_get_root_element (doc, name);
    if (elm) return elm;

    elm = MOO_MARKUP_ELEMENT (moo_markup_element_new (doc,
                              MOO_MARKUP_NODE (doc), name, NULL, NULL));

    return elm;
}


static void create_text_element (MooMarkupElement   *node,
                                 char              **path,
                                 const char         *content)
{
    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (node));
    g_return_if_fail (path != NULL);

    if (path[0])
    {
        MooMarkupNode *child = NULL;

        for (child = node->children; child != NULL; child = child->next)
            if (MOO_MARKUP_IS_ELEMENT (child) && child->name &&
                !strcmp (child->name, path[0]))
                    break;

        if (!child)
            child = moo_markup_element_new (node->doc, MOO_MARKUP_NODE (node),
                                            path[0], NULL, NULL);

        create_text_element (MOO_MARKUP_ELEMENT (child), ++path, content);
    }
    else
    {
        MooMarkupNode *child = NULL;

        for (child = node->children; child != NULL; child = child->next)
            moo_markup_node_free (child);

        node->children = NULL;
        node->last = NULL;
        g_free (node->content);
        node->content = NULL;
        g_strfreev (node->attr_names);
        node->attr_names = NULL;
        g_strfreev (node->attr_vals);
        node->attr_vals = NULL;
        node->n_attrs = 0;

        child = moo_markup_text_node_new (MOO_MARKUP_TEXT_NODE,
                                          node->doc, MOO_MARKUP_NODE (node),
                                          content, strlen (content));
        node->content = g_strdup (content);
    }
}


void             moo_markup_create_text_element
                                            (MooMarkupElement   *node,
                                             const char         *path,
                                             const char         *content)
{
    char **pieces;

    g_return_if_fail (MOO_MARKUP_IS_ELEMENT (node));
    g_return_if_fail (path != NULL);
    g_return_if_fail (content != NULL);

    pieces = g_strsplit (path, "/", 0);
    g_return_if_fail (pieces != NULL);

    create_text_element (node, pieces, content);
    g_strfreev (pieces);
}


gboolean    moo_markup_save     (MooMarkupDoc       *doc,
                                 const char         *filename,
                                 GError            **error)
{
    char *text;
    gboolean result;

    g_return_val_if_fail (MOO_MARKUP_IS_DOC (doc), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    text = moo_markup_node_get_string (MOO_MARKUP_NODE (doc));
    g_return_val_if_fail (text != NULL, FALSE);

    result = moo_save_file_utf8 (filename, text, -1, error);
    g_free (text);
    return result;
}


#ifdef MOO_MARKUP_NODE
#undef MOO_MARKUP_NODE
#endif
#ifdef MOO_MARKUP_DOC
#undef MOO_MARKUP_DOC
#endif
#ifdef MOO_MARKUP_ELEMENT
#undef MOO_MARKUP_ELEMENT
#endif
#ifdef MOO_MARKUP_TEXT
#undef MOO_MARKUP_TEXT
#endif
#ifdef MOO_MARKUP_COMMENT
#undef MOO_MARKUP_COMMENT
#endif


MooMarkupNode      *MOO_MARKUP_NODE    (gpointer n)
{
    MooMarkupNode *node = (MooMarkupNode*) n;
    g_return_val_if_fail (n != NULL, NULL);
    g_return_val_if_fail (node->type == MOO_MARKUP_DOC_NODE ||
                          node->type == MOO_MARKUP_ELEMENT_NODE ||
                          node->type == MOO_MARKUP_TEXT_NODE ||
                          node->type == MOO_MARKUP_COMMENT_NODE,
                          NULL);
    return node;
}

MooMarkupDoc       *MOO_MARKUP_DOC     (gpointer n)
{
    MooMarkupNode *node = (MooMarkupNode*) n;
    g_return_val_if_fail (n != NULL, NULL);
    g_return_val_if_fail (node->type == MOO_MARKUP_DOC_NODE, NULL);
    return (MooMarkupDoc*) n;
}

MooMarkupElement   *MOO_MARKUP_ELEMENT (gpointer n)
{
    MooMarkupNode *node = (MooMarkupNode*) n;
    g_return_val_if_fail (n != NULL, NULL);
    g_return_val_if_fail (node->type == MOO_MARKUP_ELEMENT_NODE, NULL);
    return (MooMarkupElement*) n;
}

MooMarkupText      *MOO_MARKUP_TEXT    (gpointer n)
{
    MooMarkupNode *node = (MooMarkupNode*) n;
    g_return_val_if_fail (n != NULL, NULL);
    g_return_val_if_fail (node->type == MOO_MARKUP_TEXT_NODE, NULL);
    return (MooMarkupText*) n;
}

MooMarkupComment   *MOO_MARKUP_COMMENT (gpointer n)
{
    MooMarkupNode *node = (MooMarkupNode*) n;
    g_return_val_if_fail (n != NULL, NULL);
    g_return_val_if_fail (node->type == MOO_MARKUP_COMMENT_NODE, NULL);
    return (MooMarkupComment*) n;
}
