/*
 *   moooutputfilter.h
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

#ifndef __MOO_OUTPUT_FILTER_H__
#define __MOO_OUTPUT_FILTER_H__

#include <mooedit/moolineview.h>

G_BEGIN_DECLS


#define MOO_TYPE_OUTPUT_FILTER                    (moo_output_filter_get_type ())
#define MOO_OUTPUT_FILTER(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_OUTPUT_FILTER, MooOutputFilter))
#define MOO_OUTPUT_FILTER_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_OUTPUT_FILTER, MooOutputFilterClass))
#define MOO_IS_OUTPUT_FILTER(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_OUTPUT_FILTER))
#define MOO_IS_OUTPUT_FILTER_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_OUTPUT_FILTER))
#define MOO_OUTPUT_FILTER_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_OUTPUT_FILTER, MooOutputFilterClass))

typedef struct _MooOutputFilter                  MooOutputFilter;
typedef struct _MooOutputFilterClass             MooOutputFilterClass;

struct _MooOutputFilter {
    GObject base;
};

struct _MooOutputFilterClass {
    GObjectClass base_class;
};


GType   moo_output_filter_get_type  (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __MOO_OUTPUT_FILTER_H__ */
