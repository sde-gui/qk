#define MOOEDIT_COMPILATION
#include "mooeditbuffer-private.h"
#include "mooeditview.h"
#include "mooutils/mooutils.h"

static void     moo_edit_buffer_finalize    (GObject            *object);
static void     moo_edit_buffer_dispose     (GObject            *object);

G_DEFINE_TYPE (MooEditBuffer, moo_edit_buffer, MOO_TYPE_TEXT_BUFFER)

static void
moo_edit_buffer_class_init (MooEditBufferClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_edit_buffer_finalize;
    gobject_class->dispose = moo_edit_buffer_dispose;

    g_type_class_add_private (klass, sizeof (MooEditBufferPrivate));
}

static void
moo_edit_buffer_init (MooEditBuffer *buffer)
{
    buffer->priv = G_TYPE_INSTANCE_GET_PRIVATE (buffer, MOO_TYPE_EDIT_BUFFER, MooEditBufferPrivate);
}

static void
moo_edit_buffer_finalize (GObject *object)
{
    G_GNUC_UNUSED MooEditBuffer *buffer = MOO_EDIT_BUFFER (object);

    G_OBJECT_CLASS (moo_edit_buffer_parent_class)->finalize (object);
}

static void
moo_edit_buffer_dispose (GObject *object)
{
    MooEditBuffer *buffer = MOO_EDIT_BUFFER (object);

    _moo_edit_buffer_set_view (buffer, NULL);

    G_OBJECT_CLASS (moo_edit_buffer_parent_class)->dispose (object);
}


void
_moo_edit_buffer_set_view (MooEditBuffer *buffer,
                           MooEditView   *view)
{
    moo_return_if_fail (MOO_IS_EDIT_BUFFER (buffer));
    moo_return_if_fail (!view || MOO_IS_EDIT_VIEW (view));
    moo_return_if_fail (!buffer->priv->view || !view);
    buffer->priv->view = view;
}

MooEditView *
moo_edit_buffer_get_view (MooEditBuffer *buffer)
{
    moo_return_val_if_fail (MOO_IS_EDIT_BUFFER (buffer), NULL);
    return buffer->priv->view;
}

MooEdit  *
moo_edit_buffer_get_doc (MooEditBuffer *buffer)
{
    moo_return_val_if_fail (MOO_IS_EDIT_BUFFER (buffer), NULL);
    return moo_edit_view_get_doc (buffer->priv->view);
}
