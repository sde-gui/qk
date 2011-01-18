#ifndef MOO_EDIT_WINDOW_PRIV_H
#define MOO_EDIT_WINDOW_PRIV_H

#include "mooeditwindow-impl.h"

G_BEGIN_DECLS

struct MooEditTab
{
    GtkVBox base;

    MooEditProgress *progress;

    GtkWidget *hpaned;
    GtkWidget *vpaned1;
    GtkWidget *vpaned2;

    MooEdit *doc;
    MooEditView *active_view;
};

struct MooEditTabClass
{
    GtkVBoxClass base_class;
};

G_END_DECLS

#endif /* MOO_EDIT_WINDOW_PRIV_H */
