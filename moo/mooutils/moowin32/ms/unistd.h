#ifndef MOO_UNISTD_H
#define MOO_UNISTD_H

#include <io.h>

#ifndef S_ISREG
#define S_ISREG(m) (((m) & (_S_IFMT)) == (_S_IFREG))
#define S_ISDIR(m) (((m) & (_S_IFMT)) == (_S_IFDIR))
#endif

#define F_OK 0x0
#define R_OK 0x4
#define W_OK 0x2
#define X_OK 0x0

#endif /* MOO_UNISTD_H */
