/*
 *   mooui/moouixml.h
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

#ifndef MOOUI_MOOUIXML_H
#define MOOUI_MOOUIXML_H

#include "mooutils/moomarkup.h"
#include "mooui/mooactiongroup.h"
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_UI_XML              (moo_ui_xml_get_type ())
#define MOO_UI_XML(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_UI_XML, MooUIXML))
#define MOO_UI_XML_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_UI_XML, MooUIXMLClass))
#define MOO_IS_UI_XML(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_UI_XML))
#define MOO_IS_UI_XML_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_UI_XML))
#define MOO_UI_XML_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_UI_XML, MooUIXMLClass))


typedef struct _MooUIXML        MooUIXML;
typedef struct _MooUIXMLPrivate MooUIXMLPrivate;
typedef struct _MooUIXMLClass   MooUIXMLClass;

struct _MooUIXML
{
    GObject          object;
    MooMarkupDoc    *doc;
};

struct _MooUIXMLClass
{
    GObjectClass parent_class;

    void    (*changed)  (MooUIXML *xml);
};


GType            moo_ui_xml_get_type            (void) G_GNUC_CONST;

MooUIXML        *moo_ui_xml_new                 (void);
MooUIXML        *moo_ui_xml_new_from_string     (const char     *xml,
                                                 GError        **error);
MooUIXML        *moo_ui_xml_new_from_file       (const char     *file,
                                                 GError        **error);

gboolean         moo_ui_xml_add_ui_from_string  (MooUIXML       *xml,
                                                 const char     *ui,
                                                 int             len,
                                                 GError        **error);
gboolean         moo_ui_xml_add_ui_from_file    (MooUIXML       *xml,
                                                 const char     *file,
                                                 GError        **error);

char            *moo_ui_xml_get_ui              (MooUIXML       *xml);
const MooMarkupDoc *moo_ui_xml_get_markup       (MooUIXML       *xml);

gboolean         moo_ui_xml_has_widget          (MooUIXML       *xml,
                                                 const char     *path);

GtkWidget       *moo_ui_xml_create_widget       (MooUIXML       *xml,
                                                 const char     *path,
                                                 MooActionGroup *actions,
                                                 GtkAccelGroup  *accel_group,
                                                 GtkTooltips    *tooltips);
GtkWidget       *moo_ui_xml_update_widget       (MooUIXML       *xml,
                                                 GtkWidget      *widget,
                                                 const char     *path,
                                                 MooActionGroup *actions,
                                                 GtkAccelGroup  *accel_group,
                                                 GtkTooltips    *tooltips);


G_END_DECLS

#endif /* MOOUI_MOOUIXML_H */

