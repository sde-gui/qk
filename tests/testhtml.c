#include "mooapp/moohtml.h"
#include <gtk/gtk.h>
#include <stdlib.h>


static void
set_title (MooHtml      *html,
           G_GNUC_UNUSED GParamSpec *pspec,
           GtkWindow    *window)
{
    char *title;
    g_object_get (html, "title", &title, NULL);
    gtk_window_set_title (window, title);
    g_free (title);
}

static void
hover_link (GtkStatusbar *statusbar,
            const char   *link)
{
    gtk_statusbar_pop (statusbar, 0);

    if (link)
        gtk_statusbar_push (statusbar, 0, link);
}


int main (int argc, char *argv[])
{
    GtkWidget *window, *swin, *html, *statusbar, *vbox;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vbox), swin, TRUE, TRUE, 0);

    html = moo_html_new ();
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (html), 6);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (html), 6);
    gtk_container_add (GTK_CONTAINER (swin), html);
    moo_html_set_font (MOO_HTML (html), "Tahoma 12");

    statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);

    g_signal_connect (html, "notify::title",
                      G_CALLBACK (set_title), window);
    g_signal_connect_swapped (html, "hover-link",
                              G_CALLBACK (hover_link), statusbar);

    if (!argv[1])
    {
        g_print ("usage: %s <file.html>\n", argv[0]);
        exit (1);
    }

    if (!moo_html_load_file (MOO_HTML (html), argv[1], NULL))
    {
        g_error ("ERROR");
    }

    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}
