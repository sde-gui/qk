s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <string.h>//
s/#include <stdio.h>//
s/#include <unistd.h>/#include "mooedit\/mooeditsearch.h"/
s/#include "mooeditfind.h"//
s/#include "callbacks.h"/#include "mooutils\/moocompat.h"/
s/create_dialog (void)/_moo_edit_create_find_dialog (gboolean replace);\nGtkWidget*\n_moo_edit_create_find_dialog (gboolean replace)/
s/create_prompt_on_replace_dialog (void)/_moo_edit_create_prompt_on_replace_dialog (void);\nGtkWidget*\n_moo_edit_create_prompt_on_replace_dialog (void)/
s/gtk_window_set_title (GTK_WINDOW (dialog), _("Find"));/if (replace) gtk_window_set_title (GTK_WINDOW (dialog), _("Replace Text"));\n  else gtk_window_set_title (GTK_WINDOW (dialog), _("Find Text"));/
s/gtk_widget_show (replace_frame);/if (replace) gtk_widget_show (replace_frame);/
s/gtk_widget_show (dont_prompt_on_replace);/if (replace) gtk_widget_show (dont_prompt_on_replace);/
s/ok_btn = gtk_button_new_from_stock ("gtk-find");/if (replace) ok_btn = gtk_button_new_from_stock (GTK_STOCK_FIND_AND_REPLACE); else ok_btn = gtk_button_new_from_stock (GTK_STOCK_FIND);/
s/[ \t]*GLADE_HOOKUP_OBJECT (dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT (prompt_on_replace_dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (prompt_on_replace_dialog, [a-z_]*[0-9][0-9]*,.*//
