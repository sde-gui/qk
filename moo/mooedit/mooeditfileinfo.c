/**
 * class:MooEditOpenInfo: (parent GObject)
 **/

/**
 * class:MooEditSaveInfo: (parent GObject)
 **/

/**
 * class:MooEditReloadInfo: (parent GObject)
 **/

#include "mooeditfileinfo.h"

static void moo_edit_open_info_class_init   (MooEditOpenInfoClass *klass);
static void moo_edit_save_info_class_init   (MooEditSaveInfoClass *klass);
static void moo_edit_reload_info_class_init (MooEditReloadInfoClass *klass);

MOO_DEFINE_OBJECT_ARRAY (MooEditOpenInfo, moo_edit_open_info)

G_DEFINE_TYPE (MooEditOpenInfo, moo_edit_open_info, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooEditSaveInfo, moo_edit_save_info, G_TYPE_OBJECT)
G_DEFINE_TYPE (MooEditReloadInfo, moo_edit_reload_info, G_TYPE_OBJECT)

/**
 * moo_edit_open_info_new: (constructor-of MooEditOpenInfo)
 **/
MooEditOpenInfo *
moo_edit_open_info_new (GFile      *file,
                        const char *encoding)
{
    MooEditOpenInfo *info;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    info = g_object_new (MOO_TYPE_EDIT_OPEN_INFO, NULL);

    info->file = g_file_dup (file);
    info->encoding = g_strdup (encoding);
    info->line = -1;

    return info;
}

/**
 * moo_edit_open_info_new_path:
 *
 * Returns: (transfer full)
 **/
MooEditOpenInfo *
moo_edit_open_info_new_path (const char *path,
                             const char *encoding)
{
    GFile *file = g_file_new_for_path (path);
    MooEditOpenInfo *info = moo_edit_open_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_edit_open_info_new_uri:
 *
 * Returns: (transfer full)
 **/
MooEditOpenInfo *
moo_edit_open_info_new_uri (const char *uri,
                            const char *encoding)
{
    GFile *file = g_file_new_for_uri (uri);
    MooEditOpenInfo *info = moo_edit_open_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_edit_open_info_dup:
 *
 * Returns: (transfer full)
 **/
MooEditOpenInfo *
moo_edit_open_info_dup (MooEditOpenInfo *info)
{
    MooEditOpenInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_edit_open_info_new (info->file, info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    copy->flags = info->flags;
    copy->line = info->line;

    return copy;
}

static void
moo_edit_open_info_finalize (GObject *object)
{
    MooEditOpenInfo *info = (MooEditOpenInfo*) object;

    g_object_unref (info->file);
    g_free (info->encoding);

    G_OBJECT_CLASS (moo_edit_open_info_parent_class)->finalize (object);
}

static void
moo_edit_open_info_class_init (MooEditOpenInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_edit_open_info_finalize;
}

static void
moo_edit_open_info_init (MooEditOpenInfo *info)
{
    info->line = -1;
}


/**
 * moo_edit_save_info_new: (constructor-of MooEditSaveInfo)
 **/
MooEditSaveInfo *
moo_edit_save_info_new (GFile      *file,
                        const char *encoding)
{
    MooEditSaveInfo *info;

    g_return_val_if_fail (G_IS_FILE (file), NULL);

    info = g_object_new (MOO_TYPE_EDIT_SAVE_INFO, NULL);

    info->file = g_file_dup (file);
    info->encoding = g_strdup (encoding);

    return info;
}

/**
 * moo_edit_save_info_new_path:
 *
 * Returns: (transfer full)
 **/
MooEditSaveInfo *
moo_edit_save_info_new_path (const char *path,
                             const char *encoding)
{
    GFile *file = g_file_new_for_path (path);
    MooEditSaveInfo *info = moo_edit_save_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_edit_save_info_new_uri:
 *
 * Returns: (transfer full)
 **/
MooEditSaveInfo *
moo_edit_save_info_new_uri (const char *uri,
                            const char *encoding)
{
    GFile *file = g_file_new_for_uri (uri);
    MooEditSaveInfo *info = moo_edit_save_info_new (file, encoding);
    g_object_unref (file);
    return info;
}

/**
 * moo_edit_save_info_dup:
 *
 * Returns: (transfer full)
 **/
MooEditSaveInfo *
moo_edit_save_info_dup (MooEditSaveInfo *info)
{
    MooEditSaveInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_edit_save_info_new (info->file, info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    return copy;
}

static void
moo_edit_save_info_finalize (GObject *object)
{
    MooEditSaveInfo *info = (MooEditSaveInfo*) object;

    g_object_unref (info->file);
    g_free (info->encoding);

    G_OBJECT_CLASS (moo_edit_save_info_parent_class)->finalize (object);
}

static void
moo_edit_save_info_class_init (MooEditSaveInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_edit_save_info_finalize;
}

static void
moo_edit_save_info_init (G_GNUC_UNUSED MooEditSaveInfo *info)
{
}


/**
 * moo_edit_reload_info_new: (constructor-of MooEditReloadInfo)
 **/
MooEditReloadInfo *
moo_edit_reload_info_new (const char *encoding)
{
    MooEditReloadInfo *info;

    info = g_object_new (MOO_TYPE_EDIT_RELOAD_INFO, NULL);

    info->encoding = g_strdup (encoding);
    info->line = -1;

    return info;
}

/**
 * moo_edit_reload_info_dup:
 *
 * Returns: (transfer full)
 **/
MooEditReloadInfo *
moo_edit_reload_info_dup (MooEditReloadInfo *info)
{
    MooEditReloadInfo *copy;

    g_return_val_if_fail (info != NULL, NULL);

    copy = moo_edit_reload_info_new (info->encoding);
    g_return_val_if_fail (copy != NULL, NULL);

    copy->line = info->line;

    return copy;
}

static void
moo_edit_reload_info_finalize (GObject *object)
{
    MooEditReloadInfo *info = (MooEditReloadInfo*) object;

    g_free (info->encoding);

    G_OBJECT_CLASS (moo_edit_reload_info_parent_class)->finalize (object);
}
static void
moo_edit_reload_info_class_init (MooEditReloadInfoClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize = moo_edit_reload_info_finalize;
}

static void
moo_edit_reload_info_init (MooEditReloadInfo *info)
{
    info->line = -1;
}
