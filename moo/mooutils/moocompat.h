/*
 *   mooutils/moocompat.h
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

#ifndef MOOUTILS_COMPAT_H
#define MOOUTILS_COMPAT_H

#include <gtk/gtk.h>

#ifndef __WIN32__ /* TODO */
#include <sys/types.h>
#endif /* __WIN32__ */

#if !GLIB_CHECK_VERSION(2,8,0)
#include "mooutils/newgtk/gmappedfile.h"

int g_mkdir_with_parents    (const gchar *pathname,
                             int          mode);
#endif /* !GLIB_CHECK_VERSION(2,8,0) */

#if !GTK_CHECK_VERSION(2,4,0)
#include "mooutils/newgtk/gtkfontbutton.h"
#include "mooutils/newgtk/gtkcolorbutton.h"
#endif /* !GTK_CHECK_VERSION(2,4,0) */


G_BEGIN_DECLS


#if !GLIB_CHECK_VERSION(2,4,0)

/* from gsignal.h */
gboolean g_signal_accumulator_true_handled (GSignalInvocationHint *ihint,
                                            GValue                *return_accu,
                                            const GValue          *handler_return,
                                            gpointer               dummy);


/* macros from gtype.h*/

#ifndef G_UNLIKELY
#define G_UNLIKELY(whatever) (whatever)
#endif

#ifndef G_DEFINE_TYPE
#define G_DEFINE_TYPE(TypeName, type_name, TYPE_PARENT) \
static void     type_name##_init              (TypeName        *self); \
static void     type_name##_class_init        (TypeName##Class *klass); \
static gpointer type_name##_parent_class = NULL; \
static void     type_name##_class_intern_init (gpointer klass) \
{ \
  type_name##_parent_class = g_type_class_peek_parent (klass); \
  type_name##_class_init ((TypeName##Class*) klass); \
} \
\
GType \
type_name##_get_type (void) \
{ \
  static GType g_define_type_id = 0; \
  if (G_UNLIKELY (g_define_type_id == 0)) \
    { \
      static const GTypeInfo g_define_type_info = { \
        sizeof (TypeName##Class), \
        (GBaseInitFunc) NULL, \
        (GBaseFinalizeFunc) NULL, \
        (GClassInitFunc) type_name##_class_intern_init, \
        (GClassFinalizeFunc) NULL, \
        NULL,   /* class_data */ \
        sizeof (TypeName), \
        0,      /* n_preallocs */ \
        (GInstanceInitFunc) type_name##_init, \
        NULL    /* value_table */ \
      }; \
      g_define_type_id = g_type_register_static (TYPE_PARENT, #TypeName, &g_define_type_info, (GTypeFlags) 0); \
    } \
  return g_define_type_id; \
}
#endif /* !G_DEFINE_TYPE */

void       g_ptr_array_foreach            (GPtrArray        *array,
                                           GFunc             func,
                                           gpointer          user_data);
void       g_ptr_array_remove_range       (GPtrArray        *array,
                                           guint             index_,
                                           guint             length);

#endif /* !GLIB_CHECK_VERSION(2,4,0) */


#if !GTK_CHECK_VERSION(2,4,0)

void gtk_text_buffer_select_range (GtkTextBuffer *buffer,
                                   const GtkTextIter *ins,
                                   const GtkTextIter *bound);

#define gtk_alignment_set_padding(wid,t,b,l,r)
#define gtk_button_set_focus_on_click(wid,s)
#define gtk_combo_box_new() gtk_label_new("")


#ifdef __WIN32__
typedef void * GPid;
#else /* !__WIN32 */
typedef pid_t GPid;
#endif /* !__WIN32 */

#endif /* !GTK_CHECK_VERSION(2,4,0) */


#if !GTK_CHECK_VERSION(2,6,0)

void        gtk_target_list_add_text_targets    (GtkTargetList  *list,
                                                 guint           info);

char       *gtk_accelerator_get_label           (guint           accelerator_key,
                                                 GdkModifierType accelerator_mods);

GtkWidget  *gtk_label_new_with_markup           (const char     *markup);
void        gtk_label_set_angle                 (GtkLabel       *label,
                                                 gdouble         angle);

void gtk_dialog_set_alternative_button_order (GtkDialog *dialog,
                                              gint       first_response_id,
                                              ...);

void           gtk_drag_dest_add_text_targets  (GtkWidget    *widget);
void           gtk_drag_dest_add_uri_targets   (GtkWidget    *widget);

gboolean gtk_selection_data_set_uris (GtkSelectionData     *selection_data,
                                      gchar               **uris);
gchar  **gtk_selection_data_get_uris (GtkSelectionData     *selection_data);

#define GTK_STOCK_EDIT      "gtk-edit"
#define GTK_STOCK_ABOUT     "gtk-about"
#define GTK_STOCK_DIRECTORY "gtk-directory"

#endif /* !GTK_CHECK_VERSION(2,6,0) */


#if !GLIB_CHECK_VERSION(2,6,0)
gchar *g_filename_display_basename (const gchar *filename);
gchar *g_filename_display_name (const gchar *filename);
guint  g_strv_length (gchar **str_array);
#endif /* !GLIB_CHECK_VERSION(2,6,0) */


#if !GTK_CHECK_VERSION(2,10,0)

#ifndef GTK_STOCK_SELECT_ALL
#define GTK_STOCK_SELECT_ALL  "gtk-select-all"
#endif

#endif /* !GTK_CHECK_VERSION(2,10,0) */


G_END_DECLS

#endif /* MOOUTILS_COMPAT_H */
