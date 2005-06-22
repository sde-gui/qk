s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <unistd.h>//
s/#include <string.h>/#include "mooutils\/moocompat.h"/
s/#include <stdio.h>/#include "mooutils\/mooprefsdialog.h"/
s/#include "callbacks.h"/#include "mooutils\/mooaccelbutton.h"/
s/#include "shortcutsprefs.h"//
s/create_window (void)/_moo_create_shortcutsprefs_page (GtkWidget *page);\nGtkWidget *_moo_create_shortcutsprefs_page (GtkWidget *page)/
s/create_dialog (void)/_moo_create_shortcutsprefs_dialog (GtkWidget *page);\nGtkWidget *_moo_create_shortcutsprefs_dialog (GtkWidget *page)/
s/GtkWidget \*window;//
s/GtkWidget \*page;//
s/window = gtk_window_new (GTK_WINDOW_TOPLEVEL);//
s/page = gtk_vbox_new (FALSE, .*//
s/gtk_widget_show (page);//
s/shortcut = gtk_button_new_with_mnemonic ("");/shortcut = moo_accel_button_new (NULL);/
s/gtk_container_add (GTK_CONTAINER (window), page);//
s/[ \t]*GLADE_HOOKUP_OBJECT (window, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT (dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (window, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (dialog, [a-z_]*[0-9][0-9]*,.*//
s/GLADE_HOOKUP_OBJECT_NO_REF (window, window, \"window\");//
s/GLADE_HOOKUP_OBJECT (window, page, \"page\");/GLADE_HOOKUP_OBJECT_NO_REF (page, page, \"page\");/
s/GLADE_HOOKUP_OBJECT (window, /GLADE_HOOKUP_OBJECT (page, /
s/GLADE_HOOKUP_OBJECT_NO_REF (window, tooltips, \"tooltips\");/GLADE_HOOKUP_OBJECT_NO_REF (page, tooltips, \"tooltips\");/
s/return window;/return GTK_WIDGET (page);/
s/g_signal_connect ((gpointer) \([A-Za-z0-9_]*\), "moo_prefs_key", G_CALLBACK (\([A-Za-z0-9_]*\)), NULL);/moo_prefs_dialog_page_bind_setting (page, \1, moo_term_setting (\2));/
s/g_signal_connect ((gpointer) \([A-Za-z0-9_]*\), "moo_sensitive", G_CALLBACK (\([A-Za-z0-9_]*\)), NULL);/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, FALSE);/
s/g_signal_connect_swapped ((gpointer) \([A-Za-z0-9_]*\), "moo_sensitive", G_CALLBACK (\([A-Za-z0-9_]*\)), GTK_OBJECT (invert));/moo_bind_sensitive (GTK_TOGGLE_BUTTON (\2), \&\1, 1, TRUE);/
s/g_signal_connect_swapped ((gpointer) \([A-Za-z0-9_]*\), "moo_radio", G_CALLBACK (\([A-Za-z0-9_]*\)), GTK_OBJECT (\([A-Za-z0-9_]*\)));/moo_prefs_dialog_page_bind_radio (page, moo_term_setting (\2), GTK_TOGGLE_BUTTON (\1), \3);/
