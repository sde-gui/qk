#include <glib.h>
#include <string.h>

#ifndef g_access
#define g_access access
#endif
#ifndef g_chmod
#define g_chmod chmod
#endif

inline static int
g_strcmp0 (const char *str1,
           const char *str2)
{
    if (!str1)
        return -(str1 != str2);
    if (!str2)
        return str1 != str2;
    return strcmp (str1, str2);
}

#if !GLIB_CHECK_VERSION(2,10,0)
#define G_PARAM_STATIC_NAME 0
#define G_PARAM_STATIC_NICK 0
#define G_PARAM_STATIC_BLURB 0
#endif

#if !GLIB_CHECK_VERSION(2,10,0)
#define g_slice_new0(type) g_new0(type, 1)
#define g_slice_new(type) g_new(type, 1)
#define g_slice_free(type,ptr) g_free(ptr)
#endif

#if !GLIB_CHECK_VERSION(2,12,0)
static gboolean
hash_table_remove_all_func__ (G_GNUC_UNUSED gpointer key,
                              G_GNUC_UNUSED gpointer value,
                              G_GNUC_UNUSED gpointer user_data)
{
    return TRUE;
}

inline static void
g_hash_table_remove_all (GHashTable *hash)
{
    g_hash_table_foreach_remove (hash, hash_table_remove_all_func__, NULL);
}
#endif

#if !GLIB_CHECK_VERSION(2,14,0)

#define	G_PARAM_STATIC_STRINGS 0

inline static gboolean
g_once_init_enter (volatile gsize *value_location)
{
    return *value_location == 0;
}

inline static void
g_once_init_leave (volatile gsize *value_location,
                   gsize           initialization_value)
{
    *value_location = initialization_value;
}

inline static guint
g_timeout_add_seconds (guint       interval,
                       GSourceFunc function,
                       gpointer    data)
{
    return g_timeout_add (1000 * interval, function, data);
}
#endif

inline static void
g_set_error_literal (GError      **err,
                     GQuark        domain,
                     gint          code,
                     const gchar  *message)
{
    g_set_error (err, domain, code, "%s", message);
}

inline static void
g_warn_if_fail_func (gboolean cond, const char *message)
{
    if (!cond)
        g_warning ("assertion `%s' failed", message);
}

#define g_warn_if_fail(what) g_warn_if_fail_func ((what), #what)
#define g_warn_if_reached() g_warning ("warning!")

#if !GLIB_CHECK_VERSION(2,10,0)
inline static void
g_thread_pool_set_sort_function (G_GNUC_UNUSED GThreadPool *pool,
                                 G_GNUC_UNUSED GCompareDataFunc func,
                                 G_GNUC_UNUSED gpointer user_data)
{
}

inline static void
g_thread_pool_set_max_idle_time (G_GNUC_UNUSED guint interval)
{
}
#endif
