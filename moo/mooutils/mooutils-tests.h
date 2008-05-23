#ifndef MOO_UTILS_TESTS_H
#define MOO_UTILS_TESTS_H

#include "moo-test-macros.h"

G_BEGIN_DECLS

void    moo_test_gobject            (void);
void    moo_test_mooaccel           (void);
void    moo_test_mooutils_fs        (void);
void    moo_test_moo_file_writer    (void);
void    moo_test_mooutils_misc      (void);

#ifdef __WIN32__
void    moo_test_mooutils_win32     (void);
#endif

G_END_DECLS

#endif /* MOO_UTILS_TESTS_H */
