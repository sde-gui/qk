/*
 *   moofilelist.c
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#include "config.h"
#include "mooeditplugins.h"
#include "mooedit/mooplugin-macro.h"
#include "mooutils/mooi18n.h"
#include "mooutils/mootype-macros.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mooutils-treeview.h"
#include "mooutils/mooutils-fs.h"
#include <gtk/gtk.h>

#define FILE_LIST_PLUGIN_ID "MooFileList"
#define WindowPlugin FileListWindowPlugin

#define FILE_LIST(model) ((FileList*)model)
#define GROUP_ITEM(itm) ((Group*)itm)
#define FILE_ITEM(itm) ((File*)itm)
#define ITEM(itm) ((Item*)itm)
#define ITEM_IS_FILE(itm) ((itm) && ((Item*)(itm))->type == ITEM_FILE)
#define ITEM_IS_GROUP(itm) ((itm) && ((Item*)(itm))->type == ITEM_GROUP)

#define CONFIG_FILE "file-list-config.xml"
#define ELM_CONFIG "file-list-config"
#define PROP_VERSION "version"
#define VALUE_VERSION "1.0"
#define ELM_GROUP "group"
#define ELM_FILE "file"
#define PROP_NAME "name"
#define PROP_URI "uri"

typedef struct Item Item;
typedef struct Group Group;
typedef struct File File;

enum {
    COLUMN_ITEM,
    COLUMN_TOOLTIP
};

typedef enum {
    ITEM_FILE,
    ITEM_GROUP
} ItemType;

struct Item {
    ItemType type;
    guint ref_count;
};

struct File {
    Item base;
    char *uri; /* real uri or "Untitled" */
    char *display_basename;
    char *display_name;
    MooEdit *doc;
};

struct Group {
    Item base;
    char *name;
};

typedef struct {
    GtkTreeStore base;
    int n_user_items;
    GSList *docs;
} FileList;

typedef struct {
    GtkTreeStoreClass base_class;
} FileListClass;

typedef struct {
    MooPlugin parent;
    guint ui_merge_id;
} FileListPlugin;

typedef struct {
    MooWinPlugin parent;
    FileList *list;
    GtkWidget *treeview;
    GtkTreeViewColumn *column;
    GtkCellRenderer *text_cell;
    guint update_idle;
    gboolean first_time_show;
    char *filename;
} FileListWindowPlugin;

static GdkAtom atom_tree_model_row;
static GdkAtom atom_uri_list;

static GType         item_get_type          (void) G_GNUC_CONST;
static Item         *file_new               (MooEdit    *doc,
                                             const char *uri);
static void          file_update            (File       *file);
static void          file_set_doc           (File       *file,
                                             MooEdit    *doc);
static char         *file_get_uri           (File       *file);
static Item         *item_ref               (Item       *item);
static void          item_unref             (Item       *item);
static const char   *item_get_tooltip       (Item       *item);

static void          file_list_load_config  (FileList   *list,
                                             const char *filename);
static void          file_list_save_config  (FileList   *list,
                                             const char *filename);
static void          file_list_update_docs  (FileList   *list,
                                             GSList     *docs);
static void          file_list_update_doc   (FileList   *list,
                                             MooEdit    *doc);

static void file_list_drag_source_iface_init    (GtkTreeDragSourceIface *iface);
static void file_list_drag_dest_iface_init      (GtkTreeDragDestIface   *iface);

static gboolean drag_source_row_draggable       (GtkTreeDragSource  *drag_source,
                                                 GtkTreePath        *path);
static gboolean drag_source_drag_data_get       (GtkTreeDragSource  *drag_source,
                                                 GtkTreePath        *path,
                                                 GtkSelectionData   *selection_data);
static gboolean drag_source_drag_data_delete    (GtkTreeDragSource  *drag_source,
                                                 GtkTreePath        *path);
static gboolean drag_dest_drag_data_received    (GtkTreeDragDest    *drag_dest,
                                                 GtkTreePath        *dest,
                                                 GtkSelectionData   *selection_data);
static gboolean drag_dest_row_drop_possible     (GtkTreeDragDest    *drag_dest,
                                                 GtkTreePath        *dest_path,
                                                 GtkSelectionData   *selection_data);

MOO_DEFINE_TYPE_STATIC_WITH_CODE (FileList, file_list, GTK_TYPE_TREE_STORE,
                                  G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_SOURCE,
                                                         file_list_drag_source_iface_init)
                                  G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_DEST,
                                                         file_list_drag_dest_iface_init))


static void
file_list_drag_source_iface_init (GtkTreeDragSourceIface *iface)
{
    iface->row_draggable = drag_source_row_draggable;
    iface->drag_data_get = drag_source_drag_data_get;
    iface->drag_data_delete = drag_source_drag_data_delete;
}

static void
file_list_drag_dest_iface_init (GtkTreeDragDestIface *iface)
{
    iface->drag_data_received = drag_dest_drag_data_received;
    iface->row_drop_possible = drag_dest_row_drop_possible;
}


static void
file_list_init (FileList *list)
{
    GType types[2];

    types[COLUMN_ITEM] = item_get_type ();
    types[COLUMN_TOOLTIP] = G_TYPE_STRING;

    gtk_tree_store_set_column_types (GTK_TREE_STORE (list), 2, types);

    list->n_user_items = 0;
    list->docs = NULL;
}

static void
file_list_finalize (GObject *object)
{
    FileList *list = FILE_LIST (object);

    if (list->docs)
    {
        g_critical ("%s: oops", G_STRLOC);
        g_slist_foreach (list->docs, (GFunc) g_object_unref, NULL);
        g_slist_free (list->docs);
    }

    G_OBJECT_CLASS (file_list_parent_class)->finalize (object);
}

static void
file_list_class_init (FileListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = file_list_finalize;

    atom_tree_model_row = gdk_atom_intern_static_string ("GTK_TREE_MODEL_ROW");
    atom_uri_list = gdk_atom_intern_static_string ("text/uri-list");
}

static Item *
get_item_at_iter (FileList    *list,
                  GtkTreeIter *iter)
{
    Item *item = NULL;

    gtk_tree_model_get (GTK_TREE_MODEL (list), iter, COLUMN_ITEM, &item, -1);

    if (item)
        item_unref (item);

    return item;
}

static Item *
get_item_at_path (FileList    *list,
                  GtkTreePath *path)
{
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, path))
        return NULL;
    else
        return get_item_at_iter (list, &iter);
}


static Group *
group_new (const char *name)
{
    Group *grp = _moo_new0 (Group);

    ITEM (grp)->ref_count = 1;
    ITEM (grp)->type = ITEM_GROUP;

    grp->name = g_strdup (name);

    return grp;
}

static void
group_free (Group *grp)
{
    if (grp)
    {
        g_free (grp->name);
        _moo_free (Group, grp);
    }
}

static char *
uri_get_basename (const char *uri)
{
    const char *last_slash = strrchr (uri, '/');
    if (last_slash && last_slash[1])
        return g_strdup (last_slash + 1);
    else
        return g_strdup (uri);
}

static const char *
item_get_tooltip (Item *item)
{
    if (ITEM_IS_FILE (item))
        return FILE_ITEM (item)->display_name;
    else
        return NULL;
}

static char *
file_get_uri (File *file)
{
    if (file->doc)
        return moo_edit_get_uri (file->doc);
    else
        return NULL;
}

static void
file_update (File *file)
{
    if (file->doc)
    {
        g_free (file->uri);
        g_free (file->display_name);
        g_free (file->display_basename);
        file->uri = moo_edit_get_uri (file->doc);
        file->display_name = g_strdup (moo_edit_get_display_name (file->doc));
        file->display_basename = g_strdup (moo_edit_get_display_basename (file->doc));
    }
}

static void
file_set_doc (File    *file,
              MooEdit *doc)
{
    if (file->doc)
        g_object_unref (file->doc);

    file->doc = doc ? g_object_ref (doc) : NULL;
    file_update (file);
}

static Item *
file_new (MooEdit    *doc,
          const char *uri)
{
    File *file = _moo_new0 (File);

    ITEM (file)->ref_count = 1;
    ITEM (file)->type = ITEM_FILE;

    file->doc = doc ? g_object_ref (doc) : NULL;
    file->uri = g_strdup (uri);

    if (doc)
    {
        file->display_name = g_strdup (moo_edit_get_display_name (doc));
        file->display_basename = g_strdup (moo_edit_get_display_basename (doc));
    }
    else if (uri)
    {
        char *filename;

        filename = g_filename_from_uri (uri, NULL, NULL);

        if (filename)
        {
            file->display_name = g_filename_display_name (filename);
            file->display_basename = g_filename_display_basename (filename);
        }
        else
        {
            file->display_name = g_strdup (uri);
            file->display_basename = uri_get_basename (uri);
        }
    }

    return ITEM (file);
}

static void
file_free (File *file)
{
    if (file)
    {
        if (file->doc)
            g_object_unref (file->doc);
        g_free (file->uri);
        g_free (file->display_name);
        g_free (file->display_basename);
        _moo_free (File, file);
    }
}

static Item *
item_ref (Item *item)
{
    if (item)
        item->ref_count += 1;
    return item;
}

static void
item_unref (Item *item)
{
    if (item && !--item->ref_count)
    {
        switch (item->type)
        {
            case ITEM_FILE:
                file_free (FILE_ITEM (item));
                break;
            case ITEM_GROUP:
                group_free (GROUP_ITEM (item));
                break;
        }
    }
}

static GType
item_get_type (void)
{
    static GType type;

    if (G_UNLIKELY (!type))
        type = g_boxed_type_register_static ("MooFileListItem",
                                             (GBoxedCopyFunc) item_ref,
                                             (GBoxedFreeFunc) item_unref);

    return type;
}


static char *
get_doc_uri (MooEdit *doc)
{
    char *uri = moo_edit_get_uri (doc);
    if (!uri)
        uri = g_strdup (moo_edit_get_display_name (doc));
    return uri;
}

static gboolean
iter_is_auto (FileList    *list,
              GtkTreeIter *iter)
{
    gboolean retval;
    GtkTreePath *path;

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (list), iter);

    retval = gtk_tree_path_get_depth (path) == 1 &&
             gtk_tree_path_get_indices (path)[0] >= list->n_user_items;

    gtk_tree_path_free (path);
    return retval;
}


static void
check_separator (FileList *list)
{
#if 1
    GtkTreeIter iter;
    int index = 0;

    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (list), &iter, NULL))
    {
        do
        {
            Item *item = get_item_at_iter (list, &iter);

            if (list->n_user_items)
            {
                g_assert (index == list->n_user_items || item != NULL);
                g_assert (index != list->n_user_items || item == NULL);
            }
            else
            {
                g_assert (item != NULL);
            }

            index += 1;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));
    }

    g_assert (list->n_user_items == 0 ||
              list->n_user_items < gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list), NULL));
#endif
}


static gboolean
iter_find_doc (FileList     *list,
               MooEdit      *doc,
               GtkTreeIter  *parent,
               GtkTreeIter  *dest)
{
    GtkTreeIter iter;

    if (parent)
    {
        Item *item = get_item_at_iter (list, parent);

        if (ITEM_IS_FILE (item) && FILE_ITEM (item)->doc == doc)
        {
            *dest = *parent;
            return TRUE;
        }
    }

    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (list), &iter, parent))
        do
        {
            if (iter_find_doc (list, doc, &iter, dest))
                return TRUE;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));

    return FALSE;
}

static gboolean
file_list_find_doc (FileList     *list,
                    MooEdit      *doc,
                    GtkTreeIter  *dest)
{
    return iter_find_doc (list, doc, NULL, dest);
}

static gboolean
iter_find_uri (FileList     *list,
               const char   *uri,
               GtkTreeIter  *parent,
               GtkTreeIter  *dest)
{
    GtkTreeIter iter;

    if (parent)
    {
        Item *item = get_item_at_iter (list, parent);

        if (ITEM_IS_FILE (item) &&
            FILE_ITEM (item)->uri &&
            strcmp (FILE_ITEM (item)->uri, uri) == 0)
        {
            *dest = *parent;
            return TRUE;
        }
    }

    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (list), &iter, parent))
        do
        {
            if (iter_find_uri (list, uri, &iter, dest))
                return TRUE;
        }
        while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));

    return FALSE;
}

static gboolean
file_list_find_uri (FileList    *list,
                    const char  *uri,
                    GtkTreeIter *iter)
{
    return iter_find_uri (list, uri, NULL, iter);
}

static void
file_list_remove_row (FileList    *list,
                      GtkTreeIter *iter)
{
    GtkTreeIter parent;
    gboolean last_user_item = FALSE;

    if (!gtk_tree_model_iter_parent (GTK_TREE_MODEL (list), &parent, iter) &&
        !iter_is_auto (list, iter))
    {
        g_assert (list->n_user_items > 0);
        list->n_user_items -= 1;
        last_user_item = list->n_user_items == 0;
    }

    gtk_tree_store_remove (GTK_TREE_STORE (list), iter);

    if (last_user_item)
    {
        GtkTreeIter sep;
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list), &sep, NULL, 0);
        gtk_tree_store_remove (GTK_TREE_STORE (list), &sep);
    }

    check_separator (list);
}

static void
file_list_append_row (FileList *list,
                      Item     *item)
{
    GtkTreeIter iter;
    gtk_tree_store_append (GTK_TREE_STORE (list), &iter, NULL);
    gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                        COLUMN_ITEM, item,
                        COLUMN_TOOLTIP, item_get_tooltip (item),
                        -1);
}

static void
file_list_insert_row (FileList    *list,
                      Item        *item,
                      GtkTreeIter *iter,
                      GtkTreeIter *parent_iter,
                      int          index)
{
    gboolean first_user_item = FALSE;

    if (index < 0)
    {
        if (!parent_iter)
            index = list->n_user_items;
        else
            index = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list),
                                                    parent_iter);
    }

    if (!parent_iter)
    {
        g_assert (index <= list->n_user_items);
        list->n_user_items += 1;
        first_user_item = list->n_user_items == 1;
    }

    gtk_tree_store_insert_with_values (GTK_TREE_STORE (list),
                                       iter, parent_iter, index,
                                       COLUMN_ITEM, item,
                                       COLUMN_TOOLTIP, item_get_tooltip (item),
                                       -1);

    if (first_user_item)
    {
        GtkTreeIter sep;
        gtk_tree_store_insert (GTK_TREE_STORE (list), &sep, NULL, list->n_user_items);
        gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list), iter, parent_iter, index);
    }

    check_separator (list);
}

static void
doc_filename_changed (MooEdit    *doc,
                      G_GNUC_UNUSED const char *new_filename,
                      FileList   *list)
{
    file_list_update_doc (list, doc);
}

static void
file_list_update_doc (FileList *list,
                      MooEdit  *doc)
{
    char *new_uri;

    new_uri = get_doc_uri (doc);

    if (g_slist_find (list->docs, doc))
    {
        GtkTreeIter iter;
        Item *item;

        if (!file_list_find_doc (list, doc, &iter))
        {
            g_signal_handlers_disconnect_by_func (doc,
                                                  (gpointer) doc_filename_changed,
                                                  list);
            list->docs = g_slist_remove (list->docs, doc);
            g_object_unref (doc);
            file_list_update_doc (list, doc);
            return;
        }

        item = get_item_at_iter (list, &iter);
        g_return_if_fail (ITEM_IS_FILE (item));
        g_return_if_fail (FILE_ITEM (item)->doc == doc);

        if (!FILE_ITEM (item)->uri || strcmp (new_uri, FILE_ITEM (item)->uri) != 0)
        {
            if (iter_is_auto (list, &iter))
            {
                GtkTreeIter new_iter;

                if (file_list_find_uri (list, new_uri, &new_iter))
                {
                    Item *new_item;

                    new_item = get_item_at_iter (list, &new_iter);
                    g_return_if_fail (ITEM_IS_FILE (new_item));

                    if (FILE_ITEM (new_item)->doc)
                    {
                        MooEdit *old_doc = FILE_ITEM (new_item)->doc;

                        g_signal_handlers_disconnect_by_func (old_doc,
                                                              (gpointer) doc_filename_changed,
                                                              list);
                        list->docs = g_slist_remove (list->docs, old_doc);
                        g_object_unref (old_doc);
                    }

                    file_set_doc (FILE_ITEM (new_item), doc);

                    gtk_tree_store_set (GTK_TREE_STORE (list), &new_iter,
                                        COLUMN_TOOLTIP, item_get_tooltip (new_item), -1);
                    file_list_remove_row (list, &iter);
                }
                else
                {
                    file_update (FILE_ITEM (item));
                    gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                                        COLUMN_TOOLTIP, item_get_tooltip (item),
                                        -1);
                }
            }
            else
            {
                file_set_doc (FILE_ITEM (item), NULL);
                gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                                    COLUMN_TOOLTIP, item_get_tooltip (item), -1);

                item = file_new (doc, new_uri);
                file_list_append_row (list, item);
                item_unref (item);
            }
        }
    }
    else
    {
        GtkTreeIter iter;

        if (file_list_find_uri (list, new_uri, &iter))
        {
            Item *item = get_item_at_iter (list, &iter);
            g_return_if_fail (ITEM_IS_FILE (item));

            if (FILE_ITEM (item)->doc)
            {
                MooEdit *old_doc = FILE_ITEM (item)->doc;
                g_signal_handlers_disconnect_by_func (old_doc,
                                                      (gpointer) doc_filename_changed,
                                                      list);
                list->docs = g_slist_remove (list->docs, old_doc);
                g_object_unref (old_doc);
            }

            file_set_doc (FILE_ITEM (item), doc);
            gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                                COLUMN_TOOLTIP, item_get_tooltip (item), -1);
        }
        else
        {
            Item *item = file_new (doc, new_uri);
            file_list_append_row (list, item);
            item_unref (item);
        }

        g_signal_connect (doc, "filename-changed",
                          G_CALLBACK (doc_filename_changed),
                          list);
        list->docs = g_slist_prepend (list->docs, g_object_ref (doc));
    }

    g_free (new_uri);
}

static void
file_list_remove_doc (FileList *list,
                      MooEdit  *doc)
{
    GtkTreeIter iter;
    Item *item;

    if (!file_list_find_doc (list, doc, &iter))
    {
        g_critical ("%s: oops", G_STRLOC);
        return;
    }

    item = get_item_at_iter (list, &iter);
    g_return_if_fail (ITEM_IS_FILE (item));

    if (iter_is_auto (list, &iter))
    {
        file_list_remove_row (list, &iter);
    }
    else
    {
        file_set_doc (FILE_ITEM (item), NULL);
        gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                            COLUMN_TOOLTIP, item_get_tooltip (item), -1);
    }

    g_signal_handlers_disconnect_by_func (doc,
                                          (gpointer) doc_filename_changed,
                                          list);
    list->docs = g_slist_remove (list->docs, doc);
    g_object_unref (doc);
}


static int
compare_items (int       *p1,
               int       *p2,
               GPtrArray *items)
{
    Item *item1, *item2;
    char *s1, *s2;
    int retval;

    item1 = items->pdata[*p1];
    item2 = items->pdata[*p2];

    if (!item1)
        return !item2 ? 0 : -1;
    if (!item2)
        return 1;

    s1 = g_utf8_collate_key_for_filename (FILE_ITEM (item1)->display_basename, -1);
    s2 = g_utf8_collate_key_for_filename (FILE_ITEM (item2)->display_basename, -1);

    retval = strcmp (s1, s2);

    g_free (s2);
    g_free (s1);
    return retval;
}

static void
file_list_sort (FileList *list)
{
    GPtrArray *items;
    GArray *indices;
    GtkTreeIter iter;
    int i;

    if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list),
                                        &iter, NULL,
                                        list->n_user_items))
        return;

    items = g_ptr_array_new ();

    indices = g_array_new (FALSE, FALSE, sizeof (int));
    for (i = 0; i < list->n_user_items; ++i)
        g_array_append_val (indices, i);

    do
    {
        int index = indices->len - list->n_user_items;
        Item *item = get_item_at_iter (list, &iter);
        g_array_append_val (indices, index);
        g_ptr_array_add (items, item);
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));

    g_qsort_with_data ((int*)indices->data + list->n_user_items,
                       indices->len - list->n_user_items,
                       sizeof (int),
                       (GCompareDataFunc) compare_items,
                       items);

    for (i = list->n_user_items; i < (int) indices->len; ++i)
    {
        int real_index = g_array_index (indices, int, i) + list->n_user_items;
        g_array_index (indices, int, i) = real_index;
    }

    gtk_tree_store_reorder (GTK_TREE_STORE (list), NULL, (int*) indices->data);

    g_array_free (indices, TRUE);
    g_ptr_array_free (items, TRUE);
}

static void
file_list_update_docs (FileList *list,
                       GSList   *docs)
{
    GSList *l;
    GSList *old_docs;

    for (l = docs; l != NULL; l = l->next)
        file_list_update_doc (list, l->data);

    old_docs = g_slist_copy (list->docs);
    for (l = docs; l != NULL; l = l->next)
        old_docs = g_slist_remove (old_docs, l->data);
    for (l = old_docs; l != NULL; l = l->next)
        file_list_remove_doc (list, l->data);

    file_list_sort (list);

    g_slist_free (old_docs);
}

static void
file_list_self_update (FileList *list)
{
    GSList *docs = g_slist_copy (list->docs);
    file_list_update_docs (list, docs);
    g_slist_free (docs);
}


static void
parse_node (FileList      *list,
            MooMarkupNode *elm,
            GtkTreeIter   *parent,
            const char    *filename)
{
    if (strcmp (elm->name, ELM_GROUP) == 0)
    {
        GtkTreeIter iter;
        const char *name;
        MooMarkupNode *child;
        Group *group;

        if (!(name = moo_markup_get_prop (elm, PROP_NAME)) || !name[0])
        {
            g_critical ("%s: in file %s, element %s: name missing",
                        G_STRLOC, filename, elm->name);
            return;
        }

        group = group_new (name);
        file_list_insert_row (list, ITEM (group), &iter, parent, -1);
        item_unref (ITEM (group));

        for (child = elm->children; child != NULL; child = child->next)
            if (MOO_MARKUP_IS_ELEMENT (child))
                parse_node (list, child, &iter, filename);
    }
    else if (strcmp (elm->name, ELM_FILE) == 0)
    {
        const char *uri;
        Item *item;
        GtkTreeIter iter;

        if (!(uri = moo_markup_get_prop (elm, PROP_URI)) || !uri[0])
        {
            g_critical ("%s: in file %s, element %s: uri missing",
                        G_STRLOC, filename, elm->name);
            return;
        }

        item = file_new (NULL, uri);
        file_list_insert_row (list, item, &iter, parent, -1);
        item_unref (item);
    }
    else
    {
        g_critical ("%s: in file %s: unexpected element '%s'",
                    G_STRLOC, filename, elm->name);
    }
}

static void
file_list_load_config (FileList   *list,
                       const char *filename)
{
    MooMarkupDoc *doc;
    GError *error = NULL;
    MooMarkupNode *root, *node;
    const char *version;

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
        return;

    doc = moo_markup_parse_file (filename, &error);

    if (!doc)
    {
        g_critical ("%s: could not open file %s: %s",
                    G_STRFUNC, filename, error ? error->message : "");
        g_error_free (error);
        return;
    }

    if (!(root = moo_markup_get_root_element (doc, ELM_CONFIG)))
    {
        g_critical ("%s: in file %s: missing element '%s'",
                    G_STRFUNC, filename, ELM_CONFIG);
        goto out;
    }

    if (!(version = moo_markup_get_prop (root, PROP_VERSION)) ||
        strcmp (version, VALUE_VERSION) != 0)
    {
        g_critical ("%s: in file %s: invalid version '%s'",
                    G_STRFUNC, filename, VALUE_VERSION);
        goto out;
    }

    for (node = root->children; node != NULL; node = node->next)
    {
        if (!MOO_MARKUP_IS_ELEMENT (node))
            continue;

        parse_node (list, node, NULL, filename);
    }

out:
    moo_markup_doc_unref (doc);
}

static void
format_item (FileList    *list,
             GtkTreeIter *iter,
             GString     *buffer,
             guint        indent)
{
    Item *item;
    char *indent_s;

    item = get_item_at_iter (list, iter);
    indent_s = g_strnfill (indent, ' ');

    if (ITEM_IS_FILE (item))
    {
        char *uri_escaped = g_markup_escape_text (FILE_ITEM (item)->uri, -1);
        if (uri_escaped)
            g_string_append_printf (buffer, "%s<%s %s=\"%s\"/>\n",
                                    indent_s, ELM_FILE, PROP_URI, uri_escaped);
        g_free (uri_escaped);
    }
    else if (ITEM_IS_GROUP (item))
    {
        char *name_escaped = g_markup_escape_text (GROUP_ITEM (item)->name, -1);

        if (name_escaped)
        {
            GtkTreeIter child;

            if (gtk_tree_model_iter_children (GTK_TREE_MODEL (list), &child, iter))
            {
                g_string_append_printf (buffer, "%s<%s %s=\"%s\">\n",
                                        indent_s, ELM_GROUP, PROP_NAME, name_escaped);

                do
                {
                    format_item (list, &child, buffer, indent + 2);
                }
                while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &child));

                g_string_append_printf (buffer, "%s</%s>\n", indent_s, ELM_GROUP);
            }
            else
            {
                g_string_append_printf (buffer, "%s<%s %s=\"%s\"/>\n",
                                        indent_s, ELM_GROUP, PROP_NAME, name_escaped);
            }
        }

        g_free (name_escaped);
    }
    else
    {
        g_critical ("%s: oops", G_STRLOC);
    }

    g_free (indent_s);
}

static void
file_list_save_config (FileList   *list,
                       const char *filename)
{
    GtkTreeIter iter;
    GString *buffer;
    GError *error = NULL;

    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list), &iter))
    {
        _moo_unlink (filename);
        return;
    }

    buffer = g_string_new ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    g_string_append (buffer, "<" ELM_CONFIG " " PROP_VERSION "=\"" VALUE_VERSION "\">\n");

    do
    {
        if (iter_is_auto (list, &iter))
            break;

        format_item (list, &iter, buffer, 2);
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &iter));

    g_string_append (buffer, "</" ELM_CONFIG ">\n");

    if (!g_file_set_contents (filename, buffer->str, buffer->len, &error))
    {
        g_critical ("%s: could not save file %s: %s",
                    G_STRLOC, filename, error ? error->message : "");
        g_error_free (error);
    }

    g_string_free (buffer, TRUE);
}


static GtkTreePath *
file_list_add_group (FileList    *list,
                     GtkTreePath *path)
{
    GtkTreePath *parent = NULL;
    GtkTreeIter parent_iter, new_iter;
    GtkTreeIter *piter = NULL;
    Group *group;
    int index = -1;

    if (path)
    {
        GtkTreeIter iter;
        Item *item;

        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, path);
        item = get_item_at_iter (list, &iter);

        if (ITEM_IS_GROUP (item))
        {
            parent = gtk_tree_path_copy (path);
            index = 0;
        }
        else if (!iter_is_auto (list, &iter))
        {
            GtkTreeIter parent_iter;

            if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (list), &parent_iter, &iter))
            {
                parent = gtk_tree_path_copy (path);
                gtk_tree_path_up (parent);
                index = gtk_tree_path_get_indices (path)[gtk_tree_path_get_depth (path)-1] + 1;
            }
            else
            {
                index = gtk_tree_path_get_indices (path)[0] + 1;
            }
        }
    }

    if (parent)
    {
        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &parent_iter, parent);
        piter = &parent_iter;
    }

    if (index < 0)
        index = list->n_user_items;

    group = group_new ("Group");
    file_list_insert_row (list, ITEM (group), &new_iter, piter, index);
    item_unref (ITEM (group));

    return gtk_tree_model_get_path (GTK_TREE_MODEL (list), &new_iter);
}

static void
file_list_remove_item (FileList    *list,
                       GtkTreePath *path)
{
    GtkTreeIter iter;
    Item *item;

    if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, path))
        return;

    if (iter_is_auto (list, &iter))
        return;

    item = get_item_at_iter (list, &iter);
    item_ref (item);

    file_list_remove_row (list, &iter);

    if (ITEM_IS_FILE (item) && FILE_ITEM (item)->doc)
        file_list_append_row (list, item);

    /* XXX */
    file_list_self_update (list);

    item_unref (item);
}

static void
file_list_remove_items (FileList *list,
                        GList    *paths)
{
    GSList *rows = NULL;

    while (paths)
    {
        GtkTreeRowReference *row;

        row = gtk_tree_row_reference_new (GTK_TREE_MODEL (list), paths->data);

        if (row)
            rows = g_slist_prepend (rows, row);

        paths = paths->next;
    }

    rows = g_slist_reverse (rows);

    while (rows)
    {
        GtkTreePath *path = NULL;

        if (gtk_tree_row_reference_valid (rows->data))
            path = gtk_tree_row_reference_get_path (rows->data);

        if (path)
        {
            file_list_remove_item (list, path);
            gtk_tree_path_free (path);
        }

        gtk_tree_row_reference_free (rows->data);
        rows = g_slist_delete_link (rows, rows);
    }
}


static gboolean
drag_source_row_draggable (G_GNUC_UNUSED GtkTreeDragSource *drag_source,
                           G_GNUC_UNUSED GtkTreePath       *path)
{
    return TRUE;
}

static gboolean
drag_source_drag_data_get (GtkTreeDragSource *drag_source,
                           GtkTreePath       *path,
                           GtkSelectionData  *selection_data)
{
//     char *name = gdk_atom_name (selection_data->target);
//     g_print ("%s: %s\n", G_STRFUNC, name);
//     g_free (name);

    if (selection_data->target == atom_tree_model_row)
    {
        gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL (drag_source), path);
        return TRUE;
    }
    else if (selection_data->target == atom_uri_list)
    {
        Item *item;
        char *uris[2] = {NULL, NULL};

        item = get_item_at_path (FILE_LIST (drag_source), path);

        if (ITEM_IS_FILE (item))
            uris[0] = file_get_uri (FILE_ITEM (item));

        if (uris[0])
        {
            gtk_selection_data_set_uris (selection_data, (char**) uris);
            g_free (uris[0]);
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean
drag_source_drag_data_delete (G_GNUC_UNUSED GtkTreeDragSource *drag_source,
                              G_GNUC_UNUSED GtkTreePath       *path)
{
    return FALSE;
}

static void
copy_row_children (FileList    *list,
                   GtkTreeIter *source,
                   GtkTreeIter *dest)
{
    GtkTreeIter child;

    if (gtk_tree_model_iter_children (GTK_TREE_MODEL (list), &child, source))
    do
    {
        GtkTreeIter iter;
        Item *item = get_item_at_iter (list, &child);
        gtk_tree_store_append (GTK_TREE_STORE (list), &iter, dest);
        gtk_tree_store_set (GTK_TREE_STORE (list), &iter,
                            COLUMN_ITEM, item,
                            COLUMN_TOOLTIP, item_get_tooltip (item),
                            -1);
        copy_row_children (list, &child, &iter);
    }
    while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list), &child));
}

static void
copy_row (FileList    *list,
          GtkTreePath *source,
          GtkTreePath *parent,
          int          index)
{
    GtkTreeRowReference *source_row;
    GtkTreeIter iter, parent_iter;
    GtkTreeIter *piter = NULL;
    Item *item;

    gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);
    item = get_item_at_iter (list, &iter);
    g_return_if_fail (item != NULL);

    source_row = gtk_tree_row_reference_new (GTK_TREE_MODEL (list), source);

    if (parent)
    {
        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &parent_iter, parent);
        piter = &parent_iter;
    }

    file_list_insert_row (list, item, &iter, piter, index);

    parent_iter = iter;
    source = gtk_tree_row_reference_get_path (source_row);
    gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);
    copy_row_children (list, &iter, &parent_iter);
    gtk_tree_path_free (source);
}

static gboolean
move_row (FileList    *list,
          GtkTreePath *source,
          GtkTreePath *parent,
          int          index)
{
    GtkTreePath *source_parent = NULL;
    gboolean same_parent = FALSE;
    GtkTreeIter iter;
    Item *item;

    gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);

    item = get_item_at_iter (list, &iter);

    if (ITEM_IS_FILE (item) && FILE_ITEM (item)->doc)
    {
        char *uri = file_get_uri (FILE_ITEM (item));

        if (!uri)
            return FALSE;

        g_free (uri);
    }

    if (gtk_tree_path_get_depth (source) > 1)
    {
        source_parent = gtk_tree_path_copy (source);
        gtk_tree_path_up (source_parent);
    }

    if (!source_parent && !parent)
        same_parent = TRUE;
    else if (source_parent && parent && gtk_tree_path_compare (source_parent, parent) == 0)
        same_parent = TRUE;

    if (same_parent)
    {
        GtkTreeIter *piter = NULL;
        gboolean first_user = FALSE;

        if (parent)
        {
            gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, parent);
            piter = &iter;
        }
        else if (iter_is_auto (list, &iter))
        {
            list->n_user_items += 1;
            first_user = list->n_user_items == 1;
        }

        if (index == gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list), piter))
        {
            gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);
            gtk_tree_store_move_before (GTK_TREE_STORE (list), &iter, NULL);
        }
        else
        {
            GtkTreeIter ch_iter;
            gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (list), &ch_iter, piter, index);
            gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);
            gtk_tree_store_move_before (GTK_TREE_STORE (list), &iter, &ch_iter);
        }

        if (first_user)
            gtk_tree_store_insert (GTK_TREE_STORE (list), &iter,
                                   NULL, list->n_user_items);
    }
    else
    {
        GtkTreeRowReference *row;
        row = gtk_tree_row_reference_new (GTK_TREE_MODEL (list), source);
        copy_row (list, source, parent, index);
        source = gtk_tree_row_reference_get_path (row);
        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, source);

        file_list_remove_row (list, &iter);

        gtk_tree_path_free (source);
        gtk_tree_row_reference_free (row);
    }

    if (source_parent)
        gtk_tree_path_free (source_parent);

    check_separator (list);

    return TRUE;
}

static gboolean
add_row_from_uri (FileList    *list,
                  const char  *uri,
                  GtkTreePath *parent,
                  int          index)
{
    Item *item;
    GtkTreeIter iter, dummy;
    GtkTreeIter *piter = NULL;

    if (parent)
    {
        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, parent);
        piter = &iter;
    }

    item = file_new (NULL, uri);
    file_list_insert_row (list, item, &dummy, piter, index);
    item_unref (item);

    return TRUE;
}

static gboolean
find_drop_destination (FileList     *list,
                       GtkTreePath  *dest,
                       GtkTreePath **parent_path,
                       int          *index)
{
    int n_children;
    Group *parent_group = NULL;
    GtkTreeIter parent_iter;

    *parent_path = NULL;
    *index = 0;

    if (gtk_tree_path_get_depth (dest) > 1)
    {
        GtkTreePath *parent;
        Item *parent_item;

        parent = gtk_tree_path_copy (dest);
        gtk_tree_path_up (parent);

        if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &parent_iter, parent))
        {
            gtk_tree_path_free (parent);
            return FALSE;
        }

        parent_item = get_item_at_iter (list, &parent_iter);

        if (ITEM_IS_GROUP (parent_item))
        {
            parent_group = GROUP_ITEM (parent_item);
            *index = gtk_tree_path_get_indices (dest)[gtk_tree_path_get_depth (dest) - 1];
        }
        else if (gtk_tree_path_get_depth (parent) > 1)
        {
            *index = gtk_tree_path_get_indices (parent)[gtk_tree_path_get_depth (parent) - 1];
            gtk_tree_path_up (parent);

            if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &parent_iter, parent))
            {
                gtk_tree_path_free (parent);
                g_return_val_if_reached (FALSE);
            }

            parent_item = get_item_at_iter (list, &parent_iter);

            if (!ITEM_IS_GROUP (parent_item))
            {
                gtk_tree_path_free (parent);
                g_return_val_if_reached (FALSE);
            }

            parent_group = GROUP_ITEM (parent_item);
        }
        else
        {
            parent_group = NULL;
            *index = gtk_tree_path_get_indices (parent)[0];
        }

        gtk_tree_path_free (parent);
    }
    else
    {
        *index = gtk_tree_path_get_indices (dest)[0];
    }

    if (parent_group)
    {
        n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list), &parent_iter);
        *parent_path = gtk_tree_model_get_path (GTK_TREE_MODEL (list), &parent_iter);
    }
    else
    {
        n_children = list->n_user_items;
    }

    *index = CLAMP (*index, 0, n_children);

    return TRUE;
}

static gboolean
path_is_descendant (GtkTreePath *path,
                    GtkTreePath *ancestor)
{
    return gtk_tree_path_compare (path, ancestor) == 0 ||
            gtk_tree_path_is_descendant (path, ancestor);
}

static gboolean
drop_uris (FileList     *list,
           GtkTreePath  *dest,
           char        **uris)
{
    GtkTreePath *parent_path = NULL;
    int index = 0;

    if (!find_drop_destination (list, dest, &parent_path, &index))
        return FALSE;

    for ( ; uris && *uris; ++uris)
    {
        GtkTreeIter iter;

        if (file_list_find_uri (list, *uris, &iter))
        {
            GtkTreePath *source = gtk_tree_model_get_path (GTK_TREE_MODEL (list), &iter);

            if (parent_path && path_is_descendant (parent_path, source))
            {
                gtk_tree_path_free (source);
                continue;
            }

            if (move_row (list, source, parent_path, index))
                index += 1;

            gtk_tree_path_free (source);
        }
        else
        {
            if (add_row_from_uri (list, *uris, parent_path, index))
                index += 1;
        }
    }

    if (parent_path)
        gtk_tree_path_free (parent_path);

    return TRUE;
}

static gboolean
drop_tree_model_row (FileList    *list,
                     GtkTreePath *dest,
                     GtkTreePath *source)
{
    GtkTreePath *parent_path = NULL;
    int index;
    gboolean retval;

    if (!get_item_at_path (list, source))
        return FALSE;

    if (!find_drop_destination (list, dest, &parent_path, &index))
        return FALSE;

    if (parent_path && path_is_descendant (parent_path, source))
    {
        gtk_tree_path_free (parent_path);
        return FALSE;
    }

    retval = move_row (list, source, parent_path, index);

    if (parent_path)
        gtk_tree_path_free (parent_path);

    return retval;
}

static gboolean
drag_dest_drag_data_received (GtkTreeDragDest  *drag_dest,
                              GtkTreePath      *dest,
                              GtkSelectionData *selection_data)
{
    if (selection_data->target == atom_tree_model_row)
    {
        GtkTreePath *path = NULL;
        gboolean retval;

        if (!gtk_tree_get_row_drag_data (selection_data, NULL, &path))
            return FALSE;

        retval = drop_tree_model_row (FILE_LIST (drag_dest), dest, path);

        gtk_tree_path_free (path);
        return retval;
    }
    else if (selection_data->target == atom_uri_list)
    {
        char **uris;
        gboolean retval;

        if (!(uris = gtk_selection_data_get_uris (selection_data)))
            return FALSE;

        retval = drop_uris (FILE_LIST (drag_dest), dest, uris);

        g_strfreev (uris);
        return retval;
    }

    return FALSE;
}

static gboolean
drag_dest_row_drop_possible (G_GNUC_UNUSED GtkTreeDragDest  *drag_dest,
                             G_GNUC_UNUSED GtkTreePath      *dest_path,
                             G_GNUC_UNUSED GtkSelectionData *selection_data)
{
    return TRUE;
}


static gboolean
row_separator_func (GtkTreeModel *model,
                    GtkTreeIter  *iter)
{
    Item *item = get_item_at_iter (FILE_LIST (model), iter);
    return item == NULL;
}

static void
pixbuf_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                  GtkCellRenderer   *cell,
                  GtkTreeModel      *model,
                  GtkTreeIter       *iter)
{
    Item *item = get_item_at_iter (FILE_LIST (model), iter);
    if (ITEM_IS_GROUP (item))
        g_object_set (cell, "stock-id", GTK_STOCK_DIRECTORY, NULL);
    else if (ITEM_IS_FILE (item))
        g_object_set (cell, "stock-id", GTK_STOCK_FILE, NULL);
}

static void
text_data_func (G_GNUC_UNUSED GtkTreeViewColumn *column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *model,
                GtkTreeIter       *iter)
{
    Item *item = get_item_at_iter (FILE_LIST (model), iter);

    if (ITEM_IS_GROUP (item))
        g_object_set (cell, "text", GROUP_ITEM (item)->name, NULL);
    else if (ITEM_IS_FILE (item))
        g_object_set (cell, "text", FILE_ITEM (item)->display_basename, NULL);
}

static void
text_cell_edited (GtkCellRenderer *cell,
                  const char      *path_string,
                  const char      *new_text,
                  WindowPlugin    *plugin)
{
    GtkTreeIter iter;
    Item *item;
    GtkTreePath *path;

    g_object_set (cell, "editable", FALSE, NULL);

    path = gtk_tree_path_new_from_string (path_string);
    g_return_if_fail (path != NULL);

    gtk_tree_model_get_iter (GTK_TREE_MODEL (plugin->list), &iter, path);
    item = get_item_at_iter (plugin->list, &iter);
    g_return_if_fail (ITEM_IS_GROUP (item));

    MOO_ASSIGN_STRING (GROUP_ITEM (item)->name, new_text);
    gtk_tree_model_row_changed (GTK_TREE_MODEL (plugin->list), path, &iter);

    gtk_tree_path_free (path);
}

static void
text_cell_editing_canceled (GtkCellRenderer *cell)
{
    g_object_set (cell, "editable", FALSE, NULL);
}

static void
start_edit (WindowPlugin *plugin,
            GtkTreePath  *path)
{
    g_object_set (plugin->text_cell, "editable", TRUE, NULL);
    gtk_tree_view_set_cursor_on_cell (GTK_TREE_VIEW (plugin->treeview),
                                      path, plugin->column,
                                      plugin->text_cell,
                                      TRUE);
}

static GList *
get_selected_rows (WindowPlugin *plugin)
{
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (plugin->treeview));
    return gtk_tree_selection_get_selected_rows (selection, NULL);
}

static void
path_list_free (GList *paths)
{
    g_list_foreach (paths, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (paths);
}

static void
rename_activated (G_GNUC_UNUSED GtkWidget *menuitem,
                  WindowPlugin *plugin)
{
    GList *selected;
    Item *item;

    selected = get_selected_rows (plugin);
    if (!selected || selected->next)
    {
        path_list_free (selected);
        g_return_if_fail (selected && !selected->next);
    }

    item = get_item_at_path (plugin->list, selected->data);
    if (!ITEM_IS_GROUP (item))
    {
        path_list_free (selected);
        g_return_if_fail (ITEM_IS_GROUP (item));
    }

    start_edit (plugin, selected->data);

    path_list_free (selected);
}

static void
add_group_activated (G_GNUC_UNUSED GtkWidget *menuitem,
                     WindowPlugin *plugin)
{
    GtkTreePath *new_path, *path;
    GList *selected;

    selected = get_selected_rows (plugin);
    if (selected)
        path = g_list_last (selected)->data;
    else
        path = NULL;

    new_path = file_list_add_group (plugin->list, path);

    if (new_path)
    {
        GtkTreeIter iter, parent;

        gtk_tree_model_get_iter (GTK_TREE_MODEL (plugin->list), &iter, new_path);

        if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (plugin->list), &parent, &iter))
        {
            GtkTreePath *parent_path = gtk_tree_model_get_path (GTK_TREE_MODEL (plugin->list),
                                                                &parent);
            if (!gtk_tree_view_row_expanded (GTK_TREE_VIEW (plugin->treeview), parent_path))
                gtk_tree_view_expand_row (GTK_TREE_VIEW (plugin->treeview), parent_path, FALSE);
            gtk_tree_path_free (parent_path);
        }

        start_edit (plugin, new_path);
        gtk_tree_path_free (new_path);
    }

    path_list_free (selected);
}

static void
remove_activated (G_GNUC_UNUSED GtkWidget *menuitem,
                  WindowPlugin *plugin)
{
    GList *selected;
    selected = get_selected_rows (plugin);
    file_list_remove_items (plugin->list, selected);
    path_list_free (selected);
}

static void
open_file (WindowPlugin *plugin,
           GtkTreePath  *path)
{
    Item *item;

    item = get_item_at_path (plugin->list, path);
    g_return_if_fail (item != NULL);

    if (ITEM_IS_FILE (item))
    {
        if (FILE_ITEM (item)->doc)
        {
            moo_editor_set_active_doc (moo_editor_instance (),
                                       FILE_ITEM (item)->doc);
            gtk_widget_grab_focus (GTK_WIDGET (FILE_ITEM (item)->doc));
        }
        else
        {
            moo_editor_open_uri (moo_editor_instance (),
                                 MOO_WIN_PLUGIN (plugin)->window,
                                 NULL,
                                 FILE_ITEM (item)->uri,
                                 NULL);
        }
    }
    else if (ITEM_IS_GROUP (item))
    {
        GtkTreeIter iter, child;

        gtk_tree_model_get_iter (GTK_TREE_MODEL (plugin->list), &iter, path);

        if (gtk_tree_model_iter_children (GTK_TREE_MODEL (plugin->list), &child, &iter))
        {
            do
            {
                GtkTreePath *child_path;
                child_path = gtk_tree_model_get_path (GTK_TREE_MODEL (plugin->list), &child);
                open_file (plugin, child_path);
                gtk_tree_path_free (child_path);
            }
            while (gtk_tree_model_iter_next (GTK_TREE_MODEL (plugin->list), &child));
        }
    }
    else
    {
        g_return_if_reached ();
    }
}

static void
open_activated (G_GNUC_UNUSED GtkWidget *menuitem,
                WindowPlugin *plugin)
{
    GList *selected, *l;

    selected = get_selected_rows (plugin);

    for (l = selected; l != NULL; l = l->next)
        open_file (plugin, l->data);

    path_list_free (selected);
}

static gboolean
can_open (G_GNUC_UNUSED FileList *list,
          GList *paths)
{
    /* XXX */
    return paths != NULL;
}

static gboolean
can_remove (FileList *list,
            GList    *paths)
{
    while (paths)
    {
        GtkTreeIter iter;
        GtkTreePath *path = paths->data;
        gtk_tree_model_get_iter (GTK_TREE_MODEL (list), &iter, path);
        if (!iter_is_auto (list, &iter))
            return TRUE;
        paths = paths->next;
    }

    return FALSE;
}

static void
popup_menu (WindowPlugin *plugin,
            GList        *selected,
            int           button,
            guint32       time)
{
    GtkWidget *menu, *menuitem;
    GtkTreePath *single_path;
    Item *single_item;

    single_path = (selected && !selected->next) ? selected->data : NULL;
    single_item = single_path ? get_item_at_path (plugin->list, single_path) : NULL;

    menu = gtk_menu_new ();

    if (can_open (plugin->list, selected))
    {
        menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
        g_signal_connect (menuitem, "activate", G_CALLBACK (open_activated), plugin);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    }

    menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ADD, NULL);
    gtk_label_set_text (GTK_LABEL (GTK_BIN (menuitem)->child), "Add Group");
    g_signal_connect (menuitem, "activate", G_CALLBACK (add_group_activated), plugin);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

    if (selected)
    {
        menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_REMOVE, NULL);
        g_signal_connect (menuitem, "activate", G_CALLBACK (remove_activated), plugin);
        gtk_widget_set_sensitive (menuitem, can_remove (plugin->list, selected));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    }

    if (single_item && ITEM_IS_GROUP (single_item))
    {
        menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_EDIT, NULL);
        gtk_label_set_text (GTK_LABEL (GTK_BIN (menuitem)->child), "Rename");
        g_signal_connect (menuitem, "activate", G_CALLBACK (rename_activated), plugin);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
    }

    gtk_widget_show_all (menu);
    gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
}

static gboolean
treeview_button_press (GtkTreeView    *treeview,
                       GdkEventButton *event,
                       WindowPlugin   *plugin)
{
    GtkTreeSelection *selection;
    GtkTreePath *path = NULL;
    GList *selected;
    int x, y;

    if (event->type != GDK_BUTTON_PRESS || event->button != 3)
        return FALSE;

    gtk_tree_view_get_path_at_pos (treeview, event->x, event->y,
                                   &path, NULL, &x, &y);

    selection = gtk_tree_view_get_selection (treeview);

    if (!path)
        gtk_tree_selection_unselect_all (selection);
    else if (!gtk_tree_selection_path_is_selected (selection, path))
        gtk_tree_view_set_cursor (treeview, path, plugin->column, FALSE);

    selected = gtk_tree_selection_get_selected_rows (selection, NULL);
    popup_menu (plugin, selected, event->button, event->time);

    if (path)
        gtk_tree_path_free (path);

    g_list_foreach (selected, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (selected);

    return TRUE;
}

static void
treeview_row_activated (WindowPlugin *plugin,
                        GtkTreePath  *path)
{
    GtkTreeIter iter;
    Item *item;

    gtk_tree_model_get_iter (GTK_TREE_MODEL (plugin->list), &iter, path);
    item = get_item_at_iter (plugin->list, &iter);
    g_return_if_fail (item != NULL);

    if (ITEM_IS_FILE (item))
    {
        if (FILE_ITEM (item)->doc)
        {
            moo_editor_set_active_doc (moo_editor_instance (),
                                       FILE_ITEM (item)->doc);
            gtk_widget_grab_focus (GTK_WIDGET (FILE_ITEM (item)->doc));
        }
        else
        {
            moo_editor_open_uri (moo_editor_instance (),
                                 MOO_WIN_PLUGIN (plugin)->window,
                                 NULL,
                                 FILE_ITEM (item)->uri,
                                 NULL);
        }
    }
}

static void
create_treeview (WindowPlugin *plugin)
{
    GtkTreeSelection *selection;
    GtkCellRenderer *cell;
    GtkTargetEntry targets[] = {
        { (char*) "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 },
        { (char*) "text/uri-list", 0, 0 }
    };

    plugin->treeview = gtk_tree_view_new ();
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (plugin->treeview), FALSE);
    gtk_tree_view_set_row_separator_func (GTK_TREE_VIEW (plugin->treeview),
                                          (GtkTreeViewRowSeparatorFunc) row_separator_func,
                                          NULL, NULL);
#if GTK_CHECK_VERSION (2,12,0)
    gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (plugin->treeview),
                                      COLUMN_TOOLTIP);
#endif

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (plugin->treeview));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

    g_signal_connect (plugin->treeview, "button-press-event",
                      G_CALLBACK (treeview_button_press), plugin);
    g_signal_connect_swapped (plugin->treeview, "row-activated",
                              G_CALLBACK (treeview_row_activated), plugin);

    gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (plugin->treeview),
                                          targets, G_N_ELEMENTS (targets),
                                          GDK_ACTION_COPY | GDK_ACTION_MOVE |
                                            GDK_ACTION_LINK | GDK_ACTION_PRIVATE);
    gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (plugin->treeview),
                                            GDK_BUTTON1_MASK,
                                            targets, G_N_ELEMENTS (targets),
                                            GDK_ACTION_COPY | GDK_ACTION_MOVE |
                                                GDK_ACTION_LINK | GDK_ACTION_PRIVATE);

    plugin->column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (GTK_TREE_VIEW (plugin->treeview), plugin->column);

    _moo_tree_view_setup_expander (GTK_TREE_VIEW (plugin->treeview),
                                   plugin->column);

    cell = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (plugin->column, cell, FALSE);
    gtk_tree_view_column_set_cell_data_func (plugin->column, cell,
                                             (GtkTreeCellDataFunc) pixbuf_data_func,
                                             NULL, NULL);

    plugin->text_cell = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (plugin->column, plugin->text_cell, TRUE);
    gtk_tree_view_column_set_cell_data_func (plugin->column, plugin->text_cell,
                                             (GtkTreeCellDataFunc) text_data_func,
                                             NULL, NULL);
    g_signal_connect (plugin->text_cell, "edited",
                      G_CALLBACK (text_cell_edited), plugin);
    g_signal_connect (plugin->text_cell, "editing-canceled",
                      G_CALLBACK (text_cell_editing_canceled), plugin);
}

static void
create_model (WindowPlugin *plugin)
{
    plugin->list = g_object_new (file_list_get_type (), NULL);

    file_list_load_config (plugin->list, plugin->filename);

    gtk_tree_view_set_model (GTK_TREE_VIEW (plugin->treeview),
                             GTK_TREE_MODEL (plugin->list));
}

static gboolean
do_update (WindowPlugin *plugin)
{
    GSList *docs;

    plugin->update_idle = 0;

    docs = moo_edit_window_list_docs (MOO_WIN_PLUGIN (plugin)->window);
    file_list_update_docs (plugin->list, docs);
    g_slist_free (docs);

    if (plugin->first_time_show)
    {
        plugin->first_time_show = FALSE;
        gtk_tree_view_expand_all (GTK_TREE_VIEW (plugin->treeview));
        _moo_tree_view_select_first (GTK_TREE_VIEW (plugin->treeview));
    }

    return FALSE;
}

static void
queue_update (WindowPlugin *plugin)
{
    if (!plugin->update_idle)
        plugin->update_idle = g_idle_add ((GSourceFunc) do_update,
                                          plugin);
}


static gboolean
file_list_window_plugin_create (WindowPlugin *plugin)
{
    MooPane *pane;
    MooPaneLabel *label;
    GtkWidget *scrolled_window;
    MooEditWindow *window;

    window = MOO_WIN_PLUGIN (plugin)->window;
    g_return_val_if_fail (MOO_IS_EDIT_WINDOW (window), FALSE);

    plugin->filename = moo_get_user_data_file (CONFIG_FILE);

    create_treeview (plugin);
    create_model (plugin);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scrolled_window), plugin->treeview);
    gtk_widget_show_all (scrolled_window);

    label = moo_pane_label_new (GTK_STOCK_DIRECTORY,
                                NULL, _("File List"),
                                _("File List"));
    moo_edit_window_add_pane (window,
                              FILE_LIST_PLUGIN_ID,
                              scrolled_window, label,
                              MOO_PANE_POS_RIGHT);
    moo_pane_label_free (label);

    pane = moo_big_paned_find_pane (window->paned,
                                    GTK_WIDGET (scrolled_window), NULL);
    moo_pane_set_drag_dest (pane);

    plugin->first_time_show = TRUE;
    g_signal_connect_swapped (window, "new-doc",
                              G_CALLBACK (queue_update), plugin);
    g_signal_connect_swapped (window, "close-doc",
                              G_CALLBACK (queue_update), plugin);
    queue_update (plugin);

    return TRUE;
}

static void
file_list_window_plugin_destroy (WindowPlugin *plugin)
{
    file_list_save_config (plugin->list, plugin->filename);
    g_free (plugin->filename);

    moo_edit_window_remove_pane (MOO_WIN_PLUGIN (plugin)->window,
                                 FILE_LIST_PLUGIN_ID);

    g_object_unref (plugin->list);

    if (plugin->update_idle)
        g_source_remove (plugin->update_idle);
    g_signal_handlers_disconnect_by_func (MOO_WIN_PLUGIN (plugin)->window,
                                          (gpointer) queue_update,
                                          plugin);
}

static void
show_file_list_cb (MooEditWindow *window)
{
    GtkWidget *pane;
    pane = moo_edit_window_get_pane (window, FILE_LIST_PLUGIN_ID);
    moo_big_paned_present_pane (window->paned, pane);
}

static gboolean
file_list_plugin_init (FileListPlugin *plugin)
{
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);

    g_return_val_if_fail (klass != NULL, FALSE);
    g_return_val_if_fail (editor != NULL, FALSE);

    moo_window_class_new_action (klass, "ShowFileList", NULL,
                                 "display-name", _("File List"),
                                 "label", _("File List"),
                                 "stock-id", GTK_STOCK_DIRECTORY,
                                 "closure-callback", show_file_list_cb,
                                 NULL);

    if (xml)
    {
        plugin->ui_merge_id = moo_ui_xml_new_merge_id (xml);
        moo_ui_xml_add_item (xml, plugin->ui_merge_id,
                             "Editor/Menubar/View/PanesMenu",
                             "ShowFileList", "ShowFileList", -1);
    }

    g_type_class_unref (klass);
    return TRUE;
}


static void
file_list_plugin_deinit (FileListPlugin *plugin)
{
    MooEditor *editor = moo_editor_instance ();
    MooUIXML *xml = moo_editor_get_ui_xml (editor);
    MooWindowClass *klass = g_type_class_ref (MOO_TYPE_EDIT_WINDOW);

    moo_window_class_remove_action (klass, "ShowFileList");

    if (plugin->ui_merge_id)
        moo_ui_xml_remove_ui (xml, plugin->ui_merge_id);
    plugin->ui_merge_id = 0;

    g_type_class_unref (klass);
}


MOO_PLUGIN_DEFINE_INFO (file_list,
                        N_("File List"), N_("List of files"),
                        "Yevgen Muntyan <muntyan@tamu.edu>",
                        MOO_VERSION, NULL)
MOO_WIN_PLUGIN_DEFINE (FileList, file_list)
MOO_PLUGIN_DEFINE_FULL (FileList, file_list,
                        NULL, NULL, NULL, NULL, NULL,
                        file_list_window_plugin_get_type (), 0)


gboolean
_moo_file_list_plugin_init (void)
{
    MooPluginParams params = {FALSE, TRUE};
    return moo_plugin_register (FILE_LIST_PLUGIN_ID,
                                file_list_plugin_get_type (),
                                &file_list_plugin_info,
                                &params);
}
