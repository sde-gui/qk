/*
 *   mooui/moowindow.c
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

#include "mooutils/moowindow.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/mooaccel.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
#include "mooutils/mooactionfactory.h"
#include <gtk/gtk.h>
#include <gobject/gvaluecollector.h>
#include <string.h>


#define PREFS_REMEMBER_SIZE  "window/remember_size"
#define PREFS_WIDTH          "window/width"
#define PREFS_HEIGHT         "window/height"
#define PREFS_MAXIMIZED      "window/maximized"
#define PREFS_SHOW_TOOLBAR   "window/show_toolbar"
#define PREFS_SHOW_MENUBAR   "window/show_menubar"
#define PREFS_TOOLBAR_STYLE  "window/toolbar_style"

#define TOOLBAR_STYLE_ACTION_ID "ToolbarStyle"


static GSList *window_instances = NULL;

struct _MooWindowPrivate {
    guint save_size_id;

    char *toolbar_ui_name;
    GtkWidget *toolbar_holder;
    gboolean toolbar_visible;

    char *menubar_ui_name;
    GtkWidget *menubar_holder;
    gboolean menubar_visible;

    MooUIXML *ui_xml;
    GtkActionGroup *actions;
    char *name;
    char *id;
};


static const char *setting (MooWindow *window, const char *s)
{
    static GString *key = NULL;

    if (!key)
        key = g_string_new (NULL);

    if (window->priv->id)
        g_string_printf (key, "%s/%s", window->priv->id, s);
    else
        g_string_assign (key, s);

    return key->str;
}

static void init_prefs (MooWindow *window);
static GtkToolbarStyle get_toolbar_style (MooWindow *window);


static void     moo_window_class_init               (MooWindowClass *klass);
GObject        *moo_window_constructor              (GType                  type,
                                                     guint                  n_props,
                                                     GObjectConstructParam *props);

static void     moo_window_init                     (MooWindow      *window);
static void     moo_window_finalize                 (GObject        *object);

static void     moo_window_set_property             (GObject        *object,
                                                     guint           prop_id,
                                                     const GValue   *value,
                                                     GParamSpec     *pspec);
static void     moo_window_get_property             (GObject        *object,
                                                     guint           prop_id,
                                                     GValue         *value,
                                                     GParamSpec     *pspec);

static void     moo_window_set_id                   (MooWindow      *window,
                                                     const char     *id);
static void     moo_window_add_class_actions        (MooWindow      *window);
static void     moo_window_add_action               (MooWindow      *window,
                                                     GtkAction      *action);
static void     moo_window_remove_action            (MooWindow      *window,
                                                     const char     *action_id);

static gboolean moo_window_delete_event             (GtkWidget      *widget,
                                                     GdkEventAny    *event);

static gboolean moo_window_save_size                (MooWindow      *window);

static void     moo_window_update_ui                (MooWindow      *window);
static void     moo_window_update_menubar           (MooWindow      *window);
static void     moo_window_update_toolbar           (MooWindow      *window);

static void     moo_window_shortcuts_prefs_dialog   (MooWindow      *window);

static void     moo_window_set_menubar_visible      (MooWindow      *window,
                                                     gboolean        visible);
static void     moo_window_set_toolbar_visible      (MooWindow      *window,
                                                     gboolean        visible);

static GtkAction *create_toolbar_style_action       (MooWindow      *window,
                                                     gpointer        dummy);


enum {
    PROP_0,
    PROP_ACCEL_GROUP,
    PROP_MENUBAR_UI_NAME,
    PROP_TOOLBAR_UI_NAME,
    PROP_ID,
    PROP_UI_XML,
    PROP_ACTIONS,
    PROP_TOOLBAR_VISIBLE,
    PROP_MENUBAR_VISIBLE
};

enum {
    CLOSE,
    LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = {0};


/* MOO_TYPE_WINDOW */
G_DEFINE_TYPE (MooWindow, moo_window, GTK_TYPE_WINDOW)


static void moo_window_class_init (MooWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gobject_class->constructor = moo_window_constructor;
    gobject_class->finalize = moo_window_finalize;
    gobject_class->set_property = moo_window_set_property;
    gobject_class->get_property = moo_window_get_property;

    widget_class->delete_event = moo_window_delete_event;

    moo_window_class_set_id (klass, "MooWindow", "Window");

    moo_window_class_new_action (klass, "ConfigureShortcuts",
                                 "display-name", "Configure Shortcuts",
                                 "label", "Configure _Shortcuts...",
                                 "tooltip", "Configure _Shortcuts...",
                                 "no-accel", TRUE,
                                 "stock-id", MOO_STOCK_KEYBOARD,
                                 "closure-callback", moo_window_shortcuts_prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "ShowToolbar",
                                 "action-type::", GTK_TYPE_TOGGLE_ACTION,
                                 "display-name", "Show Toolbar",
                                 "label", "Show Toolbar",
                                 "tooltip", "Show Toolbar",
                                 "condition::active", "toolbar-visible",
                                 NULL);

    moo_window_class_new_action (klass, "ShowMenubar",
                                 "action-type::", GTK_TYPE_TOGGLE_ACTION,
                                 "display-name", "Show Menubar",
                                 "label", "Show Menubar",
                                 "tooltip", "Show Menubar",
                                 "no-accel", TRUE,
                                 "condition::active", "menubar-visible",
                                 NULL);

    moo_window_class_new_action_custom (klass, TOOLBAR_STYLE_ACTION_ID,
                                        create_toolbar_style_action,
                                        NULL, NULL);

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
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_TOOLBAR_UI_NAME,
                                     g_param_spec_string ("toolbar-ui-name",
                                             "toolbar-ui-name",
                                             "toolbar-ui-name",
                                             NULL,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_ID,
                                     g_param_spec_string ("id",
                                             "id",
                                             "id",
                                             NULL,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_ACTIONS,
                                     g_param_spec_object ("actions",
                                             "actions",
                                             "actions",
                                             GTK_TYPE_ACTION_GROUP,
                                             G_PARAM_READABLE));

    g_object_class_install_property (gobject_class,
                                     PROP_UI_XML,
                                     g_param_spec_object ("ui-xml",
                                             "ui-xml",
                                             "ui-xml",
                                             MOO_TYPE_UI_XML,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (gobject_class,
                                     PROP_TOOLBAR_VISIBLE,
                                     g_param_spec_boolean ("toolbar-visible",
                                             "toolbar-visible",
                                             "toolbar-visible",
                                             TRUE,
                                             G_PARAM_READWRITE));

    g_object_class_install_property (gobject_class,
                                     PROP_MENUBAR_VISIBLE,
                                     g_param_spec_boolean ("menubar-visible",
                                             "menubar-visible",
                                             "menubar-visible",
                                             TRUE,
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


GObject *
moo_window_constructor (GType                  type,
                        guint                  n_props,
                        GObjectConstructParam *props)
{
    GtkWidget *vbox;
    MooWindow *window;
    MooWindowClass *klass;
    GtkAction *action;

    GObject *object =
        G_OBJECT_CLASS(moo_window_parent_class)->constructor (type, n_props, props);

    window = MOO_WINDOW (object);

    klass = g_type_class_ref (type);
    moo_window_set_id (window, moo_window_class_get_id (klass));
    window->priv->name = g_strdup (moo_window_class_get_name (klass));
    window->priv->actions = gtk_action_group_new (window->priv->name);

    init_prefs (window);

    moo_window_add_class_actions (window);
    window_instances = g_slist_prepend (window_instances, object);

    window->accel_group = gtk_accel_group_new ();
    gtk_window_add_accel_group (GTK_WINDOW (window),
                                window->accel_group);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    window->priv->menubar_holder = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (window->priv->menubar_holder);
    gtk_box_pack_start (GTK_BOX (vbox), window->priv->menubar_holder, FALSE, FALSE, 0);

    window->priv->toolbar_holder = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (window->priv->toolbar_holder);
    gtk_box_pack_start (GTK_BOX (vbox), window->priv->toolbar_holder, FALSE, FALSE, 0);

    window->menubar = NULL;
    window->toolbar = NULL;

    gtk_box_pack_start (GTK_BOX (vbox), window->vbox, TRUE, TRUE, 0);

    g_signal_connect (window, "notify::toolbar-ui-name",
                      G_CALLBACK (moo_window_update_toolbar), NULL);
    g_signal_connect (window, "notify::menubar-ui-name",
                      G_CALLBACK (moo_window_update_menubar), NULL);
    g_signal_connect (window, "notify::ui-xml",
                      G_CALLBACK (moo_window_update_ui), NULL);

    if (moo_prefs_get_bool (setting (window, PREFS_REMEMBER_SIZE)))
    {
        int width = moo_prefs_get_int (setting (window, PREFS_WIDTH));
        int height = moo_prefs_get_int (setting (window, PREFS_HEIGHT));

        gtk_window_set_default_size (GTK_WINDOW (window), width, height);

        if (moo_prefs_get_bool (setting (window, PREFS_MAXIMIZED)))
            gtk_window_maximize (GTK_WINDOW (window));
    }

    g_signal_connect (window, "configure-event",
                      G_CALLBACK (moo_window_save_size), NULL);

    moo_window_set_toolbar_visible (window,
        moo_prefs_get_bool (setting (window, PREFS_SHOW_TOOLBAR)));
    action = moo_window_get_action (window, "ShowToolbar");
    moo_sync_toggle_action (action, window, "toolbar-visible", FALSE);

    moo_window_set_menubar_visible (window,
        moo_prefs_get_bool (setting (window, PREFS_SHOW_MENUBAR)));
    action = moo_window_get_action (window, "ShowMenubar");
    moo_sync_toggle_action (action, window, "menubar-visible", FALSE);

    moo_window_update_ui (window);

    g_type_class_unref (klass);
    return object;
}


static void moo_window_init (MooWindow *window)
{
    window->priv = g_new0 (MooWindowPrivate, 1);
    window->vbox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (window->vbox);
    window->priv->toolbar_visible = TRUE;
    window->priv->menubar_visible = TRUE;
}


static void moo_window_finalize       (GObject      *object)
{
    MooWindow *window = MOO_WINDOW(object);

    if (window->priv->ui_xml)
        g_object_unref (window->priv->ui_xml);
    if (window->priv->actions)
        g_object_unref (window->priv->actions);

    g_free (window->priv->name);
    g_free (window->priv->id);
    g_free (window->priv->menubar_ui_name);
    g_free (window->priv->toolbar_ui_name);

    if (window->accel_group)
        g_object_unref (window->accel_group);

    if (window->priv->save_size_id)
        g_source_remove (window->priv->save_size_id);
    window->priv->save_size_id = 0;

    g_free (window->priv);

    G_OBJECT_CLASS (moo_window_parent_class)->finalize (object);
}


static gboolean
moo_window_delete_event (GtkWidget      *widget,
                         G_GNUC_UNUSED GdkEventAny *event)
{
    gboolean result = FALSE;
    g_signal_emit_by_name (widget, "close", &result);
    return result;
}


static gboolean
save_size (MooWindow *window)
{
    window->priv->save_size_id = 0;

    if (MOO_IS_WINDOW (window) && GTK_WIDGET_REALIZED (window))
    {
        GdkWindowState state;
        state = gdk_window_get_state (GTK_WIDGET(window)->window);
        moo_prefs_set_bool (setting (window, PREFS_MAXIMIZED),
                            state & GDK_WINDOW_STATE_MAXIMIZED);

        if (!(state & GDK_WINDOW_STATE_MAXIMIZED))
        {
            int width, height;
            gtk_window_get_size (GTK_WINDOW (window), &width, &height);
            moo_prefs_set_int (setting (window, PREFS_WIDTH), width);
            moo_prefs_set_int (setting (window, PREFS_HEIGHT), height);
        }
    }

    return FALSE;
}


static gboolean
moo_window_save_size (MooWindow *window)
{
    if (!window->priv->save_size_id)
        window->priv->save_size_id =
                g_idle_add ((GSourceFunc)save_size, window);
    return FALSE;
}


gboolean
moo_window_close (MooWindow *window)
{
    gboolean result = FALSE;
    g_signal_emit_by_name (window, "close", &result);
    if (!result)
    {
        gtk_widget_destroy (GTK_WIDGET (window));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


static void
moo_window_set_property (GObject      *object,
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

        case PROP_UI_XML:
            moo_window_set_ui_xml (window, g_value_get_object (value));
            break;

        case PROP_TOOLBAR_VISIBLE:
            moo_window_set_toolbar_visible (window, g_value_get_boolean (value));
            break;

        case PROP_MENUBAR_VISIBLE:
            moo_window_set_menubar_visible (window, g_value_get_boolean (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_window_get_property (GObject      *object,
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

        case PROP_ID:
            g_value_set_string (value, window->priv->id);
            break;

        case PROP_UI_XML:
            g_value_set_object (value, window->priv->ui_xml);
            break;

        case PROP_ACTIONS:
            g_value_set_object (value, moo_window_get_actions (window));
            break;

        case PROP_TOOLBAR_VISIBLE:
            g_value_set_boolean (value, window->priv->toolbar_visible);
            break;

        case PROP_MENUBAR_VISIBLE:
            g_value_set_boolean (value, window->priv->menubar_visible);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
moo_window_update_toolbar (MooWindow *window)
{
    MooUIXML *xml;
    GtkToolbarStyle style;
    char *ui_name;
    GtkActionGroup *actions;

    g_return_if_fail (MOO_IS_WINDOW (window));

    actions = moo_window_get_actions (window);
    xml = moo_window_get_ui_xml (window);
    ui_name = window->priv->toolbar_ui_name;
    ui_name = ui_name && ui_name[0] ? ui_name : NULL;

    if (window->toolbar)
    {
        MooUIXML *old_xml;
        char *old_name;

        old_xml = g_object_get_data (G_OBJECT (window->toolbar), "moo-window-ui-xml");
        old_name = g_object_get_data (G_OBJECT (window->toolbar), "moo-window-ui-name");

        if (!old_xml || old_xml != xml || !ui_name || strcmp (ui_name, old_name))
        {
            gtk_widget_destroy (window->toolbar);
            window->toolbar = NULL;
        }
    }

    if (window->toolbar || !xml || !ui_name)
        return;

    window->toolbar = moo_ui_xml_create_widget (xml, MOO_UI_TOOLBAR,
                                                ui_name, actions,
                                                window->accel_group);
    g_return_if_fail (window->toolbar != NULL);

    g_object_set_data_full (G_OBJECT (window->toolbar), "moo-window-ui-xml",
                            g_object_ref (xml), g_object_unref);
    g_object_set_data_full (G_OBJECT (window->toolbar), "moo-window-ui-name",
                            g_strdup (ui_name), g_free);

    gtk_box_pack_start (GTK_BOX (window->priv->toolbar_holder),
                        window->toolbar, FALSE, FALSE, 0);

    style = get_toolbar_style (window);
    gtk_toolbar_set_style (GTK_TOOLBAR (MOO_WINDOW(window)->toolbar), style);
}


static void
moo_window_update_menubar (MooWindow *window)
{
    MooUIXML *xml;
    char *ui_name;
    GtkActionGroup *actions;

    g_return_if_fail (MOO_IS_WINDOW (window));

    actions = moo_window_get_actions (window);
    xml = moo_window_get_ui_xml (window);
    ui_name = window->priv->menubar_ui_name;
    ui_name = ui_name && ui_name[0] ? ui_name : NULL;

    if (window->menubar)
    {
        MooUIXML *old_xml;
        char *old_name;

        old_xml = g_object_get_data (G_OBJECT (window->menubar), "moo-window-ui-xml");
        old_name = g_object_get_data (G_OBJECT (window->menubar), "moo-window-ui-name");

        if (!old_xml || old_xml != xml || !ui_name || strcmp (ui_name, old_name))
        {
            gtk_widget_destroy (window->menubar);
            window->menubar = NULL;
        }
    }

    if (window->menubar || !xml || !ui_name)
        return;

    window->menubar = moo_ui_xml_create_widget (xml, MOO_UI_MENUBAR,
                                                ui_name, actions,
                                                window->accel_group);
    g_return_if_fail (window->menubar != NULL);

    g_object_set_data_full (G_OBJECT (window->menubar), "moo-window-ui-xml",
                            g_object_ref (xml), g_object_unref);
    g_object_set_data_full (G_OBJECT (window->menubar), "moo-window-ui-name",
                            g_strdup (ui_name), g_free);

    gtk_box_pack_start (GTK_BOX (window->priv->menubar_holder),
                        window->menubar, FALSE, FALSE, 0);
}


static void
moo_window_update_ui (MooWindow *window)
{
    moo_window_update_toolbar (window);
    moo_window_update_menubar (window);
}


static void
moo_window_shortcuts_prefs_dialog (MooWindow *window)
{
    _moo_accel_prefs_dialog_run (moo_window_get_actions (window),
                                 GTK_WIDGET (window));
}


static void
moo_window_set_toolbar_visible (MooWindow  *window,
                                gboolean    visible)
{
    if (!visible != !window->priv->toolbar_visible)
    {
        window->priv->toolbar_visible = visible != 0;
        g_object_set (window->priv->toolbar_holder, "visible", visible, NULL);
        moo_prefs_set_bool (setting (window, PREFS_SHOW_TOOLBAR), visible);
        g_object_notify (G_OBJECT (window), "toolbar-visible");
    }
}


static void
moo_window_set_menubar_visible (MooWindow  *window,
                                gboolean    visible)
{
    if (!visible != !window->priv->menubar_visible)
    {
        window->priv->menubar_visible = visible != 0;
        g_object_set (window->priv->menubar_holder, "visible", visible, NULL);
        moo_prefs_set_bool (setting (window, PREFS_SHOW_MENUBAR), visible);
        g_object_notify (G_OBJECT (window), "menubar-visible");
    }
}


static void
toolbar_style_toggled (MooWindow            *window,
                       gpointer              data)
{
    GtkToolbarStyle style = GPOINTER_TO_INT (data);
    if (window->toolbar)
        gtk_toolbar_set_style (GTK_TOOLBAR (window->toolbar), style);
    moo_prefs_set_enum (setting (window, PREFS_TOOLBAR_STYLE), style);
}


#define N_STYLES 4
#define ICONS_ONLY "icons-only"
#define LABELS_ONLY "labels-only"
#define ICONS_AND_LABELS "icons-and-labels"
#define ICONS_AND_IMPORTANT_LABELS "icons-and-important-labels"

static GtkAction*
create_toolbar_style_action (MooWindow      *window,
                             G_GNUC_UNUSED gpointer dummy)
{
    GtkAction *action;
    guint i;
    GtkToolbarStyle style;
    MooMenuMgr *menu_mgr;

    const char *labels[N_STYLES] = {
        "_Icons Only",
        "_Labels Only",
        "Icons _and Labels",
        "Icons _and Important Labels"
    };

    const char *ids[N_STYLES] = {
        ICONS_ONLY,
        LABELS_ONLY,
        ICONS_AND_LABELS,
        ICONS_AND_IMPORTANT_LABELS
    };

    action = moo_menu_action_new (TOOLBAR_STYLE_ACTION_ID, "Toolbar _Style");
    menu_mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));
    moo_menu_mgr_set_use_mnemonic (menu_mgr, TRUE);

    for (i = 0; i < N_STYLES; ++i)
        moo_menu_mgr_append (menu_mgr, NULL,
                             ids[i], labels[i], MOO_MENU_ITEM_RADIO,
                             GINT_TO_POINTER (i), NULL);

    style = get_toolbar_style (window);
    moo_menu_mgr_set_active (menu_mgr, ids[style], TRUE);

    g_signal_connect_swapped (menu_mgr, "radio-set-active",
                              G_CALLBACK (toolbar_style_toggled), window);

    return action;
}


static GtkToolbarStyle get_toolbar_style_gtk (MooWindow *window)
{
    GtkSettings *settings = gtk_widget_get_settings (GTK_WIDGET (window));
    GtkToolbarStyle style = GTK_TOOLBAR_ICONS;
    gpointer toolbar_class;

    g_return_val_if_fail (settings != NULL, style);

    toolbar_class = g_type_class_ref (GTK_TYPE_TOOLBAR);
    g_object_get (settings, "gtk-toolbar-style", &style, NULL);
    g_type_class_unref (toolbar_class);

    g_return_val_if_fail (style < N_STYLES, 0);
    return style;
}


static void init_prefs (MooWindow *window)
{
    moo_prefs_new_key_bool (setting (window, PREFS_REMEMBER_SIZE), TRUE);
    moo_prefs_new_key_bool (setting (window, PREFS_MAXIMIZED), FALSE);
    moo_prefs_new_key_int (setting (window, PREFS_WIDTH), -1);
    moo_prefs_new_key_int (setting (window, PREFS_HEIGHT), -1);
    moo_prefs_new_key_bool (setting (window, PREFS_SHOW_TOOLBAR), TRUE);
    moo_prefs_new_key_bool (setting (window, PREFS_SHOW_MENUBAR), TRUE);
    moo_prefs_new_key_enum (setting (window, PREFS_TOOLBAR_STYLE),
                            GTK_TYPE_TOOLBAR_STYLE,
                            get_toolbar_style_gtk (window));
}


static GtkToolbarStyle get_toolbar_style (MooWindow *window)
{
    GtkToolbarStyle s = moo_prefs_get_enum (setting (window, PREFS_TOOLBAR_STYLE));
    g_return_val_if_fail (s < N_STYLES, GTK_TOOLBAR_ICONS);
    return s;
}
#undef N_STYLES


/*****************************************************************************/
/* Actions
 */

#define MOO_WINDOW_NAME_QUARK        (get_quark__(0))
#define MOO_WINDOW_ID_QUARK          (get_quark__(1))
#define MOO_WINDOW_ACTIONS_QUARK     (get_quark__(2))

typedef struct {
    MooActionFactory *action;
    char **conditions;
} ActionInfo;


static GQuark
get_quark__ (guint n)
{
#define N_QUARKS 3
    guint i;
    static GQuark q[N_QUARKS] = {0, 0, 0};
    static const char *names[N_QUARKS] = {
        "moo_window_name",
        "moo_window_id",
        "moo_window_actions",
    };

    if (!q[0])
    {
        for (i = 0; i < N_QUARKS; ++i)
            q[i] = g_quark_from_static_string (names[i]);
    }

    return q[n];
#undef N_QUARKS
}


static ActionInfo*
action_info_new (MooActionFactory  *action,
                 char             **conditions)
{
    ActionInfo *info;

    g_return_val_if_fail (MOO_IS_ACTION_FACTORY (action), NULL);

    info = g_new0 (ActionInfo, 1);
    info->action = g_object_ref (action);
    info->conditions = g_strdupv (conditions);

    return info;
}


static void
action_info_free (ActionInfo *info)
{
    if (info)
    {
        g_object_unref (info->action);
        g_strfreev (info->conditions);
        g_free (info);
    }
}


static GtkAction*
create_action (const char *action_id,
               ActionInfo *info,
               MooWindow  *window)
{
    GtkAction *action;
    const char *class_id;

    g_return_val_if_fail (info != NULL, NULL);
    g_return_val_if_fail (MOO_IS_ACTION_FACTORY (info->action), NULL);
    g_return_val_if_fail (action_id && action_id[0], NULL);

    class_id = moo_window_class_get_id (MOO_WINDOW_CLASS (G_OBJECT_GET_CLASS (window)));
    action = moo_action_factory_create_action (info->action, window,
                                               "closure-object", window,
                                               "toggled-object", window,
                                               "name", action_id,
                                               NULL);
    g_return_val_if_fail (action != NULL, NULL);

    if (info->conditions)
    {
        char **p;

        for (p = info->conditions; *p != NULL; p += 2)
        {
            gboolean invert;
            char *condition, *prop;

            invert = p[1][0] == '!';
            prop = p[1][0] == '!' ? p[1] + 1 : p[1];
            condition = p[0];

            if (!strcmp (condition, "active"))
                moo_sync_toggle_action (action, window, prop, invert);
            else
                moo_bind_bool_property (action, condition, window, prop, invert);
        }
    }

    return action;
}


const char*
moo_window_class_get_id (MooWindowClass *klass)
{
    GType type;

    g_return_val_if_fail (MOO_IS_WINDOW_CLASS (klass), NULL);

    type = G_OBJECT_CLASS_TYPE (klass);
    return g_type_get_qdata (type, MOO_WINDOW_ID_QUARK);
}


const char*
moo_window_class_get_name (MooWindowClass     *klass)
{
    GType type;

    g_return_val_if_fail (MOO_IS_WINDOW_CLASS (klass), NULL);

    type = G_OBJECT_CLASS_TYPE (klass);
    return g_type_get_qdata (type, MOO_WINDOW_NAME_QUARK);
}


static void
moo_window_class_install_action (MooWindowClass     *klass,
                                 const char         *action_id,
                                 MooActionFactory   *action,
                                 char              **conditions)
{
    GHashTable *actions;
    ActionInfo *info;
    GType type;
    GSList *l;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (MOO_IS_ACTION_FACTORY (action));
    g_return_if_fail (action_id && action_id[0]);

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

    if (!actions)
    {
        actions = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, (GDestroyNotify) action_info_free);
        g_type_set_qdata (type, MOO_WINDOW_ACTIONS_QUARK, actions);
    }

    if (g_hash_table_lookup (actions, action_id))
        moo_window_class_remove_action (klass, action_id);

    info = action_info_new (action, conditions);
    g_hash_table_insert (actions, g_strdup (action_id), info);

    for (l = window_instances; l != NULL; l = l->next)
    {
        if (g_type_is_a (G_OBJECT_TYPE (l->data), type))
        {
            GtkAction *action = create_action (action_id, info, l->data);

            if (action)
                moo_window_add_action (l->data, action);
        }
    }
}


static GtkAction *
custom_action_factory_func (MooWindow        *window,
                            MooActionFactory *factory)
{
    MooWindowActionFunc func;
    gpointer func_data;

    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);

    func = g_object_get_data (G_OBJECT (factory), "moo-window-class-action-func");
    func_data = g_object_get_data (G_OBJECT (factory), "moo-window-class-action-func-data");

    g_return_val_if_fail (func != NULL, NULL);

    return func (window, func_data);
}


void
moo_window_class_new_action_custom (MooWindowClass     *klass,
                                    const char         *action_id,
                                    MooWindowActionFunc func,
                                    gpointer            data,
                                    GDestroyNotify      notify)
{
    MooActionFactory *action_factory;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (action_id && action_id[0]);
    g_return_if_fail (func != NULL);

    action_factory = moo_action_factory_new_func ((MooActionFactoryFunc) custom_action_factory_func, NULL);
    g_object_set_data (G_OBJECT (action_factory), "moo-window-class", klass);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-window-class-action-id",
                            g_strdup (action_id), g_free);
    g_object_set_data (G_OBJECT (action_factory), "moo-window-class-action-func", func);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-window-class-action-func-data",
                            data, notify);

    moo_window_class_install_action (klass, action_id, action_factory, NULL);
    g_object_unref (action_factory);
}


void
moo_window_class_new_action (MooWindowClass     *klass,
                             const char         *action_id,
                             const char         *first_prop_name,
                             ...)
{
    va_list args;
    va_start (args, first_prop_name);
    moo_window_class_new_actionv (klass, action_id, first_prop_name, args);
    va_end (args);
}


gboolean
moo_window_class_find_action (MooWindowClass *klass,
                              const char     *id)
{
    GHashTable *actions;
    GType type;

    g_return_val_if_fail (MOO_IS_WINDOW_CLASS (klass), FALSE);

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

    if (actions)
        return g_hash_table_lookup (actions, id) != NULL;
    else
        return FALSE;
}


void
moo_window_class_remove_action (MooWindowClass     *klass,
                                const char         *action_id)
{
    GHashTable *actions;
    GType type;
    GSList *l;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

    if (actions)
        g_hash_table_remove (actions, action_id);

    for (l = window_instances; l != NULL; l = l->next)
        if (g_type_is_a (G_OBJECT_TYPE (l->data), type))
            moo_window_remove_action (l->data, action_id);
}


MooUIXML*
moo_window_get_ui_xml (MooWindow          *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    return window->priv->ui_xml;
}


void
moo_window_set_ui_xml (MooWindow          *window,
                       MooUIXML           *xml)
{
    g_return_if_fail (MOO_IS_WINDOW (window));
    g_return_if_fail (!xml || MOO_IS_UI_XML (xml));

    if (xml && xml == window->priv->ui_xml)
        return;

    if (window->priv->ui_xml)
        g_object_unref (window->priv->ui_xml);

    window->priv->ui_xml = xml ? g_object_ref (xml) : moo_ui_xml_new ();

    g_object_notify (G_OBJECT (window), "ui-xml");
}


GtkActionGroup*
moo_window_get_actions (MooWindow *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    return window->priv->actions;
}


GtkAction *
moo_window_get_action (MooWindow  *window,
                       const char *action)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    g_return_val_if_fail (action != NULL, NULL);
    return gtk_action_group_get_action (window->priv->actions, action);
}


char*
moo_window_get_name (MooWindow          *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    return g_strdup (window->priv->name);
}


char*
moo_window_get_id (MooWindow          *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    return g_strdup (window->priv->id);
}


static void
moo_window_set_id (MooWindow      *window,
                   const char     *id)
{
    if (id)
    {
        g_free (window->priv->id);
        window->priv->id = g_strdup (id);
    }
}


static void
add_action (const char *id,
            ActionInfo *info,
            MooWindow  *window)
{
    GtkAction *action = create_action (id, info, window);

    if (action)
        moo_window_add_action (window, action);
}

static void
moo_window_add_class_actions (MooWindow *window)
{
    GType type;

    g_return_if_fail (MOO_IS_WINDOW (window));

    type = G_OBJECT_TYPE (window);

    while (TRUE)
    {
        GHashTable *actions;

        actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

        if (actions)
            g_hash_table_foreach (actions, (GHFunc) add_action, window);

        if (type == MOO_TYPE_WINDOW)
            break;

        type = g_type_parent (type);
    }
}


void
moo_window_class_set_id (MooWindowClass     *klass,
                         const char         *id,
                         const char         *name)
{
    GType type;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (id != NULL && name != NULL);

    type = G_OBJECT_CLASS_TYPE (klass);
    g_return_if_fail (g_type_get_qdata (type, MOO_WINDOW_ID_QUARK) == NULL);
    g_type_set_qdata (type, MOO_WINDOW_ID_QUARK, g_strdup (id));
    g_return_if_fail (g_type_get_qdata (type, MOO_WINDOW_NAME_QUARK) == NULL);
    g_type_set_qdata (type, MOO_WINDOW_NAME_QUARK, g_strdup (name));
}


static void
moo_window_add_action (MooWindow          *window,
                       GtkAction          *action)
{
    GtkActionGroup *group;

    g_return_if_fail (MOO_IS_WINDOW (window));
    g_return_if_fail (GTK_IS_ACTION (action));

    group = moo_window_get_actions (window);
    gtk_action_group_add_action (group, action);

    if (!_moo_action_get_dead (action))
    {
        const char *accel, *default_accel, *accel_path;

        accel_path = _moo_action_make_accel_path (window->priv->id, gtk_action_get_name (action));
        _moo_action_set_accel_path (action, accel_path);

        accel = _moo_prefs_get_accel (accel_path);
        default_accel = moo_action_get_default_accel (action);

        _moo_set_accel (accel_path, accel ? accel : default_accel);
    }
}


static void
moo_window_remove_action (MooWindow  *window,
                          const char *action_id)
{
    GtkActionGroup *group;
    GtkAction *action;

    g_return_if_fail (MOO_IS_WINDOW (window));
    g_return_if_fail (action_id != NULL);

    group = moo_window_get_actions (window);
    action = gtk_action_group_get_action (group, action_id);

    if (action)
        gtk_action_group_remove_action (group, action);
}


void
moo_window_class_new_actionv (MooWindowClass     *klass,
                              const char         *action_id,
                              const char         *first_prop_name,
                              va_list             var_args)
{
    const char *name;
    GType action_type = 0;
    GObjectClass *action_class = NULL;
    GArray *action_params = NULL;
    GPtrArray *conditions = NULL;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (first_prop_name != NULL);

    action_params = g_array_new (FALSE, TRUE, sizeof (GParameter));
    conditions = g_ptr_array_new ();

    name = first_prop_name;
    while (name)
    {
        GParameter param = {NULL, {0, {{0}, {0}}}};
        GParamSpec *pspec;
        char *err = NULL;

        /* ignore id property */
        if (!strcmp (name, "id") || !strcmp (name, "name"))
        {
            g_critical ("%s: id property specified", G_STRLOC);
            goto error;
        }

        if (!strcmp (name, "action-type::") || !strcmp (name, "action_type::"))
        {
            g_value_init (&param.value, MOO_TYPE_GTYPE);
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                goto error;
            }

            action_type = moo_value_get_gtype (&param.value);

            if (!g_type_is_a (action_type, GTK_TYPE_ACTION))
            {
                g_warning ("%s: invalid action type", G_STRLOC);
                goto error;
            }

            action_class = g_type_class_ref (action_type);
        }
        else if (!strncmp (name, "condition::", strlen ("condition::")))
        {
            const char *suffix = strstr (name, "::");

            if (!suffix || !suffix[1] || !suffix[2])
            {
                g_warning ("%s: invalid condition name '%s'", G_STRLOC, name);
                goto error;
            }

            g_ptr_array_add (conditions, g_strdup (suffix + 2));

            name = va_arg (var_args, gchar*);

            if (!name)
            {
                g_warning ("%s: unterminated '%s' property",
                           G_STRLOC,
                           (char*) g_ptr_array_index (conditions, conditions->len - 1));
                goto error;
            }

            g_ptr_array_add (conditions, g_strdup (name));
        }
        else
        {
            if (!action_class)
            {
                if (!action_type)
                    action_type = GTK_TYPE_ACTION;
                action_class = g_type_class_ref (action_type);
            }

            pspec = _moo_action_find_property (action_class, name);

            if (!pspec)
            {
                g_warning ("%s: no property '%s' in class '%s'",
                           G_STRLOC, name, g_type_name (action_type));
                goto error;
            }

            g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                g_value_unset (&param.value);
                goto error;
            }

            param.name = g_strdup (name);
            g_array_append_val (action_params, param);
        }

        name = va_arg (var_args, gchar*);
    }

    G_STMT_START
    {
        MooActionFactory *action_factory = NULL;

        action_factory = moo_action_factory_new_a (action_type,
                                                   (GParameter*) action_params->data,
                                                   action_params->len);

        if (!action_factory)
        {
            g_warning ("%s: error in moo_action_factory_new_a()", G_STRLOC);
            goto error;
        }

        moo_param_array_free ((GParameter*) action_params->data,
                               action_params->len);
        g_array_free (action_params, FALSE);
        action_params = NULL;

        g_ptr_array_add (conditions, NULL);

        moo_window_class_install_action (klass,
                                         action_id,
                                         action_factory,
                                         (char**) conditions->pdata);

        g_strfreev ((char**) conditions->pdata);
        g_ptr_array_free (conditions, FALSE);

        if (action_class)
            g_type_class_unref (action_class);
        if (action_factory)
            g_object_unref (action_factory);

        return;
    }
    G_STMT_END;

error:
    if (action_params)
    {
        guint i;
        GParameter *params = (GParameter*) action_params->data;

        for (i = 0; i < action_params->len; ++i)
        {
            g_value_unset (&params[i].value);
            g_free ((char*) params[i].name);
        }

        g_array_free (action_params, TRUE);
    }

    if (conditions)
    {
        guint i;
        for (i = 0; i < conditions->len; ++i)
            g_free (g_ptr_array_index (conditions, i));
        g_ptr_array_free (conditions, TRUE);
    }

    if (action_class)
        g_type_class_unref (action_class);
}
