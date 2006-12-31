/*
 *   moooutputfiltersimple.h
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

#ifndef __MOO_OUTPUT_FILTER_SIMPLE_SIMPLE_H__
#define __MOO_OUTPUT_FILTER_SIMPLE_SIMPLE_H__

#include <mooedit/moooutputfilter.h>

G_BEGIN_DECLS


#define MOO_TYPE_OUTPUT_FILTER_SIMPLE              (_moo_output_filter_simple_get_type ())
#define MOO_OUTPUT_FILTER_SIMPLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_OUTPUT_FILTER_SIMPLE, MooOutputFilterSimple))
#define MOO_OUTPUT_FILTER_SIMPLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_OUTPUT_FILTER_SIMPLE, MooOutputFilterSimpleClass))
#define MOO_IS_OUTPUT_FILTER_SIMPLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_OUTPUT_FILTER_SIMPLE))
#define MOO_IS_OUTPUT_FILTER_SIMPLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_OUTPUT_FILTER_SIMPLE))
#define MOO_OUTPUT_FILTER_SIMPLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_OUTPUT_FILTER_SIMPLE, MooOutputFilterSimpleClass))

typedef struct _MooOutputFilterSimple        MooOutputFilterSimple;
typedef struct _MooOutputFilterSimplePrivate MooOutputFilterSimplePrivate;
typedef struct _MooOutputFilterSimpleClass   MooOutputFilterSimpleClass;
typedef struct _MooOutputFilterInfo          MooOutputFilterInfo;
typedef struct _MooOutputPatternInfo         MooOutputPatternInfo;

typedef enum {
    MOO_OUTPUT_STDOUT,
    MOO_OUTPUT_STDERR,
    MOO_OUTPUT_ALL
} MooOutputTextType;

struct _MooOutputFilterSimple {
    MooOutputFilter base;
    MooOutputFilterSimplePrivate *priv;
};

struct _MooOutputFilterSimpleClass {
    MooOutputFilterClass base_class;
};

struct _MooOutputPatternInfo {
    char *pattern;
    MooOutputTextType type;
};

struct _MooOutputFilterInfo {
    char *id;
    char *name;
    MooOutputPatternInfo **patterns;
    guint n_patterns;
    guint ref_count;
    guint deleted : 1;
    guint builtin : 1;
    guint enabled : 1;
};


GType                _moo_output_filter_simple_get_type (void) G_GNUC_CONST;

void                 _moo_command_filter_simple_load    (void);


G_END_DECLS

#endif /* __MOO_OUTPUT_FILTER_SIMPLE_SIMPLE_H__ */
