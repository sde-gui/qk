/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moofold.c
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

#define MOOEDIT_COMPILATION
#include "mooedit/moofold.h"
#include "mooedit/mootext-private.h"
#include "mooutils/moomarshals.h"


static void     moo_fold_finalize       (GObject        *object);


enum {
    LAST_SIGNAL
};

// static guint signals[LAST_SIGNAL];


enum {
    PROP_0
};


/* MOO_TYPE_FOLD */
G_DEFINE_TYPE (MooFold, moo_fold, G_TYPE_OBJECT)


static void
moo_fold_class_init (MooFoldClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_fold_finalize;
}


static void
moo_fold_init (G_GNUC_UNUSED MooFold *fold)
{
}


static void
moo_fold_finalize (GObject *object)
{
    MooFold *fold = MOO_FOLD (object);
    G_OBJECT_CLASS (moo_fold_parent_class)->finalize (object);
}
