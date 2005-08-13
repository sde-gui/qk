/*
 *   mooedit/moofoldermodel-private.h
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_FOLDER_MODEL_PRIVATE_H__
#define __MOO_FOLDER_MODEL_PRIVATE_H__

#ifndef MOO_FILE_SYSTEM_COMPILATION
#error "Do not include this file"
#endif

#include <glib.h>
#include <string.h>

G_BEGIN_DECLS

typedef struct _FileList FileList;

struct _FileList {
    GList       *list;                  /* MooFile*, sorted by name */
    int          size;
    GHashTable  *name_to_file;          /* char* -> MooFile* */
    GHashTable  *display_name_to_file;  /* char* -> MooFile* */
    GHashTable  *file_to_link;          /* MooFile* -> GList* */
};


static FileList *file_list_new          (void);
static void      file_list_destroy      (FileList   *flist);

static int       file_list_add          (FileList   *flist,
                                         MooFile    *file);
static int       file_list_remove       (FileList   *flist,
                                         MooFile    *file);

static MooFile  *file_list_nth          (FileList   *flist,
                                         int         index);
static gboolean  file_list_contains     (FileList   *flist,
                                         MooFile    *file);
static int       file_list_position     (FileList   *flist,
                                         MooFile    *file);

static MooFile  *file_list_find_name    (FileList   *flist,
                                         const char *name);
static MooFile  *file_list_find_display_name
                                        (FileList   *flist,
                                         const char *display_name);

static MooFile  *file_list_first        (FileList   *flist);
static MooFile  *file_list_next         (FileList   *flist,
                                         MooFile    *file);

static GList    *_list_insert_sorted    (GList         **list,
                                         int            *list_len,
                                         MooFile        *file,
                                         int            *position);
static void      _list_delete_link      (GList         **list,
                                         GList          *link,
                                         int            *list_len);
static GList    *_list_find             (FileList       *flist,
                                         MooFile        *file,
                                         int            *index);

static void      _hash_table_insert     (FileList       *flist,
                                         MooFile        *file,
                                         GList          *link);
static void      _hash_table_remove     (FileList       *flist,
                                         MooFile        *file);


#ifdef DEBUG
#if 0
#define DEFINE_CHECK_FILE_LIST_INTEGRITY
static void CHECK_FILE_LIST_INTEGRITY (FileList *flist)
{
    GList *link;

    g_assert ((int)g_list_length (flist->list) == flist->size);
    g_assert ((int)g_hash_table_size (flist->name_to_file) == flist->size);
    g_assert ((int)g_hash_table_size (flist->display_name_to_file) == flist->size);
    g_assert ((int)g_hash_table_size (flist->file_to_link) == flist->size);

    for (link = flist->list; link != NULL; link = link->next)
    {
        MooFile *file = link->data;
        g_assert (file != NULL);
        g_assert (file == g_hash_table_lookup (flist->name_to_file,
                moo_file_get_basename (file)));
        g_assert (file == g_hash_table_lookup (flist->display_name_to_file,
                moo_file_get_display_basename (file)));
        g_assert (link == g_hash_table_lookup (flist->file_to_link, file));
    }
}
#endif
#endif /* DEBUG */

#ifndef DEFINE_CHECK_FILE_LIST_INTEGRITY
#define CHECK_FILE_LIST_INTEGRITY(flist)
#endif


static FileList *file_list_new          (void)
{
    FileList *flist = g_new0 (FileList, 1);

    flist->name_to_file =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    flist->display_name_to_file =
            g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    flist->file_to_link = g_hash_table_new (g_direct_hash, g_direct_equal);

    CHECK_FILE_LIST_INTEGRITY (flist);

    return flist;
}


static void      file_list_destroy      (FileList   *flist)
{
    g_return_if_fail (flist != NULL);

    g_hash_table_destroy (flist->display_name_to_file);
    g_hash_table_destroy (flist->name_to_file);
    g_hash_table_destroy (flist->file_to_link);

    g_list_foreach (flist->list, (GFunc)moo_file_unref, NULL);
    g_list_free (flist->list);

    g_free (flist);
}


static int       file_list_add          (FileList   *flist,
                                         MooFile    *file)
{
    int index;
    GList *link;

    link = _list_insert_sorted (&flist->list,
                                &flist->size,
                                moo_file_ref (file),
                                &index);
    _hash_table_insert (flist, file, link);

    CHECK_FILE_LIST_INTEGRITY (flist);

    return index;
}


static int       file_list_remove       (FileList   *flist,
                                         MooFile    *file)
{
    int index;
    GList *link;

    link = _list_find (flist, file, &index);
    g_assert (link != NULL);

    _hash_table_remove (flist, file);
    _list_delete_link (&flist->list, link, &flist->size);

    CHECK_FILE_LIST_INTEGRITY (flist);

    moo_file_unref (file);
    return index;
}


static gboolean  file_list_contains     (FileList   *flist,
                                         MooFile    *file)
{
    return g_hash_table_lookup (flist->file_to_link, file) != NULL;
}


static MooFile  *file_list_nth          (FileList   *flist,
                                         int         index)
{
    MooFile *file;
    g_assert (0 <= index && index < flist->size);
    file = g_list_nth_data (flist->list, index);
    g_assert (file != NULL);
    return file;
}


/* TODO */
static int       file_list_position     (FileList   *flist,
                                         MooFile    *file)
{
    GList *link;
    int position;
    g_assert (file != NULL);
    link = g_hash_table_lookup (flist->file_to_link, file);
    g_assert (link != NULL);
    position = g_list_position (flist->list, link);
    g_assert (position >= 0);
    return position;
}


static MooFile  *file_list_find_name    (FileList   *flist,
                                         const char *name)
{
    return g_hash_table_lookup (flist->name_to_file, name);
}


static MooFile  *file_list_find_display_name
                                        (FileList   *flist,
                                         const char *display_name)
{
    return g_hash_table_lookup (flist->display_name_to_file,
                                display_name);
}


static MooFile  *file_list_first        (FileList   *flist)
{
    if (flist->list)
        return flist->list->data;
    else
        return NULL;
}


static MooFile  *file_list_next         (FileList   *flist,
                                         MooFile    *file)
{
    GList *link = g_hash_table_lookup (flist->file_to_link, file);
    g_assert (link != NULL);
    if (link->next)
        return link->next->data;
    else
        return NULL;
}


static void      _hash_table_insert     (FileList       *flist,
                                         MooFile        *file,
                                         GList          *link)
{
    g_hash_table_insert (flist->file_to_link, file, link);
    g_hash_table_insert (flist->name_to_file,
                         g_strdup (moo_file_get_basename (file)),
                         file);
    g_hash_table_insert (flist->display_name_to_file,
                         g_strdup (moo_file_get_display_basename (file)),
                         file);
}


static void      _hash_table_remove     (FileList       *flist,
                                         MooFile        *file)
{
    g_hash_table_remove (flist->file_to_link, file);
    g_hash_table_remove (flist->name_to_file,
                         moo_file_get_basename (file));
    g_hash_table_remove (flist->display_name_to_file,
                         moo_file_get_display_basename (file));
}


static void      _list_delete_link      (GList         **list,
                                         GList          *link,
                                         int            *list_len)
{
    g_assert (*list_len == (int)g_list_length (*list));
    *list = g_list_delete_link (*list, link);
    (*list_len)--;
    g_assert (*list_len == (int)g_list_length (*list));
}


/* TODO */
static GList    *_list_find             (FileList       *flist,
                                         MooFile        *file,
                                         int            *index)
{
    GList *link = g_hash_table_lookup (flist->file_to_link, file);
    g_return_val_if_fail (link != NULL, NULL);
    *index = g_list_position (flist->list, link);
    g_return_val_if_fail (*index >= 0, NULL);
    return link;
}


static int      _cmp_filenames          (MooFile        *f1,
                                         MooFile        *f2)
{
    return strcmp (moo_file_get_basename (f1),
                   moo_file_get_basename (f2));
}

static void     _find_insert_position   (GList          *list,
                                         int             list_len,
                                         MooFile        *file,
                                         GList         **prev,
                                         GList         **next,
                                         int            *position)
{
    GList *left = NULL, *right = NULL;
    int pos = -1;

    while (list_len)
    {
        int cmp;

        if (!left)
        {
            left = list;
            pos = 0;
        }

        cmp = _cmp_filenames (file, left->data);
        g_assert (cmp != 0);

        if (cmp < 0)
        {
            right = left;
            left = left->prev;
            pos--;
            break;
        }
        else
        {
            if (list_len == 1)
            {
                right = left->next;
                break;
            }
            else if (list_len == 2)
            {
                g_assert (left->next != NULL);
                cmp = _cmp_filenames (file, left->next->data);
                g_assert (cmp != 0);

                if (cmp < 0)
                {
                    right = left->next;
                }
                else
                {
                    right = left->next->next;
                    left = left->next;
                    pos++;
                }

                break;
            }
            else if (list_len % 2)
            {
                right = g_list_nth (left, list_len / 2);
                g_assert (right != NULL);

                cmp = _cmp_filenames (file, right->data);
                g_assert (cmp != 0);

                if (cmp > 0)
                {
                    left = right;
                    pos += list_len / 2;
                }

                list_len = list_len / 2 + 1;
            }
            else
            {
                right = g_list_nth (left, list_len / 2);
                g_assert (right != NULL);

                cmp = _cmp_filenames (file, right->data);
                g_assert (cmp != 0);

                if (cmp < 0)
                {
                    list_len = list_len / 2 + 1;
                }
                else
                {
                    left = right;
                    pos += list_len / 2;
                    list_len = list_len / 2;
                }
            }
        }
    }

    *next = right;
    *prev = left;
    *position = pos + 1;
}

/* TODO */
static GList    *_list_insert_sorted    (GList         **list,
                                         int            *list_len,
                                         MooFile        *file,
                                         int            *position)
{
    GList *link;
    GList *prev, *next;

    g_assert (*list_len == (int)g_list_length (*list));
    g_assert (g_list_find (*list, file) == NULL);

    _find_insert_position (*list, *list_len, file, &prev, &next, position);
    g_assert (*position >= 0);

    link = g_list_alloc ();
    link->data = file;
    link->prev = prev;
    link->next = next;

    if (prev)
        prev->next = link;
    else
        *list = link;

    if (next)
        next->prev = link;

    (*list_len)++;
    g_assert (g_list_nth (*list, *position) == link);
    g_assert (*list_len == (int)g_list_length (*list));
    return link;
}


G_END_DECLS

#endif /* __MOO_FOLDER_MODEL_PRIVATE_H__ */
