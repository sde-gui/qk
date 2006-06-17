/*
 *   mooactionfactory.c
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

#include "mooutils/mooactionfactory.h"
#include "mooutils/mooaction.h"
#include "mooutils/mooclosure.h"
#include "mooutils/mooutils-gobject.h"
#include <gtk/gtktoggleaction.h>
#include <gobject/gvaluecollector.h>
#include <string.h>


G_DEFINE_TYPE(MooActionFactory, moo_action_factory, G_TYPE_OBJECT)


enum {
    PROP_DISPLAY_NAME,
    PROP_ACCEL,
    PROP_NO_ACCEL,
    PROP_FORCE_ACCEL_LABEL,
    PROP_DEAD,
    PROP_HAS_SUBMENU,
    PROP_CLOSURE,
    PROP_CLOSURE_OBJECT,
    PROP_CLOSURE_SIGNAL,
    PROP_CLOSURE_CALLBACK,
    PROP_CLOSURE_PROXY_FUNC,
    PROP_TOGGLED_CALLBACK,
    PROP_TOGGLED_OBJECT,
    PROP_TOGGLED_DATA,
    PROP_LAST
};

static GParamSpec *pspecs[PROP_LAST];


static void
moo_action_factory_finalize (GObject *object)
{
    MooActionFactory *factory = MOO_ACTION_FACTORY (object);

    if (factory->real_props)
        moo_param_array_free (factory->real_props, factory->n_real_props);
    if (factory->fake_props)
        moo_param_array_free (factory->fake_props, factory->n_fake_props);

    G_OBJECT_CLASS(moo_action_factory_parent_class)->finalize (object);
}


static void
moo_action_factory_class_init (MooActionFactoryClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = moo_action_factory_finalize;
}


static void
moo_action_factory_init (MooActionFactory *factory)
{
    factory->action_type = GTK_TYPE_ACTION;
}


static void
action_init_props (void)
{
    if (pspecs[0])
        return;

    pspecs[PROP_DISPLAY_NAME] = g_param_spec_string ("display-name", "display-name", "display-name", NULL, G_PARAM_READWRITE);
    pspecs[PROP_ACCEL] = g_param_spec_string ("accel", "accel", "accel", NULL, G_PARAM_READWRITE);
    pspecs[PROP_NO_ACCEL] = g_param_spec_boolean ("no-accel", "no-accel", "no-accel", FALSE, G_PARAM_READWRITE);
    pspecs[PROP_FORCE_ACCEL_LABEL] = g_param_spec_boolean ("force-accel-label", "force-accel-label", "force-accel-label", FALSE, G_PARAM_READWRITE);
    pspecs[PROP_DEAD] = g_param_spec_boolean ("dead", "dead", "dead", FALSE, G_PARAM_READWRITE);
    pspecs[PROP_HAS_SUBMENU] = g_param_spec_boolean ("has-submenu", "has-submenu", "has-submenu", FALSE, G_PARAM_READWRITE);

    pspecs[PROP_CLOSURE] = g_param_spec_boxed ("closure", "closure", "closure", MOO_TYPE_CLOSURE, G_PARAM_READWRITE);
    pspecs[PROP_CLOSURE_OBJECT] = g_param_spec_object ("closure-object", "closure-object", "closure-object", G_TYPE_OBJECT, G_PARAM_WRITABLE);
    pspecs[PROP_CLOSURE_SIGNAL] = g_param_spec_string ("closure-signal", "closure-signal", "closure-signal", NULL, G_PARAM_WRITABLE);
    pspecs[PROP_CLOSURE_CALLBACK] = g_param_spec_pointer ("closure-callback", "closure-callback", "closure-callback", G_PARAM_WRITABLE);
    pspecs[PROP_CLOSURE_PROXY_FUNC] = g_param_spec_pointer ("closure-proxy-func", "closure-proxy-func", "closure-proxy-func", G_PARAM_WRITABLE);

    pspecs[PROP_TOGGLED_CALLBACK] = g_param_spec_pointer ("toggled-callback", "toggled-callback", "toggled-callback", G_PARAM_WRITABLE);
    pspecs[PROP_TOGGLED_OBJECT] = g_param_spec_object ("toggled-object", "toggled-object", "toggled-object", G_TYPE_OBJECT, G_PARAM_WRITABLE);
    pspecs[PROP_TOGGLED_DATA] = g_param_spec_pointer ("toggled-data", "toggled-data", "toggled-data", G_PARAM_WRITABLE);
}


GtkAction *
moo_action_group_add_action (GtkActionGroup *group,
                             const char     *action_id,
                             const char     *first_prop_name,
                             ...)
{
    GtkAction *action;
    GType action_type = GTK_TYPE_ACTION;
    va_list var_args;

    g_return_val_if_fail (GTK_IS_ACTION_GROUP (group), NULL);

    va_start (var_args, first_prop_name);

    if (first_prop_name && (!strcmp (first_prop_name, "action-type::") ||
        !strcmp (first_prop_name, "action-type::")))
    {
        action_type = va_arg (var_args, GType);
        g_return_val_if_fail (g_type_is_a (action_type, GTK_TYPE_ACTION), NULL);
        first_prop_name = va_arg (var_args, char*);
    }

    action = moo_action_new_valist (action_type, action_id, first_prop_name, var_args);

    va_end (var_args);

    g_return_val_if_fail (action != NULL, NULL);

    gtk_action_group_add_action (group, action);
    g_object_unref (action);

    return action;
}


static gboolean
collect_valist (GType        type,
                GParameter **real_props_p,
                guint       *n_real_props_p,
                GParameter **fake_props_p,
                guint       *n_fake_props_p,
                const char  *first_prop_name,
                va_list      var_args)
{
    GObjectClass *klass;
    GArray *real_props, *fake_props;
    const char *prop_name;

    g_return_val_if_fail (first_prop_name != NULL, FALSE);

    klass = g_type_class_ref (type);
    g_return_val_if_fail (klass != NULL, FALSE);

    real_props = g_array_new (FALSE, TRUE, sizeof (GParameter));
    fake_props = g_array_new (FALSE, TRUE, sizeof (GParameter));
    prop_name = first_prop_name;

    while (prop_name)
    {
        char *error = NULL;
        GParameter param;
        GParamSpec *pspec;
        GArray *add_to = NULL;

        if ((pspec = _moo_action_find_fake_property (klass, prop_name)))
        {
            add_to = fake_props;
        }
        else if ((pspec = g_object_class_find_property (klass, prop_name)))
        {
            add_to = real_props;
        }
        else
        {
            g_warning ("%s: could not find property '%s' for class '%s'",
                       G_STRLOC, prop_name, g_type_name (type));

            moo_param_array_free ((GParameter*) real_props->data, real_props->len);
            g_array_free (real_props, FALSE);
            moo_param_array_free ((GParameter*) fake_props->data, fake_props->len);
            g_array_free (fake_props, FALSE);

            g_type_class_unref (klass);
            return FALSE;
        }

        param.name = g_strdup (prop_name);
        param.value.g_type = 0;
        g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        G_VALUE_COLLECT (&param.value, var_args, 0, &error);

        if (error)
        {
            g_critical ("%s: %s", G_STRLOC, error);
            g_free (error);
            g_value_unset (&param.value);
            g_free ((char*)param.name);

            moo_param_array_free ((GParameter*) real_props->data, real_props->len);
            g_array_free (real_props, FALSE);
            moo_param_array_free ((GParameter*) fake_props->data, fake_props->len);
            g_array_free (fake_props, FALSE);

            g_type_class_unref (klass);
            return FALSE;
        }

        g_array_append_val (add_to, param);

        prop_name = va_arg (var_args, char*);
    }

    g_type_class_unref (klass);

    *n_real_props_p = real_props->len;
    *real_props_p = (GParameter*) g_array_free (real_props, FALSE);
    *n_fake_props_p = fake_props->len;
    *fake_props_p = (GParameter*) g_array_free (fake_props, FALSE);

    return TRUE;
}


MooActionFactory *
moo_action_factory_new_valist (GType       action_type,
                               const char *first_prop_name,
                               va_list     var_args)
{
    MooActionFactory *factory;

    g_return_val_if_fail (g_type_is_a (action_type, GTK_TYPE_ACTION), NULL);

    factory = g_object_new (MOO_TYPE_ACTION_FACTORY, NULL);

    factory->action_type = action_type;

    if (!collect_valist (action_type,
                         &factory->real_props, &factory->n_real_props,
                         &factory->fake_props, &factory->n_fake_props,
                         first_prop_name, var_args))
    {
        g_object_unref (factory);
        return NULL;
    }

    return factory;
}


static void
array_add_parameter (GArray     *array,
                     GParameter *param)
{
    GParameter p;
    p.name = g_strdup (param->name);
    p.value.g_type = 0;
    g_value_init (&p.value, G_VALUE_TYPE (&param->value));
    g_value_copy (&param->value, &p.value);
    g_array_append_val (array, p);
}


static GParameter *
param_array_concatenate (GParameter *props1,
                         guint       n_props1,
                         GParameter *props2,
                         guint       n_props2,
                         guint      *n_props)
{
    GArray *array;
    guint i;

    array = g_array_new (FALSE, TRUE, sizeof (GParameter));

    for (i = 0; i < n_props1; ++i)
        array_add_parameter (array, &props1[i]);
    for (i = 0; i < n_props2; ++i)
        array_add_parameter (array, &props2[i]);

    *n_props = array->len;
    return (GParameter*) g_array_free (array, FALSE);
}


static void
action_toggled_obj (GtkToggleAction *action,
                    gpointer         data)
{
    typedef void (*ToggledFunc) (gpointer data, gboolean active);
    ToggledFunc func;

    func = g_object_get_data (G_OBJECT (action), "moo-toggle-func");
    g_return_if_fail (func != NULL && G_IS_OBJECT (data));

    g_object_ref (data);
    func (data, gtk_toggle_action_get_active (action));
    g_object_unref (data);
}

static void
action_toggled_data (GtkToggleAction *action,
                     gpointer         data)
{
    typedef void (*ToggledFunc) (gpointer data, gboolean active);
    ToggledFunc func;

    func = g_object_get_data (G_OBJECT (action), "moo-toggle-func");
    g_return_if_fail (func != NULL);

    func (data, gtk_toggle_action_get_active (action));
}

static void
toggled_object_died (GtkAction *action,
                     gpointer   obj)
{
    G_GNUC_UNUSED guint n;

    n = g_signal_handlers_disconnect_by_func (action, (gpointer) action_toggled_obj, obj);
    g_assert (n);

    g_object_set_data (G_OBJECT (action), "moo-toggled-ptr", NULL);
}

static void
moo_action_set_fake_properties (gpointer    action,
                                GParameter *props,
                                guint       n_props)
{
    guint i;

    MooClosure *closure = NULL;
    gpointer closure_object = NULL;
    const char *closure_signal = NULL;
    GCallback closure_callback = NULL;
    GCallback closure_proxy_func = NULL;

    GCallback toggled_callback = NULL;
    gpointer toggled_data = NULL;
    gpointer toggled_object = NULL;

    if (!n_props)
        return;

    for (i = 0; i < n_props; ++i)
    {
        const char *name = props[i].name;
        const GValue *value = &props[i].value;

        if (!strcmp (name, "display-name"))
            moo_action_set_display_name (action, g_value_get_string (value));
        else if (!strcmp (name, "accel"))
            moo_action_set_default_accel (action, g_value_get_string (value));
        else if (!strcmp (name, "no-accel"))
            moo_action_set_no_accel (action, g_value_get_boolean (value));
        else if (!strcmp (name, "force-accel-label"))
            _moo_action_set_force_accel_label (action, g_value_get_boolean (value));
        else if (!strcmp (name, "dead"))
            _moo_action_set_dead (action, g_value_get_boolean (value));
        else if (!strcmp (name, "has-submenu"))
            _moo_action_set_has_submenu (action, g_value_get_boolean (value));
        else if (!strcmp (name, "closure"))
            closure = g_value_get_boxed (value);
        else if (!strcmp (name, "closure-object"))
            closure_object = g_value_get_object (value);
        else if (!strcmp (name, "closure-signal"))
            closure_signal = g_value_get_string (value);
        else if (!strcmp (name, "closure-callback"))
            closure_callback = g_value_get_pointer (value);
        else if (!strcmp (name, "closure-proxy-func"))
            closure_proxy_func = g_value_get_pointer (value);
        else if (!strcmp (name, "toggled-callback"))
            toggled_callback = g_value_get_pointer (value);
        else if (!strcmp (name, "toggled-data"))
            toggled_data = g_value_get_pointer (value);
        else if (!strcmp (name, "toggled-object"))
            toggled_object = g_value_get_object (value);
        else
            g_warning ("%s: unknown property '%s'", G_STRLOC, name);
    }

    if (closure_callback || closure_signal)
    {
        if (closure_object)
            closure = moo_closure_new_simple (closure_object, closure_signal,
                                              closure_callback, closure_proxy_func);
        else
            g_warning ("%s: closure data missing", G_STRLOC);
    }

    if (closure)
        _moo_action_set_closure (action, closure);

    if (toggled_callback)
    {
        g_object_set_data (G_OBJECT (action), "moo-toggle-func", toggled_callback);

        if (toggled_object)
        {
            MooObjectPtr *old_ptr;
            gpointer ptr = moo_object_ptr_new (toggled_object,
                                               (GWeakNotify) toggled_object_died,
                                               action);

            old_ptr = g_object_get_data (G_OBJECT (action), "moo-toggled-ptr");

            if (old_ptr)
                g_signal_handlers_disconnect_by_func (action,
                                                      (gpointer) action_toggled_obj,
                                                      MOO_OBJECT_PTR_GET (old_ptr));

            g_object_set_data_full (G_OBJECT (action), "moo-toggled-ptr",
                                    ptr, (GDestroyNotify) moo_object_ptr_free);

            g_signal_connect (action, "toggled",
                              G_CALLBACK (action_toggled_obj),
                              toggled_object);
        }
        else
        {
            g_signal_connect (action, "toggled",
                              G_CALLBACK (action_toggled_data),
                              toggled_data);
        }
    }
}


static gpointer
create_from_params (GType       action_type,
                    GParameter *real_props,
                    guint       n_real_props,
                    GParameter *fake_props,
                    guint       n_fake_props)
{
    gpointer action;

    action = g_object_newv (action_type, n_real_props, real_props);
    g_return_val_if_fail (action != NULL, NULL);

    moo_action_set_fake_properties (action, fake_props, n_fake_props);

    return action;
}


gpointer
moo_action_factory_create_action (MooActionFactory   *factory,
                                  gpointer            data,
                                  const char         *prop_name,
                                  ...)
{
    GObject *object;
    GParameter *real_props, *fake_props, *add_real_props, *add_fake_props;
    guint n_real_props, n_fake_props, n_add_real_props, n_add_fake_props;
    va_list var_args;
    gboolean success;

    g_return_val_if_fail (MOO_IS_ACTION_FACTORY (factory), NULL);

    if (factory->factory_func)
    {
        if (factory->factory_func_data)
            data = factory->factory_func_data;
        return factory->factory_func (data, factory);
    }

    if (!prop_name)
        return create_from_params (factory->action_type,
                                   factory->real_props,
                                   factory->n_real_props,
                                   factory->fake_props,
                                   factory->n_fake_props);

    va_start (var_args, prop_name);

    success = collect_valist (factory->action_type,
                              &add_real_props, &n_add_real_props,
                              &add_fake_props, &n_add_fake_props,
                              prop_name, var_args);

    va_end (var_args);

    if (!success)
        return NULL;

    real_props = param_array_concatenate (factory->real_props,
                                          factory->n_real_props,
                                          add_real_props,
                                          n_add_real_props,
                                          &n_real_props);
    fake_props = param_array_concatenate (factory->fake_props,
                                          factory->n_fake_props,
                                          add_fake_props,
                                          n_add_fake_props,
                                          &n_fake_props);

    object = create_from_params (factory->action_type,
                                 real_props, n_real_props,
                                 fake_props, n_fake_props);

    moo_param_array_free (real_props, n_real_props);
    moo_param_array_free (fake_props, n_fake_props);
    moo_param_array_free (add_real_props, n_add_real_props);
    moo_param_array_free (add_fake_props, n_add_fake_props);

    return object;
}


MooActionFactory *
moo_action_factory_new (GType       action_type,
                        const char *first_prop_name,
                        ...)
{
    MooActionFactory *factory;
    va_list var_args;

    g_return_val_if_fail (g_type_is_a (action_type, GTK_TYPE_ACTION), NULL);

    va_start (var_args, first_prop_name);
    factory = moo_action_factory_new_valist (action_type, first_prop_name, var_args);
    va_end (var_args);

    return factory;
}


MooActionFactory *
moo_action_factory_new_a (GType       action_type,
                          GParameter *params,
                          guint       n_params)
{
    MooActionFactory *factory;
    GObjectClass *klass;
    GArray *real_props, *fake_props;
    guint i;

    g_return_val_if_fail (g_type_is_a (action_type, GTK_TYPE_ACTION), NULL);

    klass = g_type_class_ref (action_type);
    real_props = g_array_new (FALSE, TRUE, sizeof (GParameter));
    fake_props = g_array_new (FALSE, TRUE, sizeof (GParameter));

    for (i = 0; i < n_params; ++i)
    {
        GParameter param;
        GParamSpec *pspec;
        GArray *add_to = NULL;
        const char *prop_name = params[i].name;

        if ((pspec = _moo_action_find_fake_property (klass, prop_name)))
        {
            add_to = fake_props;
        }
        else if ((pspec = g_object_class_find_property (klass, prop_name)))
        {
            add_to = real_props;
        }
        else
        {
            g_warning ("%s: could not find property '%s' for class '%s'",
                       G_STRLOC, prop_name, g_type_name (action_type));

            moo_param_array_free ((GParameter*) real_props->data, real_props->len);
            g_array_free (real_props, FALSE);
            moo_param_array_free ((GParameter*) fake_props->data, fake_props->len);
            g_array_free (fake_props, FALSE);

            g_type_class_unref (klass);
            return NULL;
        }

        param.name = g_strdup (prop_name);
        param.value.g_type = 0;
        g_value_init (&param.value, G_PARAM_SPEC_VALUE_TYPE (pspec));
        g_value_copy (&params[i].value, &param.value);

        g_array_append_val (add_to, param);
    }

    g_type_class_unref (klass);

    factory = g_object_new (MOO_TYPE_ACTION_FACTORY, NULL);
    factory->action_type = action_type;

    factory->n_real_props = real_props->len;
    factory->real_props = (GParameter*) g_array_free (real_props, FALSE);
    factory->n_fake_props = fake_props->len;
    factory->fake_props = (GParameter*) g_array_free (fake_props, FALSE);

    return factory;
}


GtkAction *
moo_action_new_valist (GType       action_type,
                       const char *name,
                       const char *first_prop_name,
                       va_list     var_args)
{
    MooActionFactory *factory;
    GtkAction *action;

    g_return_val_if_fail (g_type_is_a (action_type, GTK_TYPE_ACTION), NULL);

    factory = moo_action_factory_new_valist (action_type, first_prop_name, var_args);
    action = moo_action_factory_create_action (factory, NULL, "name", name, NULL);

    g_object_unref (factory);
    return action;
}


MooActionFactory*
moo_action_factory_new_func (MooActionFactoryFunc factory_func,
                             gpointer             data)
{
    MooActionFactory *factory;

    g_return_val_if_fail (factory_func != NULL, NULL);

    factory = g_object_new (MOO_TYPE_ACTION_FACTORY, NULL);
    factory->factory_func = factory_func;
    factory->factory_func_data = data;

    return factory;
}


GParamSpec *
_moo_action_find_fake_property (GObjectClass *klass,
                                const char   *name)
{
    guint i;
    char *norm_name;
    GParamSpec *pspec = NULL;

    g_return_val_if_fail (klass != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    action_init_props ();
    norm_name = g_strdelimit (g_strdup (name), "_", '-');

    for (i = 0; i < PROP_LAST; ++i)
    {
        if (!strcmp (g_param_spec_get_name (pspecs[i]), norm_name))
        {
            pspec = pspecs[i];
            goto out;
        }
    }

out:
    g_free (norm_name);
    return pspec;
}


GParamSpec *
_moo_action_find_property (GObjectClass *klass,
                           const char   *name)
{
    GParamSpec *pspec;

    g_return_val_if_fail (klass != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    pspec = _moo_action_find_fake_property (klass, name);

    if (!pspec)
        pspec = g_object_class_find_property (klass, name);

    return pspec;
}
