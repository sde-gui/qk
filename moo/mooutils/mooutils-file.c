#include "config.h"
#include "mooutils-file.h"
#include "mooutils.h"
#include <fnmatch.h>

MOO_DEFINE_OBJECT_ARRAY (MooFileArray, moo_file_array, GFile)

char *
moo_file_get_display_name (GFile *file)
{
    moo_return_val_if_fail (G_IS_FILE (file), NULL);
    return g_file_get_parse_name (file);
}

gboolean
moo_file_fnmatch (GFile      *file,
                  const char *glob)
{
    char *filename;

    moo_return_val_if_fail (G_IS_FILE (file), FALSE);
    moo_return_val_if_fail (glob != NULL, FALSE);

    filename = g_file_get_path (file);
    moo_return_val_if_fail (filename != NULL, FALSE);

    return fnmatch (glob, filename, 0);
}
