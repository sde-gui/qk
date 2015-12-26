#define MOO_DO_NOT_MANGLE_GLIB_FUNCTIONS
#include <mooglib/moo-glib.h>
#include <mooglib/moo-time.h>
#include <mooglib/moo-stat.h>

#include <sys/stat.h>
#include <time.h>


static mgw_time_t convert_time_t (time_t t)
{
    mgw_time_t result = { t };
    return result;
}


static void convert_g_stat_buf (const GStatBuf* gbuf, MgwStatBuf* mbuf)
{
    mbuf->atime = convert_time_t (gbuf->st_atime);
    mbuf->mtime = convert_time_t (gbuf->st_mtime);
    mbuf->ctime = convert_time_t (gbuf->st_ctime);

    mbuf->size = gbuf->st_size;

    mbuf->isreg = S_ISREG (gbuf->st_mode);
    mbuf->isdir = S_ISDIR (gbuf->st_mode);
    mbuf->islnk = S_ISLNK (gbuf->st_mode);
    mbuf->issock = S_ISSOCK (gbuf->st_mode);
    mbuf->isfifo = S_ISFIFO (gbuf->st_mode);
    mbuf->ischr = S_ISCHR (gbuf->st_mode);
    mbuf->isblk = S_ISBLK (gbuf->st_mode);
}


int mgw_stat (const gchar *filename, MgwStatBuf *buf)
{
    GStatBuf gbuf = { 0 };
    int result = g_stat (filename, &gbuf);
    convert_g_stat_buf (&gbuf, buf);
    return result;
}


int mgw_lstat (const gchar *filename, MgwStatBuf *buf)
{
    GStatBuf gbuf = { 0 };
    int result = g_lstat (filename, &gbuf);
    convert_g_stat_buf (&gbuf, buf);
    return result;
}


const struct tm *mgw_localtime(const mgw_time_t *timep)
{
    time_t t = timep->value;
    return localtime(&t);
}
