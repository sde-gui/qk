/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooscript-parser.h
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

#ifndef __MOO_SCRIPT_PARSER_H__
#define __MOO_SCRIPT_PARSER_H__

#include "mooscript-node.h"
#include "mooscript-context.h"

G_BEGIN_DECLS


typedef enum {
    MS_SCRIPT_COMPLETE,
    MS_SCRIPT_INCOMPLETE,
    MS_SCRIPT_ERROR
} MSScriptCheckResult;

MSNode              *ms_script_parse    (const char *string);
MSScriptCheckResult  ms_script_check    (const char *string);


G_END_DECLS

#endif /* __MOO_SCRIPT_PARSER_H__ */
