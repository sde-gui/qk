/*
 *   mooscript-node.h
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

#ifndef __MOO_SCRIPT_NODE_H__
#define __MOO_SCRIPT_NODE_H__

#include <mooscript/mooscript-func.h>

G_BEGIN_DECLS


typedef struct _MSNode MSNode;


MSNode         *ms_node_ref                 (MSNode     *node);
void            ms_node_unref               (MSNode     *node);

MSValue        *ms_top_node_eval            (MSNode     *node,
                                             MSContext  *ctx);


G_END_DECLS

#endif /* __MOO_SCRIPT_NODE_H__ */
