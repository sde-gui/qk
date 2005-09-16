/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject-plugin.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cproject.h"
#include "mooedit/mooplugin-macro.h"
#include <gmodule.h>

#ifndef MOO_VERSION
#define MOO_VERSION NULL
#endif


MOO_PLUGIN_DEFINE_PARAMS(info, TRUE, CPROJECT_PLUGIN_ID,
                         "CProject", "C Project",
                         "Yevgen Muntyan <muntyan@tamu.edu>",
                         MOO_VERSION);
MOO_PLUGIN_DEFINE(CProjectPlugin, cproject_plugin,
                  cproject_plugin_init, cproject_plugin_deinit,
                  cproject_plugin_attach, cproject_plugin_detach,
                  NULL,
                  info, G_TYPE_NONE);


gboolean cproject_init (void);

G_MODULE_EXPORT gboolean
cproject_init (void)
{
    return moo_plugin_register (CPROJECT_PLUGIN_TYPE);
}
