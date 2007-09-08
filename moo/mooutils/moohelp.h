#ifndef MOO_HELP_H
#define MOO_HELP_H

#include <gtk/gtkdialog.h>

#define MOO_HELP_ID_CONTENTS "contents"

typedef gboolean (*MooHelpFunc) (GtkWidget *widget,
                                 gpointer   data);

gboolean    moo_help_open               (GtkWidget     *widget);
void        moo_help_open_any           (GtkWidget     *widget);
void        moo_help_open_id            (const char    *id,
                                         GtkWidget     *parent);

void        moo_help_set_id             (GtkWidget     *widget,
                                         const char    *id);
void        moo_help_set_func           (GtkWidget     *widget,
                                         gboolean (*func) (GtkWidget*));
void        moo_help_set_func_full      (GtkWidget     *widget,
                                         MooHelpFunc    func,
                                         gpointer       func_data,
                                         GDestroyNotify notify);

void        moo_help_connect_keys       (GtkWidget     *widget);

#endif /* MOO_HELP_H */
