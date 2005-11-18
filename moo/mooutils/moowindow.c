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

#include "mooutils/moowindow.h"
#include "mooutils/mootoggleaction.h"
#include "mooutils/moomenuaction.h"
#include "mooutils/mooaccel.h"
#include "mooutils/mooprefs.h"
#include "mooutils/moomarshals.h"
#include "mooutils/moocompat.h"
#include "mooutils/moostock.h"
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
    char *menubar_ui_name;
    GtkWidget *menubar_holder;
    GtkWidget *toolbar_holder;

    MooUIXML *ui_xml;
    MooActionGroup *actions;
    char *name;
    char *id;

    gboolean drag_inside;
    gboolean drag_drop;
    GtkTargetList *targets;
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
                                                     MooAction      *action);
static void     moo_window_remove_action            (MooWindow      *window,
                                                     const char     *action_id);

static gboolean moo_window_delete_event             (GtkWidget      *widget,
                                                     GdkEventAny    *event);

static void     moo_window_drag_data_received       (GtkWidget      *widget,
                                                     GdkDragContext *drag_context,
                                                     int             x,
                                                     int             y,
                                                     GtkSelectionData *data,
                                                     guint           info,
                                                     guint           time);
static gboolean moo_window_drag_drop                (GtkWidget      *widget,
                                                     GdkDragContext *drag_context,
                                                     int             x,
                                                     int             y,
                                                     guint           time);
static void     moo_window_drag_leave               (GtkWidget      *widget,
                                                     GdkDragContext *drag_context,
                                                     guint           time);
static gboolean moo_window_drag_motion              (GtkWidget      *widget,
                                                     GdkDragContext *drag_context,
                                                     int             x,
                                                     int             y,
                                                     guint           time);

static gboolean moo_window_save_size                (MooWindow      *window);

static gboolean moo_window_create_ui                (MooWindow      *window);

static void     moo_window_shortcuts_prefs_dialog   (MooWindow      *window);

static void     moo_window_set_menubar_visible      (MooWindow      *window,
                                                     gboolean        visible);
static void     moo_window_set_toolbar_visible      (MooWindow      *window,
                                                     gboolean        visible);

static MooAction *create_toolbar_style_action       (MooWindow      *window,
                                                     gpointer        dummy);


enum {
    PROP_0,
    PROP_ACCEL_GROUP,
    PROP_MENUBAR_UI_NAME,
    PROP_TOOLBAR_UI_NAME,
    PROP_NAME,
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
    widget_class->drag_data_received = moo_window_drag_data_received;
    widget_class->drag_drop = moo_window_drag_drop;
    widget_class->drag_leave = moo_window_drag_leave;
    widget_class->drag_motion = moo_window_drag_motion;

    moo_window_class_set_id (klass, "MooWindow", "Window");

    moo_window_class_new_action (klass, "ConfigureShortcuts",
                                 "name", "Configure Shortcuts",
                                 "label", "Configure _Shortcuts...",
                                 "tooltip", "Configure _Shortcuts...",
                                 "icon-stock-id", MOO_STOCK_KEYBOARD,
                                 "closure::callback", moo_window_shortcuts_prefs_dialog,
                                 NULL);

    moo_window_class_new_action (klass, "ShowToolbar",
                                 "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                 "name", "Show Toolbar",
                                 "label", "Show Toolbar",
                                 "tooltip", "Show Toolbar",
                                 "toggled-callback", moo_window_set_toolbar_visible,
                                 "condition::active", "toolbar-visible",
                                 NULL);

    moo_window_class_new_action (klass, "ShowMenubar",
                                 "action-type::", MOO_TYPE_TOGGLE_ACTION,
                                 "name", "Show Menubar",
                                 "label", "Show Menubar",
                                 "tooltip", "Show Menubar",
                                 "toggled-callback", moo_window_set_menubar_visible,
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
                                     PROP_NAME,
                                     g_param_spec_string ("name",
                                             "name",
                                             "name",
                                             NULL,
                                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

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
                                             MOO_TYPE_ACTION_GROUP,
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


GObject    *moo_window_constructor      (GType                  type,
                                         guint                  n_props,
                                         GObjectConstructParam *props)
{
    GtkWidget *vbox;
    MooWindow *window;
    MooWindowClass *klass;

    GObject *object =
        G_OBJECT_CLASS(moo_window_parent_class)->constructor (type, n_props, props);

    window = MOO_WINDOW (object);

    klass = g_type_class_ref (type);
    moo_window_set_id (window, moo_window_class_get_id (klass));
    moo_window_set_name (window, moo_window_class_get_name (klass));

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

    window->vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), window->vbox, TRUE, TRUE, 0);
    window->statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox), window->statusbar, FALSE, FALSE, 0);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (window->statusbar), FALSE);

    g_signal_connect (window, "notify::toolbar-ui-name",
                      G_CALLBACK (moo_window_create_ui), NULL);
    g_signal_connect (window, "notify::menubar-ui-name",
                      G_CALLBACK (moo_window_create_ui), NULL);
    g_signal_connect (window, "notify::ui-object-xml",
                      G_CALLBACK (moo_window_create_ui), NULL);

    g_idle_add ((GSourceFunc) moo_window_create_ui, window);

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

    g_type_class_unref (klass);
    return object;
}


static void moo_window_init (MooWindow *window)
{
    window->priv = g_new0 (MooWindowPrivate, 1);

    window->priv->targets = gtk_target_list_new (NULL, 0);
    gtk_drag_dest_set (GTK_WIDGET (window), 0, NULL, 0,
                       GDK_ACTION_DEFAULT);
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

    gtk_target_list_unref (window->priv->targets);

    g_free (window->priv);

    G_OBJECT_CLASS (moo_window_parent_class)->finalize (object);
}


GtkTargetList*
moo_window_get_target_list (MooWindow *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    return window->priv->targets;
}


static gboolean moo_window_delete_event     (GtkWidget      *widget,
                                             G_GNUC_UNUSED GdkEventAny    *event)
{
    gboolean result = FALSE;
    g_signal_emit_by_name (widget, "close", &result);
    return result;
}


static gboolean save_size (MooWindow      *window)
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


static gboolean moo_window_save_size    (MooWindow      *window)
{
    if (!window->priv->save_size_id)
        window->priv->save_size_id =
                g_idle_add ((GSourceFunc)save_size, window);
    return FALSE;
}


gboolean moo_window_close (MooWindow *window)
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

        case PROP_NAME:
            moo_window_set_name (window, g_value_get_string (value));
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

        case PROP_NAME:
            g_value_set_string (value, window->priv->name);
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
            g_value_set_boolean (value, window->toolbar && GTK_WIDGET_VISIBLE (window->toolbar));
            break;

        case PROP_MENUBAR_VISIBLE:
            g_value_set_boolean (value, window->menubar && GTK_WIDGET_VISIBLE (window->menubar));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static gboolean
moo_window_create_ui (MooWindow  *window)
{
    MooUIXML *xml;
    MooAction *show_toolbar, *show_menubar;
    GtkToolbarStyle style;

    g_return_val_if_fail (MOO_IS_WINDOW (window), FALSE);

    xml = moo_window_get_ui_xml (window);

    if (window->menubar)
    {
        gtk_widget_destroy (window->menubar);
        window->menubar = NULL;
    }

    if (window->toolbar)
    {
        gtk_widget_destroy (window->toolbar);
        window->toolbar = NULL;
    }

    if (window->priv->menubar_ui_name && window->priv->menubar_ui_name[0])
    {
        MooActionGroup *actions = moo_window_get_actions (window);

        window->menubar =
                moo_ui_xml_create_widget (xml,
                                          MOO_UI_MENUBAR,
                                          window->priv->menubar_ui_name,
                                          actions,
                                          window->accel_group);
        g_return_val_if_fail (window->menubar != NULL, FALSE);

        gtk_box_pack_start (GTK_BOX (window->priv->menubar_holder),
                            window->menubar, FALSE, FALSE, 0);
        gtk_widget_show (window->menubar);

        show_menubar = moo_action_group_get_action (actions, "ShowMenubar");
        moo_toggle_action_set_active (MOO_TOGGLE_ACTION (show_menubar),
                                      moo_prefs_get_bool (setting (window, PREFS_SHOW_MENUBAR)));
    }

    if (window->priv->toolbar_ui_name && window->priv->toolbar_ui_name[0])
    {
        MooActionGroup *actions = moo_window_get_actions (window);

        window->toolbar =
                moo_ui_xml_create_widget (xml,
                                          MOO_UI_TOOLBAR,
                                          window->priv->toolbar_ui_name,
                                          actions,
                                          window->accel_group);
        g_return_val_if_fail (window->toolbar != NULL, FALSE);

        gtk_box_pack_start (GTK_BOX (window->priv->toolbar_holder),
                            window->toolbar, FALSE, FALSE, 0);
        gtk_widget_show (window->toolbar);

        show_toolbar = moo_action_group_get_action (actions, "ShowToolbar");
        moo_toggle_action_set_active (MOO_TOGGLE_ACTION (show_toolbar),
                                      moo_prefs_get_bool (setting (window, PREFS_SHOW_TOOLBAR)));

        style = get_toolbar_style (window);
        gtk_toolbar_set_style (GTK_TOOLBAR (MOO_WINDOW(window)->toolbar), style);
    }

    return FALSE;
}


static void moo_window_shortcuts_prefs_dialog (MooWindow *window)
{
    moo_accel_prefs_dialog_run (moo_window_get_actions (window),
                                GTK_WIDGET (window));
}


static void
moo_window_set_toolbar_visible (MooWindow  *window,
                                gboolean    visible)
{
    if (window->toolbar && visible != GTK_WIDGET_VISIBLE (window->toolbar))
    {
        if (visible)
            gtk_widget_show (window->toolbar);
        else
            gtk_widget_hide (window->toolbar);

        moo_prefs_set_bool (setting (window, PREFS_SHOW_TOOLBAR), visible);
        g_object_notify (G_OBJECT (window), "toolbar-visible");
    }
}


static void
moo_window_set_menubar_visible (MooWindow  *window,
                                gboolean    visible)
{
    if (window->menubar && visible != GTK_WIDGET_VISIBLE (window->menubar))
    {
        if (visible)
            gtk_widget_show (window->menubar);
        else
            gtk_widget_hide (window->menubar);

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

static MooAction*
create_toolbar_style_action (MooWindow      *window,
                             G_GNUC_UNUSED gpointer dummy)
{
    MooAction *action;
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

    action = moo_menu_action_new (TOOLBAR_STYLE_ACTION_ID);
    g_object_set (action, "no-accel", TRUE, NULL);
    menu_mgr = moo_menu_action_get_mgr (MOO_MENU_ACTION (action));
    moo_menu_mgr_set_use_mnemonic (menu_mgr, TRUE);

    moo_menu_mgr_append (menu_mgr, NULL,
                         TOOLBAR_STYLE_ACTION_ID,
                         "Toolbar _Style",
                         0, NULL, NULL);

    for (i = 0; i < N_STYLES; ++i)
        moo_menu_mgr_append (menu_mgr, TOOLBAR_STYLE_ACTION_ID,
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


static void
moo_window_drag_data_received (GtkWidget      *widget,
                               GdkDragContext *context,
                               int             x,
                               int             y,
                               GtkSelectionData *data,
                               guint           info,
                               guint           time)
{
    MooWindow *window = MOO_WINDOW (widget);

#if 0
    char *name;
    name = gdk_atom_name (data->target);
    g_print ("Got data: %s\n", name);
    g_free (name);
#endif

    if (window->priv->drag_drop)
    {
        gtk_drag_finish (context, TRUE, FALSE, time);
        window->priv->drag_inside = FALSE;
        window->priv->drag_drop = FALSE;
    }
    else
    {
        gdk_drag_status (context, context->suggested_action, time);
    }
}


static gboolean
moo_window_drag_drop (GtkWidget      *widget,
                      GdkDragContext *context,
                      int             x,
                      int             y,
                      guint           time)
{
    GdkAtom target = GDK_NONE;
    MooWindow *window = MOO_WINDOW (widget);

    target = gtk_drag_dest_find_target (widget, context,
                                        window->priv->targets);

    if (target != GDK_NONE)
    {
        gtk_drag_get_data (widget, context, target, time);
        window->priv->drag_drop = TRUE;
    }
    else
    {
        gtk_drag_finish (context, FALSE, FALSE, time);
    }

    window->priv->drag_inside = FALSE;
    gtk_drag_unhighlight (widget);
    return TRUE;
}


static void
moo_window_drag_leave (GtkWidget      *widget,
                       GdkDragContext *context,
                       guint           time)
{
    MooWindow *window = MOO_WINDOW (widget);

    g_return_if_fail (window->priv->drag_inside);
    window->priv->drag_inside = FALSE;
    gtk_drag_unhighlight (widget);
}


static gboolean
moo_window_drag_motion (GtkWidget      *widget,
                        GdkDragContext *context,
                        int             x,
                        int             y,
                        guint           time)
{
    MooWindow *window = MOO_WINDOW (widget);
    GdkDragAction action = 0;

#if 0
    if (!window->priv->drag_inside)
    {
        GList *l;

        g_print ("Targets:\n");

        for (l = context->targets; l != NULL; l = l->next)
        {
            GdkAtom atom = GDK_POINTER_TO_ATOM (l->data);
            char *name = gdk_atom_name (atom);
            g_print (" %s\n", name);
            g_free (name);
        }

        g_print ("==========\n");
    }
#endif

    if (gtk_drag_dest_find_target (widget, context, window->priv->targets) != GDK_NONE)
        action = context->suggested_action;

    if (action != 0 && !window->priv->drag_inside)
    {
        window->priv->drag_inside = TRUE;
        gtk_drag_highlight (widget);
    }

    gdk_drag_status (context, action, time);

    return TRUE;
}


/*****************************************************************************/
/* Actions
 */

#define MOO_WINDOW_NAME_QUARK        (get_quark__(0))
#define MOO_WINDOW_ID_QUARK          (get_quark__(1))
#define MOO_WINDOW_ACTIONS_QUARK     (get_quark__(2))


typedef struct {
    MooObjectFactory *action;
    MooObjectFactory *closure;
    char            **conditions;
} ActionFactoryData;


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


static ActionFactoryData*
action_factory_data_new (MooObjectFactory *action,
                         MooObjectFactory *closure,
                         char            **conditions)
{
    ActionFactoryData *data;

    g_return_val_if_fail (MOO_IS_OBJECT_FACTORY (action), NULL);
    g_return_val_if_fail (!closure || MOO_IS_OBJECT_FACTORY (closure), NULL);

    data = g_new0 (ActionFactoryData, 1);
    data->action = g_object_ref (action);
    data->closure = closure ? g_object_ref (closure) : NULL;
    data->conditions = g_strdupv (conditions);

    return data;
}


static void
action_factory_data_free (ActionFactoryData *data)
{
    if (data)
    {
        g_object_unref (data->action);
        if (data->closure)
            g_object_unref (data->closure);
        g_strfreev (data->conditions);
        g_free (data);
    }
}


static MooAction*
create_action (const char        *action_id,
               ActionFactoryData *data,
               MooWindow         *window)
{
    MooClosure *closure = NULL;
    MooAction *action;
    const char *class_id;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (MOO_IS_OBJECT_FACTORY (data->action), NULL);
    g_return_val_if_fail (action_id && action_id[0], NULL);

    class_id = moo_window_class_get_id (MOO_WINDOW_CLASS (G_OBJECT_GET_CLASS (window)));
    action = moo_object_factory_create_object (data->action, window, NULL);
    g_return_val_if_fail (action != NULL, NULL);
    g_object_set (action, "id", action_id, NULL);

    if (g_type_is_a (data->action->object_type, MOO_TYPE_TOGGLE_ACTION))
    {
        g_object_set (action, "toggled-data", window, NULL);
    }

    if (data->closure)
    {
        closure = MOO_CLOSURE (moo_object_factory_create_object (data->closure, NULL, NULL));
        g_return_val_if_fail (closure != NULL, action);
        g_object_set (closure, "data", window, NULL);
        g_object_set (action, "closure", closure, NULL);
    }

    if (data->conditions)
    {
        char **p;
        for (p = data->conditions; *p != NULL; p += 2)
        {
            if (p[1][0] == '!')
                moo_bind_bool_property (action, p[0], window, p[1] + 1, TRUE);
            else
                moo_bind_bool_property (action, p[0], window, p[1], FALSE);
        }
    }

    return action;
}


const char*
moo_window_class_get_id (MooWindowClass     *klass)
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


void
moo_window_class_install_action (MooWindowClass     *klass,
                                 const char         *action_id,
                                 MooObjectFactory   *action,
                                 MooObjectFactory   *closure,
                                 char              **conditions)
{
    GHashTable *actions;
    ActionFactoryData *data;
    GType type;
    GSList *l;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (MOO_IS_OBJECT_FACTORY (action));
    g_return_if_fail (closure == NULL || MOO_IS_OBJECT_FACTORY (closure));
    g_return_if_fail (action_id && action_id[0]);

    /* XXX check if action with this id exists */

    type = G_OBJECT_CLASS_TYPE (klass);
    actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

    if (!actions)
    {
        actions = g_hash_table_new_full (g_str_hash, g_str_equal,
                                         g_free, (GDestroyNotify) action_factory_data_free);
        g_type_set_qdata (type, MOO_WINDOW_ACTIONS_QUARK, actions);
    }

    data = action_factory_data_new (action, closure, conditions);
    g_hash_table_insert (actions, g_strdup (action_id), data);

    for (l = window_instances; l != NULL; l = l->next)
    {
        if (g_type_is_a (G_OBJECT_TYPE (l->data), type))
        {
            MooAction *action = create_action (action_id, data, l->data);
            if (action)
                moo_window_add_action (l->data, action);
        }
    }
}


static GObject*
custom_action_factory_func (MooWindow        *window,
                            MooObjectFactory *factory)
{
    const char *action_id;
    MooWindowActionFunc func;
    gpointer func_data;
    MooAction *action;

    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);

    action_id = g_object_get_data (G_OBJECT (factory), "moo-window-class-action-id");
    func = g_object_get_data (G_OBJECT (factory), "moo-window-class-action-func");
    func_data = g_object_get_data (G_OBJECT (factory), "moo-window-class-action-func-data");

    g_return_val_if_fail (action_id != NULL, NULL);
    g_return_val_if_fail (func != NULL, NULL);

    action = func (window, func_data);

    return action ? G_OBJECT (action) : NULL;
}


void
moo_window_class_new_action_custom (MooWindowClass     *klass,
                                    const char         *action_id,
                                    MooWindowActionFunc func,
                                    gpointer            data,
                                    GDestroyNotify      notify)
{
    MooObjectFactory *action_factory;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (action_id && action_id[0]);
    g_return_if_fail (func != NULL);

    action_factory = moo_object_factory_new_func ((MooObjectFactoryFunc) custom_action_factory_func, NULL);
    g_object_set_data (G_OBJECT (action_factory), "moo-window-class", klass);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-window-class-action-id",
                            g_strdup (action_id), g_free);
    g_object_set_data (G_OBJECT (action_factory), "moo-window-class-action-func", func);
    g_object_set_data_full (G_OBJECT (action_factory), "moo-window-class-action-func-data",
                            data, notify);

    moo_window_class_install_action (klass, action_id, action_factory, NULL, NULL);
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


MooActionGroup*
moo_window_get_actions (MooWindow *window)
{
    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);

    if (!window->priv->actions)
    {
        char *name = moo_window_get_name (window);
        window->priv->actions = moo_action_group_new (name);
        g_free (name);
    }

    return window->priv->actions;
}


MooAction*
moo_window_get_action_by_id (MooWindow          *window,
                             const char         *action_id)
{
    MooActionGroup *actions;

    g_return_val_if_fail (MOO_IS_WINDOW (window), NULL);
    g_return_val_if_fail (action_id != NULL, NULL);

    actions = moo_window_get_actions (window);
    return moo_action_group_get_action (actions, action_id);
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


void
moo_window_set_name (MooWindow          *window,
                     const char         *name)
{
    g_return_if_fail (MOO_IS_WINDOW (window));

    if (name)
    {
        g_free (window->priv->name);
        window->priv->name = g_strdup (name);
        moo_action_group_set_name (moo_window_get_actions (window), name);
        g_object_notify (G_OBJECT (window), "name");
    }
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
add_action (const char        *id,
            ActionFactoryData *data,
            MooWindow         *window)
{
    MooAction *action = create_action (id, data, window);

    if (action)
        moo_window_add_action (window, action);
}

static void
moo_window_add_class_actions (MooWindow      *window)
{
    GType type;

    g_return_if_fail (G_IS_OBJECT (window));

    type = G_OBJECT_TYPE (window);

    do
    {
        GHashTable *actions;

        actions = g_type_get_qdata (type, MOO_WINDOW_ACTIONS_QUARK);

        if (actions)
            g_hash_table_foreach (actions, (GHFunc) add_action, window);

        type = g_type_parent (type);
        g_return_if_fail (type != 0);
    }
    while (type != G_TYPE_OBJECT);
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
                       MooAction          *action)
{
    MooActionGroup *group;

    g_return_if_fail (MOO_IS_WINDOW (window));
    g_return_if_fail (MOO_IS_ACTION (action));

    g_object_set (action, "group-name", window->priv->name, NULL);

    group = moo_window_get_actions (window);
    moo_action_group_add_action (group, action);

    if (!action->dead)
    {
        const char *accel, *default_accel, *accel_path;

        accel_path = moo_action_make_accel_path (window->priv->id, moo_action_get_id (action));
        _moo_action_set_accel_path (action, accel_path);

        accel = moo_prefs_get_accel (accel_path);
        default_accel = moo_action_get_default_accel (action);

        moo_set_accel (accel_path, accel ? accel : default_accel);
    }
}


static void
moo_window_remove_action (MooWindow          *window,
                          const char         *action_id)
{
    MooActionGroup *group;

    g_return_if_fail (MOO_IS_WINDOW (window));
    g_return_if_fail (action_id != NULL);

    group = moo_window_get_actions (window);
    moo_action_group_remove_action (group, action_id);
}


void
moo_window_class_new_actionv (MooWindowClass     *klass,
                              const char         *action_id,
                              const char         *first_prop_name,
                              va_list             var_args)
{
    const char *name;
    GType action_type = 0, closure_type = 0;
    GObjectClass *action_class = NULL;
    GObjectClass *closure_class = NULL;
    GArray *action_params = NULL;
    GArray *closure_params = NULL;
    GPtrArray *conditions = NULL;

    g_return_if_fail (MOO_IS_WINDOW_CLASS (klass));
    g_return_if_fail (first_prop_name != NULL);

    action_params = g_array_new (FALSE, TRUE, sizeof (GParameter));
    closure_params = g_array_new (FALSE, TRUE, sizeof (GParameter));
    conditions = g_ptr_array_new ();

    name = first_prop_name;
    while (name)
    {
        GParameter param = {NULL, {0, {{0}, {0}}}};
        GParamSpec *pspec;
        char *err = NULL;

        /* ignore id property */
        if (!strcmp (name, "id"))
        {
            g_critical ("%s: id property specified", G_STRLOC);
            goto error;
        }

        if (!strcmp (name, "action-type::") || !strcmp (name, "action_type::"))
        {
            g_value_init (&param.value, G_TYPE_POINTER);
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                goto error;
            }

            action_type = (GType) param.value.data[0].v_pointer;

            if (!g_type_is_a (action_type, MOO_TYPE_ACTION))
            {
                g_warning ("%s: invalid action type", G_STRLOC);
                goto error;
            }

            action_class = g_type_class_ref (action_type);
        }
        else if (!strcmp (name, "closure-type::") || !strcmp (name, "closure_type::"))
        {
            g_value_init (&param.value, G_TYPE_POINTER);
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);

            if (err)
            {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                goto error;
            }

            closure_type = (GType) param.value.data[0].v_pointer;

            if (!g_type_is_a (closure_type, MOO_TYPE_CLOSURE))
            {
                g_warning ("%s: invalid closure type", G_STRLOC);
                goto error;
            }

            closure_class = g_type_class_ref (closure_type);
        }
        else if (!strncmp (name, "closure::", strlen ("closure::")))
        {
            const char *suffix = strstr (name, "::");

            if (!suffix || !suffix[1] || !suffix[2])
            {
                g_warning ("%s: invalid property name '%s'", G_STRLOC, name);
                goto error;
            }

            name = suffix + 2;

            if (!closure_class) {
                if (!closure_type)
                    closure_type = MOO_TYPE_CLOSURE;
                closure_class = g_type_class_ref (closure_type);
            }

            pspec = g_object_class_find_property (closure_class, name);

            if (!pspec)
            {
                g_warning ("%s: object class `%s' has no property named `%s'",
                           G_STRLOC, g_type_name (closure_type), name);
                goto error;
            }

            g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
            G_VALUE_COLLECT (&param.value, var_args, 0, &err);
            if (err) {
                g_warning ("%s: %s", G_STRLOC, err);
                g_free (err);
                g_value_unset (&param.value);
                goto error;
            }

            param.name = g_strdup (name);
            g_array_append_val (closure_params, param);
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
        else if (!strstr (name, "::"))
        {
            if (!action_class)
            {
                if (!action_type)
                    action_type = MOO_TYPE_ACTION;
                action_class = g_type_class_ref (action_type);
            }

            pspec = g_object_class_find_property (action_class, name);

            if (!pspec)
            {
                g_warning ("%s: object class `%s' has no property named `%s'",
                           G_STRLOC, g_type_name (action_type), name);
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
        else
        {
            g_warning ("%s: invalid property '%s'", G_STRLOC, name);
            goto error;
        }

        name = va_arg (var_args, gchar*);
    }

    G_STMT_START
    {
        MooObjectFactory *action_factory = NULL;
        MooObjectFactory *closure_factory = NULL;

        if (closure_params->len)
        {
            closure_factory =
                    moo_object_factory_new_a (closure_type,
                                              (GParameter*) closure_params->data,
                                              closure_params->len);

            if (!closure_factory)
            {
                g_warning ("%s: error in moo_object_factory_new_a()", G_STRLOC);
                goto error;
            }

            g_array_free (closure_params, FALSE);
            closure_params = NULL;
        }
        else
        {
            g_array_free (closure_params, TRUE);
            closure_params = NULL;
        }

        action_factory = moo_object_factory_new_a (action_type,
                (GParameter*) action_params->data,
                action_params->len);

        if (!action_factory)
        {
            g_warning ("%s: error in moo_object_factory_new_a()", G_STRLOC);
            g_object_unref (closure_factory);
            goto error;
        }

        g_array_free (action_params, FALSE);
        action_params = NULL;

        g_ptr_array_add (conditions, NULL);

        moo_window_class_install_action (klass,
                                         action_id,
                                         action_factory,
                                         closure_factory,
                                         (char**) conditions->pdata);

        g_strfreev ((char**) conditions->pdata);
        g_ptr_array_free (conditions, FALSE);

        if (action_class)
            g_type_class_unref (action_class);
        if (closure_class)
            g_type_class_unref (closure_class);
        if (action_factory)
            g_object_unref (action_factory);
        if (closure_factory)
            g_object_unref (closure_factory);

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

    if (closure_params)
    {
        guint i;
        GParameter *params = (GParameter*) closure_params->data;
        for (i = 0; i < closure_params->len; ++i) {
            g_value_unset (&params[i].value);
            g_free ((char*) params[i].name);
        }
        g_array_free (closure_params, TRUE);
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
    if (closure_class)
        g_type_class_unref (closure_class);
}
