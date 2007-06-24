/*
 *   mooscript-value.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_SCRIPT_VALUE_H
#define MOO_SCRIPT_VALUE_H

#include <glib-object.h>

G_BEGIN_DECLS


#define MS_VALUE_TYPE(val) (ms_value_type (val))

typedef struct _MSValue MSValue;
typedef struct _MSFunc MSFunc;

typedef enum {
    MS_VALUE_NONE,
    MS_VALUE_INT,
    MS_VALUE_STRING,
    MS_VALUE_GVALUE,
    MS_VALUE_LIST,
    MS_VALUE_DICT,
    MS_VALUE_FUNC,
    MS_VALUE_INVALID
} MSValueType;


void         ms_type_init               (void);

MSValue     *ms_value_none              (void);
MSValue     *ms_value_false             (void);
MSValue     *ms_value_true              (void);
MSValue     *ms_value_bool              (gboolean        val);

gboolean     ms_value_is_none           (MSValue        *value);

MSValue     *ms_value_int               (int             val);
MSValue     *ms_value_string            (const char     *string);
MSValue     *ms_value_string_len        (const char     *string,
                                         int             chars);
MSValue     *ms_value_take_string       (char           *string);
MSValue     *ms_value_from_gvalue       (const GValue   *gval);

MSValue     *ms_value_list              (guint           n_elms);
void         ms_value_list_set_elm      (MSValue        *list,
                                         guint           index,
                                         MSValue        *elm);

MSValue     *ms_value_dict              (void);
void         ms_value_dict_set_elm      (MSValue        *dict,
                                         const char     *key,
                                         MSValue        *val);
MSValue     *ms_value_dict_get_elm      (MSValue        *dict,
                                         const char     *key);

void         ms_value_dict_set_string   (MSValue        *dict,
                                         const char     *key,
                                         const char     *val);

MSValue     *ms_value_ref               (MSValue        *val);
void         ms_value_unref             (MSValue        *val);

gboolean     ms_value_get_bool          (MSValue        *val);
gboolean     ms_value_get_int           (MSValue        *val,
                                         int            *ival);
char        *ms_value_print             (MSValue        *val);
char        *ms_value_repr              (MSValue        *val);

int          ms_value_cmp               (MSValue        *a,
                                         MSValue        *b);

MSValue     *ms_value_func              (MSFunc         *func);


G_END_DECLS

#endif /* MOO_SCRIPT_VALUE_H */
