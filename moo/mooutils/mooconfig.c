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
    int model_stamp;
    gboolean modified;
};

struct _MooConfigItem {
    GHashTable *dict;
    char *content;
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
    PROP_MODIFIED
};

static MooConfigItem   *moo_config_item_new         (void);
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
}


static void
moo_config_finalize (GObject *object)
{
    MooConfig *config = MOO_CONFIG (object);

    g_ptr_array_foreach (config->priv->items,
                         (GFunc) moo_config_item_free,
                         NULL);
    g_ptr_array_free (config->priv->items, TRUE);

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
        case PROP_MODIFIED:
            moo_config_set_modified (config, g_value_get_boolean (value));
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
        case PROP_MODIFIED:
            g_value_set_boolean (value, config->priv->modified);
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
                                     PROP_MODIFIED,
                                     g_param_spec_boolean ("modified",
                                             "modified",
                                             "modified",
                                             FALSE,
                                             G_PARAM_READWRITE));
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


gboolean
moo_config_get_modified (MooConfig *config)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    return config->priv->modified;
}


void
moo_config_set_modified (MooConfig  *config,
                         gboolean    modified)
{
    g_return_if_fail (MOO_IS_CONFIG (config));

    modified = modified != 0;

    if (modified != config->priv->modified)
    {
        config->priv->modified = modified;
        g_object_notify (G_OBJECT (config), "modified");
    }
}


GQuark
moo_config_error_quark (void)
{
    return g_quark_from_static_string ("moo-config-error-quark");
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
moo_config_item_new (void)
{
    MooConfigItem *item = g_new0 (MooConfigItem, 1);
    item->dict = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    return item;
}


static void
moo_config_item_free (MooConfigItem *item)
{
    if (item)
    {
        g_hash_table_destroy (item->dict);
        g_free (item->content);
        g_free (item);
    }
}


MooConfig *
moo_config_new (void)
{
    return g_object_new (MOO_TYPE_CONFIG, NULL);
}


MooConfig *
moo_config_new_from_file (const char *filename,
                          gboolean    modified,
                          GError    **error)
{
    MooConfig *config;

    g_return_val_if_fail (filename != NULL, NULL);

    config = moo_config_new ();

    if (moo_config_parse_file (config, filename, modified, error))
        return config;

    g_object_unref (config);
    return NULL;
}


MooConfig *
moo_config_new_from_buffer (const char     *string,
                            int             len,
                            gboolean        modified,
                            GError        **error)
{
    MooConfig *config;

    g_return_val_if_fail (string != NULL, NULL);

    config = moo_config_new ();

    if (moo_config_parse_buffer (config, string, len, modified, error))
        return config;

    g_object_unref (config);
    return NULL;
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
    gtk_tree_model_row_changed (GTK_TREE_MODEL (config), path, &iter);
    gtk_tree_path_free (path);
}


static gboolean
is_empty_string (const char *string)
{
    if (!string)
        return TRUE;

#define IS_SPACE(c) (c == ' ' || c == '\t' || c == '\r' || c == '\n')
    while (*string)
    {
        if (*string && !IS_SPACE (*string))
            return FALSE;
        string++;
    }
#undef IS_SPACE

    return TRUE;
}


void
moo_config_set_value (MooConfig      *config,
                      MooConfigItem  *item,
                      const char     *key,
                      const char     *value,
                      gboolean        modify)
{
    char *norm, *old_value, *new_value = NULL;
    gboolean modified = FALSE;

    g_return_if_fail (item != NULL);
    g_return_if_fail (key != NULL);

    norm = normalize_key (key);
    old_value = g_hash_table_lookup (item->dict, norm);

    if (value)
    {
        new_value = g_strstrip (g_strdup (value));

        if (is_empty_string (new_value))
        {
            g_free (new_value);
            new_value = NULL;
        }
    }

    if (new_value)
    {
        if (!old_value || strcmp (new_value, old_value))
        {
            g_hash_table_insert (item->dict, norm, new_value);
            new_value = NULL;
            modified = TRUE;
        }
    }
    else if (old_value)
    {
        g_hash_table_remove (item->dict, norm);
        g_free (norm);
        modified = TRUE;
    }

    if (modified && modify)
        moo_config_set_modified (config, TRUE);

    if (modified)
        emit_row_changed (config, item);

    g_free (new_value);
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
            moo_config_set_modified (config, TRUE);

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
                        guint       index,
                        gboolean    modify)
{
    MooConfigItem *item;
    GtkTreePath *path;

    g_return_if_fail (MOO_IS_CONFIG (config));
    g_return_if_fail (index < config->priv->items->len);

    item = config->priv->items->pdata[index];
    g_ptr_array_remove_index (config->priv->items, index);
    moo_config_item_free (item);

    if (modify)
        moo_config_set_modified (config, TRUE);

    config->priv->model_stamp++;
    path = gtk_tree_path_new_from_indices (index, -1);
    gtk_tree_model_row_deleted (GTK_TREE_MODEL (config), path);
    gtk_tree_path_free (path);
}


void
moo_config_move_item (MooConfig  *config,
                      guint       index,
                      guint       new_index,
                      gboolean    modify)
{
    MooConfigItem *item;
    int *new_order;
    guint i;
    GtkTreePath *path;

    g_return_if_fail (MOO_IS_CONFIG (config));
    g_return_if_fail (index < config->priv->items->len);
    g_return_if_fail (new_index < config->priv->items->len);

    if (index == new_index)
        return;

    item = config->priv->items->pdata[index];
    g_ptr_array_remove_index (config->priv->items, index);
    g_ptr_array_add (config->priv->items, item);

    if (new_index != config->priv->items->len - 1)
    {
        memmove (config->priv->items->pdata + new_index + 1,
                 config->priv->items->pdata + new_index,
                 sizeof (gpointer) * (config->priv->items->len - new_index - 1));
        config->priv->items->pdata[new_index] = item;
    }

    if (modify)
        moo_config_set_modified (config, TRUE);

    new_order = g_new (int, config->priv->items->len);

    for (i = 0; i < config->priv->items->len; ++i)
        if (i < MIN (index, new_index) || i > MAX (index, new_index))
            new_order[i] = i;
        else if (i == new_index)
            new_order[i] = index;
        else if (index < new_index)
            new_order[i] = i + 1;
        else
            new_order[i] = i - 1;

    config->priv->model_stamp++;
    path = gtk_tree_path_new ();
    gtk_tree_model_rows_reordered (GTK_TREE_MODEL (config),
                                   path, NULL, new_order);
    g_free (new_order);
    gtk_tree_path_free (path);
}


static MooConfigItem *
moo_config_new_item_real (MooConfig     *config,
                          int            index,
                          MooConfigItem *item,
                          gboolean       modify)
{
    GtkTreePath *path;
    GtkTreeIter iter;

    if (!item)
        item = moo_config_item_new ();

    if (index >= (int) config->priv->items->len || index < 0)
        index = config->priv->items->len;

    if (index == (int) config->priv->items->len)
    {
        g_ptr_array_add (config->priv->items, item);
    }
    else
    {
        g_ptr_array_add (config->priv->items, NULL);
        memmove (config->priv->items->pdata + index + 1,
                 config->priv->items->pdata + index,
                 sizeof (gpointer) * (config->priv->items->len - index - 1));
        config->priv->items->pdata[index] = item;
    }

    if (modify)
        moo_config_set_modified (config, TRUE);

    config->priv->model_stamp++;
    ITER_SET (config, &iter, index);
    path = gtk_tree_path_new_from_indices (index, -1);
    gtk_tree_model_row_inserted (GTK_TREE_MODEL (config), path, &iter);
    gtk_tree_path_free (path);

    return item;
}


MooConfigItem *
moo_config_new_item (MooConfig  *config,
                     int         index,
                     gboolean    modify)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), NULL);
    return moo_config_new_item_real (config, index, NULL, modify);
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
                   MooConfigItem  *item,
                   const char     *filename,
                   GError        **error)
{
    char *line = lines[n];
    char *value = NULL, *key = NULL;

    value = strchr (line, ':');

    if (!value)
    {
        g_set_error (error, MOO_CONFIG_ERROR, MOO_CONFIG_ERROR_PARSE,
                     "In '%s': value missing on line %d", filename, n+1);
        goto error;
    }

    key = g_strstrip (g_strndup (line, value - line));
    value = g_strstrip (g_strdup (value + 1));

    if (!key[0])
    {
        g_set_error (error, MOO_CONFIG_ERROR, MOO_CONFIG_ERROR_PARSE,
                     "In '%s': empty key on line %d", filename, n+1);
        goto error;
    }

    if (!value[0])
    {
        g_set_error (error, MOO_CONFIG_ERROR, MOO_CONFIG_ERROR_PARSE,
                     "In '%s': value missing on line %d", filename, n+1);
        goto error;
    }

    if (g_hash_table_lookup (item->dict, key))
    {
        g_set_error (error, MOO_CONFIG_ERROR, MOO_CONFIG_ERROR_PARSE,
                     "In '%s': duplicated key '%s' on line %d",
                     filename, key, n+1);
        goto error;
    }

    g_hash_table_insert (item->dict, key, value);
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
        moo_config_new_item_real (config, -1, list->data, modify);
        list = g_slist_delete_link (list, list);
    }
}


static gboolean
moo_config_parse_lines (MooConfig  *config,
                        char      **lines,
                        guint       n_lines,
                        gboolean    modify,
                        const char *filename,
                        GError    **error)
{
    MooConfigItem *item;
    GPtrArray *content;
    guint start;
    GSList *items;

    content = g_ptr_array_new ();
    start = 0;
    items = NULL;
    item = NULL;

    while (start < n_lines)
    {
        guint content_start;

        while (start < n_lines && line_is_blank_or_comment (lines[start]))
            start++;

        if (start == n_lines)
            break;

        if (line_is_indented (lines[start]))
        {
            g_set_error (error, MOO_CONFIG_ERROR, MOO_CONFIG_ERROR_PARSE,
                         "In '%s', line %d: indented line without header",
                         filename, start+1);
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

            if (!item)
            {
                item = moo_config_item_new ();
                item->start = start;
            }

            if (!parse_header_line (lines, content_start, item, filename, error))
                goto error;

            content_start++;
        }

        if (!item)
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
                g_set_error (error, MOO_CONFIG_ERROR,
                             MOO_CONFIG_ERROR_PARSE,
                             "%s: oops", G_STRLOC);
                goto error;
            }

            str = g_string_new (NULL);

            for (i = 0; i < content->len; ++i)
            {
                char *line = content->pdata[i];
                g_string_append (str, line + indent);
                g_string_append_c (str, '\n');
            }

            item->content = str->str;
            g_string_free (str, FALSE);
            g_ptr_array_set_size (content, 0);
        }

        item->end = start;
        items = g_slist_prepend (items, item);
        item = NULL;
    }

    items = g_slist_reverse (items);

    moo_config_add_items (config, items, modify);

    g_ptr_array_free (content, TRUE);

    return TRUE;

error:
    moo_config_item_free (item);
    g_ptr_array_free (content, TRUE);
    g_slist_foreach (items, (GFunc) moo_config_item_free, NULL);
    g_slist_free (items);
    return FALSE;
}


static gboolean
moo_config_parse_buffer_real (MooConfig  *config,
                              const char *filename,
                              const char *string,
                              int         len,
                              gboolean    modify,
                              GError    **error)
{
    char **lines;
    guint n_lines;
    gboolean result;

    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    g_return_val_if_fail (string != NULL, FALSE);

    if (len < 0)
        len = strlen (string);

    lines = splitlines (string, len, &n_lines);
    result = moo_config_parse_lines (config, lines, n_lines,
                                     modify, filename, error);

    g_strfreev (lines);
    return result;
}


gboolean
moo_config_parse_buffer (MooConfig  *config,
                         const char *string,
                         int         len,
                         gboolean    modify,
                         GError    **error)
{
    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    g_return_val_if_fail (string != NULL, FALSE);
    return moo_config_parse_buffer_real (config, "<input>",
                                         string, len, modify,
                                         error);
}


gboolean
moo_config_parse_file (MooConfig  *config,
                       const char *filename,
                       gboolean    modify,
                       GError    **error)
{
    GMappedFile *file;
    GError *error_here = NULL;
    gboolean result;

    g_return_val_if_fail (MOO_IS_CONFIG (config), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    file = g_mapped_file_new (filename, FALSE, &error_here);

    if (!file)
    {
        g_set_error (error, MOO_CONFIG_ERROR,
                     MOO_CONFIG_ERROR_FILE,
                     "%s", error_here->message);
        g_error_free (error_here);
        return FALSE;
    }

    result = moo_config_parse_buffer_real (config, filename,
                                           g_mapped_file_get_contents (file),
                                           g_mapped_file_get_length (file),
                                           modify, error);

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


static void
format_key (const char *key,
            const char *value,
            GString    *string)
{
    g_string_append_printf (string, "%s: %s" LINE_TERM, key, value);
}

static void
moo_config_item_format (MooConfigItem *item,
                        GString       *string)
{
    char **lines;
    guint n_lines, i;
    char *indent;

    g_return_if_fail (g_hash_table_size (item->dict) != 0);

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

        moo_config_item_format (config->priv->items->pdata[i], string);
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
    g_return_val_if_fail (index == 0, 0);
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
    g_return_val_if_fail (config->priv->model_stamp == iter->stamp, NULL);
    return gtk_tree_path_new_from_indices (ITER_INDEX (iter), -1);
}


static void
moo_config_get_value (GtkTreeModel   *model,
                      GtkTreeIter    *iter,
                      int             column,
                      GValue         *value)
{
    MooConfig *config = MOO_CONFIG (model);
    g_return_if_fail (config->priv->model_stamp == iter->stamp);
    g_return_if_fail (column == 0);
    g_value_init (value, MOO_TYPE_CONFIG_ITEM);
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


void
moo_config_get_item_iter (MooConfig      *config,
                          MooConfigItem  *item,
                          GtkTreeIter    *iter)
{
    int index;

    g_return_if_fail (MOO_IS_CONFIG (config));
    g_return_if_fail (item != NULL);
    g_return_if_fail (iter != NULL);

    index = get_item_index (config, item);
    g_return_if_fail (index >= 0);

    moo_config_get_iter_nth (config, iter, index);
}


/****************************************************************************/
/* treeview + entries
 */

typedef struct {
    char *key;
    gboolean default_val;
    gboolean update_live;
    gulong handler;
} WidgetInfo;

typedef struct {
    GtkTreeView *treeview;
    GSList *widgets;
    GtkWidget *new_btn;
    GtkWidget *delete_btn;
    GtkWidget *up_btn;
    GtkWidget *down_btn;
    MooConfigSetupItemFunc setup_func;
    gpointer data;
} Widgets;

static WidgetInfo  *widget_info_new     (const char         *key,
                                         gboolean            update_live,
                                         gboolean            default_bool);
static void         widget_info_free    (WidgetInfo         *info);
static Widgets     *widgets_new         (GtkWidget          *tree_view,
                                         GtkWidget          *new_btn,
                                         GtkWidget          *delete_btn,
                                         GtkWidget          *up_btn,
                                         GtkWidget          *down_btn);
static void         widgets_free        (Widgets            *widgets);

static void         selection_changed   (GtkTreeSelection   *selection,
                                         Widgets            *widgets);
static void         set_from_model      (Widgets            *widgets,
                                         GtkTreeModel       *model,
                                         GtkTreePath        *path);
static void         button_new          (Widgets            *widgets);
static void         button_delete       (Widgets            *widgets);
static void         button_up           (Widgets            *widgets);
static void         button_down         (Widgets            *widgets);
static void         entry_changed       (Widgets            *widgets,
                                         GtkEntry           *entry);
static void         button_toggled      (Widgets            *widgets,
                                         GtkToggleButton    *button);


static Widgets *
get_widgets (GtkWidget *treeview)
{
    return g_object_get_data (G_OBJECT (treeview), "moo-config-widgets");
}


void
moo_config_connect_widget (GtkWidget      *treeview,
                           GtkWidget      *new_btn,
                           GtkWidget      *delete_btn,
                           GtkWidget      *up_btn,
                           GtkWidget      *down_btn,
                           MooConfigSetupItemFunc func,
                           gpointer        data)
{
    Widgets *widgets;
    GtkTreeSelection *selection;

    g_return_if_fail (GTK_IS_TREE_VIEW (treeview));
    g_return_if_fail (!get_widgets (treeview));

    widgets = widgets_new (treeview, new_btn, delete_btn, up_btn, down_btn);
    widgets->setup_func = func;
    widgets->data = data;
    g_object_set_data_full (G_OBJECT (treeview), "moo-config-widgets", widgets,
                            (GDestroyNotify) widgets_free);
    g_signal_connect (treeview, "destroy",
                      G_CALLBACK (moo_config_disconnect_widget),
                      NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (selection, "changed",
                      G_CALLBACK (selection_changed), widgets);

    if (new_btn)
        g_signal_connect_swapped (new_btn, "clicked", G_CALLBACK (button_new), widgets);
    if (delete_btn)
        g_signal_connect_swapped (delete_btn, "clicked", G_CALLBACK (button_delete), widgets);
    if (up_btn)
        g_signal_connect_swapped (up_btn, "clicked", G_CALLBACK (button_up), widgets);
    if (down_btn)
        g_signal_connect_swapped (down_btn, "clicked", G_CALLBACK (button_down), widgets);
}


static WidgetInfo *
get_info (gpointer widget)
{
    return g_object_get_data (G_OBJECT (widget), "moo-config-widget-info");
}


void
moo_config_add_widget (GtkWidget      *tree_view,
                       GtkWidget      *widget,
                       const char     *key,
                       gboolean        update_live,
                       gboolean        default_bool)
{
    Widgets *widgets;
    WidgetInfo *info;

    widgets = get_widgets (tree_view);
    g_return_if_fail (widgets != NULL);
    g_return_if_fail (!update_live || key != NULL);

    g_return_if_fail (GTK_IS_TOGGLE_BUTTON (widget) ||
            GTK_IS_ENTRY (widget) || GTK_IS_TEXT_VIEW (widget));

    widgets->widgets = g_slist_prepend (widgets->widgets, widget);
    info = widget_info_new (key, update_live, default_bool);
    g_object_set_data_full (G_OBJECT (widget), "moo-config-widget-info", info,
                            (GDestroyNotify) widget_info_free);

    if (update_live)
    {
        if (GTK_IS_TOGGLE_BUTTON (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "toggled",
                                              G_CALLBACK (button_toggled),
                                              widgets);
        else if (GTK_IS_ENTRY (widget))
            info->handler =
                    g_signal_connect_swapped (widget, "changed",
                                              G_CALLBACK (entry_changed),
                                              widgets);
    }
}


void
moo_config_disconnect_widget (GtkWidget *treeview)
{
    GtkTreeSelection *selection;
    Widgets *widgets;
    GSList *l;

    widgets = get_widgets (treeview);

    if (!widgets)
        return;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_handlers_disconnect_by_func (selection, (gpointer) selection_changed, widgets);

    if (widgets->new_btn)
        g_signal_handlers_disconnect_by_func (widgets->new_btn,
                                              (gpointer) button_new, widgets);
    if (widgets->delete_btn)
        g_signal_handlers_disconnect_by_func (widgets->delete_btn,
                                              (gpointer) button_delete, widgets);
    if (widgets->up_btn)
        g_signal_handlers_disconnect_by_func (widgets->up_btn,
                                              (gpointer) button_up, widgets);
    if (widgets->down_btn)
        g_signal_handlers_disconnect_by_func (widgets->down_btn,
                                              (gpointer) button_down, widgets);

    for (l = widgets->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget = l->data;
        WidgetInfo *info = get_info (widget);

        if (info->handler)
            g_signal_handler_disconnect (widget, info->handler);

        g_object_set_data (G_OBJECT (widget), "moo-config-widget-info", NULL);
    }

    g_object_set_data (G_OBJECT (treeview), "moo-config-widgets", NULL);
}


static WidgetInfo *
widget_info_new (const char *key,
                 gboolean    update_live,
                 gboolean    default_bool)
{
    WidgetInfo *info = g_new0 (WidgetInfo, 1);

    info->key = g_strdup (key);
    info->update_live = update_live;
    info->default_val = default_bool;

    return info;
}


static void
widget_info_free (WidgetInfo *info)
{
    if (info)
    {
        g_free (info->key);
        g_free (info);
    }
}


static Widgets *
widgets_new (GtkWidget *tree_view,
             GtkWidget *new_btn,
             GtkWidget *delete_btn,
             GtkWidget *up_btn,
             GtkWidget *down_btn)
{
    Widgets *w = g_new0 (Widgets, 1);
    w->treeview = GTK_TREE_VIEW (tree_view);
    w->new_btn = new_btn;
    w->delete_btn = delete_btn;
    w->up_btn = up_btn;
    w->down_btn = down_btn;
    return w;
}


static void
widgets_free (Widgets *widgets)
{
    if (widgets)
    {
        g_slist_free (widgets->widgets);
        g_free (widgets);
    }
}


static void
selection_changed (GtkTreeSelection *selection,
                   Widgets          *widgets)
{
    GtkTreeRowReference *old_row;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *old_path;

    old_row = g_object_get_data (G_OBJECT (selection), "moo-config-current-row");
    old_path = old_row ? gtk_tree_row_reference_get_path (old_row) : NULL;

    if (old_row && !old_path)
    {
        g_object_set_data (G_OBJECT (selection), "moo-config-current-row", NULL);
        old_row = NULL;
    }

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        path = gtk_tree_model_get_path (model, &iter);

        if (old_path && !gtk_tree_path_compare (old_path, path))
        {
            gtk_tree_path_free (old_path);
            gtk_tree_path_free (path);
            return;
        }
    }
    else
    {
        if (!old_path)
            return;

        path = NULL;
    }

    if (old_path)
        moo_config_update_tree_view (GTK_WIDGET (widgets->treeview),
                                     model, old_path);

    set_from_model (widgets, model, path);

    if (path)
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (model, path);
        g_object_set_data_full (G_OBJECT (selection), "moo-config-current-row", row,
                                (GDestroyNotify) gtk_tree_row_reference_free);
    }
    else
    {
        g_object_set_data (G_OBJECT (selection), "moo-config-current-row", NULL);
    }

    gtk_tree_path_free (path);
    gtk_tree_path_free (old_path);
}


static void
set_from_model (Widgets      *widgets,
                GtkTreeModel *model,
                GtkTreePath  *path)
{
    GtkTreeIter iter;
    MooConfigItem *item = NULL;
    GSList *l;

    g_return_if_fail (MOO_IS_CONFIG (model));

    if (path)
    {
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, 0, &item, -1);
        g_return_if_fail (item != NULL);
    }

    for (l = widgets->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        WidgetInfo *info;

        widget = l->data;
        info = get_info (widget);

        if (info->handler)
            g_signal_handler_block (widget, info->handler);

        gtk_widget_set_sensitive (widget, item != NULL);

        if (item)
        {
            if (GTK_IS_TOGGLE_BUTTON (widget))
            {
                gboolean val = moo_config_item_get_bool (item, info->key, info->default_val);
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), val);
            }
            else
            {
                const char *text;

                if (info->key)
                    text = moo_config_item_get_value (item, info->key);
                else
                    text = moo_config_item_get_content (item);

                if (!text)
                    text = "";

                if (GTK_IS_ENTRY (widget))
                    gtk_entry_set_text (GTK_ENTRY (widget), text);
                else
                    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget)),
                                              text, -1);
            }
        }
        else
        {
            if (GTK_IS_TOGGLE_BUTTON (widget))
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
            else if (GTK_IS_ENTRY (widget))
                gtk_entry_set_text (GTK_ENTRY (widget), "");
            else
                    gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget)),
                                              "", -1);
        }

        if (info->handler)
            g_signal_handler_unblock (widget, info->handler);
    }

    if (!item)
    {
        if (widgets->delete_btn)
            gtk_widget_set_sensitive (widgets->delete_btn, FALSE);
        if (widgets->up_btn)
            gtk_widget_set_sensitive (widgets->up_btn, FALSE);
        if (widgets->down_btn)
            gtk_widget_set_sensitive (widgets->down_btn, FALSE);
    }
    else
    {
        int *indices;

        if (widgets->delete_btn)
            gtk_widget_set_sensitive (widgets->delete_btn, TRUE);

        indices = gtk_tree_path_get_indices (path);

        if (widgets->up_btn)
            gtk_widget_set_sensitive (widgets->up_btn, indices[0] != 0);
        if (widgets->down_btn)
            gtk_widget_set_sensitive (widgets->down_btn, (guint) indices[0] !=
                                      moo_config_n_items (MOO_CONFIG (model)) - 1);
    }
}


void
moo_config_update_widgets (GtkWidget *treeview)
{
    Widgets *widgets;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    widgets = get_widgets (treeview);
    g_return_if_fail (widgets != NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

    if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
        set_from_model (widgets, model, path);
        gtk_tree_path_free (path);
    }
    else
    {
        set_from_model (widgets, model, NULL);
    }
}


static int
iter_get_index (GtkTreeModel *model,
                GtkTreeIter  *iter)
{
    int index;
    GtkTreePath *path;

    path = gtk_tree_model_get_path (model, iter);

    if (!path)
        return -1;

    index = gtk_tree_path_get_indices(path)[0];

    gtk_tree_path_free (path);
    return index;
}


static void
button_new (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    MooConfigItem *item;
    int index;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        index = -1;
    else
        index = iter_get_index (model, &iter) + 1;

    item = moo_config_new_item (MOO_CONFIG (model), index, TRUE);

    if (widgets->setup_func)
        widgets->setup_func (MOO_CONFIG (model), item, widgets->data);

    moo_config_get_item_iter (MOO_CONFIG (model), item, &iter);
    gtk_tree_selection_select_iter (selection, &iter);
}


static void
button_delete (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    int index;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    index = iter_get_index (model, &iter);
    moo_config_delete_item (MOO_CONFIG (model), index, TRUE);

    if (gtk_tree_model_iter_nth_child (model, &iter, NULL, index))
        gtk_tree_selection_select_iter (selection, &iter);
}


static void
button_up (Widgets *widgets)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    GtkTreeSelection *selection;
    int *indices;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);

    if (!indices[0])
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] - 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] - 1, -1);
    set_from_model (widgets, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
button_down (Widgets *widgets)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path, *new_path;
    int *indices;
    int n_children;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        g_return_if_reached ();

    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);
    n_children = gtk_tree_model_iter_n_children (model, NULL);

    if (indices[0] == n_children - 1)
        g_return_if_reached ();

    moo_config_move_item (MOO_CONFIG (model), indices[0], indices[0] + 1, TRUE);
    new_path = gtk_tree_path_new_from_indices (indices[0] + 1, -1);
    set_from_model (widgets, model, new_path);

    gtk_tree_path_free (new_path);
    gtk_tree_path_free (path);
}


static void
entry_changed (Widgets  *widgets,
               GtkEntry *entry)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    const char *text;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    info = get_info (entry);
    g_return_if_fail (info != NULL);

    text = gtk_entry_get_text (entry);
    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    if (info->key)
        moo_config_set_value (MOO_CONFIG (model), item,
                              info->key, text, TRUE);
    else
        moo_config_set_item_content (MOO_CONFIG (model), item,
                                     text, TRUE);
}


static void
button_toggled (Widgets         *widgets,
                GtkToggleButton *button)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    MooConfigItem *item = NULL;
    WidgetInfo *info;
    GtkTreeSelection *selection;

    selection = gtk_tree_view_get_selection (widgets->treeview);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
        return;

    info = get_info (button);
    g_return_if_fail (info != NULL);

    gtk_tree_model_get (model, &iter, 0, &item, -1);
    g_return_if_fail (item != NULL);

    moo_config_set_bool (MOO_CONFIG (model), item, info->key,
                         gtk_toggle_button_get_active (button), TRUE);
}


void
moo_config_update_tree_view (GtkWidget      *treeview,
                             GtkTreeModel   *model,
                             GtkTreePath    *path)
{
    GtkTreeIter iter;
    Widgets *widgets;
    GSList *l;
    MooConfigItem *item = NULL;

    widgets = get_widgets (treeview);
    g_return_if_fail (widgets != NULL);

    if (!model)
        model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));

    if (!path)
    {
        GtkTreeSelection *selection =
                gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
        if (gtk_tree_selection_get_selected (selection, NULL, &iter))
        {
            path = gtk_tree_model_get_path (model, &iter);
            gtk_tree_model_get_iter (model, &iter, path);
            gtk_tree_model_get (model, &iter, 0, &item, -1);
            gtk_tree_path_free (path);
        }
    }
    else
    {
        gtk_tree_model_get_iter (model, &iter, path);
        gtk_tree_model_get (model, &iter, 0, &item, -1);
    }

    if (!item)
        return;

    for (l = widgets->widgets; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        WidgetInfo *info;

        widget = l->data;
        info = get_info (widget);

        if (GTK_IS_TOGGLE_BUTTON (widget))
        {
            moo_config_set_bool (MOO_CONFIG (model), item, info->key,
                                 gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)),
                                 TRUE);
        }
        else
        {
            const char *value;
            char *freeme = NULL;

            if (GTK_IS_ENTRY (widget))
            {
                value = gtk_entry_get_text (GTK_ENTRY (widget));
            }
            else
            {
                GtkTextBuffer *buffer;
                GtkTextIter start, end;
                buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
                gtk_text_buffer_get_bounds (buffer, &start, &end);
                freeme = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
                value = freeme;
            }

            if (info->key)
                moo_config_set_value (MOO_CONFIG (model), item, info->key, value, TRUE);
            else
                moo_config_set_item_content (MOO_CONFIG (model), item, value, TRUE);

            g_free (freeme);
        }
    }
}
