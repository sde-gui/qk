/*
 *   mooconfig.c
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "mooutils/mooconfig.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/moocompat.h"
#include <string.h>


#ifdef __WIN32__
#define LINE_TERM "\r\n"
#else
#define LINE_TERM "\n"
#endif

struct _MooConfigPrivate {
    GPtrArray *items;
    char *id_key;
    int model_stamp;
    GHashTable *ids;
    gboolean modified;
};

typedef struct {
    GHashTable *dict;
    char *content;
    guint start;
    guint end;
} ItemData;

struct _MooConfigItem {
    GHashTable *dict;
    char *content;
    char *id;
    guint start;
    guint end;
};

#define NTH_ITEM(cfg,n) ((MooConfigItem*) (cfg)->priv->items->pdata[n])

#define ITER_INDEX(iter)            GPOINTER_TO_UINT((iter)->user_data)
#define ITER_SET(config,iter,ind)                   \
G_STMT_START {                                      \
    (iter)->user_data = GUINT_TO_POINTER(ind);      \
    (iter)->stamp = (config)->priv->model_stamp;    \
} G_STMT_END
#define ITER_SET_INVALID(iter) (iter)->stamp = 0

static GObjectClass *moo_config_parent_class;

enum {
    PROP_0,
    PROP_ITEM_ID_KEY
};

static ItemData        *item_data_new               (void);
static void             item_data_free              (ItemData       *data);

static MooConfigItem   *moo_config_item_new         (const char     *id,
                                                     GHashTable     *dict);
static void             moo_config_item_free        (MooConfigItem  *item);

static int              get_item_index              (MooConfig     *config,
                                                     MooConfigItem *item);

static GtkTreeModelFlags moo_config_get_flags       (GtkTreeModel   *model);
static int              moo_config_get_n_columns    (GtkTreeModel   *model);
static GType            moo_config_get_column_type  (GtkTreeModel   *model,
                                                     int             index);
static gboolean         moo_config_get_iter         (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter,
                                                     GtkTreePath    *path);
static GtkTreePath     *moo_config_get_path         (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter);
static void             moo_config_get_value        (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter,
                                                     int             column,
                                                     GValue         *value);
static gboolean         moo_config_iter_next        (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter);
static gboolean         moo_config_iter_children    (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter,
                                                     GtkTreeIter    *parent);
static gboolean         moo_config_iter_has_child   (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter);
static gint             moo_config_iter_n_children  (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter);
static gboolean         moo_config_iter_nth_child   (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter,
                                                     GtkTreeIter    *parent,
                                                     int             n);
static gboolean         moo_config_iter_parent      (GtkTreeModel   *model,
                                                     GtkTreeIter    *iter,
                                                     GtkTreeIter    *child);


static void
moo_config_init (MooConfig *config)
{
    config->priv = g_new0 (MooConfigPrivate, 1);
    config->priv->items = g_ptr_array_new ();
    config->priv->model_stamp = 1;
    config->priv->ids = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}


static void
moo_config_finalize (GObject *object)
{
    MooConfig *config = MOO_CONFIG (object);

    g_ptr_array_foreach (config->priv->items,
                         (GFunc) moo_config_item_free,
                         NULL);
    g_ptr_array_free (config->priv->items, TRUE);
    g_free (config->priv->id_key);

    g_hash_table_destroy (config->priv->ids);

    g_free (config->priv);

    moo_config_parent_class->finalize (object);
}


static void
moo_config_set_property (GObject        *object,
                         guint           param_id,
                         const GValue   *value,
                         GParamSpec     *pspec)
{
    MooConfig *config = MOO_CONFIG (object);

    switch (param_id)
    {
        case PROP_ITEM_ID_KEY:
            g_free (config->priv->id_key);
            config->priv->id_key = g_strdup (g_value_get_string (value));
            g_object_notify (object, "item-id-key");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void
moo_config_get_property (GObject        *object,
                         guint           param_id,
                         GValue         *value,
                         GParamSpec     *pspec)
{
    MooConfig *config = MOO_CONFIG (object);

    switch (param_id)
    {
        case PROP_ITEM_ID_KEY:
            g_value_set_string (value, config->priv->id_key);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
            break;
    }
}


static void
moo_config_class_init (MooConfigClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    moo_config_parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = moo_config_finalize;
    gobject_class->set_property = moo_config_set_property;
    gobject_class->get_property = moo_config_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_ITEM_ID_KEY,
                                     g_param_spec_string ("item-id-key",
                                             "item-id-key",
                                             "item-id-key",
                                             NULL,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


static void
moo_config_tree_model_init (GtkTreeModelIface *iface)
{
    iface->get_flags = moo_config_get_flags;
    iface->get_n_columns = moo_config_get_n_columns;
    iface->get_column_type = moo_config_get_column_type;
    iface->get_iter = moo_config_get_iter;
    iface->get_path = moo_config_get_path;
    iface->get_value = moo_config_get_value;
    iface->iter_next = moo_config_iter_next;
    iface->iter_children = moo_config_iter_children;
    iface->iter_has_child = moo_config_iter_has_child;
    iface->iter_n_children = moo_config_iter_n_children;
    iface->iter_nth_child = moo_config_iter_nth_child;
    iface->iter_parent = moo_config_iter_parent;
}


GType
moo_config_get_type (void)
{
    static GType type;

    if (!type)
    {
        static GTypeInfo info = {
            sizeof (MooConfigClass),
            NULL,           /* base_init */
            NULL,           /* base_finalize */
            (GClassInitFunc) moo_config_class_init,
            NULL,           /* class_finalize */
            NULL,           /* class_data */
            sizeof (MooConfig),
            0,
            (GInstanceInitFunc) moo_config_init,
            NULL
        };

        static const GInterfaceInfo tree_model_info = {
            (GInterfaceInitFunc) moo_config_tree_model_init,
            NULL,
            NULL
        };

        type = g_type_register_static (G_TYPE_OBJECT, "MooConfig", &info, 0);

        g_type_add_interface_static (type,
                                     GTK_TYPE_TREE_MODEL,
                                     &tree_model_info);
    }

    return type;
}


GType
moo_config_item_get_type (void)
{
    static GType type;

    if (!type)
        type = g_pointer_type_register_static ("MooConfigItem");

    return type;
}


static MooConfigItem *
moo_config_item_new (const char  *id,
                     GHashTable  *dict)
{
    MooConfigItem *item = g_new0 (MooConfigItem, 1);
    item->id = g_strdup (id);
    item->dict = dict ? dict : g_hash_table_new_full (g_str_hash, g_str_equal,
                                                      g_free, g_free);
    return item;
}


static void
moo_config_item_free (MooConfigItem *item)
{
    if (item)
    {
        g_hash_table_destroy (item->dict);
        g_free (item->id);
        g_free (item->content);
        g_free (item);
    }
}


static ItemData *
item_data_new (void)
{
    ItemData *data = g_new0 (ItemData, 1);
    data->dict = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    return data;
}


static void
item_data_free (ItemData *data)
{
    if (data)
    {
        if (data->dict)
            g_hash_table_destroy (data->dict);
        g_free (data->content);
        g_free (data);
    }
}


MooConfig *
moo_config_new (const char *item_id_key)
{
    g_return_val_if_fail (item_id_key != NULL, NULL);
    return g_object_new (MOO_TYPE_CONFIG, "item-id-key",
                         item_id_key, NULL);
}


guint
moo_config_n_items (MooConfig *config)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), 0);
    return config->priv->items->len;
}


MooConfigItem *
moo_config_nth_item (MooConfig  *config,
                     guint       n)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), NULL);
    g_return_val_if_fail (n < config->priv->items->len, NULL);
    return NTH_ITEM (config, n);
}


static char *
normalize_key (const char *string)
{
    char *norm = g_ascii_strdown (string, -1);
    return g_strdelimit (norm, "_", '-');
}


const char *
moo_config_item_get_value (MooConfigItem  *item,
                           const char     *key)
{
    char *norm;
    const char *value;

    g_return_val_if_fail (item != NULL, NULL);
    g_return_val_if_fail (key != NULL, NULL);

    norm = normalize_key (key);
    value = g_hash_table_lookup (item->dict, norm);

    g_free (norm);
    return value;
}


static void
emit_row_changed (MooConfig      *config,
                  MooConfigItem  *item)
{
    int index;
    GtkTreeIter iter;
    GtkTreePath *path;

    index = get_item_index (config, item);
    g_return_if_fail (index >= 0);

    config->priv->model_stamp++;
    path = gtk_tree_path_new_from_indices (index, -1);
    ITER_SET (config, &iter, index);
    g_signal_emit_by_name (config, "row-changed", path, &iter);
    gtk_tree_path_free (path);
}


void
moo_config_set_value (MooConfig      *config,
                      MooConfigItem  *item,
                      const char     *key,
                      const char     *value,
                      gboolean        modify)
{
    char *norm, *old_value;
    gboolean modified = FALSE;

    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);

    norm = normalize_key (key);
    old_value = g_hash_table_lookup (item->dict, norm);

    if (value)
    {
        if (!old_value || strcmp (value, old_value))
        {
            g_hash_table_insert (item->dict, norm, g_strdup (value));
            modified = TRUE;
        }
    }
    else
    {
        if (old_value)
        {
            g_hash_table_remove (item->dict, norm);
            g_free (norm);
            modified = TRUE;
        }
    }

    if (modified && modify)
        config->priv->modified = TRUE;

    if (modified)
        emit_row_changed (config, item);
}


const char *
moo_config_item_get_content (MooConfigItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->content;
}


void
moo_config_set_item_content (MooConfig      *config,
                             MooConfigItem  *item,
                             const char     *content,
                             gboolean        modify)
{
    char *tmp;

    g_return_if_fail (item != NULL);
    g_return_if_fail (MOO_IS_CONFIG (config));

    if (item->content == content)
        return;

    if ((!item->content && content) || (item->content && !content) ||
          strcmp (content, item->content))
    {
        tmp = item->content;
        item->content = g_strdup (content);
        g_free (tmp);

        if (modify)
            config->priv->modified = TRUE;

        emit_row_changed (config, item);
    }
}


static int
get_item_index (MooConfig     *config,
                MooConfigItem *item)
{
    guint i;

    for (i = 0; i < config->priv->items->len; ++i)
        if (config->priv->items->pdata[i] == item)
            return i;

    return -1;
}


void
moo_config_delete_item (MooConfig  *config,
                        const char *id,
                        gboolean    modify)
{
    MooConfigItem *item;
    GtkTreePath *path;
    int index;

    g_return_if_fail (MOO_IS_CONFIG (config));
    g_return_if_fail (id != NULL);

    item = g_hash_table_lookup (config->priv->ids, id);

    if (!item)
        return;

    index = get_item_index (config, item);
    g_return_if_fail (index >= 0);

    g_ptr_array_remove_index (config->priv->items, index);
    g_hash_table_remove (config->priv->ids, id);
    moo_config_item_free (item);

    if (modify)
        config->priv->modified = TRUE;

    config->priv->model_stamp++;
    path = gtk_tree_path_new_from_indices (index, -1);
    g_signal_emit_by_name (config, "row-deleted", path);
    gtk_tree_path_free (path);
}


static MooConfigItem *
moo_config_new_item_real (MooConfig  *config,
                          const char *id,
                          GHashTable *dict,
                          gboolean    modify)
{
    MooConfigItem *item;
    GtkTreePath *path;
    GtkTreeIter iter;

    item = g_hash_table_lookup (config->priv->ids, id);

    if (item)
        moo_config_delete_item (config, id, modify);

    if (dict)
        g_hash_table_remove (dict, config->priv->id_key);

    item = moo_config_item_new (id, dict);
    g_ptr_array_add (config->priv->items, item);
    g_hash_table_insert (config->priv->ids, g_strdup (id), item);

    if (modify)
        config->priv->modified = TRUE;

    config->priv->model_stamp++;
    ITER_SET (config, &iter, config->priv->items->len - 1);
    path = gtk_tree_path_new_from_indices (config->priv->items->len - 1, -1);
    g_signal_emit_by_name (config, "row-inserted", path, &iter);
    gtk_tree_path_free (path);

    return item;
}


MooConfigItem *
moo_config_new_item (MooConfig  *config,
                     const char *id,
                     gboolean    modify)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), NULL);
    g_return_val_if_fail (id && id[0], NULL);
    return moo_config_new_item_real (config, id, NULL, modify);
}


/*********************************************************************/
/* Parsing
 */

inline static const char *
find_line_term (const char *string,
                guint       len,
                guint      *term_len)
{
    while (len)
    {
        switch (string[0])
        {
            case '\r':
                if (len > 1 && string[1] == '\n')
                    *term_len = 2;
                else
                    *term_len = 1;
                return string;

            case '\n':
                *term_len = 1;
                return string;

            default:
                len--;
                string++;
        }
    }

    return NULL;
}

inline static char *
normalize_indent (const char *line,
                  guint       len)
{
    char *norm;
    guint n_tabs;

    if (!len || line[0] != '\t')
        return g_strndup (line, len);

    for (n_tabs = 0; n_tabs < len && line[n_tabs] == '\t'; ++n_tabs) ;

    norm = g_new (char, len - n_tabs + n_tabs * 8 + 1);
    norm[len - n_tabs + n_tabs * 8] = 0;

    memset (norm, ' ', n_tabs * 8);

    if (n_tabs < len)
        memcpy (norm + n_tabs * 8, line + n_tabs, len - n_tabs);

    return norm;
}

static char **
splitlines (const char *string,
            guint       len,
            guint      *n_lines_p)
{
    GSList *list = NULL;
    char **lines;
    guint n_lines = 0, i;

    if (!len)
    {
        *n_lines_p = 0;
        return g_new0 (char*, 1);
    }

    while (len)
    {
        guint term_len = 0;
        const char *term = find_line_term (string, len, &term_len);

        n_lines++;

        if (term)
        {
            list = g_slist_prepend (list, normalize_indent (string, term - string));
            len -= (term - string + term_len);
            string = term + term_len;
        }
        else
        {
            list = g_slist_prepend (list, normalize_indent (string, len));
            break;
        }
    }

    list = g_slist_reverse (list);
    lines = g_new (char*, n_lines + 1);
    lines[n_lines] = NULL;
    i = 0;

    while (list)
    {
        lines[i++] = list->data;
        list = g_slist_delete_link (list, list);
    }

    *n_lines_p = n_lines;
    return lines;
}


static gboolean
line_is_blank_or_comment (const char *line)
{
    if (!line[0] || line[0] == '#')
        return TRUE;

    while (*line)
    {
        if (*line != ' ' && *line != '\t')
            return FALSE;
        line++;
    }

    return TRUE;
}


static gboolean
line_is_blank (const char *line)
{
    while (line[0])
    {
        if (line[0] != ' ' && line[0] != '\t')
            return FALSE;

        line++;
    }

    return TRUE;
}


static guint
line_indent (const char *line)
{
    guint indent = 0;

    while (TRUE)
    {
        switch (*line++)
        {
            case ' ':
                indent += 1;
                break;

            case '\t':
                indent += 8;
                break;

            default:
                return indent;
        }
    }
}


inline static gboolean
line_is_comment (const char *line)
{
    return line[0] == '#';
}


inline static gboolean
line_is_indented (const char *line)
{
    return line[0] == ' ';
}


static gboolean
parse_header_line (char          **lines,
                   guint           n,
                   ItemData       *data)
{
    char *line = lines[n];
    char *value = NULL, *key = NULL;

    value = strchr (line, ':');

    if (!value)
    {
        g_warning ("%s: value missing on line %d", G_STRLOC, n+1);
        goto error;
    }

    key = g_strstrip (g_strndup (line, value - line));
    value = g_strstrip (g_strdup (value + 1));

    if (!key[0])
    {
        g_warning ("%s: empty key on line %d", G_STRLOC, n+1);
        goto error;
    }

    if (!value[0])
    {
        g_warning ("%s: value missing on line %d", G_STRLOC, n+1);
        goto error;
    }

    if (g_hash_table_lookup (data->dict, key))
    {
        g_warning ("%s: duplicated key '%s' on line %d", G_STRLOC, key, n+1);
        goto error;
    }

    g_hash_table_insert (data->dict, key, value);
    return TRUE;

error:
    g_free (value);
    g_free (key);
    return FALSE;
}


static void
moo_config_add_items (MooConfig *config,
                      GSList    *list,
                      gboolean   modify)
{
    while (list)
    {
        ItemData *data = list->data;
        MooConfigItem *item;
        char *id;

        id = g_hash_table_lookup (data->dict, config->priv->id_key);

        if (!id)
        {
            g_warning ("%s: '%s' missing on line %d",
                       G_STRLOC, config->priv->id_key,
                       data->start);
            goto skip;
        }

        id = g_strdup (id);
        item = moo_config_new_item_real (config, id, data->dict, modify);
        item->content = data->content;
        item->start = data->start;
        item->end = data->end;
        data->dict = NULL;
        data->content = NULL;
        g_free (id);

skip:
        list = list->next;
    }
}


static gboolean
moo_config_parse_lines (MooConfig *config,
                        char     **lines,
                        guint      n_lines,
                        gboolean   modify)
{
    ItemData *data;
    GPtrArray *content;
    guint start;
    GSList *items;

    content = g_ptr_array_new ();
    start = 0;
    items = NULL;
    data = NULL;

    while (start < n_lines)
    {
        guint content_start;

        while (start < n_lines && line_is_blank_or_comment (lines[start]))
            start++;

        if (start == n_lines)
            break;

        if (line_is_indented (lines[start]))
        {
            g_warning ("line %d", start+1);
            goto error;
        }

        content_start = start;

        while (content_start < n_lines)
        {
            const char *line = lines[content_start];

            if (line_is_blank (line) || line_is_indented (line))
                break;

            if (line_is_comment (lines[content_start]))
            {
                content_start++;
                continue;
            }

            if (!data)
                data = item_data_new ();

            if (!parse_header_line (lines, content_start, data))
                goto error;

            content_start++;
        }

        if (!data)
            break;

        start = content_start;

        if (start < n_lines &&
            line_is_indented (lines[start]) &&
            !line_is_blank (lines[start]))
        {
            while (start < n_lines)
            {
                if (line_is_blank_or_comment (lines[start]))
                {
                    start++;
                    continue;
                }

                if (!line_is_indented (lines[start]))
                    break;

                g_ptr_array_add (content, lines[start]);
                start++;
            }
        }

        if (content->len)
        {
            guint indent, i;
            GString *str;

            indent = line_indent (content->pdata[0]);

            for (i = 1; i < content->len; ++i)
            {
                guint indent_here = line_indent (content->pdata[i]);
                indent = MIN (indent_here, indent);
            }

            if (!indent)
            {
                g_critical ("%s: oops", G_STRLOC);
                goto error;
            }

            str = g_string_new (NULL);

            for (i = 0; i < content->len; ++i)
            {
                char *line = content->pdata[i];
                g_string_append (str, line + indent);
                g_string_append_c (str, '\n');
            }

            data->content = str->str;
            g_string_free (str, FALSE);
            g_ptr_array_set_size (content, 0);
        }

        items = g_slist_prepend (items, data);
        data = NULL;
    }

    items = g_slist_reverse (items);

    moo_config_add_items (config, items, modify);

    g_slist_foreach (items, (GFunc) item_data_free, NULL);
    g_slist_free (items);
    g_ptr_array_free (content, TRUE);

    return TRUE;

error:
    item_data_free (data);
    g_ptr_array_free (content, TRUE);
    g_slist_foreach (items, (GFunc) item_data_free, NULL);
    g_slist_free (items);
    return FALSE;
}


gboolean
moo_config_parse_buffer (MooConfig  *config,
                         const char *string,
                         int         len,
                         gboolean    modify)
{
    char **lines;
    guint n_lines;
    gboolean result;

    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    g_return_val_if_fail (string != NULL, FALSE);

    if (len < 0)
        len = strlen (string);

    lines = splitlines (string, len, &n_lines);
    result = moo_config_parse_lines (config, lines, n_lines, modify);

    g_strfreev (lines);
    return result;
}


gboolean
moo_config_parse_file (MooConfig  *config,
                       const char *filename,
                       gboolean    modify)
{
    GMappedFile *file;
    GError *error = NULL;
    gboolean result;

    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    file = g_mapped_file_new (filename, FALSE, &error);

    if (!file)
    {
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return FALSE;
    }

    result = moo_config_parse_buffer (config, g_mapped_file_get_contents (file),
                                      g_mapped_file_get_length (file), modify);

    g_mapped_file_free (file);
    return result;
}


gboolean
moo_config_item_get_bool (MooConfigItem  *item,
                          const char     *key,
                          gboolean        default_val)
{
    return moo_convert_string_to_bool (moo_config_item_get_value (item, key),
                                       default_val);
}


void
moo_config_set_bool (MooConfig      *config,
                     MooConfigItem  *item,
                     const char     *key,
                     gboolean        value,
                     gboolean        modify)
{
    moo_config_set_value (config, item, key,
                          moo_convert_bool_to_string (value),
                          modify);
}


const char *
moo_config_item_get_id (MooConfigItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return item->id;
}


static void
format_key (const char *key,
            const char *value,
            GString    *string)
{
    g_string_append_printf (string, "%s: %s" LINE_TERM, key, value);
}

static void
moo_config_item_format (MooConfigItem *item,
                        const char    *id_key,
                        GString       *string)
{
    char **lines;
    guint n_lines, i;
    char *indent;

    g_string_append_printf (string, "%s: %s" LINE_TERM, id_key, item->id);

    g_hash_table_foreach (item->dict, (GHFunc) format_key, string);

    if (!item->content)
        return;

    lines = splitlines (item->content, strlen (item->content), &n_lines);
    indent = g_strnfill (2, ' ');

    for (i = 0; i < n_lines; ++i)
    {
        g_string_append (string, indent);
        g_string_append (string, lines[i]);
        g_string_append (string, LINE_TERM);
    }

    g_free (indent);
    g_strfreev (lines);
}


char *
moo_config_format (MooConfig *config)
{
    GString *string;
    guint i;

    g_return_val_if_fail (config != NULL, NULL);

    string = g_string_new (NULL);

    for (i = 0; i < config->priv->items->len; ++i)
    {
        if (i > 0)
            g_string_append (string, LINE_TERM);

        moo_config_item_format (config->priv->items->pdata[i],
                                config->priv->id_key, string);
    }

    return g_string_free (string, FALSE);
}


/***********************************************************************/
/* GtkTreeModel interface
 */

static GtkTreeModelFlags
moo_config_get_flags (G_GNUC_UNUSED GtkTreeModel *model)
{
    return GTK_TREE_MODEL_LIST_ONLY;
}


static int
moo_config_get_n_columns (G_GNUC_UNUSED GtkTreeModel *model)
{
    return 1;
}


static GType
moo_config_get_column_type (G_GNUC_UNUSED GtkTreeModel *model,
                            int           index)
{
    g_return_val_if_fail (index != 1, 0);
    return MOO_TYPE_CONFIG_ITEM;
}


static gboolean
moo_config_get_iter_nth (MooConfig   *config,
                         GtkTreeIter *iter,
                         int          index)
{
    if (index < 0 || index >= (int) config->priv->items->len)
    {
        ITER_SET_INVALID (iter);
        return FALSE;
    }

    ITER_SET (config, iter, index);
    return TRUE;
}


static gboolean
moo_config_get_iter (GtkTreeModel *model,
                     GtkTreeIter  *iter,
                     GtkTreePath  *path)
{
    if (gtk_tree_path_get_depth (path) != 1)
    {
        ITER_SET_INVALID (iter);
        return FALSE;
    }

    return moo_config_get_iter_nth (MOO_CONFIG (model), iter,
                                    gtk_tree_path_get_indices(path)[0]);
}


static GtkTreePath *
moo_config_get_path (GtkTreeModel   *model,
                     GtkTreeIter    *iter)
{
    MooConfig *config = MOO_CONFIG (model);
    g_return_val_if_fail (config->priv->model_stamp != iter->stamp, NULL);
    return gtk_tree_path_new_from_indices (ITER_INDEX (iter), -1);
}


static void
moo_config_get_value (GtkTreeModel   *model,
                      GtkTreeIter    *iter,
                      int             column,
                      GValue         *value)
{
    MooConfig *config = MOO_CONFIG (model);
    g_return_if_fail (config->priv->model_stamp != iter->stamp);
    g_return_if_fail (column != 0);
    g_value_set_pointer (value, config->priv->items->pdata[ITER_INDEX (iter)]);
}


static gboolean
moo_config_iter_next (GtkTreeModel   *model,
                      GtkTreeIter    *iter)
{
    return moo_config_get_iter_nth (MOO_CONFIG (model), iter,
                                    ITER_INDEX (iter) + 1);
}


static gboolean
moo_config_iter_children (GtkTreeModel   *model,
                          GtkTreeIter    *iter,
                          GtkTreeIter    *parent)
{
    if (parent)
    {
        ITER_SET_INVALID (iter);
        return FALSE;
    }
    else
    {
        return moo_config_get_iter_nth (MOO_CONFIG (model), iter, 0);
    }
}


static gboolean
moo_config_iter_has_child (GtkTreeModel   *model,
                           GtkTreeIter    *iter)
{
    if (iter)
        return FALSE;
    else
        return MOO_CONFIG(model)->priv->items->len != 0;
}


static int
moo_config_iter_n_children (GtkTreeModel   *model,
                            GtkTreeIter    *iter)
{
    if (iter)
        return 0;
    else
        return MOO_CONFIG(model)->priv->items->len;
}


static gboolean
moo_config_iter_nth_child (GtkTreeModel   *model,
                           GtkTreeIter    *iter,
                           GtkTreeIter    *parent,
                           int             n)
{
    if (parent)
    {
        ITER_SET_INVALID (iter);
        return FALSE;
    }
    else
    {
        return moo_config_get_iter_nth (MOO_CONFIG (model), iter, n);
    }
}


static gboolean
moo_config_iter_parent (G_GNUC_UNUSED GtkTreeModel *model,
                        G_GNUC_UNUSED GtkTreeIter  *iter,
                        G_GNUC_UNUSED GtkTreeIter  *child)
{
    ITER_SET_INVALID (iter);
    return FALSE;
}
