s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <string.h>//
s/#include <stdio.h>//
s/#include <unistd.h>//
s/#include "mooeditgotoline.h"//
s/#include "callbacks.h"/#include "mooutils\/moocompat.h"/
s/create_dialog (void)/_moo_edit_create_go_to_line_dialog (void);\nGtkWidget*\n_moo_edit_create_go_to_line_dialog (void)/
s/[ \t]*GLADE_HOOKUP_OBJECT (dialog, [a-z_]*[0-9][0-9]*,.*//
s/[ \t]*GLADE_HOOKUP_OBJECT_NO_REF (dialog, [a-z_]*[0-9][0-9]*,.*//
