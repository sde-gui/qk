#ifndef MOO_ONCE_H
#define MOO_ONCE_H

#include <glib.h>

#if !GLIB_CHECK_VERSION(2,14,0)
inline static gboolean
moo_once_init_enter (volatile gsize *value_location)
{
    return *value_location == 0;
}

inline static void
moo_once_init_leave (volatile gsize *value_location,
                     gsize           initialization_value)
{
    *value_location = initialization_value;
}
#elif !GLIB_CHECK_VERSION(2,16,0)
inline static gboolean
moo_once_init_enter (volatile gsize *value_location)
{
    return g_once_init_enter ((volatile gpointer*) value_location);
}

inline static void
moo_once_init_leave (volatile gsize *value_location,
                     gsize           initialization_value)
{
    g_once_init_leave ((volatile gpointer*) value_location,
                       (gpointer) initialization_value);
}
#else
#define moo_once_init_enter g_once_init_enter
#define moo_once_init_leave g_once_init_leave
#endif

#define MOO_BEGIN_DO_ONCE                       \
do {                                            \
    static gsize _moo_do_once = 0;              \
    if (moo_once_init_enter (&_moo_do_once))    \
    {

#define MOO_END_DO_ONCE                         \
        moo_once_init_leave (&_moo_do_once, 1); \
    }                                           \
} while (0);

#endif /* MOO_ONCE_H */
