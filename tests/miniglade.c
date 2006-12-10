#include "mooutils/mooglade.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


static void
show_it (MooGladeXML *xml,
         const char  *name)
{
    GtkWidget *window, *widget;

    if (name)
        widget = moo_glade_xml_get_widget (xml, name);
    else
        widget = moo_glade_xml_get_root (xml);

    g_return_if_fail (widget != NULL);

    if (GTK_IS_WINDOW (widget))
    {
        window = widget;
    }
    else
    {
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_container_add (GTK_CONTAINER (window), widget);
    }

    gtk_widget_show (widget);
    gtk_widget_show (window);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_main ();
}

int main (int argc, char *argv[])
{
    MooGladeXML *xml;
    gboolean parse_only = FALSE;
    char *root = NULL;
    GOptionContext *opt_ctx;
    GError *error = NULL;
    const char *file;

    GOptionEntry options[] = {
        { "parse-only", 0, 0, G_OPTION_ARG_NONE, &parse_only, "do not show widgets", NULL },
        { "root", 0, 0, G_OPTION_ARG_STRING, &root, "root widget to show", "ROOT" },
        { NULL, 0, 0, 0, NULL, NULL, NULL },
    };

    opt_ctx = g_option_context_new ("FILE - test glade parser");
    g_option_context_add_main_entries (opt_ctx, options, NULL);
    g_option_context_add_group (opt_ctx, gtk_get_option_group (FALSE));

    if (!g_option_context_parse (opt_ctx, &argc, &argv, &error))
    {
        g_print ("%s\n", error->message);
        exit (1);
    }

    gtk_init_check (NULL, NULL);

    if (argc != 2)
    {
        g_print ("usage: %s [OPTIONS] FILE\n", argv[0]);
        exit (1);
    }

    file = argv[1];
    xml = moo_glade_xml_new (file, root, NULL, &error);

    if (!xml)
    {
        g_print ("could not parse file '%s': %s\n", file, error->message);
        exit (1);
    }

    g_print ("*** Success ***\n");

    if (!parse_only)
        show_it (xml, root);

    g_object_unref (xml);
    return 0;
}
