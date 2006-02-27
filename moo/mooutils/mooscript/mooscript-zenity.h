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
MSFunc  *ms_zenity_entry            (void);
/* Text(text = none, dialog_text = none) */
MSFunc  *ms_zenity_text             (void);

MSFunc  *ms_zenity_info             (void); /* Info(text = none) */
MSFunc  *ms_zenity_error            (void); /* Error(text = none) */
MSFunc  *ms_zenity_question         (void); /* Question(text = none) */
MSFunc  *ms_zenity_warning          (void); /* Warning(text = none) */

MSFunc  *ms_zenity_choose_file      (void); /* ChooseFile(title = none, start = none) */
MSFunc  *ms_zenity_choose_files     (void); /* ChooseFile(title = none, start = none) */
MSFunc  *ms_zenity_choose_dir       (void); /* ChooseDir(title = none, start = none) */
MSFunc  *ms_zenity_choose_file_save (void); /* ChooseFileSave(title = none, start = none) */


G_END_DECLS

#endif /* __MOO_SCRIPT_ZENITY_H__ */
