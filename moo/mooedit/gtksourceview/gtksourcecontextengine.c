/* -*- mode: c; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *  gtksourcecontextengine.c
 *
 *  Copyright (C) 2003 - Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *  Copyright (C) 2005, 2006 - Marco Barisione, Emanuele Aina
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* FIXME: toplevel children must have extend-parent=TRUE, because create_reg_all wants
   end regex in toplevel context in this case */

#include "gtksourceview-i18n.h"
#include "gtksourcecontextengine.h"
#include "gtktextregion.h"
#include "gtksourcetag.h"
#include "gtksourcelanguage-private.h"
#include "gtksourcebuffer.h"
#include "eggregex.h"
#include <errno.h>
#include <string.h>

#undef ENABLE_DEBUG
#undef ENABLE_PROFILE
#undef ENABLE_CHECK_TREE

#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

#ifdef ENABLE_PROFILE
#define PROFILE(x) (x)
#else
#define PROFILE(x)
#endif

#if defined (ENABLE_DEBUG) || defined (ENABLE_PROFILE) || \
    defined (ENABLE_CHECK_TREE)
#define NEED_DEBUG_ID
#endif

/* Regex used to match "\%{...@start}". */
#define START_REF_REGEX "(?<!\\\\)(\\\\\\\\)*\\\\%\\{(.*?)@start\\}"

#define FIRST_UPDATE_PRIORITY		G_PRIORITY_HIGH_IDLE
#define INCREMENTAL_UPDATE_PRIORITY	GTK_TEXT_VIEW_PRIORITY_VALIDATE
/* In milliseconds. */
#define FIRST_UPDATE_TIME_SLICE		10
#define INCREMENTAL_UPDATE_TIME_SLICE	30

#define GTK_SOURCE_CONTEXT_ENGINE_ERROR (gtk_source_context_engine_error_quark ())

/* Returns the definition corrsponding to the specified id. */
#define LOOKUP_DEFINITION(ce, id) \
	(g_hash_table_lookup ((ce)->priv->definitions, (id)))

/* Can the context be terminated by ancestor? */
#define ANCESTOR_CAN_END_CONTEXT(context) \
	(!(context)->definition->extend_parent || \
	 !(context)->all_ancestors_extend)

#define CONTEXT_IS_SIMPLE(c) ((c)->definition->type == CONTEXT_TYPE_SIMPLE)
#define CONTEXT_IS_CONTAINER(c) ((c)->definition->type == CONTEXT_TYPE_CONTAINER)
#define SEGMENT_IS_INVALID(s) ((s)->context == NULL)
#define SEGMENT_IS_SIMPLE(s) CONTEXT_IS_SIMPLE ((s)->context)
#define SEGMENT_IS_CONTAINER(s) CONTEXT_IS_CONTAINER ((s)->context)

typedef struct _RegexInfo RegexInfo;
typedef struct _Regex Regex;
typedef struct _SubPatternDefinition SubPatternDefinition;
typedef struct _SubPattern SubPattern;
typedef struct _Segment Segment;
typedef struct _Context Context;
typedef struct _ContextPtr ContextPtr;
typedef struct _ContextDefinition ContextDefinition;
typedef struct _DefinitionChild DefinitionChild;
typedef struct _DefinitionsIter DefinitionsIter;
typedef struct _LineInfo LineInfo;
typedef struct _InvalidRegion InvalidRegion;

typedef enum {
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_DUPLICATED_ID = 0,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_ARGS,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_PARENT,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_REF,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_WHERE,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_START_REF,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_REGEX,
	GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_STYLE
} GtkSourceContextEngineError;

typedef enum {
	CONTEXT_TYPE_SIMPLE = 0,
	CONTEXT_TYPE_CONTAINER,
} ContextType;

typedef enum {
	SUB_PATTERN_WHERE_DEFAULT = 0,
	SUB_PATTERN_WHERE_START,
	SUB_PATTERN_WHERE_END
} SubPatternWhere;

struct _RegexInfo
{
	gchar			*pattern;
	EggRegexCompileFlags	 flags;
};

/* We do not use directly EggRegex to allow the use of "\%{...@start}". */
struct _Regex
{
	union {
		EggRegex	*regex;
		RegexInfo	 info;
	} u;
	guint			 ref_count;
	guint			 resolved : 1;
};

struct _ContextDefinition
{
	gchar			*id;

	ContextType		 type;
	union
	{
		Regex		*match;
		struct {
			Regex	*start;
			Regex	*end;
		}		 start_end;
	} u;

	/* Name of the style used for contexts of this type. */
	gchar			*style;

	/* This is a list of DefinitionChild pointers. */
	GSList			*children;

	/* Sub patterns (list of SubPatternDefinition pointers.) */
	GSList			*sub_patterns;
	guint			 n_sub_patterns;

	/* Union of every regular expression we can find from this
	 * context. */
	Regex			*reg_all;

	/* Should this context start only at the first line? */
	guint			 first_line_only : 1;

	/* Should this context end before end of the line (namely,
	 * before the line terminating characters)? */
	guint			 end_at_line_end : 1;

	/* Does this context can extend its parent? */
	guint			 extend_parent : 1;
};

struct _SubPatternDefinition
{
#ifdef NEED_DEBUG_ID
	/* We need the id only for debugging. */
	gchar			*id;
#endif
	gchar			*style;
	SubPatternWhere		 where;

	/* index in the ContextDefinition's list */
	guint			 index;

	union
	{
		gint	 	 num;
		gchar		*name;
	} u;
	guint			 is_named : 1;
};

struct _DefinitionChild
{
	ContextDefinition	*definition;
	/* Whether this child is a reference to all child contexts of
	 * <definition>. */
	guint			 is_ref_all : 1;
};

struct _DefinitionsIter
{
	GSList			*children_stack;
};

struct _Context
{
	/* Definition for the context. */
	ContextDefinition	*definition;

	Context			*parent;
	ContextPtr		*children;

	/* This is the regex returned by regex_resolve() called on
	 * definition->start_end.end. */
	Regex			*end;
	/* The regular expression containing every regular expression that
	 * could be matched in this context. */
	Regex			*reg_all;

	GtkTextTag		*tag;
	GtkTextTag	       **subpattern_tags;

	guint			 ref_count;
	guint                    frozen : 1;

	/* Do all the ancestors extend their parent? */
	guint			 all_ancestors_extend : 1;
};

struct _ContextPtr
{
	ContextDefinition	*definition;

	ContextPtr		*next;

	union {
		Context		*context;
		GHashTable	*hash; /* char* -> Context* */
	} u;
	guint			 fixed : 1;
};

struct _Segment
{
	Segment			*parent;
	Segment			*next;
	Segment			*prev;
	Segment			*children;
	Segment			*last_child;

	/* This is NULL if and only if it's a dummy segment which denotes
	 * inserted or deleted text. */
	Context			*context;

	/* Subpatterns found in this segment. */
	SubPattern		*sub_patterns;

	/* The context is used in the interval [start_at; end_at). */
	gint			 start_at;
	gint			 end_at;
};

struct _SubPattern
{
	SubPatternDefinition	*definition;
	gint			 start_at;
	gint			 end_at;
	SubPattern		*next;
};

/* Line terminator characters (\n, \r, \r\n, or unicode paragraph separator)
 * are removed from the line text. The problem is that pcre does not understand
 * arbitrary line terminators, so $ in pcre means (?=\n) (not quite, it's also
 * end of matched string), while we really need "((?=\r\n)|(?=[\r\n])|(?=\xE2\x80\xA9)|$)".
 * It could be worked around by replacing line terminator in matched text with
 * \n, but it's a good source of errors, since offsets (not all, unfortunately) returned
 * from pcre need to be compared to line length, and adjusted when necessary.
 * Not using line terminator only means that \n can't be in patterns, it's not a
 * big deal: line end can't be highlighted anyway; if a rule needs to match it, it can
 * can use "$" as start and "^" as end (not in a single pattern, "$^" will never match).
 */
#define NEXT_LINE_OFFSET(l_) ((l_)->start_at + (l_)->length + (l_)->eol_length)
struct _LineInfo
{
	/* Line text. */
	gchar			*text;
	/* Character offset of the line in text buffer. */
	gint			 start_at;
	/* Character length of <text>. */
	gint			 length;
	/* Character length of line terminator, or 0 if it's the
	 * last line in buffer. */
	gint			 eol_length;
};

struct _InvalidRegion
{
	gboolean		 empty;
	GtkTextMark		*start;
	GtkTextMark		*end;
	/* offset_at(end) - delta == original offset,
	 * i.e. offset in the tree */
	gint			 delta;
};

struct _GtkSourceContextEnginePrivate
{
	/* Language id. */
	gchar			*id;
	GtkSourceLanguage	*lang;

	GtkTextBuffer		*buffer;
	GtkSourceStyleScheme	*style_scheme;

	/* Contains every ContextDefinition indexed by its id. */
	GHashTable		*definitions;
	/* All tags indexed by style name: values are GSList's of tags, ref()'ed. */
	GHashTable		*tags;

	/* Whether or not to actually highlight the buffer. */
	gboolean		 highlight;

	/* Region covering the unhighlighted text. */
	GtkTextRegion		*refresh_region;

	/* Tree of contexts. */
	Context			*root_context;
	Segment			*root_segment;
	Segment			*hint;
	/* list of Segment* */
	GSList			*invalid;
	InvalidRegion		 invalid_region;

	guint			 first_update;
	guint			 incremental_update;

	/* Views highlight requests. */
	GtkTextRegion		*highlight_requests;
};


#ifdef ENABLE_CHECK_TREE
static void CHECK_TREE (GtkSourceContextEngine *ce);
static void CHECK_SEGMENT_LIST (Segment *segment);
static void CHECK_SEGMENT_CHILDREN (Segment *segment);
#else
#define CHECK_TREE(ce)
#define CHECK_SEGMENT_LIST(s)
#define CHECK_SEGMENT_CHILDREN(s)
#endif


static GQuark		gtk_source_context_engine_error_quark (void) G_GNUC_CONST;

static Segment	       *create_segment		(GtkSourceContextEngine *ce,
						 Segment		*parent,
						 Context		*context,
						 gint			 start_at,
						 gint			 end_at,
						 Segment		*hint);
static Segment	       *segment_new		(GtkSourceContextEngine *ce,
						 Segment		*parent,
						 Context		*context,
						 gint			 start_at,
						 gint			 end_at);
static Context	       *context_new		(Context		*parent,
						 ContextDefinition	*definition,
						 const gchar		*line_text);
static void		context_unref		(Context		*context);
static void		context_freeze		(Context		*context);
static void		context_thaw		(Context		*context);
static void		erase_segments		(GtkSourceContextEngine *ce,
						 gint                    start,
						 gint                    end,
						 Segment                *hint);
static void		find_insertion_place	(Segment		*segment,
						 gint			 offset,
						 Segment	       **parent,
						 Segment	       **prev,
						 Segment	       **next,
						 Segment		*hint);
static void		segment_destroy		(GtkSourceContextEngine	*ce,
						 Segment		*segment);
static void		context_definition_free	(ContextDefinition	*definition);

static void		segment_extend		(Segment		*state,
						 gint			 end_at);
static Context	       *ancestor_context_ends_here (Context		*state,
						 LineInfo		*line,
						 gint			 pos);
static void		definition_iter_init	(DefinitionsIter	*iter,
						 ContextDefinition	*definition);
static ContextDefinition *definition_iter_next	(DefinitionsIter	*iter);
static void		definition_iter_destroy	(DefinitionsIter	*iter);

static void		update_syntax		(GtkSourceContextEngine	*ce,
						 const GtkTextIter	*end,
						 gint			 time);
static void		install_idle_worker	(GtkSourceContextEngine	*ce);
static void		install_first_update	(GtkSourceContextEngine	*ce);


/* MODIFICATIONS AND STUFF ------------------------------------------------ */

static void
unhighlight_region_cb (G_GNUC_UNUSED gpointer style,
		       GSList   *tags,
		       gpointer  user_data)
{
	struct {
		GtkTextBuffer *buffer;
		const GtkTextIter *start, *end;
	} *data = user_data;

	while (tags)
	{
		gtk_text_buffer_remove_tag (data->buffer,
					    tags->data,
					    data->start,
					    data->end);
		tags = tags->next;
	}
}

static void
unhighlight_region (GtkSourceContextEngine *ce,
		    const GtkTextIter      *start,
		    const GtkTextIter      *end)
{
	struct {
		GtkTextBuffer *buffer;
		const GtkTextIter *start, *end;
	} data = {ce->priv->buffer, start, end};

	if (gtk_text_iter_equal (start, end))
		return;

	g_hash_table_foreach (ce->priv->tags, (GHFunc) unhighlight_region_cb, &data);
}

static void
set_tag_style (GtkSourceContextEngine *ce,
	       GtkTextTag             *tag,
	       const gchar            *style_name)
{
	GtkSourceStyle *style;

	g_return_if_fail (GTK_IS_TEXT_TAG (tag));
	g_return_if_fail (style_name != NULL);

	_gtk_source_style_unapply (tag);

	if (!ce->priv->style_scheme)
		return;

	style = gtk_source_style_scheme_get_style (ce->priv->style_scheme, style_name);

	if (!style)
	{
		const char *map_to = style_name;
		while (!style && (map_to = g_hash_table_lookup (ce->priv->lang->priv->styles, map_to)))
			style = gtk_source_style_scheme_get_style (ce->priv->style_scheme, map_to);
	}

	if (style)
	{
		_gtk_source_style_apply (style, tag);
		gtk_source_style_free (style);
	}
	else
	{
		g_warning ("could not find style '%s'", style_name);
	}
}

static GtkTextTag *
create_tag (GtkSourceContextEngine *ce,
	    const gchar            *style_name)
{
	GSList *tags;
	GtkTextTag *new_tag;
	GtkTextTagTable *table;

	g_assert (style_name != NULL);

	tags = g_hash_table_lookup (ce->priv->tags, style_name);

	new_tag = g_object_new (GTK_TYPE_SOURCE_TAG, NULL);
	table = gtk_text_buffer_get_tag_table (ce->priv->buffer);
	gtk_text_tag_table_add (table, new_tag);
	set_tag_style (ce, new_tag, style_name);

	tags = g_slist_prepend (tags, new_tag);
	g_hash_table_insert (ce->priv->tags, g_strdup (style_name), tags);

	return new_tag;
}

/* Find tag which has to be overridden. */
static GtkTextTag *
get_parent_tag (Context    *context,
		const char *style)
{
	while (context)
	{
		/* Lang files may repeat same style for nested contexts,
		 * ignore them. */
		if (context->definition->style &&
		    strcmp (context->definition->style, style) != 0)
		{
			g_assert (context->tag != NULL);
			return context->tag;
		}

		context = context->parent;
	}

	return NULL;
}

static GtkTextTag *
get_tag_for_parent (GtkSourceContextEngine *ce,
		    const char             *style,
		    Context                *parent)
{
	GSList *tags;
	GtkTextTag *parent_tag = NULL;
	GtkTextTag *tag;

	g_return_val_if_fail (style != NULL, NULL);

	parent_tag = get_parent_tag (parent, style);
	tags = g_hash_table_lookup (ce->priv->tags, style);

	if (tags && (!parent_tag ||
		gtk_text_tag_get_priority (tags->data) > gtk_text_tag_get_priority (parent_tag)))
	{
		GSList *link;

		tag = tags->data;

		/* Now get the tag with lowest priority, so that tag lists do not grow
		 * indefinitely. */
		for (link = tags->next; link != NULL; link = link->next)
		{
			if (parent_tag &&
			    gtk_text_tag_get_priority (link->data) < gtk_text_tag_get_priority (parent_tag))
				break;
			tag = link->data;
		}
	}
	else
	{
		tag = create_tag (ce, style);

#ifdef ENABLE_DEBUG
		{
			GString *style_path = g_string_new (style);
			gint n;

			while (parent)
			{
				if (parent->definition->style)
				{
					g_string_prepend (style_path, "/");
					g_string_prepend (style_path,
							  parent->definition->style);
				}

				parent = parent->parent;
			}

			tags = g_hash_table_lookup (ce->priv->tags, style);
			n = g_slist_length (tags);
			g_print ("created %d tag for style %s: %s\n", n, style, style_path->str);
			g_string_free (style_path, TRUE);
		}
#endif
	}

	return tag;
}

static GtkTextTag *
get_subpattern_tag (GtkSourceContextEngine *ce,
		    Context                *context,
		    SubPatternDefinition   *sp_def)
{
	if (!sp_def->style)
		return NULL;

	g_assert (sp_def->index < context->definition->n_sub_patterns);

	if (!context->subpattern_tags)
		context->subpattern_tags = g_new0 (GtkTextTag*, context->definition->n_sub_patterns);

	if (!context->subpattern_tags[sp_def->index])
		context->subpattern_tags[sp_def->index] = get_tag_for_parent (ce, sp_def->style, context);

	g_return_val_if_fail (context->subpattern_tags[sp_def->index] != NULL, NULL);
	return context->subpattern_tags[sp_def->index];
}

static GtkTextTag *
get_context_tag (GtkSourceContextEngine *ce,
		 Context                *context)
{
	if (context->definition->style && !context->tag)
		context->tag = get_tag_for_parent (ce,
						   context->definition->style,
						   context->parent);
	return context->tag;
}

static void
apply_tags (GtkSourceContextEngine *ce,
	    Segment                *segment,
	    gint                    start_offset,
	    gint                    end_offset)
{
	GtkTextTag *tag;
	GtkTextIter start_iter, end_iter;
	SubPattern *sp;
	guint i;
	Segment *child;

	g_assert (segment != NULL);

	if (SEGMENT_IS_INVALID (segment))
		return;

	if (segment->start_at >= end_offset || segment->end_at <= start_offset)
		return;

	start_offset = MAX (start_offset, segment->start_at);
	end_offset = MIN (end_offset, segment->end_at);

	tag = get_context_tag (ce, segment->context);

	if (tag)
	{
		gtk_text_buffer_get_iter_at_offset (ce->priv->buffer, &start_iter, start_offset);
		gtk_text_buffer_get_iter_at_offset (ce->priv->buffer, &end_iter, end_offset);
		gtk_text_buffer_apply_tag (ce->priv->buffer, tag, &start_iter, &end_iter);
	}

	for (sp = segment->sub_patterns, i = 0; sp != NULL; sp = sp->next, ++i)
	{
		if (sp->start_at >= start_offset && sp->end_at <= end_offset)
		{
			tag = get_subpattern_tag (ce, segment->context, sp->definition);

			if (tag)
			{
				gtk_text_buffer_get_iter_at_offset (ce->priv->buffer, &start_iter,
								    MAX (start_offset, sp->start_at));
				gtk_text_buffer_get_iter_at_offset (ce->priv->buffer, &end_iter,
								    MIN (end_offset, sp->end_at));
				gtk_text_buffer_apply_tag (ce->priv->buffer, tag, &start_iter, &end_iter);
			}
		}
	}

	for (child = segment->children;
	     child != NULL && child->start_at < end_offset;
	     child = child->next)
	{
		if (child->end_at <= start_offset)
			continue;
		apply_tags (ce, child, start_offset, end_offset);
	}
}

/**
 * highlight_region:
 *
 * @ce: a #GtkSourceContextEngine.
 * @start: the beginning of the region to highlight.
 * @end: the end of the region to highlight.
 *
 * Highlights the specified region.
 */
static void
highlight_region (GtkSourceContextEngine *ce,
		  GtkTextIter            *start,
		  GtkTextIter            *end)
{
#ifdef ENABLE_PROFILE
	GTimer *timer;
#endif

	if (gtk_text_iter_starts_line (end))
		gtk_text_iter_backward_char (end);
	if (gtk_text_iter_compare (start, end) >= 0)
		return;

#ifdef ENABLE_PROFILE
	timer = g_timer_new ();
#endif

	/* First we need to delete tags in the regions. */
	unhighlight_region (ce, start, end);

	apply_tags (ce, ce->priv->root_segment,
		    gtk_text_iter_get_offset (start),
		    gtk_text_iter_get_offset (end));

#ifdef ENABLE_PROFILE
	g_print ("highlight (from %d to %d), %g ms elapsed\n",
		 gtk_text_iter_get_offset (start),
		 gtk_text_iter_get_offset (end),
		 g_timer_elapsed (timer, NULL) * 1000);
	g_timer_destroy (timer);
#endif
}

/**
 * ensure_highlighted:
 *
 * @ce: a #GtkSourceContextEngine.
 * @start: the beginning of the region to highlight.
 * @end: the end of the region to highlight.
 *
 * Updates text tags in reanalyzed parts of given area.
 * It applies tags according to whatever is in the syntax
 * tree currently, so highlighting may not be correct
 * (gtk_source_context_engine_update_highlight is the method
 * that actually ensures correct highlighting).
 */
static void
ensure_highlighted (GtkSourceContextEngine *ce,
		    const GtkTextIter      *start,
		    const GtkTextIter      *end)
{
	GtkTextRegion *region;
	GtkTextRegionIterator reg_iter;

	/* Get the subregions not yet highlighted. */
	region = gtk_text_region_intersect (ce->priv->refresh_region, start, end);

	if (!region)
		return;

	gtk_text_region_get_iterator (region, &reg_iter, 0);

	/* Highlight all subregions from the intersection.
	 * hopefully this will only be one subregion. */
	while (!gtk_text_region_iterator_is_end (&reg_iter))
	{
		GtkTextIter s, e;
		gtk_text_region_iterator_get_subregion (&reg_iter, &s, &e);
		highlight_region (ce, &s, &e);
		gtk_text_region_iterator_next (&reg_iter);
	}

	gtk_text_region_destroy (region, TRUE);

	/* Remove the just highlighted region. */
	gtk_text_region_subtract (ce->priv->refresh_region, start, end);
}

/**
 * refresh_range:
 *
 * @ce: a #GtkSourceContextEngine.
 * @start: the beginning of updated area.
 * @end: the end of updated area.
 * @modify_refresh_region: whether updated area should be added to
 * refresh_region.
 *
 * Marks the area as updated - notifies view about it, and adds it to
 * refresh_region if %modify_refresh_region is TRUE (update_syntax may
 * process huge area though actually updated is couple of lines, so in
 * that case update_syntax() takes care of refresh_region, and this
 * function only notifies the view).
 */
static void
refresh_range (GtkSourceContextEngine *ce,
	       const GtkTextIter      *start,
	       const GtkTextIter      *end,
	       gboolean                modify_refresh_region)
{
	GtkTextIter real_end;

	if (gtk_text_iter_equal (start, end))
		return;

	if (modify_refresh_region)
		gtk_text_region_add (ce->priv->refresh_region, start, end);

	/* Here we need to make sure we do not make it redraw next line */
	real_end = *end;
	if (gtk_text_iter_starts_line (&real_end))
		/* XXX if it's in the middle of \r\n, what line is it? */
		gtk_text_iter_backward_char (&real_end);

	g_signal_emit_by_name (ce->priv->buffer,
			       "highlight_updated",
			       start,
			       &real_end);
}

static gint
segment_cmp (Segment *s1,
	     Segment *s2)
{
	if (s1->start_at < s2->start_at)
		return -1;
	else if (s1->start_at > s2->start_at)
		return 1;
	/* one of them must be zero-length */
	g_assert (s1->start_at == s1->end_at || s2->start_at == s2->end_at);
#ifdef ENABLE_DEBUG
	/* A new zero-length segment should never be created if there is
	 * already an invalid segment. */
	g_assert_not_reached ();
#endif
	g_return_val_if_reached (s1->end_at < s2->end_at ? -1 :
                                 (s1->end_at > s2->end_at ? 1 : 0));
}

static void
add_invalid (GtkSourceContextEngine *ce,
	     Segment                *segment)
{
#ifdef ENABLE_CHECK_TREE
	g_assert (!g_slist_find (ce->priv->invalid, segment));
#endif
	g_assert (SEGMENT_IS_INVALID (segment));

	ce->priv->invalid = g_slist_insert_sorted (ce->priv->invalid,
						   segment,
						   (GCompareFunc) segment_cmp);

	DEBUG (g_print ("%d invalid\n", g_slist_length (ce->priv->invalid)));
}

static void
remove_invalid (GtkSourceContextEngine *ce,
		Segment                *segment)
{
	g_assert (g_slist_find (ce->priv->invalid, segment) != NULL);
	ce->priv->invalid = g_slist_remove (ce->priv->invalid, segment);
}

static void
fix_offsets_insert_ (Segment *segment,
		     gint     start,
		     gint     delta)
{
	Segment *child;
	SubPattern *sp;

	g_assert (segment->start_at >= start);

	if (!delta)
		return;

	segment->start_at += delta;
	segment->end_at += delta;

	for (child = segment->children; child != NULL; child = child->next)
		fix_offsets_insert_ (child, start, delta);

	for (sp = segment->sub_patterns; sp != NULL; sp = sp->next)
	{
		sp->start_at += delta;
		sp->end_at += delta;
	}
}

static void
find_insertion_place_forward_ (Segment  *segment,
			       gint      offset,
			       Segment  *start,
			       Segment **parent,
			       Segment **prev,
			       Segment **next)
{
	Segment *child;

	g_assert (start->end_at < offset);

	for (child = start; child != NULL; child = child->next)
	{
		if (child->start_at <= offset && child->end_at >= offset)
			return find_insertion_place (child, offset, parent, prev, next, NULL);

		if (child->end_at == offset)
		{
			if (SEGMENT_IS_INVALID (child))
			{
				*parent = child;
				*prev = NULL;
				*next = NULL;
			}
			else
			{
				*prev = child;
				*next = child->next;
				*parent = segment;
			}

			return;
		}

		if (child->end_at < offset)
		{
			*prev = child;
			continue;
		}

		if (child->start_at > offset)
		{
			*next = child;
			break;
		}

		g_assert_not_reached ();
	}

	*parent = segment;
}

static void
find_insertion_place_backward_ (Segment  *segment,
				gint      offset,
				Segment  *start,
				Segment **parent,
				Segment **prev,
				Segment **next)
{
	Segment *child;

	g_assert (start->end_at >= offset);

	for (child = start; child != NULL; child = child->prev)
	{
		if (child->start_at <= offset && child->end_at >= offset)
			return find_insertion_place (child, offset, parent, prev, next, NULL);

		if (child->end_at == offset)
		{
			if (SEGMENT_IS_INVALID (child))
			{
				*parent = child;
				*prev = NULL;
				*next = NULL;
			}
			else
			{
				*prev = child;
				*next = child->next;
				*parent = segment;
			}

			return;
		}

		if (child->end_at < offset)
		{
			*prev = child;
			*next = child->next;
			break;
		}

		if (child->start_at > offset)
		{
			*next = child;
			continue;
		}

		g_assert_not_reached ();
	}

	*parent = segment;
}

static void
find_insertion_place (Segment  *segment,
		      gint      offset,
		      Segment **parent,
		      Segment **prev,
		      Segment **next,
		      Segment  *hint)
{
	g_assert (segment->start_at <= offset && segment->end_at >= offset);

	*prev = NULL;
	*next = NULL;

	if (SEGMENT_IS_INVALID (segment) || segment->children == NULL)
	{
		*parent = segment;
		return;
	}

	/* XXX grand child might be invalid, so we still can get two
	 * adjacent zero-length segments, and crash */
	if (segment->start_at == offset)
	{
		if (SEGMENT_IS_INVALID (segment->children) &&
		    segment->children->start_at == offset)
		{
			*parent = segment->children;
		}
		else
		{
			*parent = segment;
			*next = segment->children;
		}

		return;
	}

	if (hint)
		while (hint && hint->parent != segment)
			hint = hint->parent;

	if (!hint)
		hint = segment->children;

	if (hint->end_at < offset)
		find_insertion_place_forward_ (segment, offset, hint, parent, prev, next);
	else
		find_insertion_place_backward_ (segment, offset, hint, parent, prev, next);
}

static Segment *
get_invalid_at_ (GtkSourceContextEngine *ce,
		 gint                    offset)
{
	GSList *link = ce->priv->invalid;

	while (link)
	{
		Segment *segment = link->data;

		link = link->next;

		if (segment->start_at > offset)
			break;

		if (segment->end_at < offset)
			continue;

		return segment;
	}

	return NULL;
}

static void
segment_add_subpattern (Segment    *state,
			SubPattern *sp)
{
	sp->next = state->sub_patterns;
	state->sub_patterns = sp;
}

static SubPattern *
sub_pattern_new (Segment              *segment,
		 gint                  start_at,
		 gint                  end_at,
		 SubPatternDefinition *sp_def)
{
	SubPattern *sp;

	sp = g_new0 (SubPattern, 1);
	sp->start_at = start_at;
	sp->end_at = end_at;
	sp->definition = sp_def;

	segment_add_subpattern (segment, sp);

	return sp;
}

static inline void
sub_pattern_free (SubPattern *sp)
{
	g_free (sp);
}

static void
segment_make_invalid_ (GtkSourceContextEngine *ce,
		       Segment                *segment)
{
	Context *ctx;
	SubPattern *sp;

	g_assert (!SEGMENT_IS_INVALID (segment));

	sp = segment->sub_patterns;
	segment->sub_patterns = NULL;

	while (sp)
	{
		SubPattern *next = sp->next;
		sub_pattern_free (sp);
		sp = next;
	}

	ctx = segment->context;
	segment->context = NULL;
	add_invalid (ce, segment);
	context_unref (ctx);
}

static Segment *
simple_segment_split_ (GtkSourceContextEngine *ce,
		       Segment                *segment,
		       gint                    offset)
{
	SubPattern *subpatterns, *sp;
	Segment *new_segment, *invalid;
	gint end_at = segment->end_at;

	g_assert (SEGMENT_IS_SIMPLE (segment));
	g_assert (segment->start_at < offset && offset < segment->end_at);

	subpatterns = segment->sub_patterns;
	segment->sub_patterns = NULL;
	segment->end_at = offset;

	invalid = create_segment (ce, segment->parent, NULL, offset, offset, segment);
	new_segment = create_segment (ce, segment->parent, segment->context, offset, end_at, invalid);

	sp = subpatterns;
	while (sp != NULL)
	{
		Segment *append_to = NULL;
		SubPattern *next = sp->next;

		if (sp->end_at <= offset)
		{
			append_to = segment;
		}
		else if (sp->start_at >= offset)
		{
			append_to = new_segment;
		}
		else
		{
			sub_pattern_new (new_segment,
					 offset,
					 sp->end_at,
					 sp->definition);
			sp->end_at = offset;
			append_to = segment;
		}

		segment_add_subpattern (append_to, sp);

		sp = next;
	}

	return invalid;
}

/**
 * invalidate_region:
 *
 * @ce: a #GtkSourceContextEngine.
 * @offset: the start of invalidated area.
 * @length: the length of the area.
 *
 * Adds the area to the invalid region and queues highlighting.
 * %length may be negative which means deletion; positive
 * means insertion; 0 means "something happened here", it's
 * treated as zero-length insertion.
 */
static gboolean
invalidate_region (GtkSourceContextEngine *ce,
		   gint                    offset,
		   gint                    length)
{
	InvalidRegion *region = &ce->priv->invalid_region;
	GtkTextBuffer *buffer = ce->priv->buffer;
	GtkTextIter iter;
	gint end_offset;

	end_offset = length >= 0 ? offset + length : offset;

	if (region->empty)
	{
		region->empty = FALSE;
		region->delta = length;

		gtk_text_buffer_get_iter_at_offset (buffer, &iter, offset);
		gtk_text_buffer_move_mark (buffer, region->start, &iter);

		gtk_text_iter_set_offset (&iter, end_offset);
		gtk_text_buffer_move_mark (buffer, region->end, &iter);
	}
	else
	{
		gtk_text_buffer_get_iter_at_mark (buffer, &iter, region->start);

		if (gtk_text_iter_get_offset (&iter) > offset)
		{
			gtk_text_iter_set_offset (&iter, offset);
			gtk_text_buffer_move_mark (buffer, region->start, &iter);
		}

		gtk_text_buffer_get_iter_at_mark (buffer, &iter, region->end);

		if (gtk_text_iter_get_offset (&iter) < end_offset)
		{
			gtk_text_iter_set_offset (&iter, end_offset);
			gtk_text_buffer_move_mark (buffer, region->end, &iter);
		}

		region->delta += length;
	}

	DEBUG (({
		gint start, end;
		gtk_text_buffer_get_iter_at_mark (buffer, &iter, region->start);
		start = gtk_text_iter_get_offset (&iter);
		gtk_text_buffer_get_iter_at_mark (buffer, &iter, region->end);
		end = gtk_text_iter_get_offset (&iter);
		g_assert (start <= end - region->delta);
	}));

	CHECK_TREE (ce);

	install_first_update (ce);

	return TRUE;
}

/**
 * insert_range:
 *
 * @ce: a #GtkSourceContextEngine.
 * @offset: the start of new segment.
 * @length: the length of the segment.
 *
 * Updates segment tree after insertion: it updates tree
 * offsets as appropriate, and inserts a new invalid segment
 * or extends existing invalid segment as %offset, so
 * after the call segment [offset, offset + length) is marked
 * invalid in the tree.
 * It may be safely called with length == 0 at any moment
 * (and it's used here and there).
 */
static void
insert_range (GtkSourceContextEngine *ce,
	      gint                    offset,
	      gint                    length)
{
	Segment *parent, *prev = NULL, *next = NULL, *new_segment;
	Segment *segment;

	/* If there is an invalid segment adjacent to offset, use it.
	 * Otherwise, find the deepest segment to split and insert
	 * dummy segment in there. */
	if (!(parent = get_invalid_at_ (ce, offset)))
		find_insertion_place (ce->priv->root_segment, offset,
				      &parent, &prev, &next,
				      ce->priv->hint);

	g_assert (parent->start_at <= offset);
	g_assert (parent->end_at >= offset);
	g_assert (!prev || prev->parent == parent);
	g_assert (!next || next->parent == parent);
	g_assert (!prev || prev->next == next);
	g_assert (!next || next->prev == prev);

	if (SEGMENT_IS_INVALID (parent))
	{
		/* If length is zero, and we already have an invalid segment there,
		 * do nothing. */
		if (!length)
			return;

		segment = parent;
	}
	else if (SEGMENT_IS_SIMPLE (parent))
	{
		/* If it's a simple context, then:
		 * if one of its ends is offset, then we just invalidate it;
		 * otherwise, we split it into two, and insert zero-lentgh
		 * invalid segment in the middle. */
		if (parent->start_at < offset && parent->end_at > offset)
		{
			segment = simple_segment_split_ (ce, parent, offset);
		}
		else
		{
			segment_make_invalid_ (ce, parent);
			segment = parent;
		}
	}
	else
	{
		/* Just insert new zero-length invalid segment. */

		new_segment = segment_new (ce, parent, NULL, offset, offset);

		new_segment->next = next;
		new_segment->prev = prev;

		if (next)
			next->prev = new_segment;
		else
			parent->last_child = new_segment;

		if (prev)
			prev->next = new_segment;
		else
			parent->children = new_segment;

		segment = new_segment;
	}

	g_assert (!segment->children);

	if (length != 0)
	{
		/* now fix offsets in all the segments "to the right"
		 * of segment. */
		while (segment)
		{
			Segment *tmp;
			SubPattern *sp;

			for (tmp = segment->next; tmp != NULL; tmp = tmp->next)
				fix_offsets_insert_ (tmp, offset, length);

			segment->end_at += length;

			for (sp = segment->sub_patterns; sp != NULL; sp = sp->next)
			{
				if (sp->start_at > offset)
					sp->start_at += length;
				if (sp->end_at > offset)
					sp->end_at += length;
			}

			segment = segment->parent;
		}
	}

	CHECK_TREE (ce);
}

/**
 * gtk_source_context_engine_text_inserted:
 *
 * @ce: a #GtkSourceContextEngine.
 * @start_offset: the start of inserted text.
 * @end_offset: the end of inserted text.
 *
 * Called from GtkTextBuffer::insert_text.
 */
static void
gtk_source_context_engine_text_inserted (GtkSourceEngine *engine,
					 gint             start_offset,
					 gint             end_offset)
{
	g_return_if_fail (start_offset < end_offset);
	invalidate_region (GTK_SOURCE_CONTEXT_ENGINE (engine),
			   start_offset,
			   end_offset - start_offset);
}

static inline gint
fix_offset_delete_ (gint offset,
		    gint start,
		    gint length)
{
	if (offset > start)
	{
		if (offset >= start + length)
			offset -= length;
		else
			offset = start;
	}

	return offset;
}

static void
fix_offsets_delete_ (Segment *segment,
		     gint     offset,
		     gint     length,
		     Segment *hint)
{
	Segment *child;
	SubPattern *sp;

	g_return_if_fail (segment->end_at > offset);

	if (hint)
		while (hint && hint->parent != segment)
			hint = hint->parent;

	if (!hint)
		hint = segment->children;

	for (child = hint; child != NULL; child = child->next)
	{
		if (child->end_at <= offset)
			continue;
		fix_offsets_delete_ (child, offset, length, NULL);
	}

	for (child = hint ? hint->prev : NULL; child != NULL; child = child->prev)
	{
		if (child->end_at <= offset)
			break;
		fix_offsets_delete_ (child, offset, length, NULL);
	}

	for (sp = segment->sub_patterns; sp != NULL; sp = sp->next)
	{
		sp->start_at = fix_offset_delete_ (sp->start_at, offset, length);
		sp->end_at = fix_offset_delete_ (sp->end_at, offset, length);
	}

	segment->start_at = fix_offset_delete_ (segment->start_at, offset, length);
	segment->end_at = fix_offset_delete_ (segment->end_at, offset, length);
}

/**
 * delete_range_:
 *
 * @ce: a #GtkSourceContextEngine.
 * @start: the start of deleted area.
 * @end: the end of deleted area.
 *
 * Updates segment tree after deletion: removes segments at deleted
 * interval, updates tree offsets, etc.
 * It's called only from update_tree().
 */
static void
delete_range_ (GtkSourceContextEngine *ce,
	       gint                    start,
	       gint                    end)
{
	g_return_if_fail (start < end);

	/* XXX it may make two invalid segments adjacent, and we can get crash */
	erase_segments (ce, start, end, NULL);
	fix_offsets_delete_ (ce->priv->root_segment, start, end - start, ce->priv->hint);

	/* no need to invalidate at start, update_tree will do it */

	CHECK_TREE (ce);
};

/**
 * gtk_source_context_engine_text_deleted:
 *
 * @ce: a #GtkSourceContextEngine.
 * @offset: the start of deleted text.
 * @length: the length (in characters) of deleted text.
 *
 * Called from GtkTextBuffer::delete_range.
 */
static void
gtk_source_context_engine_text_deleted (GtkSourceEngine *engine,
					gint             offset,
					gint             length)
{
	g_return_if_fail (length > 0);
	invalidate_region (GTK_SOURCE_CONTEXT_ENGINE (engine),
			   offset,
			   - length);
}

/**
 * get_invalid_segment:
 *
 * @ce: a #GtkSourceContextEngine.
 *
 * Returns first invalid segment, or NULL.
 */
static Segment *
get_invalid_segment (GtkSourceContextEngine *ce)
{
	g_return_val_if_fail (ce->priv->invalid_region.empty, NULL);
	return ce->priv->invalid ? ce->priv->invalid->data : NULL;
}

/**
 * get_invalid_line:
 *
 * @ce: a #GtkSourceContextEngine.
 *
 * Returns first invalid line, or -1.
 */
static gint
get_invalid_line (GtkSourceContextEngine *ce)
{
	GtkTextIter iter;
	gint offset = G_MAXINT;

	if (!ce->priv->invalid_region.empty)
	{
		gint tmp;
		gtk_text_buffer_get_iter_at_mark (ce->priv->buffer,
						  &iter,
						  ce->priv->invalid_region.start);
		tmp = gtk_text_iter_get_offset (&iter);
		offset = MIN (offset, tmp);
	}

	if (ce->priv->invalid)
	{
		Segment *segment = ce->priv->invalid->data;
		offset = MIN (offset, segment->start_at);
	}

	if (offset == G_MAXINT)
		return -1;

	gtk_text_buffer_get_iter_at_offset (ce->priv->buffer, &iter, offset);
	return gtk_text_iter_get_line (&iter);
}

/**
 * update_tree:
 *
 * @ce: a #GtkSourceContextEngine.
 *
 * Modifies syntax tree according to data in invalid_region.
 */
static void
update_tree (GtkSourceContextEngine *ce)
{
	InvalidRegion *region = &ce->priv->invalid_region;
	gint start, end, delta;
	gint erase_start, erase_end;
	GtkTextIter iter;

	if (region->empty)
		return;

	gtk_text_buffer_get_iter_at_mark (ce->priv->buffer, &iter, region->start);
	start = gtk_text_iter_get_offset (&iter);
	gtk_text_buffer_get_iter_at_mark (ce->priv->buffer, &iter, region->end);
	end = gtk_text_iter_get_offset (&iter);

	delta = region->delta;

	g_assert (start <= MIN (end, end - delta));

	/* Here start and end are actual offsets in the buffer (they do not match offsets
	 * in the tree if delta is not zero); delta is how much was inserted/removed.
	 * First, we insert/delete range from the tree, to make offsets in tree
	 * match offsets in the buffer. Then, create an invalid segment for the rest
	 * of the area if needed. */

	if (delta > 0)
		insert_range (ce, start, delta);
	else if (delta < 0)
		delete_range_ (ce, end, end - delta);

	if (delta <= 0)
	{
		erase_start = start;
		erase_end = end;
	}
	else
	{
		erase_start = start + delta;
		erase_end = end;
	}

	if (erase_start < erase_end)
	{
		erase_segments (ce, erase_start, erase_end, NULL);
		create_segment (ce, ce->priv->root_segment, NULL, erase_start, erase_end, NULL);
	}
	else if (!get_invalid_at_ (ce, start))
	{
		insert_range (ce, start, 0);
	}

	region->empty = TRUE;

#ifdef ENABLE_CHECK_TREE
	g_assert (get_invalid_at_ (ce, start) != NULL);
	CHECK_TREE (ce);
#endif
}

/* XXX make sure regions requested and highlighted are the same,
   so we don't install an idle just because a view gave us a
   start iter of the line it doesn't care about (and vice versa
   in update_syntax) */
static void
gtk_source_context_engine_update_highlight (GtkSourceEngine   *engine,
					    const GtkTextIter *start,
					    const GtkTextIter *end,
					    gboolean           synchronous)
{
	gint invalid_line;
	gint end_line;
	GtkSourceContextEngine *ce = GTK_SOURCE_CONTEXT_ENGINE (engine);

	if (!ce->priv->highlight)
		return;

	invalid_line = get_invalid_line (ce);
	end_line = gtk_text_iter_get_line (end);

	if (gtk_text_iter_starts_line (end) && end_line > 0)
		end_line -= 1;

	if (invalid_line < 0 || invalid_line > end_line)
	{
		ensure_highlighted (ce, start, end);
	}
	else if (synchronous)
	{
		/* analyze whole region */
		update_syntax (ce, end, 0);
		ensure_highlighted (ce, start, end);
	}
	else
	{
		if (gtk_text_iter_get_line (start) >= invalid_line)
		{
			gtk_text_region_add (ce->priv->highlight_requests, start, end);
		}
		else
		{
			GtkTextIter valid_end = *start;
			gtk_text_iter_set_line (&valid_end, invalid_line);
			ensure_highlighted (ce, start, &valid_end);
			gtk_text_region_add (ce->priv->highlight_requests, &valid_end, end);
		}

		install_first_update (ce);
	}
}

static void
enable_highlight (GtkSourceContextEngine *ce,
		  gboolean                enable)
{
	GtkTextIter start, end;

	if (!enable == !ce->priv->highlight)
		return;

	ce->priv->highlight = enable != 0;
	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (ce->priv->buffer),
				    &start, &end);

	if (enable)
		refresh_range (ce, &start, &end, TRUE);
	else
		unhighlight_region (ce, &start, &end);
}

static void
buffer_notify_highlight_cb (GtkSourceContextEngine *ce)
{
	gboolean highlight;
	g_object_get (ce->priv->buffer, "highlight", &highlight, NULL);
	enable_highlight (ce, highlight);
}


/* IDLE WORKER CODE ------------------------------------------------------- */

static gboolean
all_analyzed (GtkSourceContextEngine *ce)
{
	return ce->priv->invalid == NULL && ce->priv->invalid_region.empty;
}

/**
 * idle_worker:
 *
 * @ce: #GtkSourceContextEngine.
 *
 * Analyzes a batch in idle. Stops when
 * whole buffer is analyzed.
 */
static gboolean
idle_worker (GtkSourceContextEngine *ce)
{
	g_return_val_if_fail (ce->priv->buffer != NULL, FALSE);

	/* analyze batch of text */
	update_syntax (ce, NULL, INCREMENTAL_UPDATE_TIME_SLICE);
	CHECK_TREE (ce);

	if (all_analyzed (ce))
	{
		ce->priv->incremental_update = 0;
		return FALSE;
	}

	return TRUE;
}

static gboolean
first_update_callback (GtkSourceContextEngine *ce)
{
	g_return_val_if_fail (ce->priv->buffer != NULL, FALSE);

	/* analyze batch of text */
	update_syntax (ce, NULL, FIRST_UPDATE_TIME_SLICE);
	CHECK_TREE (ce);

	ce->priv->first_update = 0;

	if (!all_analyzed (ce))
		install_idle_worker (ce);

	return FALSE;
}

/**
 * install_idle_worker:
 *
 * @ce: #GtkSourceContextEngine.
 *
 * Schedules reanalyzing buffer in idle.
 * Always safe to call.
 */
static void
install_idle_worker (GtkSourceContextEngine *ce)
{
	if (!ce->priv->first_update && !ce->priv->incremental_update)
		ce->priv->incremental_update =
			g_idle_add_full (INCREMENTAL_UPDATE_PRIORITY,
					 (GSourceFunc) idle_worker, ce, NULL);
}

static void
install_first_update (GtkSourceContextEngine *ce)
{
	if (!ce->priv->first_update)
	{
		if (ce->priv->incremental_update)
		{
			g_source_remove (ce->priv->incremental_update);
			ce->priv->incremental_update = 0;
		}

		ce->priv->first_update =
			g_idle_add_full (FIRST_UPDATE_PRIORITY,
					 (GSourceFunc) first_update_callback,
					 ce, NULL);
	}
}

/* GtkSourceContextEngine class ------------------------------------------- */

G_DEFINE_TYPE (GtkSourceContextEngine, _gtk_source_context_engine, GTK_TYPE_SOURCE_ENGINE)

static GQuark
gtk_source_context_engine_error_quark (void)
{
	static GQuark err_q = 0;
	if (err_q == 0)
		err_q = g_quark_from_static_string ("gtk-source-context-engine-error-quark");
	return err_q;
}

static void
remove_tags_hash_cb (G_GNUC_UNUSED gpointer style,
		     GSList          *tags,
		     GtkTextTagTable *table)
{
	GSList *l = tags;

	while (l != NULL)
	{
		gtk_text_tag_table_remove (table, l->data);
		g_object_unref (l->data);
		l = l->next;
	}

	g_slist_free (tags);
}

static void
destroy_tags_hash (GtkSourceContextEngine *ce)
{
	g_hash_table_foreach (ce->priv->tags, (GHFunc) remove_tags_hash_cb,
                              gtk_text_buffer_get_tag_table (ce->priv->buffer));
	g_hash_table_destroy (ce->priv->tags);
	ce->priv->tags = NULL;
}

/**
 * gtk_source_context_engine_attach_buffer:
 *
 * @ce: #GtkSourceContextEngine.
 * @buffer: buffer.
 *
 * Detaches engine from previous buffer, and attaches to @buffer if
 * it's not NULL.
 */
static void
gtk_source_context_engine_attach_buffer (GtkSourceEngine *engine,
					 GtkTextBuffer   *buffer)
{
	GtkSourceContextEngine *ce = GTK_SOURCE_CONTEXT_ENGINE (engine);

	g_return_if_fail (!buffer || GTK_IS_TEXT_BUFFER (buffer));

	if (ce->priv->buffer == buffer)
		return;

	/* Detach previous buffer if there is one. */
	if (ce->priv->buffer)
	{
		g_signal_handlers_disconnect_by_func (ce->priv->buffer,
						      (gpointer) buffer_notify_highlight_cb,
						      ce);

		if (ce->priv->first_update)
			g_source_remove (ce->priv->first_update);
		if (ce->priv->incremental_update)
			g_source_remove (ce->priv->incremental_update);
		ce->priv->first_update = 0;
		ce->priv->incremental_update = 0;

		segment_destroy (ce, ce->priv->root_segment);
		context_unref (ce->priv->root_context);
		g_slist_free (ce->priv->invalid);
		ce->priv->root_segment = NULL;
		ce->priv->root_context = NULL;
		ce->priv->invalid = NULL;

		gtk_text_buffer_delete_mark (ce->priv->buffer,
					     ce->priv->invalid_region.start);
		gtk_text_buffer_delete_mark (ce->priv->buffer,
					     ce->priv->invalid_region.end);
		ce->priv->invalid_region.start = NULL;
		ce->priv->invalid_region.end = NULL;

		/* this deletes tags from the tag table, therefore there is no need
		 * in removing tags from the text (it may be very slow). */
		destroy_tags_hash (ce);

		if (ce->priv->refresh_region)
			gtk_text_region_destroy (ce->priv->refresh_region, FALSE);
		if (ce->priv->highlight_requests)
			gtk_text_region_destroy (ce->priv->highlight_requests, FALSE);
		ce->priv->refresh_region = NULL;
		ce->priv->highlight_requests = NULL;
	}

	ce->priv->buffer = buffer;

	if (buffer)
	{
		gchar *root_id;
		ContextDefinition *main_definition;
		GtkTextIter start, end;

		/* Create the root context. */
		root_id = g_strdup_printf ("%s:%s", ce->priv->id, ce->priv->id);
		main_definition = g_hash_table_lookup (ce->priv->definitions,
						       root_id);
		g_free (root_id);

		if (!main_definition)
		{
			g_warning (_ ("Missing main language "
				      "definition (id = \"%s\".)"),
				   ce->priv->id);
			return;
		}

		ce->priv->root_context = context_new (NULL, main_definition, NULL);
		ce->priv->root_segment = create_segment (ce, NULL, ce->priv->root_context, 0, 0, NULL);

		ce->priv->tags = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

		gtk_text_buffer_get_bounds (buffer, &start, &end);
		ce->priv->invalid_region.start = gtk_text_buffer_create_mark (buffer, NULL,
									      &start, TRUE);
		ce->priv->invalid_region.end = gtk_text_buffer_create_mark (buffer, NULL,
									    &end, FALSE);
		ce->priv->invalid_region.empty = FALSE;
		ce->priv->invalid_region.delta = gtk_text_buffer_get_char_count (buffer);

		g_object_get (ce->priv->buffer, "highlight", &ce->priv->highlight, NULL);
		ce->priv->refresh_region = gtk_text_region_new (buffer);
		ce->priv->highlight_requests = gtk_text_region_new (buffer);

		g_signal_connect_swapped (buffer, "notify::highlight",
					  G_CALLBACK (buffer_notify_highlight_cb), ce);

		install_first_update (ce);
	}
}

static void
set_tag_style_hash_cb (const char             *style,
		       GSList                 *tags,
		       GtkSourceContextEngine *ce)
{
	while (tags)
	{
		set_tag_style (ce, tags->data, style);
		tags = tags->next;
	}
}

static void
gtk_source_context_engine_set_style_scheme (GtkSourceEngine      *engine,
					    GtkSourceStyleScheme *scheme)
{
	GtkSourceContextEngine *ce;

	g_return_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (engine));
	g_return_if_fail (GTK_IS_SOURCE_STYLE_SCHEME (scheme));

	ce = GTK_SOURCE_CONTEXT_ENGINE (engine);

	if (scheme == ce->priv->style_scheme)
		return;

	if (ce->priv->style_scheme)
		g_object_unref (ce->priv->style_scheme);

	ce->priv->style_scheme = g_object_ref (scheme);
	g_hash_table_foreach (ce->priv->tags, (GHFunc) set_tag_style_hash_cb, ce);
}

static void
gtk_source_context_engine_finalize (GObject *object)
{
	GtkSourceContextEngine *ce = GTK_SOURCE_CONTEXT_ENGINE (object);

	if (ce->priv->buffer != NULL)
	{
		g_critical ("finalizing engine with attached buffer");
		/* Disconnect the buffer (if there is one), which destroys almost
		 * everything. */
		gtk_source_context_engine_attach_buffer (GTK_SOURCE_ENGINE (ce), NULL);
	}

	g_assert (!ce->priv->tags);
	g_assert (!ce->priv->root_context);
	g_assert (!ce->priv->root_segment);
	g_assert (!ce->priv->first_update);
	g_assert (!ce->priv->incremental_update);

	g_hash_table_destroy (ce->priv->definitions);
	g_free (ce->priv->id);

	if (ce->priv->style_scheme)
		g_object_unref (ce->priv->style_scheme);

	G_OBJECT_CLASS (_gtk_source_context_engine_parent_class)->finalize (object);
}

static void
_gtk_source_context_engine_class_init (GtkSourceContextEngineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkSourceEngineClass *engine_class = GTK_SOURCE_ENGINE_CLASS (klass);

	object_class->finalize = gtk_source_context_engine_finalize;

	engine_class->attach_buffer = gtk_source_context_engine_attach_buffer;
	engine_class->text_inserted = gtk_source_context_engine_text_inserted;
	engine_class->text_deleted = gtk_source_context_engine_text_deleted;
	engine_class->update_highlight = gtk_source_context_engine_update_highlight;
	engine_class->set_style_scheme = gtk_source_context_engine_set_style_scheme;

	g_type_class_add_private (object_class, sizeof (GtkSourceContextEnginePrivate));
}

static void
_gtk_source_context_engine_init (GtkSourceContextEngine *ce)
{
	ce->priv = G_TYPE_INSTANCE_GET_PRIVATE (ce, GTK_TYPE_SOURCE_CONTEXT_ENGINE,
						GtkSourceContextEnginePrivate);
	ce->priv->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
						       (GDestroyNotify) context_definition_free);
}

GtkSourceContextEngine *
_gtk_source_context_engine_new (GtkSourceLanguage *lang)
{
	GtkSourceContextEngine *ce;

	g_return_val_if_fail (GTK_IS_SOURCE_LANGUAGE (lang), NULL);

	ce = g_object_new (GTK_TYPE_SOURCE_CONTEXT_ENGINE, NULL);
	ce->priv->id = g_strdup (lang->priv->id);
	ce->priv->lang = lang;

	return ce;
}


/* REGEX HANDLING --------------------------------------------------------- */

static Regex *
regex_ref (Regex *regex)
{
	if (regex)
		regex->ref_count++;
	return regex;
}

static void
regex_unref (Regex *regex)
{
	if (regex && --regex->ref_count == 0)
	{
		if (regex->resolved)
			egg_regex_unref (regex->u.regex);
		else
			g_free (regex->u.info.pattern);
		g_free (regex);
	}
}

static gboolean
find_single_byte_escape (const gchar *string)
{
	const char *p = string;

	while ((p = strstr (p, "\\C")))
	{
		const char *slash;
		gboolean found;

		if (p == string)
			return TRUE;

		found = TRUE;
		slash = p - 1;

		while (slash >= string && *slash == '\\')
		{
			found = !found;
			slash--;
		}

		if (found)
			return TRUE;

		p += 2;
	}

	return FALSE;
}

/**
 * regex_new:
 *
 * @pattern: the regular expression.
 * @flags: compile options for @pattern.
 * @error: location to store the error occuring, or NULL to ignore errors.
 *
 * Creates a new regex.
 *
 * Return value: a newly-allocated #Regex.
 */
static Regex *
regex_new (const gchar           *pattern,
	   EggRegexCompileFlags   flags,
	   GError               **error)
{
	Regex *regex;

	if (find_single_byte_escape (pattern))
	{
		g_set_error (error, GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_REGEX,
			     "using \\C is not supported");
		return NULL;
	}

	regex = g_new0 (Regex, 1);
	regex->ref_count = 1;

	if (egg_regex_match_simple (START_REF_REGEX, pattern, 0, 0))
	{
		regex->resolved = FALSE;
		regex->u.info.pattern = g_strdup (pattern);
		regex->u.info.flags = flags;
	}
	else
	{
		regex->resolved = TRUE;
		regex->u.regex = egg_regex_new (pattern, flags, 0, error);

		if (!regex->u.regex)
		{
			g_free (regex);
			regex = NULL;
		}
		else
		{
			egg_regex_optimize (regex->u.regex, NULL);
		}
	}

	return regex;
}

static gint
sub_pattern_to_int (const gchar *name)
{
	guint64 number;
	gchar *end_name;

	if (!*name)
		return -1;

	errno = 0;
	number = g_ascii_strtoull (name, &end_name, 10);

	if (errno !=0 || number > G_MAXINT || *end_name != 0)
		return -1;

	return number;
}

static gboolean
replace_start_regex (const EggRegex *regex,
		     const gchar    *matched_text,
		     GString        *expanded_regex,
		     gpointer        user_data)
{
	gchar *num_string, *subst, *subst_escaped, *escapes;
	gint num;
	struct
	{
		Regex       *start_regex;
		const gchar *matched_text;
	} *data = user_data;

	escapes = egg_regex_fetch (regex, 1, matched_text);
	num_string = egg_regex_fetch (regex, 2, matched_text);
	num = sub_pattern_to_int (num_string);

	if (num < 0)
		subst = egg_regex_fetch_named (data->start_regex->u.regex,
					       data->matched_text,
					       num_string);
	else
		subst = egg_regex_fetch (data->start_regex->u.regex,
					 num,
					 data->matched_text);

	if (subst)
	{
		subst_escaped = egg_regex_escape_string (subst, -1);
	}
	else
	{
		g_warning ("Invalid group: %s", num_string);
		subst_escaped = g_strdup ("");
	}

	g_string_append (expanded_regex, escapes);
	g_string_append (expanded_regex, subst_escaped);

	g_free (escapes);
	g_free (num_string);
	g_free (subst);
	g_free (subst_escaped);

	return FALSE;
}

/**
 * regex_resolve:
 *
 * @regex: a #Regex.
 * @start_regex: a #Regex.
 * @matched_text: the text matched against @start_regex.
 *
 * If the regular expression does not contain references to the start
 * regular expression, the functions increases the reference count
 * of @regex and returns it.
 *
 * If the regular expression contains references to the start regular
 * expression in the form "\%{start_sub_pattern@start}", it replaces
 * them (they are extracted from @start_regex and @matched_text) and
 * returns the new regular expression.
 *
 * Return value: a #Regex.
 */
static Regex *
regex_resolve (Regex       *regex,
	       Regex       *start_regex,
	       const gchar *matched_text)
{
	EggRegex *start_ref;
	gchar *expanded_regex;
	Regex *new_regex;
	struct {
		Regex       *start_regex;
		const gchar *matched_text;
	} data;

	if (!regex || regex->resolved)
		return regex_ref (regex);

	start_ref = egg_regex_new (START_REF_REGEX, 0, 0, NULL);
	data.start_regex = start_regex;
	data.matched_text = matched_text;
	expanded_regex = egg_regex_replace_eval (start_ref,
						 regex->u.info.pattern,
						 -1, 0, 0,
						 replace_start_regex,
						 &data);
	new_regex = regex_new (expanded_regex, regex->u.info.flags, NULL);

	if (!new_regex || !new_regex->resolved)
	{
		regex_unref (new_regex);
		g_warning ("Regular expression %s cannot be expanded.",
			   regex->u.info.pattern);
		/* Returns a regex that nevers matches. */
		new_regex = regex_new ("$never-match^", 0, NULL);
	}

	egg_regex_unref (start_ref);
	return new_regex;
}

static gboolean
regex_match (Regex       *regex,
	     const gchar *line,
	     gint         line_length,
	     gint         line_pos)
{
	gint byte_length = line_length;
	gint byte_pos = line_pos;

	g_assert (regex->resolved);

	if (line_length > 0)
		byte_length = (g_utf8_offset_to_pointer (line, line_length) - line);

	if (line_pos > 0)
		byte_pos = (g_utf8_offset_to_pointer (line, line_pos) - line);

	return egg_regex_match_full (regex->u.regex, line,
				     byte_length, byte_pos,
				     0, NULL);
}

static gchar *
regex_fetch (Regex       *regex,
	     const gchar *line,
	     gint         num)
{
	g_assert (regex->resolved);
	return egg_regex_fetch (regex->u.regex, num, line);
}

static void
regex_fetch_pos (Regex       *regex,
		 const gchar *text,
		 gint         num,
		 gint        *start_pos,
		 gint        *end_pos)
{
	gint byte_start_pos, byte_end_pos;

	g_assert (regex->resolved);

	if (!egg_regex_fetch_pos (regex->u.regex, num, &byte_start_pos, &byte_end_pos))
	{
		if (start_pos)
			*start_pos = -1;
		if (end_pos)
			*end_pos = -1;
	}
	else
	{
		if (start_pos)
			*start_pos = g_utf8_pointer_to_offset (text, text + byte_start_pos);
		if (end_pos)
			*end_pos = g_utf8_pointer_to_offset (text, text + byte_end_pos);
	}
}

static void
regex_fetch_named_pos (Regex       *regex,
		       const gchar *text,
		       const gchar *name,
		       gint        *start_pos,
		       gint        *end_pos)
{
	gint byte_start_pos, byte_end_pos;

	g_assert (regex->resolved);

	if (!egg_regex_fetch_named_pos (regex->u.regex, name, &byte_start_pos, &byte_end_pos))
	{
		if (start_pos)
			*start_pos = -1;
		if (end_pos)
			*end_pos = -1;
	}
	else
	{
		if (start_pos)
			*start_pos = g_utf8_pointer_to_offset (text, text + byte_start_pos);
		if (end_pos)
			*end_pos = g_utf8_pointer_to_offset (text, text + byte_end_pos);
	}
}

static const gchar *
regex_get_pattern (Regex *regex)
{
	g_return_val_if_fail (regex && regex->resolved, "");
	return egg_regex_get_pattern (regex->u.regex);
}

/* SYNTAX TREE ------------------------------------------------------------ */

/**
 * apply_sub_patterns:
 *
 * @contextstate: a #Context.
 * @line_starts_at: beginning offset of the line.
 * @line: the line to analyze.
 * @line_pos: the position inside @line.
 * @line_length: the length of @line.
 * @regex: regex that matched.
 * @where: kind of sub patterns to apply.
 *
 * Applies sub patterns of kind @where to the matched text.
 */
static void
apply_sub_patterns (Segment         *state,
		    LineInfo        *line,
		    Regex           *regex,
		    SubPatternWhere  where)
{
	GSList *sub_pattern_list = state->context->definition->sub_patterns;

	while (sub_pattern_list != NULL)
	{
		SubPatternDefinition *sp_def = sub_pattern_list->data;

		if (sp_def->where == where)
		{
			gint start_pos;
			gint end_pos;

			if (sp_def->is_named)
				regex_fetch_named_pos (regex,
						       line->text,
						       sp_def->u.name,
						       &start_pos,
						       &end_pos);
			else
				regex_fetch_pos (regex,
						 line->text,
						 sp_def->u.num,
						 &start_pos,
						 &end_pos);

			if (start_pos >= 0 && start_pos != end_pos)
			{
				sub_pattern_new (state,
						 line->start_at + start_pos,
						 line->start_at + end_pos,
						 sp_def);
			}
		}

		sub_pattern_list = sub_pattern_list->next;
	}
}

/**
 * apply_match:
 *
 * @state: the current state of the parser.
 * @line_starts_at: beginning offset of the line.
 * @line: the line to analyze.
 * @line_pos: the position inside @line.
 * @line_length: the length of @line.
 * @regex: regex that matched.
 * @where: kind of sub patterns to apply.
 *
 * Moves @line_pos after the matched text. @line_pos is not
 * updated and the function returns %FALSE if the match cannot be
 * applied beacuse an ancestor ends in the middle of the matched
 * text.
 *
 * If the match can be applied the function applies the appropriate
 * sub patterns.
 *
 * Return value: %TRUE if the match can be applied.
 */
static gboolean
can_apply_match (Context  *state,
		 LineInfo *line,
		 gint      match_start,
		 gint     *match_end,
		 Regex    *regex)
{
	gint end_match_pos;
	gboolean ancestor_ends;
	gint pos;

	ancestor_ends = FALSE;
	/* end_match_pos is the position of the end of the matched regex. */
	regex_fetch_pos (regex, line->text, 0, NULL, &end_match_pos);

	/* Verify if an ancestor ends in the matched text. */
	if (ANCESTOR_CAN_END_CONTEXT (state))
	{
		pos = match_start + 1;

		while (pos < end_match_pos)
		{
			if (ancestor_context_ends_here (state, line, pos))
			{
				ancestor_ends = TRUE;
				break;
			}

			pos++;
		}
	}
	else
	{
		pos = end_match_pos;
	}

	if (ancestor_ends)
	{
		/* An ancestor ends in the middle of the match, we verify
		 * if the regex matches against the available string before
		 * the end of the ancestor.
		 * For instance in C a net-address context matches even if
		 * it contains the end of a multi-line comment. */
		/* XXX pos and match_start ?? */
		if (!regex_match (regex, line->text, pos, match_start))
		{
			/* This match is not valid, so we can try to match
			 * the next definition, so the position should not
			 * change. */
			return FALSE;
		}
	}

	*match_end = pos;
	return TRUE;
}

static gboolean
apply_match (Segment         *state,
	     LineInfo        *line,
	     gint            *line_pos,
	     Regex           *regex,
	     SubPatternWhere  where)
{
	gint match_end;

	if (!can_apply_match (state->context, line, *line_pos, &match_end, regex))
		return FALSE;

	segment_extend (state, line->start_at + match_end);
	apply_sub_patterns (state, line, regex, where);
	*line_pos = match_end;
	return TRUE;
}

static Regex *
create_reg_all (Context           *context,
		ContextDefinition *definition)
{
	DefinitionsIter iter;
	ContextDefinition *child_def;
	GString *all;
	Regex *regex;

	g_return_val_if_fail ((!context && definition) || (context && !definition), NULL);

	if (!definition)
		definition = context->definition;

	all = g_string_new ("(");

	/* Closing regex. */
	if (definition->type == CONTEXT_TYPE_CONTAINER &&
	    definition->u.start_end.end != NULL)
	{
		Regex *end;

		if (definition->u.start_end.end->resolved)
		{
			end = definition->u.start_end.end;
		}
		else
		{
			g_return_val_if_fail (context && context->end, NULL);
			end = context->end;
		}

		g_string_append (all, regex_get_pattern (end));
		g_string_append (all, "|");
	}

	/* Ancestors. */
	if (context)
	{
		Context *tmp = context;

		while (ANCESTOR_CAN_END_CONTEXT (tmp))
		{
			if (!tmp->definition->extend_parent)
			{
				g_string_append (all, regex_get_pattern (tmp->parent->end));
				g_string_append (all, "|");
			}

			tmp = tmp->parent;
		}
	}

	/* Children. */
	definition_iter_init (&iter, definition);
	while ((child_def = definition_iter_next (&iter)))
	{
		Regex *child_regex = NULL;

		switch (child_def->type)
		{
			case CONTEXT_TYPE_CONTAINER:
				child_regex = child_def->u.start_end.start;
				break;
			case CONTEXT_TYPE_SIMPLE:
				child_regex = child_def->u.match;
				break;
			default:
				g_return_val_if_reached (NULL);
		}

		if (child_regex)
		{
			g_string_append (all, regex_get_pattern (child_regex));
			g_string_append (all, "|");
		}
	}
	definition_iter_destroy (&iter);

	if (all->len > 1)
		g_string_truncate (all, all->len - 1);
	g_string_append (all, ")");

	if (!(regex = regex_new (all->str, 0, NULL)))
	{
		/* regex_new could fail, for instance if there are different
		 * named sub-patterns with the same name. */
		g_warning ("Cannot create a regex for all the transitions, "
			   "the syntax highlighting process will be slower "
			   "than usual.");
	}

	g_string_free (all, TRUE);
	return regex;
}

static Context *
context_ref (Context *context)
{
	if (context)
		context->ref_count++;
	return context;
}

static Context *
context_new (Context           *parent,
	     ContextDefinition *definition,
	     const gchar       *line_text)
{
	Context *context;

	context = g_new0 (Context, 1);
	context->ref_count = 1;
	context->definition = definition;
	context->parent = parent;

	if (!parent || (parent->all_ancestors_extend &&
	    parent->definition->extend_parent))
	{
		context->all_ancestors_extend = TRUE;
	}

	if (line_text &&
	    definition->type == CONTEXT_TYPE_CONTAINER &&
	    definition->u.start_end.end)
	{
		context->end = regex_resolve (definition->u.start_end.end,
					      definition->u.start_end.start,
					      line_text);
	}

	/* Create reg_all. If it is possibile we share the same reg_all
	 * for more contexts storing it in the definition. */
	if (ANCESTOR_CAN_END_CONTEXT (context) ||
	    (definition->type == CONTEXT_TYPE_CONTAINER &&
	     definition->u.start_end.end &&
	     !definition->u.start_end.end->resolved))
	{
		context->reg_all = create_reg_all (context, NULL);
	}
	else
	{
		if (!definition->reg_all)
			definition->reg_all = create_reg_all (NULL, definition);
		context->reg_all = regex_ref (definition->reg_all);
	}

#ifdef ENABLE_DEBUG
	{
		GString *str = g_string_new (definition->id);
		Context *tmp = context->parent;
		while (tmp)
		{
			g_string_prepend (str, "/");
			g_string_prepend (str, tmp->definition->id);
			tmp = tmp->parent;
		}
		g_print ("created context %s: %s\n", definition->id, str->str);
		g_string_free (str, TRUE);
	}
#endif

	return context;
}

static void
context_unref_hash_cb (G_GNUC_UNUSED gpointer text,
		       Context *context)
{
	context->parent = NULL;
	context_unref (context);
}

static gboolean
remove_context_cb (G_GNUC_UNUSED gpointer text,
		   Context *context,
		   Context *target)
{
	return context == target;
}

static void
context_remove_child (Context *parent,
		      Context *context)
{
	ContextPtr *ptr, *prev = NULL;
	gboolean delete = TRUE;

	g_assert (context->parent == parent);

	for (ptr = context->parent->children; ptr; ptr = ptr->next)
	{
		if (ptr->definition == context->definition)
			break;
		prev = ptr;
	}

	g_assert (ptr != NULL);

	if (!ptr->fixed)
	{
		g_hash_table_foreach_remove (ptr->u.hash,
					     (GHRFunc) remove_context_cb,
					     context);

		if (g_hash_table_size (ptr->u.hash))
			delete = FALSE;
	}

	if (delete)
	{
		if (prev)
			prev->next = ptr->next;
		else
			context->parent->children = ptr->next;

		if (!ptr->fixed)
			g_hash_table_destroy (ptr->u.hash);

		g_free (ptr);
	}
}

/**
 * context_unref:
 *
 * @context: the context.
 *
 * Decreases reference count and removes @context
 * from the tree when it drops to zero.
 */
static void
context_unref (Context *context)
{
	ContextPtr *children;

	if (!context || --context->ref_count)
		return;

	DEBUG (g_print ("destroying context %s\n", context->definition->id));

	children = context->children;
	context->children = NULL;

	while (children)
	{
		ContextPtr *ptr = children;

		children = children->next;

		if (ptr->fixed)
		{
			ptr->u.context->parent = NULL;
			context_unref (ptr->u.context);
		}
		else
		{
			g_hash_table_foreach (ptr->u.hash,
					      (GHFunc) context_unref_hash_cb,
					      NULL);
			g_hash_table_destroy (ptr->u.hash);
		}

		g_free (ptr);
	}

	if (context->parent)
		context_remove_child (context->parent, context);

	regex_unref (context->end);
	regex_unref (context->reg_all);
	g_free (context->subpattern_tags);
	g_free (context);
}

static void
context_freeze_hash_cb (G_GNUC_UNUSED gpointer text,
		        Context *context)
{
	context_freeze (context);
}

static void
context_freeze (Context *ctx)
{
	ContextPtr *ptr;

	g_assert (!ctx->frozen);
	ctx->frozen = TRUE;
	context_ref (ctx);

	for (ptr = ctx->children; ptr != NULL; ptr = ptr->next)
	{
		if (ptr->fixed)
		{
			context_freeze (ptr->u.context);
		}
		else
		{
			g_hash_table_foreach (ptr->u.hash,
					      (GHFunc) context_freeze_hash_cb,
					      NULL);
		}
	}
}

static void
get_child_contexts_hash_cb (G_GNUC_UNUSED gpointer text,
			    Context *context,
			    GSList **list)
{
	*list = g_slist_prepend (*list, context);
}

static void
context_thaw (Context *ctx)
{
	ContextPtr *ptr;

	if (!ctx->frozen)
		return;

	for (ptr = ctx->children; ptr != NULL; )
	{
		ContextPtr *next = ptr->next;

		if (ptr->fixed)
		{
			context_thaw (ptr->u.context);
		}
		else
		{
			GSList *children = NULL;
			g_hash_table_foreach (ptr->u.hash,
					      (GHFunc) get_child_contexts_hash_cb,
					      &children);
			g_slist_foreach (children, (GFunc) context_thaw, NULL);
			g_slist_free (children);
		}

		ptr = next;
	}

	ctx->frozen = FALSE;
	context_unref (ctx);
}

static Context *
create_child_context (Context           *parent,
		      ContextDefinition *definition,
		      const gchar       *line_text)
{
	Context *context;
	ContextPtr *ptr;
	gchar *match = NULL;

	g_return_val_if_fail (parent != NULL, NULL);

	for (ptr = parent->children;
	     ptr != NULL && ptr->definition != definition;
	     ptr = ptr->next) ;

	if (!ptr)
	{
		ptr = g_new0 (ContextPtr, 1);
		ptr->next = parent->children;
		parent->children = ptr;
		ptr->definition = definition;

		if (definition->type != CONTEXT_TYPE_CONTAINER ||
		    !definition->u.start_end.end ||
		    definition->u.start_end.end->resolved)
		{
			ptr->fixed = TRUE;
		}

		if (!ptr->fixed)
			ptr->u.hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	}

	if (ptr->fixed)
	{
		context = ptr->u.context;
	}
	else
	{
		match = regex_fetch (definition->u.start_end.start, line_text, 0);
		g_return_val_if_fail (match != NULL, NULL);
		context = g_hash_table_lookup (ptr->u.hash, match);
	}

	if (context)
	{
		g_free (match);
		return context_ref (context);
	}

	context = context_new (parent, definition, line_text);
	g_return_val_if_fail (context != NULL, NULL);

	if (ptr->fixed)
		ptr->u.context = context;
	else
		g_hash_table_insert (ptr->u.hash, match, context);

	return context;
};

static Segment *
segment_new (GtkSourceContextEngine *ce,
	     Segment                *parent,
	     Context                *context,
	     gint                    start_at,
	     gint                    end_at)
{
	Segment *segment;

	segment = g_new0 (Segment, 1);
	segment->parent = parent;
	segment->context = context_ref (context);
	segment->start_at = start_at;
	segment->end_at = end_at;

	if (!context)
		add_invalid (ce, segment);

	return segment;
}

static void
find_segment_position_forward_ (Segment  *segment,
				gint      start_at,
				gint      end_at,
				Segment **prev,
				Segment **next)
{
	g_assert (segment->start_at <= start_at);

	while (segment)
	{
		if (segment->end_at == start_at)
		{
			while (segment->next && segment->next->start_at == start_at)
				segment = segment->next;

			*prev = segment;
			*next = segment->next;

			break;
		}

		if (segment->start_at == end_at)
		{
			*next = segment;
			*prev = segment->prev;
			break;
		}

		if (segment->start_at > end_at)
		{
			*next = segment;
			break;
		}

		if (segment->end_at < start_at)
			*prev = segment;

		segment = segment->next;
	}
}

static void
find_segment_position_backward_ (Segment  *segment,
				 gint      start_at,
				 gint      end_at,
				 Segment **prev,
				 Segment **next)
{
	g_assert (start_at < segment->end_at);

	while (segment)
	{
		if (segment->end_at <= start_at)
		{
			*prev = segment;
			break;
		}

		g_assert (segment->start_at >= end_at);

		*next = segment;
		segment = segment->prev;
	}
}

static void
find_segment_position (Segment  *parent,
		       Segment  *hint,
		       gint      start_at,
		       gint      end_at,
		       Segment **prev,
		       Segment **next)
{
	Segment *tmp;

	g_assert (parent->start_at <= start_at && end_at <= parent->end_at);
	g_assert (!hint || hint->parent == parent);

	*prev = *next = NULL;

	if (!parent->children)
		return;

	if (!parent->children->next)
	{
		tmp = parent->children;

		if (start_at >= tmp->end_at)
			*prev = tmp;
		else
			*next = tmp;

		return;
	}

	if (!hint)
		hint = parent->children;

	if (hint->end_at <= start_at)
		return find_segment_position_forward_ (hint, start_at, end_at, prev, next);
	else
		return find_segment_position_backward_ (hint, start_at, end_at, prev, next);
}

static Segment *
create_segment (GtkSourceContextEngine *ce,
		Segment                *parent,
		Context                *context,
		gint                    start_at,
		gint                    end_at,
		Segment                *hint)
{
	Segment *segment;

	g_assert (!parent || (parent->start_at <= start_at && end_at <= parent->end_at));

	segment = segment_new (ce, parent, context, start_at, end_at);

	if (parent)
	{
		Segment *prev, *next;

		if (!hint)
		{
			hint = ce->priv->hint;
			while (hint && hint->parent != parent)
				hint = hint->parent;
		}

		find_segment_position (parent, hint,
				       start_at, end_at,
				       &prev, &next);

		g_assert ((!parent->children && !prev && !next) ||
			  (parent->children && (prev || next)));
		g_assert (!prev || prev->next == next);
		g_assert (!next || next->prev == prev);

		segment->next = next;
		segment->prev = prev;

		if (next)
			next->prev = segment;
		else
			parent->last_child = segment;

		if (prev)
			prev->next = segment;
		else
			parent->children = segment;

		CHECK_SEGMENT_LIST (parent);
	}

	return segment;
}

static void
segment_extend (Segment *state,
		gint     end_at)
{
	while (state && state->end_at < end_at)
	{
		state->end_at = end_at;
		state = state->parent;
	}
	CHECK_SEGMENT_LIST (state->parent);
}

static void
segment_destroy_children (GtkSourceContextEngine *ce,
			  Segment                *segment)
{
	Segment *children;
	SubPattern *sp;

	g_assert (segment != NULL);

	children = segment->children;
	segment->children = NULL;
	segment->last_child = NULL;

	while (children)
	{
		Segment *tmp = children;
		children = children->next;
		segment_destroy (ce, tmp);
	}

	for (sp = segment->sub_patterns; sp; sp = sp->next)
		sub_pattern_free (sp);

	segment->sub_patterns = NULL;
}

static void
segment_destroy (GtkSourceContextEngine *ce,
		 Segment                *segment)
{
	g_assert (segment != NULL);

	segment_destroy_children (ce, segment);

	/* segment neighbours and parent may be invalid here,
	 * so we only can unset the hint */
	if (ce->priv->hint == segment)
		ce->priv->hint = NULL;

	if (SEGMENT_IS_INVALID (segment))
		remove_invalid (ce, segment);

	context_unref (segment->context);

#ifdef ENABLE_DEBUG
	g_assert (!g_slist_find (ce->priv->invalid, segment));
	memset (segment, 0, sizeof (Segment));
#else
	g_free (segment);
#endif
}

static gboolean
container_context_starts_here (GtkSourceContextEngine  *ce,
			       Segment                 *state,
			       ContextDefinition       *definition,
			       LineInfo                *line,
			       gint                    *line_pos,
			       Segment                **new_state,
			       Segment                **hint)
{
	Context *new_context;
	Segment *new_segment;
	gint match_end;

	/* We can have a container context definition (i.e. the main
	 * language definition) without start_end.start. */
	if (definition->u.start_end.start == NULL)
		return FALSE;

	if (!regex_match (definition->u.start_end.start,
			  line->text, line->length,
			  *line_pos))
	{
		return FALSE;
	}

	new_context = create_child_context (state->context, definition, line->text);
	g_return_val_if_fail (new_context != NULL, FALSE);

	if (!can_apply_match (new_context, line, *line_pos, &match_end,
			      definition->u.start_end.start))
	{
		context_unref (new_context);
		return FALSE;
	}

        segment_extend (state, line->start_at + match_end);
        new_segment = create_segment (ce, state, new_context,
				      line->start_at + *line_pos,
				      line->start_at + match_end,
				      *hint);
	apply_sub_patterns (new_segment, line,
			    definition->u.start_end.start,
			    SUB_PATTERN_WHERE_START);
	*line_pos = match_end;
	*new_state = new_segment;
	*hint = NULL;
	context_unref (new_context);
	return TRUE;
}

static gboolean
simple_context_starts_here (GtkSourceContextEngine *ce,
			    Segment                *state,
			    ContextDefinition      *definition,
			    LineInfo               *line,
			    gint                   *line_pos,
			    Segment               **new_state,
			    Segment               **hint)
{
	gint match_end;
	Context *new_context;
	Segment *new_segment;

	g_return_val_if_fail (definition->u.match != NULL, FALSE);

	if (!regex_match (definition->u.match, line->text, line->length, *line_pos))
		return FALSE;

	new_context = create_child_context (state->context, definition, line->text);
	g_return_val_if_fail (new_context != NULL, FALSE);

	if (!can_apply_match (new_context, line, *line_pos, &match_end, definition->u.match))
	{
		context_unref (new_context);
		return FALSE;
	}

        segment_extend (state, line->start_at + match_end);
        new_segment = create_segment (ce, state, new_context,
				      line->start_at + *line_pos,
				      line->start_at + match_end,
				      *hint);
	apply_sub_patterns (new_segment, line, definition->u.match, SUB_PATTERN_WHERE_DEFAULT);
	*line_pos = match_end;
	*new_state = state;
	*hint = new_segment;
	context_unref (new_context);
	return TRUE;
}

/**
 * child_starts_here:
 *
 * @state: the current state.
 * @curr_definition: child #ContextDefinition.
 * @line: the line to analyze.
 * @line_pos: the position inside @line.
 * @line_length: the length of @line.
 * @new_state: where to store the new state.
 *
 * Verifies if a context of the type in @curr_definition starts at
 * @line_pos in @line. If the contexts start here @new_state and
 * @line_pos are updated.
 *
 * Return value: %TRUE if the context starts here.
 */
static gboolean
child_starts_here (GtkSourceContextEngine *ce,
		   Segment                *state,
		   ContextDefinition      *definition,
		   LineInfo               *line,
		   gint                   *line_pos,
		   Segment               **new_state,
		   Segment               **hint)
{
	switch (definition->type)
	{
		case CONTEXT_TYPE_SIMPLE:
			return simple_context_starts_here (ce,
							   state,
							   definition,
							   line,
							   line_pos,
							   new_state,
							   hint);
		case CONTEXT_TYPE_CONTAINER:
			return container_context_starts_here (ce,
							      state,
							      definition,
							      line,
							      line_pos,
							      new_state,
							      hint);
		default:
			g_return_val_if_reached (FALSE);
	}
}

static gboolean
segment_ends_here (Segment  *state,
		   LineInfo *line,
		   gint      pos)
{
	g_assert (SEGMENT_IS_CONTAINER (state));
	return state->context->definition->u.start_end.end &&
		regex_match (state->context->end,
			     line->text,
			     line->length,
			     pos);
}

/**
 * ancestor_ends_here:
 *
 * @state: current state.
 * @line: the line to analyze.
 * @line_pos: the position inside @line.
 * @line_length: the length of @line.
 * @new_state: where to store the new state.
 *
 * Verifies if an ancestor context ends at the current position. If
 * state changed and @new_state is not NULL, then the new state is stored
 * in @new_state, and descendants of @new_state are closed, so the
 * terminating segment becomes current state.
 *
 * Return value: %TRUE if an ancestor ends at the given position.
 */
static Context *
ancestor_context_ends_here (Context                *state,
			    LineInfo               *line,
			    gint                    line_pos)
{
	Context *current_context;
	GSList *current_context_list;
	GSList *check_ancestors;
	Context *terminating_context;

	/* A context can be terminated by the parent if extend_parent is
	 * FALSE, so we need to verify the end of all the parents of
	 * not-extending contexts. The list is ordered by ascending
	 * depth. */
	check_ancestors = NULL;
	current_context = state;
	while (ANCESTOR_CAN_END_CONTEXT (current_context))
	{
		if (!current_context->definition->extend_parent)
			check_ancestors = g_slist_prepend (check_ancestors,
							   current_context->parent);
		current_context = current_context->parent;
	}

	/* The first context that ends here terminates its descendants. */
	terminating_context = NULL;
	current_context_list = check_ancestors;
	while (current_context_list)
	{
		current_context = current_context_list->data;

		if (current_context->end &&
		    current_context->end->u.regex &&
		    regex_match (current_context->end,
				 line->text,
				 line->length,
				 line_pos))
		{
			terminating_context = current_context;
			break;
		}

		current_context_list = current_context_list->next;
	}
	g_slist_free (check_ancestors);

	return terminating_context;
}

static gboolean
ancestor_ends_here (Segment                *state,
		    LineInfo               *line,
		    gint                    line_pos,
		    Segment               **new_state)
{
	Context *terminating_context;

	terminating_context = ancestor_context_ends_here (state->context, line, line_pos);

	if (new_state && terminating_context)
	{
		/* We have found a context that ends here, so we close
		 * all the descendants. terminating_segment will be
		 * closed by next next_segment() call from analyze_line. */
		Segment *current_segment = state;

		while (current_segment->context != terminating_context)
		{
			g_assert (current_segment->end_at >= line->start_at + line_pos);
			current_segment = current_segment->parent;
		}

		*new_state = current_segment;
		g_assert (*new_state != NULL);
	}

	return terminating_context != NULL;
}

/**
 * next_segment:
 *
 * @ce: #GtkSourceContextEngine.
 * @state: current state.
 * @line: analyzed line.
 * @line_pos: position inside @line.
 * @new_state: where to store the new state.
 * @hint: child of @state used to optimize tree operations.
 *
 * Verifies if a context starts or ends in @line at @line_pos of after it.
 * If the contexts starts or ends here @new_state and @line_pos are updated.
 *
 * Return value: %FALSE is there are no more contexts in @line.
 */
static gboolean
next_segment (GtkSourceContextEngine  *ce,
	      Segment                 *state,
	      LineInfo                *line,
	      gint                    *line_pos,
	      Segment                **new_state,
	      Segment                **hint)
{
	gint pos = *line_pos;

	g_assert (!(*hint) || (*hint)->parent == state);

	while (pos <= line->length)
	{
		DefinitionsIter def_iter;
		gboolean context_end_found;
		ContextDefinition *child_def;

		if (state->context->reg_all)
		{
			if (!regex_match (state->context->reg_all,
					  line->text,
					  line->length,
					  pos))
			{
				return FALSE;
			}

			regex_fetch_pos (state->context->reg_all,
					 line->text, 0, &pos, NULL);
		}

		/* Does an ancestor end here? */
		if (ANCESTOR_CAN_END_CONTEXT (state->context) &&
		    ancestor_ends_here (state, line, pos, new_state))
		{
			segment_extend (state, line->start_at + pos);
			*line_pos = pos;
			return TRUE;
		}

		/* Does the current context end here? */
		context_end_found = segment_ends_here (state, line, pos);

		/* Iter over the definitions we can find in the current
		 * context. */
		definition_iter_init (&def_iter, state->context->definition);
		while ((child_def = definition_iter_next (&def_iter)))
		{
			gboolean try_this = TRUE;

			/* If the child definition does not extend the parent
			 * and the current context could end here we do not
			 * need to examine this child. */
			if (!child_def->extend_parent && context_end_found)
				try_this = FALSE;

			if (child_def->first_line_only && line->start_at != 0)
				try_this = FALSE;

			if (try_this)
			{
				/* Does this child definition start a new
				 * context at the current position? */
				if (child_starts_here (ce, state, child_def,
						       line, &pos, new_state,
						       hint))
				{
					*line_pos = pos;
					definition_iter_destroy (&def_iter);
					return TRUE;
				}
			}

			/* This child does not start here, so we analyze
			 * another definition. */
		}
		definition_iter_destroy (&def_iter);

		if (context_end_found)
		{
			/* We have found that the current context could end
			 * here and that it cannot be extended by a child.
			 * Still, it may happen that parent context ends in
			 * the middle of the end regex match, apply_match()
			 * checks this. */
			if (apply_match (state, line, &pos,
					 state->context->end,
					 SUB_PATTERN_WHERE_END))
			{
				g_assert (state->end_at >= line->start_at + pos);
				/* FIXME: if child may terminate parent */
				*new_state = state->parent;
				*hint = state;
				*line_pos = pos;
				return TRUE;
			}
		}

		/* Nothing new at this position, go to next char. */
		pos++;
	}

	return FALSE;
}

/**
 * check_line_end:
 *
 * @state: current state.
 * @line: analyzed line.
 * @hint: child of @state used in analyze_line() and next_segment().
 *
 * Closes the contexts that cannot contain end of lines if needed.
 * Updates hint if new state is different from @state.
 *
 * Return value: the new state.
 */
static Segment *
check_line_end (Segment   *state,
		LineInfo  *line,
		Segment  **hint)
{
	Segment *current_segment;
	Segment *terminating_segment;

	g_assert (!(*hint) || (*hint)->parent == state);

	/* A context can be terminated by the parent if extend_parent is
	 * FALSE, so we need to verify the end of all the parents of
	 * not-extending contexts. */
	terminating_segment = NULL;
	current_segment = state;

	do
	{
		if (current_segment->context->definition->end_at_line_end)
			terminating_segment = current_segment;
		current_segment = current_segment->parent;
	}
	while (ANCESTOR_CAN_END_CONTEXT (current_segment->context));

	if (terminating_segment)
	{
		/* We have found a context that ends here, so we close
		 * it and its descendants. */
		current_segment = state;

		do
		{
			g_assert (current_segment->end_at >= line->length);
			current_segment = current_segment->parent;
		}
		while (current_segment != terminating_segment->parent);

		*hint = terminating_segment;
		return terminating_segment->parent;
	}
	else
	{
		return state;
	}
}

/**
 * analyze_line:
 *
 * @ce: #GtkSourceContextEngine.
 * @state: the state at the beginning of line.
 * @line: the line.
 * @hint: a child of @state around start of line, to make it faster.
 *
 * Finds contexts at the line and updates the syntax tree on it.
 *
 * Return value: starting state at the next line.
 */
static Segment *
analyze_line (GtkSourceContextEngine *ce,
	      Segment                *state,
	      LineInfo               *line,
	      Segment               **hint)
{
	gint line_pos = 0;

	g_assert (SEGMENT_IS_CONTAINER (state));
	g_assert (!(*hint) || (*hint)->parent == state);

	/* Find the contexts in the line. */
	while (line_pos <= line->length)
	{
		Segment *new_state = NULL;

		if (!next_segment (ce, state, line, &line_pos, &new_state, hint))
			break;

		g_assert (new_state != NULL);
		state = new_state;
		g_assert (SEGMENT_IS_CONTAINER (state));
	}

	/* Extend current state to the end of line. */
	segment_extend (state, line->start_at + line->length);
	g_assert (line_pos <= line->length);

	/* Verify if we need to close the context because we are at
	 * the end of the line. */
	if (ANCESTOR_CAN_END_CONTEXT (state->context) ||
	    state->context->definition->end_at_line_end)
	{
		state = check_line_end (state, line, hint);
	}

	/* Extend the segment to the beginning of next line. */
	g_assert (SEGMENT_IS_CONTAINER (state));
	segment_extend (state, NEXT_LINE_OFFSET (line));
	return state;
}

/**
 * get_line_info:
 *
 * @buffer: #GtkTextBuffer.
 * @line_start: iterator pointing to the beginning of line.
 * @line_start: iterator pointing to the beginning of next line or to the end
 * of this line if it's the last line in @buffer.
 * @line: #LineInfo structure to be filled.
 *
 * Retrieves line text from the buffer, finds line terminator and fills
 * @line structure.
 */
static void
get_line_info (GtkTextBuffer     *buffer,
	       const GtkTextIter *line_start,
	       const GtkTextIter *line_end,
	       LineInfo          *line)
{
	g_assert (!gtk_text_iter_equal (line_start, line_end));

	line->text = gtk_text_buffer_get_slice (buffer, line_start, line_end, TRUE);
	line->start_at = gtk_text_iter_get_offset (line_start);

	if (!gtk_text_iter_starts_line (line_end))
	{
		line->eol_length = 0;
		line->length = g_utf8_strlen (line->text, -1);
	}
	else
	{
		gint eol_index, next_line_index;

		pango_find_paragraph_boundary (line->text, -1,
					       &eol_index,
					       &next_line_index);

		g_assert (eol_index < next_line_index);

		line->length = g_utf8_strlen (line->text, eol_index);
		line->eol_length = g_utf8_strlen (line->text + eol_index, -1);
	}

	g_assert (gtk_text_iter_get_offset (line_end) ==
			line->start_at + line->length + line->eol_length);
}

/**
 * line_info_destroy:
 *
 * @line: #LineInfo.
 *
 * Destroys data allocated by get_line_info().
 */
static void
line_info_destroy (LineInfo *line)
{
	g_free (line->text);
}

/**
 * segment_tree_zero_len:
 *
 * @ce: #GtkSoucreContextEngine.
 *
 * Erases syntax tree and sets root segment length to zero.
 * It's a shortcut for case when all the text is deleted from
 * the buffer.
 */
static void
segment_tree_zero_len (GtkSourceContextEngine *ce)
{
	Segment *root = ce->priv->root_segment;
	segment_destroy_children (ce, root);
	root->start_at = root->end_at = 0;
	CHECK_TREE (ce);
}

#ifdef ENABLE_CHECK_TREE
static Segment *
get_segment_at_offset_slow_ (Segment *segment,
			     gint     offset)
{
	Segment *child;

start:
	if (!segment->parent && offset == segment->end_at)
		return segment;

	if (segment->start_at > offset)
	{
		g_assert (segment->parent != NULL);
		segment = segment->parent;
		goto start;
	}

	if (segment->start_at == offset)
	{
		if (segment->children && segment->children->start_at == offset)
		{
			segment = segment->children;
			goto start;
		}

		return segment;
	}

        if (segment->end_at <= offset && segment->parent)
	{
		if (segment->next)
		{
			if (segment->next->start_at > offset)
				return segment->parent;

			segment = segment->next;
		}
		else
		{
			segment = segment->parent;
		}

		goto start;
	}

	for (child = segment->children; child != NULL; child = child->next)
	{
		if (child->start_at == offset)
		{
			segment = child;
			goto start;
		}

		if (child->end_at <= offset)
			continue;

		if (child->start_at > offset)
			break;

		segment = child;
		goto start;
	}

	return segment;
}
#endif /* ENABLE_CHECK_TREE */

#define SEGMENT_IS_ZERO_LEN_AT(s,o) ((s)->start_at == (o) && (s)->end_at == (o))
#define SEGMENT_CONTAINS(s,o) ((s)->start_at <= (o) && (s)->end_at > (o))
#define SEGMENT_DISTANCE(s,o) (MIN (ABS ((s)->start_at - (o)), ABS ((s)->end_at - (o))))
static Segment *
get_segment_in_ (Segment *segment,
		 gint     offset)
{
	Segment *child;

	g_assert (segment->start_at <= offset && segment->end_at > offset);

	if (!segment->children)
		return segment;

	if (segment->children == segment->last_child)
	{
		if (SEGMENT_IS_ZERO_LEN_AT (segment->children, offset))
			return segment->children;

		if (SEGMENT_CONTAINS (segment->children, offset))
			return get_segment_in_ (segment->children, offset);

		return segment;
	}

	if (segment->children->start_at > offset || segment->last_child->end_at < offset)
		return segment;

	if (SEGMENT_DISTANCE (segment->children, offset) >= SEGMENT_DISTANCE (segment->last_child, offset))
	{
		for (child = segment->children; child; child = child->next)
		{
			if (child->start_at > offset)
				return segment;

			if (SEGMENT_IS_ZERO_LEN_AT (child, offset))
				return child;

			if (SEGMENT_CONTAINS (child, offset))
				return get_segment_in_ (child, offset);
		}
	}
	else
	{
		for (child = segment->last_child; child; child = child->prev)
		{
			if (SEGMENT_IS_ZERO_LEN_AT (child, offset))
			{
				while (child->prev && SEGMENT_IS_ZERO_LEN_AT (child->prev, offset))
					child = child->prev;
				return child;
			}

			if (child->end_at <= offset)
				return segment;

			if (SEGMENT_CONTAINS (child, offset))
				return get_segment_in_ (child, offset);
		}
	}

	return segment;
}

/* assumes zero-length segments can't have children */
static Segment *
get_segment_ (Segment *segment,
	      gint     offset)
{
	if (segment->parent)
	{
		if (!SEGMENT_CONTAINS (segment->parent, offset))
			return get_segment_ (segment->parent, offset);
	}
	else
	{
		g_assert (offset >= segment->start_at);
		g_assert (offset <= segment->end_at);
	}

	if (SEGMENT_CONTAINS (segment, offset))
		return get_segment_in_ (segment, offset);

	if (SEGMENT_IS_ZERO_LEN_AT (segment, offset))
	{
		while (segment->prev && SEGMENT_IS_ZERO_LEN_AT (segment->prev, offset))
			segment = segment->prev;
		return segment;
	}

	if (offset < segment->start_at)
	{
		while (segment->prev && segment->prev->start_at > offset)
			segment = segment->prev;

		g_assert (!segment->prev || segment->prev->start_at <= offset);

		if (!segment->prev)
			return segment->parent;

		if (segment->prev->end_at > offset)
			return get_segment_in_ (segment->prev, offset);

		if (segment->prev->end_at == offset)
		{
			if (SEGMENT_IS_ZERO_LEN_AT (segment->prev, offset))
			{
				segment = segment->prev;
				while (segment->prev && SEGMENT_IS_ZERO_LEN_AT (segment->prev, offset))
					segment = segment->prev;
				return segment;
			}

			return segment->parent;
		}

		/* segment->prev->end_at < offset */
		return segment->parent;
	}

	/* offset >= segment->end_at, not zero-length */

	while (segment->next)
	{
		if (SEGMENT_IS_ZERO_LEN_AT (segment->next, offset))
			return segment->next;

		if (segment->next->end_at > offset)
		{
			if (segment->next->start_at <= offset)
				return get_segment_in_ (segment->next, offset);
			else
				return segment->parent;
		}

		segment = segment->next;
	}

	return segment->parent;
}
#undef SEGMENT_IS_ZERO_LEN_AT
#undef SEGMENT_CONTAINS
#undef SEGMENT_DISTANCE

/**
 * get_segment_at_offset:
 *
 * @ce: #GtkSoucreContextEngine.
 * @hint: segment to start search from or NULL.
 * @offset: the offset.
 *
 * Finds the deepest segment "at @offset".
 * More precisely, it returns toplevel segment if
 * @offset is equal to length of buffer; or non-zero-length
 * segment which contains character at @offset; or zero-length
 * segment at @offset. In case when there are several zero-length
 * segments, it returns the first one.
 */
static Segment *
get_segment_at_offset (GtkSourceContextEngine *ce,
		       Segment                *hint,
		       gint                    offset)
{
	Segment *result;

	if (offset == ce->priv->root_segment->end_at)
		return ce->priv->root_segment;

	if (!hint || hint == ce->priv->root_segment)
	{
		static int c;
		g_print ("searching from root %d\n", ++c);
	}

	result = get_segment_ (hint ? hint : ce->priv->root_segment, offset);

#ifdef ENABLE_CHECK_TREE
	g_assert (result == get_segment_at_offset_slow_ (hint, offset));
#endif

	return result;
}

/**
 * segment_remove:
 *
 * @ce: #GtkSoucreContextEngine.
 * @segment: segment remove.
 *
 * Removes the segment from syntax tree and frees it.
 * It correctly updates parent's children list, not
 * like segment_destroy() where caller has to take care
 * of tree integrity.
 */
static void
segment_remove (GtkSourceContextEngine *ce,
		Segment                *segment)
{
	if (segment->next)
		segment->next->prev = segment->prev;
	else
		segment->parent->last_child = segment->prev;

	if (segment->prev)
		segment->prev->next = segment->next;
	else
		segment->parent->children = segment->next;

	/* if ce->priv->hint is being deleted, set it to some
	 * neighbour segment */
	if (ce->priv->hint == segment)
	{
		if (segment->next)
			ce->priv->hint = segment->next;
		else if (segment->prev)
			ce->priv->hint = segment->prev;
		else
			ce->priv->hint = segment->parent;
	}

	segment_destroy (ce, segment);
}

static void
segment_erase_middle_ (GtkSourceContextEngine *ce,
		       Segment                *segment,
		       gint                    start,
		       gint                    end)
{
	Segment *new_segment, *children, *child;
	SubPattern *sub_patterns, *sp;

	new_segment = segment_new (ce,
				   segment->parent,
				   segment->context,
				   end,
				   segment->end_at);
	segment->end_at = start;

	new_segment->next = segment->next;
	segment->next = new_segment;
	new_segment->prev = segment;

	if (new_segment->next)
		new_segment->next->prev = new_segment;
	else
		new_segment->parent->last_child = new_segment;

	children = segment->children;
	segment->children = NULL;
	segment->last_child = NULL;

	for (child = children; child != NULL; )
	{
		Segment *append_to;
		Segment *next = child->next;

		if (child->start_at < start)
		{
			g_assert (child->end_at <= start);
			append_to = segment;
		}
		else
		{
			g_assert (child->start_at >= end);
			append_to = new_segment;
		}

		child->parent = append_to;

		if (append_to->last_child)
		{
			append_to->last_child->next = child;
			child->prev = append_to->last_child;
			child->next = NULL;
			append_to->last_child = child;
		}
		else
		{
			child->next = child->prev = NULL;
			append_to->last_child = child;
			append_to->children = child;
		}

		child = next;
	}

	sub_patterns = segment->sub_patterns;
	segment->sub_patterns = NULL;

	for (sp = sub_patterns; sp != NULL; )
	{
		SubPattern *next = sp->next;
		Segment *append_to;

		if (sp->start_at < start)
		{
			sp->end_at = MIN (sp->end_at, start);
			append_to = segment;
		}
		else
		{
			g_assert (sp->end_at > end);
			sp->start_at = MAX (sp->start_at, end);
			append_to = new_segment;
		}

		sp->next = append_to->sub_patterns;
		append_to->sub_patterns = sp;
		sp = next;
	}

	CHECK_SEGMENT_CHILDREN (segment);
	CHECK_SEGMENT_CHILDREN (new_segment);
}

/**
 * segment_erase_range_:
 *
 * @ce: #GtkSourceContextEngine.
 * @segment: the segment.
 * @start: start offset of range to erase.
 * @end: end offset of range to erase.
 *
 * Recurisvely removes segments from [@start, @end] interval
 * starting from @segment. If @segment belongs to the range,
 * or it's a zero-length segment at @end offset, and it's not
 * the toplevel segment, then it's removed from the tree.
 * If @segment intersects with the range (unless it's the toplevel
 * segment), then its ends are adjusted appropriately, and it's
 * split into two if it completely contains the range.
 */
static void
segment_erase_range_ (GtkSourceContextEngine *ce,
		      Segment                *segment,
		      gint                    start,
		      gint                    end)
{
	Segment *child;

	g_assert (start < end);

	if (segment->start_at == segment->end_at)
	{
		if (segment->start_at >= start && segment->start_at <= end)
			segment_remove (ce, segment);
		return;
	}

	if (segment->start_at > end || segment->end_at < start)
		return;

	if (segment->start_at >= start && segment->end_at <= end && segment->parent)
	{
		segment_remove (ce, segment);
		return;
	}

	if (segment->start_at == end)
	{
		for (child = segment->children; child != NULL && child->start_at == end; )
		{
			Segment *next = child->next;
			segment_erase_range_ (ce, child, start, end);
			child = next;
		}
	}
	else if (segment->end_at == start)
	{
		for (child = segment->last_child; child != NULL && child->end_at == start; )
		{
			Segment *prev = child->prev;
			segment_erase_range_ (ce, child, start, end);
			child = prev;
		}
	}
	else
	{
		for (child = segment->children; child != NULL; )
		{
			Segment *next = child->next;
			segment_erase_range_ (ce, child, start, end);
			child = next;
		}
	}

	if (segment->sub_patterns)
	{
		SubPattern *sub_patterns, *sp;

		sub_patterns = segment->sub_patterns;
		segment->sub_patterns = NULL;

		for (sp = sub_patterns; sp != NULL; )
		{
			SubPattern *next = sp->next;

			if (sp->start_at >= start && sp->end_at <= end)
				sub_pattern_free (sp);
			else
				segment_add_subpattern (segment, sp);

			sp = next;
		}
	}

	if (segment->parent)
	{
		/* Now all children and subpatterns are cleaned up,
		 * so we only need to split segment properly if its middle
		 * was erased. Otherwise, only ends need to be adjusted. */
		if (segment->start_at < start && segment->end_at > end)
		{
			segment_erase_middle_ (ce, segment, start, end);
		}
		else
		{
			g_assert ((segment->start_at >= start && segment->end_at > end) ||
				  (segment->start_at < start && segment->end_at <= end));

			if (segment->end_at > end)
				segment->start_at = end;
			else
				segment->end_at = start;
		}
	}
}

/**
 * segment_merge:
 *
 * @ce: #GtkSourceContextEngine.
 * @first: first segment.
 * @second: second segment.
 *
 * Merges adjacent segments @first and @second given
 * their contexts are equal.
 */
static void
segment_merge (GtkSourceContextEngine *ce,
	       Segment                *first,
	       Segment                *second)
{
	Segment *parent;

	if (first == second)
		return;

	g_assert (!SEGMENT_IS_INVALID (first));
	g_assert (first->context == second->context);
	g_assert (first->end_at == second->start_at);

	if (first->parent != second->parent)
		segment_merge (ce, first->parent, second->parent);

	parent = first->parent;

	g_assert (first->next == second);
	g_assert (first->parent == second->parent);
	g_assert (second != parent->children);

	if (second == parent->last_child)
		parent->last_child = first;
	first->next = second->next;
	if (second->next)
		second->next->prev = first;

	first->end_at = second->end_at;

	if (second->children)
	{
		Segment *child;

		for (child = second->children; child != NULL; child = child->next)
			child->parent = first;

		if (!first->children)
		{
			g_assert (!first->last_child);
			first->children = second->children;
			first->last_child = second->last_child;
		}
		else
		{
			first->last_child->next = second->children;
			second->children->prev = first->last_child;
			first->last_child = second->last_child;
		}
	}

	if (second->sub_patterns)
	{
		if (!first->sub_patterns)
		{
			first->sub_patterns = second->sub_patterns;
		}
		else
		{
			while (second->sub_patterns)
			{
				SubPattern *sp = second->sub_patterns;
				second->sub_patterns = second->sub_patterns->next;
				sp->next = first->sub_patterns;
				first->sub_patterns = sp;
			}
		}
	}

	second->children = NULL;
	second->last_child = NULL;
	second->sub_patterns = NULL;

	segment_destroy (ce, second);
}

/**
 * erase_segments:
 *
 * @ce: #GtkSourceContextEngine.
 * @start: start offset of region to erase.
 * @end: end offset of region to erase.
 * @hint: segment around @start to make it faster.
 *
 * Erases all non-toplevel segments in the interval
 * [@start, @end]. Its action on the tree is roughly
 * equivalent to segment_erase_range_(ce->priv->root_segment, start, end)
 * (but that does not accept toplevel segment).
 */
static void
erase_segments (GtkSourceContextEngine *ce,
		gint                    start,
		gint                    end,
		Segment                *hint)
{
	Segment *root = ce->priv->root_segment;
	Segment *child, *hint_prev;

	if (!root->children)
		return;

	if (!hint)
		hint = ce->priv->hint;

	if (hint)
		while (hint && hint->parent != ce->priv->root_segment)
			hint = hint->parent;

	if (!hint)
		hint = root->children;

	hint_prev = hint->prev;

	child = hint;
	while (child)
	{
		Segment *next = child->next;

		if (child->end_at < start)
		{
			child = next;

			if (next)
				ce->priv->hint = next;

			continue;
		}

		if (child->start_at > end)
		{
			ce->priv->hint = child;
			break;
		}

		segment_erase_range_ (ce, child, start, end);
		child = next;
	}

	child = hint_prev;
	while (child)
	{
		Segment *prev = child->prev;

		if (!ce->priv->hint)
			ce->priv->hint = child;

		if (child->start_at > end)
		{
			child = prev;
			continue;
		}

		if (child->end_at < start)
		{
			break;
		}

		segment_erase_range_ (ce, child, start, end);
		child = prev;
	}

	CHECK_TREE (ce);
}

/**
 * update_syntax:
 *
 * @ce: #GtkSourceContextEngine.
 * @end: desired end of region to analyze or NULL.
 * @time: maximal amount of time in milliseconds allowed to spend here
 * or 0 for 'unlimited'.
 *
 * Updates syntax tree. If %end is not NULL, then it analyzes
 * (reanalyzes invalid areas in) region from start of buffer
 * to %end. Otherwise, it analyzes batch of text starting at
 * first invalid line.
 * In order to avoid blocking ui it uses a timer and stops
 * when time elapsed is greater than %time, so analyzed region is
 * not necessarily what's requested (unless %time is 0).
 */
/* XXX it needs to be refactored. I'm not doing it now because it's
 * not clear whether it does the right thing (it was rewritten two times
 * already). */
static void
update_syntax (GtkSourceContextEngine *ce,
	       const GtkTextIter      *end,
	       gint                    time)
{
	Segment *invalid;
	GtkTextIter start_iter, end_iter;
	GtkTextIter line_start, line_end;
	gint start_offset, end_offset;
	gint line_start_offset, line_end_offset;
        gint analyzed_end;
	GtkTextBuffer *buffer = ce->priv->buffer;
	Segment *state = ce->priv->root_segment;
	Segment *hint = ce->priv->hint;
	GTimer *timer;

	context_freeze (ce->priv->root_context);
	update_tree (ce);

	if (!gtk_text_buffer_get_char_count (buffer))
	{
		segment_tree_zero_len (ce);
		goto out;
	}

	invalid = get_invalid_segment (ce);

	if (!invalid)
		goto out;

	if (end && invalid->start_at >= gtk_text_iter_get_offset (end))
		goto out;

	if (end)
	{
		end_offset = gtk_text_iter_get_offset (end);
		start_offset = MIN (end_offset, invalid->start_at);
	}
	else
	{
		start_offset = invalid->start_at;
		end_offset = gtk_text_buffer_get_char_count (buffer);
	}

	gtk_text_buffer_get_iter_at_offset (buffer, &start_iter, start_offset);
	gtk_text_buffer_get_iter_at_offset (buffer, &end_iter, end_offset);

	if (!gtk_text_iter_starts_line (&start_iter))
	{
		gtk_text_iter_set_line_offset (&start_iter, 0);
		start_offset = gtk_text_iter_get_offset (&start_iter);
	}

	if (!gtk_text_iter_starts_line (&end_iter))
	{
		gtk_text_iter_forward_line (&end_iter);
		end_offset = gtk_text_iter_get_offset (&end_iter);
	}

	/* This happens after deleting all text on last line. */
	if (start_offset == end_offset)
	{
		g_assert (end_offset == gtk_text_buffer_get_char_count (buffer));
		g_assert (g_slist_length (ce->priv->invalid) == 1);
		segment_remove (ce, invalid);
		CHECK_TREE (ce);
		goto out;
	}


	/* Main loop */

	line_start = start_iter;
	line_start_offset = start_offset;
	line_end = line_start;
	gtk_text_iter_forward_line (&line_end);
	line_end_offset = gtk_text_iter_get_offset (&line_end);
	analyzed_end = line_end_offset;

	timer = g_timer_new ();

	while (TRUE)
	{
		LineInfo line;
		gboolean next_line_invalid = FALSE;
		gboolean need_invalidate_next = FALSE;

		/* Last buffer line. */
		if (line_start_offset == line_end_offset)
		{
			g_assert (line_start_offset == gtk_text_buffer_get_char_count (buffer));
			break;
		}

		/* Analyze the line */
		erase_segments (ce, line_start_offset, line_end_offset, ce->priv->hint);
                get_line_info (buffer, &line_start, &line_end, &line);

		{
			invalid = get_invalid_segment (ce);
			g_assert (!invalid || invalid->start_at >= line_end_offset);
		}

		if (!line_start_offset)
			state = ce->priv->root_segment;
		else
			state = get_segment_at_offset (ce,
						       ce->priv->hint ? ce->priv->hint : state,
						       line_start_offset - 1);
		g_assert (state->context != NULL);

		hint = ce->priv->hint;

		if (hint && hint->parent != state)
			hint = NULL;

		state = analyze_line (ce, state, &line, &hint);
		CHECK_TREE (ce);

		{
			invalid = get_invalid_segment (ce);
			g_assert (!invalid || invalid->start_at >= line_end_offset);
		}

		/* XXX this is wrong */
		if (hint)
			ce->priv->hint = hint;
		else
			ce->priv->hint = state;

		line_info_destroy (&line);

		gtk_text_region_add (ce->priv->refresh_region, &line_start, &line_end);
		analyzed_end = line_end_offset;

		if ((invalid = get_invalid_segment (ce)))
		{
			GtkTextIter iter;

			gtk_text_buffer_get_iter_at_offset (buffer, &iter, invalid->start_at);
			gtk_text_iter_set_line_offset (&iter, 0);

			if (gtk_text_iter_get_offset (&iter) == line_end_offset)
				next_line_invalid = TRUE;
		}

		if (!next_line_invalid)
		{
			Segment *old_state;

			hint = ce->priv->hint ? ce->priv->hint : state;
			old_state = get_segment_at_offset (ce, hint, line_end_offset);

			if (old_state->context != state->context)
			{
				need_invalidate_next = TRUE;
				next_line_invalid = TRUE;
			}
			else
			{
				segment_merge (ce, state, old_state);
				CHECK_TREE (ce);
			}
		}

		if ((time && g_timer_elapsed (timer, NULL) * 1000 > time) ||
		    line_end_offset >= end_offset ||
		    (!invalid && !next_line_invalid))
		{
			if (need_invalidate_next)
				insert_range (ce, line_end_offset, 0);
			break;
		}

		if (next_line_invalid)
		{
			line_start_offset = line_end_offset;
			line_start = line_end;
			gtk_text_iter_forward_line (&line_end);
			line_end_offset = gtk_text_iter_get_offset (&line_end);
		}
		else
		{
			gtk_text_buffer_get_iter_at_offset (buffer, &line_start, invalid->start_at);
			gtk_text_iter_set_line_offset (&line_start, 0);
			line_start_offset = gtk_text_iter_get_offset (&line_start);
			line_end = line_start;
			gtk_text_iter_forward_line (&line_end);
			line_end_offset = gtk_text_iter_get_offset (&line_end);
		}
	}

	if (analyzed_end == gtk_text_buffer_get_char_count (buffer))
	{
		g_assert (g_slist_length (ce->priv->invalid) <= 1);

		if (ce->priv->invalid != NULL)
		{
			invalid = get_invalid_segment (ce);
			segment_remove (ce, invalid);
			CHECK_TREE (ce);
		}
	}

	if (!all_analyzed (ce))
		install_idle_worker (ce);

	gtk_text_iter_set_offset (&end_iter, analyzed_end);
	refresh_range (ce, &start_iter, &end_iter, FALSE);

	PROFILE (g_print ("analyzed %d chars from %d to %d in %fms\n",
			  analyzed_end - start_offset, start_offset, analyzed_end,
			  g_timer_elapsed (timer, NULL) * 1000));

	g_timer_destroy (timer);

out:
	context_thaw (ce->priv->root_context);
}


/* DEFINITIONS MANAGEMENT ------------------------------------------------- */

static DefinitionChild *
definition_child_new (ContextDefinition *definition,
		      ContextDefinition *child_def,
		      gboolean           is_ref_all)
{
	DefinitionChild *ch = g_new0 (DefinitionChild, 1);

	ch->is_ref_all = is_ref_all;
	ch->definition = child_def;

	definition->children = g_slist_append (definition->children, ch);

	return ch;
}

static void
definition_child_free (DefinitionChild *ch)
{
	g_free (ch);
}

static ContextDefinition *
context_definition_new (const gchar        *id,
			ContextType         type,
			ContextDefinition  *parent,
			const gchar        *match,
			const gchar        *start,
			const gchar        *end,
			const gchar        *style,
			GtkSourceContextMatchOptions options,
			GError            **error)
{
	ContextDefinition *definition;
	gboolean regex_error = FALSE;
	gboolean unresolved_error = FALSE;

	g_return_val_if_fail (id != NULL, NULL);

	switch (type)
	{
		case CONTEXT_TYPE_SIMPLE:
			g_return_val_if_fail (match != NULL, NULL);
			g_return_val_if_fail (!end && !start, NULL);
			break;
		case CONTEXT_TYPE_CONTAINER:
			g_return_val_if_fail (!match, NULL);
			g_return_val_if_fail (!end || start, NULL);
			break;
	}

	definition = g_new0 (ContextDefinition, 1);

	if (match)
	{
		definition->u.match = regex_new (match, EGG_REGEX_ANCHORED, error);

		if (!definition->u.match)
		{
			regex_error = TRUE;
		}
		else if (!definition->u.match->resolved)
		{
			unresolved_error = TRUE;
			regex_unref (definition->u.match);
			definition->u.match = NULL;
		}
	}

	if (start)
	{
		definition->u.start_end.start = regex_new (start, EGG_REGEX_ANCHORED, error);

		if (!definition->u.start_end.start)
		{
			regex_error = TRUE;
		}
		else if (!definition->u.start_end.start->resolved)
		{
			unresolved_error = TRUE;
			regex_unref (definition->u.start_end.start);
			definition->u.start_end.start = NULL;
		}
	}

	if (end)
	{
		definition->u.start_end.end = regex_new (end, EGG_REGEX_ANCHORED, error);

		if (!definition->u.start_end.end)
			regex_error = TRUE;
	}

	if (unresolved_error)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_START_REF,
			     "context '%s' cannot contain a \\%%{...@start} command",
			     id);
		regex_error = TRUE;
	}

	if (regex_error)
	{
		g_free (definition);
		return NULL;
	}

	definition->id = g_strdup (id);
	definition->style = g_strdup (style);
	definition->type = type;
	definition->extend_parent = (options & GTK_SOURCE_CONTEXT_EXTEND_PARENT) != 0;
	definition->end_at_line_end = (options & GTK_SOURCE_CONTEXT_END_AT_LINE_END) != 0;
	definition->first_line_only = (options & GTK_SOURCE_CONTEXT_FIRST_LINE_ONLY) != 0;
	definition->children = NULL;
	definition->sub_patterns = NULL;
	definition->n_sub_patterns = 0;

	/* Main contexts (i.e. the contexts with id "language:language")
	 * should have extend-parent="true" and end-at-line-end="false". */
	if (!parent && egg_regex_match_simple ("^(.+):\\1$", id, 0, 0))
	{
		if (definition->end_at_line_end)
		{
			g_warning ("end-at-line-end should be "
				   "\"false\" for main contexts (id: %s)",
				   id);
			definition->end_at_line_end = FALSE;
		}

		if (!definition->extend_parent)
		{
			g_warning ("extend-parent should be "
				   "\"true\" for main contexts (id: %s)",
				   id);
			definition->extend_parent = TRUE;
		}
	}

	return definition;
}

static GtkSourceContextMatchOptions
context_definition_get_options (ContextDefinition *definition)
{
	GtkSourceContextMatchOptions options = 0;

	if (definition->extend_parent)
		options |= GTK_SOURCE_CONTEXT_EXTEND_PARENT;
	if (definition->end_at_line_end)
		options |= GTK_SOURCE_CONTEXT_END_AT_LINE_END;
	if (definition->first_line_only)
		options |= GTK_SOURCE_CONTEXT_FIRST_LINE_ONLY;

	return options;
}

static ContextDefinition *
context_definition_copy (GtkSourceContextEngine *ce,
			 ContextDefinition      *definition,
			 ContextDefinition      *parent,
			 const char             *style,
			 GError                **error)
{
	ContextDefinition *copy;
	gchar *id;
	GtkSourceContextMatchOptions options;
	const gchar *match = NULL;
	const gchar *start = NULL, *end = NULL;

	switch (definition->type)
	{
		case CONTEXT_TYPE_SIMPLE:
			match = regex_get_pattern (definition->u.match);
			break;
		case CONTEXT_TYPE_CONTAINER:
			if (definition->u.start_end.start)
				start = regex_get_pattern (definition->u.start_end.start);
			if (definition->u.start_end.end)
				end = regex_get_pattern (definition->u.start_end.end);
			break;
		default:
			g_return_val_if_reached (NULL);
	}

	DEBUG ({
		g_print ("match: %s\n", match ? match : "(null)");
		g_print ("start: %s\n", start ? start : "(null)");
		g_print ("end: %s\n", end ? end : "(null)");
	});

	id = g_strdup_printf ("%s@%s@%s", ce->priv->id, parent->id, definition->id);

	if (g_hash_table_lookup (ce->priv->definitions, id) != NULL)
	{
		guint i = 1;
		while (TRUE)
		{
			g_free (id);
			id = g_strdup_printf ("%s@%s@%s@%d", ce->priv->id, parent->id, definition->id, i);
			if (g_hash_table_lookup (ce->priv->definitions, id) == NULL)
				break;
			++i;
		}
	}

	options = context_definition_get_options (definition);

	copy = context_definition_new (id, definition->type, parent,
				       match, start, end,
				       style, options, error);

	g_free (id);
	return copy;
}

static void
context_definition_free (ContextDefinition *definition)
{
	GSList *sub_pattern_list;

	if (definition == NULL)
		return;

	switch (definition->type)
	{
		case CONTEXT_TYPE_SIMPLE:
			regex_unref (definition->u.match);
			break;
		case CONTEXT_TYPE_CONTAINER:
			regex_unref (definition->u.start_end.start);
			regex_unref (definition->u.start_end.end);
			break;
	}

	sub_pattern_list = definition->sub_patterns;
	while (sub_pattern_list != NULL)
	{
		SubPatternDefinition *sp_def = sub_pattern_list->data;
#ifdef NEED_DEBUG_ID
		g_free (sp_def->id);
#endif
		g_free (sp_def->style);
		if (sp_def->is_named)
			g_free (sp_def->u.name);
		g_free (sp_def);
		sub_pattern_list = sub_pattern_list->next;
	}
	g_slist_free (definition->sub_patterns);

	g_free (definition->id);
	g_free (definition->style);
	regex_unref (definition->reg_all);

	g_slist_foreach (definition->children, (GFunc) definition_child_free, NULL);
	g_slist_free (definition->children);
	g_free (definition);
}

static void
definition_iter_init (DefinitionsIter   *iter,
		      ContextDefinition *definition)
{
	iter->children_stack = g_slist_prepend (NULL, definition->children);
}

static void
definition_iter_destroy (DefinitionsIter *iter)
{
	g_slist_free (iter->children_stack);
}

static ContextDefinition *
definition_iter_next (DefinitionsIter *iter)
{
	GSList *children_list;

	if (!iter->children_stack)
		return NULL;

	children_list = iter->children_stack->data;
	if (children_list == NULL)
	{
		iter->children_stack = g_slist_delete_link (iter->children_stack,
							    iter->children_stack);
		return definition_iter_next (iter);
	}
	else
	{
		DefinitionChild *curr_child = children_list->data;
		ContextDefinition *definition = curr_child->definition;
		children_list = g_slist_next (children_list);
		iter->children_stack->data = children_list;
		if (curr_child->is_ref_all)
		{
			iter->children_stack = g_slist_prepend (iter->children_stack,
								definition->children);
			return definition_iter_next (iter);
		}
		else
		{
			return definition;
		}
	}
}

gboolean
_gtk_source_context_engine_define_context (GtkSourceContextEngine  *ce,
					   const gchar             *id,
					   const gchar             *parent_id,
					   const gchar             *match_regex,
					   const gchar             *start_regex,
					   const gchar             *end_regex,
					   const gchar             *style,
					   GtkSourceContextMatchOptions options,
					   GError                 **error)
{
	ContextDefinition *definition, *parent = NULL;
	ContextType type;

	gboolean wrong_args = FALSE;

	g_return_val_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (ce), FALSE);
	g_return_val_if_fail (id != NULL, FALSE);

	/* If the id is already present in the hashtable it is a duplicate,
	 * so we report the error (probably there is a duplicate id in the
	 * XML lang file) */
	if (LOOKUP_DEFINITION (ce, id) != NULL)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_DUPLICATED_ID,
			     "duplicated context id '%s'", id);
		return FALSE;
	}

	if (match_regex != NULL)
		type = CONTEXT_TYPE_SIMPLE;
	else
		type = CONTEXT_TYPE_CONTAINER;

	/* Check if the arguments passed are exactly what we expect, no more, no less. */
	switch (type)
	{
		case CONTEXT_TYPE_SIMPLE:
			if (start_regex || end_regex)
				wrong_args = TRUE;
			break;
		case CONTEXT_TYPE_CONTAINER:
			if (match_regex)
				wrong_args = TRUE;
			break;
	}

	if (wrong_args)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_ARGS,
			     "insufficient or redunduant arguments creating "
			     "the context '%s'", id);
		return FALSE;
	}

	if (!parent_id)
	{
		parent = NULL;
	}
	else
	{
		parent = LOOKUP_DEFINITION (ce, parent_id);
		g_return_val_if_fail (parent != NULL, FALSE);
	}

	definition = context_definition_new (id, type, parent, match_regex,
					     start_regex, end_regex, style,
					     options, error);
	if (!definition)
		return FALSE;

	g_hash_table_insert (ce->priv->definitions, g_strdup (id), definition);

	if (parent)
		definition_child_new (parent, definition, FALSE);

	return TRUE;
}

gboolean
_gtk_source_context_engine_add_sub_pattern (GtkSourceContextEngine  *ce,
					    const gchar             *id,
					    const gchar             *parent_id,
					    const gchar             *name,
					    const gchar             *where,
					    const gchar             *style,
					    GError                 **error)
{
	ContextDefinition *parent;
	SubPatternDefinition *sp_def;
	SubPatternWhere where_num;
	gint number;

	g_return_val_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (ce), FALSE);
	g_return_val_if_fail (id != NULL, FALSE);
	g_return_val_if_fail (parent_id != NULL, FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	/* If the id is already present in the hashtable it is a duplicate,
	 * so we report the error (probably there is a duplicate id in the
	 * XML lang file) */
	if (LOOKUP_DEFINITION (ce, id) != NULL)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_DUPLICATED_ID,
			     "duplicated context id '%s'", id);
		return FALSE;
	}

	parent = LOOKUP_DEFINITION (ce, parent_id);
	g_return_val_if_fail (parent != NULL, FALSE);

	if (!where || !where[0] || !strcmp (where, "default"))
		where_num = SUB_PATTERN_WHERE_DEFAULT;
	else if (!strcmp (where, "start"))
		where_num = SUB_PATTERN_WHERE_START;
	else if (!strcmp (where, "end"))
		where_num = SUB_PATTERN_WHERE_END;
	else
		where_num = (SubPatternWhere) -1;

	if ((parent->type == CONTEXT_TYPE_SIMPLE && where_num != SUB_PATTERN_WHERE_DEFAULT) ||
	    (parent->type == CONTEXT_TYPE_CONTAINER && where_num == SUB_PATTERN_WHERE_DEFAULT))
	{
		where_num = (SubPatternWhere) -1;
	}

	if (where_num == (SubPatternWhere) -1)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_WHERE,
			     "invalid location ('%s') for sub pattern '%s'",
			     where, id);
		return FALSE;
	}

	sp_def = g_new0 (SubPatternDefinition, 1);
#ifdef NEED_DEBUG_ID
	sp_def->id = g_strdup (id);
#endif
	sp_def->style = g_strdup (style);
	sp_def->where = where_num;
	number = sub_pattern_to_int (name);

	if (number < 0)
	{
		sp_def->is_named = TRUE;
		sp_def->u.name = g_strdup (name);
	}
	else
	{
		sp_def->is_named = FALSE;
		sp_def->u.num = number;
	}

	parent->sub_patterns = g_slist_append (parent->sub_patterns, sp_def);
	sp_def->index = parent->n_sub_patterns++;

	return TRUE;
}

gboolean
_gtk_source_context_engine_add_ref (GtkSourceContextEngine    *ce,
				    const gchar               *parent_id,
				    const gchar               *ref_id,
				    GtkSourceContextRefOptions options,
				    const gchar               *style,
				    gboolean                   all,
				    GError                   **error)
{
	ContextDefinition *parent;
	ContextDefinition *ref;

	g_return_val_if_fail (parent_id != NULL, FALSE);
	g_return_val_if_fail (ref_id != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (ce), FALSE);

	if (all && (options & (GTK_SOURCE_CONTEXT_IGNORE_STYLE | GTK_SOURCE_CONTEXT_OVERRIDE_STYLE)))
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_STYLE,
			     "can't override style for '%s' reference",
			     ref_id);
		return FALSE;
	}

	/* If the id is already present in the hashtable it is a duplicate,
	 * so we report the error (probably there is a duplicate id in the
	 * XML lang file). */
	ref = LOOKUP_DEFINITION (ce, ref_id);

	if (!ref)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_REF,
			     "invalid id '%s', the definition does not exist",
			     ref_id);
		return FALSE;
	}

	if (all && ref->type != CONTEXT_TYPE_CONTAINER)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_REF,
			     "context '%s' is not a container context",
			     ref_id);
		return FALSE;
	}

	parent = LOOKUP_DEFINITION (ce, parent_id);
	g_return_val_if_fail (parent != NULL, FALSE);

	if (parent->type != CONTEXT_TYPE_CONTAINER)
	{
		g_set_error (error,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR,
			     GTK_SOURCE_CONTEXT_ENGINE_ERROR_INVALID_PARENT,
			     "invalid parent type for the context '%s'",
			     ref_id);
		return FALSE;
	}

	if (options & (GTK_SOURCE_CONTEXT_IGNORE_STYLE | GTK_SOURCE_CONTEXT_OVERRIDE_STYLE))
	{
		ref = context_definition_copy (ce, ref, parent, style, error);

		if (!ref)
			return FALSE;

		g_hash_table_insert (ce->priv->definitions, g_strdup (ref->id), ref);
	}

	definition_child_new (parent, ref, all);

	return TRUE;
}

static void
add_escape_ref (ContextDefinition      *definition,
		GtkSourceContextEngine *ce)
{
	GError *error = NULL;

	/* XXX */
	if (definition->type != CONTEXT_TYPE_CONTAINER)
		return;

	_gtk_source_context_engine_add_ref (ce, definition->id,
					    "gtk-source-context-engine-escape",
					    0, NULL, FALSE, &error);

	if (error)
		goto out;

	_gtk_source_context_engine_add_ref (ce, definition->id,
					    "gtk-source-context-engine-line-escape",
					    0, NULL, FALSE, &error);

out:
	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
	}
}

static void
prepend_definition (G_GNUC_UNUSED gchar *id,
		    ContextDefinition *definition,
		    GSList **list)
{
	*list = g_slist_prepend (*list, definition);
}

/* Only for lang files version 1, do not use it */
/* It's called after lang file is parsed. It creates two special contexts
   contexts and puts them into every container context defined. These contexts
   are 'x.' and 'x$', where 'x' is the escape char. In this way, patterns from
   lang files are matched only if match doesn't start with escaped char, and
   escaped char in the end of line means that the current contexts extends to the
   next line. */
/* XXX unicode */
void
_gtk_source_context_engine_set_escape_char (GtkSourceContextEngine *ce,
					    gunichar                escape_char)
{
	GError *error = NULL;
	char buf[10];
	gint len;
	char *escaped, *pattern;
	GSList *definitions = NULL;

	g_return_if_fail (GTK_IS_SOURCE_CONTEXT_ENGINE (ce));
	g_return_if_fail (escape_char != 0);

	len = g_unichar_to_utf8 (escape_char, buf);
	g_return_if_fail (len > 0);

	escaped = egg_regex_escape_string (buf, 1);
	pattern = g_strdup_printf ("%s.", escaped);

	g_hash_table_foreach (ce->priv->definitions, (GHFunc) prepend_definition, &definitions);
	definitions = g_slist_reverse (definitions);

	if (!_gtk_source_context_engine_define_context (ce, "gtk-source-context-engine-escape",
							NULL, pattern, NULL, NULL, NULL,
							GTK_SOURCE_CONTEXT_EXTEND_PARENT,
							&error))
		goto out;

	g_free (pattern);
	pattern = g_strdup_printf ("%s$", escaped);

	if (!_gtk_source_context_engine_define_context (ce, "gtk-source-context-engine-line-escape",
							NULL, NULL, pattern, "^", NULL,
							GTK_SOURCE_CONTEXT_EXTEND_PARENT,
							&error))
		goto out;

	g_slist_foreach (definitions, (GFunc) add_escape_ref, ce);

out:
	if (error)
	{
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	g_free (pattern);
	g_free (escaped);
	g_slist_free (definitions);
}


/* DEBUG CODE ------------------------------------------------------------- */

#ifdef ENABLE_CHECK_TREE
static void
check_segment (GtkSourceContextEngine *ce,
	       Segment                *segment)
{
	Segment *child;

	g_assert (segment != NULL);
	g_assert (segment->start_at <= segment->end_at);
	g_assert (!segment->next || segment->next->start_at >= segment->end_at);

	if (SEGMENT_IS_INVALID (segment))
		g_assert (g_slist_find (ce->priv->invalid, segment) != NULL);
	else
		g_assert (g_slist_find (ce->priv->invalid, segment) == NULL);

	if (segment->children)
		g_assert (!SEGMENT_IS_INVALID (segment) && SEGMENT_IS_CONTAINER (segment));

	for (child = segment->children; child != NULL; child = child->next)
	{
		g_assert (child->parent == segment);
		g_assert (child->start_at >= segment->start_at);
		g_assert (child->end_at <= segment->end_at);
		g_assert (child->prev || child == segment->children);
		g_assert (child->next || child == segment->last_child);
		check_segment (ce, child);
	}
}

static void
check_context_hash_cb (const char *text,
		       Context    *context,
		       gpointer    user_data)
{
	struct {
		Context *parent;
		ContextDefinition *definition;
	} *data = user_data;

	g_assert (text != NULL);
	g_assert (context != NULL);
	g_assert (context->definition == data->definition);
	g_assert (context->parent == data->parent);
}

static void
check_context (Context *context)
{
	ContextPtr *ptr;

	for (ptr = context->children; ptr != NULL; ptr = ptr->next)
	{
		if (ptr->fixed)
		{
			g_assert (ptr->u.context->parent == context);
			g_assert (ptr->u.context->definition == ptr->definition);
			check_context (ptr->u.context);
		}
		else
		{
			struct {
				Context *parent;
				ContextDefinition *definition;
			} data = {context, ptr->definition};
			g_hash_table_foreach (ptr->u.hash,
					      (GHFunc) check_context_hash_cb,
					      &data);
		}
	}
}

static void
check_regex (void)
{
	static gboolean done;

	if (!done)
	{
		g_assert (!find_single_byte_escape ("gfregerg"));
		g_assert (!find_single_byte_escape ("\\\\C"));
		g_assert (find_single_byte_escape ("\\C"));
		g_assert (find_single_byte_escape ("ewfwefwefwef\\Cwefwefwefwe"));
		g_assert (find_single_byte_escape ("ewfwefwefwef\\\\Cwefw\\Cefwefwe"));
		g_assert (!find_single_byte_escape ("ewfwefwefwef\\\\Cwefw\\\\Cefwefwe"));

		done = TRUE;
	}
}

static void
CHECK_TREE (GtkSourceContextEngine *ce)
{
	Segment *root = ce->priv->root_segment;

	check_regex ();

	g_assert (root->start_at == 0);

	if (ce->priv->invalid_region.empty)
		g_assert (root->end_at == gtk_text_buffer_get_char_count (ce->priv->buffer));

	g_assert (!root->parent);
	check_segment (ce, root);

	g_assert (!ce->priv->root_context->parent);
	g_assert (root->context == ce->priv->root_context);
	check_context (ce->priv->root_context);
}

static void
CHECK_SEGMENT_CHILDREN (Segment *segment)
{
	Segment *ch;

	g_assert (segment != NULL);
	CHECK_SEGMENT_LIST (segment->parent);

	for (ch = segment->children; ch != NULL; ch = ch->next)
	{
		g_assert (ch->parent == segment);
		g_assert (ch->start_at <= ch->end_at);
		g_assert (!ch->next || ch->next->start_at >= ch->end_at);
		g_assert (ch->start_at >= segment->start_at);
		g_assert (ch->end_at <= segment->end_at);
		g_assert (ch->prev || ch == segment->children);
		g_assert (ch->next || ch == segment->last_child);
	}
}

static void
CHECK_SEGMENT_LIST (Segment *segment)
{
	Segment *ch;

	if (!segment)
		return;

	for (ch = segment->children; ch != NULL; ch = ch->next)
	{
		g_assert (ch->parent == segment);
		g_assert (ch->start_at <= ch->end_at);
		g_assert (!ch->next || ch->next->start_at >= ch->end_at);
		g_assert (ch->prev || ch == segment->children);
		g_assert (ch->next || ch == segment->last_child);
	}
}
#endif /* ENABLE_CHECK_TREE */
