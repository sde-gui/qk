/*
 *   moouixml.h
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

#ifndef __MOO_UI_XML_H__
#define __MOO_UI_XML_H__

#include <mooutils/moomarkup.h>
#include <mooutils/mooactioncollection.h>

G_BEGIN_DECLS


typedef enum {
    MOO_UI_NODE_CONTAINER = 1,
    MOO_UI_NODE_WIDGET,
    MOO_UI_NODE_ITEM,
    MOO_UI_NODE_SEPARATOR,
    MOO_UI_NODE_PLACEHOLDER
} MooUINodeType;

typedef enum {
    MOO_UI_NODE_ENABLE_EMPTY = 1 << 0
} MooUINodeFlags;

typedef struct _MooUINode            MooUINode;
typedef struct _MooUIWidgetNode      MooUIWidgetNode;
typedef struct _MooUIItemNode        MooUIItemNode;
typedef struct _MooUISeparatorNode   MooUISeparatorNode;
typedef struct _MooUIPlaceholderNode MooUIPlaceholderNode;


#define MOO_UI_NODE_STRUCT      \
    MooUINodeType type;         \
    char *name;                 \
    MooUINode *parent;          \
    GSList *children;           \
    guint flags : 1

struct _MooUINode {
    MOO_UI_NODE_STRUCT;
};

struct _MooUIWidgetNode {
    MOO_UI_NODE_STRUCT;
};

struct _MooUISeparatorNode {
    MOO_UI_NODE_STRUCT;
};

struct _MooUIPlaceholderNode {
    MOO_UI_NODE_STRUCT;
};

struct _MooUIItemNode {
    MOO_UI_NODE_STRUCT;

    char *action;

    char *label;
    char *tooltip;
    char *stock_id;
    char *icon_stock_id;
};

typedef enum {
    MOO_UI_MENUBAR = 1,
    MOO_UI_MENU,
    MOO_UI_TOOLBAR
} MooUIWidgetType;


#define MOO_TYPE_UI_NODE             (moo_ui_node_get_type ())
#define MOO_TYPE_UI_WIDGET_TYPE      (moo_ui_widget_type_get_type ())

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
    MooUIXMLPrivate *priv;
};

struct _MooUIXMLClass
{
    GObjectClass parent_class;
};


GType       moo_ui_xml_get_type             (void) G_GNUC_CONST;
GType       moo_ui_node_get_type            (void) G_GNUC_CONST;
GType       moo_ui_widget_type_get_type     (void) G_GNUC_CONST;

MooUIXML   *moo_ui_xml_new                  (void);

void        moo_ui_xml_add_ui_from_string   (MooUIXML       *xml,
                                             const char     *buffer,
                                             gssize          length);

MooUINode  *moo_ui_xml_get_node             (MooUIXML       *xml,
                                             const char     *path);
MooUINode  *moo_ui_xml_find_node            (MooUIXML       *xml,
                                             const char     *path_or_placeholder);
MooUINode  *moo_ui_xml_find_placeholder     (MooUIXML       *xml,
                                             const char     *name);
char       *moo_ui_node_get_path            (MooUINode      *node);
MooUINode  *moo_ui_node_get_child           (MooUINode      *node,
                                             const char     *path);

gpointer    moo_ui_xml_create_widget        (MooUIXML       *xml,
                                             MooUIWidgetType type,
                                             const char     *path,
                                             MooActionCollection *actions,
                                             GtkAccelGroup  *accel_group);
GtkWidget  *moo_ui_xml_get_widget           (MooUIXML       *xml,
                                             GtkWidget      *toplevel,
                                             const char     *path);

guint       moo_ui_xml_new_merge_id         (MooUIXML       *xml);

MooUINode  *moo_ui_xml_add_item             (MooUIXML       *xml,
                                             guint           merge_id,
                                             const char     *parent_path,
                                             const char     *name,
                                             const char     *action,
                                             int             position);

void        moo_ui_xml_insert_after         (MooUIXML       *xml,
                                             guint           merge_id,
                                             MooUINode      *parent,
                                             MooUINode      *after,
                                             const char     *markup);
void        moo_ui_xml_insert_before        (MooUIXML       *xml,
                                             guint           merge_id,
                                             MooUINode      *parent,
                                             MooUINode      *before,
                                             const char     *markup);
void        moo_ui_xml_insert               (MooUIXML       *xml,
                                             guint           merge_id,
                                             MooUINode      *parent,
                                             int             position,
                                             const char     *markup);

void        moo_ui_xml_insert_markup_after  (MooUIXML       *xml,
                                             guint           merge_id,
                                             const char     *parent_path,
                                             const char     *after,
                                             const char     *markup);
void        moo_ui_xml_insert_markup_before (MooUIXML       *xml,
                                             guint           merge_id,
                                             const char     *parent_path,
                                             const char     *before,
                                             const char     *markup);
void        moo_ui_xml_insert_markup        (MooUIXML       *xml,
                                             guint           merge_id,
                                             const char     *parent_path,
                                             int             position,
                                             const char     *markup);

void        moo_ui_xml_remove_ui            (MooUIXML       *xml,
                                             guint           merge_id);

void        moo_ui_xml_remove_node          (MooUIXML       *xml,
                                             MooUINode      *node);


G_END_DECLS

#endif /* __MOO_UI_XML_H__ */

