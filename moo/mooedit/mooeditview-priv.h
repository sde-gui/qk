#ifndef MOO_EDIT_VIEW_PRIV_H
#define MOO_EDIT_VIEW_PRIV_H

#include "mooedit/mooeditview-impl.h"

G_BEGIN_DECLS

#define PROGRESS_TIMEOUT    100
#define PROGRESS_WIDTH      300
#define PROGRESS_HEIGHT     100

struct MooEditViewPrivate
{
    MooEdit *doc;
    MooEditor *editor;
    MooEditTab *tab;

    MooEditState state;
    guint progress_timeout;
    GtkWidget *progress;
    GtkWidget *progressbar;
    char *progress_text;
    GDestroyNotify cancel_op;
    gpointer cancel_data;
};

G_END_DECLS

#endif /* MOO_EDIT_VIEW_PRIV_H */
