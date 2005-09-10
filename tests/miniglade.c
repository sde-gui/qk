#include "mooutils/mooglade.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


int main (int argc, char *argv[])
{
    MooGladeXML *my_xml;
    gboolean check = FALSE;
    const char *file;

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

    my_xml = moo_glade_xml_new (file, NULL);

    if (!my_xml)
    {
        g_print ("could not parse '%s'\n", file);
        exit (1);
    }

    moo_glade_xml_unref (my_xml);
    g_print ("*** Success ***\n");

    if (!check)
        gtk_main ();

    return 0;
}
