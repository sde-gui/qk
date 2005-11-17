/*
 *   moohighlighter.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moohighlighter.h"
#include "mooedit/moolang-rules.h"
#include "mooedit/gtksourceiter.h"
#include <string.h>


#define IDLE_HIGHLIGHT_PRIORITY GTK_TEXT_VIEW_PRIORITY_VALIDATE
// #define IDLE_HIGHLIGHT_PRIORITY G_PRIORITY_DEFAULT_IDLE
#define IDLE_HIGHLIGHT_TIME     30
// #define IDLE_QUEUE_TIMEOUT      100


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

static void          hl_compute_line        (MooHighlighter     *hl,
                                             Line               *line,
                                             int                 line_no,
                                             MatchData          *data,
                                             CtxNode           **node_p,
                                             gboolean            apply_tags,
                                             gboolean            is_empty);
static void          hl_compute_range       (MooHighlighter     *hl,
                                             Interval           *lines,
                                             gboolean            apply_tags,
                                             int                 time);


/* MOO_TYPE_SYNTAX_TAG */
G_DEFINE_TYPE (MooSyntaxTag, moo_syntax_tag, GTK_TYPE_TEXT_TAG)


// static void
// moo_syntax_tag_finalize (GObject *object)
// {
//     g_free (MOO_SYNTAX_TAG(object)->stack);
//     G_OBJECT_CLASS(moo_syntax_tag_parent_class)->finalize (object);
// }


static void
moo_syntax_tag_class_init (G_GNUC_UNUSED MooSyntaxTagClass *klass)
{
//     G_OBJECT_CLASS(klass)->finalize = moo_syntax_tag_finalize;
}


static void
moo_syntax_tag_init (MooSyntaxTag *tag)
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
moo_highlighter_destroy (MooHighlighter *hl,
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
    MooSyntaxTag *tag = iter_get_syntax_tag (iter);
    return tag ? GTK_TEXT_TAG (tag) : NULL;
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
#if 0
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
           CtxNode            *ctx_node,
           CtxNode            *match_node,
           MooRule            *rule,
           const GtkTextIter  *start,
           const GtkTextIter  *end,
           gboolean            remove_old)
{
    GtkTextIter tag_start = *start;
    GtkTextTag *tag = get_syntax_tag (hl, ctx_node, match_node, rule);

    g_assert (!tag || (MOO_IS_SYNTAX_TAG (tag) && MOO_SYNTAX_TAG(tag)->rule == rule));
    g_assert (!tag || MOO_SYNTAX_TAG(tag)->match_node != NULL || MOO_SYNTAX_TAG(tag)->ctx_node != hl->root);

    if (!gtk_text_iter_compare (start, end))
        return;

    while (remove_old && gtk_text_iter_compare (&tag_start, end) < 0)
    {
        MooSyntaxTag *old_tag = iter_get_syntax_tag (&tag_start);

        if (old_tag && (old_tag != (MooSyntaxTag*) tag))
        {
            GtkTextIter tag_end = tag_start;

            g_assert (!gtk_text_iter_ends_tag (&tag_end, GTK_TEXT_TAG (old_tag)));

            gtk_text_iter_forward_to_tag_toggle (&tag_end, GTK_TEXT_TAG (old_tag));

            if (gtk_text_iter_compare (end, &tag_end) < 0)
                tag_end = *end;

            gtk_text_buffer_remove_tag (hl->buffer, GTK_TEXT_TAG (old_tag), &tag_start, &tag_end);

            tag_start = tag_end;
        }
        else
        {
            if (!gtk_text_iter_forward_to_tag_toggle (&tag_start, NULL))
                break;
        }
    }

    g_assert (iter_get_syntax_tag (start) == NULL ||
            iter_get_syntax_tag (start) == MOO_SYNTAX_TAG (tag));

    if (tag)
        _moo_text_buffer_apply_syntax_tag (MOO_TEXT_BUFFER (hl->buffer), tag, start, end);
}


MooHighlighter*
moo_highlighter_new (GtkTextBuffer *buffer,
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
    GtkTextTag *tag = moo_syntax_tag_new (ctx_node, match_node, rule);

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
    CtxNode *last_node;

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


static void
hl_compute_line (MooHighlighter     *hl,
                 Line               *line,
                 int                 line_no,
                 MatchData          *data,
                 CtxNode           **node_p,
                 gboolean            apply_tags,
                 gboolean            dirty)
{
    CtxNode *node = *node_p;
    MooRule *matched_rule = NULL;
    MatchResult result;

    if (apply_tags)
    {
        LINE_SET_TAGS_APPLIED (line);
    }

    if (!gtk_text_iter_ends_line (&data->start_iter) &&
         (matched_rule = moo_rule_array_match (node->ctx->rules, data, &result)))
    {
        CtxNode *match_node;

        /* XXX make it a cycle instead of recursive function */

        if (result.match_offset < 0)
            result.match_offset = g_utf8_pointer_to_offset (data->start, result.match_start);

        if (result.match_len < 0)
            result.match_len = g_utf8_pointer_to_offset (result.match_start, result.match_end);

        moo_line_add_segment (line, result.match_offset, node, NULL, NULL);

        *node_p = get_next_node (hl, node, matched_rule);

        if (matched_rule->flags & MOO_RULE_INCLUDE_INTO_NEXT)
            match_node = *node_p;
        else
            match_node = node;

        moo_line_add_segment (line, result.match_len, match_node, node, matched_rule);

        if (apply_tags)
        {
            GtkTextIter m_start = data->start_iter, m_end;
            gtk_text_iter_forward_chars (&m_start, result.match_offset);
            m_end = m_start;
            gtk_text_iter_forward_chars (&m_end, result.match_len);

            apply_tag (hl, node, NULL, NULL, &data->start_iter, &m_start, dirty);
            apply_tag (hl, match_node, node, matched_rule, &m_start, &m_end, dirty);

            moo_match_data_set_start (data, &m_end, result.match_end,
                                      data->start_offset + result.match_offset + result.match_len);
        }
        else
        {
            moo_match_data_set_start (data, NULL, result.match_end,
                                      data->start_offset + result.match_offset + result.match_len);
        }

        return hl_compute_line (hl, line, line_no, data, node_p, apply_tags, dirty);
    }

    if (!gtk_text_iter_ends_line (&data->start_iter))
    {
        moo_line_add_segment (line, -1, node, NULL, NULL);

        if (apply_tags)
        {
            GtkTextIter eol = data->start_iter;
            gtk_text_iter_forward_to_line_end (&eol);
            apply_tag (hl, node, NULL, NULL, &data->start_iter, &eol, dirty);
        }
    }

    *node_p = get_next_line_node (hl, line);
}


static void
hl_compute_range (MooHighlighter *hl,
                  Interval       *lines,
                  gboolean        apply_tags,
                  int             time)
{
    GtkTextIter iter;
    CtxNode *node;
    int line_no;
    GTimer *timer = NULL;
    double secs = ((double) time) / 1000;

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
        Line *prev = moo_line_buffer_get_line (hl->line_buf, lines->first - 1);
        node = get_next_line_node (hl, prev);
    }

    g_return_if_fail (node != NULL);

    gtk_text_buffer_get_iter_at_line (hl->buffer, &iter, lines->first);

    for (line_no = lines->first; line_no <= lines->last; ++line_no)
    {
        Line *line = moo_line_buffer_get_line (hl->line_buf, line_no);
        gboolean dirty = LINE_DIRTY (line);
        MatchData match_data;

        g_assert (dirty || !LINE_TAGS_APPLIED (line));
        g_assert (line_no == gtk_text_iter_get_line (&iter));

        line->hl_info->start_node = node;
        LINE_UNSET_TAGS_APPLIED (line);
        moo_line_erase_segments (line);

        /* TODO: there is no need to recompute line if its start context matches
                 context implied by the previous line */
        moo_match_data_init (&match_data, line_no, &iter, NULL);
        hl_compute_line (hl, line, line_no, &match_data, &node, apply_tags, dirty);
        moo_match_data_destroy (&match_data);

        if (!gtk_text_iter_forward_line (&iter))
        {
            g_assert (line_no >= lines->last - 1); /* if last line is empty,
                                                   then this last line 'does not
                                                   really exist'*/
            g_assert (line_no >= hl->line_buf->invalid.last - 1);
            goto done;
        }

        if (line_no == hl->line_buf->invalid.last)
        {
            Line *next = moo_line_buffer_get_line (hl->line_buf, line_no + 1);

            if (node == next->hl_info->start_node)
            {
                goto done;
            }
            else
            {
                hl->line_buf->invalid.last++;
            }
        }

        if (timer && (g_timer_elapsed (timer, NULL) > secs))
            break;
    }

    /* not done yet */
    hl->line_buf->invalid.first = line_no;
    if (line_no <= lines->last)
        hl->line_buf->invalid.first++;
    moo_line_buffer_clamp_invalid (hl->line_buf);

    if (timer)
        g_timer_destroy (timer);

    return;

done:
    BUF_SET_CLEAN (hl->line_buf);
    if (timer)
        g_timer_destroy (timer);
    return;
}


static void
moo_highlighter_compute_timed (MooHighlighter     *hl,
                               int                 first_line,
                               int                 last_line,
                               gboolean            apply_tags,
                               int                 time)
{
    Interval to_highlight;

    if (!hl->lang || !hl->buffer)
        return;

    if (BUF_CLEAN (hl->line_buf))
        return;

    if (last_line < 0)
        last_line = moo_text_btree_size (hl->line_buf->tree) - 1;

    g_assert (first_line >= 0);
    g_assert (last_line >= first_line);

    to_highlight.first = first_line;
    to_highlight.last = last_line;

    if (hl->line_buf->invalid.first > to_highlight.last)
        return;

    to_highlight.first = hl->line_buf->invalid.first;
    hl_compute_range (hl, &to_highlight, apply_tags, time);
}


void
moo_highlighter_compute (MooHighlighter     *hl,
                         int                 first,
                         int                 last,
                         gboolean            apply_tags)
{
    moo_highlighter_compute_timed (hl, first, last, apply_tags, -1);
}


static gboolean
compute_in_idle (MooHighlighter *hl)
{
    hl->idle = 0;

    if (!BUF_CLEAN (hl->line_buf))
        moo_highlighter_compute_timed (hl, 0, -1, hl->apply_tags,
                                       IDLE_HIGHLIGHT_TIME);

    if (!BUF_CLEAN (hl->line_buf))
        moo_highlighter_queue_compute (hl, hl->apply_tags);

    return FALSE;
}


void
moo_highlighter_queue_compute (MooHighlighter     *hl,
                               gboolean            apply_tags)
{
    if (!hl->lang || !hl->buffer || BUF_CLEAN (hl->line_buf))
        return;

    if (!hl->idle)
        hl->idle = g_idle_add_full (IDLE_HIGHLIGHT_PRIORITY,
                                    (GSourceFunc) compute_in_idle,
                                    hl, NULL);

    hl->apply_tags = apply_tags;
}


void
moo_highlighter_apply_tags (MooHighlighter     *hl,
                            int                 first_line,
                            int                 last_line)
{
    int line_no;

    if (!hl->lang || !hl->buffer)
        return;

    if (last_line < 0)
        last_line = moo_text_btree_size (hl->line_buf->tree) - 1;

    g_assert (first_line >= 0);
    g_assert (last_line >= first_line);

    moo_highlighter_compute (hl, first_line, last_line, TRUE);

    for (line_no = first_line; line_no <= last_line; ++line_no)
    {
        Line *line = moo_line_buffer_get_line (hl->line_buf, line_no);
        HLInfo *info = line->hl_info;
        guint i;
        GtkTextIter t_start, t_end;
        gboolean got_iter = FALSE;

        if (LINE_TAGS_APPLIED (line))
            continue;

        for (i = 0; i < info->n_segments; ++i)
        {
            if (!info->segments[i].len)
                continue;

            if (!got_iter)
            {
                gtk_text_buffer_get_iter_at_line (hl->buffer, &t_start, line_no);
                t_end = t_start;
                got_iter = TRUE;
            }
            else
            {
                t_start = t_end;
            }

            if (info->segments[i].len < 0)
                gtk_text_iter_forward_to_line_end (&t_end);
            else
                gtk_text_iter_forward_chars (&t_end, info->segments[i].len);

            apply_tag (hl, info->segments[i].ctx_node,
                       info->segments[i].match_node,
                       info->segments[i].rule,
                       &t_start, &t_end, LINE_DIRTY (line));
        }

        LINE_SET_TAGS_APPLIED (line);
    }
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
moo_highlighter_apply_scheme (MooHighlighter     *hl,
                              MooTextStyleScheme *scheme)
{
    g_return_if_fail (hl != NULL && scheme != NULL);
    g_slist_foreach (hl->nodes, (GFunc) ctx_node_set_scheme, scheme);
}
