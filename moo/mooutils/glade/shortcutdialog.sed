s/#include <sys\/types.h>//
s/#include <sys\/stat.h>//
s/#include <unistd.h>//
s/#include <string.h>/#include "mooutils\/moocompat.h"/
s/#include <stdio.h>//
s/#include "callbacks.h"//
s/#include "shortcutdialog.h"//
s/create_dialog (void)/_moo_create_accel_dialog (void);\nGtkWidget *_moo_create_accel_dialog (void)/
