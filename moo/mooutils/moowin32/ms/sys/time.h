#ifndef MOO_SYS_TIME_H
#define MOO_SYS_TIME_H

#ifndef __MINGW32__
/* for struct timeval */
#include <winsock.h>
#else
#include <sys/time.h>
#endif

#define gettimeofday _moo_win32_gettimeofday
int _moo_win32_gettimeofday (struct timeval *tp,
                             void           *tzp);


#endif /* MOO_SYS_TIME_H */
