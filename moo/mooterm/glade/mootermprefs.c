/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "mootermprefs.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_term_prefs_window (void)
{
  GtkWidget *term_prefs_window;
  GtkWidget *notebook;
  GtkWidget *frame15;
  GtkWidget *alignment25;
  GtkWidget *vbox9;
  GtkWidget *hbox7;
  GtkWidget *cursor_blinks;
  GtkWidget *label1;
  GtkObject *cursor_blink_time_adj;
  GtkWidget *cursor_blink_time;
  GtkWidget *label2;
  GtkWidget *auto_hide_mouse;
  GtkWidget *fancy_cmd_line;
  GtkWidget *alignment27;
  GtkWidget *hseparator4;
  GtkWidget *frame16;
  GtkWidget *alignment28;
  GtkWidget *vbox10;
  GtkWidget *hbox8;
  GtkWidget *cursor_underline;
  GSList *cursor_underline_group = NULL;
  GtkWidget *cursor_height_label;
  GtkObject *cursor_height_adj;
  GtkWidget *cursor_height;
  GtkWidget *cursor_block;
  GtkWidget *label63;
  GtkWidget *label62;
  GtkWidget *frame14;
  GtkWidget *alignment21;
  GtkWidget *table11;
  GtkWidget *label13;
  GtkWidget *label16;
  GtkWidget *selected_foreground_label;
  GtkWidget *selected_background_label;
  GtkWidget *cursor_foreground_label;
  GtkWidget *cursor_background_label;
  GtkWidget *use_text_colors;
  GtkWidget *foreground;
  GtkWidget *background;
  GtkWidget *selected_foreground;
  GtkWidget *selected_background;
  GtkWidget *cursor_foreground;
  GtkWidget *cursor_background;
  GtkWidget *alignment29;
  GtkWidget *hseparator5;
  GtkWidget *hbox9;
  GtkWidget *label64;
  GtkWidget *font;
  GtkWidget *label58;
  GtkTooltips *tooltips;

  tooltips = gtk_tooltips_new ();

  term_prefs_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (term_prefs_window), _("Terminal Preferences"));

  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_container_add (GTK_CONTAINER (term_prefs_window), notebook);

  frame15 = gtk_frame_new (NULL);
  gtk_widget_show (frame15);
  gtk_container_add (GTK_CONTAINER (notebook), frame15);

  alignment25 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment25);
  gtk_container_add (GTK_CONTAINER (frame15), alignment25);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment25), 0, 3, 3, 3);

  vbox9 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox9);
  gtk_container_add (GTK_CONTAINER (alignment25), vbox9);

  hbox7 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (vbox9), hbox7, FALSE, FALSE, 0);

  cursor_blinks = gtk_check_button_new_with_mnemonic (_("_Cursor blinks   "));
  gtk_widget_show (cursor_blinks);
  gtk_box_pack_start (GTK_BOX (hbox7), cursor_blinks, FALSE, FALSE, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (cursor_blinks), FALSE);

  label1 = gtk_label_new_with_mnemonic (_("_Freq.: "));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox7), label1, FALSE, FALSE, 0);

  cursor_blink_time_adj = gtk_adjustment_new (1, 0, 100000, 1, 10, 10);
  cursor_blink_time = gtk_spin_button_new (GTK_ADJUSTMENT (cursor_blink_time_adj), 1, 0);
  gtk_widget_show (cursor_blink_time);
  gtk_box_pack_start (GTK_BOX (hbox7), cursor_blink_time, FALSE, FALSE, 0);

  label2 = gtk_label_new (_(" ms"));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox7), label2, FALSE, FALSE, 0);

  auto_hide_mouse = gtk_check_button_new_with_mnemonic (_("Auto_hide mouse cursor"));
  gtk_widget_show (auto_hide_mouse);
  gtk_box_pack_start (GTK_BOX (vbox9), auto_hide_mouse, FALSE, FALSE, 0);
  gtk_widget_set_sensitive (auto_hide_mouse, FALSE);
  gtk_button_set_focus_on_click (GTK_BUTTON (auto_hide_mouse), FALSE);

  fancy_cmd_line = gtk_check_button_new_with_mnemonic (_("Fancy command _line"));
  gtk_widget_show (fancy_cmd_line);
  gtk_box_pack_start (GTK_BOX (vbox9), fancy_cmd_line, FALSE, FALSE, 0);
  gtk_tooltips_set_tip (tooltips, fancy_cmd_line, _("Pretend command line was a text editor"), NULL);
  gtk_button_set_focus_on_click (GTK_BUTTON (fancy_cmd_line), FALSE);

  alignment27 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment27);
  gtk_box_pack_start (GTK_BOX (vbox9), alignment27, FALSE, FALSE, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment27), 3, 3, 0, 0);

  hseparator4 = gtk_hseparator_new ();
  gtk_widget_show (hseparator4);
  gtk_container_add (GTK_CONTAINER (alignment27), hseparator4);

  frame16 = gtk_frame_new (NULL);
  gtk_widget_show (frame16);
  gtk_box_pack_start (GTK_BOX (vbox9), frame16, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame16), GTK_SHADOW_NONE);

  alignment28 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment28);
  gtk_container_add (GTK_CONTAINER (frame16), alignment28);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment28), 0, 0, 6, 0);

  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox10);
  gtk_container_add (GTK_CONTAINER (alignment28), vbox10);

  hbox8 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox8);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox8, TRUE, TRUE, 0);

  cursor_underline = gtk_radio_button_new_with_mnemonic (NULL, _("_Underline    "));
  gtk_widget_show (cursor_underline);
  gtk_box_pack_start (GTK_BOX (hbox8), cursor_underline, FALSE, FALSE, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (cursor_underline), FALSE);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (cursor_underline), cursor_underline_group);
  cursor_underline_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (cursor_underline));

  cursor_height_label = gtk_label_new_with_mnemonic (_("_Height: "));
  gtk_widget_show (cursor_height_label);
  gtk_box_pack_start (GTK_BOX (hbox8), cursor_height_label, FALSE, FALSE, 0);

  cursor_height_adj = gtk_adjustment_new (1, 1, 100, 1, 10, 10);
  cursor_height = gtk_spin_button_new (GTK_ADJUSTMENT (cursor_height_adj), 1, 0);
  gtk_widget_show (cursor_height);
  gtk_box_pack_start (GTK_BOX (hbox8), cursor_height, FALSE, FALSE, 0);
  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (cursor_height), TRUE);

  cursor_block = gtk_radio_button_new_with_mnemonic (NULL, _("_Block"));
  gtk_widget_show (cursor_block);
  gtk_box_pack_start (GTK_BOX (vbox10), cursor_block, FALSE, FALSE, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (cursor_block), FALSE);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (cursor_block), cursor_underline_group);
  cursor_underline_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (cursor_block));

  label63 = gtk_label_new (_("Cursor appearance"));
  gtk_widget_show (label63);
  gtk_frame_set_label_widget (GTK_FRAME (frame16), label63);

  label62 = gtk_label_new_with_mnemonic (_("_Terminal"));
  gtk_widget_show (label62);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0), label62);

  frame14 = gtk_frame_new (NULL);
  gtk_widget_show (frame14);
  gtk_container_add (GTK_CONTAINER (notebook), frame14);
  gtk_container_set_border_width (GTK_CONTAINER (frame14), 3);

  alignment21 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment21);
  gtk_container_add (GTK_CONTAINER (frame14), alignment21);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment21), 3, 3, 3, 3);

  table11 = gtk_table_new (9, 2, FALSE);
  gtk_widget_show (table11);
  gtk_container_add (GTK_CONTAINER (alignment21), table11);

  label13 = gtk_label_new (_("Foreground color: "));
  gtk_widget_show (label13);
  gtk_table_attach (GTK_TABLE (table11), label13, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);

  label16 = gtk_label_new (_("Background color: "));
  gtk_widget_show (label16);
  gtk_table_attach (GTK_TABLE (table11), label16, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label16), 0, 0.5);

  selected_foreground_label = gtk_label_new (_("    Selected text foreground: "));
  gtk_widget_show (selected_foreground_label);
  gtk_table_attach (GTK_TABLE (table11), selected_foreground_label, 0, 1, 5, 6,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (selected_foreground_label), 0, 0.5);

  selected_background_label = gtk_label_new (_("    Selected text background: "));
  gtk_widget_show (selected_background_label);
  gtk_table_attach (GTK_TABLE (table11), selected_background_label, 0, 1, 6, 7,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (selected_background_label), 0, 0.5);

  cursor_foreground_label = gtk_label_new (_("    Cursor foreground: "));
  gtk_widget_show (cursor_foreground_label);
  gtk_table_attach (GTK_TABLE (table11), cursor_foreground_label, 0, 1, 7, 8,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cursor_foreground_label), 0, 0.5);

  cursor_background_label = gtk_label_new (_("    Cursor background: "));
  gtk_widget_show (cursor_background_label);
  gtk_table_attach (GTK_TABLE (table11), cursor_background_label, 0, 1, 8, 9,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (cursor_background_label), 0, 0.5);

  use_text_colors = gtk_check_button_new_with_mnemonic (_("Use text colors for selection and cursor"));
  gtk_widget_show (use_text_colors);
  gtk_table_attach (GTK_TABLE (table11), use_text_colors, 0, 2, 4, 5,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (use_text_colors), FALSE);

  foreground = gtk_color_button_new ();
  gtk_widget_show (foreground);
  gtk_table_attach (GTK_TABLE (table11), foreground, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_color_button_set_title (GTK_COLOR_BUTTON (foreground), _("Pick Terminal Foreground Color"));
  gtk_button_set_focus_on_click (GTK_BUTTON (foreground), FALSE);

  background = gtk_color_button_new ();
  gtk_widget_show (background);
  gtk_table_attach (GTK_TABLE (table11), background, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_color_button_set_title (GTK_COLOR_BUTTON (background), _("Pick Terminal Background Color"));
  gtk_button_set_focus_on_click (GTK_BUTTON (background), FALSE);

  selected_foreground = gtk_color_button_new ();
  gtk_widget_show (selected_foreground);
  gtk_table_attach (GTK_TABLE (table11), selected_foreground, 1, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (selected_foreground), FALSE);

  selected_background = gtk_color_button_new ();
  gtk_widget_show (selected_background);
  gtk_table_attach (GTK_TABLE (table11), selected_background, 1, 2, 6, 7,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (selected_background), FALSE);

  cursor_foreground = gtk_color_button_new ();
  gtk_widget_show (cursor_foreground);
  gtk_table_attach (GTK_TABLE (table11), cursor_foreground, 1, 2, 7, 8,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (cursor_foreground), FALSE);

  cursor_background = gtk_color_button_new ();
  gtk_widget_show (cursor_background);
  gtk_table_attach (GTK_TABLE (table11), cursor_background, 1, 2, 8, 9,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_button_set_focus_on_click (GTK_BUTTON (cursor_background), FALSE);

  alignment29 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment29);
  gtk_table_attach (GTK_TABLE (table11), alignment29, 0, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment29), 3, 3, 0, 0);

  hseparator5 = gtk_hseparator_new ();
  gtk_widget_show (hseparator5);
  gtk_container_add (GTK_CONTAINER (alignment29), hseparator5);

  hbox9 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox9);
  gtk_table_attach (GTK_TABLE (table11), hbox9, 0, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  label64 = gtk_label_new_with_mnemonic (_("F_ont: "));
  gtk_widget_show (label64);
  gtk_box_pack_start (GTK_BOX (hbox9), label64, FALSE, FALSE, 0);

  font = gtk_font_button_new ();
  gtk_widget_show (font);
  gtk_box_pack_start (GTK_BOX (hbox9), font, TRUE, TRUE, 0);
  gtk_font_button_set_use_font (GTK_FONT_BUTTON (font), TRUE);
  gtk_font_button_set_use_size (GTK_FONT_BUTTON (font), TRUE);
  gtk_button_set_focus_on_click (GTK_BUTTON (font), FALSE);

  label58 = gtk_label_new_with_mnemonic (_("_Font and Colors"));
  gtk_widget_show (label58);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 1), label58);

  g_signal_connect ((gpointer) cursor_blinks, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_CURSOR_BLINKS),
                    NULL);
  g_signal_connect ((gpointer) label1, "moo_sensitive",
                    G_CALLBACK (cursor_blinks),
                    NULL);
  g_signal_connect ((gpointer) cursor_blink_time, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_CURSOR_BLINK_TIME),
                    NULL);
  g_signal_connect ((gpointer) cursor_blink_time, "moo_sensitive",
                    G_CALLBACK (cursor_blinks),
                    NULL);
  g_signal_connect ((gpointer) label2, "moo_sensitive",
                    G_CALLBACK (cursor_blinks),
                    NULL);
  g_signal_connect ((gpointer) auto_hide_mouse, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_AUTO_HIDE_MOUSE),
                    NULL);
  g_signal_connect ((gpointer) fancy_cmd_line, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_FANCY_CMD_LINE),
                    NULL);
  g_signal_connect_swapped ((gpointer) cursor_underline, "moo_radio",
                            G_CALLBACK (MOO_TERM_PREFS_CURSOR_SHAPE),
                            GTK_OBJECT (MOO_TERM_PREFS_CURSOR_UNDERLINE));
  g_signal_connect_swapped ((gpointer) cursor_height_label, "moo_sensitive",
                            G_CALLBACK (cursor_underline),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) cursor_height, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_CURSOR_HEIGHT),
                    NULL);
  g_signal_connect ((gpointer) cursor_height, "moo_sensitive",
                    G_CALLBACK (cursor_underline),
                    NULL);
  g_signal_connect_swapped ((gpointer) cursor_block, "moo_radio",
                            G_CALLBACK (MOO_TERM_PREFS_CURSOR_SHAPE),
                            GTK_OBJECT (MOO_TERM_PREFS_CURSOR_BLOCK));
  g_signal_connect_swapped ((gpointer) selected_foreground_label, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect_swapped ((gpointer) selected_background_label, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect_swapped ((gpointer) cursor_foreground_label, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect_swapped ((gpointer) cursor_background_label, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) use_text_colors, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_USE_TEXT_COLORS),
                    NULL);
  g_signal_connect ((gpointer) foreground, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_FOREGROUND),
                    NULL);
  g_signal_connect ((gpointer) background, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_BACKGROUND),
                    NULL);
  g_signal_connect ((gpointer) selected_foreground, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_SELECTION_FOREGROUND),
                    NULL);
  g_signal_connect_swapped ((gpointer) selected_foreground, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) selected_background, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_SELECTION_BACKGROUND),
                    NULL);
  g_signal_connect_swapped ((gpointer) selected_background, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) cursor_foreground, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_CURSOR_FOREGROUND),
                    NULL);
  g_signal_connect_swapped ((gpointer) cursor_foreground, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) cursor_background, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_CURSOR_BACKGROUND),
                    NULL);
  g_signal_connect_swapped ((gpointer) cursor_background, "moo_sensitive",
                            G_CALLBACK (use_text_colors),
                            GTK_OBJECT (invert));
  g_signal_connect ((gpointer) font, "moo_prefs_key",
                    G_CALLBACK (MOO_TERM_PREFS_FONT),
                    NULL);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label1), cursor_blink_time);
  gtk_label_set_mnemonic_widget (GTK_LABEL (cursor_height_label), cursor_height);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label62), cursor_blinks);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label64), font);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (term_prefs_window, term_prefs_window, "term_prefs_window");
  GLADE_HOOKUP_OBJECT (term_prefs_window, notebook, "notebook");
  GLADE_HOOKUP_OBJECT (term_prefs_window, frame15, "frame15");
  GLADE_HOOKUP_OBJECT (term_prefs_window, alignment25, "alignment25");
  GLADE_HOOKUP_OBJECT (term_prefs_window, vbox9, "vbox9");
  GLADE_HOOKUP_OBJECT (term_prefs_window, hbox7, "hbox7");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_blinks, "cursor_blinks");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label1, "label1");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_blink_time, "cursor_blink_time");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label2, "label2");
  GLADE_HOOKUP_OBJECT (term_prefs_window, auto_hide_mouse, "auto_hide_mouse");
  GLADE_HOOKUP_OBJECT (term_prefs_window, fancy_cmd_line, "fancy_cmd_line");
  GLADE_HOOKUP_OBJECT (term_prefs_window, alignment27, "alignment27");
  GLADE_HOOKUP_OBJECT (term_prefs_window, hseparator4, "hseparator4");
  GLADE_HOOKUP_OBJECT (term_prefs_window, frame16, "frame16");
  GLADE_HOOKUP_OBJECT (term_prefs_window, alignment28, "alignment28");
  GLADE_HOOKUP_OBJECT (term_prefs_window, vbox10, "vbox10");
  GLADE_HOOKUP_OBJECT (term_prefs_window, hbox8, "hbox8");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_underline, "cursor_underline");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_height_label, "cursor_height_label");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_height, "cursor_height");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_block, "cursor_block");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label63, "label63");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label62, "label62");
  GLADE_HOOKUP_OBJECT (term_prefs_window, frame14, "frame14");
  GLADE_HOOKUP_OBJECT (term_prefs_window, alignment21, "alignment21");
  GLADE_HOOKUP_OBJECT (term_prefs_window, table11, "table11");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label13, "label13");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label16, "label16");
  GLADE_HOOKUP_OBJECT (term_prefs_window, selected_foreground_label, "selected_foreground_label");
  GLADE_HOOKUP_OBJECT (term_prefs_window, selected_background_label, "selected_background_label");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_foreground_label, "cursor_foreground_label");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_background_label, "cursor_background_label");
  GLADE_HOOKUP_OBJECT (term_prefs_window, use_text_colors, "use_text_colors");
  GLADE_HOOKUP_OBJECT (term_prefs_window, foreground, "foreground");
  GLADE_HOOKUP_OBJECT (term_prefs_window, background, "background");
  GLADE_HOOKUP_OBJECT (term_prefs_window, selected_foreground, "selected_foreground");
  GLADE_HOOKUP_OBJECT (term_prefs_window, selected_background, "selected_background");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_foreground, "cursor_foreground");
  GLADE_HOOKUP_OBJECT (term_prefs_window, cursor_background, "cursor_background");
  GLADE_HOOKUP_OBJECT (term_prefs_window, alignment29, "alignment29");
  GLADE_HOOKUP_OBJECT (term_prefs_window, hseparator5, "hseparator5");
  GLADE_HOOKUP_OBJECT (term_prefs_window, hbox9, "hbox9");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label64, "label64");
  GLADE_HOOKUP_OBJECT (term_prefs_window, font, "font");
  GLADE_HOOKUP_OBJECT (term_prefs_window, label58, "label58");
  GLADE_HOOKUP_OBJECT_NO_REF (term_prefs_window, tooltips, "tooltips");

  return term_prefs_window;
}

