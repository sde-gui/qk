/*
 *   mooui/moowindow.c
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

#include "mooui/moowindow.h"
#include "mooui/moouiobject-impl.h"
#include "mooui/mooshortcutsprefs.h"
#include "mooui/mootoggleaction.h"
#include "mooui/moomenuaction.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include <gtk/gtk.h>


#define MOO_WINDOW_PREFS_SAVE_POSITION  "window_save_position"
#define MOO_WINDOW_PREFS_X              "window_x"
#define MOO_WINDOW_PREFS_Y              "window_y"
#define MOO_WINDOW_PREFS_SAVE_SIZE      "window_save_size"
#define MOO_WINDOW_PREFS_WIDTH          "window_width"
#define MOO_WINDOW_PREFS_HEIGHT         "window_height"
#define MOO_WINDOW_PREFS_SHOW_TOOLBAR   "window_show_toolbar"
#define MOO_WINDOW_PREFS_SHOW_MENUBAR   "window_show_menubar"
#define MOO_WINDOW_PREFS_TOOLBAR_STYLE  "window_toolbar_style"

inline static const char *setting (MooWindow *window, const char *s)
{
    static char *string = NULL;

    const char *id = moo_ui_object_get_id (MOO_UI_OBJECT (window));

    g_free (string);
    if (id)
        return string = g_strdup_printf ("%s::%s", id, s);
    else
        return string = g_strdup (s);
}


struct _MooWindowPrivate {
    int update_id;
    char *toolbar_ui_name;
    char *menubar_ui_name;
};


static void moo_window_class_init               (MooWindowClass *klass);
GObject    *moo_window_constructor              (GType                  type,
                                                 guint                  n_props,
                                                 GObjectConstructParam *props);

static void moo_window_init                     (MooWindow      *window);
static void moo_window_finalize                 (GObject        *object);

static void moo_window_set_property             (GObject        *object,
                                                 guint           prop_id,
                                                 const GValue   *value,
                                                 GParamSpec     *pspec);
static void moo_window_get_property             (GObject        *object,
                                                 guint           prop_id,
                                                 GValue         *value,
                                                 GParamSpec     *pspec);

static gboolean moo_window_delete_event         (GtkWidget      *widget,
                                                 GdkEventAny    *event);

static void moo_window_realize                  (MooWindow      *window);
static void moo_window_save_settings            (MooWindow      *window);

static void moo_window_shortcuts_prefs_dialog   (MooWindow      *window);

static void moo_window_ui_changed               (MooWindow      *window);
static gboolean update_ui_callback              (MooWindow      *window);

static void moo_window_show_menubar_toggled     (MooWindow      *window,
                                                 gboolean        show);
static void moo_window_show_toolbar_toggled     (MooWindow      *window,
                                                 gboolean        show);

static GtkMenuItem *moo_window_create_toolbar_style_menu (MooWindow *window);


enum {
    PROP_0,
    PROP_ACCEL_GROUP,
    PROP_MENUBAR_UI_NAME,
    PROP_TOOLBAR_UI_NAME
};

enum {
    CLOSE,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_WINDOW */
static gpointer moo_window_parent_class = NULL;
GType moo_window_get_type (void)
{
    static GType type = 0;
    if (!type)
    {
        static const GTypeInfo info = {
            sizeof (MooWindowClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) moo_window_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,   /* class_data */
            sizeof (MooWindow),
            0,      /* n_preallocs */
            (GInstanceInitFunc) moo_window_init,
            NULL    /* value_table */
        };

        static const GInterfaceInfo iface_info = {
            NULL,
            NULL,
            NULL
        };

        type = g_type_register_static (GTK_TYPE_WINDOW, "MooWindow",
                                       &info, (GTypeFlags) 0);

        g_type_add_interface_static (type,
                                     MOO_TYPE_UI_OBJECT,
                                     &iface_info);
    }
    return type;
}


static void moo_window_class_init (MooWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    moo_window_parent_class = g_type_class_peek_parent (klass);

    gobject_class->constructor = moo_window_constructor;
    gobject_class->finalize = moo_window_finalize;
    gobject_class->set_property = moo_window_set_property;
    gobject_class->get_property = moo_window_get_property;

    widget_class->delete_event = moo_window_delete_event;

    moo_ui_object_class_init (gobject_class, "MooWindow", "Window");

    moo_ui_object_class_new_action (gobject_class,
                                    "id", "ConfigureShortcuts",
                                    "name", "Configure Shortcuts",
                                    "label", "Configure _Shortcuts...",
                                    "tooltip", "Configure _Shortcuts...",
                                    "icon-stock-id", MOO_STOCK_KEYBOARD,
                                    "closure::callback", moo_window_shortcuts_prefs_dialog,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                    "id", "ShowToolbar",
                                    "name", "Show Toolbar",
                                    "label", "Show Toolbar",
                                    "tooltip", "Show Toolbar",
                                    "toggled-callback", moo_window_show_toolbar_toggled,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                    "id", "ShowMenubar",
                                    "name", "Show Menubar",
                                    "label", "Show Menubar",
                                    "tooltip", "Show Menubar",
                                    "toggled-callback", moo_window_show_menubar_toggled,
                                    NULL);

    moo_ui_object_class_new_action (gobject_class,
                                    "action-type::", MOO_TYPE_MENU_ACTION,
                                    "id", "ToolbarStyle",
                                    "name", "Toolbar Style",
                                    "create-menu-func", moo_window_create_toolbar_style_menu,
                                    NULL);

    g_object_class_install_property (gobject_class,
                                     PROP_ACCEL_GROUP,
                                     g_param_spec_object ("accel-group",
                                                          "accel-group",
                                                          "accel-group",
                                                          GTK_TYPE_ACCEL_GROUP,
                                                          G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_MENUBAR_UI_NAME,
                                     g_param_spec_string ("menubar-ui-name",
                                                          "menubar-ui-name",
                                                          "menubar-ui-name",
                                                          "Menubar",
                                                          G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TOOLBAR_UI_NAME,
                                     g_param_spec_string ("toolbar-ui-name",
                                                          "toolbar-ui-name",
                                                          "toolbar-ui-name",
                                                          "Toolbar",
                                                          G_PARAM_READWRITE));

    signals[CLOSE] =
        g_signal_new ("close",
                      G_OBJECT_CLASS_TYPE (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MooWindowClass, close),
                      g_signal_accumulator_true_handled, NULL,
                      _moo_marshal_BOOL__VOID,
                      G_TYPE_BOOLEAN, 0);
}


GObject    *moo_window_constructor      (GType                  type,
                                         guint                  n_props,
                                         GObjectConstructParam *props)
{
    GtkWidget *vbox;
    MooActionGroup *actions;
    MooWindow *window;

    GObject *object =
        G_OBJECT_CLASS(moo_window_parent_class)->constructor (type, n_props, props);

    moo_ui_object_init (MOO_UI_OBJECT (object));

    window = MOO_WINDOW (object);
    window->accel_group = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (window),
                                window->accel_group);
    window->tooltips = gtk_tooltips_new ();

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (window), vbox);
    window->menubar = gtk_menu_bar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), window->menubar, FALSE, FALSE, 0);
    window->toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), window->toolbar, FALSE, FALSE, 0);
    window->vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), window->vbox, TRUE, TRUE, 0);
    window->statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), window->statusbar, FALSE, FALSE, 0);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (window->statusbar), FALSE);

    actions = moo_ui_object_get_actions (MOO_UI_OBJECT (window));
    moo_action_group_set_accel_group (actions, window->accel_group);
    moo_action_group_set_tooltips  (actions, window->tooltips);

    g_signal_connect_swapped (moo_ui_object_get_ui_xml (MOO_UI_OBJECT (window)),
                              "changed",
                              G_CALLBACK (moo_window_ui_changed),
                              window);
    moo_window_ui_changed (window);

    g_signal_connect (window, "realize",
                      G_CALLBACK (moo_window_realize), NULL);
    return object;
}


static void moo_window_init (MooWindow *window)
{
    window->priv = g_new0 (MooWindowPrivate, 1);
    window->priv->menubar_ui_name = NULL;
    window->priv->toolbar_ui_name = NULL;
}


static void moo_window_finalize       (GObject      *object)
{
    MooWindow *window = MOO_WINDOW(object);

    if (window->priv->update_id > 0)
        g_source_remove (window->priv->update_id);
    g_free (window->priv->menubar_ui_name);
    g_free (window->priv->toolbar_ui_name);
    g_free (window->priv);

    if (window->accel_group)
        g_object_unref (window->accel_group);
    if (window->tooltips)
        g_object_unref (window->tooltips);

    g_signal_handlers_disconnect_by_func (moo_ui_object_get_ui_xml (MOO_UI_OBJECT (window)),
                                          (gpointer) moo_window_ui_changed, window);

    G_OBJECT_CLASS (moo_window_parent_class)->finalize (object);
}


static gboolean moo_window_delete_event     (GtkWidget      *widget,
                                             G_GNUC_UNUSED GdkEventAny    *event)
{
    gboolean result = FALSE;
    g_signal_emit_by_name (widget, "close", &result);
    if (!result) moo_window_save_settings (MOO_WINDOW (widget));
    return result;
}


void     moo_window_realize (MooWindow *window)
{
    MooAction *show_toolbar, *show_menubar;
    GtkToolbarStyle style;

    if (moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SAVE_POSITION))) {
        int x = moo_prefs_get_int (setting (window, MOO_WINDOW_PREFS_X));
        int y = moo_prefs_get_int (setting (window, MOO_WINDOW_PREFS_Y));
        gtk_window_move (GTK_WINDOW (window), x, y);
    }
    if (moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SAVE_SIZE))) {
        int width = moo_prefs_get_int (setting (window, MOO_WINDOW_PREFS_WIDTH));
        int height = moo_prefs_get_int (setting (window, MOO_WINDOW_PREFS_HEIGHT));
        if (width > 0 && height > 0)
            gtk_window_set_default_size (GTK_WINDOW (window), width, height);
    }

    moo_prefs_set_bool_if_not_set_ignore_change (
            setting (window, MOO_WINDOW_PREFS_SHOW_MENUBAR), TRUE);
    moo_prefs_set_bool_if_not_set_ignore_change (
            setting (window, MOO_WINDOW_PREFS_SHOW_TOOLBAR), TRUE);
    moo_prefs_set_if_not_set_ignore_change (setting (window, MOO_WINDOW_PREFS_TOOLBAR_STYLE),
                                            "GTK_TOOLBAR_ICONS");

    show_menubar =
            moo_action_group_get_action (moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
                                         "ShowMenubar");
    show_toolbar =
            moo_action_group_get_action (moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
                                         "ShowToolbar");
    moo_toggle_action_set_active (MOO_TOGGLE_ACTION (show_menubar),
                                  moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SHOW_MENUBAR)));
    moo_toggle_action_set_active (MOO_TOGGLE_ACTION (show_toolbar),
                                  moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SHOW_TOOLBAR)));

    style = moo_prefs_get_enum (setting (window, MOO_WINDOW_PREFS_TOOLBAR_STYLE),
                                GTK_TYPE_TOOLBAR_STYLE);
    gtk_toolbar_set_style (GTK_TOOLBAR (MOO_WINDOW(window)->toolbar), style);
}


static void moo_window_save_settings    (MooWindow      *window)
{
    if (moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SAVE_POSITION))) {
        int x, y;
        gtk_window_get_position (GTK_WINDOW (window), &x, &y);
        moo_prefs_set_int (setting (window, MOO_WINDOW_PREFS_X), x);
        moo_prefs_set_int (setting (window, MOO_WINDOW_PREFS_Y), y);
    }
    if (moo_prefs_get_bool (setting (window, MOO_WINDOW_PREFS_SAVE_SIZE))) {
        int width, height;
        gtk_window_get_size (GTK_WINDOW (window), &width, &height);
        moo_prefs_set_int (setting (window, MOO_WINDOW_PREFS_WIDTH), width);
        moo_prefs_set_int (setting (window, MOO_WINDOW_PREFS_HEIGHT), height);
    }
}


gboolean moo_window_close (MooWindow *window)
{
    gboolean result = FALSE;
    g_signal_emit_by_name (window, "close", &result);
    if (!result) {
        moo_window_save_settings (window);
        gtk_widget_destroy (GTK_WIDGET (window));
        return TRUE;
    }
    else
        return FALSE;
}


static void moo_window_set_property     (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
    const char *name = NULL;
    MooWindow *window = MOO_WINDOW (object);

    switch (prop_id)
    {
        case PROP_TOOLBAR_UI_NAME:
            name = g_value_get_string (value);
            g_free (window->priv->toolbar_ui_name);
            window->priv->toolbar_ui_name = name ? g_strdup (name) : g_strdup ("");
            g_object_notify (object, "toolbar-ui-name");
            break;

        case PROP_MENUBAR_UI_NAME:
            name = g_value_get_string (value);
            g_free (window->priv->menubar_ui_name);
            window->priv->menubar_ui_name = name ? g_strdup (name) : g_strdup ("");
            g_object_notify (object, "menubar-ui-name");
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void moo_window_get_property     (GObject      *object,
                                         guint         prop_id,
                                         GValue       *value,
                                         GParamSpec   *pspec)
{
    MooWindow *window = MOO_WINDOW (object);

    switch (prop_id)
    {
        case PROP_ACCEL_GROUP:
            g_value_set_object (value,
                                window->accel_group);
            break;

        case PROP_TOOLBAR_UI_NAME:
            g_value_set_string (value,
                                window->priv->toolbar_ui_name);
            break;

        case PROP_MENUBAR_UI_NAME:
            g_value_set_string (value,
                                window->priv->menubar_ui_name);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void moo_window_ui_changed (MooWindow *window)
{
    if (!window->priv->update_id)
    {
        if (!gtk_main_level ())
        {
            window->priv->update_id = -1;
            gtk_init_add ((GtkFunction) update_ui_callback,
                          window);
        }
        else
            window->priv->update_id =
                g_idle_add_full (G_PRIORITY_HIGH,
                                 (GSourceFunc) update_ui_callback,
                                 window, NULL);
    }
}


static gboolean update_ui_callback (MooWindow *window)
{
    moo_window_update_ui (window);
    window->priv->update_id = 0;
    return FALSE;
}


void        moo_window_update_ui            (MooWindow  *window)
{
    MooUIXML *xml;

    g_return_if_fail (MOO_IS_WINDOW (window) && window->priv != NULL);

    xml = moo_ui_object_get_ui_xml (MOO_UI_OBJECT (window));

    if (window->priv->menubar_ui_name && window->priv->menubar_ui_name[0] &&
        moo_ui_xml_has_widget (xml, window->priv->menubar_ui_name))
            moo_ui_xml_update_widget (xml, window->menubar,
                                      window->priv->menubar_ui_name,
                                      moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
                                      window->accel_group, window->tooltips);

    if (window->priv->toolbar_ui_name && window->priv->toolbar_ui_name[0] &&
        moo_ui_xml_has_widget (xml, window->priv->toolbar_ui_name))
            moo_ui_xml_update_widget (xml, window->toolbar,
                                      window->priv->toolbar_ui_name,
                                      moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
                                      window->accel_group, window->tooltips);
}


static void moo_window_shortcuts_prefs_dialog (MooWindow *window)
{
    moo_shortcuts_prefs_dialog_run (
        moo_ui_object_get_actions (MOO_UI_OBJECT (window)),
        GTK_WIDGET (window));
}


static void moo_window_show_toolbar_toggled (MooWindow  *window,
                                             gboolean        show)
{
    if (show) gtk_widget_show (window->toolbar);
    else gtk_widget_hide (window->toolbar);
    moo_prefs_set_bool (setting (window, MOO_WINDOW_PREFS_SHOW_TOOLBAR), show);
}


static void moo_window_show_menubar_toggled (MooWindow  *window,
                                             gboolean        show)
{
    if (show) gtk_widget_show (window->menubar);
    else gtk_widget_hide (window->menubar);
    moo_prefs_set_bool (setting (window, MOO_WINDOW_PREFS_SHOW_MENUBAR), show);
}


static void window_destroyed (GObject *menuitem,
                              gpointer window);
static void toolbar_style_menu_destroyed (GObject *window,
                                          gpointer menuitem)
{
    g_object_weak_unref (window, (GWeakNotify)window_destroyed, menuitem);
}
static void window_destroyed (GObject *menuitem,
                              gpointer window)
{
    g_object_weak_unref (menuitem, (GWeakNotify)toolbar_style_menu_destroyed, window);
    gtk_widget_destroy (GTK_WIDGET (menuitem));
}


static void toolbar_style_toggled (GtkCheckMenuItem *item,
                                   MooWindow *window)
{
    int style;
    GEnumClass *enum_class;
    GEnumValue *val;

    if (!gtk_check_menu_item_get_active (item))
        return;

    style = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item),
                                                "moo_window_toolbar_style"));
    gtk_toolbar_set_style (GTK_TOOLBAR (window->toolbar), (GtkToolbarStyle)style);

    enum_class = G_ENUM_CLASS (g_type_class_peek (GTK_TYPE_TOOLBAR_STYLE));
    g_return_if_fail (enum_class != NULL);
    val = g_enum_get_value (enum_class, style);
    g_return_if_fail (val != NULL);
    moo_prefs_set (setting (window, MOO_WINDOW_PREFS_TOOLBAR_STYLE), val->value_name);
}


static GtkMenuItem *moo_window_create_toolbar_style_menu (MooWindow *window)
{
    GtkWidget *item;
    GSList *group = NULL;
    GtkWidget *items[3];
    GtkMenuShell *menu;
    guint i;
    GtkToolbarStyle style;

    const char *labels[3] = {
        "_Icons Only",
        "_Labels Only",
        "Icons _and Labels"
    };

    item = gtk_menu_item_new_with_mnemonic ("Toolbar _Style");

    menu = GTK_MENU_SHELL (gtk_menu_new ());
    for (i = 0; i < 3; ++i) {
        items[i] = gtk_radio_menu_item_new_with_mnemonic (group, labels[i]);
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (items[i]));
        gtk_menu_shell_append (menu, items[i]);
        g_object_set_data (G_OBJECT (items[i]), "moo_window_toolbar_style", GINT_TO_POINTER (i));
    }
    gtk_widget_show_all (GTK_WIDGET (menu));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), GTK_WIDGET (menu));

    style = moo_prefs_get_enum (
                setting (window, MOO_WINDOW_PREFS_TOOLBAR_STYLE), GTK_TYPE_TOOLBAR_STYLE);
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (items[style]), TRUE);

    for (i = 0; i < 3; ++i) {
        g_signal_connect (items[i], "toggled",
                          G_CALLBACK (toolbar_style_toggled), window);
    }

    g_object_weak_ref (G_OBJECT (window), (GWeakNotify)window_destroyed, item);
    g_object_weak_ref (G_OBJECT (item), (GWeakNotify)toolbar_style_menu_destroyed, window);

    return GTK_MENU_ITEM (item);
}
