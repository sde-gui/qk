s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <unistd.h>//
s/#include <string.h>/#include "mooutils\/moocompat.h"/
s/#include <stdio.h>/#include "mooterm\/mooterm.h"/
s/#include "callbacks.h"/#include "mooutils\/mooprefsdialog.h"/
s/#include "mootermprefs.h"/#include "mooterm\/mootermprefs.h"/
s/create_term_prefs_window (void)/_create_moo_term_prefs_notebook (MooPrefsDialogPage *page);\nGtkWidget *_create_moo_term_prefs_notebook (MooPrefsDialogPage *page)/
s/GtkWidget \*term_prefs_window;//
s/term_prefs_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);//
s/gtk_window_set_title (GTK_WINDOW (term_prefs_window), _(\"Terminal Preferences\"));//
s/gtk_container_add (GTK_CONTAINER (term_prefs_window), notebook);//
s/[ \t]*GLADE_HOOKUP_OBJECT (term_prefs_window, [a-z_]*[0-9][0-9]*,.*//
s/GLADE_HOOKUP_OBJECT_NO_REF (term_prefs_window, term_prefs_window, \"term_prefs_window\");//
s/GLADE_HOOKUP_OBJECT (term_prefs_window, notebook, \"notebook\");/GLADE_HOOKUP_OBJECT_NO_REF (notebook, notebook, \"notebook\");/
s/GLADE_HOOKUP_OBJECT (term_prefs_window, /GLADE_HOOKUP_OBJECT (notebook, /
s/GLADE_HOOKUP_OBJECT_NO_REF (term_prefs_window, tooltips, \"tooltips\");/GLADE_HOOKUP_OBJECT_NO_REF (notebook, tooltips, \"tooltips\");/
s/return term_prefs_window;/return notebook;/
s/g_signal_connect ((gpointer) \([A-Za-z0-9_]*\), "moo_prefs_key", G_CALLBACK (\([A-Za-z0-9_]*\)), NULL);/moo_prefs_dialog_page_bind_setting (page, \1, moo_term_setting (\2), NULL);/
s/g_signal_connect ((gpointer) \([A-Za-z0-9_]*\), "moo_sensitive", G_CALLBACK (\([A-Za-z0-9_]*\)), NULL);/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, FALSE);/
s/g_signal_connect_swapped ((gpointer) \([A-Za-z0-9_]*\), "moo_sensitive", G_CALLBACK (\([A-Za-z0-9_]*\)), GTK_OBJECT (invert));/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, TRUE);/
s/g_signal_connect_swapped ((gpointer) \([A-Za-z0-9_]*\), "moo_radio", G_CALLBACK (\([A-Za-z0-9_]*\)), GTK_OBJECT (\([A-Za-z0-9_]*\)));/moo_prefs_dialog_page_bind_radio (page, moo_term_setting (\2), GTK_TOGGLE_BUTTON (\1), \3);/
