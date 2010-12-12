#include "moo-lua-api-util.h"

// methods of GObject

// methods of GFile

// methods of GdkPixbuf

// methods of GtkObject

// methods of GtkAccelGroup

// methods of GtkWidget

void
gtk_lua_api_register (void)
{
    static gboolean been_here = FALSE;

    if (been_here)
        return;

    been_here = TRUE;

}
