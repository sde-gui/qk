/*
 *   ctags-doc.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef CTAGS_DOC_H
#define CTAGS_DOC_H

#include <mooedit/mooplugin.h>

#define MOO_TYPE_CTAGS_ENTRY (_moo_ctags_entry_get_type ())

#define MOO_TYPE_CTAGS_DOC_PLUGIN              (_moo_ctags_doc_plugin_get_type ())
#define MOO_CTAGS_DOC_PLUGIN(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_CTAGS_DOC_PLUGIN, MooCtagsDocPlugin))
#define MOO_CTAGS_DOC_PLUGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_CTAGS_DOC_PLUGIN, MooCtagsDocPluginClass))
#define MOO_IS_CTAGS_DOC_PLUGIN(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_CTAGS_DOC_PLUGIN))
#define MOO_IS_CTAGS_DOC_PLUGIN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_CTAGS_DOC_PLUGIN))
#define MOO_CTAGS_DOC_PLUGIN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_CTAGS_DOC_PLUGIN, MooCtagsDocPluginClass))

typedef struct _MooCtagsDocPlugin MooCtagsDocPlugin;
typedef struct _MooCtagsDocPluginPrivate MooCtagsDocPluginPrivate;
typedef struct _MooCtagsDocPluginClass MooCtagsDocPluginClass;
typedef struct _MooCtagsEntry MooCtagsEntry;

struct _MooCtagsDocPlugin
{
    MooDocPlugin base;
    MooCtagsDocPluginPrivate *priv;
};

struct _MooCtagsDocPluginClass
{
    MooDocPluginClass base_class;
};

struct _MooCtagsEntry
{
    guint ref_count;
    char *name;
    const char *kind;
    int line;
    char *klass;
    guint file_scope : 1;
};


GType               _moo_ctags_doc_plugin_get_type      (void) G_GNUC_CONST;
GtkTreeModel       *_moo_ctags_doc_plugin_get_store     (MooCtagsDocPlugin  *plugin);

GType               _moo_ctags_entry_get_type           (void) G_GNUC_CONST;
MooCtagsEntry      *_moo_ctags_entry_ref                (MooCtagsEntry      *entry);
void                _moo_ctags_entry_unref              (MooCtagsEntry      *entry);


#endif /* CTAGS_DOC_H */
