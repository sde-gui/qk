#ifndef MOO_APP_ACCELS_H
#define MOO_APP_ACCELS_H

#include <gtk/gtk.h>

#ifndef GDK_WINDOWING_QUARTZ

#define MOO_APP_ACCEL_HELP "F1"
#define MOO_APP_ACCEL_QUIT "<Ctrl>Q"

#else /* GDK_WINDOWING_QUARTZ */

#define MOO_APP_ACCEL_HELP "<Meta>question"
#define MOO_APP_ACCEL_QUIT "<Meta>Q"

#endif /* GDK_WINDOWING_QUARTZ */

#endif /* MOO_APP_ACCELS_H */
