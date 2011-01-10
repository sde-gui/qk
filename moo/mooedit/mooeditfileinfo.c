/**
 * class:MooOpenInfo: (parent GObject)
 **/

/**
 * class:MooSaveInfo: (parent GObject)
 **/

/**
 * class:MooReloadInfo: (parent GObject)
 **/

#include "mooeditfileinfo.h"

static void moo_open_info_class_init   (MooOpenInfoClass *klass);
static void moo_save_info_class_init   (MooSaveInfoClass *klass);
static void moo_reload_info_class_init (MooReloadInfoClass *klass);

MOO_DEFINE_OBJECT_ARRAY (MooOpenInfo, moo_open_info)

G_DEFINE_TYPE (MooOpenInfo, moo_open_info, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooSaveInfo, moo_save_info, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooReloadInfo, moo_reload_info, G_TYPE_OBJECT)

/**
 * moo_open_info_new: (constructor-of MooOpenInfo)
 *
 * @file:
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 **/
MooOpenInfo *
moo_open_info_new (GFile      *file,
                   const char *encoding)
{
    MooOpenInfo *info;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    info = g_object_new (MOO_TYPE_OPEN_INFO, NULL);

    info->file = g_file_dup (file);
    info->encoding = g_strdup (encoding);
    info->line = -1;

    return info;
}

/**
 * moo_open_info_new_path: (static-method-of MooOpenInfo)
 *
 * @path: (type const-filename)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_new_path (const char *path,
                        const char *encoding)
{
    GFile *file = g_file_new_for_path (path);
    MooOpenInfo *info = moo_open_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_open_info_new_uri: (static-method-of MooOpenInfo)
 *
 * @uri: (type const-utf8)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_new_uri (const char *uri,
                       const char *encoding)
{
    GFile *file = g_file_new_for_uri (uri);
    MooOpenInfo *info = moo_open_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_open_info_dup:
 *
 * Returns: (transfer full)
 **/
MooOpenInfo *
moo_open_info_dup (MooOpenInfo *info)
{
    MooOpenInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_open_info_new (info->file, info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    copy->flags = info->flags;
    copy->line = info->line;

    return copy;
}

void
moo_open_info_free (MooOpenInfo *info)
{
    if (info)
        g_object_unref (info);
}

static void
moo_open_info_finalize (GObject *object)
{
    MooOpenInfo *info = (MooOpenInfo*) object;

    g_object_unref (info->file);
    g_free (info->encoding);

    G_OBJECT_CLASS (moo_open_info_parent_class)->finalize (object);
}

static void
moo_open_info_class_init (MooOpenInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_open_info_finalize;
}

static void
moo_open_info_init (MooOpenInfo *info)
{
    info->line = -1;
}


/**
 * moo_save_info_new: (constructor-of MooSaveInfo)
 *
 * @file:
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 **/
MooSaveInfo *
moo_save_info_new (GFile      *file,
                   const char *encoding)
{
    MooSaveInfo *info;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    info = g_object_new (MOO_TYPE_SAVE_INFO, NULL);

    info->file = g_file_dup (file);
    info->encoding = g_strdup (encoding);

    return info;
}

/**
 * moo_save_info_new_path: (static-method-of MooSaveInfo)
 *
 * @path: (type const-filename)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_new_path (const char *path,
                        const char *encoding)
{
    GFile *file = g_file_new_for_path (path);
    MooSaveInfo *info = moo_save_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_save_info_new_uri: (static-method-of MooSaveInfo)
 *
 * @uri: (type const-utf8)
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_new_uri (const char *uri,
                       const char *encoding)
{
    GFile *file = g_file_new_for_uri (uri);
    MooSaveInfo *info = moo_save_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_save_info_dup:
 *
 * Returns: (transfer full)
 **/
MooSaveInfo *
moo_save_info_dup (MooSaveInfo *info)
{
    MooSaveInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_save_info_new (info->file, info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    return copy;
}

void
moo_save_info_free (MooSaveInfo *info)
{
    if (info)
        g_object_unref (info);
}

static void
moo_save_info_finalize (GObject *object)
{
    MooSaveInfo *info = (MooSaveInfo*) object;

    g_object_unref (info->file);
    g_free (info->encoding);

    G_OBJECT_CLASS (moo_save_info_parent_class)->finalize (object);
}

static void
moo_save_info_class_init (MooSaveInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_save_info_finalize;
}

static void
moo_save_info_init (G_GNUC_UNUSED MooSaveInfo *info)
{
}


/**
 * moo_reload_info_new: (constructor-of MooReloadInfo)
 *
 * @encoding: (type const-utf8) (allow-none) (default NULL)
 **/
MooReloadInfo *
moo_reload_info_new (const char *encoding)
{
    MooReloadInfo *info;

    info = g_object_new (MOO_TYPE_RELOAD_INFO, NULL);

    info->encoding = g_strdup (encoding);
    info->line = -1;

    return info;
}

/**
 * moo_reload_info_dup:
 *
 * Returns: (transfer full)
 **/
MooReloadInfo *
moo_reload_info_dup (MooReloadInfo *info)
{
    MooReloadInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_reload_info_new (info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    copy->line = info->line;

    return copy;
}

void
moo_reload_info_free (MooReloadInfo *info)
{
    if (info)
        g_object_unref (info);
}

static void
moo_reload_info_finalize (GObject *object)
{
    MooReloadInfo *info = (MooReloadInfo*) object;

    g_free (info->encoding);

    G_OBJECT_CLASS (moo_reload_info_parent_class)->finalize (object);
}

static void
moo_reload_info_class_init (MooReloadInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_reload_info_finalize;
}

static void
moo_reload_info_init (MooReloadInfo *info)
{
    info->line = -1;
}
