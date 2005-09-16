/*
 *   mooui/moouixml.c
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

#include "mooui/moouixml.h"
#include "mooutils/moocompat.h"
#include "mooutils/moomarshals.h"
#include <gtk/gtk.h>
#include <string.h>


/* XXX weak ref actions */

typedef MooUINodeType NodeType;
typedef MooUINode Node;
typedef MooUIWidgetNode Widget;
typedef MooUIItemNode Item;
typedef MooUISeparatorNode Separator;
typedef MooUIPlaceholderNode Placeholder;

#define CONTAINER MOO_UI_NODE_CONTAINER
#define WIDGET MOO_UI_NODE_WIDGET
#define TOOLBAR MOO_UI_NODE_TOOLBAR
#define ITEM MOO_UI_NODE_ITEM
#define SEPARATOR MOO_UI_NODE_SEPARATOR
#define PLACEHOLDER MOO_UI_NODE_PLACEHOLDER

static const char *NODE_TYPE_NAME[] = {
    NULL,
    "MOO_UI_NODE_CONTAINER",
    "MOO_UI_NODE_WIDGET",
    "MOO_UI_NODE_ITEM",
    "MOO_UI_NODE_SEPARATOR",
    "MOO_UI_NODE_PLACEHOLDER"
};

struct _MooUIXMLPrivate {
    Node *ui;
    GSList *toplevels;  /* Toplevel* */
    guint last_merge_id;
    GSList *merged_ui; /* Merge* */
};

typedef struct {
    Node *node;
    GtkWidget *widget;
    GHashTable *children; /* Node* -> GtkWidget* */
    MooActionGroup *actions;
    GtkAccelGroup *accel_group;
} Toplevel;

typedef struct {
    guint id;
    GSList *nodes;
} Merge;

typedef enum {
    UPDATE_ADD_NODE,
    UPDATE_REMOVE_NODE,
    UPDATE_CHANGE_NODE
} UpdateType;

#define TOPLEVEL_QUARK (toplevel_quark ())
#define NODE_QUARK (node_quark ())


static void     moo_ui_xml_finalize     (GObject        *object);

static void     moo_ui_xml_add_markup   (MooUIXML       *xml,
                                         MooMarkupNode  *mnode);

static void     update_widgets          (MooUIXML       *xml,
                                         UpdateType      type,
                                         Node           *node);

static Node    *parse_markup            (MooMarkupNode  *mnode);
static Node    *parse_object            (MooMarkupNode  *mnode);
static Node    *parse_widget            (MooMarkupNode  *mnode);
static Node    *parse_placeholder       (MooMarkupNode  *mnode);
static Node    *parse_item              (MooMarkupNode  *mnode);
static Node    *parse_separator         (MooMarkupNode  *mnode);
static gboolean placeholder_check       (Node           *node);
static gboolean item_check              (Node           *node);
static gboolean widget_check            (Node           *node);
static gboolean container_check         (Node           *node);

static Merge   *lookup_merge            (MooUIXML       *xml,
                                         guint           id);

static Node    *node_new                (gsize           size,
                                         NodeType        type,
                                         const char     *name);
static Item    *item_new                (const char     *name,
                                         const char     *action,
                                         const char     *stock_id,
                                         const char     *label,
                                         const char     *icon_stock_id);
static gboolean node_is_ancestor        (Node           *node,
                                         Node           *ancestor);
static gboolean node_is_empty           (Node           *node);
static GSList  *node_list_children      (Node           *ndoe);
static void     node_free               (Node           *node);

static void     merge_add_node          (Merge          *merge,
                                         Node           *node);
static void     merge_remove_node       (Merge          *merge,
                                         Node           *node);

static Toplevel *toplevel_new           (Node           *node,
                                         MooActionGroup *actions,
                                         GtkAccelGroup  *accel_group);
static void     toplevel_free           (Toplevel       *toplevel);
static GtkWidget *toplevel_get_widget   (Toplevel       *toplevel,
                                         Node           *node);

static GQuark   toplevel_quark          (void);
static GQuark   node_quark              (void);

static void     xml_add_item_widget     (MooUIXML       *xml,
                                         GtkWidget      *widget);
static void     xml_add_widget          (MooUIXML       *xml,
                                         GtkWidget      *widget,
                                         Toplevel       *toplevel,
                                         Node           *node);
static void     xml_remove_widget       (MooUIXML       *xml,
                                         GtkWidget      *widget);
static void     xml_delete_toplevel     (MooUIXML       *xml,
                                         Toplevel       *toplevel);
static void     xml_connect_toplevel    (MooUIXML       *xml,
                                         Toplevel       *toplevel);

static gboolean create_menu_separator   (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         GtkMenuShell   *menu,
                                         Node           *node,
                                         int             index);
static void     create_menu_item        (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         GtkMenuShell   *menu,
                                         Node           *node,
                                         int             index);
static gboolean create_menu_shell       (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         MooUIWidgetType type);
static gboolean fill_menu_shell         (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         Node           *menu_node,
                                         GtkMenuShell   *menu);
static void     update_separators       (Node           *parent,
                                         Toplevel       *toplevel);
static gboolean create_tool_separator   (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         GtkToolbar     *toolbar,
                                         Node           *node,
                                         int             index);
static gboolean create_tool_item        (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         GtkToolbar     *toolbar,
                                         Node           *node,
                                         int             index);
static gboolean fill_toolbar            (MooUIXML       *xml,
                                         Toplevel       *toplevel,
                                         Node           *toolbar_node,
                                         GtkToolbar     *toolbar);
static gboolean create_toolbar          (MooUIXML       *xml,
                                         Toplevel       *toplevel);


/* MOO_TYPE_UI_XML */
G_DEFINE_TYPE (MooUIXML, moo_ui_xml, G_TYPE_OBJECT)


static void
moo_ui_xml_class_init (MooUIXMLClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_ui_xml_finalize;
}


static Node*
node_new (gsize size, NodeType type, const char *name)
{
    Node *node = g_malloc0 (size);
    node->type = type;
    node->name = g_strdup (name);
    return node;
}

#define NODE_NEW(typename, type, name) (node_new (sizeof(typename), type, name))


static void
moo_ui_xml_init (MooUIXML *xml)
{
    xml->priv = g_new0 (MooUIXMLPrivate, 1);
    xml->priv->ui = NODE_NEW (Node, CONTAINER, "ui");
}


MooUIXML*
moo_ui_xml_new (void)
{
    return g_object_new (MOO_TYPE_UI_XML, NULL);
}


static Node*
parse_object (MooMarkupNode *mnode)
{
    Node *node;
    const char *name;
    MooMarkupNode *mchild;

    name = moo_markup_get_prop (mnode, "name");

    if (!name || !name[0])
    {
        g_warning ("%s: object name missing", G_STRLOC);
        return NULL;
    }

    node = NODE_NEW (Node, CONTAINER, name);

    for (mchild = mnode->children; mchild != NULL; mchild = mchild->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (mchild))
        {
            Node *child = parse_markup (mchild);

            if (!child)
            {
                node_free (node);
                return NULL;
            }
            else
            {
                child->parent = node;
                node->children = g_slist_append (node->children, child);
            }
        }
    }

    return node;
}


static Node*
parse_widget (MooMarkupNode *mnode)
{
    Node *node;
    const char *name;
    MooMarkupNode *mchild;

    name = moo_markup_get_prop (mnode, "name");

    if (!name || !name[0])
    {
        g_warning ("%s: widget name missing", G_STRLOC);
        return NULL;
    }

    node = NODE_NEW (Widget, WIDGET, name);

    for (mchild = mnode->children; mchild != NULL; mchild = mchild->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (mchild))
        {
            Node *child = parse_markup (mchild);

            if (!child)
            {
                node_free (node);
                return NULL;
            }
            else
            {
                child->parent = node;
                node->children = g_slist_append (node->children, child);
            }
        }
    }

    return node;
}


static Node*
parse_placeholder (MooMarkupNode *mnode)
{
    Node *node;
    const char *name;
    MooMarkupNode *mchild;

    name = moo_markup_get_prop (mnode, "name");

    if (!name || !name[0])
    {
        g_warning ("%s: placeholder name missing", G_STRLOC);
        return NULL;
    }

    node = NODE_NEW (Placeholder, PLACEHOLDER, name);

    for (mchild = mnode->children; mchild != NULL; mchild = mchild->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (mchild))
        {
            Node *child = parse_markup (mchild);

            if (!child)
            {
                node_free (node);
                return NULL;
            }
            else
            {
                child->parent = node;
                node->children = g_slist_append (node->children, child);
            }
        }
    }

    return node;
}


static Item*
item_new (const char *name,
          const char *action,
          const char *stock_id,
          const char *label,
          const char *icon_stock_id)
{
    Item *item;

    g_return_val_if_fail (name && name[0], NULL);

    item = (Item*) NODE_NEW (Item, ITEM, name);

    item->action = g_strdup (action);
    item->stock_id = g_strdup (stock_id);
    item->label = g_strdup (label);
    item->icon_stock_id = g_strdup (icon_stock_id);

    return item;
}


static Node*
parse_item (MooMarkupNode *mnode)
{
    Item *item;
    const char *name;
    MooMarkupNode *mchild;

    name = moo_markup_get_prop (mnode, "name");

    if (!name || !name[0])
    {
        g_warning ("%s: item name missing", G_STRLOC);
        return NULL;
    }

    item = item_new (name,
                     moo_markup_get_prop (mnode, "action"),
                     moo_markup_get_prop (mnode, "stock_id"),
                     moo_markup_get_prop (mnode, "label"),
                     moo_markup_get_prop (mnode, "icon_stock_id"));

    for (mchild = mnode->children; mchild != NULL; mchild = mchild->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (mchild))
        {
            Node *child = parse_markup (mchild);

            if (!child)
            {
                node_free ((Node*) item);
                return NULL;
            }
            else
            {
                child->parent = (Node*) item;
                item->children = g_slist_append (item->children, child);
            }
        }
    }

    return (Node*) item;
}


static Node*
parse_separator (G_GNUC_UNUSED MooMarkupNode *mnode)
{
    return NODE_NEW (Separator, SEPARATOR, NULL);
}


static Node*
parse_markup (MooMarkupNode *mnode)
{
    g_return_val_if_fail (MOO_MARKUP_IS_ELEMENT (mnode), NULL);
    g_return_val_if_fail (mnode->name != NULL, NULL);

    if (!strcmp (mnode->name, "object"))
        return parse_object (mnode);
    else if (!strcmp (mnode->name, "widget"))
        return parse_widget (mnode);
    else if (!strcmp (mnode->name, "item"))
        return parse_item (mnode);
    else if (!strcmp (mnode->name, "separator"))
        return parse_separator (mnode);
    else if (!strcmp (mnode->name, "placeholder"))
        return parse_placeholder (mnode);

    g_warning ("%s: unknown element '%s'", G_STRLOC, mnode->name);
    return NULL;
}


static void
container_free (G_GNUC_UNUSED Node *node)
{
}

static void
widget_free (G_GNUC_UNUSED Node *node)
{
}

static void
separator_free (G_GNUC_UNUSED Node *node)
{
}

static void
placeholder_free (G_GNUC_UNUSED Node *node)
{
}

static void
item_free (Item *item)
{
    g_free (item->action);
    g_free (item->stock_id);
    g_free (item->label);
    g_free (item->icon_stock_id);
}

static void
node_free (Node *node)
{
    if (node)
    {
        g_slist_foreach (node->children, (GFunc) node_free, NULL);
        g_slist_free (node->children);
        node->children = NULL;

        switch (node->type)
        {
            case CONTAINER:
                container_free (node);
                break;
            case WIDGET:
                widget_free (node);
                break;
            case ITEM:
                item_free ((Item*) node);
                break;
            case SEPARATOR:
                separator_free (node);
                break;
            case PLACEHOLDER:
                placeholder_free (node);
                break;
        }

        g_free (node->name);
        g_free (node);
    }
}


void
moo_ui_xml_add_ui_from_string (MooUIXML       *xml,
                               const char     *buffer,
                               gssize          length)
{
    MooMarkupDoc *doc;
    MooMarkupNode *ui_node, *child;
    GError *error = NULL;

    g_return_if_fail (MOO_IS_UI_XML (xml));
    g_return_if_fail (buffer != NULL);

    doc = moo_markup_parse_memory (buffer, length, &error);

    if (!doc)
    {
        g_critical ("%s: could not parse markup", G_STRLOC);
        if (error)
        {
            g_critical ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }
        return;
    }

    ui_node = moo_markup_get_root_element (doc, "ui");

    if (!ui_node)
        ui_node = MOO_MARKUP_NODE (doc);

    for (child = ui_node->children; child != NULL; child = child->next)
        if (MOO_MARKUP_IS_ELEMENT (child))
            moo_ui_xml_add_markup (xml, child);

    moo_markup_doc_unref (doc);
}


static gboolean
placeholder_check (Node *node)
{
    g_return_val_if_fail (node != NULL, FALSE);
    g_return_val_if_fail (node->type == PLACEHOLDER, FALSE);
    g_return_val_if_fail (node->name && node->name[0], FALSE);
    return TRUE;
}


static gboolean
item_check (Node *node)
{
    GSList *l;

    g_return_val_if_fail (node != NULL, FALSE);
    g_return_val_if_fail (node->type == ITEM, FALSE);
    g_return_val_if_fail (node->name && node->name[0], FALSE);

    for (l = node->children; l != NULL; l = l->next)
    {
        Node *child = l->data;

        switch (child->type)
        {
            case SEPARATOR:
                break;
            case ITEM:
                if (!item_check (child))
                    return FALSE;
                break;
            case PLACEHOLDER:
                if (!placeholder_check (child))
                    return FALSE;
                break;
            default:
                g_warning ("%s: invalid menu item type %s",
                           G_STRLOC, NODE_TYPE_NAME[child->type]);
                return FALSE;
        }
    }

    return TRUE;
}


static gboolean
widget_check (Node *node)
{
    GSList *l;

    g_return_val_if_fail (node != NULL, FALSE);
    g_return_val_if_fail (node->type == WIDGET, FALSE);
    g_return_val_if_fail (node->name && node->name[0], FALSE);

    for (l = node->children; l != NULL; l = l->next)
    {
        Node *child = l->data;

        switch (child->type)
        {
            case SEPARATOR:
                break;
            case ITEM:
                if (!item_check (child))
                    return FALSE;
                break;
            case PLACEHOLDER:
                if (!placeholder_check (child))
                    return FALSE;
                break;
            default:
                g_warning ("%s: invalid widget item type %s",
                           G_STRLOC, NODE_TYPE_NAME[child->type]);
                return FALSE;
        }
    }

    return TRUE;
}


static gboolean
container_check (Node *node)
{
    GSList *l;

    g_return_val_if_fail (node != NULL, FALSE);
    g_return_val_if_fail (node->type == CONTAINER, FALSE);
    g_return_val_if_fail (node->name && node->name[0], FALSE);

    for (l = node->children; l != NULL; l = l->next)
    {
        Node *child = l->data;

        switch (child->type)
        {
            case CONTAINER:
                if (!container_check (child))
                    return FALSE;
                break;
            case WIDGET:
                if (!widget_check (child))
                    return FALSE;
                break;
            default:
                g_warning ("%s: invalid toplevel type %s",
                           G_STRLOC, NODE_TYPE_NAME[child->type]);
                return FALSE;
        }
    }

    return TRUE;
}


 void
moo_ui_xml_add_markup (MooUIXML       *xml,
                       MooMarkupNode  *mnode)
{
    Node *node = parse_markup (mnode);

    if (!node)
        return;

    switch (node->type)
    {
        case CONTAINER:
            if (!container_check (node))
            {
                node_free (node);
                return;
            }
            break;

        case WIDGET:
            if (!widget_check (node))
            {
                node_free (node);
                return;
            }
            break;

        case ITEM:
        case SEPARATOR:
        case PLACEHOLDER:
            g_warning ("%s: invalid toplevel type %s",
                       G_STRLOC, NODE_TYPE_NAME[node->type]);
            node_free (node);
            return;
    }

    if (moo_ui_xml_get_node (xml, node->name))
    {
        g_warning ("%s: implement me?", G_STRLOC);
        node_free (node);
        return;
    }

    node->parent = xml->priv->ui;
    xml->priv->ui->children = g_slist_append (xml->priv->ui->children, node);
}


guint
moo_ui_xml_new_merge_id (MooUIXML *xml)
{
    Merge *merge;

    g_return_val_if_fail (MOO_IS_UI_XML (xml), 0);

    xml->priv->last_merge_id++;
    merge = g_new0 (Merge, 1);
    merge->id = xml->priv->last_merge_id;
    merge->nodes = NULL;

    return merge->id;
}


 Merge*
lookup_merge (MooUIXML *xml,
              guint     merge_id)
{
    GSList *l;

    for (l = xml->priv->merged_ui; l != NULL; l = l->next)
    {
        Merge *merge = l->data;
        if (merge->id == merge_id)
            return merge;
    }

    return NULL;
}


MooUINode*
moo_ui_xml_add_item (MooUIXML       *xml,
                     guint           merge_id,
                     const char     *parent_path,
                     const char     *name,
                     const char     *action,
                     int             position)
{
    Merge *merge;
    MooUINode *parent;
    Item *item;

    g_return_val_if_fail (MOO_IS_UI_XML (xml), NULL);
    g_return_val_if_fail (parent_path != NULL, NULL);
    g_return_val_if_fail (name && name[0], NULL);

    merge = lookup_merge (xml, merge_id);
    g_return_val_if_fail (merge != NULL, NULL);

    parent = moo_ui_xml_get_node (xml, parent_path);
    g_return_val_if_fail (parent != NULL, NULL);

    switch (parent->type)
    {
        case MOO_UI_NODE_WIDGET:
        case MOO_UI_NODE_ITEM:
        case MOO_UI_NODE_PLACEHOLDER:
            break;

        case MOO_UI_NODE_CONTAINER:
        case MOO_UI_NODE_SEPARATOR:
            g_warning ("%s: can't add item to node of type %s",
                       G_STRLOC, NODE_TYPE_NAME[parent->type]);
    }

    item = item_new (name, action, NULL, NULL, NULL);
    item->parent = parent;
    parent->children = g_slist_insert (parent->children, item, position);

    merge_add_node (merge, (Node*) item);
    update_widgets (xml, UPDATE_ADD_NODE, (Node*) item);

    return (Node*) item;
}


#define SLIST_FOREACH(list,var)                     \
G_STMT_START {                                      \
    GSList *var;                                    \
    for (var = list; var != NULL; var = var->next)  \

#define SLIST_FOREACH_END                           \
} G_STMT_END


 gboolean
node_is_ancestor (Node           *node,
                  Node           *ancestor)
{
    Node *n;

    g_return_val_if_fail (node != NULL, FALSE);
    g_return_val_if_fail (ancestor != NULL, FALSE);

    for (n = node; n != NULL; n = n->parent)
        if (n == ancestor)
            return TRUE;

    return FALSE;
}


void
moo_ui_xml_remove_ui (MooUIXML       *xml,
                      guint           merge_id)
{
    Merge *merge;
    GSList *nodes;

    g_return_if_fail (MOO_IS_UI_XML (xml));

    merge = lookup_merge (xml, merge_id);
    g_return_if_fail (merge != NULL);

    nodes = g_slist_copy (merge->nodes);

    SLIST_FOREACH (nodes, l)
    {
        moo_ui_xml_remove_node (xml, l->data);
    }
    SLIST_FOREACH_END;

    g_return_if_fail (merge->nodes == NULL);
    xml->priv->merged_ui = g_slist_remove (xml->priv->merged_ui, merge);
    g_free (merge);
}


void
moo_ui_xml_remove_node (MooUIXML       *xml,
                        MooUINode      *node)
{
    Node *parent;

    g_return_if_fail (MOO_IS_UI_XML (xml));
    g_return_if_fail (node != NULL);
    g_return_if_fail (node_is_ancestor (node, xml->priv->ui));

    SLIST_FOREACH (xml->priv->merged_ui, l)
    {
        Merge *merge = l->data;
        GSList *merge_nodes = g_slist_copy (merge->nodes);

        SLIST_FOREACH (merge_nodes, n)
        {
            Node *merge_node = n->data;
            if (node_is_ancestor (merge_node, node))
                merge_remove_node (merge, merge_node);
        }
        SLIST_FOREACH_END;

        g_slist_free (merge_nodes);
    }
    SLIST_FOREACH_END;

    update_widgets (xml, UPDATE_REMOVE_NODE, node);

    parent = node->parent;
    parent->children = g_slist_remove (parent->children, node);
    node->parent = NULL;
    node_free (node);
}


 void
merge_add_node (Merge *merge,
                Node  *added)
{
    g_return_if_fail (merge != NULL);
    g_return_if_fail (added != NULL);

    SLIST_FOREACH (merge->nodes, l)
    {
        Node *node = l->data;

        if (node_is_ancestor (added, node))
            return;
    }
    SLIST_FOREACH_END;

    merge->nodes = g_slist_prepend (merge->nodes, added);
}


 void
merge_remove_node (Merge          *merge,
                   Node           *removed)
{
    g_return_if_fail (merge != NULL);
    g_return_if_fail (removed != NULL);

    merge->nodes = g_slist_remove (merge->nodes, removed);
}


MooUINode*
moo_ui_xml_get_node (MooUIXML       *xml,
                     const char     *path)
{
    char **pieces, **p;
    MooUINode *node;

    g_return_val_if_fail (MOO_IS_UI_XML (xml), NULL);
    g_return_val_if_fail (path != NULL, NULL);

    pieces = g_strsplit (path, "/", 0);
    g_return_val_if_fail (pieces != NULL, NULL);

    node = xml->priv->ui;

    for (p = pieces; *p != NULL; ++p)
    {
        Node *child = NULL;

        SLIST_FOREACH (node->children, l)
        {
            child = l->data;
            if (!strcmp (child->name, *p))
                break;
            else
                child = NULL;
        }
        SLIST_FOREACH_END;

        if (child)
            node = child;
        else
            return NULL;
    }

    return node;
}


static Toplevel*
toplevel_new (Node           *node,
              MooActionGroup *actions,
              GtkAccelGroup  *accel_group)
{
    Toplevel *top;

    g_return_val_if_fail (node != NULL, NULL);

    top = g_new0 (Toplevel, 1);
    top->node = node;
    top->widget = NULL;
    top->children = g_hash_table_new (g_direct_hash, g_direct_equal);
    top->actions = actions;
    top->accel_group = accel_group;

    return top;
}


static void
toplevel_free (Toplevel *toplevel)
{
    if (toplevel)
    {
        g_hash_table_destroy (toplevel->children);
        g_free (toplevel);
    }
}


static GQuark toplevel_quark (void)
{
    static GQuark q = 0;
    if (!q)
        q = g_quark_from_static_string ("moo-ui-xml-toplevel");
    return q;
}

static GQuark node_quark (void)
{
    static GQuark q = 0;
    if (!q)
        q = g_quark_from_static_string ("moo-ui-xml-node");
    return q;
}


static void
visibility_notify (GtkWidget *widget,
                   G_GNUC_UNUSED gpointer whatever,
                   MooUIXML  *xml)
{
    Toplevel *toplevel;
    Node *node, *parent;

    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (MOO_IS_UI_XML (xml));

    toplevel = g_object_get_qdata (G_OBJECT (widget), TOPLEVEL_QUARK);
    g_return_if_fail (toplevel != NULL);

    node = g_object_get_qdata (G_OBJECT (widget), NODE_QUARK);
    g_return_if_fail (node != NULL && node->parent != NULL);
    g_return_if_fail (node->type == ITEM);

    parent = node->parent;
    while (parent->type == PLACEHOLDER)
        parent = parent->parent;

    /* XXX submenu */
    update_separators (parent, toplevel);
}

static void
xml_add_item_widget (MooUIXML       *xml,
                     GtkWidget      *widget)
{
    g_signal_connect (widget, "notify::visibility",
                      G_CALLBACK (visibility_notify), xml);
}


static void
widget_destroyed (GtkWidget *widget,
                  MooUIXML  *xml)
{
    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (MOO_IS_UI_XML (xml));
    xml_remove_widget (xml, widget);
}


static void
xml_remove_widget (MooUIXML  *xml,
                   GtkWidget *widget)
{
    Toplevel *toplevel;
    Node *node;

    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (MOO_IS_UI_XML (xml));

    toplevel = g_object_get_qdata (G_OBJECT (widget), TOPLEVEL_QUARK);
    g_return_if_fail (toplevel != NULL);
    g_return_if_fail (toplevel->widget != widget);

    node = g_object_get_qdata (G_OBJECT (widget), NODE_QUARK);
    g_hash_table_remove (toplevel->children, node);

    g_object_set_qdata (G_OBJECT (widget), NODE_QUARK, NULL);
    g_object_set_qdata (G_OBJECT (widget), TOPLEVEL_QUARK, NULL);
    g_signal_handlers_disconnect_by_func (widget,
                                          (gpointer) widget_destroyed,
                                          xml);
    g_signal_handlers_disconnect_by_func (widget,
                                          (gpointer) visibility_notify,
                                          xml);
}


static void
xml_add_widget (MooUIXML  *xml,
                GtkWidget *widget,
                Toplevel  *toplevel,
                Node      *node)
{
    g_signal_connect (widget, "destroy",
                      G_CALLBACK (widget_destroyed), xml);
    g_object_set_qdata (G_OBJECT (widget), TOPLEVEL_QUARK, toplevel);
    g_object_set_qdata (G_OBJECT (widget), NODE_QUARK, node);
    g_hash_table_insert (toplevel->children, node, widget);
}


static void
prepend_value (G_GNUC_UNUSED gpointer key,
               gpointer value,
               GSList **list)
{
    *list = g_slist_prepend (*list, value);
}

static GSList*
hash_table_list_values (GHashTable *hash_table)
{
    GSList *list = NULL;
    g_hash_table_foreach (hash_table, (GHFunc) prepend_value, &list);
    return list;
}


static void
toplevel_destroyed (GtkWidget *widget,
                    MooUIXML  *xml)
{
    Toplevel *toplevel;

    g_return_if_fail (GTK_IS_WIDGET (widget));
    g_return_if_fail (MOO_IS_UI_XML (xml));

    toplevel = g_object_get_qdata (G_OBJECT (widget), TOPLEVEL_QUARK);
    g_return_if_fail (toplevel != NULL);
    g_return_if_fail (toplevel->widget == widget);

    xml_delete_toplevel (xml, toplevel);
}


static void
xml_delete_toplevel (MooUIXML *xml,
                     Toplevel *toplevel)
{
    GSList *children, *l;

    children = hash_table_list_values (toplevel->children);

    for (l = children; l != NULL; l = l->next)
    {
        GtkWidget *child = GTK_WIDGET (l->data);
        if (child != toplevel->widget)
            xml_remove_widget (xml, child);
    }

    g_signal_handlers_disconnect_by_func (toplevel->widget,
                                          (gpointer) toplevel_destroyed,
                                          xml);
    g_object_set_qdata (G_OBJECT (toplevel->widget), TOPLEVEL_QUARK, NULL);
    g_object_set_qdata (G_OBJECT (toplevel->widget), NODE_QUARK, NULL);
    xml->priv->toplevels = g_slist_remove (xml->priv->toplevels, toplevel);

    g_slist_free (children);
    toplevel_free (toplevel);
}


static void
xml_connect_toplevel (MooUIXML  *xml,
                      Toplevel  *toplevel)
{
    g_signal_connect (toplevel->widget, "destroy",
                      G_CALLBACK (toplevel_destroyed), xml);
    g_object_set_qdata (G_OBJECT (toplevel->widget), TOPLEVEL_QUARK, toplevel);
    g_object_set_qdata (G_OBJECT (toplevel->widget), NODE_QUARK, toplevel->node);
    g_hash_table_insert (toplevel->children, toplevel->node, toplevel->widget);
}


static gboolean
create_menu_separator (MooUIXML       *xml,
                       Toplevel       *toplevel,
                       GtkMenuShell   *menu,
                       Node           *node,
                       int             index)
{
    GtkWidget *item = gtk_separator_menu_item_new ();
    gtk_menu_shell_insert (menu, item, index);
    xml_add_widget (xml, item, toplevel, node);
    return TRUE;
}


static gboolean node_is_empty (Node *node)
{
    SLIST_FOREACH (node->children, l)
    {
        Node *child = l->data;

        if (child->type == SEPARATOR)
            continue;

        if (child->type == PLACEHOLDER)
        {
            if (!node_is_empty (child))
                return FALSE;
        }
        else
        {
            return FALSE;
        }
    }
    SLIST_FOREACH_END;

    return TRUE;
}


static void
create_menu_item (MooUIXML       *xml,
                  Toplevel       *toplevel,
                  GtkMenuShell   *menu,
                  Node           *node,
                  int             index)
{
    GtkWidget *menu_item = NULL;
    Item *item;

    g_return_if_fail (node != NULL && node->type == ITEM);

    item = (Item*) node;

    if (item->action)
    {
        MooAction *action;

        g_return_if_fail (toplevel->actions != NULL);

        action = moo_action_group_get_action (toplevel->actions, item->action);

        if (!action)
        {
            g_critical ("%s: could not find action '%s'",
                        G_STRLOC, item->action);
            return;
        }

        if (action->dead)
            return;

        menu_item = moo_action_create_menu_item (action);
    }
    else
    {
        if (item->stock_id)
        {
            menu_item = gtk_image_menu_item_new_from_stock (item->stock_id, NULL);
        }
        else if (item->label)
        {
            if (item->icon_stock_id)
            {
                GtkWidget *icon = gtk_image_new_from_stock (item->icon_stock_id,
                                                            GTK_ICON_SIZE_MENU);
                menu_item = gtk_image_menu_item_new_with_mnemonic (item->label);
                gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), icon);
            }
            else
            {
                menu_item = gtk_menu_item_new_with_mnemonic (item->label);
            }
        }

        if (menu_item)
            gtk_widget_show (menu_item);
    }

    g_return_if_fail (menu_item != NULL);

    gtk_menu_shell_insert (menu, menu_item, index);
    xml_add_widget (xml, menu_item, toplevel, node);
    xml_add_item_widget (xml, menu_item);

    if (!node_is_empty (node))
    {
        GtkWidget *submenu = gtk_menu_new ();
        /* XXX empty menu */
        gtk_widget_show (submenu);
        gtk_menu_set_accel_group (GTK_MENU (submenu), toplevel->accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
        fill_menu_shell (xml, toplevel, node,
                         GTK_MENU_SHELL (submenu));
    }
}


static GSList*
node_list_children (Node *parent)
{
    GSList *list = NULL, *l;

    for (l = parent->children; l != NULL; l = l->next)
    {
        GSList *tmp, *t;
        Node *node = l->data;

        switch (node->type)
        {
            case MOO_UI_NODE_ITEM:
            case MOO_UI_NODE_SEPARATOR:
                list = g_slist_prepend (list, node);
                break;
            case MOO_UI_NODE_PLACEHOLDER:
                tmp = node_list_children (node);
                for (t = tmp; t != NULL; t = t->next)
                    list = g_slist_prepend (list, t->data);
                g_slist_free (tmp);
                break;
            default:
                g_return_val_if_reached (g_slist_reverse (list));
        }
    }

    return g_slist_reverse (list);
}


static GtkWidget*
toplevel_get_widget (Toplevel  *toplevel,
                     Node      *node)
{
    return g_hash_table_lookup (toplevel->children, node);
}


static void
update_separators (Node           *parent,
                   Toplevel       *toplevel)
{
    GSList *children, *l;
    Node *separator = NULL;
    gboolean first = TRUE;
    GtkWidget *widget;

    children = node_list_children (parent);

    for (l = children; l != NULL; l = l->next)
    {
        Node *node = l->data;

        switch (node->type)
        {
            case MOO_UI_NODE_ITEM:
                widget = toplevel_get_widget (toplevel, node);

                if (!widget || !GTK_WIDGET_VISIBLE (widget))
                    continue;

                if (!first)
                {
                    if (separator)
                    {
                        GtkWidget *sep_widget = toplevel_get_widget (toplevel, separator);
                        g_return_if_fail (sep_widget != NULL);
                        gtk_widget_show (sep_widget);
                    }
                }
                else
                {
                    first = FALSE;
                    separator = NULL;
                }
                break;

            case MOO_UI_NODE_SEPARATOR:
                widget = toplevel_get_widget (toplevel, node);
                g_return_if_fail (widget != NULL);
                gtk_widget_hide (widget);
                if (!first)
                    separator = node;
                break;

            default:
                g_return_if_reached ();
        }
    }

    g_slist_free (children);
}


static gboolean
fill_menu_shell (MooUIXML       *xml,
                 Toplevel       *toplevel,
                 Node           *menu_node,
                 GtkMenuShell   *menu)
{
    GSList *children;

    children = node_list_children (menu_node);

    SLIST_FOREACH (children, l)
    {
        Node *node = l->data;

        switch (node->type)
        {
            case MOO_UI_NODE_ITEM:
                create_menu_item (xml, toplevel, menu, node, -1);
                break;
            case MOO_UI_NODE_SEPARATOR:
                create_menu_separator (xml, toplevel, menu, node, -1);
                break;

            default:
                g_warning ("%s: invalid menu item type %s",
                           G_STRLOC, NODE_TYPE_NAME[node->type]);
                return FALSE;
        }
    }
    SLIST_FOREACH_END;

    update_separators (menu_node, toplevel);

    return TRUE;
}


static gboolean
create_menu_shell (MooUIXML       *xml,
                   Toplevel       *toplevel,
                   MooUIWidgetType type)
{
    g_return_val_if_fail (toplevel != NULL, FALSE);
    g_return_val_if_fail (toplevel->widget == NULL, FALSE);
    g_return_val_if_fail (toplevel->node != NULL, FALSE);

    if (type == MOO_UI_MENUBAR)
    {
        toplevel->widget = gtk_menu_bar_new ();
    }
    else
    {
        toplevel->widget = gtk_menu_new ();
        gtk_menu_set_accel_group (GTK_MENU (toplevel->widget),
                                  toplevel->accel_group);
    }

    xml_connect_toplevel (xml, toplevel);

    return fill_menu_shell (xml, toplevel,
                            toplevel->node,
                            GTK_MENU_SHELL (toplevel->widget));
}


static gboolean
create_tool_separator (MooUIXML       *xml,
                       Toplevel       *toplevel,
                       GtkToolbar     *toolbar,
                       Node           *node,
                       int             index)
{
    GtkToolItem *item = gtk_separator_tool_item_new ();
    gtk_toolbar_insert (toolbar, item, index);
    xml_add_widget (xml, GTK_WIDGET (item), toplevel, node);
    return TRUE;
}


static gboolean
create_tool_item (MooUIXML       *xml,
                  Toplevel       *toplevel,
                  GtkToolbar     *toolbar,
                  Node           *node,
                  int             index)
{
    GtkWidget *tool_item = NULL;
    Item *item;

    g_return_val_if_fail (node != NULL && node->type == ITEM, FALSE);

    item = (Item*) node;

    if (item->action)
    {
        MooAction *action;

        g_return_val_if_fail (toplevel->actions != NULL, FALSE);

        action = moo_action_group_get_action (toplevel->actions, item->action);
        g_return_val_if_fail (action != NULL, FALSE);

        if (action->dead)
            return TRUE;

        tool_item = moo_action_create_tool_item (action, toolbar, index);
    }
    else
    {
        g_warning ("%s: implement me", G_STRLOC);
        return FALSE;
    }

    g_return_val_if_fail (tool_item != NULL, FALSE);

    xml_add_widget (xml, tool_item, toplevel, node);
    xml_add_item_widget (xml, tool_item);

    return TRUE;
}


static gboolean
fill_toolbar (MooUIXML       *xml,
              Toplevel       *toplevel,
              Node           *toolbar_node,
              GtkToolbar     *toolbar)
{
    gboolean result = TRUE;
    GSList *children;

    children = node_list_children (toolbar_node);

    SLIST_FOREACH (children, l)
    {
        Node *node = l->data;

        switch (node->type)
        {
            case MOO_UI_NODE_ITEM:
                result = create_tool_item (xml, toplevel, toolbar, node, -1);
                break;
            case MOO_UI_NODE_SEPARATOR:
                create_tool_separator (xml, toplevel, toolbar, node, -1);
                break;

            default:
                g_warning ("%s: invalid tool item type %s",
                           G_STRLOC, NODE_TYPE_NAME[node->type]);
                return FALSE;
        }

        if (!result)
            return FALSE;
    }
    SLIST_FOREACH_END;

    update_separators (toolbar_node, toplevel);

    return TRUE;
}


static gboolean
create_toolbar (MooUIXML       *xml,
                Toplevel       *toplevel)
{
    g_return_val_if_fail (toplevel != NULL, FALSE);
    g_return_val_if_fail (toplevel->widget == NULL, FALSE);
    g_return_val_if_fail (toplevel->node != NULL, FALSE);

    toplevel->widget = gtk_toolbar_new ();
    xml_connect_toplevel (xml, toplevel);

    return fill_toolbar (xml, toplevel,
                         toplevel->node,
                         GTK_TOOLBAR (toplevel->widget));
}


GtkWidget*
moo_ui_xml_create_widget (MooUIXML       *xml,
                          MooUIWidgetType type,
                          const char     *path,
                          MooActionGroup *actions,
                          GtkAccelGroup  *accel_group)
{
    Node *node;
    Toplevel *toplevel;
    gboolean result;

    g_return_val_if_fail (MOO_IS_UI_XML (xml), NULL);
    g_return_val_if_fail (path != NULL, NULL);
    g_return_val_if_fail (!actions || MOO_IS_ACTION_GROUP (actions), NULL);

    node = moo_ui_xml_get_node (xml, path);
    g_return_val_if_fail (node != NULL, NULL);

    if (node->type != WIDGET)
    {
        g_warning ("%s: can create widgets only for nodes of type %s",
                   G_STRLOC, NODE_TYPE_NAME[WIDGET]);
        return NULL;
    }

    if (type < 1 || type > 3)
    {
        g_warning ("%s: invalid widget type %d", G_STRLOC, type);
        return NULL;
    }

    toplevel = toplevel_new (node, actions, accel_group);
    xml->priv->toplevels = g_slist_append (xml->priv->toplevels, toplevel);

    switch (type)
    {
        case MOO_UI_MENUBAR:
            result = create_menu_shell (xml, toplevel, MOO_UI_MENUBAR);
            break;
        case MOO_UI_MENU:
            result = create_menu_shell (xml, toplevel, MOO_UI_MENU);
            break;
        case MOO_UI_TOOLBAR:
            result = create_toolbar (xml, toplevel);
            break;
    }

    if (!result)
    {
        xml_delete_toplevel (xml, toplevel);
        return NULL;
    }

    return toplevel->widget;
}


static Node*
effective_parent (Node *node)
{
    Node *parent;
    g_return_val_if_fail (node != NULL, NULL);
    parent = node->parent;
    while (parent && parent->type == PLACEHOLDER)
        parent = parent->parent;
    return parent;
}


static int
effective_index (Node *parent,
                 Node *node)
{
    GSList *children;
    int index;
    g_return_val_if_fail (effective_parent (node) == parent, -1);
    children = node_list_children (parent);
    index = g_slist_index (children, node);
    g_slist_free (children);
    g_return_val_if_fail (index >= 0, -1);
    return index;
}


static void
toplevel_add_node (MooUIXML *xml,
                   Toplevel *toplevel,
                   Node     *node)
{
    g_return_if_fail (GTK_IS_WIDGET (toplevel->widget));
    g_return_if_fail (node->type == ITEM);
    g_return_if_fail (node_is_ancestor (node, toplevel->node));

    if (GTK_IS_TOOLBAR (toplevel->widget))
    {
        g_return_if_fail (effective_parent (node) == toplevel->node);
        create_tool_item (xml, toplevel,
                          GTK_TOOLBAR (toplevel->widget),
                          node, effective_index (toplevel->node, node));
        update_separators (toplevel->node, toplevel);
    }
    else if (GTK_IS_MENU_SHELL (toplevel->widget))
    {
        GtkWidget *parent_widget, *menu_shell;
        Node *parent = effective_parent (node);

        g_return_if_fail (parent != NULL);
        parent_widget = toplevel_get_widget (toplevel, parent);
        g_return_if_fail (parent_widget != NULL);

        if (GTK_IS_MENU_SHELL (parent_widget))
        {
            menu_shell = parent_widget;
        }
        else
        {
            g_return_if_fail (GTK_IS_MENU_ITEM (parent_widget));
            menu_shell = gtk_menu_item_get_submenu (GTK_MENU_ITEM (parent_widget));
            if (!menu_shell)
            {
                menu_shell = gtk_menu_new ();
                gtk_widget_show (menu_shell);
                gtk_menu_item_set_submenu (GTK_MENU_ITEM (parent_widget), menu_shell);
            }
        }

        create_menu_item (xml, toplevel, GTK_MENU_SHELL (menu_shell),
                          node, effective_index (toplevel->node, node));
    }
    else
    {
        g_return_if_reached ();
    }
}


static GSList*
node_list_all_children (Node *node)
{
    GSList *list, *l;

    g_return_val_if_fail (node != NULL, NULL);

    list = g_slist_append (NULL, node);

    for (l = node->children; l != NULL; l = l->next)
        list = g_slist_append (list, node_list_all_children (l->data));

    return list;
}


static void
toplevel_remove_node (G_GNUC_UNUSED MooUIXML *xml,
                      Toplevel *toplevel,
                      Node     *node)
{
    GSList *children, *l;

    g_return_if_fail (node != toplevel->node);
    g_return_if_fail (node_is_ancestor (node, toplevel->node));

    children = node_list_all_children (node);

    for (l = children; l != NULL; l = l->next)
    {
        GtkWidget *widget = g_hash_table_lookup (toplevel->children, node);

        if (widget)
            gtk_widget_destroy (widget);
    }

    g_slist_free (children);
}


static void
update_widgets (MooUIXML       *xml,
                UpdateType      type,
                Node           *node)
{
    switch (type)
    {
        case UPDATE_ADD_NODE:
            SLIST_FOREACH (xml->priv->toplevels, l)
            {
                Toplevel *toplevel = l->data;

                if (node_is_ancestor (node, toplevel->node))
                    toplevel_add_node (xml, toplevel, node);
            }
            SLIST_FOREACH_END;
            break;

        case UPDATE_REMOVE_NODE:
            SLIST_FOREACH (xml->priv->toplevels, l)
            {
                Toplevel *toplevel = l->data;

                if (node_is_ancestor (toplevel->node, node))
                    xml_delete_toplevel (xml, toplevel);
                else if (node_is_ancestor (node, toplevel->node))
                    toplevel_remove_node (xml, toplevel, node);
            }
            SLIST_FOREACH_END;
            break;

        case UPDATE_CHANGE_NODE:
            g_warning ("%s: implement me", G_STRLOC);
            break;

        default:
            g_return_if_reached ();
    }
}


static void
moo_ui_xml_finalize (GObject *object)
{
    g_message ("%s: implement me", G_STRLOC);
    G_OBJECT_CLASS(moo_ui_xml_parent_class)->finalize (object);
}
