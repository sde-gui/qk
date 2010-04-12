#ifndef MOO_STAT_H
#define MOO_STAT_H

#include <glib/gstdio.h>

#if defined(G_OS_WIN32) && GLIB_CHECK_VERSION(2,22,0)

inline static void _moo_check_stat_struct(void)
{
    struct _g_stat_struct statbuf;
    // check that _g_stat_struct is the same as plain struct stat
    _off_t *pofft = &statbuf.st_size;
    time_t *ptimet = &statbuf.st_atime;
    (void) pofft;
    (void) ptimet;
}

#endif

G_BEGIN_DECLS

inline static int moo_stat (const char *filename, struct stat *buf)
{
#if defined(G_OS_WIN32) && GLIB_CHECK_VERSION(2,22,0)
    /* _moo_check_stat_struct above checks that struct stat is okay,
       cast to void* is to avoid using glib's internal _g_stat_struct */
    return g_stat (filename, (void*) buf);
#else
    return g_stat (filename, buf);
#endif
}

inline static int moo_lstat (const char *filename, struct stat *buf)
{
#if defined(G_OS_WIN32) && GLIB_CHECK_VERSION(2,22,0)
    /* _moo_check_stat_struct above checks that struct stat is okay,
       cast to void* is to avoid using glib's internal _g_stat_struct */
    return g_lstat (filename, (void*) buf);
#else
    return g_lstat (filename, buf);
#endif
}

G_END_DECLS

#endif /* MOO_STAT_H */
