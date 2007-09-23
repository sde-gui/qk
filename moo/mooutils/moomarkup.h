/*
 *   moomarkup.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_MARKUP_H
#define MOO_MARKUP_H

#include <glib/gerror.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOO_TYPE_MARKUP_DOC (moo_markup_doc_get_type())

typedef enum {
    MOO_MARKUP_DOC_NODE,
    MOO_MARKUP_ELEMENT_NODE,
    MOO_MARKUP_TEXT_NODE,
    MOO_MARKUP_COMMENT_NODE
} MooMarkupNodeType;


typedef struct _MooMarkupNode MooMarkupNode;
typedef struct _MooMarkupDoc MooMarkupDoc;
typedef struct _MooMarkupElement MooMarkupElement;
typedef struct _MooMarkupText MooMarkupText;
typedef struct _MooMarkupText MooMarkupComment;


#if 0 && defined(ENABLE_DEBUG)
#define MOO_MARKUP_NODE(n)      (MOO_MARKUP_NODE_CHECK_CAST(n))
#define MOO_MARKUP_DOC(n)       (MOO_MARKUP_DOC_CHECK_CAST(n))
#define MOO_MARKUP_ELEMENT(n)   (MOO_MARKUP_ELEMENT_CHECK_CAST(n))
#define MOO_MARKUP_TEXT(n)      (MOO_MARKUP_TEXT_CHECK_CAST(n))
#define MOO_MARKUP_COMMENT(n)   (MOO_MARKUP_COMMENT_CHECK_CAST(n))
#else /* ENABLE_DEBUG */
#define MOO_MARKUP_NODE(n)      ((MooMarkupNode*)n)
#define MOO_MARKUP_DOC(n)       ((MooMarkupDoc*)(n))
#define MOO_MARKUP_ELEMENT(n)   ((MooMarkupElement*)(n))
#define MOO_MARKUP_TEXT(n)      ((MooMarkupText*)(n))
#define MOO_MARKUP_COMMENT(n)   ((MooMarkupComment*)(n))
#endif /* ENABLE_DEBUG */

#define MOO_MARKUP_IS_DOC(n)        ((n) != NULL && MOO_MARKUP_NODE(n)->type == MOO_MARKUP_DOC_NODE)
#define MOO_MARKUP_IS_ELEMENT(n)    ((n) != NULL && MOO_MARKUP_NODE(n)->type == MOO_MARKUP_ELEMENT_NODE)
#define MOO_MARKUP_IS_TEXT(n)       ((n) != NULL && MOO_MARKUP_NODE(n)->type == MOO_MARKUP_TEXT_NODE)
#define MOO_MARKUP_IS_COMMENT(n)    ((n) != NULL && MOO_MARKUP_NODE(n)->type == MOO_MARKUP_COMMENT_NODE)


struct _MooMarkupNode {
    MooMarkupNodeType        type;      /* type of the node */
    char                    *name;      /* the name of the node */
    MooMarkupNode           *children;  /* parent->childs link */
    MooMarkupNode           *last;      /* last child link */
    MooMarkupNode           *parent;    /* child->parent link */
    MooMarkupNode           *next;      /* next sibling link  */
    MooMarkupNode           *prev;      /* previous sibling link  */
    MooMarkupDoc            *doc;       /* the containing document */
};


struct _MooMarkupDoc {
    MooMarkupNodeType        type;      /* MOO_MARKUP_DOC_NODE */
    char                    *name;      /* name/filename/URI of the document */
    MooMarkupNode           *children;  /* the document tree */
    MooMarkupNode           *last;      /* last child link */
    MooMarkupNode           *parent;    /* child->parent link */
    MooMarkupNode           *next;      /* NULL */
    MooMarkupNode           *prev;      /* NULL */
    MooMarkupDoc            *doc;       /* self */

    guint                    ref_count;
    guint                    modified : 1;
    guint                    track_modified : 1;
};


struct _MooMarkupElement {
    MooMarkupNodeType        type;      /* MOO_MARKUP_ELEMENT_NODE */
    char                    *name;      /* name */
    MooMarkupNode           *children;  /* content */
    MooMarkupNode           *last;      /* last child link */
    MooMarkupNode           *parent;    /* child->parent link */
    MooMarkupNode           *next;      /* next sibling */
    MooMarkupNode           *prev;      /* previous sibling */
    MooMarkupDoc            *doc;       /* containing document */

    char                    *content;   /* text content of the node */
    char                   **attr_names;/* NULL-terminated list of attribute names */
    char                   **attr_vals; /* NULL-terminated list of attribute values */
    guint                    n_attrs;
};


struct _MooMarkupText {
    MooMarkupNodeType        type;      /* MOO_MARKUP_TEXT_NODE */
    char                    *name;      /* "TEXT" */
    MooMarkupNode           *children;  /* NULL */
    MooMarkupNode           *last;      /* NULL */
    MooMarkupNode           *parent;    /* child->parent link */
    MooMarkupNode           *next;      /* next sibling link  */
    MooMarkupNode           *prev;      /* previous sibling link  */
    MooMarkupDoc            *doc;       /* the containing document */

    char                    *text;      /* 0-terminated content of the node */
    guint                    size;      /* strlen (text) */
};


GType               moo_markup_doc_get_type         (void);

MooMarkupNode      *MOO_MARKUP_NODE_CHECK_CAST     (gpointer node);
MooMarkupDoc       *MOO_MARKUP_DOC_CHECK_CAST      (gpointer node);
MooMarkupElement   *MOO_MARKUP_ELEMENT_CHECK_CAST  (gpointer node);
MooMarkupText      *MOO_MARKUP_TEXT_CHECK_CAST     (gpointer node);
MooMarkupComment   *MOO_MARKUP_COMMENT_CHECK_CAST  (gpointer node);

MooMarkupDoc       *moo_markup_doc_new              (const char         *name);

MooMarkupDoc       *moo_markup_parse_file           (const char         *filename,
                                                     GError            **error);
MooMarkupDoc       *moo_markup_parse_memory         (const char         *buffer,
                                                     int                 size,
                                                     GError            **error);
gboolean            moo_markup_save_pretty          (MooMarkupDoc       *doc,
                                                     const char         *filename,
                                                     guint               indent,
                                                     GError            **error);
char               *moo_markup_format_pretty        (MooMarkupDoc       *doc,
                                                     guint               indent);
char               *moo_markup_node_get_string      (MooMarkupNode      *node);

MooMarkupDoc       *moo_markup_doc_ref              (MooMarkupDoc       *doc);
void                moo_markup_doc_unref            (MooMarkupDoc       *doc);

MooMarkupNode      *moo_markup_get_root_element     (MooMarkupDoc       *doc,
                                                     const char         *name);

MooMarkupNode      *moo_markup_get_element          (MooMarkupNode      *node,
                                                     const char         *path);

const char         *moo_markup_get_prop             (MooMarkupNode      *node,
                                                     const char         *prop_name);
void                moo_markup_set_prop             (MooMarkupNode      *node,
                                                     const char         *prop_name,
                                                     const char         *val);

int                 moo_markup_get_int_prop         (MooMarkupNode      *node,
                                                     const char         *prop_name,
                                                     int                 default_val);
void                moo_markup_set_int_prop         (MooMarkupNode      *node,
                                                     const char         *prop_name,
                                                     int                 val);
gboolean            moo_markup_get_bool_prop        (MooMarkupNode      *node,
                                                     const char         *prop_name,
                                                     gboolean            default_val);
void                moo_markup_set_bool_prop        (MooMarkupNode      *node,
                                                     const char         *prop_name,
                                                     gboolean            val);


void                moo_markup_set_content          (MooMarkupNode      *node,
                                                     const char         *text);
const char         *moo_markup_get_content          (MooMarkupNode      *node);

char               *moo_markup_element_get_path     (MooMarkupElement   *node);

void                moo_markup_delete_node          (MooMarkupNode      *node);

MooMarkupNode      *moo_markup_create_root_element  (MooMarkupDoc       *doc,
                                                     const char         *name);
MooMarkupNode      *moo_markup_create_element       (MooMarkupNode      *parent,
                                                     const char         *path);

MooMarkupNode      *moo_markup_create_text_element  (MooMarkupNode      *parent,
                                                     const char         *path,
                                                     const char         *content);

void                _moo_markup_set_modified        (MooMarkupDoc       *doc,
                                                     gboolean            modified);
void                _moo_markup_set_track_modified  (MooMarkupDoc       *doc,
                                                     gboolean            track);
gboolean            _moo_markup_get_modified        (MooMarkupDoc       *doc);


G_END_DECLS

#endif /* MOO_MARKUP_H */
