#ifndef MOO_FILE_INFO_H
#define MOO_FILE_INFO_H

#include <gio/gio.h>
#include <mooedit/mooedittypes.h>

G_BEGIN_DECLS

#define MOO_TYPE_FILE_ENC (moo_file_enc_get_type ())

struct _MooFileEnc {
    GFile *file;
    char *encoding;
};

GType        moo_file_enc_get_type      (void) G_GNUC_CONST;

MooFileEnc  *moo_file_enc_new           (GFile      *file,
                                         const char *encoding);
MooFileEnc  *moo_file_enc_new_for_path  (const char *path,
                                         const char *encoding);
MooFileEnc  *moo_file_enc_new_for_uri   (const char *uri,
                                         const char *encoding);
MooFileEnc  *moo_file_enc_copy          (MooFileEnc *fenc);
void         moo_file_enc_free          (MooFileEnc *fenc);

G_END_DECLS

#endif /* MOO_FILE_INFO_H */
