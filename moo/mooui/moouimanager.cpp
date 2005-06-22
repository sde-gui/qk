//
//   mooui/moouimanager.cpp
//
//   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   See COPYING file that comes with this distribution.
//

#include "mooui/moouimanager.h"
#include "mooui/moouiobject-impl.h"
#include <glib/gmessages.h>


static void moo_ui_manager_class_init       (MooUIManagerClass     *klass);
static void moo_ui_manager_ui_object_init   (MooUIObjectIface   *klass);

static void moo_ui_manager_init             (MooUIManager     *mgr);
static void moo_ui_manager_finalize         (GObject        *object);

static void moo_ui_manager_set_property     (GObject      *object,
                                             guint         prop_id,
                                             const GValue *value,
                                             GParamSpec   *pspec);
static void moo_ui_manager_get_property     (GObject      *object,
                                             guint         prop_id,
                                             GValue       *value,
                                             GParamSpec   *pspec);


enum {
    PROP_0,
    PROP_ID,
    PROP_NAME,
    PROP_ACTIONS,
    PROP_UI_XML,
    PROP_ACCEL_GROUP
};


/// MOO_TYPE_UI_MANAGER
static gpointer moo_ui_manager_parent_class = NULL;
GType moo_ui_manager_get_type ()
{
    static GType type = 0;
    if (!type)
    {
        static const GTypeInfo info = {
            sizeof (MooUIManagerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_ui_manager_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooUIManager),
            0,      /* n_preallocs */
            (GInstanceInitFunc) moo_ui_manager_init,
            NULL    /* value_table */
        };
        type = g_type_register_static (G_TYPE_OBJECT, "MooUIManager",
                                       &info, (GTypeFlags) 0);

        static const GInterfaceInfo iface_info = {
            (GInterfaceInitFunc) moo_ui_manager_ui_object_init, 0, 0
        };
        g_type_add_interface_static (type,
                                     MOO_TYPE_UI_OBJECT,
                                     &iface_info);
    }
    return type;
}


static void moo_ui_manager_class_init (MooUIManagerClass *klass)
{
    moo_ui_manager_parent_class = g_type_class_peek_parent (klass);

    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    gobject_class->finalize = moo_ui_manager_finalize;
    gobject_class->set_property = moo_ui_manager_set_property;
    gobject_class->get_property = moo_ui_manager_get_property;

    g_object_class_install_property (G_OBJECT_CLASS (klass),
                                     PROP_ID,
                                     g_param_spec_string ("id",
                                                          "id",
                                                          "id",
                                                          "",
                                                          (GParamFlags) (
                                                            G_PARAM_CONSTRUCT |
                                                            G_PARAM_READWRITE)));

    g_object_class_install_property (G_OBJECT_CLASS (klass),
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                                          "name",
                                                          "name",
                                                          "",
                                                          (GParamFlags) (
                                                            G_PARAM_CONSTRUCT |
                                                            G_PARAM_READWRITE)));

    g_object_class_install_property (G_OBJECT_CLASS (klass),
                                     PROP_ACTIONS,
                                     g_param_spec_object ("actions",
                                                          "actions",
                                                          "actions",
                                                          MOO_TYPE_ACTION_GROUP,
                                                          G_PARAM_READABLE));

    g_object_class_install_property (G_OBJECT_CLASS (klass),
                                     PROP_UI_XML,
                                     g_param_spec_object ("ui-xml",
                                                          "ui-xml",
                                                          "ui-xml",
                                                          MOO_TYPE_UI_XML,
                                                          G_PARAM_READABLE));

    g_object_class_install_property (G_OBJECT_CLASS (klass),
                                     PROP_ACCEL_GROUP,
                                     g_param_spec_object ("accel-group",
                                                          "accel-group",
                                                          "accel-group",
                                                          GTK_TYPE_ACCEL_GROUP,
                                                          (GParamFlags)G_PARAM_READWRITE));
}


static void moo_ui_manager_ui_object_init (MooUIObjectIface *klass)
{
    _moo_ui_object_base_init (klass);
}


static void moo_ui_manager_init (MooUIManager *mgr)
{
    mgr->accel_group = NULL;
}


static void moo_ui_manager_finalize       (GObject      *object)
{
    MooUIManager *mgr = MOO_UI_MANAGER (object);
    if (mgr->accel_group) g_object_unref (G_OBJECT (mgr->accel_group));
    mgr->accel_group = NULL;
    if (mgr->tooltips) g_object_unref (G_OBJECT (mgr->tooltips));
    mgr->tooltips = NULL;
    G_OBJECT_CLASS (moo_ui_manager_parent_class)->finalize (object);
}


static void moo_ui_manager_set_property       (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
    switch (prop_id)
    {
        case PROP_ACCEL_GROUP:
            moo_ui_manager_set_accel_group (MOO_UI_MANAGER (object),
                                            GTK_ACCEL_GROUP (g_value_get_object (value)));
            break;

        case PROP_ID:
            _moo_ui_object_set_id (MOO_UI_OBJECT (object), g_value_get_string (value));
            break;

        case PROP_NAME:
            _moo_ui_object_set_name (MOO_UI_OBJECT (object), g_value_get_string (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void moo_ui_manager_get_property       (GObject      *object,
                                               guint         prop_id,
                                               GValue       *value,
                                               GParamSpec   *pspec)
{
    switch (prop_id)
    {
        case PROP_ACCEL_GROUP:
            g_value_set_object (value,
                                moo_ui_manager_get_accel_group (MOO_UI_MANAGER (object)));
            break;

        case PROP_ACTIONS:
            g_value_set_object (value,
                                moo_ui_object_get_actions (MOO_UI_OBJECT (object)));
            break;

        case PROP_UI_XML:
            g_value_set_object (value,
                                moo_ui_object_get_ui_xml (MOO_UI_OBJECT (object)));
            break;

        case PROP_ID:
            g_value_set_string (value, _moo_ui_object_get_id (MOO_UI_OBJECT (object)));
            break;

        case PROP_NAME:
            g_value_set_string (value, _moo_ui_object_get_name (MOO_UI_OBJECT (object)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


MooUIManager *moo_ui_manager_new               (void)
{
    return MOO_UI_MANAGER (g_object_new (MOO_TYPE_UI_MANAGER, NULL));
}


MooActionGroup *moo_ui_manager_get_actions (MooUIManager   *mgr)
{
    return moo_ui_object_get_actions (MOO_UI_OBJECT (mgr));
}


gboolean        moo_ui_manager_add_ui_from_string   (MooUIManager   *mgr,
                                                     const char     *ui,
                                                     int             len,
                                                     GError        **error)
{
    g_return_val_if_fail (MOO_IS_UI_MANAGER (mgr) && ui != NULL, false);
    MooUIXML *xml = moo_ui_object_get_ui_xml (MOO_UI_OBJECT (mgr));
    return moo_ui_xml_add_ui_from_string (xml, ui, len, error);
}


GtkWidget      *moo_ui_manager_create_widget        (MooUIManager   *mgr,
                                                     const char     *path)
{
    g_return_val_if_fail (MOO_IS_UI_MANAGER (mgr) && path != NULL, NULL);
    MooUIXML *xml = moo_ui_object_get_ui_xml (MOO_UI_OBJECT (mgr));
    MooActionGroup *actions = moo_ui_object_get_actions (MOO_UI_OBJECT (mgr));
    return moo_ui_xml_create_widget (xml, path, actions,
                                     moo_ui_manager_get_accel_group (mgr),
                                     moo_ui_manager_get_tooltips (mgr));
}


void            moo_ui_manager_set_accel_group      (MooUIManager   *mgr,
                                                     GtkAccelGroup  *accel_group)
{
    g_return_if_fail (MOO_IS_UI_MANAGER (mgr) && GTK_IS_ACCEL_GROUP (accel_group));
    if (mgr->accel_group == accel_group) return;
    if (mgr->accel_group) g_object_unref (G_OBJECT (mgr->accel_group));
    mgr->accel_group = accel_group;
    if (mgr->accel_group) g_object_ref (G_OBJECT (mgr->accel_group));
    MooActionGroup *actions = moo_ui_object_get_actions (MOO_UI_OBJECT (mgr));
    moo_action_group_set_accel_group (actions, accel_group);
}


GtkAccelGroup  *moo_ui_manager_get_accel_group      (MooUIManager   *mgr)
{
    g_return_val_if_fail (MOO_IS_UI_MANAGER (mgr), NULL);
    if (!mgr->accel_group)
        moo_ui_manager_set_accel_group (mgr, gtk_accel_group_new ());
    return mgr->accel_group;
}


void            moo_ui_manager_set_tooltips      (MooUIManager   *mgr,
                                                  GtkTooltips    *tooltips)
{
    g_return_if_fail (MOO_IS_UI_MANAGER (mgr) && GTK_IS_TOOLTIPS (tooltips));
    if (mgr->tooltips == tooltips) return;
    if (mgr->tooltips) g_object_unref (G_OBJECT (mgr->tooltips));
    mgr->tooltips = tooltips;
    if (mgr->tooltips) g_object_ref (G_OBJECT (mgr->tooltips));
}


GtkTooltips    *moo_ui_manager_get_tooltips      (MooUIManager   *mgr)
{
    g_return_val_if_fail (MOO_IS_UI_MANAGER (mgr), NULL);
    if (!mgr->tooltips) mgr->tooltips = gtk_tooltips_new ();
    return mgr->tooltips;
}
