#ifndef MOO_ENVIRON_H
#define MOO_ENVIRON_H

#include <config.h>

#if defined(MOO_OS_DARWIN)
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#elif !defined(MOO_OS_MINGW)
extern char **environ;
#endif

#endif /* MOO_ENVIRON_H */
