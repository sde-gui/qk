#pragma once

/* for struct timeval */
#include <winsock.h>

G_BEGIN_DECLS

#define gettimeofday _moo_win32_gettimeofday
int _moo_win32_gettimeofday (struct timeval *tp,
                             void           *tzp);

G_END_DECLS
