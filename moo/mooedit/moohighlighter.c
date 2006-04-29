/*
 *   moohighlighter.c
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
#include "mooedit/moohighlighter.h"
#include "mooedit/moolang-rules.h"
#include "mooedit/gtksourceiter.h"
#include "mooedit/mootext-private.h"
#include <string.h>


#define IDLE_HIGHLIGHT_PRIORITY GTK_TEXT_VIEW_PRIORITY_VALIDATE
#define IDLE_HIGHLIGHT_TIME 30
#define COMPUTE_NOW_TIME 20


static MooSyntaxTag *iter_get_syntax_tag    (const GtkTextIter *iter);

static GtkTextTag   *moo_syntax_tag_new     (CtxNode            *ctx_node,
                                             CtxNode            *match_node,
                                             MooRule            *rule);
static GtkTextTag   *create_tag             (MooHighlighter     *hl,
                                             CtxNode            *ctx_node,
                                             CtxNode            *match_node,
                                             MooRule            *rule);
static GtkTextTag   *get_syntax_tag         (MooHighlighter     *hl,
                                             CtxNode            *ctx_node,
                                             CtxNode            *match_node,
                                             MooRule            *rule);

static CtxNode      *ctx_node_new           (MooHighlighter     *hl,
                                             CtxNode            *parent,
                                             MooContext         *context,
                                             gboolean            create_tag);
static void          ctx_node_free          (CtxNode            *node,
                                             GtkTextTagTable    *table);
static CtxNode      *get_next_node          (MooHighlighter     *hl,
                                             CtxNode            *node,
                                             MooRule            *rule);
static CtxNode      *get_line_end_node      (MooHighlighter     *hl,
                                             CtxNode            *node);

static CtxNode      *hl_compute_line        (MooHighlighter     *hl,
                                             Line               *line,
                                             MatchData          *data,
                                             CtxNode            *node);
static gboolean      hl_compute_range       (MooHighlighter     *hl,
                                             Interval           *lines,
                                             int                 time);


/* MOO_TYPE_SYNTAX_TAG */
G_DEFINE_TYPE (MooSyntaxTag, _moo_syntax_tag, GTK_TYPE_TEXT_TAG)


// static void
// moo_syntax_tag_finalize (GObject *object)
// {
//     g_free (MOO_SYNTAX_TAG(object)->stack);
//     G_OBJECT_CLASS(moo_syntax_tag_parent_class)->finalize (object);
// }


static void
_moo_syntax_tag_class_init (G_GNUC_UNUSED MooSyntaxTagClass *klass)
{
//     G_OBJECT_CLASS(klass)->finalize = moo_syntax_tag_finalize;
}


static void
_moo_syntax_tag_init (MooSyntaxTag *tag)
{
    tag->ctx_node = NULL;
    tag->match_node = NULL;
    tag->rule = NULL;
}


static void
remove_tag (G_GNUC_UNUSED gpointer rule,
            GtkTextTag      *tag,
            GtkTextTagTable *table)
{
    gtk_text_tag_table_remove (table, tag);
}

static void
ctx_node_free (CtxNode          *node,
               GtkTextTagTable  *table)
{
    if (table)
        g_hash_table_foreach (node->match_tags, (GHFunc) remove_tag, table);

    g_hash_table_destroy (node->match_tags);
    g_hash_table_destroy (node->children);
    g_slist_free (node->child_tags);

    if (node->context_tag && table)
        gtk_text_tag_table_remove (table, node->context_tag);

    g_free (node);
}


void
_moo_highlighter_destroy (MooHighlighter *hl,
                          gboolean        delete_tags)
{
    GtkTextTagTable *table = NULL;

    g_return_if_fail (hl != NULL);

    if (delete_tags)
        table = gtk_text_buffer_get_tag_table (hl->buffer);

    g_slist_foreach (hl->nodes, (GFunc) ctx_node_free, table);

    if (hl->lang)
        moo_lang_unref (hl->lang);
    if (hl->idle)
        g_source_remove (hl->idle);

    g_free (hl);
}


GtkTextTag *
_moo_text_iter_get_syntax_tag (const GtkTextIter *iter)
{
    return (GtkTextTag*) iter_get_syntax_tag (iter);
}


static MooSyntaxTag*
iter_get_syntax_tag (const GtkTextIter *iter)
{
    MooSyntaxTag *tag = NULL;
    GSList *l;
    GSList *list = gtk_text_iter_get_tags (iter);

    for (l = list; l != NULL; l = l->next)
    {
        if (MOO_IS_SYNTAX_TAG (l->data))
        {
#ifndef MOO_DEBUG
            tag = l->data;
            break;
#else
            g_assert (tag == NULL);
            tag = l->data;
#endif
        }
    }

    g_slist_free (list);
    return tag;
}


MooContext*
_moo_text_iter_get_context (const GtkTextIter *iter)
{
    MooSyntaxTag *tag = iter_get_syntax_tag (iter);
    return tag ? tag->ctx_node->ctx : NULL;
}


static void
apply_tag (MooHighlighter     *hl,
           Line               *line,
           CtxNode            *ctx_node,
           CtxNode            *match_node,
           MooRule            *rule,
           const GtkTextIter  *start,
           const GtkTextIter  *end)
{
    GtkTextTag *tag = get_syntax_tag (hl, ctx_node, match_node, rule);

    g_assert (!tag || (MOO_IS_SYNTAX_TAG (tag) && MOO_SYNTAX_TAG(tag)->rule == rule));
    g_assert (!tag || MOO_SYNTAX_TAG(tag)->match_node != NULL || MOO_SYNTAX_TAG(tag)->ctx_node != hl->root);

    if (!gtk_text_iter_compare (start, end))
        return;

    g_assert (iter_get_syntax_tag (start) == NULL);

    if (tag)
    {
#if 0
        g_print ("* applying tag '%s' on line %d\n",
                 rule ? rule->description : "NULL",
                 _moo_line_buffer_get_line_index (hl->line_buf, line));
#endif
        line->hl_info->tags = g_slist_prepend (line->hl_info->tags, g_object_ref (tag));
        _moo_text_buffer_apply_syntax_tag (MOO_TEXT_BUFFER (hl->buffer), tag, start, end);
    }
}


MooHighlighter*
_moo_highlighter_new (GtkTextBuffer *buffer,
                      LineBuffer    *line_buf,
                      MooLang       *lang)
{
    MooHighlighter *hl;

    g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

    hl = g_new0 (MooHighlighter, 1);
    hl->lang = lang ? moo_lang_ref (lang) : NULL;
    hl->buffer = buffer;
    hl->line_buf = line_buf;

    return hl;
}


static GtkTextTag*
moo_syntax_tag_new (CtxNode *ctx_node,
                    CtxNode *match_node,
                    MooRule *rule)
{
    GtkTextTag *tag = g_object_new (MOO_TYPE_SYNTAX_TAG, NULL);

    g_assert (ctx_node != NULL);
    g_assert (rule != NULL || match_node == NULL);
    g_assert (rule == NULL || match_node != NULL);

    MOO_SYNTAX_TAG(tag)->ctx_node = ctx_node;
    MOO_SYNTAX_TAG(tag)->match_node = match_node;
    MOO_SYNTAX_TAG(tag)->rule = rule;

    return tag;
}


static GtkTextTag*
create_tag (MooHighlighter *hl,
            CtxNode        *ctx_node,
            CtxNode        *match_node,
            MooRule        *rule)
{
    GtkTextTag *tag;

    if (!ctx_node->parent && !match_node && !rule)
        return NULL;

    tag = moo_syntax_tag_new (ctx_node, match_node, rule);

    gtk_text_tag_table_add (gtk_text_buffer_get_tag_table (hl->buffer), tag);
    g_object_unref (tag);

    _moo_lang_set_tag_style (rule ? rule->context->lang : ctx_node->ctx->lang,
                             tag,
                             rule ? rule->context : ctx_node->ctx,
                             rule, NULL);
    ctx_node->child_tags = g_slist_prepend (ctx_node->child_tags, tag);

    return tag;
}


static GtkTextTag*
get_syntax_tag (MooHighlighter *hl,
                CtxNode        *ctx_node,
                CtxNode        *match_node,
                MooRule        *rule)
{
    GtkTextTag *tag;

    g_assert (ctx_node != NULL);
    g_assert ((!match_node && !rule) || (match_node && rule));

    if (!match_node)
        return ctx_node->context_tag;

    tag = g_hash_table_lookup (match_node->match_tags, rule);

    if (!tag)
    {
        tag = create_tag (hl, ctx_node, match_node, rule);
        g_hash_table_insert (match_node->match_tags, rule, tag);
    }

    return tag;
}


static CtxNode*
ctx_node_new (MooHighlighter *hl,
              CtxNode        *parent,
              MooContext     *context,
              gboolean        create_text_tag)
{
    CtxNode *node = g_new0 (CtxNode, 1);

    g_return_val_if_fail (context != NULL, NULL);

    node->ctx = context;
    node->parent = parent;

    node->match_tags = g_hash_table_new (g_direct_hash, g_direct_equal);
    node->children = g_hash_table_new (g_direct_hash, g_direct_equal);

    if (parent)
        node->depth = parent->depth + 1;
    else
        node->depth = 0;

    if (create_text_tag)
        node->context_tag = create_tag (hl, node, NULL, NULL);

    hl->nodes = g_slist_prepend (hl->nodes, node);

    return node;
}


static CtxNode*
get_next_node (MooHighlighter *hl,
               CtxNode        *node,
               MooRule        *rule)
{
    guint i;
    CtxNode *next;

    g_assert (rule != NULL);

    if ((next = g_hash_table_lookup (node->children, rule)))
        return next;

    switch (rule->exit.type)
    {
        case MOO_CONTEXT_STAY:
            next = node;
            break;

        case MOO_CONTEXT_POP:
            for (i = 0, next = node; next->parent != NULL && i < rule->exit.num;
                 ++i, next = next->parent);
            break;

        case MOO_CONTEXT_SWITCH:
            next = ctx_node_new (hl, node, rule->exit.ctx, TRUE);
            break;
    }

    g_assert (next != NULL);
    g_hash_table_insert (node->children, rule, next);

    return next;
}


static CtxNode*
get_line_end_node (MooHighlighter *hl,
                   CtxNode        *node)
{
    guint i;
    CtxNode *next = NULL;
    MooContext *ctx = node->ctx;

    if (node->line_end)
        return node->line_end;

    switch (ctx->line_end.type)
    {
        case MOO_CONTEXT_STAY:
            next = node;
            break;

        case MOO_CONTEXT_POP:
            for (i = 0, next = node; next->parent != NULL && i < ctx->line_end.num;
                 ++i, next = next->parent);
            break;

        case MOO_CONTEXT_SWITCH:
            next = ctx_node_new (hl, node, ctx->line_end.ctx, TRUE);
            break;
    }

    g_assert (next != NULL);
    node->line_end = next;

    return next;
}


static CtxNode*
get_next_line_node (MooHighlighter *hl,
                    Line           *line)
{
    HLInfo *info = line->hl_info;
    CtxNode *last_node = NULL;

    if (!info->n_segments)
    {
        last_node = info->start_node;
    }
    else
    {
        Segment *last = &info->segments[info->n_segments - 1];

        if (last->rule)
        {
            if (last->rule->include_eol)
                return get_next_node (hl, last->ctx_node, last->rule);
            else
                last_node = get_next_node (hl, last->ctx_node, last->rule);
        }
        else
        {
            last_node = last->ctx_node;
        }
    }

    g_assert (last_node != NULL);
    return get_line_end_node (hl, last_node);
}


static CtxNode *
hl_compute_line (MooHighlighter     *hl,
                 Line               *line,
                 MatchData          *data,
                 CtxNode            *node)
{
    MooRule *matched_rule;
    MatchResult result;

    while (!gtk_text_iter_ends_line (&data->start_iter))
    {
        if ((matched_rule = _moo_rule_array_match (node->ctx->rules, data, &result)))
        {
            CtxNode *match_node, *next_node;

            if (result.match_offset < 0)
                result.match_offset = g_utf8_pointer_to_offset (data->start, result.match_start);

            if (result.match_len < 0)
                result.match_len = g_utf8_pointer_to_offset (result.match_start, result.match_end);

            if (result.match_offset)
                _moo_line_add_segment (line, result.match_offset, node, NULL, NULL);

            next_node = get_next_node (hl, node, matched_rule);

            if (matched_rule->flags & MOO_RULE_INCLUDE_INTO_NEXT)
                match_node = next_node;
            else
                match_node = node;

            _moo_line_add_segment (line, result.match_len, match_node, node, matched_rule);

            _moo_match_data_set_start (data, NULL, result.match_end,
                                       data->start_offset + result.match_offset + result.match_len);

            node = next_node;
        }
        else
        {
            _moo_line_add_segment (line, -1, node, NULL, NULL);
            break;
        }
    }

    return get_next_line_node (hl, line);
}


static gboolean
hl_compute_range (MooHighlighter *hl,
                  Interval       *lines,
                  int             time)
{
    GtkTextIter iter;
    CtxNode *node = NULL;
    int line_no;
    GTimer *timer = NULL;
    double secs = ((double) time) / 1000;
    gboolean done = FALSE;

    g_assert (!hl->line_buf->invalid.empty);
    g_assert (lines->first == hl->line_buf->invalid.first);

    if (time > 0)
        timer = g_timer_new ();

    if (lines->first == 0)
    {
        if (!hl->root)
            hl->root = ctx_node_new (hl, NULL,
                                     moo_lang_get_default_context (hl->lang),
                                     FALSE);
        node = hl->root;
    }
    else
    {
        Line *prev = _moo_line_buffer_get_line (hl->line_buf, lines->first - 1);
        node = get_next_line_node (hl, prev);
    }

    g_return_val_if_fail (node != NULL, TRUE);

    gtk_text_buffer_get_iter_at_line (hl->buffer, &iter, lines->first);

//     g_print ("hl_compute_range: %d to %d\n", lines->first, lines->last);
//     g_print ("hl_compute_range: invalid %d to %d\n",
//              hl->line_buf->invalid.first,
//              hl->line_buf->invalid.last);

    for (line_no = lines->first; line_no <= lines->last; ++line_no)
    {
        Line *line = _moo_line_buffer_get_line (hl->line_buf, line_no);
        MatchData match_data;

//         g_assert (line->hl_info->tags || !line->hl_info->tags_applied || !line->hl_info->n_segments);
        g_assert (line_no == gtk_text_iter_get_line (&iter));

        line->hl_info->start_node = node;
        line->hl_info->tags_applied = FALSE;
        _moo_line_erase_segments (line);

        /* TODO: there is no need to recompute line if its start context matches
                 context implied by the previous line */
        _moo_match_data_init (&match_data, line_no, &iter, NULL);
        node = hl_compute_line (hl, line, &match_data, node);
        _moo_match_data_destroy (&match_data);

#if 0
        {
            guint i;
            g_print ("line %d, %d segments\n", line_no, line->hl_info->n_segments);
            for (i = 0; i < line->hl_info->n_segments; ++i)
            {
                Segment *s = &line->hl_info->segments[i];
                g_print ("segment %d: %d chars, context '%s'", i, s->len, s->ctx_node->ctx->name);
                if (s->match_node)
                    g_print (", match context '%s'", s->match_node->ctx->name);
                if (s->rule)
                    g_print (", rule '%s'", s->rule->description);
                g_print ("\n");
            }
        }
#endif

        if (!gtk_text_iter_forward_line (&iter))
        {
            g_assert (line_no >= lines->last - 1); /* if last line is empty,
                                                   then this last line 'does not
                                                   really exist'*/
            g_assert (line_no >= hl->line_buf->invalid.last - 1);
            done = TRUE;
            break;
        }

        if (line_no == hl->line_buf->invalid.last)
        {
            Line *next = _moo_line_buffer_get_line (hl->line_buf, line_no + 1);

            if (node == next->hl_info->start_node)
            {
                done = TRUE;
                break;
            }
            else
            {
                hl->line_buf->invalid.last++;
            }
        }

        if (timer && (g_timer_elapsed (timer, NULL) > secs))
            break;
    }

    if (!done)
    {
        hl->line_buf->invalid.first = line_no;
        if (line_no <= lines->last)
            hl->line_buf->invalid.first++;
        _moo_line_buffer_clamp_invalid (hl->line_buf);
    }
    else
    {
        BUF_SET_CLEAN (hl->line_buf);
    }

    if (timer)
        g_timer_destroy (timer);

    _moo_text_buffer_highlighting_changed (MOO_TEXT_BUFFER (hl->buffer),
                                           lines->first, line_no);
//     g_print ("changed %d to %d\n", lines->first, line_no);
    return done;
}


static gboolean
moo_highlighter_compute_timed (MooHighlighter     *hl,
                               int                 first_line,
                               int                 last_line,
                               int                 time)
{
    Interval to_highlight;

    if (!hl->lang || !hl->buffer)
        return TRUE;

    if (BUF_CLEAN (hl->line_buf))
        return TRUE;

    if (last_line < 0)
        last_line = _moo_text_btree_size (hl->line_buf->tree) - 1;

    g_assert (first_line >= 0);
    g_assert (last_line >= first_line);

    to_highlight.first = first_line;
    to_highlight.last = last_line;

    if (hl->line_buf->invalid.first > to_highlight.last)
        return TRUE;

    to_highlight.first = hl->line_buf->invalid.first;
    return hl_compute_range (hl, &to_highlight, time);
}


static gboolean
compute_in_idle (MooHighlighter *hl)
{
    hl->idle = 0;

    if (!BUF_CLEAN (hl->line_buf))
        moo_highlighter_compute_timed (hl, 0, -1,
                                       IDLE_HIGHLIGHT_TIME);

    if (!BUF_CLEAN (hl->line_buf))
        hl->idle = g_timeout_add_full (IDLE_HIGHLIGHT_PRIORITY,
                                       IDLE_HIGHLIGHT_TIME,
                                       (GSourceFunc) compute_in_idle,
                                       hl, NULL);

    return FALSE;
}


void
_moo_highlighter_queue_compute (MooHighlighter *hl)
{
    if (!hl->lang || !hl->buffer || BUF_CLEAN (hl->line_buf))
        return;

    if (!hl->idle)
        hl->idle = g_idle_add_full (IDLE_HIGHLIGHT_PRIORITY,
                                    (GSourceFunc) compute_in_idle,
                                    hl, NULL);
}


void
_moo_highlighter_apply_tags (MooHighlighter     *hl,
                             int                 first_line,
                             int                 last_line)
{
    int line_no;
    int total;
    int first_changed, last_changed;

    if (!hl->lang || !hl->buffer)
        return;

    total = _moo_text_btree_size (hl->line_buf->tree);

    if (last_line < 0)
        last_line = total - 1;

    first_line = CLAMP (first_line, 0, total - 1);
    last_line = CLAMP (last_line, first_line, total - 1);

    g_assert (first_line >= 0);
    g_assert (last_line >= first_line);

    if (!moo_highlighter_compute_timed (hl, first_line, last_line,
                                        COMPUTE_NOW_TIME))
        _moo_highlighter_queue_compute (hl);

    first_changed = last_changed = -1;

    for (line_no = first_line; line_no <= last_line; ++line_no)
    {
        Line *line = _moo_line_buffer_get_line (hl->line_buf, line_no);
        HLInfo *info = line->hl_info;
        guint i;
        GtkTextIter t_start, t_end;
        GSList *tags;

        if (!hl->line_buf->invalid.empty && line_no >= hl->line_buf->invalid.first)
            break;

        if (line->hl_info->tags_applied)
            continue;

        if (first_changed < 0)
            first_changed = line_no;
        last_changed = line_no;

        line->hl_info->tags_applied = TRUE;
        tags = line->hl_info->tags;
        line->hl_info->tags = NULL;

        gtk_text_buffer_get_iter_at_line (hl->buffer, &t_start, line_no);

        if (gtk_text_iter_ends_line (&t_start))
        {
            g_slist_foreach (tags, (GFunc) g_object_unref, NULL);
            g_slist_free (tags);
            continue;
        }

        t_end = t_start;
        gtk_text_iter_forward_to_line_end (&t_end);

        while (tags)
        {
            gtk_text_buffer_remove_tag (hl->buffer, tags->data, &t_start, &t_end);
            g_object_unref (tags->data);
            tags = g_slist_delete_link (tags, tags);
        }

        t_end = t_start;

        for (i = 0; i < info->n_segments; ++i)
        {
            if (info->segments[i].len < 0)
                gtk_text_iter_forward_to_line_end (&t_end);
            else
                gtk_text_iter_forward_chars (&t_end, info->segments[i].len);

            apply_tag (hl, line, info->segments[i].ctx_node,
                       info->segments[i].match_node,
                       info->segments[i].rule,
                       &t_start, &t_end);

            t_start = t_end;
        }
    }

    if (first_changed >= 0)
        _moo_text_buffer_tags_changed (MOO_TEXT_BUFFER (hl->buffer),
                                       first_changed, last_changed);
}


static void
tag_set_scheme (G_GNUC_UNUSED gpointer whatever,
                MooSyntaxTag       *tag,
                MooTextStyleScheme *scheme)
{
    if (tag)
    {
        _moo_lang_erase_tag_style (GTK_TEXT_TAG (tag));
        _moo_lang_set_tag_style (tag->rule ? tag->rule->context->lang : tag->ctx_node->ctx->lang,
                                 GTK_TEXT_TAG (tag),
                                 tag->rule ? tag->rule->context : tag->ctx_node->ctx,
                                 tag->rule, scheme);
    }
}

static void
ctx_node_set_scheme (CtxNode            *node,
                     MooTextStyleScheme *scheme)
{
    g_hash_table_foreach (node->match_tags, (GHFunc) tag_set_scheme, scheme);
    tag_set_scheme (NULL, MOO_SYNTAX_TAG (node->context_tag), scheme);
}

void
_moo_highlighter_apply_scheme (MooHighlighter     *hl,
                               MooTextStyleScheme *scheme)
{
    g_return_if_fail (hl != NULL && scheme != NULL);
    g_slist_foreach (hl->nodes, (GFunc) ctx_node_set_scheme, scheme);
}
