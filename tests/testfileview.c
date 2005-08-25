#include "moofileview/moofileview.h"


int main (int argc, char *argv[])
{
    GtkWidget *window, *tree;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);
    gtk_window_set_default_size (GTK_WINDOW (window), -1, 500);

    tree = moo_file_view_new ();
    gtk_container_add (GTK_CONTAINER (window), tree);

    moo_file_view_chdir (MOO_FILE_VIEW (tree),
                         g_get_home_dir (), NULL);;

    gtk_widget_show_all (window);

    gtk_main ();
    return 0;
}
