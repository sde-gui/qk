#pragma once

#include <glib.h>
#include <glib/gstdio.h>

G_BEGIN_DECLS

#ifndef __WIN32__
#define MGW_ERROR_IF_NOT_SHARED_LIBC
#else
#define MGW_ERROR_IF_NOT_SHARED_LIBC \
    #error "C libraries may not be shared between medit and glib"
#endif

typedef struct mgw_errno_t mgw_errno_t;
typedef struct MGW_FILE MGW_FILE;

enum mgw_errno_value_t
{
    MGW_ENOERROR = 0,
    MGW_EACCES,
    MGW_EPERM,
    MGW_EEXIST,
    MGW_ELOOP,
    MGW_ENAMETOOLONG,
    MGW_ENOENT,
    MGW_ENOTDIR,
    MGW_EROFS,
    MGW_EXDEV,
};

typedef enum mgw_errno_value_t mgw_errno_value_t;

struct mgw_errno_t
{
    mgw_errno_value_t value;
};

extern const mgw_errno_t MGW_E_NOERROR;
extern const mgw_errno_t MGW_E_EXIST;

inline static gboolean mgw_errno_is_set (mgw_errno_t err) { return err.value != MGW_ENOERROR; }
const char *mgw_strerror (mgw_errno_t err);
GFileError mgw_file_error_from_errno (mgw_errno_t err);

guint64 mgw_ascii_strtoull (const gchar *nptr, gchar **endptr, guint base, mgw_errno_t *err);
gdouble mgw_ascii_strtod (const gchar *nptr, gchar **endptr, mgw_errno_t *err);

MGW_FILE *mgw_fopen (const char *filename, const char *mode, mgw_errno_t *err);
int mgw_fclose (MGW_FILE *file);
gsize mgw_fread(void *ptr, gsize size, gsize nmemb, MGW_FILE *stream, mgw_errno_t *err);
gsize mgw_fwrite(const void *ptr, gsize size, gsize nmemb, MGW_FILE *stream);
int mgw_ferror (MGW_FILE *file);
char *mgw_fgets(char *s, int size, MGW_FILE *stream);

int mgw_unlink (const char *path, mgw_errno_t *err);
int mgw_remove (const char *path, mgw_errno_t *err);
int mgw_mkdir (const gchar *filename, int mode, mgw_errno_t *err);
int mgw_mkdir_with_parents (const gchar *pathname, gint mode, mgw_errno_t *err);


#ifndef MOO_DO_NOT_MANGLE_GLIB_FUNCTIONS

#undef g_stat
#undef g_lstat
#undef g_strerror
#undef g_ascii_strtoull
#undef g_file_error_from_errno
#undef g_ascii_strtod
#undef g_fopen
#undef g_unlink
#undef g_mkdir
#undef g_mkdir_with_parents

#define g_stat DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_lstat DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_strerror DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_ascii_strtoull DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_file_error_from_errno DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_ascii_strtod DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_fopen DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_unlink DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_mkdir DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_mkdir_with_parents DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD

#endif // MOO_DO_NOT_MANGLE_GLIB_FUNCTIONS

G_END_DECLS
