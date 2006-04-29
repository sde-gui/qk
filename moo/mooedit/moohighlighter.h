/*
 *   moohighlighter.h
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

#ifndef __MOO_HIGHLIGHTER_H__
#define __MOO_HIGHLIGHTER_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include "mooedit/moolang.h"
#include "mooedit/moolinebuffer.h"

G_BEGIN_DECLS


#define MOO_TYPE_SYNTAX_TAG              (_moo_syntax_tag_get_type ())
#define MOO_SYNTAX_TAG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_SYNTAX_TAG, MooSyntaxTag))
#define MOO_SYNTAX_TAG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_SYNTAX_TAG, MooSyntaxTagClass))
#define MOO_IS_SYNTAX_TAG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_SYNTAX_TAG))
#define MOO_IS_SYNTAX_TAG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_SYNTAX_TAG))
#define MOO_SYNTAX_TAG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_SYNTAX_TAG, MooSyntaxTagClass))

typedef struct _MooHighlighter    MooHighlighter;
typedef struct _MooSyntaxTag      MooSyntaxTag;
typedef struct _MooSyntaxTagClass MooSyntaxTagClass;

struct _CtxNode
{
    guint depth;
    MooContext *ctx;
    CtxNode *parent;
    GHashTable *children; /* rule -> CtxNode* */
    CtxNode *line_end;
    GtkTextTag *context_tag;
    GHashTable *match_tags; /* rule -> GtkTextTag* */
    /* Match tags that may be contained in this context.
       They are not necessarily in match_tags list due to
       include-into-next rule property */
    GSList *child_tags;
};

struct _MooHighlighter {
    GtkTextBuffer *buffer;
    LineBuffer *line_buf;
    MooLang *lang;
    CtxNode *root;
    GSList *nodes;
    guint idle;
};

struct _MooSyntaxTag {
    GtkTextTag parent;
    CtxNode *ctx_node;      /* self == ctx_node->context_tag if rule == NULL, */
    CtxNode *match_node;    /* and match_node->match_tags[rule->id] otherwise */
    MooRule *rule;
};

struct _MooSyntaxTagClass {
    GtkTextTagClass parent_class;
};


GType   _moo_syntax_tag_get_type            (void) G_GNUC_CONST;


MooHighlighter *_moo_highlighter_new        (GtkTextBuffer      *buffer,
                                             LineBuffer         *line_buf,
                                             MooLang            *lang);
void    _moo_highlighter_destroy            (MooHighlighter     *highlight,
                                             gboolean            destroy_tags);

void    _moo_highlighter_queue_compute      (MooHighlighter     *highlight);
void    _moo_highlighter_apply_tags         (MooHighlighter     *highlight,
                                             int                 first_line,
                                             int                 last_line);

void    _moo_highlighter_apply_scheme       (MooHighlighter     *highlight,
                                             MooTextStyleScheme *scheme);


G_END_DECLS

#endif /* __MOO_HIGHLIGHTER_H__ */
