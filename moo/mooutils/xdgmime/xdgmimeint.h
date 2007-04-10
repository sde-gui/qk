/* -*- mode: C; c-file-style: "gnu" -*- */
/* xdgmimeint.h: Internal defines and functions.
 *
 * More info can be found at http://www.freedesktop.org/standards/
 *
 * Copyright (C) 2003  Red Hat, Inc.
 * Copyright (C) 2003  Jonathan Blandford <jrb@alum.mit.edu>
 *
 * Licensed under the Academic Free License version 2.0
 * Or under the following terms:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __XDG_MIME_INT_H__
#define __XDG_MIME_INT_H__

#include "xdgmime.h"
#include <glib.h>
#include <string.h>

#ifndef	FALSE
#define	FALSE (0)
#endif

#ifndef	TRUE
#define	TRUE (!FALSE)
#endif

typedef gunichar xdg_unichar_t;
typedef guint8   xdg_uchar8_t;
typedef guint16  xdg_uint16_t;
typedef guint32  xdg_uint32_t;

#if 0 && defined(XDG_PREFIX)
#define _xdg_utf8_to_ucs4   XDG_ENTRY(utf8_to_ucs4)
#define _xdg_ucs4_to_lower   XDG_ENTRY(ucs4_to_lower)
#define _xdg_utf8_validate   XDG_ENTRY(utf8_validate)
#define _xdg_get_base_name   XDG_ENTRY(get_base_name)
#define _xdg_utf8_skip       XDG_ENTRY(utf8_skip)
#endif

#define SWAP_BE16_TO_LE16 GUINT16_SWAP_LE_BE
#define SWAP_BE32_TO_LE32 GUINT32_SWAP_LE_BE

/* UTF-8 utils
 */
#define _xdg_utf8_next_char g_utf8_next_char
#define _xdg_utf8_to_ucs4   g_utf8_get_char
#define _xdg_ucs4_to_lower  g_unichar_tolower

#define _xdg_utf8_validate(s) g_utf8_validate ((s), -1, NULL)

inline static const char *
_xdg_get_base_name (const char *file_name)
{
  const char *base_name;

  if (file_name == NULL)
    return NULL;

  base_name = strrchr (file_name, '/');

  if (base_name == NULL)
    return file_name;
  else
    return base_name + 1;
}

/*
xdg_unichar_t  _xdg_utf8_to_ucs4  (const char    *source);
xdg_unichar_t  _xdg_ucs4_to_lower (xdg_unichar_t  source);
int            _xdg_utf8_validate (const char    *source);
const char    *_xdg_get_base_name (const char    *file_name);
*/

#ifdef XDG_PREFIX
#define _xdg_mime_buffer_is_text  XDG_ENTRY(buffer_is_text)
#endif
int _xdg_mime_buffer_is_text (unsigned char *buffer,
			      int            len);

#define malloc g_try_malloc
#define realloc g_try_realloc
#define free g_free
#define strdup g_strdup
#define calloc(nmemb,size) g_malloc0 ((nmemb)*(size))

#endif /* __XDG_MIME_INT_H__ */
