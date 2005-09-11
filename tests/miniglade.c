#include "mooutils/mooglade.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[])
{
    MooGladeXML *my_xml;
    gboolean check = FALSE;
    const char *file;
    GtkWidget *root;

    gtk_init (&argc, &argv);

    if (argc < 2)
    {
        g_print ("usage: %s [--check] <filename>\n", argv[0]);
        exit (1);
    }

    if (!strcmp (argv[1], "--check"))
    {
        if (argc < 3)
        {
            g_print ("usage: %s [--check] <filename>\n", argv[0]);
            exit (1);
        }
        else
        {
            file = argv[2];
            check = TRUE;
        }
    }
    else
    {
        file = argv[1];
    }

    my_xml = moo_glade_xml_new (file, NULL, NULL);

    if (!my_xml)
    {
        g_print ("could not parse '%s'\n", file);
        exit (1);
    }

    moo_glade_xml_unref (my_xml);
    g_print ("*** Success ***\n");

    if (!check)
    {
        root = moo_glade_xml_get_root (my_xml);

        if (GTK_IS_WINDOW (root))
        {
            gtk_widget_show (root);
            gtk_main ();
        }
    }

    return 0;
}
