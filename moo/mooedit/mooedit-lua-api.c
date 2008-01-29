/*
 *   mooedit-lua-api.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License version 2.1 as published by the Free Software Foundation.
 *
 *   See COPYING file that comes with this distribution.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-lua.h"
#include "mooedit/mooeditor.h"
#include "mooutils/mooutils-misc.h"
#include <string.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>


/********************************************************************/
/* GObject
 */

#ifdef MOO_ENABLE_UNIT_TESTS
static int object_count;
#endif

static GQuark
lua_type_quark (void)
{
    static GQuark q;
    if (G_UNLIKELY (!q))
        q = g_quark_from_static_string ("moo-lua-type");
    return q;
}

static GObject **
check_gobject (lua_State *L)
{
    return luaL_checkudata (L, 1, "medit.GObject");
}

static gpointer
get_gobject (lua_State *L,
             int        arg,
             GType      type)
{
    GObject **ptr = check_gobject (L);

    if (!G_TYPE_CHECK_INSTANCE_TYPE (*ptr, type))
        luaL_error (L, "arg %d is not a %s", arg, g_type_name (type));

    return *ptr;
}

#define GET_GOBJECT(L) G_OBJECT (get_gobject (L, 1, G_TYPE_OBJECT))

static int
cfunc_GObject__gc (lua_State *L)
{
    GObject **op = check_gobject (L);

    if (*op)
    {
        g_object_unref (*op);
        *op = NULL;
#ifdef MOO_ENABLE_UNIT_TESTS
        object_count -= 1;
#endif
    }

    return 0;
}

static int
cfunc_GObject__tostring (lua_State *L)
{
    GObject *object;
    object = GET_GOBJECT (L);
    lua_pushfstring (L, "<%s at %p>",
                     g_type_name (G_OBJECT_TYPE (object)),
                     (gpointer) object);
    return 1;
}

static void
gvalue_from_lua (lua_State *L,
                 int        arg,
                 GValue    *val)
{
    switch (G_TYPE_FUNDAMENTAL (val->g_type))
    {
        case G_TYPE_CHAR:
            g_value_set_char (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_UCHAR:
            g_value_set_uchar (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_BOOLEAN:
            g_value_set_boolean (val, lua_toboolean (L, arg));
            break;
        case G_TYPE_INT:
            g_value_set_int (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_UINT:
            g_value_set_uint (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_LONG:
            g_value_set_long (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_ULONG:
            g_value_set_ulong (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_INT64:
            g_value_set_int64 (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_UINT64:
            g_value_set_uint64 (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_ENUM:
            g_value_set_enum (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_FLAGS:
            g_value_set_flags (val, lua_tointeger (L, arg));
            break;
        case G_TYPE_FLOAT:
            g_value_set_float (val, lua_tonumber (L, arg));
            break;
        case G_TYPE_DOUBLE:
            g_value_set_double (val, lua_tonumber (L, arg));
            break;
        case G_TYPE_STRING:
            if (lua_isnil (L, arg))
                g_value_set_string (val, NULL);
            else
                g_value_set_string (val, lua_tostring (L, arg));
            break;
        case G_TYPE_OBJECT:
            if (lua_isnil (L, arg))
                g_value_set_object (val, NULL);
            else
                g_value_set_object (val, get_gobject (L, arg, G_TYPE_OBJECT));
            break;
        default:
            luaL_error (L, "unsupported value type '%s'",
                        g_type_name (val->g_type));
    }
}

static int
gvalue_to_lua (lua_State    *L,
               const GValue *val)
{
    const char *s;
    GObject *o;

    switch (G_TYPE_FUNDAMENTAL (val->g_type))
    {
        case G_TYPE_CHAR:
            lua_pushnumber (L, g_value_get_char (val));
            break;
        case G_TYPE_UCHAR:
            lua_pushnumber (L, g_value_get_uchar (val));
            break;
        case G_TYPE_BOOLEAN:
            lua_pushboolean (L, g_value_get_boolean (val));
            break;
        case G_TYPE_INT:
            lua_pushnumber (L, g_value_get_int (val));
            break;
        case G_TYPE_UINT:
            lua_pushnumber (L, g_value_get_uint (val));
            break;
        case G_TYPE_LONG:
            lua_pushnumber (L, g_value_get_long (val));
            break;
        case G_TYPE_ULONG:
            lua_pushnumber (L, g_value_get_ulong (val));
            break;
        case G_TYPE_INT64:
            lua_pushnumber (L, g_value_get_int64 (val));
            break;
        case G_TYPE_UINT64:
            lua_pushnumber (L, g_value_get_uint64 (val));
            break;
        case G_TYPE_ENUM:
            lua_pushnumber (L, g_value_get_enum (val));
            break;
        case G_TYPE_FLAGS:
            lua_pushnumber (L, g_value_get_flags (val));
            break;
        case G_TYPE_FLOAT:
            lua_pushnumber (L, g_value_get_float (val));
            break;
        case G_TYPE_DOUBLE:
            lua_pushnumber (L, g_value_get_double (val));
            break;
        case G_TYPE_STRING:
            if ((s = g_value_get_string (val)))
                lua_pushstring (L, s);
            else
                lua_pushnil (L);
            break;
        case G_TYPE_OBJECT:
            if ((o = g_value_get_object (val)))
                _moo_lua_push_object (L, o);
            else
                lua_pushnil (L);
            break;
        default:
            luaL_error (L, "unsupported value type '%s'",
                        g_type_name (val->g_type));
    }

    return 1;
}

static int
cfunc_GObject_set_property (lua_State *L)
{
    GObject *object;
    const char *propname;
    GParamSpec *pspec;
    GValue val = {0};

    object = GET_GOBJECT (L);
    propname = luaL_checkstring (L, 2);
    luaL_checkany (L, 3);

    pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), propname);
    if (!pspec)
        luaL_error (L, "no property named '%s' in object '<%s at %p>'",
                    propname, g_type_name (G_OBJECT_TYPE (object)),
                    (gpointer) object);

    g_value_init (&val, pspec->value_type);
    gvalue_from_lua (L, 3, &val);
    g_object_set_property (object, propname, &val);
    g_value_unset (&val);

    return 0;
}

static int
cfunc_GObject_get_property (lua_State *L)
{
    GObject *object;
    const char *propname;
    GParamSpec *pspec;
    GValue val = {0};
    int n_ret;

    object = GET_GOBJECT (L);
    propname = luaL_checkstring (L, 2);

    pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), propname);
    if (!pspec)
        luaL_error (L, "no property named '%s' in object '<%s at %p>'",
                    propname, g_type_name (G_OBJECT_TYPE (object)),
                    (gpointer) object);

    g_value_init (&val, pspec->value_type);
    g_object_get_property (object, propname, &val);
    n_ret = gvalue_to_lua (L, &val);
    g_value_unset (&val);

    return n_ret;
}

static int
cfunc_GObject__index (lua_State *L)
{
    GObject *object;
    GType type;

    object = GET_GOBJECT (L);
    luaL_checkany (L, 2);

    for (type = G_OBJECT_TYPE (object); type != G_TYPE_OBJECT; type = g_type_parent (type))
    {
        const luaL_Reg *meths;

        if ((meths = g_type_get_qdata (type, lua_type_quark ())))
        {
            char *tname;

            tname = g_strdup_printf ("medit.%s", g_type_name (type));
            if (luaL_newmetatable (L, tname))
                luaL_register (L, NULL, meths);
            g_free (tname);

            lua_pushvalue (L, 2);
            lua_rawget (L, -2);

            if (!lua_isnil (L, -1))
                return 1;

            lua_pop (L, 1);
        }
    }

    return 0;
}

static const luaL_Reg meths_GObject[] = {
    { "__gc", cfunc_GObject__gc },
    { "__tostring", cfunc_GObject__tostring },
    { "__index", cfunc_GObject__index },
    { "set_property", cfunc_GObject_set_property },
    { "get_property", cfunc_GObject_get_property },
    { NULL, NULL }
};


#define METH_VOID_VOID(Typ,GET_OBJ,name,func)   \
static int                                      \
cfunc_##Typ##_##name (lua_State *L)             \
{                                               \
    func (GET_OBJ (L));                         \
    return 0;                                   \
}

/********************************************************************/
/* GtkWidget
 */

#define GET_WIDGET(L) GTK_WIDGET (get_gobject (L, 1, GTK_TYPE_WIDGET))

// METH_VOID_VOID (GtkWidget, GET_WIDGET, hide, gtk_widget_hide)
// METH_VOID_VOID (GtkWidget, GET_WIDGET, show, gtk_widget_show)

static const luaL_Reg meths_GtkWidget[] = {
    { NULL, NULL }
};


/********************************************************************/
/* GtkTextView
 */

#define GET_TEXT_VIEW(L) GTK_TEXT_VIEW (get_gobject (L, 1, GTK_TYPE_TEXT_VIEW))

static const luaL_Reg meths_GtkTextView[] = {
    { NULL, NULL }
};


/********************************************************************/
/* MooTextView
 */

#define GET_MOO_TEXT_VIEW(L) MOO_TEXT_VIEW (get_gobject (L, 1, MOO_TYPE_TEXT_VIEW))

static const luaL_Reg meths_MooTextView[] = {
    { NULL, NULL }
};


/********************************************************************/
/* MooEdit
 */

#define GET_MOO_EDIT(L) MOO_EDIT (get_gobject (L, 1, MOO_TYPE_EDIT))

static const luaL_Reg meths_MooEdit[] = {
    { NULL, NULL }
};


/********************************************************************/
/* MooEditor
 */

#define GET_MOO_EDITOR(L) MOO_EDITOR (get_gobject (L, 1, MOO_TYPE_EDITOR))

static const luaL_Reg meths_MooEditor[] = {
    { NULL, NULL }
};


/********************************************************************/
/* External API
 */

static void
register_type (GType           type,
               const luaL_Reg *meths)
{
    if (meths && meths[0].name)
        g_type_set_qdata (type, lua_type_quark (), (gpointer) meths);
}

static void
init_types (void)
{
    static gboolean been_here;

    if (been_here)
        return;

    been_here = TRUE;

    register_type (GTK_TYPE_WIDGET, meths_GtkWidget);
    register_type (GTK_TYPE_TEXT_VIEW, meths_GtkTextView);
    register_type (MOO_TYPE_TEXT_VIEW, meths_MooTextView);
    register_type (MOO_TYPE_EDIT, meths_MooEdit);
    register_type (MOO_TYPE_EDITOR, meths_MooEditor);
}

void
_moo_lua_push_object (lua_State *L,
                      GObject   *object)
{
    GObject **object_ptr;

    init_types ();

    object_ptr = lua_newuserdata (L, sizeof (gpointer));
    *object_ptr = MOO_OBJECT_REF_SINK (object);
#ifdef MOO_ENABLE_UNIT_TESTS
    object_count += 1;
#endif

    if (luaL_newmetatable (L, "medit.GObject"))
    {
        luaL_register (L, NULL, meths_GObject);
        lua_pushstring (L, "__index");
        lua_pushvalue (L, -2);  /* pushes the metatable */
        lua_settable (L, -3);  /* metatable.__index = metatable */
    }

    lua_setmetatable (L, -2);
}

#ifdef MOO_ENABLE_UNIT_TESTS

#include "moo-tests-lua.h"

static int
cfunc_present_widget (lua_State *L)
{
    GtkWidget *window;
    GtkWidget *widget;

    widget = GET_WIDGET (L);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (window, 300, 300);
    gtk_container_add (GTK_CONTAINER (window), widget);
    gtk_widget_show_all (window);

    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    gtk_main ();

    return 0;
}

static int
cfunc_new_text_view (lua_State *L)
{
    GtkWidget *textview = gtk_text_view_new ();
    _moo_lua_push_object (L, G_OBJECT (textview));
    return 1;
}

static const luaL_reg mooedittestlib[] = {
    { "new_text_view", cfunc_new_text_view },
    { "present", cfunc_present_widget },
    { NULL, NULL }
};

static void
add_test_api (lua_State *L)
{
    luaL_register (L, "mooedittest", mooedittestlib);
    lua_pop (L, 1);
}

static void
test_gtk_text_view (void)
{
    TEST_ASSERT (object_count == 0);
    moo_test_run_lua_file ("textview.lua", add_test_api, NULL);
    TEST_ASSERT (object_count == 0);
}

static void
test_gobject (void)
{
    TEST_ASSERT (object_count == 0);
    moo_test_run_lua_file ("gobject.lua", add_test_api, NULL);
    TEST_ASSERT (object_count == 0);
}

void
moo_test_mooedit_lua_api (void)
{
    CU_pSuite suite = CU_add_suite ("mooedit/mooedit-lua-api.c", NULL, NULL);
    CU_add_test (suite, "test of GObject", test_gobject);
    CU_add_test (suite, "test of GtkTextView", test_gtk_text_view);
}

#endif
