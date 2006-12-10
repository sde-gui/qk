#define MOOEDIT_COMPILATION
#include "mooedit/moofold.h"
#include "mooedit/mootextbuffer.h"
#include <gtk/gtk.h>
#include <string.h>


int main (int argc, char *argv[])
{
    MooFoldTree *tree;
    MooTextBuffer *buffer;
    char *text;
    MooFold *f1, *f2, *f3, *f4, *f5;

    gtk_init (&argc, &argv);

    buffer = g_object_new (MOO_TYPE_TEXT_BUFFER, NULL);
    text = g_new0 (char, 1000);
    memset (text, '\n', 999);
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (buffer), text, -1);

    tree = _moo_fold_tree_new (buffer);

    f1 = _moo_fold_tree_add (tree, 0, 2);
    g_assert (f1 != NULL);

    g_assert (!_moo_fold_tree_add (tree, 0, 3));
    g_assert (!_moo_fold_tree_add (tree, 2, 3));
    g_assert (!_moo_fold_tree_add (tree, 2, 3));

    f2 = _moo_fold_tree_add (tree, 1, 2);
    g_assert (f2 != NULL);

    f3 = _moo_fold_tree_add (tree, 10, 20);
    g_assert (f3 != NULL);

    g_assert (!_moo_fold_tree_add (tree, 0, 15));
    g_assert (!_moo_fold_tree_add (tree, 3, 15));

    _moo_fold_tree_remove (tree, f1);
    _moo_fold_tree_remove (tree, f2);
    _moo_fold_tree_remove (tree, f3);

    return 0;
}
