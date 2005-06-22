if [ ! -z $1 ]; then
    if [ ! -e $1 ]; then
        echo $0: error, file $1 doesn\'t exist
        exit 1
    fi
fi

macros () {
cat <<EOF
#include <gtk/gtk.h>

#if GTK_MINOR_VERSION >= 4

#define NEW_TOOLBAR_SEPARATOR(sep,toolbar)                                  \\
    sep = (GtkWidget*) gtk_separator_tool_item_new ();                      \\
    gtk_widget_show (sep);                                                  \\
    gtk_container_add (GTK_CONTAINER (toolbar), sep);

#define NEW_TOOLBAR_SEPARATOR_HIDDEN(sep,toolbar)                           \\
    sep = (GtkWidget*) gtk_separator_tool_item_new ();                      \\
    gtk_container_add (GTK_CONTAINER (toolbar), sep);

#define NEW_TOOL_ITEM_WITH_TOOLTIP(btn,img,label,toolbar,tip)               \\
    NEW_TOOL_ITEM(btn,img,label,toolbar)                                    \\
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (btn), tooltips, _(tip), NULL);

#define NEW_TOOL_ITEM(btn,img,label,toolbar)                                \\
    btn = (GtkWidget*) gtk_tool_button_new (img, _(label));                 \\
    gtk_widget_show (btn);                                                  \\
    gtk_container_add (GTK_CONTAINER (toolbar), btn);                       \\

#define NEW_TOOL_ITEM_HIDDEN(btn,img,label,toolbar)                         \\
    btn = (GtkWidget*) gtk_tool_button_new (img, _(label));                 \\
    gtk_container_add (GTK_CONTAINER (toolbar), btn);                       \\

#define NEW_TOOL_ITEM_HIDDEN_WITH_TOOLTIP(btn,img,label,toolbar,tip)        \\
    btn = (GtkWidget*) gtk_tool_button_new (img, _(label));                 \\
    gtk_container_add (GTK_CONTAINER (toolbar), btn);                       \\
    gtk_tool_item_set_tooltip (GTK_TOOL_ITEM (btn), tooltips, _(tip), NULL);

#else /* GTK_MINOR_VERSION < 4 */

#define NEW_TOOLBAR_SEPARATOR(sep,toolbar)                                      \\
    sep = NULL;                                                                 \\
    gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

#define NEW_TOOLBAR_SEPARATOR_HIDDEN(sep,toolbar)                               \\
    sep = NULL;

#define NEW_TOOL_ITEM_WITH_TOOLTIP(btn,img,label,toolbar,tip)                   \\
    btn = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), label,                \\
                                   tip, tip, img, NULL, NULL);

#define NEW_TOOL_ITEM_HIDDEN_WITH_TOOLTIP(btn,img,label,toolbar,tip)            \\
    btn = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), label,                \\
                                   tip, tip, img, NULL, NULL);                  \\
    gtk_widget_hide (btn);

#define NEW_TOOL_ITEM(btn,img,label,toolbar)                                    \\
    btn = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), label,                \\
                                   NULL, NULL, img, NULL, NULL);

#define NEW_TOOL_ITEM_HIDDEN(btn,img,label,toolbar)                             \\
    btn = gtk_toolbar_append_item (GTK_TOOLBAR (toolbar), label,                \\
                                   NULL, NULL, img, NULL, NULL);                \\
    gtk_widget_hide (btn);

#endif /* GTK_MINOR_VERSION < 4 */

EOF
}

replace () {
    sed -e :a -e '$!N;s/\([a-zA-Z0-9_]*\) = (GtkWidget\*) gtk_separator_tool_item_new ();\n  gtk_widget_show ([a-zA-Z0-9_]*);/TOOL_SEPARATOR \1/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/TOOL_SEPARATOR \([a-zA-Z0-9_]*\)\n  gtk_container_add (GTK_CONTAINER (\([a-zA-Z0-9_]*\)), [a-zA-Z0-9_]*);/NEW_TOOLBAR_SEPARATOR (\1, \2)/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/\([a-zA-Z0-9_]*\) = (GtkWidget\*) gtk_separator_tool_item_new ();\n  gtk_container_add (GTK_CONTAINER (\([a-zA-Z0-9_]*\)), [a-zA-Z0-9_]*);/NEW_TOOLBAR_SEPARATOR_HIDDEN (\1, \2)/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/\([a-zA-Z0-9_]*\) = (GtkWidget\*) gtk_tool_button_new (\([a-zA-Z0-9_]*\), _("\([a-zA-Z0-9_ +,.-]*\)"));\n  gtk_widget_show ([a-zA-Z0-9_]*);/TOOL_ITEM \1 \2 "\3"/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/TOOL_ITEM \([a-zA-Z0-9_]*\) \([a-zA-Z0-9_]*\) "\([a-zA-Z0-9 _+,.-]*\)"\n  gtk_container_add (GTK_CONTAINER (\([a-zA-Z0-9_]*\)), [a-zA-Z0-9_]*);/NEW_TOOL_ITEM (\1, \2, "\3", \4)/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/NEW_TOOL_ITEM (\([a-zA-Z0-9_]*\), \([a-zA-Z0-9_]*\), "\([a-zA-Z0-9 _+,.-]*\)", \([a-zA-Z0-9_]*\))\n  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM ([a-zA-Z0-9_]*), tooltips, _("\([a-zA-Z0-9 _+,.-]*\)"), NULL);/NEW_TOOL_ITEM_WITH_TOOLTIP (\1, \2, "\3", \4, "\5")/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/\([a-zA-Z0-9_]*\) = (GtkWidget\*) gtk_tool_button_new (\([a-zA-Z0-9_]*\), _("\([+,.a-zA-Z0-9 _-]*\)"));\n  gtk_container_add (GTK_CONTAINER (\([a-zA-Z0-9_]*\)), [a-zA-Z0-9_]*);/NEW_TOOL_ITEM_HIDDEN (\1, \2, "\3", \4)/;ta' -e 'P;D' | \
    sed -e :a -e '$!N;s/NEW_TOOL_ITEM_HIDDEN (\([a-zA-Z0-9_]*\), \([a-zA-Z0-9_]*\), "\([,.+a-zA-Z0-9 _-]*\)", \([a-zA-Z0-9_]*\))\n  gtk_tool_item_set_tooltip (GTK_TOOL_ITEM ([a-zA-Z0-9_]*), tooltips, _("\([.,+a-zA-Z0-9_ -]*\)"), NULL);/NEW_TOOL_ITEM_HIDDEN_WITH_TOOLTIP (\1, \2, "\3", \4, "\5")/;ta' -e 'P;D'
}

if [ -z $1 ]; then
    macros
    replace
else
    macros
    cat $1 | replace
fi

