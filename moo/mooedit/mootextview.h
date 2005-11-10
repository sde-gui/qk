/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooedit.h
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

#ifndef __MOO_TEXT_VIEW_H__
#define __MOO_TEXT_VIEW_H__

#include <gtk/gtktextview.h>
#include <mooedit/mooindenter.h>
#include <mooedit/moolang.h>

G_BEGIN_DECLS


#define MOO_TYPE_TEXT_SELECTION_TYPE    (moo_text_selection_type_get_type ())

#define MOO_TYPE_TEXT_VIEW              (moo_text_view_get_type ())
#define MOO_TEXT_VIEW(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_TEXT_VIEW, MooTextView))
#define MOO_TEXT_VIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_TEXT_VIEW, MooTextViewClass))
#define MOO_IS_TEXT_VIEW(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_TEXT_VIEW))
#define MOO_IS_TEXT_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_TEXT_VIEW))
#define MOO_TEXT_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_TEXT_VIEW, MooTextViewClass))


typedef struct _MooTextView         MooTextView;
typedef struct _MooTextViewPrivate  MooTextViewPrivate;
typedef struct _MooTextViewClass    MooTextViewClass;

typedef enum {
    MOO_TEXT_SELECT_CHARS,
    MOO_TEXT_SELECT_WORDS,
    MOO_TEXT_SELECT_LINES
} MooTextSelectionType;

struct _MooTextView
{
    GtkTextView  parent;

    MooTextViewPrivate *priv;
};

struct _MooTextViewClass
{
    GtkTextViewClass parent_class;

    void (* delete_selection)       (MooTextView    *view);
    void (* undo)                   (MooTextView    *view);
    void (* redo)                   (MooTextView    *view);

    gboolean (* char_inserted)      (MooTextView    *view,
                                     GtkTextIter    *where, /* points to position after the char */
                                     guint           character); /* gunichar */

    /* these are made signals for convenience */
    void (* find_interactive)       (MooTextView    *view);
    void (* replace_interactive)    (MooTextView    *view);
    void (* find_next_interactive)  (MooTextView    *view);
    void (* find_prev_interactive)  (MooTextView    *view);
    void (* goto_line_interactive)  (MooTextView    *view);

    /* methods */
    /* adjusts start and end so that selection bound goes to start
       and insert goes to end,
       returns whether selection is not empty */
    gboolean (* extend_selection)   (MooTextView    *view,
                                     MooTextSelectionType type,
                                     GtkTextIter    *start,
                                     GtkTextIter    *end);
};


GType        moo_text_view_get_type                 (void) G_GNUC_CONST;
GType        moo_text_selection_type_get_type       (void) G_GNUC_CONST;

MooTextView *moo_text_view_new                      (void);

void         moo_text_view_select_all               (MooTextView        *view);

char        *moo_text_view_get_selection            (MooTextView        *view);
char        *moo_text_view_get_text                 (MooTextView        *view);
gboolean     moo_text_view_has_selection            (MooTextView        *view);
gboolean     moo_text_view_has_text                 (MooTextView        *view);

void         moo_text_view_delete_selection         (MooTextView        *view);

gboolean     moo_text_view_can_redo                 (MooTextView        *view);
gboolean     moo_text_view_can_undo                 (MooTextView        *view);
void         moo_text_view_redo                     (MooTextView        *view);
void         moo_text_view_undo                     (MooTextView        *view);
void         moo_text_view_start_not_undoable_action(MooTextView        *view);
void         moo_text_view_end_not_undoable_action  (MooTextView        *view);

void         moo_text_view_find_interactive         (MooTextView        *view);
void         moo_text_view_replace_interactive      (MooTextView        *view);
void         moo_text_view_find_next_interactive    (MooTextView        *view);
void         moo_text_view_find_prev_interactive    (MooTextView        *view);
void         moo_text_view_goto_line                (MooTextView        *view,
                                                     int                 line);

void         moo_text_view_set_font_from_string     (MooTextView        *view,
                                                     const char         *font);

MooIndenter *moo_text_view_get_indenter             (MooTextView        *view);
void         moo_text_view_set_indenter             (MooTextView        *view,
                                                     MooIndenter        *indenter);

void         moo_text_view_get_cursor               (MooTextView        *view,
                                                     GtkTextIter        *iter);
void         moo_text_view_move_cursor              (MooTextView        *view,
                                                     int                 line,
                                                     int                 character,
                                                     gboolean            in_idle);

void         moo_text_view_set_highlight_current_line
                                                    (MooTextView        *view,
                                                     gboolean            highlight);
void         moo_text_view_set_current_line_color   (MooTextView        *view,
                                                     const GdkColor     *color);
void         moo_text_view_set_cursor_color         (MooTextView        *view,
                                                     const GdkColor     *color);
void         moo_text_view_set_show_tabs            (MooTextView        *view,
                                                     gboolean            show);
void         moo_text_view_apply_scheme             (MooTextView        *view,
                                                     MooTextStyleScheme *scheme);

GtkTextTag  *moo_text_view_lookup_tag               (MooTextView        *view,
                                                     const char         *name);

void         moo_text_view_set_lang                 (MooTextView        *view,
                                                     MooLang            *lang);


G_END_DECLS

#endif /* __MOO_TEXT_VIEW_H__ */
