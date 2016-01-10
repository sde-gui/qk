#pragma once

// #include "mooutils/mooutils-misc.h"
// #include "mooutils/mooutils-fs.h"

G_BEGIN_DECLS

#define fnmatch _moo_win32_fnmatch
int _moo_win32_fnmatch  (const char *pattern,
                         const char *string,
                         int         flags);

G_END_DECLS
