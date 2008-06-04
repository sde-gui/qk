/*
 *   mdhistorymgr.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mdhistorymgr.h"
#include "moofileview/moofile.h"
#include "mooutils/mooapp-ipc.h"
#include "mooutils/moofilewatch.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-fs.h"
#include "mooutils/mooutils-treeview.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooprefs.h"
#include "marshals.h"
#include <stdarg.h>

#define N_MENU_ITEMS 10

#define IPC_ID "MdHistoryMgr"

struct MdHistoryMgrPrivate {
    char *filename;
    char *basename;
    char *name;
    char *ipc_id;

    guint save_idle;

    guint update_widgets_idle;
    GSList *widgets;

    GQueue *files;
    GHashTable *hash;
};

typedef struct {
    MdHistoryCallback callback;
    gpointer data;
    GDestroyNotify notify;
} CallbackData;

struct MdHistoryItem {
    char *uri;
    GData *data;
};

typedef enum {
    UPDATE_ITEM_UPDATE,
    UPDATE_ITEM_REMOVE,
    UPDATE_ITEM_ADD
} UpdateType;

static GObject     *md_history_mgr_constructor  (GType           type,
                                                 guint           n_props,
                                                 GObjectConstructParam *props);
static void         md_history_mgr_dispose      (GObject        *object);
static void         md_history_mgr_set_property (GObject        *object,
                                                 guint           property_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void         md_history_mgr_get_property (GObject        *object,
                                                 guint           property_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static const char  *get_filename                (MdHistoryMgr   *mgr);
static const char  *get_basename                (MdHistoryMgr   *mgr);

static void         ensure_files                (MdHistoryMgr   *mgr);
static void         schedule_save               (MdHistoryMgr   *mgr);
static void         md_history_mgr_save         (MdHistoryMgr   *mgr);

static void         populate_menu               (MdHistoryMgr   *mgr,
                                                 GtkWidget      *menu);
static void         schedule_update_widgets     (MdHistoryMgr   *mgr);

static void         md_history_item_format      (MdHistoryItem  *item,
                                                 GString        *buffer);
static gboolean     md_history_item_equal       (MdHistoryItem  *item1,
                                                 MdHistoryItem  *item2);
static char        *uri_get_basename            (const char     *uri);
static char        *uri_get_display_name        (const char     *uri);

static void         ipc_callback                (GObject        *obj,
                                                 const char     *data,
                                                 gsize           len);
static void         ipc_notify_add_file         (MdHistoryMgr   *mgr,
                                                 MdHistoryItem  *item);
static void         ipc_notify_update_file      (MdHistoryMgr   *mgr,
                                                 MdHistoryItem  *item);
static void         ipc_notify_remove_file      (MdHistoryMgr   *mgr,
                                                 const char     *uri);

G_DEFINE_TYPE (MdHistoryMgr, md_history_mgr, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_NAME,
    PROP_EMPTY
};

enum {
    CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

static void
md_history_mgr_class_init (MdHistoryMgrClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (MdHistoryMgrPrivate));

    object_class->constructor = md_history_mgr_constructor;
    object_class->set_property = md_history_mgr_set_property;
    object_class->get_property = md_history_mgr_get_property;
    object_class->dispose = md_history_mgr_dispose;

    g_object_class_install_property (object_class, PROP_NAME,
        g_param_spec_string ("name", "name", "name",
                             NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (object_class, PROP_EMPTY,
        g_param_spec_boolean ("empty", "empty", "empty",
                              TRUE, G_PARAM_READABLE));

    signals[CHANGED] =
        _moo_signal_new_cb ("changed",
                            MD_TYPE_HISTORY_MGR,
                            G_SIGNAL_RUN_LAST,
                            G_CALLBACK (schedule_update_widgets),
                            NULL, NULL,
                            _moo_marshal_VOID__VOID,
                            G_TYPE_NONE, 0);
}

static void
md_history_mgr_init (MdHistoryMgr *mgr)
{
    mgr->priv = G_TYPE_INSTANCE_GET_PRIVATE (mgr, MD_TYPE_HISTORY_MGR, MdHistoryMgrPrivate);
    mgr->priv->filename = NULL;
    mgr->priv->basename = NULL;
}

static GObject *
md_history_mgr_constructor (GType           type,
                            guint           n_props,
                            GObjectConstructParam *props)
{
    GObject *object;
    MdHistoryMgr *mgr;

    object = G_OBJECT_CLASS (md_history_mgr_parent_class)->
                constructor (type, n_props, props);
    mgr = MD_HISTORY_MGR (object);

    if (mgr->priv->name)
    {
        mgr->priv->ipc_id = g_strdup_printf (IPC_ID "/%s", mgr->priv->name);
        moo_ipc_register_client (G_OBJECT (mgr), mgr->priv->ipc_id, ipc_callback);
    }

    return object;
}

static void
md_history_mgr_dispose (GObject *object)
{
    MdHistoryMgr *mgr = MD_HISTORY_MGR (object);

    if (mgr->priv)
    {
        md_history_mgr_shutdown (mgr);

        if (mgr->priv->files)
        {
            g_queue_foreach (mgr->priv->files, (GFunc) md_history_item_free, NULL);
            g_queue_free (mgr->priv->files);
            g_hash_table_destroy (mgr->priv->hash);
        }

        g_free (mgr->priv->name);
        g_free (mgr->priv->filename);
        g_free (mgr->priv->basename);

        mgr->priv = NULL;
    }

    G_OBJECT_CLASS (md_history_mgr_parent_class)->dispose (object);
}

void
md_history_mgr_shutdown (MdHistoryMgr *mgr)
{
    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));

    if (!mgr->priv)
        return;

    if (mgr->priv->ipc_id)
    {
        moo_ipc_unregister_client (G_OBJECT (mgr), mgr->priv->ipc_id);
        g_free (mgr->priv->ipc_id);
        mgr->priv->ipc_id = NULL;
    }

    if (mgr->priv->save_idle)
    {
        g_source_remove (mgr->priv->save_idle);
        mgr->priv->save_idle = 0;
        md_history_mgr_save (mgr);
    }

    if (mgr->priv->update_widgets_idle)
    {
        g_source_remove (mgr->priv->update_widgets_idle);
        mgr->priv->update_widgets_idle = 0;
    }

    while (mgr->priv->widgets)
        gtk_widget_destroy (mgr->priv->widgets->data);
}

static void
md_history_mgr_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    MdHistoryMgr *mgr = MD_HISTORY_MGR (object);

    switch (prop_id)
    {
        case PROP_NAME:
            MOO_ASSIGN_STRING (mgr->priv->name, g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
md_history_mgr_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    MdHistoryMgr *mgr = MD_HISTORY_MGR (object);

    switch (prop_id)
    {
        case PROP_NAME:
            g_value_set_string (value, mgr->priv->name);
            break;

        case PROP_EMPTY:
            g_value_set_boolean (value, md_history_mgr_get_n_items (mgr) == 0);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static const char *
get_basename (MdHistoryMgr *mgr)
{
    if (!mgr->priv->basename)
    {
        if (mgr->priv->name)
        {
            char *name = g_ascii_strdown (mgr->priv->name, -1);
            mgr->priv->basename = g_strdup_printf ("recent-files-%s.xml", name);
            g_free (name);
        }
        else
        {
            mgr->priv->basename = g_strdup ("recent-files.xml");
        }
    }

    return mgr->priv->basename;
}

static const char *
get_filename (MdHistoryMgr *mgr)
{
    if (!mgr->priv->filename)
        mgr->priv->filename = moo_get_user_cache_file (get_basename (mgr));
    return mgr->priv->filename;
}


/*****************************************************************/
/* Loading and saving
 */

#define ELM_ROOT "md-recent-files"
#define ELM_UPDATE "md-recent-files-update"
#define ELM_ITEM "item"
#define ELM_DATA "data"
#define PROP_VERSION "version"
#define PROP_VERSION_VALUE "1.0"
#define PROP_URI "uri"
#define PROP_KEY "key"
#define PROP_VALUE "value"
#define PROP_TYPE "type"

#define ELEMENT_RECENT_ITEMS "recent-items"
#define ELEMENT_ITEM         "item"

static void
add_file (MdHistoryMgr  *mgr,
          MdHistoryItem *item)
{
    const char *uri;

    uri = md_history_item_get_uri (item);

    if (g_hash_table_lookup (mgr->priv->hash, uri) != NULL)
    {
        g_critical ("%s: duplicated uri '%s'", G_STRLOC, uri);
        md_history_item_free (item);
        return;
    }

    g_queue_push_tail (mgr->priv->files, item);
    g_hash_table_insert (mgr->priv->hash, g_strdup (uri),
                         mgr->priv->files->tail);
}

static gboolean
parse_element (const char     *filename,
               MooMarkupNode  *elm,
               MdHistoryItem **item_p)
{
    const char *uri;
    MdHistoryItem *item;
    MooMarkupNode *child;

    if (strcmp (elm->name, ELM_ITEM) != 0)
    {
        g_critical ("%s: in file '%s': invalid element '%s'",
                    G_STRLOC, filename ? filename : "NONE", elm->name);
        return FALSE;
    }

    if (!(uri = moo_markup_get_prop (elm, PROP_URI)))
    {
        g_critical ("%s: in file '%s': attribute '%s' missing",
                    G_STRLOC, filename ? filename : "NONE", PROP_URI);
        return FALSE;
    }

    item = md_history_item_new (uri, NULL);
    g_return_val_if_fail (item != NULL, FALSE);

    for (child = elm->children; child != NULL; child = child->next)
    {
        const char *key, *value;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        if (strcmp (child->name, ELM_DATA) != 0)
        {
            g_critical ("%s: in file '%s': invalid element '%s'",
                        G_STRLOC, filename ? filename : "NONE", child->name);
            continue;
        }

        key = moo_markup_get_prop (child, PROP_KEY);
        value = moo_markup_get_prop (child, PROP_VALUE);

        if (!key || !key[0])
        {
            g_critical ("%s: in file '%s': attribute '%s' missing",
                        G_STRLOC, filename ? filename : "NONE", PROP_KEY);
            continue;
        }
        else if (!value || !value[0])
        {
            g_critical ("%s: in file '%s': attribute '%s' missing",
                        G_STRLOC, filename ? filename : "NONE", PROP_VALUE);
            continue;
        }

        md_history_item_set (item, key, value);
    }

    *item_p = item;
    return TRUE;
}

static void
load_legacy (MdHistoryMgr *mgr)
{
    MooMarkupDoc *xml;
    MooMarkupNode *root, *node;
    char *root_path;

    if (!mgr->priv->name)
        return;

    xml = moo_prefs_get_markup (MOO_PREFS_STATE);
    g_return_if_fail (xml != NULL);

    root_path = g_strdup_printf ("%s/" ELEMENT_RECENT_ITEMS, mgr->priv->name);
    root = moo_markup_get_element (MOO_MARKUP_NODE (xml), root_path);
    g_free (root_path);

    if (!root)
        return;

    for (node = root->children; node != NULL; node = node->next)
    {
        char *uri = NULL;
        MdHistoryItem *item = NULL;

        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        if (!strcmp (node->name, ELEMENT_ITEM))
        {
            const char *filename = moo_markup_get_content (node);
            uri = g_filename_to_uri (filename, NULL, NULL);
        }

        if (uri)
            item = md_history_item_new (uri, NULL);

        if (item)
            add_file (mgr, item);

        g_free (uri);
    }

    moo_markup_delete_node (root);
}

static void
load_file (MdHistoryMgr *mgr)
{
    /* XXX: use GMarkupParser */
    MooMarkupDoc *doc;
    MooMarkupNode *root, *child;
    const char *filename;
    const char *version;
    GError *error = NULL;

    mgr->priv->files = g_queue_new ();
    mgr->priv->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, NULL);

    filename = get_filename (mgr);
    g_return_if_fail (filename != NULL);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        load_legacy (mgr);
        return;
    }

    doc = moo_markup_parse_file (filename, &error);

    if (!doc)
    {
        g_critical ("%s: could not open file '%s': %s",
                    G_STRLOC, filename, error ? error->message : "");
        g_error_free (error);
        return;
    }

    if (!(root = moo_markup_get_root_element (doc, ELM_ROOT)))
    {
        g_critical ("%s: in file '%s': missing element %s",
                    G_STRLOC, filename, ELM_ROOT);
        moo_markup_doc_unref (doc);
        return;
    }

    if (!(version = moo_markup_get_prop (root, PROP_VERSION)) ||
        strcmp (version, PROP_VERSION_VALUE) != 0)
    {
        g_critical ("%s: in file '%s': invalid version value '%s'",
                    G_STRLOC, filename, version ? version : "(null)");
        moo_markup_doc_unref (doc);
        return;
    }

    for (child = root->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
        {
            MdHistoryItem *item;
            if (parse_element (filename, child, &item))
                add_file (mgr, item);
        }
    }

    moo_markup_doc_unref (doc);
}

static gboolean
parse_update_item (MooMarkupDoc   *xml,
                   MdHistoryItem **item,
                   UpdateType     *type)
{
    const char *version;
    const char *update_type_string;
    MooMarkupNode *root, *child;

    if (!(root = moo_markup_get_root_element (xml, ELM_UPDATE)))
    {
        g_critical ("%s: missing element %s",
                    G_STRLOC, ELM_UPDATE);
        return FALSE;
    }

    if (!(version = moo_markup_get_prop (root, PROP_VERSION)) ||
        strcmp (version, PROP_VERSION_VALUE) != 0)
    {
        g_critical ("%s: invalid version value '%s'",
                    G_STRLOC, version ? version : "(null)");
        return FALSE;
    }

    if (!(update_type_string = moo_markup_get_prop (root, PROP_TYPE)))
    {
        g_critical ("%s: attribute '%s' missing", G_STRLOC, PROP_TYPE);
        return FALSE;
    }

    if (strcmp (update_type_string, "add") == 0)
        *type = UPDATE_ITEM_ADD;
    else if (strcmp (update_type_string, "remove") == 0)
        *type = UPDATE_ITEM_REMOVE;
    else if (strcmp (update_type_string, "update") == 0)
        *type = UPDATE_ITEM_UPDATE;
    else
    {
        g_critical ("%s: invalid value '%s' for attribute '%s'",
                    G_STRLOC, update_type_string, PROP_TYPE);
        return FALSE;
    }

    for (child = root->children; child != NULL; child = child->next)
        if (MOO_MARKUP_IS_ELEMENT (child))
            return parse_element (NULL, child, item);

    g_critical ("%s: element '%s' missing", G_STRLOC, ELM_ITEM);
    return FALSE;
}

static void
ensure_files (MdHistoryMgr *mgr)
{
    if (!mgr->priv->files)
        load_file (mgr);
}


static gboolean
save_in_idle (MdHistoryMgr *mgr)
{
    mgr->priv->save_idle = 0;
    md_history_mgr_save (mgr);
    return FALSE;
}

static void
schedule_save (MdHistoryMgr *mgr)
{
    if (!mgr->priv->save_idle)
        mgr->priv->save_idle = g_idle_add ((GSourceFunc) save_in_idle, mgr);
}

static void
md_history_mgr_save (MdHistoryMgr *mgr)
{
    const char *filename;
    GError *error = NULL;
    MooFileWriter *writer;

    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));

    if (!mgr->priv->files)
        return;

    filename = get_filename (mgr);

    if (!mgr->priv->files->length)
    {
        _moo_unlink (filename);
        return;
    }

    if ((writer = moo_text_writer_new (filename, FALSE, &error)))
    {
        GString *string;
        GList *l;

        moo_file_writer_write (writer, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", -1);
        moo_file_writer_write (writer, "<" ELM_ROOT " " PROP_VERSION "=\"" PROP_VERSION_VALUE "\">\n", -1);

        string = g_string_new (NULL);

        for (l = mgr->priv->files->head; l != NULL; l = l->next)
        {
            MdHistoryItem *item = l->data;
            g_string_truncate (string, 0);
            md_history_item_format (item, string);
            if (!moo_file_writer_write (writer, string->str, -1))
                break;
        }

        g_string_free (string, TRUE);

        moo_file_writer_write (writer, "</" ELM_ROOT ">\n", -1);
        moo_file_writer_close (writer, &error);
    }

    if (error)
    {
        g_critical ("%s: could not save file '%s': %s",
                    G_STRLOC, filename,
                    error ? error->message : "");
        g_error_free (error);
    }
}


static char *
format_for_update (MdHistoryItem *item,
                   UpdateType     type)
{
    GString *buffer;
    const char *update_types[3] = {"update", "remove", "add"};

    g_return_val_if_fail (type < 3, NULL);

    buffer = g_string_new (NULL);
    g_string_append_printf (buffer, "<%s %s=\"%s\" %s=\"%s\">\n",
                            ELM_UPDATE, PROP_VERSION, PROP_VERSION_VALUE,
                            PROP_TYPE, update_types[type]);

    md_history_item_format (item, buffer);

    g_string_append (buffer, "</" ELM_UPDATE ">\n");

    return g_string_free (buffer, FALSE);
}


guint
md_history_mgr_get_n_items (MdHistoryMgr *mgr)
{
    g_return_val_if_fail (MD_IS_HISTORY_MGR (mgr), 0);
    ensure_files (mgr);
    return mgr->priv->files->length;
}


void
md_history_mgr_add_uri (MdHistoryMgr *mgr,
                        const char   *uri)
{
    MdHistoryItem *freeme = NULL;
    MdHistoryItem *item;

    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));
    g_return_if_fail (uri && uri[0]);

    if (!(item = md_history_mgr_find_uri (mgr, uri)))
    {
        freeme = md_history_item_new (uri, NULL);
        item = freeme;
    }

    md_history_mgr_add_file (mgr, item);

    md_history_item_free (freeme);
}

static void
md_history_mgr_add_file_real (MdHistoryMgr  *mgr,
                              MdHistoryItem *item,
                              gboolean       notify)
{
    const char *uri;
    GList *link;
    MdHistoryItem *new_item = NULL;

    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));
    g_return_if_fail (item != NULL);

    uri = md_history_item_get_uri (item);
    link = g_hash_table_lookup (mgr->priv->hash, uri);

    if (!link)
    {
        MdHistoryItem *copy = md_history_item_copy (item);
        g_queue_push_head (mgr->priv->files, copy);
        new_item = copy;
        g_hash_table_insert (mgr->priv->hash, g_strdup (uri),
                             mgr->priv->files->head);
    }
    else if (link != mgr->priv->files->head ||
            !md_history_item_equal (item, link->data))
    {
        MdHistoryItem *tmp = link->data;

        g_queue_unlink (mgr->priv->files, link);
        g_queue_push_head_link (mgr->priv->files, link);

        new_item = link->data = md_history_item_copy (item);

        md_history_item_free (tmp);
    }

    if (new_item)
    {
        g_signal_emit (mgr, signals[CHANGED], 0);

        if (notify)
        {
            schedule_save (mgr);
            ipc_notify_add_file (mgr, new_item);
        }

        if (mgr->priv->files->length == 1)
            g_object_notify (G_OBJECT (mgr), "empty");
    }
}

void
md_history_mgr_add_file (MdHistoryMgr  *mgr,
                         MdHistoryItem *item)
{
    md_history_mgr_add_file_real (mgr, item, TRUE);
}

static void
md_history_mgr_update_file_real (MdHistoryMgr  *mgr,
                                 MdHistoryItem *file,
                                 gboolean       notify)
{
    const char *uri;
    GList *link;

    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));
    g_return_if_fail (file != NULL);

    uri = md_history_item_get_uri (file);
    link = g_hash_table_lookup (mgr->priv->hash, uri);

    if (!link)
    {
        md_history_mgr_add_file (mgr, file);
    }
    else if (!md_history_item_equal (link->data, file))
    {
        MdHistoryItem *tmp = link->data;
        link->data = md_history_item_copy (file);
        md_history_item_free (tmp);
        g_signal_emit (mgr, signals[CHANGED], 0);

        if (notify)
        {
            schedule_save (mgr);
            ipc_notify_update_file (mgr, link->data);
        }
    }
}

void
md_history_mgr_update_file (MdHistoryMgr  *mgr,
                            MdHistoryItem *file)
{
    md_history_mgr_update_file_real (mgr, file, TRUE);
}

MdHistoryItem *
md_history_mgr_find_uri (MdHistoryMgr *mgr,
                         const char   *uri)
{
    GList *link;

    g_return_val_if_fail (MD_IS_HISTORY_MGR (mgr), NULL);
    g_return_val_if_fail (uri != NULL, NULL);

    link = g_hash_table_lookup (mgr->priv->hash, uri);

    return link ? link->data : NULL;
}

static void
md_history_mgr_remove_uri_real (MdHistoryMgr *mgr,
                                const char   *uri,
                                gboolean      notify)
{
    GList *link;

    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));
    g_return_if_fail (uri != NULL);

    link = g_hash_table_lookup (mgr->priv->hash, uri);

    if (!link)
        return;

    g_hash_table_remove (mgr->priv->hash, uri);
    md_history_item_free (link->data);
    g_queue_delete_link (mgr->priv->files, link);

    g_signal_emit (mgr, signals[CHANGED], 0);

    if (notify)
    {
        schedule_save (mgr);
        ipc_notify_remove_file (mgr, uri);
    }

    if (mgr->priv->files->length == 0)
        g_object_notify (G_OBJECT (mgr), "empty");
}

void
md_history_mgr_remove_uri (MdHistoryMgr *mgr,
                           const char   *uri)
{
    md_history_mgr_remove_uri_real (mgr, uri, TRUE);
}


static void
ipc_callback (GObject    *obj,
              const char *data,
              gsize       len)
{
    MdHistoryMgr *mgr;
    MooMarkupDoc *xml;
    GError *error = NULL;
    MdHistoryItem *item;
    UpdateType type;

    g_return_if_fail (MD_IS_HISTORY_MGR (obj));

    mgr = MD_HISTORY_MGR (obj);
    ensure_files (mgr);

    xml = moo_markup_parse_memory (data, len, &error);

    if (!xml)
    {
        g_critical ("%s: got invalid data: %.*s", G_STRLOC, (int) len, data);
        return;
    }

    g_print ("%s: got data: %.*s\n", G_STRLOC, (int) len, data);

    if (parse_update_item (xml, &item, &type))
    {
        switch (type)
        {
            case UPDATE_ITEM_UPDATE:
                md_history_mgr_update_file_real (mgr, item, FALSE);
                break;
            case UPDATE_ITEM_ADD:
                md_history_mgr_add_file_real (mgr, item, FALSE);
                break;
            case UPDATE_ITEM_REMOVE:
                md_history_mgr_remove_uri_real (mgr, md_history_item_get_uri (item), FALSE);
                break;
        }

        md_history_item_free (item);
    }

    moo_markup_doc_unref (xml);
}

static void
ipc_notify (MdHistoryMgr  *mgr,
            MdHistoryItem *item,
            UpdateType     type)
{
    if (mgr->priv->ipc_id)
    {
        char *string = format_for_update (item, type);
        moo_ipc_send (G_OBJECT (mgr), mgr->priv->ipc_id, string, -1);
        g_free (string);
    }
}

static void
ipc_notify_add_file (MdHistoryMgr  *mgr,
                     MdHistoryItem *item)
{
    ipc_notify (mgr, item, UPDATE_ITEM_ADD);
}

static void
ipc_notify_update_file (MdHistoryMgr  *mgr,
                        MdHistoryItem *item)
{
    ipc_notify (mgr, item, UPDATE_ITEM_UPDATE);
}

static void
ipc_notify_remove_file (MdHistoryMgr *mgr,
                        const char   *uri)
{
    MdHistoryItem *item = md_history_item_new (uri, NULL);
    ipc_notify (mgr, item, UPDATE_ITEM_REMOVE);
    md_history_item_free (item);
}


/*****************************************************************/
/* Menu
 */

static void
callback_data_free (CallbackData *data)
{
    if (data)
    {
        if (data->notify)
            data->notify (data->data);
        moo_free (CallbackData, data);
    }
}

static void
view_destroyed (GtkWidget    *widget,
                MdHistoryMgr *mgr)
{
    g_object_set_data (G_OBJECT (widget), "md-history-mgr-callback-data", NULL);
    mgr->priv->widgets = g_slist_remove (mgr->priv->widgets, widget);
}

static void
update_menu (MdHistoryMgr *mgr,
             GtkWidget    *menu)
{
    GList *children;

    children = gtk_container_get_children (GTK_CONTAINER (menu));

    while (children)
    {
        GtkWidget *item = children->data;

        if (g_object_get_data (G_OBJECT (item), "md-history-menu-item-file"))
            gtk_widget_destroy (item);

        children = g_list_delete_link (children, children);
    }

    populate_menu (mgr, menu);
}

GtkWidget *
md_history_mgr_create_menu (MdHistoryMgr   *mgr,
                            MdHistoryCallback callback,
                            gpointer        data,
                            GDestroyNotify  notify)
{
    GtkWidget *menu;
    CallbackData *cb_data;

    g_return_val_if_fail (MD_IS_HISTORY_MGR (mgr), NULL);
    g_return_val_if_fail (callback != NULL, NULL);

    menu = gtk_menu_new ();
    gtk_widget_show (menu);
    g_signal_connect (menu, "destroy", G_CALLBACK (view_destroyed), mgr);

    cb_data = moo_new0 (CallbackData);
    cb_data->callback = callback;
    cb_data->data = data;
    cb_data->notify = notify;
    g_object_set_data_full (G_OBJECT (menu), "md-history-mgr-callback-data",
                            cb_data, (GDestroyNotify) callback_data_free);

    populate_menu (mgr, menu);
    mgr->priv->widgets = g_slist_prepend (mgr->priv->widgets, menu);

    return menu;
}

static void
menu_item_activated (GtkWidget *menu_item)
{
    GtkWidget *parent = menu_item->parent;
    CallbackData *data;
    MdHistoryItem *item;
    GSList *list;

    g_return_if_fail (parent != NULL);

    data = g_object_get_data (G_OBJECT (parent), "md-history-mgr-callback-data");
    item = g_object_get_data (G_OBJECT (menu_item), "md-history-menu-item-file");
    g_return_if_fail (data && item);

    list = g_slist_prepend (NULL, md_history_item_copy (item));
    data->callback (list, data->data);
    md_history_item_free (list->data);
    g_slist_free (list);
}

static void
populate_menu (MdHistoryMgr *mgr,
               GtkWidget    *menu)
{
    guint n_items, i;
    GList *l;

    ensure_files (mgr);

    n_items = MIN (mgr->priv->files->length, N_MENU_ITEMS);

    for (i = 0, l = mgr->priv->files->head; i < n_items; i++, l = l->next)
    {
        GtkWidget *item, *image;
        MdHistoryItem *hist_item = l->data;
        char *display_name, *display_basename;
        GdkPixbuf *pixbuf;

        display_basename = uri_get_basename (hist_item->uri);
        display_name = uri_get_display_name (hist_item->uri);

        item = gtk_image_menu_item_new_with_label (display_basename);
        _moo_widget_set_tooltip (item, display_name);
        gtk_widget_show (item);
        gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item, i);

        /* XXX */
        pixbuf = _moo_get_icon_for_path (display_name, GTK_WIDGET (item),
                                         GTK_ICON_SIZE_MENU);
        image = gtk_image_new_from_pixbuf (pixbuf);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

        g_object_set_data_full (G_OBJECT (item), "md-history-menu-item-file",
                                md_history_item_copy (hist_item),
                                (GDestroyNotify) md_history_item_free);
        g_signal_connect (item, "activate", G_CALLBACK (menu_item_activated), NULL);

        g_free (display_basename);
        g_free (display_name);
    }
}


enum {
    COLUMN_PIXBUF,
    COLUMN_NAME,
    COLUMN_TOOLTIP,
    COLUMN_URI,
    N_COLUMNS
};

static void
open_selected (GtkTreeView *tree_view)
{
    CallbackData *data;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    MdHistoryMgr *mgr;
    GList *selected;
    GSList *items;

    mgr = g_object_get_data (G_OBJECT (tree_view), "md-history-mgr");
    g_return_if_fail (MD_IS_HISTORY_MGR (mgr));

    data = g_object_get_data (G_OBJECT (tree_view), "md-history-mgr-callback-data");
    g_return_if_fail (data != NULL);

    selection = gtk_tree_view_get_selection (tree_view);
    selected = gtk_tree_selection_get_selected_rows (selection, &model);

    for (items = NULL; selected != NULL; )
    {
        char *uri = NULL;
        MdHistoryItem *item;
        GtkTreePath *path = selected->data;

        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, COLUMN_URI, &uri, -1);
        item = md_history_mgr_find_uri (mgr, uri);

        if (item)
            items = g_slist_prepend (items, md_history_item_copy (item));

        g_free (uri);
        gtk_tree_path_free (path);
        selected = g_list_delete_link (selected, selected);
    }

    items = g_slist_reverse (items);

    if (items)
        data->callback (items, data->data);

    g_slist_foreach (items, (GFunc) md_history_item_free, NULL);
    g_slist_free (items);
}

static GtkWidget *
create_tree_view (void)
{
    GtkWidget *tree_view;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkListStore *store;

    tree_view = gtk_tree_view_new ();
    store = gtk_list_store_new (N_COLUMNS, GDK_TYPE_PIXBUF, G_TYPE_STRING,
                                G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);
#if GTK_CHECK_VERSION(2,12,0)
    gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (tree_view),
                                      COLUMN_TOOLTIP);
#endif
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)),
                                 GTK_SELECTION_MULTIPLE);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, cell, FALSE);
    gtk_tree_view_column_set_attributes (column, cell, "pixbuf", COLUMN_PIXBUF, NULL);

    cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, cell, TRUE);
    gtk_tree_view_column_set_attributes (column, cell, "text", COLUMN_NAME, NULL);

    return tree_view;
}

static void
populate_tree_view (MdHistoryMgr *mgr,
                    GtkWidget    *tree_view)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GList *l;

    ensure_files (mgr);

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    store = GTK_LIST_STORE (model);

    for (l = mgr->priv->files->head; l != NULL; l = l->next)
    {
        MdHistoryItem *item = l->data;
        char *display_name, *display_basename;
        GdkPixbuf *pixbuf;
        GtkTreeIter iter;

        display_basename = uri_get_basename (item->uri);
        display_name = uri_get_display_name (item->uri);

        /* XXX */
        pixbuf = _moo_get_icon_for_path (display_name, tree_view, GTK_ICON_SIZE_MENU);

        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter,
                            COLUMN_PIXBUF, pixbuf,
                            COLUMN_NAME, display_basename,
                            COLUMN_TOOLTIP, display_name,
                            COLUMN_URI, md_history_item_get_uri (item),
                            -1);

        g_free (display_basename);
        g_free (display_name);
    }
}

static void
update_tree_view (MdHistoryMgr *mgr,
                  GtkWidget    *tree_view)
{
    GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    gtk_list_store_clear (GTK_LIST_STORE (model));
    populate_tree_view (mgr, tree_view);
}

static GtkWidget *
md_history_mgr_create_tree_view (MdHistoryMgr   *mgr,
                                 MdHistoryCallback callback,
                                 gpointer        data,
                                 GDestroyNotify  notify)
{
    GtkWidget *tree_view;
    CallbackData *cb_data;

    g_return_val_if_fail (MD_IS_HISTORY_MGR (mgr), NULL);
    g_return_val_if_fail (callback != NULL, NULL);

    tree_view = create_tree_view ();
    gtk_widget_show (tree_view);
    g_signal_connect (tree_view, "destroy", G_CALLBACK (view_destroyed), mgr);

    cb_data = moo_new0 (CallbackData);
    cb_data->callback = callback;
    cb_data->data = data;
    cb_data->notify = notify;
    g_object_set_data_full (G_OBJECT (tree_view), "md-history-mgr-callback-data",
                            cb_data, (GDestroyNotify) callback_data_free);
    g_object_set_data (G_OBJECT (tree_view), "md-history-mgr", mgr);

    populate_tree_view (mgr, tree_view);
    if (mgr->priv->files->head)
        _moo_tree_view_select_first (GTK_TREE_VIEW (tree_view));
    mgr->priv->widgets = g_slist_prepend (mgr->priv->widgets, tree_view);

    return tree_view;
}

static void
dialog_response (GtkTreeView *tree_view,
                 int          response)
{
    if (response == GTK_RESPONSE_OK)
        open_selected (tree_view);
}

static void
row_activated (GtkDialog *dialog)
{
    gtk_dialog_response (dialog, GTK_RESPONSE_OK);
}

GtkWidget *
md_history_mgr_create_dialog (MdHistoryMgr   *mgr,
                              MdHistoryCallback callback,
                              gpointer        data,
                              GDestroyNotify  notify)
{
    GtkWidget *dialog, *swin, *tree_view;

    g_return_val_if_fail (MD_IS_HISTORY_MGR (mgr), NULL);
    g_return_val_if_fail (callback != NULL, NULL);

    dialog = gtk_dialog_new_with_buttons ("", NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                                          NULL);
    gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
                                             GTK_RESPONSE_OK,
                                             GTK_RESPONSE_CANCEL,
                                             -1);

    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    tree_view = md_history_mgr_create_tree_view (mgr, callback, data, notify);
    gtk_container_add (GTK_CONTAINER (swin), tree_view);
    gtk_widget_show_all (swin);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), swin, TRUE, TRUE, 0);

    g_signal_connect_swapped (tree_view, "row-activated",
                              G_CALLBACK (row_activated), dialog);
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (dialog_response), tree_view);

    return dialog;
}


static gboolean
do_update_widgets (MdHistoryMgr *mgr)
{
    GSList *l;

    mgr->priv->update_widgets_idle = 0;

    for (l = mgr->priv->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget = l->data;

        if (GTK_IS_MENU (widget))
            update_menu (mgr, widget);
        else if (GTK_IS_TREE_VIEW (widget))
            update_tree_view (mgr, widget);
        else
            g_critical ("%s: oops", G_STRFUNC);
    }

    return FALSE;
}

static void
schedule_update_widgets (MdHistoryMgr *mgr)
{
    if (!mgr->priv->update_widgets_idle && mgr->priv->widgets)
        mgr->priv->update_widgets_idle =
            g_idle_add ((GSourceFunc) do_update_widgets, mgr);
}


void
md_history_item_free (MdHistoryItem *item)
{
    if (item)
    {
        g_free (item->uri);
        g_datalist_clear (&item->data);
        moo_free (MdHistoryItem, item);
    }
}

static MdHistoryItem *
md_history_item_new_uri (const char *uri)
{
    MdHistoryItem *item = moo_new (MdHistoryItem);
    item->uri = g_strdup (uri);
    item->data = NULL;
    return item;
}

static MdHistoryItem *
md_history_item_newv (const char *uri,
                      const char *first_key,
                      va_list     args)
{
    const char *key;
    MdHistoryItem *item;

    item = md_history_item_new_uri (uri);

    for (key = first_key; key != NULL; )
    {
        const char *value = va_arg (args, const char *);
        md_history_item_set (item, key, value);
        key = va_arg (args, const char *);
    }

    return item;
}

MdHistoryItem *
md_history_item_new (const char *uri,
                     const char *first_key,
                     ...)
{
    va_list args;
    MdHistoryItem *item;

    g_return_val_if_fail (uri != NULL, NULL);

    va_start (args, first_key);
    item = md_history_item_newv (uri, first_key, args);
    va_end (args);

    return item;
}

static void
copy_data (GQuark         key,
           const char    *value,
           MdHistoryItem *dest)
{
    g_datalist_id_set_data_full (&dest->data, key, g_strdup (value), g_free);
}

MdHistoryItem *
md_history_item_copy (MdHistoryItem *item)
{
    MdHistoryItem *copy;

    if (!item)
        return NULL;

    copy = md_history_item_new_uri (item->uri);
    g_datalist_foreach (&item->data, (GDataForeachFunc) copy_data, copy);

    return copy;
}

typedef struct {
    MdHistoryItem *item;
    gboolean equal;
} CmpData;

static void
cmp_data (GQuark      key,
          const char *value,
          CmpData    *data)
{
    const char *value2;

    if (!data->equal)
        return;

    value2 = g_datalist_id_get_data (&data->item->data, key);
    if (!value2 || strcmp (value2, value) != 0)
        data->equal = FALSE;
}

static gboolean
md_history_item_equal (MdHistoryItem *item1,
                       MdHistoryItem *item2)
{
    CmpData data;

    g_return_val_if_fail (item1 && item2, FALSE);

    if (item1 == item2)
        return TRUE;

    if (strcmp (item1->uri, item2->uri) != 0)
        return FALSE;

    data.equal = TRUE;

    data.item = item1;
    g_datalist_foreach (&item2->data, (GDataForeachFunc) cmp_data, &data);

    if (data.equal)
    {
        data.item = item2;
        g_datalist_foreach (&item1->data, (GDataForeachFunc) cmp_data, &data);
    }

    return data.equal;
}

void
md_history_item_set (MdHistoryItem *item,
                     const char    *key,
                     const char    *value)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);

    if (value)
        g_datalist_set_data_full (&item->data, key, g_strdup (value), g_free);
    else
        g_datalist_remove_data (&item->data, key);
}

const char *
md_history_item_get (MdHistoryItem *item,
                     const char    *key)
{
    g_return_val_if_fail (item != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);
    return g_datalist_get_data (&item->data, key);
}

const char *
md_history_item_get_uri (MdHistoryItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->uri;
}

static void
format_data (GQuark      key_id,
             const char *value,
             GString    *dest)
{
    const char *key = g_quark_to_string (key_id);
    char *key_escaped = g_markup_escape_text (key, -1);
    char *value_escaped = g_markup_escape_text (value, -1);
    g_string_append_printf (dest, "    <data key=\"%s\" value=\"%s\"/>\n",
                            key_escaped, value_escaped);
    g_free (value_escaped);
    g_free (key_escaped);
}

static void
md_history_item_format (MdHistoryItem *item,
                        GString       *dest)
{
    char *uri_escaped;

    g_return_if_fail (item != NULL);
    g_return_if_fail (dest != NULL);

    uri_escaped = g_markup_escape_text (item->uri, -1);

    if (item->data)
    {
        g_string_append_printf (dest, "  <item uri=\"%s\">\n", uri_escaped);
        g_datalist_foreach (&item->data, (GDataForeachFunc) format_data, dest);
        g_string_append (dest, "  </item>\n");
    }
    else
    {
        g_string_append_printf (dest, "  <item uri=\"%s\"/>\n", uri_escaped);
    }

    g_free (uri_escaped);
}

void
md_history_item_foreach (MdHistoryItem    *item,
                         GDataForeachFunc  func,
                         gpointer          user_data)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (func != NULL);
    g_datalist_foreach (&item->data, func, user_data);
}


static char *
uri_get_basename (const char *uri)
{
    const char *last_slash;

    g_return_val_if_fail (uri != NULL, NULL);

    if (g_str_has_prefix (uri, "file://"))
    {
        char *filename = g_filename_from_uri (uri, NULL, NULL);

        if (filename)
        {
            char *display_name = g_filename_display_basename (filename);

            if (display_name)
            {
                g_free (filename);
                return display_name;
            }

            g_free (filename);
        }
    }

    /* XXX percent encoding */
    last_slash = strrchr (uri, '/');
    if (last_slash)
        return g_strdup (last_slash + 1);
    else
        return g_strdup (uri);
}

static char *
uri_get_display_name (const char *uri)
{
    g_return_val_if_fail (uri != NULL, NULL);

    if (g_str_has_prefix (uri, "file://"))
    {
        char *filename = g_filename_from_uri (uri, NULL, NULL);

        if (filename)
        {
            char *display_name = g_filename_display_name (filename);

            if (display_name)
            {
                g_free (filename);
                return display_name;
            }

            g_free (filename);
        }
    }

    return g_strdup (uri);
}
