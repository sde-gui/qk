#ifndef MOO_ONCE_H
#define MOO_ONCE_H

#include <glib.h>
#include <mooutils/mooutils-macros.h>

#define MOO_DO_ONCE_BEGIN                       \
do {                                            \
    static gsize _moo_do_once = 0;              \
    if (g_once_init_enter (&_moo_do_once))      \
    {

#define MOO_DO_ONCE_END                         \
        g_once_init_leave (&_moo_do_once, 1);   \
    }                                           \
} while (0);

#ifdef MOO_DEV_MODE

inline static gboolean __moo_test_func()
{
    gboolean value;
    MOO_DO_ONCE_BEGIN
    value = TRUE;
    MOO_DO_ONCE_END
    return value;
}

#endif

#endif /* MOO_ONCE_H */
