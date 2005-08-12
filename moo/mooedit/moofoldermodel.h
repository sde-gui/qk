/*
 *   mooedit/moofoldermodel.h
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

#ifndef __MOO_FOLDER_MODEL_H__
#define __MOO_FOLDER_MODEL_H__

#include "mooedit/moofile.h"

G_BEGIN_DECLS

#define MOO_TYPE_FOLDER_MODEL           (moo_folder_model_get_type ())
#define MOO_FOLDER_MODEL(object)        (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FOLDER_MODEL, MooFolderModel))
#define MOO_FOLDER_MODEL_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FOLDER_MODEL, MooFolderModelClass))
#define MOO_IS_FOLDER_MODEL(object)     (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FOLDER_MODEL))
#define MOO_IS_FOLDER_MODELCLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FOLDER_MODEL))
#define MOO_FOLDER_MODEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FOLDER_MODEL, MooFolderModelClass))

typedef struct _MooFolderModel        MooFolderModel;
typedef struct _MooFolderModelPrivate MooFolderModelPrivate;
typedef struct _MooFolderModelClass   MooFolderModelClass;

struct _MooFolderModel
{
    GObject parent;
    MooFolderModelPrivate *priv;
};

struct _MooFolderModelClass
{
    GObjectClass    parent_class;
};


GType            moo_folder_model_get_type      (void) G_GNUC_CONST;
GtkTreeModel    *moo_folder_model_new           (MooFolder      *folder);
void             moo_folder_model_set_folder    (MooFolderModel *model,
                                                 MooFolder      *folder);
MooFolder       *moo_folder_model_get_folder    (MooFolderModel *model);


G_END_DECLS

#endif /* __MOO_FOLDER_MODEL_H__ */
