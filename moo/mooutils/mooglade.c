/*
 *   mooglade.c
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

#include "mooutils/mooglade.h"
#include "mooutils/moomarkup.h"
#include "mooutils/mooutils-gobject.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/moocompat.h"
#include "mooutils/mooprefsdialog.h"
#include "mooutils/mooprefs.h"
#include "mooutils/mooi18n.h"
#include <gtk/gtk.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED
#include <gtk/gtkcombo.h>
#include <gtk/gtklist.h>
#include <gtk/gtklistitem.h>
#include <gtk/gtkoptionmenu.h>
#define GTK_DISABLE_DEPRECATED
#endif


G_DEFINE_TYPE (MooGladeXML, moo_glade_xml, G_TYPE_OBJECT)


#define FOREACH_ELM_START(parent,elm)                           \
G_STMT_START {                                                  \
    MooMarkupNode *elm;                                         \
    for (elm = parent->children; elm != NULL; elm = elm->next)  \
    {                                                           \
        if (MOO_MARKUP_IS_ELEMENT (elm))                        \

#define FOREACH_ELM_END \
    }                   \
} G_STMT_END


typedef struct _Widget Widget;
typedef struct _Signal Signal;
typedef struct _WidgetProps WidgetProps;
typedef struct _PackingProps PackingProps;
typedef struct _Child Child;

struct _Widget {
    Child *parent_node;
    GtkWidget *widget;
    char *id;
    GType type;
    WidgetProps *props;
    GSList *children; /* Child* */
    GSList *signals; /* Signal* */
};

struct _Signal {
    char *name;
    char *handler;
    char *object;
};

struct _Child {
    Widget *parent_node;
    gboolean empty;
    char *internal_child;
    char *internal_parent_id;
    Widget *widget;
    PackingProps *props;
};

typedef enum {
    PROP_RESPONSE_ID        = 1 << 0,
    PROP_HAS_DEFAULT        = 1 << 1,
    PROP_HAS_FOCUS          = 1 << 2,
    PROP_TOOLTIP            = 1 << 3,
    PROP_MNEMONIC_WIDGET    = 1 << 4,
    PROP_RADIO_GROUP        = 1 << 5,
    PROP_LABEL              = 1 << 6,
    PROP_USE_UNDERLINE      = 1 << 7,
    PROP_USE_STOCK          = 1 << 8,
    PROP_ENABLE_TOOLTIPS    = 1 << 9,
    PROP_HISTORY            = 1 << 10
} PropMask;

struct _WidgetProps {
    GParameter *params;
    guint       n_params;

    GPtrArray  *custom_props;

    PropMask    mask;
    gboolean    visible;
    int         response_id;
    gboolean    has_default;
    gboolean    has_focus;
    char       *tooltip;
    char       *mnemonic_widget;
    char       *radio_group;
    char       *label;
    int         history;
};

typedef enum {
    PACK_PROP_LABEL_ITEM    = 1 << 0,
    PACK_PROP_TAB           = 1 << 1,
} PackPropMask;

struct _PackingProps {
    GParameter  *params;
    guint        n_params;

    PackPropMask mask;
};

struct _MooGladeXMLPrivate {
    GHashTable *widgets;
    char *root_id;

    char *translation_domain;

    GHashTable *class_to_type;
    GHashTable *id_to_type;
    GHashTable *id_to_func;

    GHashTable *props; /* char* -> char** */

    MooGladeSignalFunc signal_func;
    gpointer signal_func_data;
    MooGladePropFunc prop_func;
    gpointer prop_func_data;

    guint done : 1;
};

typedef struct {
    MooGladeCreateFunc func;
    gpointer data;
} FuncDataPair;


#define NODE_IS_WIDGET(node) (!strcmp (node->name, "widget"))
#define NODE_IS_SIGNAL(node) (!strcmp (node->name, "signal"))
#define NODE_IS_PROPERTY(node) (!strcmp (node->name, "property"))
#define NODE_IS_CHILD(node) (!strcmp (node->name, "child"))
#define NODE_IS_PACKING(node) (!strcmp (node->name, "packing"))


static FuncDataPair *func_data_pair_new         (MooGladeCreateFunc func,
                                                 gpointer        data);
static void          func_data_pair_free        (FuncDataPair   *pair);
static Widget       *widget_new                 (MooGladeXML    *xml,
                                                 Child          *parent,
                                                 MooMarkupNode  *node,
                                                 GError        **error);
static void          widget_free                (Widget         *widget);
static Child        *child_new                  (MooGladeXML    *xml,
                                                 Widget         *parent,
                                                 MooMarkupNode  *node,
                                                 GError        **error);
static void          child_free                 (Child          *child);
static WidgetProps  *widget_props_new           (MooMarkupNode  *node,
                                                 GType           widget_type,
                                                 GHashTable     *add_props,
                                                 gboolean        ignore_errors,
                                                 const char     *translation_domain,
                                                 GError        **error);
static void          widget_props_free          (WidgetProps    *props);
static PackingProps *packing_props_new          (MooMarkupNode  *node,
                                                 GType           container_type,
                                                 GError        **error);
static void          packing_props_free         (PackingProps   *props);
static Signal       *signal_new                 (const char     *id,
                                                 const char     *handler,
                                                 const char     *object);
static void          signal_free                (Signal         *signal);
static void          collect_signals            (MooMarkupNode  *node,
                                                 Widget         *widget);

static GType         get_type_by_name           (const char     *name);
static gboolean      parse_bool                 (const char     *value);
static int           parse_int                  (const char     *value);
static GtkObject    *parse_adjustment           (const char     *value);
static gboolean      parse_property             (GParamSpec     *param_spec,
                                                 const char     *value,
                                                 GParameter     *param);

static gboolean      moo_glade_xml_parse_markup (MooGladeXML    *xml,
                                                 MooMarkupDoc   *doc,
                                                 const char     *root,
                                                 GtkWidget      *root_widget,
                                                 GError        **error);
static void          moo_glade_xml_dispose      (GObject        *object);
static gboolean      moo_glade_xml_add_widget   (MooGladeXML    *xml,
                                                 const char     *id,
                                                 GtkWidget      *widget,
                                                 GError        **error);
static gboolean      moo_glade_xml_build        (MooGladeXML    *xml,
                                                 Widget         *widget_node,
                                                 GtkWidget      *widget,
                                                 GError        **error);
static void          moo_glade_xml_cleanup      (MooGladeXML    *xml);

static gboolean      create_child               (MooGladeXML    *xml,
                                                 GtkWidget      *parent,
                                                 Child          *child,
                                                 GError        **error);
static void          set_special_props          (MooGladeXML    *xml,
                                                 GtkWidget      *widget,
                                                 WidgetProps    *props);
static void          set_custom_props           (MooGladeXML    *xml,
                                                 Widget         *node);
static void          set_mnemonics              (MooGladeXML    *xml,
                                                 Widget         *node);
static gboolean      set_default                (MooGladeXML    *xml,
                                                 Widget         *node);
static gboolean      set_focus                  (MooGladeXML    *xml,
                                                 Widget         *node);
static void          connect_signals            (MooGladeXML    *xml,
                                                 Widget         *node);

static GtkWidget    *create_widget              (MooGladeXML    *xml,
                                                 Widget         *widget,
                                                 GError        **error);
static gboolean      create_children            (MooGladeXML    *xml,
                                                 Widget         *widget_node,
                                                 GtkWidget      *widget,
                                                 GError        **error);
static void          pack_children              (MooGladeXML    *xml,
                                                 Widget         *widget_node,
                                                 GtkWidget      *widget);
static GtkWidget    *moo_glade_xml_create_widget(MooGladeXML    *xml,
                                                 Widget         *node,
                                                 GtkWidget      *widget,
                                                 GError        **error);

static void          dump_widget                (Widget         *widget_node);


static gboolean
moo_glade_xml_build (MooGladeXML    *xml,
                     Widget         *widget_node,
                     GtkWidget      *widget,
                     GError        **error)
{
    if (widget)
    {
        if (!widget_node->type)
            widget_node->type = G_OBJECT_TYPE (widget);

        if (!g_type_is_a (G_OBJECT_TYPE (widget), widget_node->type))
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "incompatible types %s and %s",
                         g_type_name (G_OBJECT_TYPE (widget)),
                         g_type_name (widget_node->type));
            return FALSE;
        }
    }

    widget = moo_glade_xml_create_widget (xml, widget_node, widget, error);

    if (!widget || !moo_glade_xml_add_widget (xml, widget_node->id, widget, error))
        goto error;

    widget_node->widget = widget;

    if (!create_children (xml, widget_node, widget, error))
        goto error;

    set_special_props (xml, widget, widget_node->props);
    set_custom_props (xml, widget_node);
    set_mnemonics (xml, widget_node);
    set_default (xml, widget_node);
    set_focus (xml, widget_node);
    connect_signals (xml, widget_node);

    return TRUE;

error:
    gtk_widget_destroy (widget);
    return FALSE;
}


static void
set_mnemonics (MooGladeXML    *xml,
               Widget         *node)
{
    GSList *l;

    g_return_if_fail (node != NULL);

    if (node->props->mask & PROP_MNEMONIC_WIDGET)
    {
        GtkWidget *mnemonic;

        mnemonic = moo_glade_xml_get_widget (xml, node->props->mnemonic_widget);

        if (!mnemonic)
            g_warning ("%s: could not find widget '%s'",
                       G_STRLOC,
                       node->props->mnemonic_widget ?
                               node->props->mnemonic_widget : "NULL");
        else if (!GTK_IS_LABEL (node->widget))
            g_warning ("%s: mnemonic widget property specified for widget of class %s",
                       G_STRLOC, g_type_name (G_OBJECT_TYPE (node->widget)));
        else
            gtk_label_set_mnemonic_widget (GTK_LABEL (node->widget), mnemonic);
    }

    for (l = node->children; l != NULL; l = l->next)
    {
        Child *child = l->data;

        if (!child->empty)
            set_mnemonics (xml, child->widget);
    }
}


static gboolean
set_default (MooGladeXML    *xml,
             Widget         *node)
{
    GSList *l;

    g_return_val_if_fail (node != NULL, FALSE);

    if ((node->props->mask & PROP_HAS_DEFAULT) &&
         node->props->has_default)
    {
        gtk_widget_grab_default (node->widget);
        return TRUE;
    }

    for (l = node->children; l != NULL; l = l->next)
    {
        Child *child = l->data;

        if (!child->empty && set_default (xml, child->widget))
            break;
    }

    return FALSE;
}


static gboolean
set_focus (MooGladeXML    *xml,
           Widget         *node)
{
    GSList *l;

    g_return_val_if_fail (node != NULL, FALSE);

    if ((node->props->mask & PROP_HAS_FOCUS) &&
         node->props->has_focus)
    {
        gtk_widget_grab_focus (node->widget);
        return TRUE;
    }

    for (l = node->children; l != NULL; l = l->next)
    {
        Child *child = l->data;

        if (!child->empty && set_focus (xml, child->widget))
            break;
    }

    return FALSE;
}


static void
set_moo_sensitive (MooGladeXML *xml,
                   GtkWidget   *widget,
                   const char  *value)
{
    GtkWidget *btn;
    gboolean invert = FALSE;

    if (value[0] == '!')
    {
        value = value + 1;
        invert = TRUE;
    }

    btn = moo_glade_xml_get_widget (xml, value);
    g_return_if_fail (GTK_IS_TOGGLE_BUTTON (btn));

    moo_bind_sensitive (btn, &widget, 1, invert);
}


static void
set_custom_props (MooGladeXML    *xml,
                  Widget         *node)
{
    guint i;

    if (!node->props->custom_props)
        return;

    for (i = 0; i < node->props->custom_props->len; i += 2)
    {
        const char *prop, *value;

        prop = node->props->custom_props->pdata[i];
        value = node->props->custom_props->pdata[i+1];

        if (!strcmp (prop, "moo-sensitive"))
        {
            set_moo_sensitive (xml, node->widget, value);
        }
        else if (!xml->priv->prop_func ||
                  !xml->priv->prop_func (xml, node->id, node->widget,
                                         prop, value, xml->priv->prop_func_data))
        {
//             g_message ("%s: unknown property '%s'", G_STRLOC, prop);
        }
    }
}


static gboolean
connect_special_signal (MooGladeXML    *xml,
                        Widget         *node,
                        Signal         *signal)
{
    if (!strcmp (signal->name, "moo-sensitive"))
    {
        GtkWidget *btn;
        gboolean invert = FALSE;

        btn = moo_glade_xml_get_widget (xml, signal->handler);
        g_return_val_if_fail (GTK_IS_TOGGLE_BUTTON (btn), FALSE);

        if (signal->object)
        {
            if (!strcmp (signal->object, "invert"))
                invert = TRUE;
            else
                g_warning ("%s: invalid string '%s'",
                           G_STRLOC, signal->object);
        }

        moo_bind_sensitive (btn, &node->widget, 1, invert);

        return TRUE;
    }

    return FALSE;
}


static void
connect_signals (MooGladeXML    *xml,
                 Widget         *node)
{
    GSList *l;

    g_return_if_fail (node != NULL);

    for (l = node->signals; l != NULL; l = l->next)
    {
        Signal *signal = l->data;
        gboolean connected = FALSE;
        gpointer object = NULL;

        if (!signal->name || !signal->name[0])
        {
            g_warning ("%s: empty signal name in widget %s",
                       G_STRLOC, node->id);
            continue;
        }

        if (!signal->handler || !signal->handler[0])
        {
            g_warning ("%s: empty handler of signal '%s' in widget %s",
                       G_STRLOC, signal->name, node->id);
            continue;
        }

        if (connect_special_signal (xml, node, signal))
            continue;

        if (xml->priv->signal_func)
        {
            connected = xml->priv->signal_func (xml, node->id,
                                                node->widget,
                                                signal->name,
                                                signal->handler,
                                                signal->object,
                                                xml->priv->signal_func_data);

            if (connected)
                continue;
        }

        if (signal->object)
        {
            object = moo_glade_xml_get_widget (xml, signal->object);

            if (!object)
            {
                g_warning ("%s: could not find object '%s' for signal '%s' of widget '%s'",
                           G_STRLOC, signal->object, signal->name, node->id);
                continue;
            }
        }

        g_warning ("%s: unconnected signal '%s' of widget '%s'",
                   G_STRLOC, signal->name, node->id);
    }

    for (l = node->children; l != NULL; l = l->next)
    {
        Child *child = l->data;

        if (child && child->widget)
            connect_signals (xml, child->widget);
    }
}


static GtkWidget*
create_widget (MooGladeXML *xml,
               Widget      *widget_node,
               GError     **error)
{
    GtkWidget *widget;

    widget = moo_glade_xml_create_widget (xml, widget_node, NULL, error);

    if (!widget ||
        !moo_glade_xml_add_widget (xml, widget_node->id, widget, error) ||
        !create_children (xml, widget_node, widget, error))
    {
        gtk_widget_destroy (widget);
        return NULL;
    }

    return widget;
}


/* XXX */
static void
set_special_props (MooGladeXML    *xml,
                   GtkWidget      *widget,
                   WidgetProps    *props)
{
    if (props->visible)
        gtk_widget_show (widget);

    if (props->mask & PROP_TOOLTIP)
        moo_widget_set_tooltip (widget, props->tooltip);

    if (props->mask & PROP_ENABLE_TOOLTIPS)
    {
        if (GTK_IS_TOOLBAR (widget))
            gtk_toolbar_set_tooltips (GTK_TOOLBAR (widget), TRUE);
        else
            g_warning ("%s: oops", G_STRLOC);
    }

    if (props->mask & PROP_HISTORY)
    {
        if (GTK_IS_OPTION_MENU (widget))
            gtk_option_menu_set_history (GTK_OPTION_MENU (widget),
                                         props->history);
        else
            g_warning ("%s: oops", G_STRLOC);
    }

    if (props->mask & PROP_RADIO_GROUP)
    {
        if (GTK_IS_RADIO_BUTTON (widget))
        {
            GtkWidget *group_button;
            GSList *group;

            group_button = moo_glade_xml_get_widget (xml, props->radio_group);

            g_return_if_fail (GTK_IS_RADIO_BUTTON (group_button));

            group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (group_button));
            gtk_radio_button_set_group (GTK_RADIO_BUTTON (widget), group);
        }
        else if (GTK_IS_RADIO_MENU_ITEM (widget))
        {
            GtkWidget *group_item;
            GSList *group;

            group_item = moo_glade_xml_get_widget (xml, props->radio_group);

            g_return_if_fail (GTK_IS_RADIO_MENU_ITEM (group_item));

            group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (group_item));
            gtk_radio_menu_item_set_group (GTK_RADIO_MENU_ITEM (widget), group);
        }
        else
        {
            g_return_if_reached ();
        }
    }
}


static GtkWidget*
moo_glade_xml_create_widget (MooGladeXML *xml,
                             Widget      *node,
                             GtkWidget   *widget,
                             GError     **error)
{
    WidgetProps *props;
    GType type = 0;

    g_return_val_if_fail (node != NULL, NULL);

    if (!widget)
    {
        FuncDataPair *pair;
        pair = g_hash_table_lookup (xml->priv->id_to_func, node->id);
        if (pair)
            widget = pair->func (xml, node->id, pair->data);
    }

    if (!widget)
        type = node->type;

    if (!widget && !type)
    {
        g_set_error (error, MOO_GLADE_XML_ERROR,
                     MOO_GLADE_XML_ERROR_FAILED,
                     "coudl not create widget '%s'",
                     node->id);
        return NULL;
    }

    props = node->props;

    if (!widget)
    {
        if (props->mask & PROP_LABEL)
        {
            if (!props->label)
            {
                g_warning ("%s: oops", G_STRLOC);
            }
            if (type == GTK_TYPE_MENU_ITEM)
            {
                if (props->mask & PROP_USE_UNDERLINE)
                    widget = gtk_menu_item_new_with_mnemonic (props->label);
                else
                    widget = gtk_menu_item_new_with_label (props->label);
            }
            else if (type == GTK_TYPE_IMAGE_MENU_ITEM)
            {
                if (props->mask & PROP_USE_STOCK)
                    widget = gtk_image_menu_item_new_from_stock (props->label, NULL);
                else if (props->mask & PROP_USE_UNDERLINE)
                    widget = gtk_image_menu_item_new_with_mnemonic (props->label);
                else
                    widget = gtk_image_menu_item_new_with_label (props->label);
            }
            else if (type == GTK_TYPE_RADIO_MENU_ITEM)
            {
                if (props->mask & PROP_USE_UNDERLINE)
                    widget = gtk_radio_menu_item_new_with_mnemonic (NULL, props->label);
                else
                    widget = gtk_radio_menu_item_new_with_label (NULL, props->label);
            }
            else if (type == GTK_TYPE_CHECK_BUTTON)
            {
                if (props->mask & PROP_USE_UNDERLINE)
                    widget = gtk_check_button_new_with_mnemonic (props->label);
                else
                    widget = gtk_check_button_new_with_label (props->label);
            }
            else if (type == GTK_TYPE_RADIO_BUTTON)
            {
                if (props->mask & PROP_USE_UNDERLINE)
                    widget = gtk_radio_button_new_with_mnemonic (NULL, props->label);
                else
                    widget = gtk_radio_button_new_with_label (NULL, props->label);
            }
            else if (type == GTK_TYPE_LIST_ITEM)
            {
                widget = gtk_list_item_new_with_label (props->label);
            }
            else
            {
                g_warning ("%s: oops", G_STRLOC);
            }
        }
    }

    if (!widget)
    {
        widget = g_object_newv (type, props->n_params,
                                props->params);
    }
    else
    {
        guint i;
        for (i = 0; i < props->n_params; i++)
            g_object_set_property (G_OBJECT (widget),
                                   props->params[i].name,
                                   &props->params[i].value);
    }

    g_return_val_if_fail (widget != NULL, NULL);

    return widget;
}


static gboolean
create_children (MooGladeXML  *xml,
                 Widget       *widget_node,
                 GtkWidget    *widget,
                 GError      **error)
{
    GSList *l;

    for (l = widget_node->children; l != NULL; l = l->next)
    {
        Child *child = l->data;
        if (!create_child (xml, widget, child, error))
            return FALSE;
    }

    pack_children (xml, widget_node, widget);
    return TRUE;
}


static gboolean
create_child (MooGladeXML    *xml,
              GtkWidget      *parent,
              Child          *child,
              GError        **error)
{
    GtkWidget *widget = NULL;

    g_return_val_if_fail (child != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_CONTAINER (parent), FALSE);

    if (child->empty)
        return TRUE;

    if (child->internal_child)
    {
        gboolean added = FALSE;
        GtkWidget *real_parent;

        real_parent = moo_glade_xml_get_widget (xml, child->internal_parent_id);

        if (!real_parent)
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not find widget '%s'",
                         child->internal_parent_id);
            return FALSE;
        }

        if (!strcmp (child->internal_child, "vbox") && GTK_IS_DIALOG (real_parent))
        {
            widget = GTK_DIALOG (real_parent)->vbox;
        }
        else if (!strcmp (child->internal_child, "action_area") && GTK_IS_DIALOG (real_parent))
        {
            widget = GTK_DIALOG (real_parent)->action_area;
        }
        else if (!strcmp (child->internal_child, "entry") && GTK_IS_COMBO (real_parent))
        {
            widget = GTK_COMBO (real_parent)->entry;
        }
        else if (!strcmp (child->internal_child, "list") && GTK_IS_COMBO (real_parent))
        {
            widget = GTK_COMBO (real_parent)->list;
        }
        else if (!strcmp (child->internal_child, "image") && GTK_IS_IMAGE_MENU_ITEM (real_parent))
        {
            g_object_get (real_parent, "image", &widget, NULL);

            if (!widget)
            {
                widget = create_widget (xml, child->widget, error);

                if (!widget)
                    return FALSE;

                gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (real_parent), widget);
                added = TRUE;
            }
        }

        if (!widget)
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not find internal child '%s' for widget '%s'",
                         child->internal_child, child->internal_parent_id);
            return FALSE;
        }

        if (!added && !moo_glade_xml_add_widget (xml, child->widget->id, widget, error))
            return FALSE;

        if (child->widget->props->n_params)
        {
            guint i;
            for (i = 0; i < child->widget->props->n_params; ++i)
                g_object_set_property (G_OBJECT (widget),
                                       child->widget->props->params[i].name,
                                       &child->widget->props->params[i].value);
        }

        if (!create_children (xml, child->widget, widget, error))
            return FALSE;

        set_special_props (xml, widget, child->widget->props);
    }
    else
    {
        widget = create_widget (xml, child->widget, error);

        if (!widget)
            return FALSE;
    }

    child->widget->widget = widget;
    set_custom_props (xml, child->widget);

    return TRUE;
}


static void
pack_children (MooGladeXML    *xml,
               Widget         *parent_node,
               GtkWidget      *parent)
{
    GSList *l;

    g_return_if_fail (parent_node != NULL);
    g_return_if_fail (parent != NULL);

    for (l = parent_node->children; l != NULL; l = l->next)
    {
        GtkWidget *widget;
        Child *child = l->data;
        gboolean packed = FALSE;

        if (child->empty || child->internal_child)
            continue;

        widget = child->widget->widget;
        g_return_if_fail (widget != NULL);

        if (GTK_IS_FRAME (parent) &&
            (child->props->mask & PACK_PROP_LABEL_ITEM))
        {
            gtk_frame_set_label_widget (GTK_FRAME (parent), widget);
            packed = TRUE;
        }
        else if (GTK_IS_NOTEBOOK (parent) &&
                 (child->props->mask & PACK_PROP_TAB))
        {
            int index = g_slist_index (parent_node->children, child);

            if (index <= 0)
            {
                g_warning ("%s: oops", G_STRLOC);
            }
            else
            {
                Child *page_child = g_slist_nth_data (parent_node->children, index - 1);

                if (!page_child->widget || !page_child->widget->widget)
                {
                    g_warning ("%s: empty notebook page with non-empty label", G_STRLOC);
                }
                else
                {
                    gtk_notebook_set_tab_label (GTK_NOTEBOOK (parent),
                                                page_child->widget->widget,
                                                widget);
                    packed = TRUE;
                }
            }
        }
        else if (GTK_IS_MENU_ITEM (parent) && GTK_IS_MENU (widget))
        {
            gtk_menu_item_set_submenu (GTK_MENU_ITEM (parent), widget);
            packed = TRUE;
        }
        else if (GTK_IS_OPTION_MENU (parent) && GTK_IS_MENU (widget))
        {
            gtk_option_menu_set_menu (GTK_OPTION_MENU (parent), widget);
            packed = TRUE;
        }
        else if (child->widget->props->mask & PROP_RESPONSE_ID)
        {
            Child *parent = child->parent_node->parent_node;
            if (!parent || !parent->internal_child ||
                 strcmp (parent->internal_child, "action_area"))
            {
                g_warning ("%s: oops", G_STRLOC);
            }
            else
            {
                GtkWidget *dialog = moo_glade_xml_get_widget (xml, parent->internal_parent_id);

                if (!dialog || !GTK_IS_DIALOG (dialog))
                {
                    g_warning ("%s: oops", G_STRLOC);
                }
                else
                {
                    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget,
                            child->widget->props->response_id);
                    packed = TRUE;
                }
            }
        }

        if (!packed)
            gtk_container_add (GTK_CONTAINER (parent), widget);

        if (child->props->n_params)
        {
            guint i;
            for (i = 0; i < child->props->n_params; ++i)
                gtk_container_child_set_property (GTK_CONTAINER (parent),
                        widget,
                        child->props->params[i].name,
                        &child->props->params[i].value);
        }

        set_special_props (xml, widget, child->widget->props);
        set_custom_props (xml, child->widget);
    }
}


static Signal*
signal_new (const char     *name,
            const char     *handler,
            const char     *object)
{
    Signal *signal = g_new (Signal, 1);
    signal->name = g_strdup (name);
    signal->handler = g_strdup (handler);
    signal->object = g_strdup (object);

    if (signal->name)
        g_strdelimit (signal->name, "-_", '-');

    return signal;
}


static void
signal_free (Signal *signal)
{
    if (signal)
    {
        g_free (signal->name);
        g_free (signal->handler);
        g_free (signal->object);
        g_free (signal);
    }
}


static void
collect_signals (MooMarkupNode  *node,
                 Widget         *widget)
{
    g_return_if_fail (node != NULL && NODE_IS_WIDGET (node));
    g_return_if_fail (widget != NULL);

    FOREACH_ELM_START (node, elm)
    {
        if (NODE_IS_SIGNAL (elm))
        {
            const char *name, *handler, *object;
            name = moo_markup_get_prop (elm, "name");
            handler = moo_markup_get_prop (elm, "handler");
            object = moo_markup_get_prop (elm, "object");
            g_return_if_fail (name != NULL && handler != NULL);
            widget->signals = g_slist_append (widget->signals,
                                              signal_new (name, handler, object));
        }
    }
    FOREACH_ELM_END;
}


static void
widget_free (Widget *widget)
{
    if (widget)
    {
        g_free (widget->id);
        widget_props_free (widget->props);
        g_slist_foreach (widget->children, (GFunc) child_free, NULL);
        g_slist_free (widget->children);
        g_slist_foreach (widget->signals, (GFunc) signal_free, NULL);
        g_slist_free (widget->signals);
        g_free (widget);
    }
}


static void
child_free (Child *child)
{
    if (child)
    {
        g_free (child->internal_child);
        g_free (child->internal_parent_id);
        widget_free (child->widget);
        packing_props_free (child->props);
        g_free (child);
    }
}


static void
widget_props_free (WidgetProps *props)
{
    if (props)
    {
        if (props->params)
            _moo_param_array_free (props->params, props->n_params);

        if (props->custom_props)
        {
            g_ptr_array_foreach (props->custom_props, (GFunc) g_free, NULL);
            g_ptr_array_free (props->custom_props, TRUE);
        }

        g_free (props->tooltip);
        g_free (props->mnemonic_widget);
        g_free (props->label);
        g_free (props->radio_group);
        g_free (props);
    }
}


static void
packing_props_free (PackingProps *props)
{
    if (props)
    {
        if (props->params)
            _moo_param_array_free (props->params, props->n_params);
        g_free (props);
    }
}


static Widget*
widget_new (MooGladeXML    *xml,
            Child          *parent,
            MooMarkupNode  *node,
            GError        **error)
{
    Widget *widget;
    WidgetProps *props;
    const char *id, *class_name;
    GType type;
    gboolean ignore_errors = FALSE;

    g_return_val_if_fail (NODE_IS_WIDGET (node), NULL);

    id = moo_markup_get_prop (node, "id");

    if (g_hash_table_lookup (xml->priv->id_to_func, id))
    {
        type = GTK_TYPE_WIDGET;
    }
    else
    {
        type = GPOINTER_TO_SIZE (g_hash_table_lookup (xml->priv->id_to_type, id));

        if (!type)
        {
            class_name = moo_markup_get_prop (node, "class");
            g_return_val_if_fail (id != NULL && class_name != NULL, NULL);

            type = GPOINTER_TO_SIZE (g_hash_table_lookup (xml->priv->class_to_type, class_name));

            if (!type)
            {
                ignore_errors = FALSE;
                type = get_type_by_name (class_name);
            }
        }

        if (!type)
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not find type of class '%s'",
                         class_name);
            return NULL;
        }
    }

    props = widget_props_new (node, type,
                              g_hash_table_lookup (xml->priv->props, id),
                              ignore_errors, xml->priv->translation_domain,
                              error);
    g_return_val_if_fail (props != NULL, NULL);

    widget = g_new0 (Widget, 1);
    widget->parent_node = parent;
    widget->id = g_strdup (id);
    widget->type = type;
    widget->props = props;
    widget->children = NULL;

    collect_signals (node, widget);

    FOREACH_ELM_START (node, elm)
    {
        if (NODE_IS_CHILD (elm))
        {
            Child *child = child_new (xml, widget, elm, error);

            if (!child)
            {
                widget_free (widget);
                g_return_val_if_fail (child != NULL, NULL);
            }

            widget->children = g_slist_prepend (widget->children, child);
        }
    }
    FOREACH_ELM_END;

    widget->children = g_slist_reverse (widget->children);
    return widget;
}


static Child*
child_new (MooGladeXML    *xml,
           Widget         *parent,
           MooMarkupNode  *node,
           GError        **error)
{
    Child *child;
    PackingProps *props;
    const char *class_name, *internal_child, *internal_parent_id = NULL;
    MooMarkupNode *widget_node, *packing_node;

    g_return_val_if_fail (parent != NULL, NULL);
    g_return_val_if_fail (NODE_IS_CHILD (node), NULL);

    if (moo_markup_get_element (node, "placeholder"))
    {
        child = g_new0 (Child, 1);
        child->empty = TRUE;
        child->parent_node = parent;
        return child;
    }

    widget_node = moo_markup_get_element (node, "widget");

    if (!widget_node)
    {
        char *text = moo_markup_node_get_string (node);
        g_set_error (error, MOO_GLADE_XML_ERROR,
                     MOO_GLADE_XML_ERROR_FAILED,
                     "no widget element in '%s'",
                     text);
        g_free (text);
        return NULL;
    }

    class_name = moo_markup_get_prop (widget_node, "class");

    if (!class_name)
    {
        g_set_error (error, MOO_GLADE_XML_ERROR,
                     MOO_GLADE_XML_ERROR_FAILED,
                     "no class attribute in widget element");
        return NULL;
    }

    if (!strcmp (class_name, "GtkToolbar"))
    {
        g_warning ("Ignoring %s", class_name);
        child = g_new0 (Child, 1);
        child->empty = TRUE;
        child->parent_node = parent;
        return child;
    }

    internal_child = moo_markup_get_prop (node, "internal-child");

    if (internal_child)
    {
        GType parent_type;
        Widget *real_parent;

        if (!strcmp (internal_child, "vbox") ||
             !strcmp (internal_child, "action_area"))
        {
            parent_type = GTK_TYPE_DIALOG;
        }
        else if (!strcmp (internal_child, "entry") ||
                  !strcmp (internal_child, "list"))
        {
            parent_type = GTK_TYPE_COMBO;
        }
        else if (!strcmp (internal_child, "image"))
        {
            parent_type = GTK_TYPE_IMAGE_MENU_ITEM;
        }
        else
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "unknown internal child name '%s'",
                         internal_child);
            return NULL;
        }

        real_parent = parent;

        while (TRUE)
        {
            if (!g_type_is_a (real_parent->type, parent_type))
            {
                if (!real_parent->parent_node)
                {
                    g_set_error (error, MOO_GLADE_XML_ERROR,
                                 MOO_GLADE_XML_ERROR_FAILED,
                                 "could not find parent of '%s'",
                                 internal_child);
                    return NULL;
                }
                else
                {
                    real_parent = real_parent->parent_node->parent_node;
                }
            }
            else
            {
                internal_parent_id = real_parent->id;
                break;
            }
        }
    }

    packing_node = moo_markup_get_element (node, "packing");

    if (packing_node)
        props = packing_props_new (packing_node, parent->type, error);
    else
        props = packing_props_new (NULL, 0, error);

    if (!props)
        return NULL;

    child = g_new0 (Child, 1);
    child->parent_node = parent;
    child->internal_child = g_strdup (internal_child);
    child->internal_parent_id = g_strdup (internal_parent_id);
    child->props = props;

    child->widget = widget_new (xml, child, widget_node, error);

    if (!child->widget)
    {
        child_free (child);
        return NULL;
    }

    return child;
}


static gboolean
widget_props_add (WidgetProps  *props,
                  GArray       *params,
                  GObjectClass *klass,
                  const char   *name,
                  const char   *value,
                  gboolean      ignore_errors,
                  GError      **error)
{
    GParameter param = {NULL, {0, {{0}, {0}}}};
    gboolean special = TRUE;

    if (!strcmp (name, "visible"))
    {
        props->visible = parse_bool (value);
    }
    else if (!strcmp (name, "response_id"))
    {
        props->response_id = parse_int (value);
        props->mask |= PROP_RESPONSE_ID;
    }
    else if (!strcmp (name, "has_default"))
    {
        props->has_default = parse_bool (value);
        props->mask |= PROP_HAS_DEFAULT;
    }
    else if (!strcmp (name, "has_focus"))
    {
        props->has_focus = parse_bool (value);
        props->mask |= PROP_HAS_FOCUS;
    }
    else if (!strcmp (name, "tooltip"))
    {
        props->tooltip = g_strdup (value);
        props->mask |= PROP_TOOLTIP;
    }
    else if (!strcmp (name, "mnemonic_widget") &&
              GTK_IS_LABEL_CLASS (klass))
    {
        props->mnemonic_widget = g_strdup (value);
        props->mask |= PROP_MNEMONIC_WIDGET;
    }
    else if (!strcmp (name, "text") &&
              GTK_IS_TEXT_VIEW_CLASS (klass))
    {
        if (value && value[0])
            g_message ("%s: ignoring TextView text property", G_STRLOC);
    }
    else if (!strcmp (name, "group") &&
              (GTK_IS_RADIO_BUTTON_CLASS (klass) ||
                      GTK_IS_RADIO_MENU_ITEM_CLASS (klass)))
    {
        props->radio_group = g_strdup (value);
        props->mask |= PROP_RADIO_GROUP;
    }
    else if (!strcmp (name, "label") &&
              (GTK_IS_MENU_ITEM_CLASS (klass) ||
                      GTK_IS_CHECK_BUTTON_CLASS (klass) ||
                      GTK_IS_LIST_ITEM_CLASS (klass)))
    {
        props->label = g_strdup (value);
        props->mask |= PROP_LABEL;
    }
    else if (!strcmp (name, "use_underline") &&
              GTK_IS_MENU_ITEM_CLASS (klass))
    {
        props->mask |= PROP_USE_UNDERLINE;
    }
    else if (!strcmp (name, "use_stock") &&
              GTK_IS_IMAGE_MENU_ITEM_CLASS (klass))
    {
        props->mask |= PROP_USE_STOCK;
    }
    else if (!strcmp (name, "tooltips") &&
              GTK_IS_TOOLBAR_CLASS (klass))
    {
        props->mask |= PROP_ENABLE_TOOLTIPS;
    }
    else if (!strcmp (name, "history") &&
              GTK_IS_OPTION_MENU_CLASS (klass))
    {
        props->mask |= PROP_HISTORY;
        props->history = parse_int (value);
    }
#if GTK_CHECK_VERSION(2,4,0)
    else if (!strcmp (name, "items") &&
             GTK_IS_COMBO_BOX_CLASS (klass))
    {
//         if (value && value[0])
//             g_message ("%s: ignoring ComboBox items property", G_STRLOC);
    }
#endif /* GTK_CHECK_VERSION(2,4,0) */
    else
    {
        special = FALSE;
    }

    if (!special)
    {
        GParamSpec *param_spec = g_object_class_find_property (klass, name);

        if (!param_spec)
        {
            if (!props->custom_props)
                props->custom_props = g_ptr_array_new ();
            g_ptr_array_add (props->custom_props, g_strdelimit (g_strdup (name), "_", '-'));
            g_ptr_array_add (props->custom_props, g_strdup (value));
        }
        else if (parse_property (param_spec, value, &param))
        {
            g_array_append_val (params, param);
        }
        else if (!ignore_errors)
        {
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not convert '%s' to property '%s'",
                         value, name);
            return FALSE;
        }
    }

    return TRUE;
}


static void
widget_props_add_one (const char *name,
                      const char *value,
                      gpointer    user_data)
{
    struct {
        WidgetProps *props;
        GArray *params;
        GObjectClass *klass;
        gboolean result;
        gboolean ignore_errors;
        GError **error;
    } *data = user_data;

    if (!data->result)
        return;

    data->result = widget_props_add (data->props, data->params,
                                     data->klass, name, value,
                                     data->ignore_errors,
                                     data->error);
}


static const char *
get_property_text (MooMarkupNode *node,
                   const char    *translation_domain)
{
    gboolean translatable = FALSE, strip_context = FALSE;
    const char *prop, *translatable_attr;

#ifndef ENABLE_NLS
    return moo_markup_get_content (node);
#endif

    prop = moo_markup_get_content (node);

    if (!prop || !prop[0])
        return prop;

    translatable_attr = moo_markup_get_prop (node, "translatable");

    if (translatable_attr && !strcmp (translatable_attr, "yes"))
    {
        const char *ctx_attr = moo_markup_get_prop (node, "context");

        if (ctx_attr && !strcmp (ctx_attr, "yes"))
            strip_context = TRUE;

        translatable = TRUE;
    }

    if (!translatable)
        return prop;

    if (translation_domain)
    {
        const char *translated;

        if (strip_context)
            translated = g_strip_context (prop, dgettext (prop, translation_domain));
        else
            translated = dgettext (prop, translation_domain);

        if (strcmp (translated, translation_domain) != 0)
            return translated;
    }

    if (strip_context)
        return g_strip_context (prop, gettext (prop));
    else
        return gettext (prop);
}


static WidgetProps*
widget_props_new (MooMarkupNode  *node,
                  GType           type,
                  GHashTable     *add_props,
                  gboolean        ignore_errors,
                  const char     *translation_domain,
                  GError        **error)
{
    GArray *params;
    GObjectClass *klass;
    WidgetProps *props;

    g_return_val_if_fail (node != NULL, FALSE);

    klass = g_type_class_ref (type);
    g_return_val_if_fail (klass != NULL, FALSE);

    props = g_new0 (WidgetProps, 1);
    params = g_array_new (FALSE, FALSE, sizeof (GParameter));

    props->visible = FALSE;

    FOREACH_ELM_START(node, elm)
    {
        if (NODE_IS_PROPERTY (elm))
        {
            const char *name = moo_markup_get_prop (elm, "name");
            const char *value = get_property_text (elm, translation_domain);
            gboolean result;

            result = widget_props_add (props, params, klass,
                                       name, value,
                                       ignore_errors,
                                       error);

            if (!result)
                goto error;
        }
    }
    FOREACH_ELM_END;

    if (add_props)
    {
        struct {
            WidgetProps *props;
            GArray *params;
            GObjectClass *klass;
            gboolean result;
            gboolean ignore_errors;
            GError **error;
        } data = {props, params, klass, TRUE, ignore_errors, error};

        g_hash_table_foreach (add_props, (GHFunc) widget_props_add_one, &data);

        if (!data.result)
            goto error;
    }

    props->n_params = params->len;
    props->params = (GParameter*) g_array_free (params, FALSE);
    g_type_class_unref (klass);
    return props;

error:
    _moo_param_array_free ((GParameter*) params->data, params->len);
    g_array_free (params, FALSE);
    g_type_class_unref (klass);
    widget_props_free (props);
    return FALSE;
}


static PackingProps *
packing_props_new (MooMarkupNode  *node,
                   GType           container_type,
                   GError        **error)
{
    GArray *params;
    GObjectClass *klass;
    PackingProps *props;

    if (!node && !container_type)
        return g_new0 (PackingProps, 1);

    g_return_val_if_fail (node != NULL, FALSE);

    klass = g_type_class_ref (container_type);
    g_return_val_if_fail (klass != NULL, FALSE);

    props = g_new0 (PackingProps, 1);
    params = g_array_new (FALSE, FALSE, sizeof (GParameter));

    FOREACH_ELM_START(node, elm)
    {
        if (NODE_IS_PROPERTY (elm))
        {
            GParameter param = {NULL, {0, {{0}, {0}}}};
            const char *name = moo_markup_get_prop (elm, "name");
            const char *value = moo_markup_get_content (elm);
            gboolean special = FALSE;

            if (!strcmp (name, "type"))
            {
                if (!strcmp (value, "label_item") && GTK_IS_FRAME_CLASS (klass))
                {
                    special = TRUE;
                    props->mask |= PACK_PROP_LABEL_ITEM;
                }
                else if (!strcmp (value, "tab") && GTK_IS_NOTEBOOK_CLASS (klass))
                {
                    special = TRUE;
                    props->mask |= PACK_PROP_TAB;
                }
            }

            if (!special)
            {
                GParamSpec *param_spec = gtk_container_class_find_child_property (klass, name);

                if (!param_spec)
                {
                    g_set_error (error, MOO_GLADE_XML_ERROR,
                                 MOO_GLADE_XML_ERROR_FAILED,
                                 "could not find property '%s'",
                                 name);
                    _moo_param_array_free ((GParameter*) params->data, params->len);
                    g_array_free (params, FALSE);
                    g_type_class_unref (klass);
                    packing_props_free (props);
                    return NULL;
                }

                if (parse_property (param_spec, value, &param))
                {
                    g_array_append_val (params, param);
                }
                else
                {
                    g_set_error (error, MOO_GLADE_XML_ERROR,
                                 MOO_GLADE_XML_ERROR_FAILED,
                                 "could not convert '%s' to property '%s'",
                                 value, name);
                    _moo_param_array_free ((GParameter*) params->data, params->len);
                    g_array_free (params, FALSE);
                    g_type_class_unref (klass);
                    packing_props_free (props);
                    return NULL;
                }
            }
        }
    }
    FOREACH_ELM_END;

    props->n_params = params->len;
    props->params = (GParameter*) g_array_free (params, FALSE);
    g_type_class_unref (klass);
    return props;
}


static gboolean
parse_property (GParamSpec     *param_spec,
                const char     *value,
                GParameter     *param)
{
    gboolean take_null = FALSE;

    g_return_val_if_fail (param_spec != NULL, FALSE);

    if (param_spec->value_type == GTK_TYPE_ADJUSTMENT)
        take_null = TRUE;

    if (!value && !take_null)
    {
        g_warning ("NULL value for property '%s' of type '%s'",
                   param_spec->name,
                   g_type_name (param_spec->value_type));
        return FALSE;
    }

    g_value_init (&param->value, param_spec->value_type);

    errno = 0;

    if (param_spec->value_type == G_TYPE_CHAR)
    {
        g_value_set_char (&param->value, value[0]);
    }
    else if (param_spec->value_type == G_TYPE_UCHAR)
    {
        g_value_set_uchar (&param->value, value[0]);
    }
    else if (param_spec->value_type == G_TYPE_BOOLEAN)
    {
        g_value_set_boolean (&param->value, parse_bool (value));
    }
    else if (param_spec->value_type == G_TYPE_INT ||
             param_spec->value_type == G_TYPE_LONG ||
                 param_spec->value_type == G_TYPE_INT64) /* XXX */
    {
        long val = strtol (value, NULL, 0);

        if (errno)
        {
            g_warning ("%s: could not convert string '%s' to an int",
                       G_STRLOC, value);
            return FALSE;
        }
        else
        {
            if (param_spec->value_type == G_TYPE_INT)
                g_value_set_int (&param->value, val);
            else if (param_spec->value_type == G_TYPE_LONG)
                g_value_set_long (&param->value, val);
            else
                g_value_set_int64 (&param->value, val);
        }
    }
    else if (param_spec->value_type == G_TYPE_UINT ||
             param_spec->value_type == G_TYPE_ULONG ||
             param_spec->value_type == G_TYPE_UINT64) /* XXX */
    {
        guint64 val = g_ascii_strtoull (value, NULL, 0);

        if (errno)
        {
            g_warning ("%s: could not convert string '%s' to a guint",
                       G_STRLOC, value);
            return FALSE;
        }
        else
        {
            if (param_spec->value_type == G_TYPE_UINT)
                g_value_set_uint (&param->value, val);
            else if (param_spec->value_type == G_TYPE_ULONG)
                g_value_set_ulong (&param->value, val);
            else
                g_value_set_uint64 (&param->value, val);
        }
    }
    else if (param_spec->value_type == G_TYPE_FLOAT ||
             param_spec->value_type == G_TYPE_DOUBLE) /* XXX */
    {
        double val = g_ascii_strtod (value, NULL);

        if (errno)
        {
            g_warning ("%s: could not convert string '%s' to double",
                       G_STRLOC, value);
            return FALSE;
        }
        else
        {
            if (param_spec->value_type == G_TYPE_FLOAT)
                g_value_set_float (&param->value, val);
            else
                g_value_set_double (&param->value, val);
        }
    }
    else if (param_spec->value_type == G_TYPE_STRING)
    {
        g_value_set_string (&param->value, value);
    }
    else if (G_TYPE_IS_ENUM (param_spec->value_type))
    {
        GEnumClass *enum_class;
        GEnumValue *val;

        enum_class = g_type_class_ref (param_spec->value_type);

        val = g_enum_get_value_by_name (enum_class, value);

        if (!val)
            val = g_enum_get_value_by_nick (enum_class, value);

        g_type_class_unref (enum_class);

        if (!val)
        {
            const char *typename = g_type_name (param_spec->value_type);
            g_warning ("%s: can not convert string '%s' to a value of type %s",
                       G_STRLOC, value, typename ? typename : "<unknown>");
            return FALSE;
        }

        g_value_set_enum (&param->value, val->value);
    }
    else if (G_TYPE_IS_FLAGS (param_spec->value_type))
    {
        if (!value[0])
        {
            g_value_set_flags (&param->value, 0);
        }
        else
        {
            GFlagsClass *flags_class;
            char **pieces;
            guint i, len;
            int result = 0;

            pieces = g_strsplit (value, "|", 0);
            len = g_strv_length (pieces);

            flags_class = g_type_class_ref (param_spec->value_type);

            for (i = 0; i < len; ++i)
            {
                GFlagsValue *val;

                val = g_flags_get_value_by_name (flags_class, pieces[i]);

                if (!val)
                    val = g_flags_get_value_by_nick (flags_class, pieces[i]);

                if (!val)
                {
                    const char *typename = g_type_name (param_spec->value_type);
                    g_warning ("%s: can not convert string '%s' to a value of type %s",
                               G_STRLOC, pieces[i], typename ? typename : "<unknown>");
                    g_type_class_unref (flags_class);
                    return FALSE;
                }
                else
                {
                    result |= val->value;
                }
            }

            g_value_set_flags (&param->value, result);
            g_type_class_unref (flags_class);
            g_strfreev (pieces);
        }
    }
    else if (param_spec->value_type == GDK_TYPE_PIXBUF)
    {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (value, NULL);

        if (!pixbuf)
        {
            g_message ("%s: could not find %s", G_STRLOC, value);
            g_value_set_object (&param->value, NULL);
        }
        else
        {
            g_value_set_object (&param->value, pixbuf);
            g_object_unref (pixbuf);
        }
    }
    else if (param_spec->value_type == GTK_TYPE_ADJUSTMENT)
    {
        GtkObject *adjustment = parse_adjustment (value);

        if (!adjustment)
        {
            const char *typename = g_type_name (param_spec->value_type);
            g_warning ("%s: could not convert string '%s' to a value of type %s",
                       G_STRLOC, value, typename ? typename : "<unknown>");
            return FALSE;
        }
        else
        {
            gtk_object_sink (g_object_ref (adjustment));
            g_value_set_object (&param->value, adjustment);
            g_object_unref (adjustment);
        }
    }
    else if (param_spec->value_type == G_TYPE_STRV)
    {
        char **strv = moo_splitlines (value);
        g_value_take_boxed (&param->value, strv);
    }
    else
    {
        const char *typename = g_type_name (param_spec->value_type);
        g_warning ("%s: could not convert string '%s' to a value of type %s",
                   G_STRLOC, value, typename ? typename : "<unknown>");
        return FALSE;
    }

    param->name = g_strdup (param_spec->name);

    return TRUE;
}


static gboolean
parse_bool (const char *value)
{
    g_return_val_if_fail (value != NULL, FALSE);
    return !g_ascii_strcasecmp (value, "true");
}


static int
parse_int (const char *value)
{
    long val;

    g_return_val_if_fail (value != NULL, 0);

    errno = 0;
    val = strtol (value, NULL, 0);

    if (errno)
    {
        g_warning ("%s: could not convert string '%s' to an int",
                   G_STRLOC, value);
        return 0;
    }
    else
    {
        return val;
    }
}


static GtkObject*
parse_adjustment (const char *value)
{
    char **pieces;
    GtkObject *adj = NULL;
    double vals[6];
    guint i;

    /* XXX is this correct? */
    if (!value)
        return g_object_new (GTK_TYPE_ADJUSTMENT, NULL);

    pieces = g_strsplit (value, " ", 0);
    g_return_val_if_fail (pieces != NULL, NULL);

    if (g_strv_length (pieces) != 6)
        goto out;

    errno = 0;

    for (i = 0; i < 6; ++i)
    {
        vals[i] = g_ascii_strtod (pieces[i], NULL);

        if (errno)
        {
            g_warning ("could not convert '%s' to double", pieces[i]);
            goto out;
        }
    }

    adj = gtk_adjustment_new (vals[0], vals[1], vals[2],
                              vals[3], vals[4], vals[5]);

out:
    g_strfreev (pieces);
    return adj;
}


static void
moo_glade_xml_class_init (MooGladeXMLClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = moo_glade_xml_dispose;
}


static void
moo_glade_xml_init (MooGladeXML *xml)
{
    xml->priv = g_new0 (MooGladeXMLPrivate, 1);

    xml->priv->widgets = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                g_free, NULL);
    xml->priv->class_to_type = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                      g_free, NULL);
    xml->priv->id_to_type = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                   g_free, NULL);
    xml->priv->id_to_func = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                   (GDestroyNotify) func_data_pair_free);
    xml->priv->props = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                              (GDestroyNotify) g_hash_table_destroy);
}


static char *
normalize_prop_name (const char *prop_name)
{
    char *norm, *p;

    norm = g_strdup (prop_name);
    p = norm;

    while ((p = strchr (p, '_')))
        *p = '-';

    return norm;
}


void
moo_glade_xml_set_property (MooGladeXML    *xml,
                            const char     *widget,
                            const char     *prop_name,
                            const char     *value)
{
    GHashTable *props;
    char *norm_prop_name;

    g_return_if_fail (MOO_IS_GLADE_XML (xml));
    g_return_if_fail (widget != NULL && prop_name != NULL);

    props = g_hash_table_lookup (xml->priv->props, widget);

    if (!props && !value)
        return;

    norm_prop_name = normalize_prop_name (prop_name);

    if (!props)
    {
        props = g_hash_table_new_full (g_str_hash, g_str_equal,
                                       g_free, g_free);
        g_hash_table_insert (xml->priv->props, g_strdup (widget), props);
    }

    if (value)
    {
        g_hash_table_insert (props, norm_prop_name, g_strdup (value));
    }
    else
    {
        g_hash_table_remove (props, norm_prop_name);
        g_free (norm_prop_name);
    }
}


MooGladeXML *
moo_glade_xml_new_empty (const char *domain)
{
    MooGladeXML *xml = g_object_new (MOO_TYPE_GLADE_XML, NULL);
    xml->priv->translation_domain = g_strdup (domain);
    return xml;
}


gboolean
moo_glade_xml_parse_file (MooGladeXML    *xml,
                          const char     *file,
                          const char     *root,
                          GError        **error)
{
    MooMarkupDoc *doc;
    gboolean result;

    g_return_val_if_fail (xml != NULL, FALSE);
    g_return_val_if_fail (file != NULL, FALSE);

    doc = moo_markup_parse_file (file, error);

    if (!doc)
        return FALSE;

    result = moo_glade_xml_parse_markup (xml, doc, root, NULL, error);

    moo_markup_doc_unref (doc);
    return result;
}


gboolean
moo_glade_xml_parse_memory (MooGladeXML    *xml,
                            const char     *buffer,
                            int             size,
                            const char     *root,
                            GError        **error)
{
    MooMarkupDoc *doc;
    gboolean result;
    GError *error_here = NULL;

    g_return_val_if_fail (xml != NULL, FALSE);
    g_return_val_if_fail (buffer != NULL, FALSE);

    doc = moo_markup_parse_memory (buffer, size, &error_here);

    if (!doc)
    {
        if (error)
        {
            g_propagate_error (error, error_here);
        }
        else
        {
            g_warning ("could not parse markup: %s",
                       error_here->message);
            g_error_free (error_here);
        }

        return FALSE;
    }

    result = moo_glade_xml_parse_markup (xml, doc, root, NULL, &error_here);

    if (!result)
    {
        if (error)
        {
            g_propagate_error (error, error_here);
        }
        else
        {
            g_warning ("%s: %s", G_STRLOC, error_here->message);
            g_error_free (error_here);
        }
    }

    moo_markup_doc_unref (doc);
    return result;
}


MooGladeXML *
moo_glade_xml_new (const char     *file,
                   const char     *root,
                   const char     *domain,
                   GError        **error)
{
    MooGladeXML *xml;

    g_return_val_if_fail (file != NULL, NULL);

    xml = moo_glade_xml_new_empty (domain);

    if (!moo_glade_xml_parse_file (xml, file, root, error))
    {
        g_object_unref (xml);
        return NULL;
    }

    return xml;
}


MooGladeXML*
moo_glade_xml_new_from_buf (const char     *buffer,
                            int             size,
                            const char     *root,
                            const char     *domain,
                            GError        **error)
{
    MooGladeXML *xml;
    GError *error_here = NULL;

    g_return_val_if_fail (buffer != NULL, NULL);

    xml = moo_glade_xml_new_empty (domain);

    if (!moo_glade_xml_parse_memory (xml, buffer, size, root, &error_here))
    {
        if (error)
        {
            g_propagate_error (error, error_here);
        }
        else
        {
            g_warning ("%s: %s", G_STRLOC, error_here->message);
            g_error_free (error_here);
        }

        g_object_unref (xml);
        return NULL;
    }

    return xml;
}


static MooMarkupNode*
find_widget (MooMarkupNode *node,
             const char    *id)
{
    const char *wid_id;

    g_return_val_if_fail (NODE_IS_WIDGET (node), NULL);

    wid_id = moo_markup_get_prop (node, "id");
    g_return_val_if_fail (wid_id != NULL, NULL);

    if (!strcmp (wid_id, id))
        return node;

    FOREACH_ELM_START(node, elm)
    {
        if (NODE_IS_CHILD (elm))
        {
            FOREACH_ELM_START (elm, child)
            {
                if (NODE_IS_WIDGET (child))
                {
                    MooMarkupNode *wid = find_widget (child, id);
                    if (wid)
                        return wid;
                }
            }
            FOREACH_ELM_END;
        }
    }
    FOREACH_ELM_END;

    return NULL;
}


static gboolean
moo_glade_xml_parse_markup (MooGladeXML  *xml,
                            MooMarkupDoc *doc,
                            const char   *root_id,
                            GtkWidget    *root_widget,
                            GError      **error)
{
    MooMarkupNode *glade_elm;
    MooMarkupNode *root = NULL;
    Widget *widget = NULL;
    gboolean result = FALSE;

    g_return_val_if_fail (doc != NULL, FALSE);
    g_return_val_if_fail (!xml->priv->done, FALSE);
    g_return_val_if_fail (!root_widget || GTK_IS_WIDGET (root_widget), FALSE);

    glade_elm = moo_markup_get_root_element (doc, "glade-interface");

    if (!glade_elm)
    {
        g_set_error (error, MOO_GLADE_XML_ERROR,
                     MOO_GLADE_XML_ERROR_FAILED,
                     "no 'glade-interface' element");
        return FALSE;
    }

    FOREACH_ELM_START(glade_elm, elm)
    {
        if (NODE_IS_WIDGET (elm))
        {
            if (root_id)
            {
                root = find_widget (elm, root_id);

                if (root)
                    break;
            }
            else
            {
                root = elm;
                break;
            }
        }
        else if (!strcmp (elm->name, "requires"))
        {
            g_message ("%s: ignoring '%s'", G_STRLOC, elm->name);
        }
        else
        {
            g_warning ("%s: invalid element '%s'", G_STRLOC, elm->name);
        }
    }
    FOREACH_ELM_END;

    if (!root)
    {
        if (root_id)
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not find root element '%s'",
                         root_id);
        else
            g_set_error (error, MOO_GLADE_XML_ERROR,
                         MOO_GLADE_XML_ERROR_FAILED,
                         "could not find root element");

        goto out;
    }

    widget = widget_new (xml, NULL, root, error);

    if (!widget)
        goto out;

    dump_widget (widget);

    if (!moo_glade_xml_build (xml, widget, root_widget, error))
        goto out;

#if 0
    gtk_widget_show_all (widget->widget);
#endif

    xml->priv->root_id = g_strdup (widget->id);
    result = TRUE;

out:
    if (widget)
        widget_free (widget);
    moo_glade_xml_cleanup (xml);
    xml->priv->done = TRUE;
    return result;
}


gboolean
moo_glade_xml_fill_widget (MooGladeXML    *xml,
                           GtkWidget      *target,
                           const char     *buffer,
                           int             size,
                           const char     *target_name,
                           GError        **error)
{
    MooMarkupDoc *doc;
    gboolean result;

    g_return_val_if_fail (xml != NULL, FALSE);
    g_return_val_if_fail (buffer != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (target), FALSE);

    doc = moo_markup_parse_memory (buffer, size, error);

    if (!doc)
        return FALSE;

    result = moo_glade_xml_parse_markup (xml, doc,
                                         target_name,
                                         target,
                                         error);

    moo_markup_doc_unref (doc);
    return result;
}


GtkWidget*
moo_glade_xml_get_root (MooGladeXML *xml)
{
    g_return_val_if_fail (xml != NULL, NULL);
    return moo_glade_xml_get_widget (xml, xml->priv->root_id);
}


static gboolean
cmp_widget (G_GNUC_UNUSED gpointer key,
            gpointer widget,
            gpointer dead)
{
    return widget == dead;
}

static void
widget_destroyed (MooGladeXML *xml,
                  gpointer     widget)
{
    guint removed;
    removed = g_hash_table_foreach_remove (xml->priv->widgets,
                                           (GHRFunc) cmp_widget, widget);
    g_return_if_fail (removed == 1);
}

static gboolean
moo_glade_xml_add_widget (MooGladeXML    *xml,
                          const char     *id,
                          GtkWidget      *widget,
                          GError        **error)
{
    g_return_val_if_fail (id != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

    if (g_hash_table_lookup (xml->priv->widgets, id))
    {
        g_set_error (error, MOO_GLADE_XML_ERROR,
                     MOO_GLADE_XML_ERROR_FAILED,
                     "duplicated id '%s'", id);
        return FALSE;
    }

    g_assert (!g_hash_table_lookup (xml->priv->widgets, id));
    g_hash_table_insert (xml->priv->widgets, g_strdup (id), widget);
    g_object_weak_ref (G_OBJECT (widget), (GWeakNotify) widget_destroyed, xml);

    return TRUE;
}


static void
moo_glade_xml_cleanup (MooGladeXML *xml)
{
    if (xml->priv->class_to_type)
        g_hash_table_destroy (xml->priv->class_to_type);
    if (xml->priv->id_to_type)
        g_hash_table_destroy (xml->priv->id_to_type);
    if (xml->priv->id_to_func)
        g_hash_table_destroy (xml->priv->id_to_func);
    if (xml->priv->props)
        g_hash_table_destroy (xml->priv->props);

    xml->priv->class_to_type = NULL;
    xml->priv->id_to_type = NULL;
    xml->priv->id_to_func = NULL;
    xml->priv->props = NULL;
}


static void
unref_widget (G_GNUC_UNUSED gpointer key,
              GObject     *widget,
              MooGladeXML *xml)
{
    g_object_weak_unref (widget, (GWeakNotify) widget_destroyed, xml);
}

static void
moo_glade_xml_dispose (GObject *object)
{
    MooGladeXML *xml = MOO_GLADE_XML (object);

    if (xml->priv)
    {
        moo_glade_xml_cleanup (xml);
        g_hash_table_foreach (xml->priv->widgets,
                              (GHFunc) unref_widget, xml);
        g_hash_table_destroy (xml->priv->widgets);
        g_free (xml->priv->translation_domain);
        g_free (xml->priv->root_id);
        g_free (xml->priv);
        xml->priv = NULL;
    }

    G_OBJECT_CLASS(moo_glade_xml_parent_class)->dispose (object);
}


gpointer
moo_glade_xml_get_widget (MooGladeXML    *xml,
                          const char     *id)
{
    g_return_val_if_fail (xml != NULL, NULL);
    g_return_val_if_fail (id != NULL, NULL);
    return g_hash_table_lookup (xml->priv->widgets, id);
}


void
moo_glade_xml_map_class (MooGladeXML    *xml,
                         const char     *class_name,
                         GType           type)
{
    g_return_if_fail (xml != NULL);
    g_return_if_fail (class_name != NULL);

    if (type)
        g_hash_table_insert (xml->priv->class_to_type,
                             g_strdup (class_name),
                             GSIZE_TO_POINTER (type));
    else
        g_hash_table_remove (xml->priv->class_to_type,
                             class_name);
}


void
moo_glade_xml_map_id (MooGladeXML    *xml,
                      const char     *id,
                      GType           use_type)
{
    g_return_if_fail (xml != NULL);
    g_return_if_fail (id != NULL);

    if (use_type)
        g_hash_table_insert (xml->priv->id_to_type,
                             g_strdup (id),
                             GSIZE_TO_POINTER (use_type));
    else
        g_hash_table_remove (xml->priv->id_to_type, id);
}


void         moo_glade_xml_map_custom   (MooGladeXML    *xml,
                                         const char     *id,
                                         MooGladeCreateFunc func,
                                         gpointer        data)
{
    g_return_if_fail (xml != NULL);
    g_return_if_fail (id != NULL);

    if (func)
        g_hash_table_insert (xml->priv->id_to_func,
                             g_strdup (id),
                             func_data_pair_new (func, data));
    else
        g_hash_table_remove (xml->priv->id_to_func, id);
}


void
moo_glade_xml_set_signal_func (MooGladeXML    *xml,
                               MooGladeSignalFunc func,
                               gpointer        data)
{
    g_return_if_fail (xml != NULL);
    xml->priv->signal_func = func;
    xml->priv->signal_func_data = data;
}


void
moo_glade_xml_set_prop_func (MooGladeXML    *xml,
                             MooGladePropFunc func,
                             gpointer        data)
{
    g_return_if_fail (xml != NULL);
    xml->priv->prop_func = func;
    xml->priv->prop_func_data = data;
}


static FuncDataPair*
func_data_pair_new (MooGladeCreateFunc func,
                    gpointer        data)
{
    FuncDataPair *pair;

    g_return_val_if_fail (func != NULL, NULL);

    pair = g_new (FuncDataPair, 1);
    pair->func = func;
    pair->data = data;

    return pair;
}


static void
func_data_pair_free (FuncDataPair *pair)
{
    g_free (pair);
}


GQuark
moo_glade_xml_error_quark (void)
{
    return g_quark_from_static_string ("moo-glade-xml-error");
}


#define REGISTER_TYPE(class_name,type)  \
    if (!strcmp (name, class_name))     \
        return type


static GType
get_type_by_name (const char *name)
{
    GType type = g_type_from_name (name);

    if (type)
        return type;

#if GTK_CHECK_VERSION(2,6,0)
    REGISTER_TYPE ("GtkAboutDialog", GTK_TYPE_ABOUT_DIALOG);
    REGISTER_TYPE ("GtkCellRendererCombo", GTK_TYPE_CELL_RENDERER_COMBO);
    REGISTER_TYPE ("GtkCellRendererProgress", GTK_TYPE_CELL_RENDERER_PROGRESS);
    REGISTER_TYPE ("GtkCellView", GTK_TYPE_CELL_VIEW);
    REGISTER_TYPE ("GtkIconView", GTK_TYPE_ICON_VIEW);
    REGISTER_TYPE ("GtkMenuToolButton", GTK_TYPE_MENU_TOOL_BUTTON);
#endif

#if GTK_CHECK_VERSION(2,4,0)
    REGISTER_TYPE ("GtkAccelMap", GTK_TYPE_ACCEL_MAP);
    REGISTER_TYPE ("GtkAction", GTK_TYPE_ACTION);
    REGISTER_TYPE ("GtkActionGroup", GTK_TYPE_ACTION_GROUP);
    REGISTER_TYPE ("GtkCellLayout", GTK_TYPE_CELL_LAYOUT);
    REGISTER_TYPE ("GtkComboBox", GTK_TYPE_COMBO_BOX);
    REGISTER_TYPE ("GtkComboBoxEntry", GTK_TYPE_COMBO_BOX_ENTRY);
    REGISTER_TYPE ("GtkEntryCompletion", GTK_TYPE_ENTRY_COMPLETION);
    REGISTER_TYPE ("GtkExpander", GTK_TYPE_EXPANDER);
    REGISTER_TYPE ("GtkFileChooser", GTK_TYPE_FILE_CHOOSER);
    REGISTER_TYPE ("GtkFileChooserAction", GTK_TYPE_FILE_CHOOSER_ACTION);
    REGISTER_TYPE ("GtkFileChooserDialog", GTK_TYPE_FILE_CHOOSER_DIALOG);
    REGISTER_TYPE ("GtkFileChooserError", GTK_TYPE_FILE_CHOOSER_ERROR);
    REGISTER_TYPE ("GtkFileChooserWidget", GTK_TYPE_FILE_CHOOSER_WIDGET);
    REGISTER_TYPE ("GtkFileFilter", GTK_TYPE_FILE_FILTER);
    REGISTER_TYPE ("GtkFileFilterFlags", GTK_TYPE_FILE_FILTER_FLAGS);
    REGISTER_TYPE ("GtkIconInfo", GTK_TYPE_ICON_INFO);
    REGISTER_TYPE ("GtkIconLookupFlags", GTK_TYPE_ICON_LOOKUP_FLAGS);
    REGISTER_TYPE ("GtkIconTheme", GTK_TYPE_ICON_THEME);
    REGISTER_TYPE ("GtkIconThemeError", GTK_TYPE_ICON_THEME_ERROR);
    REGISTER_TYPE ("GtkRadioAction", GTK_TYPE_RADIO_ACTION);
    REGISTER_TYPE ("GtkRadioToolButton", GTK_TYPE_RADIO_TOOL_BUTTON);
    REGISTER_TYPE ("GtkScrollStep", GTK_TYPE_SCROLL_STEP);
    REGISTER_TYPE ("GtkSeparatorToolItem", GTK_TYPE_SEPARATOR_TOOL_ITEM);
    REGISTER_TYPE ("GtkToggleAction", GTK_TYPE_TOGGLE_ACTION);
    REGISTER_TYPE ("GtkToggleToolButton", GTK_TYPE_TOGGLE_TOOL_BUTTON);
    REGISTER_TYPE ("GtkToolButton", GTK_TYPE_TOOL_BUTTON);
    REGISTER_TYPE ("GtkToolItem", GTK_TYPE_TOOL_ITEM);
    REGISTER_TYPE ("GtkTreeModelFilter", GTK_TYPE_TREE_MODEL_FILTER);
    REGISTER_TYPE ("GtkUiManager", GTK_TYPE_UI_MANAGER);
    REGISTER_TYPE ("GtkUiManagerItemType", GTK_TYPE_UI_MANAGER_ITEM_TYPE);
#endif /* GTK_CHECK_VERSION(2,4,0) */
    REGISTER_TYPE ("GtkAccelFlags", GTK_TYPE_ACCEL_FLAGS);
    REGISTER_TYPE ("GtkAccelGroup", GTK_TYPE_ACCEL_GROUP);
    REGISTER_TYPE ("GtkAccelLabel", GTK_TYPE_ACCEL_LABEL);
    REGISTER_TYPE ("GtkAccessible", GTK_TYPE_ACCESSIBLE);
    REGISTER_TYPE ("GtkAdjustment", GTK_TYPE_ADJUSTMENT);
    REGISTER_TYPE ("GtkAlignment", GTK_TYPE_ALIGNMENT);
    REGISTER_TYPE ("GtkAnchorType", GTK_TYPE_ANCHOR_TYPE);
    REGISTER_TYPE ("GtkArgFlags", GTK_TYPE_ARG_FLAGS);
    REGISTER_TYPE ("GtkArrow", GTK_TYPE_ARROW);
    REGISTER_TYPE ("GtkArrowType", GTK_TYPE_ARROW_TYPE);
    REGISTER_TYPE ("GtkAspectFrame", GTK_TYPE_ASPECT_FRAME);
    REGISTER_TYPE ("GtkAttachOptions", GTK_TYPE_ATTACH_OPTIONS);
    REGISTER_TYPE ("GtkBin", GTK_TYPE_BIN);
    REGISTER_TYPE ("GtkBorder", GTK_TYPE_BORDER);
    REGISTER_TYPE ("GtkBox", GTK_TYPE_BOX);
    REGISTER_TYPE ("GtkButton", GTK_TYPE_BUTTON);
    REGISTER_TYPE ("GtkButtonAction", GTK_TYPE_BUTTON_ACTION);
    REGISTER_TYPE ("GtkButtonBox", GTK_TYPE_BUTTON_BOX);
    REGISTER_TYPE ("GtkButtonBoxStyle", GTK_TYPE_BUTTON_BOX_STYLE);
    REGISTER_TYPE ("GtkButtonsType", GTK_TYPE_BUTTONS_TYPE);
    REGISTER_TYPE ("GtkCalendar", GTK_TYPE_CALENDAR);
    REGISTER_TYPE ("GtkCalendarDisplayOptions", GTK_TYPE_CALENDAR_DISPLAY_OPTIONS);
    REGISTER_TYPE ("GtkCellEditable", GTK_TYPE_CELL_EDITABLE);
    REGISTER_TYPE ("GtkCellRenderer", GTK_TYPE_CELL_RENDERER);
    REGISTER_TYPE ("GtkCellRendererMode", GTK_TYPE_CELL_RENDERER_MODE);
    REGISTER_TYPE ("GtkCellRendererPixbuf", GTK_TYPE_CELL_RENDERER_PIXBUF);
    REGISTER_TYPE ("GtkCellRendererState", GTK_TYPE_CELL_RENDERER_STATE);
    REGISTER_TYPE ("GtkCellRendererText", GTK_TYPE_CELL_RENDERER_TEXT);
    REGISTER_TYPE ("GtkCellRendererToggle", GTK_TYPE_CELL_RENDERER_TOGGLE);
    REGISTER_TYPE ("GtkCellType", GTK_TYPE_CELL_TYPE);
    REGISTER_TYPE ("GtkCheckButton", GTK_TYPE_CHECK_BUTTON);
    REGISTER_TYPE ("GtkCheckMenuItem", GTK_TYPE_CHECK_MENU_ITEM);
    REGISTER_TYPE ("GtkClipboard", GTK_TYPE_CLIPBOARD);
    REGISTER_TYPE ("GtkColorButton", GTK_TYPE_COLOR_BUTTON);
    REGISTER_TYPE ("GtkColorSelection", GTK_TYPE_COLOR_SELECTION);
    REGISTER_TYPE ("GtkColorSelectionDialog", GTK_TYPE_COLOR_SELECTION_DIALOG);
    REGISTER_TYPE ("GtkCombo", GTK_TYPE_COMBO);
    REGISTER_TYPE ("GtkContainer", GTK_TYPE_CONTAINER);
    REGISTER_TYPE ("GtkCornerType", GTK_TYPE_CORNER_TYPE);
    REGISTER_TYPE ("GtkCurve", GTK_TYPE_CURVE);
    REGISTER_TYPE ("GtkCurveType", GTK_TYPE_CURVE_TYPE);
    REGISTER_TYPE ("GtkDebugFlag", GTK_TYPE_DEBUG_FLAG);
    REGISTER_TYPE ("GtkDeleteType", GTK_TYPE_DELETE_TYPE);
    REGISTER_TYPE ("GtkDestDefaults", GTK_TYPE_DEST_DEFAULTS);
    REGISTER_TYPE ("GtkDialog", GTK_TYPE_DIALOG);
    REGISTER_TYPE ("GtkDialogFlags", GTK_TYPE_DIALOG_FLAGS);
    REGISTER_TYPE ("GtkDirectionType", GTK_TYPE_DIRECTION_TYPE);
    REGISTER_TYPE ("GtkDrawingArea", GTK_TYPE_DRAWING_AREA);
    REGISTER_TYPE ("GtkEditable", GTK_TYPE_EDITABLE);
    REGISTER_TYPE ("GtkEntry", GTK_TYPE_ENTRY);
    REGISTER_TYPE ("GtkEventBox", GTK_TYPE_EVENT_BOX);
    REGISTER_TYPE ("GtkExpanderStyle", GTK_TYPE_EXPANDER_STYLE);
    REGISTER_TYPE ("GtkFixed", GTK_TYPE_FIXED);
    REGISTER_TYPE ("GtkFontButton", GTK_TYPE_FONT_BUTTON);
    REGISTER_TYPE ("GtkFontSelection", GTK_TYPE_FONT_SELECTION);
    REGISTER_TYPE ("GtkFontSelectionDialog", GTK_TYPE_FONT_SELECTION_DIALOG);
    REGISTER_TYPE ("GtkFrame", GTK_TYPE_FRAME);
    REGISTER_TYPE ("GtkGammaCurve", GTK_TYPE_GAMMA_CURVE);
    REGISTER_TYPE ("GtkHBox", GTK_TYPE_HBOX);
    REGISTER_TYPE ("GtkHButtonBox", GTK_TYPE_HBUTTON_BOX);
    REGISTER_TYPE ("GtkHPaned", GTK_TYPE_HPANED);
    REGISTER_TYPE ("GtkHRuler", GTK_TYPE_HRULER);
    REGISTER_TYPE ("GtkHScale", GTK_TYPE_HSCALE);
    REGISTER_TYPE ("GtkHScrollbar", GTK_TYPE_HSCROLLBAR);
    REGISTER_TYPE ("GtkHSeparator", GTK_TYPE_HSEPARATOR);
    REGISTER_TYPE ("GtkHandleBox", GTK_TYPE_HANDLE_BOX);
    REGISTER_TYPE ("GtkIconFactory", GTK_TYPE_ICON_FACTORY);
    REGISTER_TYPE ("GtkIconSet", GTK_TYPE_ICON_SET);
    REGISTER_TYPE ("GtkIconSize", GTK_TYPE_ICON_SIZE);
    REGISTER_TYPE ("GtkIconSource", GTK_TYPE_ICON_SOURCE);
    REGISTER_TYPE ("GtkIdentifier", GTK_TYPE_IDENTIFIER);
    REGISTER_TYPE ("GtkImContext", GTK_TYPE_IM_CONTEXT);
    REGISTER_TYPE ("GtkImContextSimple", GTK_TYPE_IM_CONTEXT_SIMPLE);
    REGISTER_TYPE ("GtkImMulticontext", GTK_TYPE_IM_MULTICONTEXT);
    REGISTER_TYPE ("GtkImPreeditStyle", GTK_TYPE_IM_PREEDIT_STYLE);
    REGISTER_TYPE ("GtkImStatusStyle", GTK_TYPE_IM_STATUS_STYLE);
    REGISTER_TYPE ("GtkImage", GTK_TYPE_IMAGE);
    REGISTER_TYPE ("GtkImageMenuItem", GTK_TYPE_IMAGE_MENU_ITEM);
    REGISTER_TYPE ("GtkImageType", GTK_TYPE_IMAGE_TYPE);
    REGISTER_TYPE ("GtkInputDialog", GTK_TYPE_INPUT_DIALOG);
    REGISTER_TYPE ("GtkInvisible", GTK_TYPE_INVISIBLE);
    REGISTER_TYPE ("GtkItem", GTK_TYPE_ITEM);
    REGISTER_TYPE ("GtkJustification", GTK_TYPE_JUSTIFICATION);
    REGISTER_TYPE ("GtkLabel", GTK_TYPE_LABEL);
    REGISTER_TYPE ("GtkLayout", GTK_TYPE_LAYOUT);
    REGISTER_TYPE ("GtkList", GTK_TYPE_LIST);
    REGISTER_TYPE ("GtkListItem", GTK_TYPE_LIST_ITEM);
    REGISTER_TYPE ("GtkListStore", GTK_TYPE_LIST_STORE);
    REGISTER_TYPE ("GtkMatchType", GTK_TYPE_MATCH_TYPE);
    REGISTER_TYPE ("GtkMenu", GTK_TYPE_MENU);
    REGISTER_TYPE ("GtkMenuBar", GTK_TYPE_MENU_BAR);
    REGISTER_TYPE ("GtkMenuDirectionType", GTK_TYPE_MENU_DIRECTION_TYPE);
    REGISTER_TYPE ("GtkMenuItem", GTK_TYPE_MENU_ITEM);
    REGISTER_TYPE ("GtkMenuShell", GTK_TYPE_MENU_SHELL);
    REGISTER_TYPE ("GtkMessageDialog", GTK_TYPE_MESSAGE_DIALOG);
    REGISTER_TYPE ("GtkMessageType", GTK_TYPE_MESSAGE_TYPE);
    REGISTER_TYPE ("GtkMetricType", GTK_TYPE_METRIC_TYPE);
    REGISTER_TYPE ("GtkMisc", GTK_TYPE_MISC);
    REGISTER_TYPE ("GtkMovementStep", GTK_TYPE_MOVEMENT_STEP);
    REGISTER_TYPE ("GtkNotebook", GTK_TYPE_NOTEBOOK);
    REGISTER_TYPE ("GtkNotebookTab", GTK_TYPE_NOTEBOOK_TAB);
    REGISTER_TYPE ("GtkObject", GTK_TYPE_OBJECT);
    REGISTER_TYPE ("GtkObjectFlags", GTK_TYPE_OBJECT_FLAGS);
    REGISTER_TYPE ("GtkOptionMenu", GTK_TYPE_OPTION_MENU);
    REGISTER_TYPE ("GtkOrientation", GTK_TYPE_ORIENTATION);
    REGISTER_TYPE ("GtkPackType", GTK_TYPE_PACK_TYPE);
    REGISTER_TYPE ("GtkPaned", GTK_TYPE_PANED);
    REGISTER_TYPE ("GtkPathPriorityType", GTK_TYPE_PATH_PRIORITY_TYPE);
    REGISTER_TYPE ("GtkPathType", GTK_TYPE_PATH_TYPE);
    REGISTER_TYPE ("GtkPolicyType", GTK_TYPE_POLICY_TYPE);
    REGISTER_TYPE ("GtkPositionType", GTK_TYPE_POSITION_TYPE);
    REGISTER_TYPE ("GtkPrivateFlags", GTK_TYPE_PRIVATE_FLAGS);
    REGISTER_TYPE ("GtkProgressBar", GTK_TYPE_PROGRESS_BAR);
    REGISTER_TYPE ("GtkProgressBarOrientation", GTK_TYPE_PROGRESS_BAR_ORIENTATION);
    REGISTER_TYPE ("GtkProgressBarStyle", GTK_TYPE_PROGRESS_BAR_STYLE);
    REGISTER_TYPE ("GtkRadioButton", GTK_TYPE_RADIO_BUTTON);
    REGISTER_TYPE ("GtkRadioMenuItem", GTK_TYPE_RADIO_MENU_ITEM);
    REGISTER_TYPE ("GtkRange", GTK_TYPE_RANGE);
    REGISTER_TYPE ("GtkRcFlags", GTK_TYPE_RC_FLAGS);
    REGISTER_TYPE ("GtkRcStyle", GTK_TYPE_RC_STYLE);
    REGISTER_TYPE ("GtkRcTokenType", GTK_TYPE_RC_TOKEN_TYPE);
    REGISTER_TYPE ("GtkReliefStyle", GTK_TYPE_RELIEF_STYLE);
    REGISTER_TYPE ("GtkRequisition", GTK_TYPE_REQUISITION);
    REGISTER_TYPE ("GtkResizeMode", GTK_TYPE_RESIZE_MODE);
    REGISTER_TYPE ("GtkResponseType", GTK_TYPE_RESPONSE_TYPE);
    REGISTER_TYPE ("GtkRuler", GTK_TYPE_RULER);
    REGISTER_TYPE ("GtkScale", GTK_TYPE_SCALE);
    REGISTER_TYPE ("GtkScrollType", GTK_TYPE_SCROLL_TYPE);
    REGISTER_TYPE ("GtkScrollbar", GTK_TYPE_SCROLLBAR);
    REGISTER_TYPE ("GtkScrolledWindow", GTK_TYPE_SCROLLED_WINDOW);
    REGISTER_TYPE ("GtkSelectionData", GTK_TYPE_SELECTION_DATA);
    REGISTER_TYPE ("GtkSelectionMode", GTK_TYPE_SELECTION_MODE);
    REGISTER_TYPE ("GtkSeparator", GTK_TYPE_SEPARATOR);
    REGISTER_TYPE ("GtkSeparatorMenuItem", GTK_TYPE_SEPARATOR_MENU_ITEM);
    REGISTER_TYPE ("GtkSettings", GTK_TYPE_SETTINGS);
    REGISTER_TYPE ("GtkShadowType", GTK_TYPE_SHADOW_TYPE);
    REGISTER_TYPE ("GtkSideType", GTK_TYPE_SIDE_TYPE);
    REGISTER_TYPE ("GtkSignalRunType", GTK_TYPE_SIGNAL_RUN_TYPE);
    REGISTER_TYPE ("GtkSizeGroup", GTK_TYPE_SIZE_GROUP);
    REGISTER_TYPE ("GtkSizeGroupMode", GTK_TYPE_SIZE_GROUP_MODE);
    REGISTER_TYPE ("GtkSortType", GTK_TYPE_SORT_TYPE);
    REGISTER_TYPE ("GtkSpinButton", GTK_TYPE_SPIN_BUTTON);
    REGISTER_TYPE ("GtkSpinButtonUpdatePolicy", GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY);
    REGISTER_TYPE ("GtkSpinType", GTK_TYPE_SPIN_TYPE);
    REGISTER_TYPE ("GtkStateType", GTK_TYPE_STATE_TYPE);
    REGISTER_TYPE ("GtkStatusbar", GTK_TYPE_STATUSBAR);
    REGISTER_TYPE ("GtkStyle", GTK_TYPE_STYLE);
    REGISTER_TYPE ("GtkSubmenuDirection", GTK_TYPE_SUBMENU_DIRECTION);
    REGISTER_TYPE ("GtkSubmenuPlacement", GTK_TYPE_SUBMENU_PLACEMENT);
    REGISTER_TYPE ("GtkTable", GTK_TYPE_TABLE);
    REGISTER_TYPE ("GtkTargetFlags", GTK_TYPE_TARGET_FLAGS);
    REGISTER_TYPE ("GtkTearoffMenuItem", GTK_TYPE_TEAROFF_MENU_ITEM);
    REGISTER_TYPE ("GtkTextAttributes", GTK_TYPE_TEXT_ATTRIBUTES);
    REGISTER_TYPE ("GtkTextBuffer", GTK_TYPE_TEXT_BUFFER);
    REGISTER_TYPE ("GtkTextChildAnchor", GTK_TYPE_TEXT_CHILD_ANCHOR);
    REGISTER_TYPE ("GtkTextDirection", GTK_TYPE_TEXT_DIRECTION);
    REGISTER_TYPE ("GtkTextIter", GTK_TYPE_TEXT_ITER);
    REGISTER_TYPE ("GtkTextMark", GTK_TYPE_TEXT_MARK);
    REGISTER_TYPE ("GtkTextSearchFlags", GTK_TYPE_TEXT_SEARCH_FLAGS);
    REGISTER_TYPE ("GtkTextTag", GTK_TYPE_TEXT_TAG);
    REGISTER_TYPE ("GtkTextTagTable", GTK_TYPE_TEXT_TAG_TABLE);
    REGISTER_TYPE ("GtkTextView", GTK_TYPE_TEXT_VIEW);
    REGISTER_TYPE ("GtkTextWindowType", GTK_TYPE_TEXT_WINDOW_TYPE);
    REGISTER_TYPE ("GtkToggleButton", GTK_TYPE_TOGGLE_BUTTON);
    REGISTER_TYPE ("GtkToolbar", GTK_TYPE_TOOLBAR);
    REGISTER_TYPE ("GtkToolbarChildType", GTK_TYPE_TOOLBAR_CHILD_TYPE);
    REGISTER_TYPE ("GtkToolbarSpaceStyle", GTK_TYPE_TOOLBAR_SPACE_STYLE);
    REGISTER_TYPE ("GtkToolbarStyle", GTK_TYPE_TOOLBAR_STYLE);
    REGISTER_TYPE ("GtkTooltips", GTK_TYPE_TOOLTIPS);
    REGISTER_TYPE ("GtkTreeDragDest", GTK_TYPE_TREE_DRAG_DEST);
    REGISTER_TYPE ("GtkTreeDragSource", GTK_TYPE_TREE_DRAG_SOURCE);
    REGISTER_TYPE ("GtkTreeIter", GTK_TYPE_TREE_ITER);
    REGISTER_TYPE ("GtkTreeModel", GTK_TYPE_TREE_MODEL);
    REGISTER_TYPE ("GtkTreeModelFlags", GTK_TYPE_TREE_MODEL_FLAGS);
    REGISTER_TYPE ("GtkTreeModelSort", GTK_TYPE_TREE_MODEL_SORT);
    REGISTER_TYPE ("GtkTreePath", GTK_TYPE_TREE_PATH);
    REGISTER_TYPE ("GtkTreeRowReference", GTK_TYPE_TREE_ROW_REFERENCE);
    REGISTER_TYPE ("GtkTreeSelection", GTK_TYPE_TREE_SELECTION);
    REGISTER_TYPE ("GtkTreeSortable", GTK_TYPE_TREE_SORTABLE);
    REGISTER_TYPE ("GtkTreeStore", GTK_TYPE_TREE_STORE);
    REGISTER_TYPE ("GtkTreeView", GTK_TYPE_TREE_VIEW);
    REGISTER_TYPE ("GtkTreeViewColumn", GTK_TYPE_TREE_VIEW_COLUMN);
    REGISTER_TYPE ("GtkTreeViewColumnSizing", GTK_TYPE_TREE_VIEW_COLUMN_SIZING);
    REGISTER_TYPE ("GtkTreeViewDropPosition", GTK_TYPE_TREE_VIEW_DROP_POSITION);
    REGISTER_TYPE ("GtkTreeViewMode", GTK_TYPE_TREE_VIEW_MODE);
    REGISTER_TYPE ("GtkUpdateType", GTK_TYPE_UPDATE_TYPE);
    REGISTER_TYPE ("GtkVBox", GTK_TYPE_VBOX);
    REGISTER_TYPE ("GtkVButtonBox", GTK_TYPE_VBUTTON_BOX);
    REGISTER_TYPE ("GtkViewport", GTK_TYPE_VIEWPORT);
    REGISTER_TYPE ("GtkVisibility", GTK_TYPE_VISIBILITY);
    REGISTER_TYPE ("GtkVPaned", GTK_TYPE_VPANED);
    REGISTER_TYPE ("GtkVRuler", GTK_TYPE_VRULER);
    REGISTER_TYPE ("GtkVScale", GTK_TYPE_VSCALE);
    REGISTER_TYPE ("GtkVScrollbar", GTK_TYPE_VSCROLLBAR);
    REGISTER_TYPE ("GtkVSeparator", GTK_TYPE_VSEPARATOR);
    REGISTER_TYPE ("GtkWidget", GTK_TYPE_WIDGET);
    REGISTER_TYPE ("GtkWidgetFlags", GTK_TYPE_WIDGET_FLAGS);
    REGISTER_TYPE ("GtkWidgetHelpType", GTK_TYPE_WIDGET_HELP_TYPE);
    REGISTER_TYPE ("GtkWindow", GTK_TYPE_WINDOW);
    REGISTER_TYPE ("GtkWindowGroup", GTK_TYPE_WINDOW_GROUP);
    REGISTER_TYPE ("GtkWindowPosition", GTK_TYPE_WINDOW_POSITION);
    REGISTER_TYPE ("GtkWindowType", GTK_TYPE_WINDOW_TYPE);
    REGISTER_TYPE ("GtkWrapMode", GTK_TYPE_WRAP_MODE);

#ifndef __WIN32__
    REGISTER_TYPE ("GtkPlug", GTK_TYPE_PLUG);
    REGISTER_TYPE ("GtkSocket", GTK_TYPE_SOCKET);
#endif

    return 0;
}


#if 0
static void print_widget    (Widget *widget, guint offset);
static void print_child     (Child *child, guint offset);
static void print_props     (GParameter *params, guint n_params, guint offset);

static void
print_widget (Widget *widget, guint offset)
{
    GSList *l;
    char *fill = g_strnfill (offset, ' ');
    g_print ("%sWidget id '%s' class '%s'\n",
             fill, widget->id, g_type_name (widget->type));
    print_props (widget->props->params, widget->props->n_params, offset);
    for (l = widget->children; l != NULL; l = l->next)
        print_child (l->data, offset + 3);
}


static void
print_child (Child *child, guint offset)
{
    char *fill = g_strnfill (offset, ' ');
    g_print ("%sChild\n", fill);
    print_props (child->props->params, child->props->n_params, offset);
    print_widget (child->widget, offset + 3);
}


static void
print_value (GValue *value)
{
    if (value->g_type == G_TYPE_CHAR)
    {
        g_print ("%c", g_value_get_char (value));
    }
    else if (value->g_type == G_TYPE_UCHAR)
    {
        g_print ("%c", g_value_get_uchar (value));
    }
    else if (value->g_type == G_TYPE_BOOLEAN)
    {
        g_print ("%s",
                 g_value_get_boolean (value) ? "TRUE" : "FALSE");
    }
    else if (value->g_type == G_TYPE_INT ||
             value->g_type == G_TYPE_LONG ||
             value->g_type == G_TYPE_INT64 ||
             value->g_type == G_TYPE_UINT ||
             value->g_type == G_TYPE_ULONG ||
             value->g_type == G_TYPE_UINT64) /* XXX */
    {
        int val;

        if (value->g_type == G_TYPE_INT)
            val = g_value_get_int (value);
        else if (value->g_type == G_TYPE_LONG)
            val = g_value_get_long (value);
        else if (value->g_type == G_TYPE_INT64)
            val = g_value_get_int64 (value);
        else if (value->g_type == G_TYPE_UINT)
            val = g_value_get_uint (value);
        else if (value->g_type == G_TYPE_ULONG)
            val = g_value_get_ulong (value);
        else if (value->g_type == G_TYPE_UINT64)
            val = g_value_get_uint64 (value);

        g_print ("%d", val);
    }
    else if (value->g_type == G_TYPE_FLOAT ||
             value->g_type == G_TYPE_DOUBLE) /* XXX */
    {
        double val;

        if (value->g_type == G_TYPE_FLOAT)
            val = g_value_get_float (value);
        else if (value->g_type == G_TYPE_DOUBLE)
            val = g_value_get_double (value);

        g_print ("%f", val);
    }
    else if (value->g_type == G_TYPE_STRING)
    {
        g_print ("%s", g_value_get_string (value));
    }
    else if (G_TYPE_IS_ENUM (value->g_type))
    {
        g_print ("%d", g_value_get_enum (value));
    }
    else if (G_TYPE_IS_FLAGS (value->g_type))
    {
        g_print ("%d", g_value_get_flags (value));
    }
    else
    {
        g_print ("<%s>", g_type_name (value->g_type));
    }
}


static void
print_props (GParameter *params, guint n_params, guint offset)
{
    guint i;
    char *fill = g_strnfill (offset, ' ');
    for (i = 0; i < n_params; ++i)
    {
        GParameter *param = &params[i];
        g_print ("%s%s = '", fill, param->name);
        print_value (&param->value);
        g_print ("'\n");
    }
}


static void
dump_widget (Widget *widget)
{
    print_widget (widget, 0);
}

#else
static void
dump_widget (G_GNUC_UNUSED Widget *widget)
{
}
#endif
