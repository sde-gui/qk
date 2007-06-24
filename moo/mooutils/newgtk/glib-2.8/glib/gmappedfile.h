#ifndef G_MAPPED_FILE_MANGLED_H
#define G_MAPPED_FILE_MANGLED_H

#include <glib.h>
#include <glib/gstdio.h>

#define MANGLE_MF_PREFIX moo
#define MANGLE_MF(func) MANGLE_MF2(MANGLE_MF_PREFIX,func)
#define MANGLE_MF2(prefix,func) MANGLE_MF3(prefix,func)
#define MANGLE_MF3(prefix,func) prefix##_##func

#define GMappedFile                 MANGLE_MF(GMappedFile)
#define _GMappedFile                MANGLE_MF(_GMappedFile)
#define g_mapped_file_new           MANGLE_MF(g_mapped_file_new)
#define g_mapped_file_get_length    MANGLE_MF(g_mapped_file_get_length)
#define g_mapped_file_get_contents  MANGLE_MF(g_mapped_file_get_contents)
#define g_mapped_file_free          MANGLE_MF(g_mapped_file_free)

#include "gmappedfile-real.h"

#endif /* G_MAPPED_FILE_MANGLED_H */
