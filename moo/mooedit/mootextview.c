/*
 *   mootextview.c
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
#include "mooedit/mootextview-private.h"
#include "mooedit/mootextview.h"
#include "mooedit/mootextbuffer.h"
#include "mooedit/mootextfind.h"
#include "mooedit/mootext-private.h"
#include "mooedit/quicksearch-glade.h"
#include "mooedit/mooeditprefs.h"
#include "mooedit/mootextbox.h"
#include "mooutils/moomarshals.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooundo.h"
#include "mooutils/mooentry.h"
#include "mooutils/mooi18n.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>

/* XXX connect to style set and update/invalidate:
    current line gc;
    digit_width
*/

#define LIGHT_BLUE "#EEF6FF"
#define BOOL_CMP(b1,b2) ((b1 && b2) || (!b1 && !b2))
#define UPDATE_PRIORITY (GTK_TEXT_VIEW_PRIORITY_VALIDATE - 5)

#define MIN_LINE_MARK_WIDTH 6
#define DEFAULT_EXPANDER_SIZE 12
#define EXPANDER_PADDING 2

static GtkTextWindowType window_types[4] = {
    GTK_TEXT_WINDOW_LEFT,
    GTK_TEXT_WINDOW_RIGHT,
    GTK_TEXT_WINDOW_TOP,
    GTK_TEXT_WINDOW_BOTTOM
};

static GdkAtom moo_text_view_atom;


static GObject *moo_text_view_constructor   (GType               type,
                                             guint               n_construct_properties,
                                             GObjectConstructParam *construct_param);
static void     moo_text_view_finalize      (GObject            *object);

static void     moo_text_view_set_property  (GObject            *object,
                                             guint               prop_id,
                                             const GValue       *value,
                                             GParamSpec         *pspec);
static void     moo_text_view_get_property  (GObject            *object,
                                             guint               prop_id,
                                             GValue             *value,
                                             GParamSpec         *pspec);

static void     moo_text_view_realize       (GtkWidget          *widget);
static void     moo_text_view_unrealize     (GtkWidget          *widget);
static gboolean moo_text_view_expose        (GtkWidget          *widget,
                                             GdkEventExpose     *event);
static gboolean moo_text_view_focus_in      (GtkWidget          *widget,
                                             GdkEventFocus      *event);
static gboolean moo_text_view_focus_out     (GtkWidget          *widget,
                                             GdkEventFocus      *event);
static void     moo_text_view_style_set     (GtkWidget          *widget,
                                             GtkStyle           *previous_style);
static void     moo_text_view_size_request  (GtkWidget          *widget,
                                             GtkRequisition     *requisition);
static void     moo_text_view_size_allocate (GtkWidget          *widget,
                                             GtkAllocation      *allocation);

static void     moo_text_view_remove        (GtkContainer       *container,
                                             GtkWidget          *child);

static void     moo_text_view_copy_clipboard (GtkTextView       *text_view);
static void     moo_text_view_cut_clipboard (GtkTextView        *text_view);
static void     moo_text_view_paste_clipboard (GtkTextView      *text_view);
static void     moo_text_view_populate_popup(GtkTextView        *text_view,
                                             GtkMenu            *menu);
static void     moo_text_view_set_scroll_adjustments (GtkTextView *text_view,
                                             GtkAdjustment      *hadj,
                                             GtkAdjustment      *vadj);

static void     create_current_line_gc      (MooTextView        *view);
static void     update_tab_width            (MooTextView        *view);

static GtkTextBuffer *get_buffer            (MooTextView        *view);
static MooTextBuffer *get_moo_buffer        (MooTextView        *view);
static GtkTextMark *get_insert              (MooTextView        *view);

static void     cursor_moved                (MooTextView        *view,
                                             GtkTextIter        *where);
static void     proxy_prop_notify           (MooTextView        *view,
                                             GParamSpec         *pspec);

static void     find_word_at_cursor         (MooTextView        *view,
                                             gboolean            forward);
static void     find_interactive            (MooTextView        *view);
static void     replace_interactive         (MooTextView        *view);
static void     find_next_interactive       (MooTextView        *view);
static void     find_prev_interactive       (MooTextView        *view);
static void     goto_line_interactive       (MooTextView        *view);
static gboolean start_quick_search          (MooTextView        *view);

static void     insert_text_cb              (MooTextView        *view,
                                             GtkTextIter        *iter,
                                             gchar              *text,
                                             gint                len);
static gboolean moo_text_view_char_inserted (MooTextView        *view,
                                             GtkTextIter        *where,
                                             guint               character);
static void     moo_text_view_delete_selection (MooTextView     *view);

static void     set_manage_clipboard        (MooTextView        *view,
                                             gboolean            manage);
static void     selection_changed           (MooTextView        *view,
                                             MooTextBuffer      *buffer);
static void     highlighting_changed        (GtkTextView        *view,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);
static void     tags_changed                (GtkTextView        *view,
                                             const GtkTextIter  *start,
                                             const GtkTextIter  *end);

static void     moo_text_view_set_scheme_real (MooTextView      *view,
                                             MooTextStyleScheme *scheme);
static void     overwrite_changed           (MooTextView        *view);
static void     check_cursor_blink          (MooTextView        *view);
static void     moo_text_view_draw_cursor   (GtkTextView        *view,
                                             GdkEventExpose     *event);
static void     set_draw_tabs               (MooTextView        *view,
                                             gboolean            draw);
static void     set_draw_trailing_spaces    (MooTextView        *view,
                                             gboolean            draw);
static void     draw_line_numbers_and_marks (MooTextView        *view,
                                             GdkEventExpose     *event,
                                             GdkWindow          *text_window,
                                             GdkWindow          *left_window);
static int      get_left_margin_width       (MooTextView        *view);
static void     draw_right_margin           (MooTextView        *view,
                                             GdkEventExpose     *event);
static void     invalidate_right_margin     (MooTextView        *view);

static void     line_mark_added             (MooTextView        *view,
                                             MooLineMark        *mark);
static void     line_mark_deleted           (MooTextView        *view,
                                             MooLineMark        *mark);
static void     line_mark_changed           (MooTextView        *view,
                                             MooLineMark        *mark);
static void     line_mark_moved             (MooTextView        *view,
                                             MooLineMark        *mark);
static void     fold_added                  (MooTextView        *view,
                                             MooFold            *fold);
static void     fold_deleted                (MooTextView        *view);
static void     fold_toggled                (MooTextView        *view,
                                             MooFold            *fold);

static gboolean has_boxes                   (MooTextView        *view);
static void     update_box_tag              (MooTextView        *view);


enum {
    DELETE_SELECTION,
    FIND_INTERACTIVE,
    FIND_WORD_AT_CURSOR,
    FIND_NEXT_INTERACTIVE,
    FIND_PREV_INTERACTIVE,
    REPLACE_INTERACTIVE,
    GOTO_LINE_INTERACTIVE,
    CURSOR_MOVED,
    CHAR_INSERTED,
    UNDO,
    REDO,
    SET_SCHEME,
    LINE_MARK_CLICKED,
    START_QUICK_SEARCH,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];


enum {
    PROP_0,
    PROP_BUFFER,

    PROP_INDENTER,
    PROP_AUTO_INDENT,
    PROP_BACKSPACE_INDENTS,

    PROP_HIGHLIGHT_CURRENT_LINE,
    PROP_HIGHLIGHT_MATCHING_BRACKETS,
    PROP_HIGHLIGHT_MISMATCHING_BRACKETS,
    PROP_CURRENT_LINE_COLOR,
    PROP_CURRENT_LINE_COLOR_GDK,
    PROP_TAB_WIDTH,
    PROP_DRAW_TABS,
    PROP_DRAW_TRAILING_SPACES,
    PROP_HAS_TEXT,
    PROP_HAS_SELECTION,
    PROP_CAN_UNDO,
    PROP_CAN_REDO,
    PROP_MANAGE_CLIPBOARD,
    PROP_SMART_HOME_END,
    PROP_ENABLE_HIGHLIGHT,
    PROP_SHOW_LINE_NUMBERS,
    PROP_SHOW_SCROLLBAR_MARKS,
    PROP_SHOW_LINE_MARKS,
    PROP_ENABLE_FOLDING,
    PROP_ENABLE_QUICK_SEARCH,
    PROP_QUICK_SEARCH_FLAGS,
    PROP_TAB_KEY_ACTION
};


/* MOO_TYPE_TEXT_VIEW */
G_DEFINE_TYPE (MooTextView, moo_text_view, GTK_TYPE_TEXT_VIEW)


static void moo_text_view_class_init (MooTextViewClass *klass)
{
    gpointer ref_class;
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
    GtkTextViewClass *text_view_class = GTK_TEXT_VIEW_CLASS (klass);
    GtkBindingSet *binding_set;

    ref_class = g_type_class_ref (MOO_TYPE_INDENTER);
    g_type_class_unref (ref_class);

    moo_text_view_atom = gdk_atom_intern ("MOO_TEXT_VIEW", FALSE);

    gobject_class->set_property = moo_text_view_set_property;
    gobject_class->get_property = moo_text_view_get_property;
    gobject_class->constructor = moo_text_view_constructor;
    gobject_class->finalize = moo_text_view_finalize;

    widget_class->key_press_event = _moo_text_view_key_press_event;
    widget_class->button_press_event = _moo_text_view_button_press_event;
    widget_class->button_release_event = _moo_text_view_button_release_event;
    widget_class->motion_notify_event = _moo_text_view_motion_event;
    widget_class->realize = moo_text_view_realize;
    widget_class->unrealize = moo_text_view_unrealize;
    widget_class->expose_event = moo_text_view_expose;
    widget_class->focus_in_event = moo_text_view_focus_in;
    widget_class->focus_out_event = moo_text_view_focus_out;
    widget_class->style_set = moo_text_view_style_set;
    widget_class->size_request = moo_text_view_size_request;
    widget_class->size_allocate = moo_text_view_size_allocate;

    container_class->remove = moo_text_view_remove;

    text_view_class->move_cursor = _moo_text_view_move_cursor;
    text_view_class->page_horizontally = _moo_text_view_page_horizontally;
    text_view_class->delete_from_cursor = _moo_text_view_delete_from_cursor;
    text_view_class->copy_clipboard = moo_text_view_copy_clipboard;
    text_view_class->cut_clipboard = moo_text_view_cut_clipboard;
    text_view_class->paste_clipboard = moo_text_view_paste_clipboard;
    text_view_class->populate_popup = moo_text_view_populate_popup;
    text_view_class->set_scroll_adjustments = moo_text_view_set_scroll_adjustments;

    klass->extend_selection = _moo_text_view_extend_selection;
    klass->find_word_at_cursor = find_word_at_cursor;
    klass->find_interactive = find_interactive;
    klass->find_next_interactive = find_next_interactive;
    klass->find_prev_interactive = find_prev_interactive;
    klass->replace_interactive = replace_interactive;
    klass->goto_line_interactive = goto_line_interactive;
    klass->undo = moo_text_view_undo;
    klass->redo = moo_text_view_redo;
    klass->char_inserted = moo_text_view_char_inserted;
    klass->set_scheme = moo_text_view_set_scheme_real;

    g_object_class_install_property (gobject_class,
                                     PROP_BUFFER,
                                     g_param_spec_object ("buffer",
                                             "buffer",
                                             "buffer",
                                             MOO_TYPE_TEXT_BUFFER,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (gobject_class,
                                     PROP_HIGHLIGHT_CURRENT_LINE,
                                     g_param_spec_boolean ("highlight-current-line",
                                             "highlight-current-line",
                                             "highlight-current-line",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_HIGHLIGHT_MATCHING_BRACKETS,
                                     g_param_spec_boolean ("highlight-matching-brackets",
                                             "highlight-matching-brackets",
                                             "highlight-matching-brackets",
                                             TRUE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_HIGHLIGHT_MISMATCHING_BRACKETS,
                                     g_param_spec_boolean ("highlight-mismatching-brackets",
                                             "highlight-mismatching-brackets",
                                             "highlight-mismatching-brackets",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_LINE_COLOR_GDK,
                                     g_param_spec_boxed ("current-line-color-gdk",
                                             "current-line-color-gdk",
                                             "current-line-color-gdk",
                                             GDK_TYPE_COLOR,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_CURRENT_LINE_COLOR,
                                     g_param_spec_string ("current-line-color",
                                             "current-line-color",
                                             "current-line-color",
                                             LIGHT_BLUE,
                                             G_PARAM_WRITABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_TAB_WIDTH,
                                     g_param_spec_uint ("tab-width",
                                             "tab-width",
                                             "tab-width",
                                             1, G_MAXUINT, 8,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DRAW_TABS,
                                     g_param_spec_boolean ("draw-tabs",
                                             "draw-tabs",
                                             "draw-tabs",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_DRAW_TRAILING_SPACES,
                                     g_param_spec_boolean ("draw-trailing-spaces",
                                             "draw-trailing-spaces",
                                             "draw-trailing-spaces",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_INDENTER,
                                     g_param_spec_object ("indenter",
                                             "indenter",
                                             "indenter",
                                             MOO_TYPE_INDENTER,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_TEXT,
                                     g_param_spec_boolean ("has-text",
                                             "has-text",
                                             "has-text",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_HAS_SELECTION,
                                     g_param_spec_boolean ("has-selection",
                                             "has-selection",
                                             "has-selection",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CAN_UNDO,
                                     g_param_spec_boolean ("can-undo",
                                             "can-undo",
                                             "can-undo",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_CAN_REDO,
                                     g_param_spec_boolean ("can-redo",
                                             "can-redo",
                                             "can-redo",
                                             FALSE,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_MANAGE_CLIPBOARD,
                                     g_param_spec_boolean ("manage-clipboard",
                                             "manage-clipboard",
                                             "manage-clipboard",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SMART_HOME_END,
                                     g_param_spec_boolean ("smart-home-end",
                                             "smart-home-end",
                                             "smart-home-end",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_HIGHLIGHT,
                                     g_param_spec_boolean ("enable-highlight",
                                             "enable-highlight",
                                             "enable-highlight",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_LINE_NUMBERS,
                                     g_param_spec_boolean ("show-line-numbers",
                                             "show-line-numbers",
                                             "show-line-numbers",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_SCROLLBAR_MARKS,
                                     g_param_spec_boolean ("show-scrollbar-marks",
                                             "show-scrollbar-marks",
                                             "show-scrollbar-marks",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_SHOW_LINE_MARKS,
                                     g_param_spec_boolean ("show-line-marks",
                                             "show-line-marks",
                                             "show-line-marks",
                                             FALSE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_FOLDING,
                                     g_param_spec_boolean ("enable-folding",
                                             "enable-folding",
                                             "enable-folding",
                                             FALSE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_ENABLE_QUICK_SEARCH,
                                     g_param_spec_boolean ("enable-quick-search",
                                             "enable-quick-search",
                                             "enable-quick-search",
                                             TRUE,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_QUICK_SEARCH_FLAGS,
                                     g_param_spec_flags ("quick-search-flags",
                                             "quick-search-flags",
                                             "quick-search-flags",
                                             MOO_TYPE_TEXT_SEARCH_FLAGS,
                                             MOO_TEXT_SEARCH_CASELESS,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_TAB_KEY_ACTION,
                                     g_param_spec_enum ("tab-key-action",
                                             "tab-key-action",
                                             "tab-key-action",
                                             MOO_TYPE_TEXT_TAB_KEY_ACTION,
                                             MOO_TEXT_TAB_KEY_INDENT,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_AUTO_INDENT,
                                     g_param_spec_boolean ("auto-indent",
                                             "auto-indent", "auto-indent",
                                             TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_BACKSPACE_INDENTS,
                                     g_param_spec_boolean ("backspace-indents",
                                             "backspace-indents", "backspace-indents",
                                             FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    gtk_widget_class_install_style_property (widget_class,
                                             g_param_spec_int ("expander-size",
                                                     "Expander Size",
                                                     "Size of the expander arrow",
                                                     0,
                                                     G_MAXINT,
                                                     DEFAULT_EXPANDER_SIZE,
                                                     G_PARAM_READABLE));

    signals[UNDO] =
            g_signal_new ("undo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, undo),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[REDO] =
            g_signal_new ("redo",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, redo),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__VOID,
                          G_TYPE_BOOLEAN, 0);

    signals[DELETE_SELECTION] =
            moo_signal_new_cb ("delete-selection",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST,
                               G_CALLBACK (moo_text_view_delete_selection),
                               NULL, NULL,
                               _moo_marshal_VOID__VOID,
                               G_TYPE_NONE, 0);

    signals[FIND_WORD_AT_CURSOR] =
            g_signal_new ("find-word-at-cursor",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_word_at_cursor),
                          NULL, NULL,
                          _moo_marshal_VOID__BOOLEAN,
                          G_TYPE_NONE, 1,
                          G_TYPE_BOOLEAN);

    signals[FIND_INTERACTIVE] =
            g_signal_new ("find-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_NEXT_INTERACTIVE] =
            g_signal_new ("find-next-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_next_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[FIND_PREV_INTERACTIVE] =
            g_signal_new ("find-prev-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, find_prev_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[REPLACE_INTERACTIVE] =
            g_signal_new ("replace-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, replace_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[GOTO_LINE_INTERACTIVE] =
            g_signal_new ("goto-line-interactive",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                          G_STRUCT_OFFSET (MooTextViewClass, goto_line_interactive),
                          NULL, NULL,
                          _moo_marshal_VOID__VOID,
                          G_TYPE_NONE, 0);

    signals[CHAR_INSERTED] =
            g_signal_new ("char-inserted",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, char_inserted),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__BOXED_UINT,
                          G_TYPE_BOOLEAN, 2,
                          GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
                          G_TYPE_UINT);

    signals[CURSOR_MOVED] =
            moo_signal_new_cb ("cursor-moved",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST,
                               NULL, NULL, NULL,
                               _moo_marshal_VOID__BOXED,
                               G_TYPE_NONE, 1,
                               GTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);

    signals[SET_SCHEME] =
            g_signal_new ("set-scheme",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, set_scheme),
                          NULL, NULL,
                          _moo_marshal_VOID__OBJECT,
                          G_TYPE_NONE, 1,
                          MOO_TYPE_TEXT_STYLE_SCHEME);

    signals[LINE_MARK_CLICKED] =
            g_signal_new ("line-mark-clicked",
                          G_OBJECT_CLASS_TYPE (klass),
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (MooTextViewClass, line_mark_clicked),
                          g_signal_accumulator_true_handled, NULL,
                          _moo_marshal_BOOLEAN__INT,
                          G_TYPE_BOOLEAN, 1,
                          G_TYPE_INT);

    signals[START_QUICK_SEARCH] =
            moo_signal_new_cb ("start-quick-search",
                               G_OBJECT_CLASS_TYPE (klass),
                               G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                               G_CALLBACK (start_quick_search),
                               NULL, NULL,
                               _moo_marshal_BOOL__VOID,
                               G_TYPE_BOOLEAN, 0);

    binding_set = gtk_binding_set_by_class (klass);
    gtk_binding_entry_add_signal (binding_set, GDK_z, GDK_CONTROL_MASK,
                                  "undo", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_z, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                  "redo", 0);
//     gtk_binding_entry_add_signal (binding_set, GDK_slash, GDK_CONTROL_MASK,
//                                   "start-quick-search", 0);
}


static void moo_text_view_init (MooTextView *view)
{
    char *name;

    view->priv = g_new0 (MooTextViewPrivate, 1);

    gdk_color_parse (LIGHT_BLUE, &view->priv->current_line_color);

    view->priv->drag_button = GDK_BUTTON_RELEASE;
    view->priv->drag_type = MOO_TEXT_VIEW_DRAG_NONE;
    view->priv->drag_start_x = -1;
    view->priv->drag_start_y = -1;

    view->priv->last_search_stamp = -1;

    view->priv->saved_cursor_visible = TRUE;
    view->priv->tab_key_action = MOO_TEXT_TAB_KEY_INDENT;
    view->priv->backspace_indents = FALSE;
    view->priv->enter_indents = TRUE;
    view->priv->ctrl_up_down_scrolls = TRUE;
    view->priv->ctrl_page_up_down_scrolls = TRUE;
    view->priv->smart_home_end = TRUE;

    view->priv->highlight_matching_brackets = TRUE;
    view->priv->highlight_mismatching_brackets = FALSE;
    view->priv->tab_width = 8;

    view->priv->bold_current_line_number = TRUE;

    view->priv->qs.flags = MOO_TEXT_SEARCH_CASELESS;

    name = g_strdup_printf ("moo-text-view-%p", view);
    gtk_widget_set_name (GTK_WIDGET (view), name);
    g_free (name);
}


static GObject*
moo_text_view_constructor (GType                  type,
                           guint                  n_construct_properties,
                           GObjectConstructParam *construct_param)
{
    GObject *object;
    MooTextView *view;
    MooUndoStack *undo_stack;
    GtkTextIter iter;

    object = G_OBJECT_CLASS (moo_text_view_parent_class)->constructor (
        type, n_construct_properties, construct_param);

    view = MOO_TEXT_VIEW (object);

    view->priv->constructed = TRUE;

    g_object_set (get_buffer (view),
                  "highlight-matching-brackets", view->priv->highlight_matching_brackets,
                  "highlight-mismatching-brackets", view->priv->highlight_mismatching_brackets,
                  NULL);

    g_signal_connect_swapped (get_buffer (view), "cursor_moved",
                              G_CALLBACK (cursor_moved), view);
    g_signal_connect_swapped (get_buffer (view), "selection-changed",
                              G_CALLBACK (selection_changed), view);
    g_signal_connect_swapped (get_buffer (view), "highlighting-changed",
                              G_CALLBACK (highlighting_changed), view);
    g_signal_connect_swapped (get_buffer (view), "tags-changed",
                              G_CALLBACK (tags_changed), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-selection",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (get_buffer (view), "notify::has-text",
                              G_CALLBACK (proxy_prop_notify), view);

    undo_stack = _moo_text_buffer_get_undo_stack (get_moo_buffer (view));
    g_signal_connect_swapped (undo_stack, "notify::can-undo",
                              G_CALLBACK (proxy_prop_notify), view);
    g_signal_connect_swapped (undo_stack, "notify::can-redo",
                              G_CALLBACK (proxy_prop_notify), view);

    g_signal_connect_data (get_buffer (view), "insert-text",
                           G_CALLBACK (insert_text_cb), view,
                           NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);

    g_signal_connect_swapped (get_buffer (view), "line-mark-added",
                              G_CALLBACK (line_mark_added), view);
    g_signal_connect_swapped (get_buffer (view), "line-mark-moved",
                              G_CALLBACK (line_mark_moved), view);
    g_signal_connect_swapped (get_buffer (view), "line-mark-deleted",
                              G_CALLBACK (line_mark_deleted), view);
    g_signal_connect_swapped (get_buffer (view), "fold-added",
                              G_CALLBACK (fold_added), view);
    g_signal_connect_swapped (get_buffer (view), "fold-deleted",
                              G_CALLBACK (fold_deleted), view);
    g_signal_connect_swapped (get_buffer (view), "fold-toggled",
                              G_CALLBACK (fold_toggled), view);

    gtk_text_buffer_get_start_iter (get_buffer (view), &iter);
    view->priv->dnd_mark = gtk_text_buffer_create_mark (get_buffer (view), NULL, &iter, FALSE);
    gtk_text_mark_set_visible (view->priv->dnd_mark, FALSE);

    g_signal_connect (view, "notify::overwrite", G_CALLBACK (overwrite_changed), NULL);

    return object;
}


static void
moo_text_view_finalize (GObject *object)
{
    GSList *l;
    MooTextView *view = MOO_TEXT_VIEW (object);

    if (view->priv->indenter)
        g_object_unref (view->priv->indenter);

    for (l = view->priv->line_marks; l != NULL; l = l->next)
    {
        MooLineMark *mark = l->data;
        _moo_line_mark_set_pretty (mark, FALSE);
        g_signal_handlers_disconnect_by_func (mark, (gpointer) line_mark_changed, view);
        g_object_unref (mark);
    }

    g_slist_free (view->priv->line_marks);

    g_free (view->priv);
    view->priv = NULL;

    G_OBJECT_CLASS (moo_text_view_parent_class)->finalize (object);
}


GtkWidget *
moo_text_view_new (void)
{
    return g_object_new (MOO_TYPE_TEXT_VIEW, NULL);
}


static void
moo_text_view_delete_selection (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    gtk_text_buffer_delete_selection (get_buffer (view), TRUE, TRUE);
}


static void
moo_text_view_message (MooTextView *view,
                       const char  *message)
{
    GtkWidget *toplevel;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (message != NULL);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (view));

    if (MOO_IS_EDIT_WINDOW (toplevel))
        moo_edit_window_message (MOO_EDIT_WINDOW (toplevel), message);
}


static void
msg_to_statusbar (const char *message,
                  gpointer    data)
{
    moo_text_view_message (data, message);
}


static void
find_word_at_cursor (MooTextView *view,
                     gboolean     forward)
{
    moo_text_view_run_find_current_word (GTK_TEXT_VIEW (view), forward,
                                         msg_to_statusbar, view);
}

static void
find_interactive (MooTextView *view)
{
    moo_text_view_run_find (GTK_TEXT_VIEW (view),
                            msg_to_statusbar, view);
}

static void
replace_interactive (MooTextView *view)
{
    moo_text_view_run_replace (GTK_TEXT_VIEW (view),
                               msg_to_statusbar, view);
}

static void
find_next_interactive (MooTextView *view)
{
    moo_text_view_run_find_next (GTK_TEXT_VIEW (view),
                                 msg_to_statusbar, view);
}

static void
find_prev_interactive (MooTextView *view)
{
    moo_text_view_run_find_prev (GTK_TEXT_VIEW (view),
                                 msg_to_statusbar, view);
}

static void
goto_line_interactive (MooTextView *view)
{
    moo_text_view_run_goto_line (GTK_TEXT_VIEW (view));
}


void
moo_text_view_set_font_from_string (MooTextView *view,
                                    const char  *font)
{
    PangoFontDescription *font_desc = NULL;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (font)
        font_desc = pango_font_description_from_string (font);

    gtk_widget_modify_font (GTK_WIDGET (view), font_desc);

    if (font_desc)
        pango_font_description_free (font_desc);
}


static MooUndoStack *
get_undo_stack (MooTextView *view)
{
    return _moo_text_buffer_get_undo_stack (get_moo_buffer (view));
}


gboolean
moo_text_view_can_redo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_undo_stack_can_redo (get_undo_stack (view));
}


gboolean
moo_text_view_can_undo (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_undo_stack_can_undo (get_undo_stack (view));
}


void
moo_text_view_begin_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_undo_stack_freeze (get_undo_stack (view));
}


void
moo_text_view_end_not_undoable_action (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_undo_stack_thaw (get_undo_stack (view));
}


gboolean
moo_text_view_redo (MooTextView    *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (moo_undo_stack_can_redo (get_undo_stack (view)))
    {
        moo_text_buffer_freeze (get_moo_buffer (view));
        moo_undo_stack_redo (get_undo_stack (view));
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                      get_insert (view),
                                      0, FALSE, 0, 0);
        moo_text_buffer_thaw (get_moo_buffer (view));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


gboolean
moo_text_view_undo (MooTextView    *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (moo_undo_stack_can_undo (get_undo_stack (view)))
    {
        moo_text_buffer_freeze (get_moo_buffer (view));
        moo_undo_stack_undo (get_undo_stack (view));
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (view),
                                      get_insert (view),
                                      0, FALSE, 0, 0);
        moo_text_buffer_thaw (get_moo_buffer (view));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void
moo_text_view_select_all (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit_by_name (view, "select-all", TRUE, NULL);
}


static void
moo_text_view_set_property (GObject        *object,
                            guint           prop_id,
                            const GValue   *value,
                            GParamSpec     *pspec)
{
    MooTextView *view = MOO_TEXT_VIEW (object);
    GtkTextBuffer *buffer;
    GdkColor color;

    switch (prop_id)
    {
        case PROP_BUFFER:
            buffer = g_value_get_object (value);

            if (buffer && !MOO_IS_TEXT_BUFFER (buffer))
            {
                g_warning ("%s: ignoring buffer not of type MooTextBuffer", G_STRLOC);
                buffer = moo_text_buffer_new (NULL);
            }
            else if (!buffer)
            {
                buffer = moo_text_buffer_new (NULL);
            }
            else
            {
                g_object_ref (buffer);
            }

            gtk_text_view_set_buffer (GTK_TEXT_VIEW (view), buffer);

            g_object_unref (buffer);
            break;

        case PROP_INDENTER:
            moo_text_view_set_indenter (view, g_value_get_object (value));
            break;

        case PROP_TAB_WIDTH:
            moo_text_view_set_tab_width (view, g_value_get_uint (value));
            break;

        case PROP_HIGHLIGHT_MATCHING_BRACKETS:
            view->priv->highlight_matching_brackets = g_value_get_boolean (value);
            if (view->priv->constructed)
                g_object_set (get_buffer (view), "highlight-matching-brackets",
                              view->priv->highlight_matching_brackets, NULL);
            break;

        case PROP_HIGHLIGHT_MISMATCHING_BRACKETS:
            view->priv->highlight_mismatching_brackets = g_value_get_boolean (value);
            if (view->priv->constructed)
                g_object_set (get_buffer (view), "highlight-mismatching-brackets",
                              view->priv->highlight_mismatching_brackets, NULL);
            break;

        case PROP_HIGHLIGHT_CURRENT_LINE:
            moo_text_view_set_highlight_current_line (view, g_value_get_boolean (value));
            break;

        case PROP_CURRENT_LINE_COLOR_GDK:
            moo_text_view_set_current_line_color (view, g_value_get_boxed (value));
            break;

        case PROP_CURRENT_LINE_COLOR:
            g_return_if_fail (g_value_get_string (value) != NULL);
            gdk_color_parse (g_value_get_string (value), &color);
            moo_text_view_set_current_line_color (view, &color);
            break;

        case PROP_DRAW_TABS:
            set_draw_tabs (view, g_value_get_boolean (value));
            break;
        case PROP_DRAW_TRAILING_SPACES:
            set_draw_trailing_spaces (view, g_value_get_boolean (value));
            break;

        case PROP_MANAGE_CLIPBOARD:
            set_manage_clipboard (view, g_value_get_boolean (value));
            break;

        case PROP_SMART_HOME_END:
            view->priv->smart_home_end = g_value_get_boolean (value) != 0;
            g_object_notify (object, "smart-home-end");
            break;

        case PROP_ENABLE_HIGHLIGHT:
            moo_text_buffer_set_highlight (get_moo_buffer (view),
                                           g_value_get_boolean (value));
            g_object_notify (object, "enable-highlight");
            break;

        case PROP_SHOW_LINE_NUMBERS:
            moo_text_view_set_show_line_numbers (view, g_value_get_boolean (value));
            break;

        case PROP_SHOW_SCROLLBAR_MARKS:
            moo_text_view_set_show_scrollbar_marks (view, g_value_get_boolean (value));
            break;

        case PROP_SHOW_LINE_MARKS:
            moo_text_view_set_show_line_marks (view, g_value_get_boolean (value));
            break;

        case PROP_ENABLE_FOLDING:
            moo_text_view_set_enable_folding (view, g_value_get_boolean (value));
            break;

        case PROP_ENABLE_QUICK_SEARCH:
            view->priv->qs.enable = g_value_get_boolean (value) != 0;
            g_object_notify (object, "enable-quick-search");
            break;

        case PROP_QUICK_SEARCH_FLAGS:
            view->priv->qs.flags = g_value_get_flags (value);
            g_object_notify (object, "quick-search-flags");
            break;

        case PROP_TAB_KEY_ACTION:
            moo_text_view_set_tab_key_action (view, g_value_get_enum (value));
            break;

        case PROP_AUTO_INDENT:
            view->priv->enter_indents = g_value_get_boolean (value);
            g_object_notify (object, "auto-indent");
            break;

        case PROP_BACKSPACE_INDENTS:
            view->priv->backspace_indents = g_value_get_boolean (value);
            g_object_notify (object, "backspace-indents");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
moo_text_view_get_property (GObject        *object,
                            guint           prop_id,
                            GValue         *value,
                            GParamSpec     *pspec)
{
    MooTextView *view = MOO_TEXT_VIEW (object);
    gboolean val;

    switch (prop_id)
    {
        case PROP_BUFFER:
            g_value_set_object (value, get_buffer (view));
            break;
        case PROP_INDENTER:
            g_value_set_object (value, view->priv->indenter);
            break;
        case PROP_TAB_WIDTH:
            g_value_set_uint (value, view->priv->tab_width);
            break;
        case PROP_HIGHLIGHT_CURRENT_LINE:
            g_value_set_boolean (value, view->priv->highlight_current_line);
            break;
        case PROP_HIGHLIGHT_MATCHING_BRACKETS:
            g_object_get (get_buffer (view), "highlight-matching-brackets", &val, NULL);
            g_value_set_boolean (value, val);
            break;
        case PROP_HIGHLIGHT_MISMATCHING_BRACKETS:
            g_object_get (get_buffer (view), "highlight-mismatching-brackets", &val, NULL);
            g_value_set_boolean (value, val);
            break;
        case PROP_CURRENT_LINE_COLOR_GDK:
            g_value_set_boxed (value, &view->priv->current_line_color);
            break;
        case PROP_DRAW_TABS:
            g_value_set_boolean (value, view->priv->draw_tabs != 0);
            break;
        case PROP_DRAW_TRAILING_SPACES:
            g_value_set_boolean (value, view->priv->draw_trailing_spaces != 0);
            break;
        case PROP_HAS_TEXT:
            g_value_set_boolean (value, moo_text_view_has_text (view));
            break;
        case PROP_HAS_SELECTION:
            g_value_set_boolean (value, moo_text_view_has_selection (view));
            break;
        case PROP_CAN_UNDO:
            g_value_set_boolean (value, moo_text_view_can_undo (view));
            break;
        case PROP_CAN_REDO:
            g_value_set_boolean (value, moo_text_view_can_redo (view));
            break;
        case PROP_MANAGE_CLIPBOARD:
            g_value_set_boolean (value, view->priv->manage_clipboard != 0);
            break;
        case PROP_SMART_HOME_END:
            g_value_set_boolean (value, view->priv->smart_home_end != 0);
            break;
        case PROP_ENABLE_HIGHLIGHT:
            g_value_set_boolean (value, moo_text_buffer_get_highlight (get_moo_buffer (view)));
            break;
        case PROP_SHOW_LINE_NUMBERS:
            g_value_set_boolean (value, view->priv->show_line_numbers);
            break;
        case PROP_SHOW_SCROLLBAR_MARKS:
            g_value_set_boolean (value, view->priv->show_scrollbar_marks);
            break;
        case PROP_SHOW_LINE_MARKS:
            g_value_set_boolean (value, view->priv->show_line_marks);
            break;
        case PROP_ENABLE_FOLDING:
            g_value_set_boolean (value, view->priv->enable_folding);
            break;
        case PROP_ENABLE_QUICK_SEARCH:
            g_value_set_boolean (value, view->priv->qs.enable);
            break;
        case PROP_QUICK_SEARCH_FLAGS:
            g_value_set_flags (value, view->priv->qs.flags);
            break;
        case PROP_TAB_KEY_ACTION:
            g_value_set_enum (value, view->priv->tab_key_action);
            break;
        case PROP_AUTO_INDENT:
            g_value_set_boolean (value, view->priv->enter_indents != 0);
            break;
        case PROP_BACKSPACE_INDENTS:
            g_value_set_boolean (value, view->priv->backspace_indents != 0);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


GType
moo_text_selection_type_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_SELECT_CHARS, (char*)"MOO_TEXT_SELECT_CHARS", (char*)"select-chars" },
            { MOO_TEXT_SELECT_WORDS, (char*)"MOO_TEXT_SELECT_WORDS", (char*)"select-words" },
            { MOO_TEXT_SELECT_LINES, (char*)"MOO_TEXT_SELECT_LINES", (char*)"select-lines" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooTextSelectionType", values);
    }

    return type;
}


GType
moo_text_tab_key_action_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GEnumValue values[] = {
            { MOO_TEXT_TAB_KEY_DO_NOTHING, (char*)"MOO_TEXT_TAB_KEY_DO_NOTHING", (char*)"do-nothing" },
            { MOO_TEXT_TAB_KEY_INDENT, (char*)"MOO_TEXT_TAB_KEY_INDENT", (char*)"indent" },
            { MOO_TEXT_TAB_KEY_FIND_PLACEHOLDER, (char*)"MOO_TEXT_TAB_KEY_FIND_PLACEHOLDER", (char*)"find-placeholder" },
            { 0, NULL, NULL }
        };

        type = g_enum_register_static ("MooTextTabKeyAction", values);
    }

    return type;
}


char*
moo_text_view_get_selection (MooTextView *view)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);

    buf = get_buffer (view);

    if (gtk_text_buffer_get_selection_bounds (buf, &start, &end))
        return gtk_text_buffer_get_slice (buf, &start, &end, TRUE);
    else
        return NULL;
}


gboolean
moo_text_view_has_selection (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_buffer_has_selection (get_moo_buffer (view));
}


gboolean
moo_text_view_has_text (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_buffer_has_text (get_moo_buffer (view));
}


char *
moo_text_view_get_text (MooTextView *view)
{
    GtkTextBuffer *buf;
    GtkTextIter start, end;
    char *text;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);

    buf = get_buffer (view);
    gtk_text_buffer_get_bounds (buf, &start, &end);
    text = gtk_text_buffer_get_slice (buf, &start, &end, TRUE);

    if (text && *text)
    {
        return text;
    }
    else
    {
        g_free (text);
        return NULL;
    }
}


static void
insert_text_cb (MooTextView        *view,
                GtkTextIter        *iter,
                gchar              *text,
                gint                len)
{
    if (view->priv->in_key_press && g_utf8_strlen (text, len) == 1)
    {
        view->priv->in_key_press = FALSE;
        view->priv->char_inserted = g_utf8_get_char (text);
        view->priv->char_inserted_offset = gtk_text_iter_get_offset (iter);
    }
}


void
_moo_text_view_check_char_inserted (MooTextView *view)
{
    if (view->priv->char_inserted)
    {
        gboolean result;
        GtkTextIter iter;

        gtk_text_buffer_get_iter_at_offset (get_buffer (view), &iter,
                                            view->priv->char_inserted_offset);

        g_signal_emit (view, signals[CHAR_INSERTED], 0,
                       &iter, (guint) view->priv->char_inserted,
                       &result);

        view->priv->char_inserted = 0;
    }
}


static gboolean
moo_text_view_char_inserted (MooTextView    *view,
                             GtkTextIter    *where,
                             guint           character)
{
    if (view->priv->indenter)
    {
        GtkTextBuffer *buffer = get_buffer (view);
        gtk_text_buffer_begin_user_action (buffer);
        moo_indenter_character (view->priv->indenter,
                                character, where);
        gtk_text_buffer_end_user_action (buffer);
        return TRUE;
    }

    return FALSE;
}


static void
cursor_moved (MooTextView    *view,
              GtkTextIter    *where)
{
    int line;

    g_signal_emit (view, signals[CURSOR_MOVED], 0, where);

    line = gtk_text_iter_get_line (where);

    if (line != view->priv->cursor_line)
    {
        view->priv->cursor_line = line;
        invalidate_right_margin (view);
    }
}


static void
proxy_prop_notify (MooTextView *view,
                   GParamSpec  *pspec)
{
    g_object_notify (G_OBJECT (view), pspec->name);
}


MooIndenter*
moo_text_view_get_indenter (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    return view->priv->indenter;
}


void
moo_text_view_set_indenter (MooTextView *view,
                            MooIndenter *indenter)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (!indenter || MOO_IS_INDENTER (indenter));

    if (view->priv->indenter == indenter)
        return;

    if (view->priv->indenter)
        g_object_unref (view->priv->indenter);
    view->priv->indenter = indenter;
    if (view->priv->indenter)
        g_object_ref (view->priv->indenter);

    g_object_notify (G_OBJECT (view), "indenter");
}


typedef struct {
    MooTextView *view;
    int      line;
    int      character;
    gboolean visual;
} Scroll;


static int
iter_get_chars_in_line (const GtkTextIter *iter)
{
    GtkTextIter i = *iter;

    if (!gtk_text_iter_ends_line (&i))
        gtk_text_iter_forward_to_line_end (&i);

    return gtk_text_iter_get_line_offset (&i);
}

static gboolean
do_move_cursor (Scroll *scroll)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_val_if_fail (scroll != NULL, FALSE);
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (scroll->view), FALSE);
    g_return_val_if_fail (scroll->line >= 0, FALSE);

    buffer = get_buffer (scroll->view);

    if (scroll->line >= gtk_text_buffer_get_line_count (buffer))
        scroll->line = gtk_text_buffer_get_line_count (buffer) - 1;

    gtk_text_buffer_get_iter_at_line (buffer, &iter, scroll->line);

    if (scroll->character >= 0)
    {
        if (scroll->visual)
        {
            int line_len = moo_text_iter_get_visual_line_length (&iter, 8);

            if (scroll->character > line_len)
                scroll->character = line_len;
            else if (scroll->character < 0)
                scroll->character = 0;

            if (scroll->character == line_len)
            {
                if (!gtk_text_iter_ends_line (&iter))
                    gtk_text_iter_forward_to_line_end (&iter);
            }
            else
            {
                moo_text_iter_set_visual_line_offset (&iter, scroll->character, 8);
            }
        }
        else
        {
            int line_len = iter_get_chars_in_line (&iter);

            if (scroll->character > line_len)
                scroll->character = line_len;
            else if (scroll->character < 0)
                scroll->character = 0;

            if (scroll->character == line_len)
            {
                if (!gtk_text_iter_ends_line (&iter))
                    gtk_text_iter_forward_to_line_end (&iter);
            }
            else
            {
                gtk_text_iter_set_line_offset (&iter, scroll->character);
            }
        }
    }

    gtk_text_buffer_place_cursor (buffer, &iter);
    gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (scroll->view),
                                  gtk_text_buffer_get_insert (buffer),
                                  0.2, FALSE, 0, 0);

    g_free (scroll);
    return FALSE;
}


void
moo_text_view_move_cursor (MooTextView  *view,
                           int           line,
                           int           offset,
                           gboolean      offset_visual,
                           gboolean      in_idle)
{
    Scroll *scroll;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    scroll = g_new (Scroll, 1);
    scroll->view = view;
    scroll->line = line;
    scroll->character = offset;
    scroll->visual = offset_visual;

    if (in_idle)
        g_idle_add ((GSourceFunc) do_move_cursor,
                     scroll);
    else
        do_move_cursor (scroll);
}


void
moo_text_view_get_cursor (MooTextView *view,
                          GtkTextIter *iter)
{
    GtkTextBuffer *buffer;

    g_return_if_fail (GTK_IS_TEXT_VIEW (view));
    g_return_if_fail (iter != NULL);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    gtk_text_buffer_get_iter_at_mark (buffer, iter,
                                      gtk_text_buffer_get_insert (buffer));
}


int
moo_text_view_get_cursor_line (MooTextView *view)
{
    GtkTextIter iter;

    g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), -1);

    moo_text_view_get_cursor (view, &iter);
    return gtk_text_iter_get_line (&iter);
}


static GtkTextBuffer*
get_buffer (MooTextView *view)
{
    return gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
}


static MooTextBuffer*
get_moo_buffer (MooTextView *view)
{
    return MOO_TEXT_BUFFER (get_buffer (view));
}


static GtkTextMark*
get_insert (MooTextView    *view)
{
    return gtk_text_buffer_get_insert (get_buffer (view));
}


void
moo_text_view_set_highlight_current_line (MooTextView    *view,
                                          gboolean        highlight)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->highlight_current_line == highlight)
        return;

    view->priv->highlight_current_line = highlight;
    g_object_notify (G_OBJECT (view), "highlight-current-line");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


void
moo_text_view_set_current_line_color (MooTextView    *view,
                                      const GdkColor *color)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (color)
    {
        view->priv->current_line_color = *color;

        if (view->priv->current_line_gc)
        {
            g_object_unref (view->priv->current_line_gc);
            view->priv->current_line_gc = NULL;
            create_current_line_gc (view);
            if (GTK_WIDGET_DRAWABLE (view))
                gtk_widget_queue_draw (GTK_WIDGET (view));
        }

        g_object_freeze_notify (G_OBJECT (view));
        g_object_notify (G_OBJECT (view), "current-line-color-gdk");
        g_object_notify (G_OBJECT (view), "current-line-color");
        g_object_notify (G_OBJECT (view), "highlight-current-line");
        g_object_thaw_notify (G_OBJECT (view));
    }
    else
    {
        if (view->priv->current_line_gc)
        {
            g_object_unref (view->priv->current_line_gc);
            view->priv->current_line_gc = NULL;
        }

        if (GTK_WIDGET_DRAWABLE (view))
            gtk_widget_queue_draw (GTK_WIDGET (view));

        g_object_notify (G_OBJECT (view), "highlight-current-line");
    }
}


/********************************************************************/
/* Clipboard
 */

enum {
    TARGET_TEXT,
    TARGET_MOO_TEXT_VIEW
};

static const GtkTargetEntry targets[] = {
    { (char*) "MOO_TEXT_VIEW", GTK_TARGET_SAME_APP, TARGET_MOO_TEXT_VIEW },
    { (char*) "STRING", 0, TARGET_TEXT },
    { (char*) "TEXT",   0, TARGET_TEXT },
    { (char*) "COMPOUND_TEXT", 0, TARGET_TEXT },
    { (char*) "UTF8_STRING", 0, TARGET_TEXT }
};


static void
add_selection_clipboard (MooTextView *view)
{
    GtkClipboard *clipboard;
    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view), GDK_SELECTION_PRIMARY);
    gtk_text_buffer_add_selection_clipboard (get_buffer (view), clipboard);
}

static void
remove_selection_clipboard (MooTextView *view)
{
    GtkClipboard *clipboard;
    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view), GDK_SELECTION_PRIMARY);
    gtk_text_buffer_remove_selection_clipboard (get_buffer (view), clipboard);
}


static void
clipboard_get_selection (G_GNUC_UNUSED GtkClipboard *clipboard,
                         GtkSelectionData *selection_data,
                         guint info,
                         gpointer data)
{
    MooTextView *view = data;
    GtkTextIter start, end;

    if (info == TARGET_MOO_TEXT_VIEW)
    {
        moo_selection_data_set_pointer (selection_data,
                                        gdk_atom_intern ("MOO_TEXT_VIEW", FALSE),
                                        data);
    }
    else if (gtk_text_buffer_get_selection_bounds (get_buffer (view), &start, &end))
    {
        char *text = gtk_text_iter_get_text (&start, &end);
        gtk_selection_data_set_text (selection_data, text, -1);
        g_free (text);
    }
}


static void
clear_primary (MooTextView *view)
{
    GtkClipboard *clipboard;

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
                                          GDK_SELECTION_PRIMARY);

    if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (view))
        gtk_clipboard_clear (clipboard);
}


static void
selection_changed (MooTextView   *view,
                   MooTextBuffer *buffer)
{
    GtkClipboard *clipboard;

    if (!view->priv->manage_clipboard)
        return;

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (view),
                                          GDK_SELECTION_PRIMARY);

    if (moo_text_buffer_has_selection (buffer))
        gtk_clipboard_set_with_owner (clipboard, targets,
                                      G_N_ELEMENTS (targets),
                                      clipboard_get_selection,
                                      NULL, G_OBJECT (view));
    else
        clear_primary (view);
}


static void
set_manage_clipboard (MooTextView    *view,
                      gboolean        manage)
{
    if (BOOL_CMP (view->priv->manage_clipboard, manage))
        return;

    view->priv->manage_clipboard = manage;

    if (GTK_WIDGET_REALIZED (view))
    {
        if (manage)
        {
            remove_selection_clipboard (view);
            selection_changed (view, get_moo_buffer (view));
        }
        else
        {
            clear_primary (view);
            add_selection_clipboard (view);
        }
    }

    g_object_notify (G_OBJECT (view), "manage-clipboard");
}


static void
get_clipboard (G_GNUC_UNUSED GtkClipboard *clipboard,
               GtkSelectionData *selection_data,
               guint info,
               gpointer view)
{
    char **contents;
    char *text;

    if (info == TARGET_MOO_TEXT_VIEW)
    {
        moo_selection_data_set_pointer (selection_data,
                                        gdk_atom_intern ("MOO_TEXT_VIEW", FALSE),
                                        view);
        return;
    }

    contents = g_object_get_data (view, "moo-text-view-clipboard");
    g_return_if_fail (contents != NULL);

    text = g_strjoinv ("", contents);
    gtk_selection_data_set_text (selection_data, text, -1);
    g_free (text);
}


static void
clear_clipboard (G_GNUC_UNUSED GtkClipboard *clipboard,
                 gpointer view)
{
    g_object_set_data (view, "moo-text-view-clipboard", NULL);
}


static void
moo_text_view_cut_or_copy (GtkTextView *text_view,
                           gboolean     delete)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    char *text;
    char **pieces;
    GtkClipboard *clipboard;

    buffer = gtk_text_view_get_buffer (text_view);

    if (!gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        return;

    text = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
    pieces = g_strsplit (text, MOO_TEXT_UNKNOWN_CHAR_S, 0);
    g_object_set_data_full (G_OBJECT (text_view), "moo-text-view-clipboard",
                            pieces, (GDestroyNotify) g_strfreev);
    g_free (text);

    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view),
                                          GDK_SELECTION_CLIPBOARD);

    if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
                                       get_clipboard, clear_clipboard,
                                       G_OBJECT (text_view)))
        return;

#if GTK_CHECK_VERSION(2,6,0)
    gtk_clipboard_set_can_store (clipboard, targets + 1,
                                 G_N_ELEMENTS (targets) - 1);
#endif

    if (delete)
    {
        gtk_text_buffer_begin_user_action (buffer);
        gtk_text_buffer_delete (buffer, &start, &end);
        gtk_text_buffer_end_user_action (buffer);
        gtk_text_view_scroll_mark_onscreen (text_view,
                                            gtk_text_buffer_get_insert (buffer));
    }
}


static void
moo_text_view_copy_clipboard (GtkTextView *text_view)
{
    moo_text_view_cut_or_copy (text_view, FALSE);
}


static void
moo_text_view_cut_clipboard (GtkTextView *text_view)
{
    moo_text_view_cut_or_copy (text_view, TRUE);
}


static void
paste_moo_text_view_content (GtkTextView *target,
                             MooTextView *source)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    char **contents, **p;

    g_return_if_fail (MOO_IS_TEXT_VIEW (source));

    buffer = gtk_text_view_get_buffer (target);

    contents = g_object_get_data (G_OBJECT (source), "moo-text-view-clipboard");
    g_return_if_fail (contents != NULL);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    for (p = contents; *p; ++p)
    {
        if (p != contents)
            moo_text_view_insert_placeholder (MOO_TEXT_VIEW (target), &end, NULL);
        gtk_text_buffer_insert (buffer, &end, *p, -1);
    }
}


static void
paste_text (GtkTextView *text_view,
            const char  *text)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    g_return_if_fail (text != NULL);

    buffer = gtk_text_view_get_buffer (text_view);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    gtk_text_buffer_insert (buffer, &end, text, -1);
}


static void
moo_text_view_paste_clipboard (GtkTextView *text_view)
{
    char *text;
    GtkTextBuffer *buffer;
    GtkClipboard *clipboard;
    gboolean need_paste_text = TRUE;

    buffer = gtk_text_view_get_buffer (text_view);
    clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view), GDK_SELECTION_CLIPBOARD);

    gtk_text_buffer_begin_user_action (buffer);

    if (gtk_clipboard_wait_is_target_available (clipboard, moo_text_view_atom))
    {
        MooTextView *source;
        GtkSelectionData *data;

        if ((data = gtk_clipboard_wait_for_contents (clipboard, moo_text_view_atom)) &&
             (source = moo_selection_data_get_pointer (data, moo_text_view_atom)))
        {
            need_paste_text = FALSE;
            paste_moo_text_view_content (text_view, source);
        }

        if (data)
            gtk_selection_data_free (data);
    }

    if (need_paste_text && (text = gtk_clipboard_wait_for_text (clipboard)))
    {
        paste_text (text_view, text);
        g_free (text);
    }

    gtk_text_buffer_end_user_action (buffer);
    gtk_text_view_scroll_mark_onscreen (text_view, gtk_text_buffer_get_insert (buffer));
}


/********************************************************************/
/* Drawing and stuff
 */

static void
update_line_mark_width (MooTextView *view)
{
    GSList *l;

    if (!GTK_WIDGET_REALIZED (view))
        return;

    view->priv->line_mark_width = MIN_LINE_MARK_WIDTH;

    for (l = view->priv->line_marks; l != NULL; l = l->next)
    {
        MooLineMark *mark;
        GdkPixbuf *pixbuf;

        mark = l->data;
        pixbuf = moo_line_mark_get_pixbuf (mark);

        if (pixbuf)
        {
            int pixbuf_width = gdk_pixbuf_get_width (pixbuf);
            view->priv->line_mark_width = MAX (view->priv->line_mark_width,
                                               pixbuf_width);
        }
    }
}


/* XXX: workaround for http://bugzilla.gnome.org/show_bug.cgi?id=336796 */
static void
lower_border_window (GtkTextView   *view,
                     MooTextViewPos pos)
{
    GdkWindow *window;

    g_return_if_fail (pos < 4);

    window = gtk_text_view_get_window (view, window_types[pos]);

    if (window)
        window = gdk_window_get_parent (window);

    if (window)
        gdk_window_lower (window);
}


static void
moo_text_view_realize (GtkWidget *widget)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);
    guint i;

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->realize (widget);

    create_current_line_gc (view);
    update_tab_width (view);
    view->priv->digit_width = 0;

    if (view->priv->manage_clipboard)
    {
        remove_selection_clipboard (view);
        selection_changed (view, get_moo_buffer (view));
    }

    update_line_mark_width (view);

    /* workaround for http://bugzilla.gnome.org/show_bug.cgi?id=336796 */
    for (i = 0; i < 4; ++i)
    {
        if (view->priv->children[i] && GTK_WIDGET_VISIBLE (view->priv->children[i]))
            lower_border_window (GTK_TEXT_VIEW (view), i);
    }

    update_box_tag (view);
}


static void
moo_text_view_unrealize (GtkWidget *widget)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);
    GtkClipboard *clipboard;

    g_slist_foreach (view->priv->line_marks, (GFunc) _moo_line_mark_unrealize, NULL);
    g_object_set_data (G_OBJECT (widget), "moo-line-mark-icons", NULL);
    g_object_set_data (G_OBJECT (widget), "moo-line-mark-colors", NULL);

    if (view->priv->update_idle)
        g_source_remove (view->priv->update_idle);
    view->priv->update_idle = 0;

    g_free (view->priv->update_rectangle);
    view->priv->update_rectangle = NULL;

    if (view->priv->current_line_gc)
        g_object_unref (view->priv->current_line_gc);
    view->priv->current_line_gc = NULL;

    view->priv->digit_width = 0;
    if (view->priv->line_numbers_font)
        pango_font_description_free (view->priv->line_numbers_font);
    view->priv->line_numbers_font = NULL;

    if (view->priv->manage_clipboard)
    {
        clear_primary (view);
        add_selection_clipboard (view);
    }

    if (view->priv->blink_timeout)
    {
        g_warning ("%s: oops", G_STRLOC);
        g_source_remove (view->priv->blink_timeout);
        view->priv->blink_timeout = 0;
    }

#if GTK_CHECK_VERSION(2,6,0)
    clipboard = gtk_widget_get_clipboard (widget, GDK_SELECTION_CLIPBOARD);
    if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (view))
        gtk_clipboard_store (clipboard);
#endif

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->unrealize (widget);
}


static void
create_current_line_gc (MooTextView *view)
{
    GtkWidget *widget = GTK_WIDGET (view);
    GdkColormap *colormap;
    gboolean success;
    GdkWindow *window;

    g_return_if_fail (GTK_WIDGET_REALIZED (widget));
    g_return_if_fail (view->priv->current_line_gc == NULL);

    colormap = gtk_widget_get_colormap (widget);
    g_return_if_fail (colormap != NULL);

    window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                       GTK_TEXT_WINDOW_TEXT);
    g_return_if_fail (window != NULL);

    success = gdk_colormap_alloc_color (colormap,
                                        &view->priv->current_line_color,
                                        FALSE, TRUE);

    if (!success)
    {
        g_warning ("%s: failed to allocate color", G_STRLOC);
        view->priv->current_line_color = widget->style->bg[GTK_STATE_NORMAL];
    }

    view->priv->current_line_gc = gdk_gc_new (window);
    gdk_gc_set_foreground (view->priv->current_line_gc,
                           &view->priv->current_line_color);
}


static void
moo_text_view_draw_current_line (GtkTextView    *text_view,
                                 GdkEventExpose *event)
{
    GdkRectangle visible_rect;
    GdkRectangle redraw_rect;
    GtkTextIter cur;
    gint y;
    gint height;
    gint win_y;

    gtk_text_buffer_get_iter_at_mark (text_view->buffer,
                                      &cur,
                                      gtk_text_buffer_get_insert (text_view->buffer));

    gtk_text_view_get_line_yrange (text_view, &cur, &y, &height);

    gtk_text_view_get_visible_rect (text_view, &visible_rect);

    gtk_text_view_buffer_to_window_coords (text_view,
                                           GTK_TEXT_WINDOW_TEXT,
                                           visible_rect.x,
                                           visible_rect.y,
                                           &redraw_rect.x,
                                           &redraw_rect.y);

    gtk_text_view_buffer_to_window_coords (text_view,
                                           GTK_TEXT_WINDOW_TEXT,
                                           0,
                                           y,
                                           NULL,
                                           &win_y);

    redraw_rect.width = visible_rect.width;
    redraw_rect.height = visible_rect.height;

    gdk_draw_rectangle (event->window,
                        MOO_TEXT_VIEW(text_view)->priv->current_line_gc,
                        TRUE,
                        redraw_rect.x + MAX (0, gtk_text_view_get_left_margin (text_view) - 1),
                        win_y,
                        redraw_rect.width,
                        height);
}


static void
draw_tab_at_iter (GtkTextView    *text_view,
                  GdkEventExpose *event,
                  GtkTextIter    *iter)
{
    GdkRectangle rect;
    GdkPoint points[3];

    gtk_text_view_get_iter_location (text_view, iter, &rect);
    gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_TEXT,
                                           rect.x, rect.y + rect.height - 2,
                                           &points[0].x, &points[0].y);
    points[1] = points[0];
    points[2] = points[0];
    points[1].y += 1;
    points[2].x += 1;
    points[2].y += 1;
    gdk_draw_polygon (event->window,
                      GTK_WIDGET(text_view)->style->text_gc[GTK_STATE_NORMAL],
                      FALSE, points, 3);
}


static void
moo_text_view_draw_tabs (GtkTextView       *text_view,
                         GdkEventExpose    *event,
                         const GtkTextIter *start,
                         const GtkTextIter *end)
{
    GtkTextIter iter = *start;

    while (gtk_text_iter_compare (&iter, end) < 0)
    {
        if (gtk_text_iter_get_char (&iter) == '\t')
            draw_tab_at_iter (text_view, event, &iter);
        if (!gtk_text_iter_forward_char (&iter))
            break;
    }
}


static void
draw_box (GtkTextView       *text_view,
          GdkEventExpose    *event,
          const GtkTextIter *iter)
{
    GtkTextBuffer *buffer;
    GtkTextIter sel_start, sel_end;
    gboolean selected = FALSE;
    GdkGC *gc;
    GdkRectangle rect;

    buffer = gtk_text_view_get_buffer (text_view);

    if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
        gtk_text_iter_compare (&sel_start, iter) <= 0 &&
        gtk_text_iter_compare (iter, &sel_end) < 0)
            selected = TRUE;

    gtk_text_view_get_iter_location (text_view, iter, &rect);
    gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_TEXT,
                                           rect.x, rect.y, &rect.x, &rect.y);

    rect.x += 1;
    rect.y += 1;
    rect.width -= 3;
    rect.height -= 3;

    if (selected)
        gc = GTK_WIDGET(text_view)->style->base_gc[GTK_STATE_NORMAL];
    else
        gc = GTK_WIDGET(text_view)->style->text_gc[GTK_STATE_NORMAL];

    gdk_draw_rectangle (event->window, gc, FALSE,
                        rect.x, rect.y, rect.width, rect.height);

    rect.x += 1;
    rect.y += 1;
    rect.width -= 2;
    rect.height -= 2;

    gdk_draw_rectangle (event->window, gc, FALSE,
                        rect.x, rect.y, rect.width, rect.height);
}


static void
moo_text_view_draw_boxes (GtkTextView       *text_view,
                          GdkEventExpose    *event,
                          const GtkTextIter *start,
                          const GtkTextIter *end)
{
    GtkTextIter iter = *start;

    while (gtk_text_iter_compare (&iter, end) < 0)
    {
        if (moo_text_view_has_box_at_iter (MOO_TEXT_VIEW (text_view), &iter))
            draw_box (text_view, event, &iter);

        if (!gtk_text_iter_forward_char (&iter))
            break;
    }
}


static void
moo_text_view_draw_trailing_spaces (GtkTextView       *text_view,
                                    GdkEventExpose    *event,
                                    const GtkTextIter *start,
                                    const GtkTextIter *end)
{
    GtkTextIter iter = *start;

    do
    {
        if (!gtk_text_iter_ends_line (&iter))
            gtk_text_iter_forward_to_line_end (&iter);

        while (!gtk_text_iter_starts_line (&iter))
        {
            gunichar c;
            gtk_text_iter_backward_char (&iter);
            c = gtk_text_iter_get_char (&iter);

            if (g_unichar_isspace (c))
                draw_tab_at_iter (text_view, event, &iter);
            else
                break;
        }

        gtk_text_iter_forward_line (&iter);
    }
    while (gtk_text_iter_compare (&iter, end) < 0);
}


static gboolean
get_show_left_margin (MooTextView *view)
{
    return view->priv->show_line_numbers ||
            view->priv->show_line_marks ||
            view->priv->enable_folding;
}

static gboolean
get_show_right_margin (MooTextView *view)
{
    return view->priv->show_scrollbar_marks;
}


static gboolean
moo_text_view_expose (GtkWidget      *widget,
                      GdkEventExpose *event)
{
    gboolean handled;
    MooTextView *view = MOO_TEXT_VIEW (widget);
    GtkTextView *text_view = GTK_TEXT_VIEW (widget);
    GdkWindow *text_window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_TEXT);
    GdkWindow *left_window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_LEFT);
    GdkWindow *right_window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_RIGHT);
    GtkTextIter start, end;
    int first_line = 0, last_line = 100000;

    view->priv->in_expose = TRUE;

    if (view->priv->highlight_current_line &&
        view->priv->current_line_gc &&
        event->window == text_window && view->priv->current_line_gc)
            moo_text_view_draw_current_line (text_view, event);

    if (get_show_left_margin (view))
    {
        int margin_width = get_left_margin_width (view);
        g_assert (margin_width > 0);
        gtk_text_view_set_border_window_size (text_view, GTK_TEXT_WINDOW_LEFT,
                                              margin_width);

        if (event->window == text_window || event->window == left_window)
            draw_line_numbers_and_marks (view, event, text_window, left_window);

    }
    else
    {
        gtk_text_view_set_border_window_size (text_view, GTK_TEXT_WINDOW_LEFT, 0);
    }

    if (get_show_right_margin (view) && event->window == right_window)
        draw_right_margin (view, event);

    if (event->window == text_window)
    {
        GdkRectangle rect = event->area;

        gtk_text_view_window_to_buffer_coords (text_view,
                                               GTK_TEXT_WINDOW_TEXT,
                                               rect.x,
                                               rect.y,
                                               &rect.x,
                                               &rect.y);

        gtk_text_view_get_line_at_y (text_view, &start, rect.y, NULL);
        gtk_text_view_get_line_at_y (text_view, &end, rect.y + rect.height, NULL);
        gtk_text_iter_forward_line (&end);

        first_line = gtk_text_iter_get_line (&start);
        last_line = gtk_text_iter_get_line (&end);

        /* it reports bogus values sometimes, like on opening huge file */
        if (last_line - first_line < 2000)
            _moo_text_buffer_ensure_highlight (get_moo_buffer (view),
                                               first_line, last_line);
    }

    handled = GTK_WIDGET_CLASS(moo_text_view_parent_class)->expose_event (widget, event);

    if (event->window == text_window)
    {
        if (last_line - first_line < 2000)
        {
            if (view->priv->draw_tabs)
                moo_text_view_draw_tabs (text_view, event, &start, &end);

            if (view->priv->draw_trailing_spaces)
                moo_text_view_draw_trailing_spaces (text_view, event, &start, &end);

            if (has_boxes (view))
                moo_text_view_draw_boxes (text_view, event, &start, &end);
        }

        if (view->priv->cursor_visible)
            moo_text_view_draw_cursor (text_view, event);
    }

    view->priv->in_expose = FALSE;

    return handled;
}


static void
highlighting_changed (GtkTextView        *text_view,
                      const GtkTextIter  *start,
                      const GtkTextIter  *end)
{
    GdkRectangle visible, changed, update;
    int y, height;

    if (!GTK_WIDGET_DRAWABLE (text_view))
        return;

    gtk_text_view_get_visible_rect (text_view, &visible);

    gtk_text_view_get_line_yrange (text_view, start, &changed.y, &height);
    gtk_text_view_get_line_yrange (text_view, end, &y, &height);
    changed.height = y - changed.y + height;
    changed.x = visible.x;
    changed.width = visible.width;

    if (gdk_rectangle_intersect (&changed, &visible, &update))
    {
        GtkTextIter update_start, update_end;
        int first_line, last_line;

        gtk_text_view_get_line_at_y (text_view, &update_start, update.y, NULL);
        gtk_text_view_get_line_at_y (text_view, &update_end, update.y + update.height - 1, NULL);

        first_line = gtk_text_iter_get_line (&update_start);
        last_line = gtk_text_iter_get_line (&update_end);

        /* it reports bogus values sometimes, like on opening huge file */
        if (last_line - first_line < 2000)
        {
//             g_print ("asking to apply tags on lines %d-%d\n", first_line, last_line);
            _moo_text_buffer_ensure_highlight (get_moo_buffer (MOO_TEXT_VIEW (text_view)),
                                               first_line, last_line);
        }
    }
}


static gboolean
invalidate_rectangle (MooTextView *view)
{
    GdkWindow *window;
    GdkRectangle *rect = view->priv->update_rectangle;

    view->priv->update_rectangle = NULL;
    view->priv->update_idle = 0;

    gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (view),
                                           GTK_TEXT_WINDOW_TEXT,
                                           rect->x, rect->y,
                                           &rect->x, &rect->y);
    window = gtk_text_view_get_window (GTK_TEXT_VIEW (view), GTK_TEXT_WINDOW_TEXT);
    gdk_window_invalidate_rect (window, rect, FALSE);

    g_free (rect);
    return FALSE;
}


static void
tags_changed (GtkTextView        *text_view,
              const GtkTextIter  *start,
              const GtkTextIter  *end)
{
    GdkRectangle visible, changed, update;
    int y, height;
    MooTextView *view = MOO_TEXT_VIEW (text_view);

    if (!GTK_WIDGET_DRAWABLE (text_view))
        return;

    gtk_text_view_get_visible_rect (text_view, &visible);

    gtk_text_view_get_line_yrange (text_view, start, &changed.y, &height);
    gtk_text_view_get_line_yrange (text_view, end, &y, &height);
    changed.height = y - changed.y + height;
    changed.x = visible.x;
    changed.width = visible.width;

    if (!gdk_rectangle_intersect (&changed, &visible, &update))
        return;

    if (view->priv->update_rectangle)
    {
        gdk_rectangle_union (view->priv->update_rectangle, &update,
                             view->priv->update_rectangle);
    }
    else
    {
        view->priv->update_rectangle = g_new (GdkRectangle, 1);
        *view->priv->update_rectangle = update;
    }

    if (view->priv->in_expose)
    {
        if (!view->priv->update_idle)
            view->priv->update_idle =
                    g_idle_add_full (G_PRIORITY_HIGH_IDLE,
                                     (GSourceFunc) invalidate_rectangle,
                                     view, NULL);
    }
    else
    {
        if (view->priv->update_idle)
            g_source_remove (view->priv->update_idle);
        view->priv->update_idle = 0;

        invalidate_rectangle (view);
    }
}


static void
set_draw_tabs (MooTextView    *view,
               gboolean        draw)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (BOOL_CMP (view->priv->draw_tabs, draw))
        return;

    view->priv->draw_tabs = draw != 0;
    g_object_notify (G_OBJECT (view), "draw-tabs");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


static void
set_draw_trailing_spaces (MooTextView    *view,
                          gboolean        draw)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (BOOL_CMP (view->priv->draw_trailing_spaces, draw))
        return;

    view->priv->draw_trailing_spaces = draw != 0;
    g_object_notify (G_OBJECT (view), "draw-trailing-spaces");

    if (GTK_WIDGET_DRAWABLE (view))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


GtkTextTag*
moo_text_view_lookup_tag (MooTextView    *view,
                          const char     *name)
{
    GtkTextBuffer *buffer;
    GtkTextTagTable *table;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    buffer = get_buffer (view);
    table = gtk_text_buffer_get_tag_table (buffer);

    return gtk_text_tag_table_lookup (table, name);
}


void
moo_text_view_set_cursor_color (MooTextView    *view,
                                const GdkColor *color)
{
    char *rc_string;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (!color)
    {
        gtk_widget_ensure_style (GTK_WIDGET (view));
        g_return_if_fail (GTK_WIDGET (view)->style != NULL);
        color = &GTK_WIDGET(view)->style->text[GTK_STATE_NORMAL];
    }

    g_return_if_fail (color != NULL);

    rc_string = g_strdup_printf (
        "style \"%p\"\n"
        "{\n"
        "   GtkWidget::cursor-color = \"#%02x%02x%02x\"\n"
        "}\n"
        "widget \"*.%s\" style \"%p\"\n",
        view,
        color->red >> 8, color->green >> 8, color->blue >> 8,
        gtk_widget_get_name (GTK_WIDGET (view)), view
    );

    gtk_rc_parse_string (rc_string);

    g_free (rc_string);
}


void
moo_text_view_set_lang (MooTextView    *view,
                        MooLang        *lang)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    moo_text_buffer_set_lang (get_moo_buffer (view), lang);
    gtk_widget_queue_draw (GTK_WIDGET (view));
}


MooLang*
moo_text_view_get_lang (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), NULL);
    return moo_text_buffer_get_lang (get_moo_buffer (view));
}


static void
moo_text_view_set_scheme_real (MooTextView      *view,
                               MooTextStyleScheme *scheme)
{
    GdkColor color;
    GdkColor *color_ptr;
    MooTextBuffer *buffer;
    GtkWidget *widget;

    g_return_if_fail (scheme != NULL);
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    widget = GTK_WIDGET (view);
    buffer = get_moo_buffer (view);
    gtk_widget_ensure_style (widget);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_text (widget, GTK_STATE_INSENSITIVE, color_ptr);
    moo_text_view_set_cursor_color (view, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_NORMAL, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_ACTIVE, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_PRELIGHT, color_ptr);
    gtk_widget_modify_base (widget, GTK_STATE_INSENSITIVE, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_FG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_FG]);
    }
    gtk_widget_modify_text (widget, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_SEL_BG], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_SEL_BG]);
    }
    gtk_widget_modify_base (widget, GTK_STATE_SELECTED, color_ptr);

    color_ptr = NULL;
    if (scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE])
    {
        if (gdk_color_parse (scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE], &color))
            color_ptr = &color;
        else
            g_warning ("%s: could not parse color '%s'", G_STRLOC,
                       scheme->text_colors[MOO_TEXT_COLOR_CUR_LINE]);
    }
    moo_text_view_set_current_line_color (view, color_ptr);

    moo_text_buffer_apply_scheme (buffer, scheme);
}


void
moo_text_view_set_scheme (MooTextView        *view,
                          MooTextStyleScheme *scheme)
{
    g_return_if_fail (MOO_IS_TEXT_STYLE_SCHEME (scheme));
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_signal_emit (view, signals[SET_SCHEME], 0, scheme);
}


void
moo_text_view_strip_whitespace (MooTextView *view)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    buffer = get_buffer (view);
    gtk_text_buffer_begin_user_action (buffer);

    for (gtk_text_buffer_get_start_iter (buffer, &iter);
         !gtk_text_iter_is_end (&iter);
         gtk_text_iter_forward_line (&iter))
    {
        GtkTextIter end;
        char *slice, *p;
        int len;

        if (gtk_text_iter_ends_line (&iter))
            continue;

        end = iter;
        gtk_text_iter_forward_to_line_end (&end);

        slice = gtk_text_buffer_get_slice (buffer, &iter, &end, TRUE);
        len = strlen (slice);
        g_assert (len > 0);

        for (p = slice + len; p > slice && (p[-1] == ' ' || p[-1] == '\t'); --p) ;

        if (*p)
        {
            gtk_text_iter_forward_chars (&iter, g_utf8_pointer_to_offset (slice, p));
            gtk_text_buffer_delete (buffer, &iter, &end);
        }

        g_free (slice);
    }

    gtk_text_buffer_end_user_action (buffer);
}


static void
moo_text_view_populate_popup (GtkTextView    *text_view,
                              GtkMenu        *menu)
{
    MooTextView *view = MOO_TEXT_VIEW (text_view);
    GtkWidget *item;

    item = gtk_separator_menu_item_new ();
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_text_view_redo), view);
    gtk_widget_set_sensitive (item, moo_text_view_can_redo (view));

    item = gtk_image_menu_item_new_from_stock (GTK_STOCK_UNDO, NULL);
    gtk_widget_show (item);
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
    g_signal_connect_swapped (item, "activate", G_CALLBACK (moo_text_view_undo), view);
    gtk_widget_set_sensitive (item, moo_text_view_can_undo (view));
}


static void
moo_text_view_set_scroll_adjustments (GtkTextView        *text_view,
                                      GtkAdjustment      *hadj,
                                      GtkAdjustment      *vadj)
{
    GtkAdjustment *old_adj;

    old_adj = text_view->vadjustment;

    if (old_adj)
        g_signal_handlers_disconnect_by_func (old_adj,
                                              (gpointer) invalidate_right_margin,
                                              text_view);


    if (vadj)
    {
        g_signal_connect_swapped (vadj, "changed",
                                  G_CALLBACK (invalidate_right_margin),
                                  text_view);
        g_signal_connect_swapped (vadj, "value-changed",
                                  G_CALLBACK (invalidate_right_margin),
                                  text_view);
    }

    GTK_TEXT_VIEW_CLASS(moo_text_view_parent_class)->
            set_scroll_adjustments (text_view, hadj, vadj);

    invalidate_right_margin (MOO_TEXT_VIEW (text_view));
}


static void
overwrite_changed (MooTextView *view)
{
    GtkTextView *text_view;

    text_view = GTK_TEXT_VIEW (view);

    if (text_view->overwrite_mode)
    {
        view->priv->saved_cursor_visible = text_view->cursor_visible != 0;
        gtk_text_view_set_cursor_visible (text_view, FALSE);
        view->priv->overwrite_mode = TRUE;
    }
    else
    {
        gtk_text_view_set_cursor_visible (text_view,
                                          view->priv->saved_cursor_visible);
        view->priv->overwrite_mode = FALSE;
    }

    check_cursor_blink (view);
}


static gboolean
moo_text_view_focus_in (GtkWidget          *widget,
                        GdkEventFocus      *event)
{
    gboolean ret;
    ret = GTK_WIDGET_CLASS(moo_text_view_parent_class)->focus_in_event (widget, event);
    check_cursor_blink (MOO_TEXT_VIEW (widget));
    return ret;
}


static gboolean
moo_text_view_focus_out (GtkWidget          *widget,
                         GdkEventFocus      *event)
{
    gboolean ret;
    ret = GTK_WIDGET_CLASS(moo_text_view_parent_class)->focus_out_event (widget, event);
    check_cursor_blink (MOO_TEXT_VIEW (widget));
    return ret;
}


#define CURSOR_ON_MULTIPLIER 0.66
#define CURSOR_OFF_MULTIPLIER 0.34
#define CURSOR_PEND_MULTIPLIER 0.5

static gboolean
get_cursor_rectangle (GtkTextView  *view,
                      GdkRectangle *cursor_rect)
{
    GtkTextIter iter;
    GdkRectangle visible_rect;
    GtkTextBuffer *buffer;
    GtkTextMark *insert;

    buffer = gtk_text_view_get_buffer (view);
    insert = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, insert);

    gtk_text_view_get_iter_location (view, &iter, cursor_rect);
    gtk_text_view_get_visible_rect (view, &visible_rect);

    if (gtk_text_iter_ends_line (&iter))
        cursor_rect->width = 6;

    if (gdk_rectangle_intersect (cursor_rect, &visible_rect, cursor_rect))
    {
        gtk_text_view_buffer_to_window_coords (view, GTK_TEXT_WINDOW_TEXT,
                                               cursor_rect->x, cursor_rect->y,
                                               &cursor_rect->x, &cursor_rect->y);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void
moo_text_view_draw_cursor (GtkTextView    *view,
                           GdkEventExpose *event)
{
    GdkRectangle cursor_rect;

    if (!get_cursor_rectangle (view, &cursor_rect))
        return;

    if (!gdk_rectangle_intersect (&cursor_rect, &event->area, &cursor_rect))
        return;

    gdk_draw_rectangle (event->window,
                        GTK_WIDGET(view)->style->text_gc[GTK_STATE_NORMAL],
                        TRUE,
                        cursor_rect.x,
                        cursor_rect.y,
                        cursor_rect.width,
                        cursor_rect.height);
}

static void
invalidate_cursor (GtkTextView *view)
{
    GdkRectangle rect;

    if (get_cursor_rectangle (view, &rect))
    {
        GdkWindow *window = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
        g_return_if_fail (window != NULL);
        gdk_window_invalidate_rect (window, &rect, FALSE);
    }
}

static gboolean
cursor_blinks (GtkWidget *widget)
{
    gboolean blink;
    GtkSettings *settings = gtk_widget_get_settings (widget);
    g_object_get (settings, "gtk-cursor-blink", &blink, NULL);
    return blink;
}

static int
get_cursor_time (GtkWidget *widget)
{
    int time;
    GtkSettings *settings = gtk_widget_get_settings (widget);
    g_object_get (settings, "gtk-cursor-blink-time", &time, NULL);
    return time;
}

static gboolean
blink_cb (MooTextView *view)
{
    GtkTextView *text_view = GTK_TEXT_VIEW (view);
    int time;

    g_return_val_if_fail (view->priv->overwrite_mode, FALSE);

    time = get_cursor_time (GTK_WIDGET (view));

    if (view->priv->cursor_visible)
        time *= CURSOR_OFF_MULTIPLIER;
    else
        time *= CURSOR_ON_MULTIPLIER;

    view->priv->blink_timeout = g_timeout_add (time, (GSourceFunc) blink_cb, view);
    view->priv->cursor_visible = !view->priv->cursor_visible;
    invalidate_cursor (text_view);

    return FALSE;
}


static void
stop_cursor_blink (MooTextView *view)
{
    if (view->priv->blink_timeout)
    {
        g_source_remove (view->priv->blink_timeout);
        view->priv->blink_timeout = 0;
    }
}

static void
check_cursor_blink (MooTextView *view)
{
    GtkTextView *text_view = GTK_TEXT_VIEW (view);

    if (view->priv->overwrite_mode && GTK_WIDGET_HAS_FOCUS (view))
    {
        if (cursor_blinks (GTK_WIDGET (view)))
        {
            if (!view->priv->blink_timeout)
            {
                int time = get_cursor_time (GTK_WIDGET (view)) * CURSOR_OFF_MULTIPLIER;
                view->priv->cursor_visible = TRUE;
                view->priv->blink_timeout = g_timeout_add (time, (GSourceFunc) blink_cb, view);
            }
        }
        else
        {
            view->priv->cursor_visible = TRUE;
        }
    }
    else
    {
        view->priv->cursor_visible = FALSE;
        stop_cursor_blink (view);
        if (GTK_WIDGET_DRAWABLE (text_view))
            invalidate_cursor (text_view);
    }
}

void
_moo_text_view_pend_cursor_blink (MooTextView *view)
{
    if (view->priv->overwrite_mode &&
        GTK_WIDGET_HAS_FOCUS (view) &&
        cursor_blinks (GTK_WIDGET (view)))
    {
        int time;

        if (view->priv->blink_timeout != 0)
        {
            g_source_remove (view->priv->blink_timeout);
            view->priv->blink_timeout = 0;
        }

        view->priv->cursor_visible = TRUE;

        time = get_cursor_time (GTK_WIDGET (view)) * CURSOR_PEND_MULTIPLIER;
        view->priv->blink_timeout = g_timeout_add (time, (GSourceFunc) blink_cb, view);
    }
}


void
moo_text_view_set_tab_key_action (MooTextView        *view,
                                  MooTextTabKeyAction action)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->tab_key_action != action)
    {
        view->priv->tab_key_action = action;
        g_object_notify (G_OBJECT (view), "tab-key-action");
    }
}


int
_moo_text_view_get_line_height (MooTextView *view)
{
    PangoContext *ctx;
    PangoLayout *layout;
    PangoRectangle rect;
    int height;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), 10);

    ctx = gtk_widget_get_pango_context (GTK_WIDGET (view));
    g_return_val_if_fail (ctx != NULL, 10);

    layout = pango_layout_new (ctx);
    pango_layout_set_text (layout, "AA", -1);

    pango_layout_get_extents (layout, NULL, &rect);
    height = rect.height / PANGO_SCALE;

    g_object_unref (layout);
    return height;
}


/*****************************************************************************/
/* Left margin
 */

#define LINE_MARK_XPAD 2
#define LINE_NUMBERS_XPAD 2

void
moo_text_view_set_show_line_numbers (MooTextView *view,
                                     gboolean     show)
{
    gboolean show_margin;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    show = show != 0;

    if (view->priv->show_line_numbers == show)
        return;

    view->priv->show_line_numbers = show;
    view->priv->digit_width = 0;

    show_margin = get_show_left_margin (view);

    if (GTK_WIDGET_REALIZED (view) && show_margin)
    {
        int margin_width = get_left_margin_width (view);
        gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                              GTK_TEXT_WINDOW_LEFT,
                                              margin_width);
    }
    else
    {
        gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                              GTK_TEXT_WINDOW_LEFT,
                                              show_margin ? 4 : 0);
    }

    if (show_margin)
        /* XXX dont' invalidate whole widget */
        gtk_widget_queue_draw (GTK_WIDGET (view));

    g_object_notify (G_OBJECT (view), "show-line-numbers");
}


void
moo_text_view_set_show_line_marks (MooTextView *view,
                                   gboolean     show)
{
    gboolean show_margin;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    show = show != 0;

    if (view->priv->show_line_marks == show)
        return;

    view->priv->show_line_marks = show;

    show_margin = get_show_left_margin (view);

    if (GTK_WIDGET_REALIZED (view) && show_margin)
    {
        int margin_width = get_left_margin_width (view);
        gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                              GTK_TEXT_WINDOW_LEFT,
                                              margin_width);
    }
    else
    {
        gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                              GTK_TEXT_WINDOW_LEFT,
                                              show_margin ? 4 : 0);
    }

    if (show_margin)
        /* XXX dont' invalidate whole widget */
        gtk_widget_queue_draw (GTK_WIDGET (view));

    g_object_notify (G_OBJECT (view), "show-line-marks");
}


static PangoLayout *
create_line_numbers_layout (MooTextView *view)
{
    PangoLayout *layout;

    layout = gtk_widget_create_pango_layout (GTK_WIDGET (view), "");

    if (!view->priv->line_numbers_font)
        view->priv->line_numbers_font = pango_font_description_from_string ("Sans 10");

    if (view->priv->line_numbers_font)
        pango_layout_set_font_description (layout, view->priv->line_numbers_font);

    return layout;
}


static int
calculate_digit_width (MooTextView *view,
                       PangoLayout *layout)
{
    int i;
    char str[32];
    int digit_width;

    if (view->priv->digit_width > 0)
        return view->priv->digit_width;

    if (!layout)
        layout = create_line_numbers_layout (view);
    else
        g_object_ref (layout);

    for (i = 0, digit_width = 0; i < 10; ++i)
    {
        int width;

//         if (view->priv->bold_current_line_number)
//             g_snprintf (str, sizeof (str), "<b>%d</b>", i);
//         else
            g_snprintf (str, sizeof (str), "%d", i);

        pango_layout_set_markup (layout, str, -1);
        pango_layout_get_pixel_size (layout, &width, NULL);
        digit_width = MAX (digit_width, width);
    }

    g_object_unref (layout);
    return view->priv->digit_width = digit_width;
}


static int
get_left_margin_width (MooTextView *view)
{
    GtkTextView *text_view;
    GtkTextBuffer *buffer;
    int margin_width, text_width;
    char str[32];

    text_view = GTK_TEXT_VIEW (view);
    buffer = gtk_text_view_get_buffer (text_view);

    margin_width = text_width = 0;

    if (view->priv->show_line_numbers)
    {
        int digit_width, digits;

        digit_width = calculate_digit_width (view, NULL);
        g_snprintf (str, sizeof(str), "%d",
                    MAX (99, gtk_text_buffer_get_line_count (buffer)));
        digits = strlen (str);
        text_width = digits * digit_width;

        margin_width += text_width + 2*LINE_NUMBERS_XPAD;
    }

    if (view->priv->show_line_marks)
        margin_width += view->priv->line_mark_width + 2*LINE_MARK_XPAD;

    if (view->priv->enable_folding)
        margin_width += view->priv->fold_margin_width;

    return margin_width;
}


static void
moo_text_view_style_set (GtkWidget *widget,
                         GtkStyle  *prev_style)
{
    MooTextView *view = MOO_TEXT_VIEW (widget);

    gtk_widget_style_get (widget,
                          "expander-size", &view->priv->expander_size,
                          NULL);
    view->priv->expander_size += EXPANDER_PADDING;

    update_box_tag (view);
    update_tab_width (view);

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->style_set (widget, prev_style);
}


void
moo_text_view_set_enable_folding (MooTextView *view,
                                  gboolean     show)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    show = show != 0;

    if (show == view->priv->enable_folding)
        return;

    if (show)
    {
        GtkTextTagTable *table = gtk_text_buffer_get_tag_table (get_buffer (view));
        if (!gtk_text_tag_table_lookup (table, MOO_FOLD_TAG))
            gtk_text_buffer_create_tag (get_buffer (view), MOO_FOLD_TAG,
                                        "invisible", TRUE, NULL);
        view->priv->fold_margin_width = view->priv->expander_size;
    }
    else
    {
        view->priv->fold_margin_width = 0;
    }

    view->priv->enable_folding = show;
    gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                          GTK_TEXT_WINDOW_LEFT,
                                          get_left_margin_width (view));
}


static void
update_tab_width (MooTextView *view)
{
    PangoTabArray *tabs;
    PangoLayout *layout;
    int tab_width;
    char *string;

    if (!GTK_WIDGET_REALIZED (view))
        return;

    g_return_if_fail (view->priv->tab_width > 0);
    g_return_if_fail (GTK_WIDGET (view)->style != NULL);

    string = g_strnfill (view->priv->tab_width, ' ');
    layout = gtk_widget_create_pango_layout (GTK_WIDGET (view), string);
    pango_layout_get_size (layout, &tab_width, NULL);

    tabs = pango_tab_array_new (2, FALSE);
    pango_tab_array_set_tab (tabs, 0, PANGO_TAB_LEFT, 0);
    pango_tab_array_set_tab (tabs, 1, PANGO_TAB_LEFT, tab_width);

    gtk_text_view_set_tabs (GTK_TEXT_VIEW (view), tabs);

    pango_tab_array_free (tabs);
    g_object_unref (layout);
    g_free (string);
}

void
moo_text_view_set_tab_width (MooTextView *view,
                             guint        width)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (width > 0);

    if (width == view->priv->tab_width)
        return;

    view->priv->tab_width = width;
    update_tab_width (view);

    g_object_notify (G_OBJECT (view), "tab-width");
}


static void
draw_marks (GdkEventExpose *event,
            GSList         *marks,
            int             line_y,
            int             line_height)
{
    GdkPixbuf *pixbuf, *composite;
    int width, height;
    MooLineMark *mark;

    g_return_if_fail (marks != NULL);

    composite = NULL;
    width = height = 0;

    while (marks && !_moo_line_mark_get_pretty (marks->data))
        marks = marks->next;

    if (!marks)
        return;

    mark = marks->data;

    while (TRUE)
    {
        pixbuf = moo_line_mark_get_pixbuf (mark);

        if (pixbuf)
        {
            if (!composite)
            {
                composite = gdk_pixbuf_copy (pixbuf);
                width = gdk_pixbuf_get_width (composite);
                height = gdk_pixbuf_get_height (composite);
            }
            else
            {
                gint pixbuf_w;
                gint pixbuf_h;

                pixbuf_w = gdk_pixbuf_get_width (pixbuf);
                pixbuf_h = gdk_pixbuf_get_height (pixbuf);
#define COMPOSITE_ALPHA 225 /* XXX ??? */
                gdk_pixbuf_composite (pixbuf,
                                      composite,
                                      0, 0,
                                      width, height,
                                      0, 0,
                                      (double) pixbuf_w / width,
                                      (double) pixbuf_h / height,
                                      GDK_INTERP_BILINEAR,
                                      COMPOSITE_ALPHA);
            }
        }

        marks = marks->next;

        while (marks && !_moo_line_mark_get_pretty (marks->data))
            marks = marks->next;

        if (!marks)
            break;

        mark = marks->data;
    }

    if (composite)
    {
        line_y += (line_height - height) / 2;
        gdk_draw_pixbuf (event->window, NULL, composite,
                         0, 0,
                         LINE_MARK_XPAD,
                         line_y,
                         width, height,
                         GDK_RGB_DITHER_NORMAL, 0, 0);
        g_object_unref (composite);
    }
}


static void
draw_fold_mark (MooTextView    *view,
                GdkEventExpose *event,
                MooFold        *fold,
                int             y,
                int             height,
                int             window_width)
{
    gtk_paint_expander (GTK_WIDGET(view)->style,
                        event->window,
                        GTK_WIDGET_STATE (view),
                        &event->area,
                        GTK_WIDGET(view),
                        "fold",
                        window_width - view->priv->fold_margin_width / 2,
                        y + height / 2,
                        fold->collapsed ? GTK_EXPANDER_COLLAPSED : GTK_EXPANDER_EXPANDED);
}


static gboolean
text_iter_forward_visible_line (MooTextView *view,
                                GtkTextIter *iter,
                                int         *line)
{
    if (!view->priv->enable_folding)
    {
        if (!gtk_text_iter_forward_line (iter))
        {
            if (gtk_text_iter_get_line (iter) == *line)
                return FALSE;
        }

        *line += 1;
        return TRUE;
    }
    else
    {
        GtkTextTagTable *table = gtk_text_buffer_get_tag_table (get_buffer (view));
        GtkTextTag *tag = gtk_text_tag_table_lookup (table, MOO_FOLD_TAG);

        g_return_val_if_fail (tag != NULL, FALSE);

        while (TRUE)
        {
            if (!gtk_text_iter_forward_line (iter))
            {
                if (gtk_text_iter_get_line (iter) == *line)
                    return FALSE;
            }

            *line += 1;

            if (!gtk_text_iter_has_tag (iter, tag) && !gtk_text_iter_begins_tag (iter, tag))
                return TRUE;
        }
    }
}


static void
draw_left_margin (MooTextView    *view,
                  GdkEventExpose *event)
{
    GtkTextView *text_view;
    GtkTextBuffer *buffer;
    PangoLayout *layout = NULL;
    int line, last_line, current_line, text_width, window_width;
    GdkRectangle area;
    GtkTextIter iter;
    char str[32];

    text_view = GTK_TEXT_VIEW (view);
    buffer = gtk_text_view_get_buffer (text_view);
    gdk_drawable_get_size (event->window, &window_width, NULL);
    text_width = 0;

    if (view->priv->show_line_numbers)
    {
        int digit_width, digits;

        layout = create_line_numbers_layout (view);

        digit_width = calculate_digit_width (view, layout);
        g_snprintf (str, sizeof(str), "%d",
                    MAX (99, gtk_text_buffer_get_line_count (buffer)));
        digits = strlen (str);
        text_width = digits * digit_width;

        pango_layout_set_width (layout, text_width);
        pango_layout_set_alignment (layout, PANGO_ALIGN_RIGHT);
    }

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    current_line = gtk_text_iter_get_line (&iter);

    area = event->area;
    gtk_text_view_window_to_buffer_coords (text_view,
                                           GTK_TEXT_WINDOW_LEFT,
                                           area.x, area.y,
                                           &area.x, &area.y);

    gtk_text_view_get_line_at_y (text_view, &iter, area.y + area.height - 1, NULL);
    last_line = gtk_text_iter_get_line (&iter);

    gtk_text_view_get_line_at_y (text_view, &iter, area.y, NULL);
    line = gtk_text_iter_get_line (&iter);

    while (TRUE)
    {
        int y, height;

        gtk_text_view_get_line_yrange (text_view, &iter, &y, &height);

        if (y > area.y + area.height)
            break;

        gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_LEFT,
                                               0, y, NULL, &y);

        if (view->priv->show_line_numbers)
        {
            if (line == current_line && view->priv->bold_current_line_number)
                g_snprintf (str, sizeof(str), "<b>%d</b>", line + 1);
            else
                g_snprintf (str, sizeof(str), "%d", line + 1);

            pango_layout_set_markup (layout, str, -1);
            gtk_paint_layout (GTK_WIDGET(view)->style,
                              event->window, GTK_WIDGET_STATE (view),
                              TRUE, &event->area,
                              GTK_WIDGET(view),
                              NULL,
                              window_width - LINE_NUMBERS_XPAD - view->priv->fold_margin_width,
                              y,
                              layout);
        }

        if (view->priv->enable_folding)
        {
            MooFold *fold = moo_text_buffer_get_fold_at_line (get_moo_buffer (view), line);

            if (fold)
                draw_fold_mark (view, event, fold, y, height, window_width);
        }

        if (view->priv->show_line_marks)
        {
            GSList *marks = moo_text_buffer_get_line_marks_at_line (get_moo_buffer (view), line);

            if (marks)
                draw_marks (event, marks, y, height);

            g_slist_free (marks);
        }

        if (!text_iter_forward_visible_line (view, &iter, &line))
            break;
    }

    if (layout)
        g_object_unref (layout);
}


static void
draw_fold_background (MooTextView    *view,
                      GdkEventExpose *event,
                      MooFold        *fold,
                      int             y,
                      int             height,
                      int             window_width)
{
    if (fold->collapsed)
        gdk_draw_line (event->window,
                       GTK_WIDGET(view)->style->text_gc[GTK_STATE_NORMAL],
                       gtk_text_view_get_left_margin (GTK_TEXT_VIEW (view)),
                       y + height - 1,
                       gtk_text_view_get_left_margin (GTK_TEXT_VIEW (view)) + window_width,
                       y + height - 1);
}


static void
draw_line_marks_background (MooTextView    *view,
                            GdkEventExpose *event)
{
    GtkTextView *text_view;
    GtkTextBuffer *buffer;
    int line, last_line, window_width;
    GdkRectangle area;
    GtkTextIter iter;

    text_view = GTK_TEXT_VIEW (view);
    buffer = gtk_text_view_get_buffer (text_view);

    area = event->area;
    gtk_text_view_window_to_buffer_coords (text_view,
                                           GTK_TEXT_WINDOW_TEXT,
                                           area.x, area.y,
                                           &area.x, &area.y);

    gdk_drawable_get_size (event->window, &window_width, NULL);
    window_width -= gtk_text_view_get_left_margin (text_view) +
                    gtk_text_view_get_right_margin (text_view);

    gtk_text_view_get_line_at_y (text_view, &iter, area.y + area.height - 1, NULL);
    last_line = gtk_text_iter_get_line (&iter);

    gtk_text_view_get_line_at_y (text_view, &iter, area.y, NULL);
    line = gtk_text_iter_get_line (&iter);

    while (TRUE)
    {
        int y, height;

        gtk_text_view_get_line_yrange (text_view, &iter, &y, &height);

        if (y > area.y + area.height)
            break;

        gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_LEFT,
                                               0, y, NULL, &y);

        if (view->priv->enable_folding)
        {
            MooFold *fold = moo_text_buffer_get_fold_at_line (get_moo_buffer (view), line);

            if (fold)
                draw_fold_background (view, event, fold, y, height, window_width);
        }

        if (view->priv->show_line_marks)
        {
            MooLineMark *mark;
            GdkGC *gc = NULL;
            GSList *marks = moo_text_buffer_get_line_marks_at_line (get_moo_buffer (view), line);

            if (marks)
            {
                while (marks)
                {
                    mark = marks->data;

                    if (_moo_line_mark_get_pretty (mark))
                        gc = moo_line_mark_get_background_gc (mark);

                    if (!gc)
                        marks = g_slist_delete_link (marks, marks);
                    else
                        break;
                }

                /* XXX compose colors */
                if (gc)
                {
                    gdk_gc_set_clip_rectangle (gc, &event->area);
                    gdk_draw_rectangle (event->window,
                                        gc,
                                        TRUE,
                                        gtk_text_view_get_left_margin (text_view),
                                        y,
                                        window_width,
                                        height);
                }
            }

            g_slist_free (marks);
        }

        if (!text_iter_forward_visible_line (view, &iter, &line))
            break;
    }
}


static void
draw_line_numbers_and_marks (MooTextView    *view,
                             GdkEventExpose *event,
                             GdkWindow      *text_window,
                             GdkWindow      *left_window)
{
    if (event->window == text_window)
        draw_line_marks_background (view, event);
    else if (event->window == left_window)
        draw_left_margin (view, event);
    else
        g_return_if_reached ();
}


/*****************************************************************************/
/* Right margin
 */

#define SCROLLBAR_MARK_WIDTH 4
#define SCROLLBAR_MARK_HEIGHT 1
#define SCROLLBAR_MARK_XPAD 1
#define SCROLLBAR_MARK_YPAD 4

void
moo_text_view_set_show_scrollbar_marks (MooTextView *view,
                                        gboolean     show)
{
    gboolean show_margin;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    show = show != 0;

    if (view->priv->show_scrollbar_marks == show)
        return;

    view->priv->show_scrollbar_marks = show;

    show_margin = get_show_right_margin (view);

    gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                          GTK_TEXT_WINDOW_RIGHT,
                                          show_margin ? 4 : 0); /* something, to get expose */

    invalidate_right_margin (view);

    g_object_notify (G_OBJECT (view), "show-scrollbar-marks");
}


static void
draw_right_margin (MooTextView        *view,
                   GdkEventExpose     *event)
{
    GtkTextView *text_view;
    GtkTextBuffer *buffer;
    GdkWindow *window;
    int line_y, line_height, current_line, margin_width, window_height;
    double pos;
    GtkTextIter iter;
    GtkAdjustment *adj;

    text_view = GTK_TEXT_VIEW (view);
    buffer = gtk_text_view_get_buffer (text_view);
    window = gtk_text_view_get_window (text_view, GTK_TEXT_WINDOW_RIGHT);
    adj = text_view->vadjustment;

    if (!get_show_right_margin (view) || event->window != window)
        return;

    if (!adj || adj->upper - adj->page_size <= 0)
        return;

    margin_width = 0;

    if (view->priv->show_scrollbar_marks)
        margin_width = SCROLLBAR_MARK_WIDTH + 2*SCROLLBAR_MARK_XPAD;

    g_return_if_fail (margin_width > 0);

    gtk_text_view_set_border_window_size (text_view,
                                          GTK_TEXT_WINDOW_RIGHT,
                                          margin_width);
    gdk_drawable_get_size (window, NULL, &window_height);
    window_height = MAX (window_height, 2*SCROLLBAR_MARK_YPAD + SCROLLBAR_MARK_HEIGHT);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    current_line = gtk_text_iter_get_line (&iter);
    gtk_text_view_get_line_yrange (text_view, &iter, &line_y, &line_height);

    pos = line_y + line_height / 2;
    pos = pos * window_height / MAX (adj->upper, 1);
    line_y = pos;

    line_y = CLAMP (line_y + SCROLLBAR_MARK_YPAD, SCROLLBAR_MARK_YPAD,
                    window_height - SCROLLBAR_MARK_HEIGHT - 2*SCROLLBAR_MARK_YPAD) + SCROLLBAR_MARK_YPAD;

    gdk_draw_rectangle (window,
                        GTK_WIDGET(view)->style->text_gc[GTK_WIDGET_STATE(view)],
                        TRUE,
                        SCROLLBAR_MARK_XPAD,
                        line_y,
                        SCROLLBAR_MARK_WIDTH,
                        SCROLLBAR_MARK_HEIGHT);
}


static void
invalidate_right_margin (MooTextView *view)
{
    if (GTK_WIDGET_DRAWABLE (view) && get_show_right_margin (view))
    {
        GdkWindow *window;
        GdkRectangle rect = {0, 0, 0, 0};

        window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                           GTK_TEXT_WINDOW_RIGHT);

        if (window)
        {
            gdk_drawable_get_size (window, &rect.width, &rect.height);
            gdk_window_invalidate_rect (window, &rect, FALSE);
        }
    }
}


static void
invalidate_line (MooTextView *view,
                 int          line,
                 gboolean     left,
                 gboolean     text)
{
    GtkTextIter iter;
    GdkRectangle rect = {0, 0, 0, 0};

    if (!GTK_WIDGET_DRAWABLE (view))
        return;

    gtk_text_buffer_get_iter_at_line (get_buffer (view), &iter, line);
    gtk_text_view_get_line_yrange (GTK_TEXT_VIEW (view), &iter, &rect.y, &rect.height);

    gtk_text_view_buffer_to_window_coords (GTK_TEXT_VIEW (view),
                                           GTK_TEXT_WINDOW_TEXT,
                                           0, rect.y, NULL, &rect.y);

    if (left)
    {
        GdkWindow *window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                                      GTK_TEXT_WINDOW_LEFT);
        if (window)
        {
            gdk_drawable_get_size (window, &rect.width, NULL);
            gdk_window_invalidate_rect (window, &rect, FALSE);
        }
    }

    if (text)
    {
        GdkWindow *window = gtk_text_view_get_window (GTK_TEXT_VIEW (view),
                                                      GTK_TEXT_WINDOW_TEXT);
        if (window)
        {
            gdk_drawable_get_size (window, &rect.width, NULL);
            gdk_window_invalidate_rect (window, &rect, FALSE);
        }
    }
}


static void
line_mark_deleted (MooTextView *view,
                   MooLineMark *mark)
{
    if (_moo_line_mark_get_pretty (mark))
    {
        _moo_line_mark_set_pretty (mark, FALSE);
        view->priv->line_marks = g_slist_remove (view->priv->line_marks, mark);
        g_signal_handlers_disconnect_by_func (mark, (gpointer) line_mark_changed, view);
        g_object_unref (mark);
        update_line_mark_width (view);
        gtk_widget_queue_draw (GTK_WIDGET (view));
    }
}


static void
line_mark_changed (MooTextView *view,
                   MooLineMark *mark)
{
    update_line_mark_width (view);
    invalidate_line (view, moo_line_mark_get_line (mark), TRUE, TRUE);
}


static void
line_mark_moved (MooTextView *view,
                 MooLineMark *mark)
{
    /* XXX */
    if (_moo_line_mark_get_pretty (mark))
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


static void
line_mark_added (MooTextView *view,
                 MooLineMark *mark)
{
    g_return_if_fail (!g_slist_find (view->priv->line_marks, mark));

    if (!moo_line_mark_get_visible (mark))
        return;

    _moo_line_mark_set_pretty (mark, TRUE);
    view->priv->line_marks = g_slist_prepend (view->priv->line_marks,
                                              g_object_ref (mark));
    g_signal_connect_swapped (mark, "changed",
                              G_CALLBACK (line_mark_changed), view);

    if (GTK_WIDGET_REALIZED (view))
        _moo_line_mark_realize (mark, GTK_WIDGET (view));

    update_line_mark_width (view);
    invalidate_line (view, moo_line_mark_get_line (mark), TRUE, TRUE);
}


static void
fold_added (MooTextView        *view,
            MooFold            *fold)
{
    if (view->priv->enable_folding)
        invalidate_line (view, _moo_fold_get_start (fold), TRUE, fold->collapsed);
}


static void
fold_deleted (MooTextView *view)
{
    if (view->priv->enable_folding)
        gtk_widget_queue_draw (GTK_WIDGET (view));
}


static void
fold_toggled (MooTextView        *view,
              MooFold            *fold)
{
    if (view->priv->enable_folding)
        invalidate_line (view, _moo_fold_get_start (fold), TRUE, TRUE);
}


/***************************************************************************/
/* Children
 */

/* http://bugzilla.gnome.org/show_bug.cgi?id=323843 */
static int
get_border_window_size (GtkTextView      *text_view,
                        GtkTextWindowType type)
{
    if (GTK_WIDGET_REALIZED (text_view) && gtk_text_view_get_window (text_view, type))
        return gtk_text_view_get_border_window_size (text_view, type);
    else
        return 0;
}


void
moo_text_view_add_child_in_border (MooTextView        *view,
                                   GtkWidget          *widget,
                                   GtkTextWindowType   which_border)
{
    MooTextViewPos pos = MOO_TEXT_VIEW_POS_INVALID;
    GtkWidget **child;
    GtkRequisition child_req = {0, 0};
    int border_size = 0;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (widget->parent == NULL);

    switch (which_border)
    {
        case GTK_TEXT_WINDOW_LEFT:
            pos = MOO_TEXT_VIEW_POS_LEFT;
            break;
        case GTK_TEXT_WINDOW_RIGHT:
            pos = MOO_TEXT_VIEW_POS_RIGHT;
            break;
        case GTK_TEXT_WINDOW_TOP:
            pos = MOO_TEXT_VIEW_POS_TOP;
            break;
        case GTK_TEXT_WINDOW_BOTTOM:
            pos = MOO_TEXT_VIEW_POS_BOTTOM;
            break;
        default:
            g_return_if_reached ();
    }

    g_return_if_fail (pos < MOO_TEXT_VIEW_POS_INVALID);

    child = &view->priv->children[pos];
    g_return_if_fail (*child == NULL);

    gtk_object_sink (g_object_ref (widget));

    *child = widget;

    if (GTK_WIDGET_VISIBLE (widget))
    {
        gtk_widget_size_request (widget, &child_req);

        switch (which_border)
        {
            case GTK_TEXT_WINDOW_LEFT:
            case GTK_TEXT_WINDOW_RIGHT:
                border_size = child_req.width;
                break;
            case GTK_TEXT_WINDOW_TOP:
            case GTK_TEXT_WINDOW_BOTTOM:
                border_size = child_req.height;
                break;
            default:
                g_assert_not_reached ();
        }

        gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                              which_border, MIN (1, border_size));
        lower_border_window (GTK_TEXT_VIEW (view), pos);
    }

    gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (view), widget,
                                       GTK_TEXT_WINDOW_WIDGET, 0, 0);
}


static void
moo_text_view_size_request (GtkWidget      *widget,
                            GtkRequisition *requisition)
{
    guint i;
    MooTextView *view;
    GtkTextView *text_view;

    view = MOO_TEXT_VIEW (widget);
    text_view = GTK_TEXT_VIEW (widget);

    for (i = 0; i < 4; i++)
    {
        int border_size = 0;
        GtkWidget *child = view->priv->children[i];
        GtkRequisition child_req;

        if (child && GTK_WIDGET_VISIBLE (child))
            gtk_widget_size_request (child, &child_req);
        else
            child_req.width = child_req.height = 0;

        if (child)
        {
            int old_size;

            switch (i)
            {
                case MOO_TEXT_VIEW_POS_LEFT:
                case MOO_TEXT_VIEW_POS_RIGHT:
                    border_size = child_req.width;
                    break;
                case MOO_TEXT_VIEW_POS_TOP:
                case MOO_TEXT_VIEW_POS_BOTTOM:
                    border_size = child_req.height;
                    break;
            }

            old_size = get_border_window_size (text_view,
                                               window_types[i]);
            gtk_text_view_set_border_window_size (text_view,
                                                  window_types[i],
                                                  border_size);
            if (!old_size)
                lower_border_window (GTK_TEXT_VIEW (view), i);
        }
    }

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->size_request (widget, requisition);
}


static void
moo_text_view_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
    guint i;
    int right, left, bottom, top, border_width;
    MooTextView *view;
    GtkTextView *text_view;

    view = MOO_TEXT_VIEW (widget);
    text_view = GTK_TEXT_VIEW (widget);

    GTK_WIDGET_CLASS(moo_text_view_parent_class)->size_allocate (widget, allocation);

    border_width = GTK_CONTAINER(widget)->border_width;

    right = get_border_window_size (text_view, GTK_TEXT_WINDOW_RIGHT);
    left = get_border_window_size (text_view, GTK_TEXT_WINDOW_LEFT);
    bottom = get_border_window_size (text_view, GTK_TEXT_WINDOW_BOTTOM);
    top = get_border_window_size (text_view, GTK_TEXT_WINDOW_TOP);

    for (i = 0; i < 4; i++)
    {
        GtkWidget *child = view->priv->children[i];
        GtkAllocation child_alloc = {left + border_width, top + border_width, 0, 0};
        GtkRequisition child_req;

        if (!child || !GTK_WIDGET_VISIBLE (child))
            continue;

        gtk_widget_get_child_requisition (child, &child_req);

        switch (i)
        {
            case MOO_TEXT_VIEW_POS_RIGHT:
                child_alloc.x = MAX (allocation->width - border_width - right, 0);
            case MOO_TEXT_VIEW_POS_LEFT:
                child_alloc.width = child_req.width;
                child_alloc.height = MAX (allocation->height - 2*border_width - top - bottom, 1);
                break;

            case MOO_TEXT_VIEW_POS_BOTTOM:
                child_alloc.y = MAX (allocation->height - bottom - border_width, 0);
            case MOO_TEXT_VIEW_POS_TOP:
                child_alloc.height = child_req.height;
                child_alloc.width = MAX (allocation->width - 2*border_width - left - right, 1);
                break;
        }

        gtk_text_view_move_child (text_view, child, child_alloc.x, child_alloc.y);
        gtk_widget_size_allocate (child, &child_alloc);
    }
}


static void
moo_text_view_remove (GtkContainer *container,
                      GtkWidget    *widget)
{
    guint i;
    MooTextView *view = MOO_TEXT_VIEW (container);

    if (MOO_IS_TEXT_BOX (widget))
        view->priv->boxes = g_slist_remove (view->priv->boxes, widget);

    if (widget == view->priv->qs.evbox)
    {
        view->priv->qs.in_search = FALSE;
        view->priv->qs.evbox = NULL;
        view->priv->qs.entry = NULL;
        view->priv->qs.case_sensitive = NULL;
        view->priv->qs.regex = NULL;
    }

    for (i = 0; i < 4; ++i)
    {
        if (view->priv->children[i] == widget)
        {
            view->priv->children[i] = NULL;
            g_object_unref (widget);
            gtk_text_view_set_border_window_size (GTK_TEXT_VIEW (view),
                                                  window_types[i], 0);
            break;
        }
    }

    GTK_CONTAINER_CLASS(moo_text_view_parent_class)->remove (container, widget);
}


/***************************************************************************/
/* Search
 */

static void
quick_search_option_toggled (MooTextView *view)
{
    MooTextSearchFlags flags = 0;

    if (!gtk_toggle_button_get_active (view->priv->qs.case_sensitive))
        flags |= MOO_TEXT_SEARCH_CASELESS;
    if (gtk_toggle_button_get_active (view->priv->qs.regex))
        flags |= MOO_TEXT_SEARCH_REGEX;

    moo_text_view_set_quick_search_flags (view, flags);

    if (MOO_IS_EDIT (view))
        moo_prefs_set_flags (moo_edit_setting (MOO_EDIT_PREFS_QUICK_SEARCH_FLAGS), flags);
}


static void
quick_search_set_widgets_from_flags (MooTextView *view)
{
    g_signal_handlers_block_by_func (view->priv->qs.case_sensitive,
                                     quick_search_option_toggled, view);
    g_signal_handlers_block_by_func (view->priv->qs.regex,
                                     quick_search_option_toggled, view);

    gtk_toggle_button_set_active (view->priv->qs.case_sensitive,
                                  !(view->priv->qs.flags & MOO_TEXT_SEARCH_CASELESS));
    gtk_toggle_button_set_active (view->priv->qs.regex,
                                  view->priv->qs.flags & MOO_TEXT_SEARCH_REGEX);

    g_signal_handlers_unblock_by_func (view->priv->qs.case_sensitive,
                                       quick_search_option_toggled, view);
    g_signal_handlers_unblock_by_func (view->priv->qs.regex,
                                       quick_search_option_toggled, view);
}


void
moo_text_view_set_quick_search_flags (MooTextView        *view,
                                      MooTextSearchFlags  flags)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (flags != view->priv->qs.flags)
    {
        view->priv->qs.flags = flags;

        if (view->priv->qs.evbox)
            quick_search_set_widgets_from_flags (view);

        g_object_notify (G_OBJECT (view), "quick-search-flags");
    }
}


static void
scroll_selection_onscreen (GtkTextView *text_view)
{
    GtkTextIter iter;
    GtkTextBuffer *buffer;
    GdkRectangle rect, visible_rect;

    buffer = gtk_text_view_get_buffer (text_view);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_selection_bound (buffer));
    gtk_text_view_scroll_to_iter (text_view, &iter, 0.0, FALSE, 0.0, 0.0);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));

    gtk_text_view_get_iter_location (text_view, &iter, &rect);
    gtk_text_view_get_visible_rect (text_view, &visible_rect);

    if (rect.x < visible_rect.x || rect.y < visible_rect.y ||
        rect.x + rect.width > visible_rect.x + visible_rect.width ||
        rect.y + rect.height > visible_rect.y + visible_rect.height)
            gtk_text_view_scroll_to_iter (text_view, &iter, 0.0, FALSE, 0.0, 0.0);
}


static void
quick_search_message (MooTextView *view,
                      const char  *msg)
{
    GtkWidget *window;

    window = gtk_widget_get_toplevel (GTK_WIDGET (view));

    if (MOO_IS_EDIT_WINDOW (window))
        moo_edit_window_message (MOO_EDIT_WINDOW (window), msg);
}


static void
quick_search_find_from (MooTextView *view,
                        const char  *text,
                        GtkTextIter *start)
{
    gboolean found;
    GtkTextIter match_start, match_end;
    GtkTextBuffer *buffer;

    if (view->priv->qs.flags & MOO_TEXT_SEARCH_REGEX)
    {
        GError *error = NULL;
        EggRegex *re = egg_regex_new (text, 0, 0, &error);

        if (!re)
        {
            char *msg = g_strdup_printf ("Invalid pattern '%s'", text);
            quick_search_message (view, msg);
            g_free (msg);
            return;
        }

        egg_regex_unref (re);
    }

    buffer = get_buffer (view);
    found = moo_text_search_forward (start, text, view->priv->qs.flags,
                                     &match_start, &match_end, NULL);

    if (!found)
    {
        GtkTextIter iter;

        gtk_text_buffer_get_start_iter (buffer, &iter);

        if (!gtk_text_iter_equal (start, &iter))
            found = moo_text_search_forward (&iter, text, view->priv->qs.flags,
                                             &match_start, &match_end, NULL);
    }

    if (found)
    {
        gtk_text_buffer_select_range (buffer, &match_end, &match_start);
        scroll_selection_onscreen (GTK_TEXT_VIEW (view));
    }
    else
    {
        char *message;

        if (view->priv->qs.flags & MOO_TEXT_SEARCH_REGEX)
            message = g_strdup_printf ("Pattern '%s' not found", text);
        else
            message = g_strdup_printf ("Text '%s' not found", text);

        quick_search_message (view, message);
        g_free (message);
    }
}


static void
quick_search_find (MooTextView *view,
                   const char  *text)
{
    GtkTextIter iter1, iter2, insert;
    GtkTextBuffer *buffer = get_buffer (view);

    if (gtk_text_buffer_get_selection_bounds (buffer, &iter1, &iter2))
    {
        gtk_text_buffer_get_iter_at_mark (buffer, &insert, gtk_text_buffer_get_insert (buffer));

        if (gtk_text_iter_equal (&iter1, &insert))
        {
            gtk_text_buffer_place_cursor (buffer, &insert);
            return quick_search_find_from (view, text, &insert);
        }
    }

    quick_search_find_from (view, text, &iter1);
}


static void
quick_search_find_next (MooTextView *view,
                        GtkEntry    *entry,
                        gboolean     from_start)
{
    const char *text;
    GtkTextIter start;
    GtkTextBuffer *buffer = get_buffer (view);

    text = gtk_entry_get_text (entry);

    if (text[0])
    {
        if (from_start)
            gtk_text_buffer_get_start_iter (buffer, &start);
        else
            gtk_text_buffer_get_iter_at_mark (buffer, &start, gtk_text_buffer_get_insert (buffer));
        quick_search_find_from (view, text, &start);
    }
}


static void
search_entry_changed (MooTextView *view,
                      GtkEntry    *entry)
{
    const char *text;

    if (!view->priv->qs.in_search)
        return;

    text = gtk_entry_get_text (entry);

    if (text[0])
        quick_search_find (view, text);
}


static gboolean
search_entry_focus_out (MooTextView *view)
{
    moo_text_view_stop_quick_search (view);
    return FALSE;
}


static gboolean
search_entry_key_press (MooTextView *view,
                        GdkEventKey *event,
                        GtkEntry    *entry)
{
    if (!view->priv->qs.in_search)
        return FALSE;

    switch (event->keyval)
    {
        case GDK_Escape:
            moo_text_view_stop_quick_search (view);
            return TRUE;

        case GDK_Return:
        case GDK_KP_Enter:
            quick_search_find_next (view, entry,
                                    event->state & GDK_CONTROL_MASK);
            return TRUE;
    }

    return FALSE;
}


void
moo_text_view_start_quick_search (MooTextView *view)
{
    char *text = NULL;
    GtkTextIter iter1, iter2;
    GtkTextBuffer *buffer;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->qs.in_search)
        return;

    if (!view->priv->qs.entry)
    {
        MooGladeXML *xml;

        xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
        moo_glade_xml_map_class (xml, "GtkEntry", MOO_TYPE_ENTRY);
        moo_glade_xml_parse_memory (xml, QUICK_SEARCH_GLADE_XML, -1, "evbox", NULL);

        view->priv->qs.evbox = moo_glade_xml_get_widget (xml, "evbox");
        g_return_if_fail (view->priv->qs.evbox != NULL);

        view->priv->qs.entry = moo_glade_xml_get_widget (xml, "entry");
        view->priv->qs.case_sensitive = moo_glade_xml_get_widget (xml, "case_sensitive");
        view->priv->qs.regex = moo_glade_xml_get_widget (xml, "regex");

        g_signal_connect_swapped (view->priv->qs.entry, "changed",
                                  G_CALLBACK (search_entry_changed), view);
        g_signal_connect_swapped (view->priv->qs.entry, "focus-out-event",
                                  G_CALLBACK (search_entry_focus_out), view);
        g_signal_connect_swapped (view->priv->qs.entry, "key-press-event",
                                  G_CALLBACK (search_entry_key_press), view);

        g_signal_connect_swapped (view->priv->qs.case_sensitive, "toggled",
                                  G_CALLBACK (quick_search_option_toggled), view);
        g_signal_connect_swapped (view->priv->qs.regex, "toggled",
                                  G_CALLBACK (quick_search_option_toggled), view);
        quick_search_set_widgets_from_flags (view);

        moo_text_view_add_child_in_border (view, view->priv->qs.evbox,
                                           GTK_TEXT_WINDOW_BOTTOM);

        g_object_unref (xml);
    }

    buffer = get_buffer (view);

    if (gtk_text_buffer_get_selection_bounds (buffer, &iter1, &iter2) &&
        gtk_text_iter_get_line (&iter1) == gtk_text_iter_get_line (&iter2))
    {
        text = gtk_text_buffer_get_slice (buffer, &iter1, &iter2, TRUE);
    }

    if (text)
        gtk_entry_set_text (GTK_ENTRY (view->priv->qs.entry), text);

    gtk_widget_show (view->priv->qs.evbox);
    gtk_widget_grab_focus (view->priv->qs.entry);

    view->priv->qs.in_search = TRUE;

    g_free (text);
}


void
moo_text_view_stop_quick_search (MooTextView *view)
{
    g_return_if_fail (MOO_IS_TEXT_VIEW (view));

    if (view->priv->qs.in_search)
    {
        view->priv->qs.in_search = FALSE;
        gtk_widget_hide (view->priv->qs.evbox);
        gtk_widget_grab_focus (GTK_WIDGET (view));
    }
}


static gboolean
start_quick_search (MooTextView *view)
{
    if (view->priv->qs.enable)
    {
        moo_text_view_start_quick_search (view);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*****************************************************************************/
/* Placeholders
 */

static void
update_box_tag (MooTextView *view)
{
    GtkTextTag *tag = moo_text_view_lookup_tag (view, "moo-text-box");

    if (tag && GTK_WIDGET_REALIZED (view))
    {
        PangoContext *ctx;
        PangoLayout *layout;
        PangoLayoutLine *line;
        PangoRectangle rect;
        int rise;

        ctx = gtk_widget_get_pango_context (GTK_WIDGET (view));
        g_return_if_fail (ctx != NULL);

        layout = pango_layout_new (ctx);
        pango_layout_set_text (layout, "AA", -1);
        line = pango_layout_get_line (layout, 0);

        pango_layout_line_get_extents (line, NULL, &rect);

        rise = rect.y + rect.height;

        if (tag)
            g_object_set (tag, "rise", -rise, NULL);

        g_object_unref (layout);
    }
}


static GtkTextTag *
create_box_tag (MooTextView *view)
{
    GtkTextTag *tag;

    tag = moo_text_view_lookup_tag (view, "moo-text-box");

    if (!tag)
    {
        GtkTextBuffer *buffer = get_buffer (view);
        tag = gtk_text_buffer_create_tag (buffer, "moo-text-box", NULL);
        update_box_tag (view);
    }

    return tag;
}


static GtkTextTag *
get_placeholder_tag (MooTextView *view)
{
    return moo_text_view_lookup_tag (view, MOO_PLACEHOLDER_TAG);
}


static GtkTextTag *
create_placeholder_tag (MooTextView *view)
{
    GtkTextTag *tag;

    tag = moo_text_view_lookup_tag (view, MOO_PLACEHOLDER_TAG);

    if (!tag)
    {
        GtkTextBuffer *buffer = get_buffer (view);
        tag = gtk_text_buffer_create_tag (buffer, MOO_PLACEHOLDER_TAG, NULL);
        g_object_set (tag, "background", "yellow", NULL);
    }

    return tag;
}


static void
moo_text_view_insert_box (MooTextView *view,
                          GtkTextIter *iter)
{
    GtkTextBuffer *buffer;
    GtkTextChildAnchor *anchor;
    GtkWidget *box;
    GtkTextTag *tag;
    GtkTextIter start;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (iter != NULL);

    anchor = g_object_new (MOO_TYPE_TEXT_ANCHOR, NULL);
    box = g_object_new (MOO_TYPE_TEXT_BOX, NULL);
    MOO_TEXT_ANCHOR (anchor)->widget = box;

    buffer = get_buffer (view);
    gtk_text_buffer_insert_child_anchor (buffer, iter, anchor);

    tag = create_box_tag (view);
    start = *iter;
    gtk_text_iter_backward_char (&start);
    gtk_text_buffer_apply_tag (buffer, tag, &start, iter);

    gtk_widget_show (box);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (view), box, anchor);
    view->priv->boxes = g_slist_prepend (view->priv->boxes, box);

    g_object_unref (anchor);
}


void
moo_text_view_insert_placeholder (MooTextView  *view,
                                  GtkTextIter  *iter,
                                  const char   *text)
{
    MooTextBuffer *buffer;
    GtkTextTag *tag;

    g_return_if_fail (MOO_IS_TEXT_VIEW (view));
    g_return_if_fail (iter != NULL);

    if (!text || !text[0])
        return moo_text_view_insert_box (view, iter);

    tag = create_placeholder_tag (view);
    buffer = get_moo_buffer (view);
    gtk_text_buffer_insert_with_tags (GTK_TEXT_BUFFER (buffer),
                                      iter, text, -1, tag, NULL);
}


static gboolean
has_boxes (MooTextView *view)
{
    return view->priv->boxes != NULL;
}


gboolean
moo_text_view_has_box_at_iter (MooTextView *view,
                               GtkTextIter *iter)
{
    GtkTextChildAnchor *anchor;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    g_return_val_if_fail (iter != NULL, FALSE);

    if (gtk_text_iter_get_char (iter) != MOO_TEXT_UNKNOWN_CHAR)
        return FALSE;

    anchor = gtk_text_iter_get_child_anchor (iter);
    return MOO_IS_TEXT_ANCHOR (anchor) &&
            MOO_IS_TEXT_BOX (MOO_TEXT_ANCHOR(anchor)->widget);
}


static gboolean
moo_text_view_find_box_forward (MooTextView *view,
                                GtkTextIter *match_start,
                                GtkTextIter *match_end)
{
    GtkTextIter start;
    GtkTextBuffer *buffer;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    buffer = get_buffer (view);
    gtk_text_buffer_get_selection_bounds (buffer, NULL, &start);

    while (gtk_text_iter_forward_search (&start, MOO_TEXT_UNKNOWN_CHAR_S,
                                         0, match_start, match_end, NULL))
    {
        if (moo_text_view_has_box_at_iter (view, match_start))
            return TRUE;
        else
            start = *match_end;
    }

    return FALSE;
}


static gboolean
moo_text_view_find_placeholder_forward (MooTextView *view,
                                        GtkTextIter *match_start,
                                        GtkTextIter *match_end)
{
    GtkTextBuffer *buffer;
    GtkTextTag *tag;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (!(tag = get_placeholder_tag (view)))
        return FALSE;

    buffer = get_buffer (view);
    gtk_text_buffer_get_selection_bounds (buffer, NULL, match_start);

    if (gtk_text_iter_has_tag (match_start, tag))
    {
        if (gtk_text_iter_begins_tag (match_start, tag))
        {
            *match_end = *match_start;
            gtk_text_iter_forward_to_tag_toggle (match_end, tag);
            return TRUE;
        }

        if (!gtk_text_iter_forward_to_tag_toggle (match_start, tag))
            return FALSE;
    }

    if (!gtk_text_iter_forward_to_tag_toggle (match_start, tag))
        return FALSE;

    g_assert (gtk_text_iter_begins_tag (match_start, tag));

    *match_end = *match_start;
    gtk_text_iter_forward_to_tag_toggle (match_end, tag);

    return TRUE;
}


static gboolean
moo_text_view_find_placeholder_backward (MooTextView *view,
                                         GtkTextIter *match_start,
                                         GtkTextIter *match_end)
{
    GtkTextBuffer *buffer;
    GtkTextTag *tag;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    if (!(tag = get_placeholder_tag (view)))
        return FALSE;

    buffer = get_buffer (view);
    gtk_text_buffer_get_selection_bounds (buffer, match_start, NULL);

    if (gtk_text_iter_has_tag (match_start, tag))
    {
        if (!gtk_text_iter_begins_tag (match_start, tag))
            gtk_text_iter_backward_to_tag_toggle (match_start, tag);
    }
    else if (gtk_text_iter_ends_tag (match_start, tag))
    {
        *match_end = *match_start;
        gtk_text_iter_backward_to_tag_toggle (match_end, tag);
        return TRUE;
    }

    if (!gtk_text_iter_backward_to_tag_toggle (match_start, tag))
        return FALSE;

    g_assert (gtk_text_iter_ends_tag (match_start, tag));

    *match_end = *match_start;
    gtk_text_iter_backward_to_tag_toggle (match_end, tag);

    return TRUE;
}


static gboolean
moo_text_view_find_box_backward (MooTextView *view,
                                 GtkTextIter *match_start,
                                 GtkTextIter *match_end)
{
    GtkTextIter start;
    GtkTextBuffer *buffer;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    buffer = get_buffer (view);
    gtk_text_buffer_get_selection_bounds (buffer, &start, NULL);

    while (gtk_text_iter_backward_search (&start, MOO_TEXT_UNKNOWN_CHAR_S,
                                          0, match_start, match_end, NULL))
    {
        if (moo_text_view_has_box_at_iter (view, match_start))
        {
            start = *match_start;
            *match_start = *match_end;
            *match_end = start;
            return TRUE;
        }
        else
        {
            start = *match_start;
        }
    }

    return FALSE;
}


static gboolean
moo_text_view_find_placeholder (MooTextView *view,
                                gboolean     forward)
{
    GtkTextIter box_start, box_end, ph_start, ph_end;
    GtkTextIter *start, *end;
    GtkTextBuffer *buffer;
    gboolean found_box, found_ph;

    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);

    buffer = get_buffer (view);

    if (forward)
    {
        found_box = moo_text_view_find_box_forward (view, &box_start, &box_end);
        found_ph = moo_text_view_find_placeholder_forward (view, &ph_start, &ph_end);
    }
    else
    {
        found_box = moo_text_view_find_box_backward (view, &box_start, &box_end);
        found_ph = moo_text_view_find_placeholder_backward (view, &ph_start, &ph_end);
    }

    if (!found_box && !found_ph)
    {
        moo_text_view_message (view, "No placeholder found");
        return FALSE;
    }

    if (found_box && found_ph)
    {
        if (forward)
            found_box = gtk_text_iter_compare (&box_start, &ph_start) < 0;
        else
            found_box = gtk_text_iter_compare (&box_start, &ph_start) > 0;
    }

    if (found_box)
    {
        start = &box_start;
        end = &box_end;
    }
    else
    {
        start = &ph_start;
        end = &ph_end;
    }

    if (forward)
        gtk_text_buffer_select_range (buffer, start, end);
    else
        gtk_text_buffer_select_range (buffer, end, start);

    scroll_selection_onscreen (GTK_TEXT_VIEW (view));
    return TRUE;
}


gboolean
moo_text_view_prev_placeholder (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_view_find_placeholder (view, FALSE);
}


gboolean
moo_text_view_next_placeholder (MooTextView *view)
{
    g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
    return moo_text_view_find_placeholder (view, TRUE);
}
