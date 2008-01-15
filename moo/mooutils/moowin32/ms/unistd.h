#ifndef MOO_UNISTD_H
#define MOO_UNISTD_H

#include <io.h>

#ifndef S_ISREG
#define S_ISREG(m) (((m) & (_S_IFMT)) == (_S_IFREG))
#define S_ISDIR(m) (((m) & (_S_IFMT)) == (_S_IFDIR))
#endif

#endif /* MOO_UNISTD_H */
