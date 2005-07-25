/*
 *   mooutils/moocompat.c
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
 *  Some functions here are taken from GTK+ and GLIB (http://gtk.org/)
 *
 *  g_signal_accumulator_true_handled function is taken from gobject/gsignal.c;
 *  it's Copyright (C) 2000-2001 Red Hat, Inc.
 *
 *  substitute_underscores, and gtk_accelerator_get_label are taken from gtk/gtkaccelgroup.c.
 *  it's Copyright (C) 1998, 2001 Tim Janik
 *
 *  gtk_target_list_add_text_targets is from gtk/gtkselection.c
 *  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 */


#include "mooutils/moocompat.h"
#include <string.h>


#if !GLIB_CHECK_VERSION(2,4,0)

/* from gsignal.c */
gboolean
g_signal_accumulator_true_handled (GSignalInvocationHint *ihint,
				   GValue                *return_accu,
				   const GValue          *handler_return,
				   gpointer               dummy)
{
  gboolean continue_emission;
  gboolean signal_handled;

  signal_handled = g_value_get_boolean (handler_return);
  g_value_set_boolean (return_accu, signal_handled);
  continue_emission = !signal_handled;

  return continue_emission;
}


void g_ptr_array_foreach (GPtrArray *array,
                          GFunc      func,
                          gpointer   data)
{
    guint i;
    for (i = 0; i < array->len; ++i)
        (*func) (array->pdata[i], data);
}

void g_ptr_array_remove_range (GPtrArray *array,
                               guint      index,
                               guint      length)
{
    g_return_if_fail (array);
    g_return_if_fail (index < array->len);
    g_return_if_fail (index + length <= array->len);

    if (index + length != array->len)
        g_memmove (&array->pdata[index],
                    &array->pdata[index + length],
                    (array->len - (index + length)) * sizeof (gpointer));

    g_ptr_array_set_size (array, array->len - length);
}

#endif /* !GLIB_CHECK_VERSION(2,4,0) */


#if !GTK_CHECK_VERSION(2,6,0)

GtkWidget *gtk_label_new_with_markup (const char *markup)
{
    GtkWidget *label = gtk_label_new ("");
    gtk_label_set_markup (GTK_LABEL (label), markup);
    return label;
}


/* substitute_underscores from gtk/gtkaccellabel.c */
static void substitute_underscores (char *str)
{
  char *p;

  for (p = str; *p; p++)
    if (*p == '_')
      *p = ' ';
}

/* _gtk_accel_label_class_get_accelerator_label from gtk/gtkaccellabel.c */
char *gtk_accelerator_get_label (guint accelerator_key,
                                 GdkModifierType accelerator_mods)
{
  GtkAccelLabelClass *klass;
  GString *gstring;
  gboolean seen_mod = FALSE;
  gunichar ch;

  klass = g_type_class_ref (GTK_TYPE_ACCEL_LABEL);

  gstring = g_string_new ("");

  if (accelerator_mods & GDK_SHIFT_MASK)
    {
      g_string_append (gstring, klass->mod_name_shift);
      seen_mod = TRUE;
    }
  if (accelerator_mods & GDK_CONTROL_MASK)
    {
      if (seen_mod)
	g_string_append (gstring, klass->mod_separator);
      g_string_append (gstring, klass->mod_name_control);
      seen_mod = TRUE;
    }
  if (accelerator_mods & GDK_MOD1_MASK)
    {
      if (seen_mod)
	g_string_append (gstring, klass->mod_separator);
      g_string_append (gstring, klass->mod_name_alt);
      seen_mod = TRUE;
    }
  if (seen_mod)
    g_string_append (gstring, klass->mod_separator);

  ch = gdk_keyval_to_unicode (accelerator_key);
  if (ch && (g_unichar_isgraph (ch) || ch == ' ') &&
      (ch < 0x80 || klass->latin1_to_char))
    {
      switch (ch)
	{
	case ' ':
	  g_string_append (gstring, "Space");
	  break;
	case '\\':
	  g_string_append (gstring, "Backslash");
	  break;
	default:
	  g_string_append_unichar (gstring, g_unichar_toupper (ch));
	  break;
	}
    }
  else
    {
      gchar *tmp;

      tmp = gtk_accelerator_name (accelerator_key, 0);
      if (tmp[0] != 0 && tmp[1] == 0)
	tmp[0] = g_ascii_toupper (tmp[0]);
      substitute_underscores (tmp);
      g_string_append (gstring, tmp);
      g_free (tmp);
    }

  g_type_class_unref (klass); /* klass is kept alive since gtk uses static types */
  return g_string_free (gstring, FALSE);
}


/****************************************************************************/
/** gtk_target_list_add_text_targets                                        */
/** from gtk/gtkselection.c                                                 */

static GdkAtom utf8_atom;
static GdkAtom text_atom;
static GdkAtom ctext_atom;
static GdkAtom text_plain_atom;
static GdkAtom text_plain_utf8_atom;
static GdkAtom text_plain_locale_atom;
static GdkAtom text_uri_list_atom;

static void init_atoms (void)
{
  gchar *tmp;
  const gchar *charset;

  if (!utf8_atom)
    {
      utf8_atom = gdk_atom_intern ("UTF8_STRING", FALSE);
      text_atom = gdk_atom_intern ("TEXT", FALSE);
      ctext_atom = gdk_atom_intern ("COMPOUND_TEXT", FALSE);
      text_plain_atom = gdk_atom_intern ("text/plain", FALSE);
      text_plain_utf8_atom = gdk_atom_intern ("text/plain;charset=utf-8", FALSE);
      g_get_charset (&charset);
      tmp = g_strdup_printf ("text/plain;charset=%s", charset);
      text_plain_locale_atom = gdk_atom_intern (tmp, FALSE);
      g_free (tmp);

      text_uri_list_atom = gdk_atom_intern ("text/uri-list", FALSE);
    }
}

void  gtk_target_list_add_text_targets (GtkTargetList *list,
                                        guint          info)
{
  g_return_if_fail (list != NULL);

  init_atoms ();

  /* Keep in sync with gtk_selection_data_targets_include_text()
   */
  gtk_target_list_add (list, utf8_atom, 0, info);
  gtk_target_list_add (list, ctext_atom, 0, info);
  gtk_target_list_add (list, text_atom, 0, info);
  gtk_target_list_add (list, GDK_TARGET_STRING, 0, info);
  gtk_target_list_add (list, text_plain_utf8_atom, 0, info);
  gtk_target_list_add (list, text_plain_locale_atom, 0, info);
  gtk_target_list_add (list, text_plain_atom, 0, info);
}

#endif /* !GTK_CHECK_VERSION(2,6,0) */


#if !GTK_CHECK_VERSION(2,4,0)

void gtk_text_buffer_select_range (GtkTextBuffer *buffer,
                                   const GtkTextIter *ins,
                                   const GtkTextIter *bound)
{
    gtk_text_buffer_place_cursor (buffer, bound);
    gtk_text_buffer_move_mark (buffer,
                               gtk_text_buffer_get_selection_bound (buffer), ins);
}

/* TODO: need to put real function here */
/* TODO: why are they called at all? */
#ifdef gtk_button_set_focus_on_click
#undef gtk_button_set_focus_on_click
#endif
void gtk_button_set_focus_on_click (GtkWidget *wid, gboolean setting)
{
    g_warning ("%s: this function should not be called", G_STRLOC);
}

#ifdef gtk_alignment_set_padding
#undef gtk_alignment_set_padding
#endif
void gtk_alignment_set_padding (GtkAlignment *alignment,
                                guint padding_top,
                                guint padding_bottom,
                                guint padding_left,
                                guint padding_right)
{
    g_warning ("%s: this function should not be called", G_STRLOC);
}

#endif /* !GTK_CHECK_VERSION(2,4,0) */
