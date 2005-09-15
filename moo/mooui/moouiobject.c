/*
 *   mooui/moouiobject.c
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

#include "mooui/moouiobject-impl.h"
#include "mooui/mooaccel.h"
#include "mooui/mootoggleaction.h"
#include "mooui/moomenuaction.h"
#include "mooutils/mooprefs.h"
#include <gobject/gvaluecollector.h>
#include <string.h>


static const GQuark *get_quark (void) G_GNUC_CONST;
#define MOO_UI_OBJECT_NAME_QUARK    (get_quark()[0])
#define MOO_UI_OBJECT_ID_QUARK      (get_quark()[1])
#define MOO_UI_OBJECT_ACTIONS_QUARK (get_quark()[2])
#define MOO_UI_OBJECT_UI_XML_QUARK  (get_quark()[3])


static void     moo_ui_object_iface_init        (gpointer        g_iface);
static void     moo_ui_object_set_id            (MooUIObject    *object,
                                                 const char     *id);
static void     moo_ui_object_add_class_actions (MooUIObject    *object);


GType moo_ui_object_get_type (void)
{
    static GType type = 0;

    if (!type)
    {
        static const GTypeInfo info =
        {
            sizeof (MooUIObjectIface),      /* class_size */
            moo_ui_object_iface_init,       /* base_init */
            NULL,                           /* base_finalize */
            NULL,                           /* class_init */
            0, 0, 0, 0, 0, 0
        };

        type = g_type_register_static (G_TYPE_INTERFACE,
                                       "MooUIObject",
                                       &info, (GTypeFlags)0);
    }

    return type;
}


static void    moo_ui_object_iface_init    (G_GNUC_UNUSED gpointer g_iface)
{
    static gboolean done = FALSE;
    if (done) return;
    done = TRUE;

#if GLIB_CHECK_VERSION(2,4,0)
    g_object_interface_install_property (g_iface,
                                         g_param_spec_string ("ui-object-name",
                                                 "ui-object-name",
                                                 "ui-object-name",
                                                 NULL,
                                                 G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_interface_install_property (g_iface,
                                         g_param_spec_string ("ui-object-id",
                                                 "ui-object-id",
                                                 "ui-object-id",
                                                 NULL,
                                                 G_PARAM_READABLE));

    g_object_interface_install_property (g_iface,
                                         g_param_spec_object ("ui-object-actions",
                                                 "ui-object-actions",
                                                 "ui-object-actions",
                                                 MOO_TYPE_ACTION_GROUP,
                                                 G_PARAM_READABLE));

    g_object_interface_install_property (g_iface,
                                         g_param_spec_object ("ui-object-xml",
                                                 "ui-object-xml",
                                                 "ui-object-xml",
                                                 MOO_TYPE_UI_XML,
                                                 G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
#endif /* GLIB_CHECK_VERSION(2,4,0) */
}


const char      *_moo_ui_object_get_name_impl   (MooUIObject        *object)
{
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    return g_object_get_qdata (G_OBJECT (object), MOO_UI_OBJECT_NAME_QUARK);
}


const char      *_moo_ui_object_get_id_impl     (MooUIObject        *object)
{
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    return g_object_get_qdata (G_OBJECT (object), MOO_UI_OBJECT_ID_QUARK);
}


void             _moo_ui_object_set_name_impl   (MooUIObject        *object,
                                                 const char         *name)
{
    g_return_if_fail (MOO_IS_UI_OBJECT (object) && name != NULL);
    g_object_set_qdata_full (G_OBJECT (object),
                             MOO_UI_OBJECT_NAME_QUARK,
                             g_strdup (name),
                             (GDestroyNotify) g_free);
    moo_action_group_set_name (moo_ui_object_get_actions (object), name);
    g_object_notify (G_OBJECT (object), "ui-object-name");
}


static void     moo_ui_object_set_id            (MooUIObject    *object,
                                                 const char     *id)
{
    g_return_if_fail (MOO_IS_UI_OBJECT (object) && id != NULL);
    g_object_set_qdata_full (G_OBJECT (object),
                             MOO_UI_OBJECT_ID_QUARK,
                             g_strdup (id),
                             (GDestroyNotify) g_free);
}


MooActionGroup  *_moo_ui_object_get_actions_impl(MooUIObject        *object)
{
    MooActionGroup *group;

    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);

    group = g_object_get_qdata (G_OBJECT (object), MOO_UI_OBJECT_ACTIONS_QUARK);

    if (!group)
    {
        group = moo_action_group_new (moo_ui_object_get_name (object));
        g_object_set_qdata_full (G_OBJECT (object),
                                 MOO_UI_OBJECT_ACTIONS_QUARK,
                                 group,
                                 (GDestroyNotify) g_object_unref);
        g_object_notify (G_OBJECT (object), "ui-object-actions");
    }

    return group;
}


MooUIXML        *_moo_ui_object_get_ui_xml_impl (MooUIObject        *object)
{
    MooUIXML *xml;

    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);

    xml = g_object_get_qdata (G_OBJECT (object), MOO_UI_OBJECT_UI_XML_QUARK);

    if (!xml)
    {
        xml = moo_ui_xml_new ();
        g_object_set_qdata_full (G_OBJECT (object),
                                 MOO_UI_OBJECT_UI_XML_QUARK,
                                 xml, g_object_unref);
        g_object_notify (G_OBJECT (object), "ui-object-xml");
    }

    return xml;
}


void             _moo_ui_object_set_ui_xml_impl (MooUIObject        *object,
                                                 MooUIXML           *xml)
{
    MooUIXML *old;

    g_return_if_fail (MOO_IS_UI_OBJECT (object));
    g_return_if_fail (!xml || MOO_IS_UI_XML (xml));

    old = g_object_get_qdata (G_OBJECT (object), MOO_UI_OBJECT_UI_XML_QUARK);

    if (old == xml)
        return;

    if (xml)
        g_object_set_qdata_full (G_OBJECT (object),
                                 MOO_UI_OBJECT_UI_XML_QUARK,
                                 g_object_ref (xml),
                                 g_object_unref);
    else
        g_object_set_qdata (G_OBJECT (object),
                            MOO_UI_OBJECT_UI_XML_QUARK,
                            NULL);

    g_object_notify (G_OBJECT (object), "ui-object-xml");
}


/****************************************************************************/

typedef struct {
    MooObjectFactory *action;
    MooObjectFactory *closure;
} FactoryData;


static MooAction *create_action (FactoryData *data,
                                 MooUIObject *object)
{
    MooClosure *closure = NULL;
    MooAction *action;
    const char *class_id;

    g_return_val_if_fail (data != NULL && data->action != NULL, NULL);

    class_id = moo_ui_object_class_get_id (G_OBJECT_GET_CLASS (object));
    action = MOO_ACTION (moo_object_factory_create_object (data->action, object,
                         "group-id", class_id,
                         NULL));
    g_return_val_if_fail (action != NULL, NULL);

    if (g_type_is_a (data->action->object_type, MOO_TYPE_TOGGLE_ACTION)) {
        g_object_set (action, "toggled-data", object, NULL);
    }

    if (g_type_is_a (data->action->object_type, MOO_TYPE_MENU_ACTION)) {
        g_object_set (action,
                      "create-menu-data", object,
                      "no-accel", TRUE,
                      NULL);
    }

    if (data->closure) {
        closure = MOO_CLOSURE (moo_object_factory_create_object (data->closure, NULL, NULL));
        g_return_val_if_fail (closure != NULL, action);
        g_object_set (closure, "data", object, NULL);
        g_object_set (action, "closure", closure, NULL);
    }

    return action;
}


void        moo_ui_object_class_init        (GObjectClass   *klass,
                                             const char     *id,
                                             const char     *name)
{
    GType type;

    g_return_if_fail (G_IS_OBJECT_CLASS (klass));
    g_return_if_fail (id != NULL && name != NULL);

    type = G_OBJECT_CLASS_TYPE (klass);
    g_return_if_fail (g_type_get_qdata (type, MOO_UI_OBJECT_ID_QUARK) == NULL);
    g_type_set_qdata (type, MOO_UI_OBJECT_ID_QUARK, g_strdup (id));
    g_return_if_fail (g_type_get_qdata (type, MOO_UI_OBJECT_NAME_QUARK) == NULL);
    g_type_set_qdata (type, MOO_UI_OBJECT_NAME_QUARK, g_strdup (name));
}


const char      *moo_ui_object_class_get_id     (GObjectClass       *klass)
{
    GType type;
    g_return_val_if_fail (G_IS_OBJECT_CLASS (klass), NULL);
    type = G_OBJECT_CLASS_TYPE (klass);
    return g_type_get_qdata (type, MOO_UI_OBJECT_ID_QUARK);
}


const char      *moo_ui_object_class_get_name   (GObjectClass       *klass)
{
    GType type;
    g_return_val_if_fail (G_IS_OBJECT_CLASS (klass), NULL);
    type = G_OBJECT_CLASS_TYPE (klass);
    return g_type_get_qdata (type, MOO_UI_OBJECT_NAME_QUARK);
}


/* XXX check existing instances */
void             moo_ui_object_class_install_action
                                                (GObjectClass       *klass,
                                                 MooObjectFactory   *action,
                                                 MooObjectFactory   *closure)
{
    GArray *array;
    FactoryData d = {action, closure};
    GType type;

    g_return_if_fail (G_IS_OBJECT_CLASS (klass));
    g_return_if_fail (MOO_IS_OBJECT_FACTORY (action));
    g_return_if_fail (closure == NULL || MOO_IS_OBJECT_FACTORY (closure));

    type = G_OBJECT_CLASS_TYPE (klass);
    array = g_type_get_qdata (type, MOO_UI_OBJECT_ACTIONS_QUARK);
    if (!array) {
        array = g_array_new (FALSE, FALSE, sizeof (FactoryData));
        g_type_set_qdata (type, MOO_UI_OBJECT_ACTIONS_QUARK, array);
    }
    g_array_append_val (array, d);
}


void        moo_ui_object_init              (MooUIObject    *object)
{
    GType type;
    GObjectClass *klass;

    type = G_OBJECT_TYPE (object);
    klass = g_type_class_ref (type);

    g_return_if_fail (klass != NULL);

    moo_ui_object_set_id (object,
                          moo_ui_object_class_get_id (klass));
    moo_ui_object_set_name (object,
                            moo_ui_object_class_get_name (klass));

    moo_ui_object_add_class_actions (object);
}


static void     moo_ui_object_add_class_actions (MooUIObject    *object)
{
    GType type;

    g_return_if_fail (G_IS_OBJECT (object));

    type = G_OBJECT_TYPE (object);
    do
    {
        GArray *array;
        guint i;

        array = g_type_get_qdata (type, MOO_UI_OBJECT_ACTIONS_QUARK);
        if (array)
            for (i = 0; i < array->len; ++i)
        {
            FactoryData *d = &g_array_index (array, FactoryData, i);
            MooAction *action = create_action (d, object);
            if (action)
                moo_ui_object_add_action (object, action);
        }

        type = g_type_parent (type);
        g_return_if_fail (type != 0);
    }
    while (type != G_TYPE_OBJECT);
}


void             moo_ui_object_add_action       (MooUIObject    *object,
                                                 MooAction      *action)
{
    MooActionGroup *group;

    g_return_if_fail (MOO_IS_UI_OBJECT (object));
    g_return_if_fail (MOO_IS_ACTION (action));

    group = moo_ui_object_get_actions (object);
    moo_action_group_add_action (group, action);

    if (!action->dead)
    {
        const char *accel;
        accel = moo_prefs_get_accel (moo_action_get_accel_path (action));
        if (accel) moo_action_set_accel (action, accel);
    }
}


void             moo_ui_object_class_new_action (GObjectClass   *klass,
                                                 const char     *first_prop_name,
                                                 ...)
{
    va_list args;
    va_start (args, first_prop_name);
    moo_ui_object_class_new_actionv (klass, first_prop_name, args);
    va_end (args);
}


void             moo_ui_object_class_new_actionv(GObjectClass   *object_class,
                                                 const char     *first_prop_name,
                                                 va_list         var_args)
{
    const char *name;
    GType action_type = 0, closure_type = 0;
    GObjectClass *action_class = NULL;
    GObjectClass *closure_class = NULL;
    GArray *action_params = NULL;
    GArray *closure_params = NULL;

    g_return_if_fail (G_IS_OBJECT_CLASS (object_class));
    g_return_if_fail (first_prop_name != NULL);

    action_params = g_array_new (FALSE, TRUE, sizeof (GParameter));
    closure_params = g_array_new (FALSE, TRUE, sizeof (GParameter));

    name = first_prop_name;
    while (name)
    {
        GParameter param = {0};
        GParamSpec *pspec;
        char *err = NULL;

        if (strstr (name, "::") && strncmp (name, "closure::", strlen ("closure::")))
        {
            if (!strcmp (name, "action-type::") || !strcmp (name, "action_type::"))
            {
                g_value_init (&param.value, G_TYPE_POINTER);
                G_VALUE_COLLECT (&param.value, var_args, 0, &err);
                if (err) {
                    g_warning ("%s: %s", G_STRLOC, err);
                    g_free (err);
                    goto error;
                }
                action_type = (GType) param.value.data[0].v_pointer;
                if (!g_type_is_a (action_type, MOO_TYPE_ACTION)) {
                    g_warning ("%s: invalid action type", G_STRLOC);
                    goto error;
                }
                action_class = g_type_class_ref (action_type);
            }
            else if (!strcmp (name, "closure-type::") || !strcmp (name, "closure_type::"))
            {
                g_value_init (&param.value, G_TYPE_POINTER);
                G_VALUE_COLLECT (&param.value, var_args, 0, &err);
                if (err) {
                    g_warning ("%s: %s", G_STRLOC, err);
                    g_free (err);
                    goto error;
                }
                closure_type = (GType) param.value.data[0].v_pointer;
                if (!g_type_is_a (closure_type, MOO_TYPE_CLOSURE)) {
                    g_warning ("%s: invalid closure type", G_STRLOC);
                    goto error;
                }
                closure_class = g_type_class_ref (closure_type);
            }
            else
            {
                g_warning ("%s: invalid argument %s", G_STRLOC, name);
                goto error;
            }
        }
        else if (!strstr (name, "::"))
        {   /* action property */
            if (!action_class) {
                if (!action_type)
                    action_type = MOO_TYPE_ACTION;
                action_class = g_type_class_ref (action_type);
            }
            pspec = g_object_class_find_property (action_class, name);
            if (!pspec) {
                g_warning ("%s: object class `%s' has no property named `%s'",
                           G_STRLOC, g_type_name (action_type), name);
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
            g_array_append_val (action_params, param);
        }
        else
        {   /* closure property */
            const char *suffix = strstr (name, "::");
            if (!suffix || !suffix[1] || !suffix[2]) {
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
            if (!pspec) {
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

        name = va_arg (var_args, gchar*);
    }

    {
        MooObjectFactory *action_factory = NULL;
        MooObjectFactory *closure_factory = NULL;

        if (closure_params->len) {
            closure_factory =
                    moo_object_factory_new_a (closure_type,
                                              (GParameter*) closure_params->data,
                                              closure_params->len);
            if (!closure_factory) {
                g_warning ("%s: error in moo_object_factory_new_a()", G_STRLOC);
                goto error;
            }
            g_array_free (closure_params, FALSE);
            closure_params = NULL;
        }
        else {
            g_array_free (closure_params, TRUE);
            closure_params = NULL;
        }

        action_factory = moo_object_factory_new_a (action_type,
                                                   (GParameter*) action_params->data,
                                                   action_params->len);
        if (!action_factory) {
            g_warning ("%s: error in moo_object_factory_new_a()", G_STRLOC);
            g_object_unref (closure_factory);
            goto error;
        }
        g_array_free (action_params, FALSE);
        action_params = NULL;

        moo_ui_object_class_install_action (object_class,
                                            action_factory,
                                            closure_factory);

        if (action_class)
            g_type_class_unref (action_class);
        if (closure_class)
            g_type_class_unref (closure_class);
        return;
    }

error:
    if (action_params) {
        guint i;
        GParameter *params = (GParameter*) action_params->data;
        for (i = 0; i < action_params->len; ++i) {
            g_value_unset (&params[i].value);
            g_free ((char*) params[i].name);
        }
        g_array_free (action_params, TRUE);
    }
    if (closure_params) {
        guint i;
        GParameter *params = (GParameter*) closure_params->data;
        for (i = 0; i < closure_params->len; ++i) {
            g_value_unset (&params[i].value);
            g_free ((char*) params[i].name);
        }
        g_array_free (closure_params, TRUE);
    }
    if (action_class)
        g_type_class_unref (action_class);
    if (closure_class)
        g_type_class_unref (closure_class);
}


static const GQuark *get_quark (void)
{
    static GQuark q[4] = {0, 0, 0, 0};
    if (!q[0]) {
        q[0] = g_quark_from_static_string ("moo_ui_object_actions");
        q[1] = g_quark_from_static_string ("moo_ui_object_ui_xml");
        q[2] = g_quark_from_static_string ("moo_ui_object_id");
        q[3] = g_quark_from_static_string ("moo_ui_object_name");
    }
    return q;
}


MooUIXML        *moo_ui_object_get_ui_xml       (MooUIObject        *object)
{
    MooUIXML *xml;
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    g_object_get (G_OBJECT (object), "ui-object-xml", &xml, NULL);
    if (xml) g_object_unref (xml);
    return xml;
}


void             moo_ui_object_set_ui_xml       (MooUIObject        *object,
                                                 MooUIXML           *xml)
{
    g_return_if_fail (MOO_IS_UI_OBJECT (object));
    g_return_if_fail (!xml || MOO_IS_UI_XML (xml));
    g_object_set (G_OBJECT (object), "ui-object-xml", xml, NULL);
}


MooActionGroup  *moo_ui_object_get_actions      (MooUIObject        *object)
{
    MooActionGroup *actions;
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    g_object_get (G_OBJECT (object), "ui-object-actions", &actions, NULL);
    if (actions) g_object_unref (actions);
    return actions;
}


const char      *moo_ui_object_get_name         (MooUIObject        *object)
{
    static char *name = NULL;
    g_free (name);
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    g_object_get (G_OBJECT (object), "ui-object-name", &name, NULL);
    return name;
}


const char      *moo_ui_object_get_id           (MooUIObject        *object)
{
    static char *id = NULL;
    g_free (id);
    g_return_val_if_fail (MOO_IS_UI_OBJECT (object), NULL);
    g_object_get (G_OBJECT (object), "ui-object-id", &id, NULL);
    return id;
}


void             moo_ui_object_set_name         (MooUIObject        *object,
                                                 const char         *name)
{
    g_return_if_fail (MOO_IS_UI_OBJECT (object));
    g_object_set (G_OBJECT (object), "ui-object-name", name, NULL);
}
