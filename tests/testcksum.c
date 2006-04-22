#include "mooutils/moocksum.h"
#include <gtk/gtk.h>
#include <stdlib.h>


int main (int argc, char *argv[])
{
    int i;

    if (argc < 2)
    {
        g_print ("usage: %s <file> [<file2> ...]\n", argv[0]);
        exit (1);
    }

    for (i = 1; i < argc; ++i)
    {
        char *sum = moo_cksum (argv[i]);
        g_print ("%s  %s\n", sum ? sum : "*** ERROR ***", argv[i]);
        g_free (sum);
    }

    return 0;
}
