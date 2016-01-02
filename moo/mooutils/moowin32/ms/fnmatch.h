#pragma once

// #include "mooutils/mooutils-misc.h"
// #include "mooutils/mooutils-fs.h"

#define fnmatch _moo_win32_fnmatch

#ifdef __cplusplus
extern "C"
#endif
int _moo_win32_fnmatch  (const char *pattern,
                         const char *string,
                         int         flags);
