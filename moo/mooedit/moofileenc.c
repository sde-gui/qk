/**
 * boxed:MooFileEnc:
 **/

#include "config.h"
#include "moofileenc.h"

MOO_DEFINE_BOXED_TYPE_C (MooFileEnc, moo_file_enc)

MOO_DEFINE_PTR_ARRAY (MooFileEncArray, moo_file_enc_array, MooFileEnc,
                      moo_file_enc_copy, moo_file_enc_free)

MooFileEnc *
moo_file_enc_new (GFile      *file,
                  const char *encoding)
{
    MooFileEnc *fenc;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    fenc = g_slice_new0 (MooFileEnc);
    fenc->file = g_file_dup (file);
    fenc->encoding = g_strdup (encoding);

    return fenc;
}

MooFileEnc *
moo_file_enc_copy (MooFileEnc *fenc)
{
    g_return_val_if_fail (fenc != NULL, NULL);
    return moo_file_enc_new (fenc->file, fenc->encoding);
}

void
moo_file_enc_free (MooFileEnc *fenc)
{
    if (fenc)
    {
        g_object_unref (fenc->file);
        g_free (fenc->encoding);
        g_slice_free (MooFileEnc, fenc);
    }
}

MooFileEnc *
moo_file_enc_new_for_path (const char *path,
                           const char *encoding)
{
    GFile *file = g_file_new_for_path (path);
    MooFileEnc *fenc = moo_file_enc_new (file, encoding);
    g_object_unref (file);
    return fenc;
}

MooFileEnc *
moo_file_enc_new_for_uri (const char *uri,
                          const char *encoding)
{
    GFile *file = g_file_new_for_uri (uri);
    MooFileEnc *fenc = moo_file_enc_new (file, encoding);
    g_object_unref (file);
    return fenc;
}
