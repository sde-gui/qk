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


static void moo_ui_xml_class_init       (MooUIXMLClass  *klass);

static void moo_ui_xml_init             (MooUIXML       *xml);
static void moo_ui_xml_finalize         (GObject        *object);

static void moo_ui_xml_set_property     (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static void moo_ui_xml_get_property     (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec);

static void moo_ui_xml_set_ui           (MooUIXML       *xml,
                                         const char     *ui);
static void moo_ui_xml_set_markup       (MooUIXML       *xml,
                                         MooMarkupDoc   *doc);


enum {
    PROP_0,
    PROP_UI,
    PROP_MARKUP
};

enum {
    CHANGED,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_UI_XML */
G_DEFINE_TYPE (MooUIXML, moo_ui_xml, G_TYPE_OBJECT)


static void moo_ui_xml_class_init (MooUIXMLClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_ui_xml_finalize;
    gobject_class->set_property = moo_ui_xml_set_property;
    gobject_class->get_property = moo_ui_xml_get_property;

    g_object_class_install_property (gobject_class,
                                     PROP_UI,
                                     g_param_spec_string ("ui",
                                                          "ui",
                                                          "ui",
                                                          NULL,
                                                          (GParamFlags) (
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_CONSTRUCT)));

    g_object_class_install_property (gobject_class,
                                     PROP_MARKUP,
                                     g_param_spec_boxed ("markup",
                                                         "markup",
                                                         "markup",
                                                         MOO_TYPE_MARKUP_DOC,
                                                         (GParamFlags) (
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_CONSTRUCT)));

    signals[CHANGED] =
        g_signal_new ("changed",
                      G_OBJECT_CLASS_TYPE (klass),
                      (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                      G_STRUCT_OFFSET (MooUIXMLClass, changed),
                      NULL, NULL,
                      _moo_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}


static void moo_ui_xml_init (MooUIXML *xml)
{
    xml->doc = NULL;
}


static void moo_ui_xml_finalize       (GObject      *object)
{
    MooUIXML *xml = MOO_UI_XML (object);

    if (xml->doc)
        moo_markup_doc_unref (xml->doc);
    xml->doc = NULL;

    G_OBJECT_CLASS (moo_ui_xml_parent_class)->finalize (object);
}


static void moo_ui_xml_get_property     (GObject        *object,
                                         guint           prop_id,
                                         GValue         *value,
                                         GParamSpec     *pspec)
{
    MooUIXML *xml = MOO_UI_XML (object);

    switch (prop_id)
    {
        case PROP_UI:
            g_value_set_string (value,
                                moo_ui_xml_get_ui (xml));
            break;

        case PROP_MARKUP:
            g_value_set_boxed (value,
                               moo_ui_xml_get_markup (xml));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_ui_xml_set_property     (GObject        *object,
                                         guint           prop_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec)
{
    MooUIXML *xml = MOO_UI_XML (object);

    switch (prop_id)
    {
        case PROP_UI:
            moo_ui_xml_set_ui (xml, g_value_get_string (value));
            break;

        case PROP_MARKUP:
            if (g_value_get_boxed (value))
                moo_ui_xml_set_markup (xml, MOO_MARKUP_DOC (g_value_get_boxed (value)));
            else
                moo_ui_xml_set_markup (xml, NULL);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void moo_ui_xml_set_ui           (MooUIXML       *xml,
                                         const char     *ui)
{
    if (!ui)
    {
        if (xml->doc)
            moo_markup_doc_unref (xml->doc);
        xml->doc = NULL;
    }
    else
    {
        if (!xml->doc)
        {
            GError *err = NULL;
            xml->doc = moo_markup_parse_memory (ui, -1, &err);
            if (!xml->doc)
            {
                g_critical ("moo_ui_xml_set_ui: could not parse markup\n%s", ui);
                if (err)
                {
                    g_critical ("%s", err->message);
                    g_error_free (err);
                }
            }
        }
        else
        {
            g_critical ("%s: implement me", G_STRLOC);
        }
    }
}


static void moo_ui_xml_set_markup       (MooUIXML       *xml,
                                         MooMarkupDoc   *doc)
{
    if (doc == xml->doc)
        return;

    if (xml->doc)
        moo_markup_doc_unref (xml->doc);

    xml->doc = doc;

    if (xml->doc)
        moo_markup_doc_ref (xml->doc);
}


char            *moo_ui_xml_get_ui              (MooUIXML       *xml)
{
    g_return_val_if_fail (MOO_IS_UI_XML (xml), NULL);

    if (!xml->doc)
        return NULL;
    else
        return moo_markup_node_get_string (MOO_MARKUP_NODE (xml->doc));
}


const MooMarkupDoc *moo_ui_xml_get_markup       (MooUIXML       *xml)
{
    g_return_val_if_fail (MOO_IS_UI_XML (xml), NULL);
    return xml->doc;
}


gboolean         moo_ui_xml_add_ui_from_string  (MooUIXML       *xml,
                                                 const char     *ui,
                                                 int             len,
                                                 GError        **error)
{
    GError *err = NULL;

    g_return_val_if_fail (MOO_IS_UI_XML (xml) && ui != NULL, FALSE);

    if (xml->doc)
    {
        g_critical ("%s: implement me", G_STRLOC);
        return FALSE;
    }

    xml->doc = moo_markup_parse_memory (ui, len, &err);

    if (xml->doc)
    {
        g_clear_error (error);
        g_signal_emit (xml, signals[CHANGED], 0);
        return TRUE;
    }
    else
    {
        if (err)
            g_propagate_error (error, err);

        return FALSE;
    }
}


gboolean         moo_ui_xml_add_ui_from_file    (MooUIXML       *xml,
                                                 const char     *file,
                                                 GError        **error)
{
    g_return_val_if_fail (MOO_IS_UI_XML (xml) && file != NULL, FALSE);

    if (xml->doc)
    {
        g_critical ("%s: implement me", G_STRLOC);
        return FALSE;
    }

    xml->doc = moo_markup_parse_file (file, error);

    if (xml->doc)
    {
        g_signal_emit (xml, signals[CHANGED], 0);
        return TRUE;
    }

    return FALSE;
}


static void erase_container (GtkContainer *container)
{
    GList *children = gtk_container_get_children (container);
    GList *l;
    for (l = children; l; l = l->next)
        gtk_container_remove (container, GTK_WIDGET (l->data));
    g_list_free (children);
}


static GtkWidget *create_widget     (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkWidget          *widget);
static GtkWidget *create_menu_bar   (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenuBar         *menubar);
static GtkWidget *create_menu       (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenu            *menu);
static gboolean create_menu_item    (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenuShell       *menu_shell,
                                     int                 position);
static GtkWidget *create_toolbar    (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkToolbar         *toolbar);
static gboolean   create_tool_item  (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkToolbar         *toolbar,
                                     int                 position);

GtkWidget       *moo_ui_xml_create_widget       (MooUIXML       *xml,
                                                 const char     *path,
                                                 MooActionGroup *actions,
                                                 GtkAccelGroup  *accel_group,
                                                 GtkTooltips    *tooltips)
{
    MooMarkupElement *ui, *node;

    g_return_val_if_fail (MOO_IS_UI_XML (xml) && path != NULL, NULL);
    g_return_val_if_fail (xml->doc != NULL, NULL);

    ui = moo_markup_get_root_element (xml->doc, "ui");
    g_return_val_if_fail (ui != NULL, NULL);
    node = moo_markup_get_element (MOO_MARKUP_NODE (ui), path);
    g_return_val_if_fail (node != NULL, NULL);

    return create_widget (node, actions, accel_group, tooltips, NULL);
}

gboolean         moo_ui_xml_has_widget          (MooUIXML       *xml,
                                                 const char     *path)
{
    MooMarkupElement *ui, *node;

    g_return_val_if_fail (MOO_IS_UI_XML (xml) && path != NULL, FALSE);

    if (!xml->doc) return FALSE;

    ui = moo_markup_get_root_element (xml->doc, "ui");
    g_return_val_if_fail (ui != NULL, FALSE);
    node = moo_markup_get_element (MOO_MARKUP_NODE (ui), path);
    return node != NULL;
}


static GtkWidget *create_widget (MooMarkupElement   *node,
                                 MooActionGroup     *actions,
                                 GtkAccelGroup      *accel_group,
                                 GtkTooltips        *tooltips,
                                 GtkWidget          *widget)
{
    if (!g_ascii_strcasecmp (node->name, "menubar")) {
        g_return_val_if_fail (!widget || GTK_IS_MENU_BAR (widget), NULL);
        return create_menu_bar (node, actions, accel_group, tooltips, GTK_MENU_BAR (widget));
    }
    else if (!g_ascii_strcasecmp (node->name, "menu")) {
        g_return_val_if_fail (!widget || GTK_IS_MENU (widget), NULL);
        return create_menu (node, actions, accel_group, tooltips, GTK_MENU (widget));
    }
    else if (!g_ascii_strcasecmp (node->name, "toolbar")) {
        g_return_val_if_fail (!widget || GTK_IS_TOOLBAR (widget), NULL);
        return create_toolbar (node, actions, accel_group, tooltips, GTK_TOOLBAR (widget));
    }
    else
    {
        g_critical ("cannot create widget '%s'", node->name);
        return NULL;
    }
}


GtkWidget       *moo_ui_xml_update_widget       (MooUIXML       *xml,
                                                 GtkWidget      *widget,
                                                 const char     *path,
                                                 MooActionGroup *actions,
                                                 GtkAccelGroup  *accel_group,
                                                 GtkTooltips    *tooltips)
{
    MooMarkupElement *ui, *node;

    g_return_val_if_fail (MOO_IS_UI_XML (xml) && path != NULL, NULL);
    g_return_val_if_fail (xml->doc != NULL, NULL);

    ui = moo_markup_get_root_element (xml->doc, "ui");
    g_return_val_if_fail (ui != NULL, NULL);
    node = moo_markup_get_element (MOO_MARKUP_NODE (ui), path);
    g_return_val_if_fail (node != NULL, NULL);

    return create_widget (node, actions, accel_group, tooltips, widget);
}


static GtkWidget *create_menu_bar   (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenuBar         *menubar)
{
    MooMarkupNode *child;

    if (menubar)
        erase_container (GTK_CONTAINER (menubar));
    else
        menubar = GTK_MENU_BAR (gtk_menu_bar_new ());

    for (child = node->children; child != NULL; child = child->next)
    {
        if (MOO_MARKUP_IS_ELEMENT (child))
        {
            if (!create_menu_item (MOO_MARKUP_ELEMENT (child),
                                   actions,
                                   accel_group, tooltips,
                                   GTK_MENU_SHELL (menubar), -1))
            {
                g_return_val_if_reached (GTK_WIDGET (menubar));
            }
        }
    }
    return GTK_WIDGET (menubar);
}


static GtkWidget *create_menu       (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenu            *menu)
{
    gboolean start = TRUE;
    gboolean need_separator = FALSE;
    MooMarkupNode *ch;

    if (menu)
        erase_container (GTK_CONTAINER (menu));
    else
        menu = GTK_MENU (gtk_menu_new ());

    gtk_menu_set_accel_group (GTK_MENU (menu), accel_group);

    for (ch = node->children; ch != NULL; ch = ch->next)
        if (MOO_MARKUP_IS_ELEMENT (ch))
        {
            MooMarkupElement *child = MOO_MARKUP_ELEMENT (ch);

            if (!g_ascii_strcasecmp (child->name, "menu") ||
                !g_ascii_strcasecmp (child->name, "item"))
            {
                const char *action_name = moo_markup_get_prop (child, "action");

                if (action_name)
                {
                    MooAction *action = moo_action_group_get_action (actions, action_name);

                    if (!action)
                    {
                        g_critical ("could not find action '%s'", action_name);
                        continue;
                    }

                    if (action->dead)
                        continue;
                }

                if (need_separator)
                {
                    GtkWidget *sep = gtk_separator_menu_item_new ();
                    gtk_widget_show (sep);
                    gtk_menu_shell_append (GTK_MENU_SHELL (menu), sep);
                    need_separator = FALSE;
                }

                if (!create_menu_item (child, actions,
                                       accel_group, tooltips,
                                       GTK_MENU_SHELL (menu), -1))
                {
                    g_return_val_if_reached (GTK_WIDGET (menu));
                }

                start = FALSE;
            }
            else if (!g_ascii_strcasecmp (child->name, "placeholder"))
            {
            }
            else if (!g_ascii_strcasecmp (child->name, "separator"))
            {
                if (!start)
                    need_separator = TRUE;
            }
            else
            {
                g_critical ("unknown node %s\n", child->name);
                return GTK_WIDGET (menu);
            }
        }

    return GTK_WIDGET (menu);
}


static gboolean create_menu_item    (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkMenuShell       *menu_shell,
                                     int                 position)
{
    GtkWidget *menuitem = NULL;
    const char *action_name = moo_markup_get_prop (node, "action");
    MooAction *action = NULL;

    if (action_name)
    {
        action = moo_action_group_get_action (actions, action_name);
        if (!action) g_critical ("could not find action '%s'", action_name);
    }

    if (action)
    {
        if (action->dead)
            return TRUE;

        menuitem = moo_action_create_menu_item (action, menu_shell, position);
    }
    else
    {
        const char *stock_id = moo_markup_get_prop (node, "stock");
        if (stock_id)
        {
            menuitem = gtk_image_menu_item_new_from_stock (stock_id, NULL);
        }
        else
        {
            const char *label = moo_markup_get_prop (node, "label");
            if (!label) {
                g_warning ("could not get label for menu");
                label = moo_markup_get_prop (node, "name");
                if (!label) {
                    g_warning ("using node name as label");
                    label = moo_markup_get_prop (node, "name");
                    if (!label) label = "";
                }
            }
            if (!label)
                menuitem = gtk_menu_item_new ();
            else
                menuitem = gtk_menu_item_new_with_mnemonic (label);
        }

        if (menuitem)
        {
            gtk_widget_show (menuitem);
            if (position >= 0)
                gtk_menu_shell_insert (menu_shell, menuitem, position);
            else
                gtk_menu_shell_append (menu_shell, menuitem);
        }
    }

    if (!menuitem)
        return FALSE;

    if (!g_ascii_strcasecmp (node->name, "menu"))
    {
        GtkWidget *menu = create_menu (node, actions, accel_group, tooltips, NULL);
        gtk_widget_show (menu);
        g_return_val_if_fail (menu != NULL, FALSE);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);
    }

    return TRUE;
}


static GtkWidget *create_toolbar    (MooMarkupElement   *node,
                                     MooActionGroup     *actions,
                                     GtkAccelGroup      *accel_group,
                                     GtkTooltips        *tooltips,
                                     GtkToolbar         *toolbar)
{
    gboolean start = TRUE;
    gboolean need_separator = FALSE;
    MooMarkupNode *ch;

    if (toolbar)
        erase_container (GTK_CONTAINER (toolbar));
    else
        toolbar = GTK_TOOLBAR (gtk_toolbar_new ());

    for (ch = node->children; ch != NULL; ch = ch->next)
    {
        MooMarkupElement *child;

        if (!MOO_MARKUP_IS_ELEMENT (ch))
            continue;

        child = MOO_MARKUP_ELEMENT (ch);

        if (!g_ascii_strcasecmp (child->name, "item"))
        {
            const char *action_name = moo_markup_get_prop (child, "action");

            if (action_name)
            {
                MooAction *action = moo_action_group_get_action (actions, action_name);

                if (!action)
                {
                    g_critical ("could not find action '%s'", action_name);
                    continue;
                }

                if (action->dead)
                    continue;
            }

            if (need_separator)
            {
#if GTK_CHECK_VERSION(2,4,0)
                GtkToolItem *sep = gtk_separator_tool_item_new ();
                gtk_widget_show (GTK_WIDGET (sep));
                gtk_toolbar_insert (toolbar, sep, -1);
#else /* !GTK_CHECK_VERSION(2,4,0) */
                gtk_toolbar_append_space (toolbar);
#endif /* !GTK_CHECK_VERSION(2,4,0) */
                need_separator = FALSE;
            }
            create_tool_item (child, actions, accel_group, tooltips, toolbar, -1);
            start = FALSE;
        }
        else if (!g_ascii_strcasecmp (child->name, "separator"))
        {
            if (!start) need_separator = TRUE;
        }
        else
            g_critical ("unknown toolbar item '%s'", child->name);
    }

    return GTK_WIDGET (toolbar);
}


static gboolean     create_tool_item  (MooMarkupElement             *node,
                                       MooActionGroup               *actions,
                                       G_GNUC_UNUSED GtkAccelGroup  *accel_group,
                                       G_GNUC_UNUSED GtkTooltips    *tooltips,
                                       GtkToolbar                   *toolbar,
                                       int                           position)
{
    const char *action_name = moo_markup_get_prop (node, "action");
    const char *label, *tip;
#if GTK_CHECK_VERSION(2,4,0)
    GtkToolItem *item;
#else /* !GTK_CHECK_VERSION(2,4,0) */
    GtkWidget *item;
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    if (action_name)
    {
        MooAction *action = moo_action_group_get_action (actions, action_name);
        if (!action)
        {
            g_critical ("could not find action '%s'", action_name);
        }
        else
        {
            if (action->dead) return TRUE;
            if (!moo_action_create_tool_item (action, toolbar, position))
                g_critical ("could not create tool item for action %s", action_name);
            else
                return TRUE;
        }
    }

    label = moo_markup_get_prop (node, "label");
    if (!label) {
        g_warning ("could not get label for toolbar button");
        label = moo_markup_get_prop (node, "name");
        if (!label) {
            g_warning ("using node name as label");
            label = moo_markup_get_prop (node, "name");
            if (!label) label = "";
        }
    }

    tip = moo_markup_get_prop (node, "tooltip");

#if GTK_CHECK_VERSION(2,4,0)
    item = gtk_tool_button_new (NULL, label ? label : "");
    if (tip) gtk_tool_item_set_tooltip (item, tooltips, tip, tip);
    gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (item), TRUE);
    gtk_widget_show (GTK_WIDGET (item));
    gtk_toolbar_insert (toolbar, item, position);
    gtk_container_child_set (GTK_CONTAINER (toolbar), GTK_WIDGET (item),
                             "homogeneous", FALSE, NULL);
#else /* !GTK_CHECK_VERSION(2,4,0) */
    item = gtk_toolbar_insert_item (toolbar, label ? label : "",
                                    tip, tip,
                                    NULL, NULL, NULL, position);
    gtk_button_set_use_underline (GTK_BUTTON (item), TRUE);
#endif /* !GTK_CHECK_VERSION(2,4,0) */

    return TRUE;
}


MooUIXML        *moo_ui_xml_new                 (void)
{
    return MOO_UI_XML (g_object_new (MOO_TYPE_UI_XML, NULL));
}


MooUIXML        *moo_ui_xml_new_from_string     (const char     *xml,
                                                 GError        **error)
{
    GError *err = NULL;
    MooMarkupDoc *doc;

    g_return_val_if_fail (xml != NULL, NULL);
    doc = moo_markup_parse_memory (xml, -1, &err);
    if (!doc) {
        if (err) {
            if (error) *error = err;
            else g_error_free (err);
        }
        else if (error)
            *error = NULL;
        return NULL;
    }

    return MOO_UI_XML (g_object_new (MOO_TYPE_UI_XML,
                                     "markup", doc,
                                     NULL));
}


MooUIXML        *moo_ui_xml_new_from_file       (const char     *file,
                                                 GError        **error)
{
    GError *err = NULL;
    MooMarkupDoc *doc;

    g_return_val_if_fail (file != NULL, NULL);

    doc = moo_markup_parse_file (file, &err);
    if (!doc) {
        if (err) {
            if (error) *error = err;
            else g_error_free (err);
        }
        else if (error)
            *error = NULL;
        return NULL;
    }
    return MOO_UI_XML (g_object_new (MOO_TYPE_UI_XML,
                                     "markup", doc,
                                     NULL));
}
