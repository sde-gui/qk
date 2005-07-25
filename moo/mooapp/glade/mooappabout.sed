s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <string.h>//
s/#include <stdio.h>//
s/#include <unistd.h>//
s/#include "mooappabout.h"/#include "mooutils\/moocompat.h"/
s/#include "callbacks.h"//
s/#include "support.h"//
s/create_about_dialog (void)/_moo_app_create_about_dialog (const char *comment, const char *copyright, const char *name, const char *logo_name);\nGtkWidget *_moo_app_create_about_dialog (const char *comment, const char *copyright, const char *name, const char *logo_name)/
s/name_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/name_label = gtk_label_new (_(name));/
s/comment_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/comment_label = gtk_label_new (_(comment));/
s/copyright_label = gtk_label_new (_("[a-zA-Z0-9_<>/-]*"));/copyright_label = gtk_label_new (_(copyright));/
s/logo = gtk_image_new_from_stock ("[a-zA-Z0-9_-]*", GTK_ICON_SIZE_DIALOG);/logo = gtk_image_new_from_stock (logo_name, GTK_ICON_SIZE_DIALOG);/
s/[ \t]*GLADE_HOOKUP_OBJECT (about_dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (about_dialog, [a-z_]*[0-9][0-9]*,.*//
