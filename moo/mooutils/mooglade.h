/*
 *   mooglade.h
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

#ifndef __MOO_GLADE_H__
#define __MOO_GLADE_H__

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

typedef struct _MooGladeXML MooGladeXML;

#define MOO_TYPE_GLADE_XML (moo_glade_xml_get_type ())

GType        moo_glade_xml_get_type  (void);

MooGladeXML *moo_glade_xml_parse_file   (const char     *file,
                                         GError        **error);
MooGladeXML *moo_glade_xml_parse_memory (const char     *buffer,
                                         int             size,
                                         GError        **error);

MooGladeXML *moo_glade_xml_ref          (MooGladeXML    *xml);
void         moo_glade_xml_unref        (MooGladeXML    *xml);

GtkWidget   *moo_glade_xml_get_widget   (MooGladeXML    *xml,
                                         const char     *id);


G_END_DECLS

#endif /* __MOO_GLADE_H__ */
