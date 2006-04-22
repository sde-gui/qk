/*
 *   moocksum.h
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

#ifndef __MOO_CKSUM_H__
#define __MOO_CKSUM_H__

#include <glib.h>

G_BEGIN_DECLS


/* md5sum */
char   *moo_cksum_for_buffer    (const char *input,
                                 gssize      length);
char   *moo_cksum               (const char *path);


G_END_DECLS

#endif /* __MOO_CKSUM_H__ */
