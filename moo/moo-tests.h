#ifndef MOO_TESTS_H
#define MOO_TESTS_H

#include "moo-test-macros.h"

G_BEGIN_DECLS


void    moo_test_gobject            (void);
void    moo_test_mooaccel           (void);
void    moo_test_mooutils_fs        (void);
void    moo_test_moo_file_writer    (void);
void    moo_test_mooutils_misc      (void);
void    moo_test_lua                (void);

#ifdef __WIN32__
void    moo_test_mooutils_win32     (void);
#endif

void    moo_test_mooedit_lua_api    (void);
void    moo_test_key_file           (void);
void    moo_test_editor             (void);


G_END_DECLS

#endif /* MOO_TESTS_H */
