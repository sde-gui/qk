#include "mooedit/mootextview.h"
#include <gtk/gtk.h>


int main (int argc, char *argv[])
{
    GtkWidget *window, *textview;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_default_size (GTK_WINDOW (window), 500, 400);

    textview = moo_text_view_new ();
    gtk_container_add (GTK_CONTAINER (window), textview);
    gtk_widget_show_all (window);

    gtk_main ();
    return 0;
}
