/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofold.h
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

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#ifndef __MOO_FOLD_H__
#define __MOO_FOLD_H__

#include <mooedit/moolinemark.h>

G_BEGIN_DECLS


#define MOO_TYPE_FOLD              (moo_fold_get_type ())
#define MOO_FOLD(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_FOLD, MooFold))
#define MOO_FOLD_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_FOLD, MooFoldClass))
#define MOO_IS_FOLD(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_FOLD))
#define MOO_IS_FOLD_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_FOLD))
#define MOO_FOLD_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_FOLD, MooFoldClass))


typedef struct _MooFold         MooFold;
typedef struct _MooFoldPrivate  MooFoldPrivate;
typedef struct _MooFoldClass    MooFoldClass;

struct _MooFold
{
    GObject object;

    MooFold *parent;
    MooFold *next;
    GSList *children;

    MooLineMark *start;
    MooLineMark *end;

    guint collapsed : 1;
};

struct _MooFoldClass
{
    GObjectClass object_class;
};


GType       moo_fold_get_type              (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __MOO_FOLD_H__ */
