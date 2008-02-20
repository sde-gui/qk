#ifndef MOO_APP_IPC_H
#define MOO_APP_IPC_H

#include <glib-object.h>

G_BEGIN_DECLS


typedef void (*MooDataCallback) (GObject    *object,
                                 const char *data,
                                 gsize       len);

void    moo_ipc_register_client     (GObject        *object,
                                     const char     *id,
                                     MooDataCallback callback);
void    moo_ipc_unregister_client   (GObject        *object,
                                     const char     *id);

void    moo_ipc_send                (GObject        *sender,
                                     const char     *id,
                                     const char     *data,
                                     gssize          len);

void   _moo_ipc_dispatch            (const char *data,
                                     gsize       len);


G_END_DECLS

#endif /* MOO_APP_IPC_H */
