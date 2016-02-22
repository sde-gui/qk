/* -*- Mode: C; c-basic-offset: 4 -*-
 * pyglib - Python bindings for GLib toolkit.
 * Copyright (C) 1998-2003  James Henstridge
 *               2004-2008  Johan Dahlin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef __PYG_SOURCE_H__
#define __PYG_SOURCE_H__

extern PyTypeObject PyGSource_Type;
extern PyTypeObject PyGIdle_Type;
extern PyTypeObject PyGTimeout_Type;
extern PyTypeObject PyGPollFD_Type;

typedef struct
{
    PyObject_HEAD
    GPollFD pollfd;
    PyObject *fd_obj;
} PyGPollFD;

void pyglib_source_register_types(PyObject *d);

#endif /* __PYG_SOURCE_H__ */
