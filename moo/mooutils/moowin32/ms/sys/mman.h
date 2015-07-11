#ifndef MOO_SYS_MMAN_H
#define MOO_SYS_MMAN_H

#include <glib.h>

#define mmap	_moo_win32_mmap
#define munmap	_moo_win32_munmap

void    *_moo_win32_mmap    (gpointer        start,
                             guint64         length,
                             int             prot,
                             int             flags,
                             int             fd,
                             guint64         offset);
int      _moo_win32_munmap  (gpointer        start,
                             gsize           length);

#define PROT_READ   1
#define MAP_SHARED  1
#define MAP_FAILED  ((gpointer) -1)

#endif /* MOO_SYS_MMAN_H */
