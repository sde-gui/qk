/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-zenity.h
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

#ifndef __MOO_SCRIPT_ZENITY_H__
#define __MOO_SCRIPT_ZENITY_H__

#include "mooscript-func.h"

G_BEGIN_DECLS


/* Entry(dialog_text = none, entry_text = none, hide_text = false) */
MSFunc  *ms_zenity_entry    (void);

/* Info(text = none) */
MSFunc  *ms_zenity_info     (void);

/* Error(text = none) */
MSFunc  *ms_zenity_error    (void);


G_END_DECLS

#endif /* __MOO_SCRIPT_ZENITY_H__ */
