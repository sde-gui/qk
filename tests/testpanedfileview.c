#include "mooutils/moopaned.h"
#include "mooedit/moofileview.h"


static int WINDOWS = 0;

static void window_destroyed (void)
{
    if (!--WINDOWS) gtk_main_quit ();
}


static void create_window_with_paned (GtkPositionType pane_position)
{
    GtkWidget *window, *paned, *textview, *swin, *fileview;
    GtkTextBuffer *buffer;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (window_destroyed), NULL);
    WINDOWS++;

    paned = moo_paned_new (pane_position);
    gtk_widget_show (paned);
    gtk_container_add (GTK_CONTAINER (window), paned);

    textview = gtk_text_view_new ();
    gtk_widget_show (textview);
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_show (swin);
    gtk_container_add (GTK_CONTAINER (paned), swin);
    gtk_container_add (GTK_CONTAINER (swin), textview);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview));
    gtk_text_buffer_insert_at_cursor (buffer, "Click a button. Click a button. "
            "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. "
                    "Click a button. Click a button. Click a button. Click a button. ",
            -1);

    fileview = moo_file_view_new ();
    moo_file_view_chdir (MOO_FILE_VIEW (fileview),
                         g_get_home_dir (), NULL);;
    moo_paned_add_pane (MOO_PANED (paned),
                        fileview,
                        "TextView",
                        GTK_STOCK_OK);
    moo_paned_add_pane (MOO_PANED (paned),
                        gtk_label_new ("This is a label"),
                        "Label",
                        GTK_STOCK_CANCEL);

    gtk_widget_grab_focus (textview);
    gtk_widget_show_all (window);
}


int main (int argc, char *argv[])
{
    gtk_init (&argc, &argv);

//     gdk_window_set_debug_updates (TRUE);

//     create_window_with_paned (GTK_POS_RIGHT);
    create_window_with_paned (GTK_POS_LEFT);
//     create_window_with_paned (GTK_POS_TOP);
//     create_window_with_paned (GTK_POS_BOTTOM);

    gtk_main ();
    return 0;
}
