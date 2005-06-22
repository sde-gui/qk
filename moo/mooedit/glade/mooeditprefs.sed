s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <unistd.h>//
s/#include <string.h>/#include "mooutils\/moocompat.h"/
s/#include <stdio.h>/#include "mooedit\/mooedit.h"/
s/#include "callbacks.h"/#include "mooutils\/mooprefsdialog.h"/
s/#include "mooeditprefs.h"/#include "mooedit\/mooeditprefs.h"/
s/create_window (void)/_create_moo_edit_prefs_notebook (MooPrefsDialogPage *page);\nGtkWidget*\n_create_moo_edit_prefs_notebook (MooPrefsDialogPage *page)/
s/GtkWidget \*window;//
s/window = gtk_window_new (GTK_WINDOW_TOPLEVEL);//
s/gtk_window_set_title (GTK_WINDOW (window), _("window"));//
s/gtk_container_add (GTK_CONTAINER (window), notebook);//
s/language_combo = gtk_combo_box_new_text ();/language_combo = gtk_combo_box_new ();/
s/GLADE_HOOKUP_OBJECT_NO_REF (window, window, "window");//
s/GLADE_HOOKUP_OBJECT (window, notebook, "notebook");/GLADE_HOOKUP_OBJECT_NO_REF (notebook, notebook, "notebook");/
s/[ \t]*GLADE_HOOKUP_OBJECT (window, [a-z_]*[0-9][0-9]*,.*//
s/GLADE_HOOKUP_OBJECT (window, /GLADE_HOOKUP_OBJECT (notebook, /
s/GLADE_HOOKUP_OBJECT_NO_REF (window, tooltips, "tooltips");/GLADE_HOOKUP_OBJECT_NO_REF (notebook, tooltips, "tooltips");/
s/return window;/return notebook;/
s/g_signal_connect ((gpointer) \([a-z_]*\), "moo_prefs_key", G_CALLBACK (\([A-Z_]*\)), NULL);/moo_prefs_dialog_page_bind_setting (MOO_PREFS_DIALOG_PAGE (page), \1, moo_edit_setting (\2), NULL);/
s/g_signal_connect_swapped ((gpointer) \([a-z_]*\), "moo_prefs_key", G_CALLBACK (\([A-Z_]*\)), GTK_OBJECT (\([a-z_]*\)));/moo_prefs_dialog_page_bind_setting (MOO_PREFS_DIALOG_PAGE (page), \1, moo_edit_setting (\2), GTK_TOGGLE_BUTTON (\3));/
s/g_signal_connect ((gpointer) \([a-z_]*\), "moo_sensitive", G_CALLBACK (\([a-z_]*\)), NULL);/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, FALSE);/
s/g_signal_connect_swapped ((gpointer) \([a-z_]*\), "moo_sensitive", G_CALLBACK (\([a-z_]*\)), GTK_OBJECT (invert));/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, TRUE);/
#
s/  language_combo = gtk_combo_box_new ();/#if GTK_MINOR_VERSION < 4\nlanguage_combo = gtk_option_menu_new ();\n#else\nlanguage_combo = gtk_combo_box_new ();\n#endif/
#
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_FOREGROUND/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_FOREGROUND/
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_BACKGROUND/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_BACKGROUND/
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_ITALIC/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_ITALIC/
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_STRIKETHROUGH/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_STRIKETHROUGH/
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_UNDERLINE/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_UNDERLINE/
s/MOO_EDIT_PREFS_BRACKET_CORRECT_MATCH_BOLD/MOO_EDIT_MATCHING_BRACKETS_CORRECT "::" MOO_EDIT_PREFS_BOLD/
#
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_FOREGROUND/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_FOREGROUND/
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_BACKGROUND/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_BACKGROUND/
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_ITALIC/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_ITALIC/
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_STRIKETHROUGH/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_STRIKETHROUGH/
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_UNDERLINE/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_UNDERLINE/
s/MOO_EDIT_PREFS_BRACKET_INCORRECT_MATCH_BOLD/MOO_EDIT_MATCHING_BRACKETS_INCORRECT "::" MOO_EDIT_PREFS_BOLD/
