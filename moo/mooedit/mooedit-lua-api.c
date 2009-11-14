/*
 *   mooedit-lua-api.c
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MOOEDIT_COMPILATION
#include "mooedit/mooedit-lua.h"
#include "mooedit/mooeditor.h"
#include "mooutils/mooutils-misc.h"
#include "mooutils/mootype-macros.h"
#include <string.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>


/********************************************************************/
/* GObject
 */

MOO_DEFINE_QUARK_STATIC (moo-lua-type, lua_type_quark)

#ifdef MOO_ENABLE_UNIT_TESTS
static int object_count;
#endif

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

    for (type = G_OBJECT_TYPE (object); ; type = g_type_parent (type))
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

        if (type == G_TYPE_OBJECT)
            break;
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

#define METH_LREG(Type,name) { #name, cfunc_##Type##_##name }
#define METH(Type,name) static int cfunc_##Type##_##name (lua_State *L)

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
#define GET_BUFFER(L) GtkTextBuffer *buffer = gtk_text_view_get_buffer (GET_TEXT_VIEW (L))

static void
push_offset (lua_State *L,
             int        c_offset)
{
    lua_pushinteger (L, c_offset + 1);
}

static void
push_iter_offset (lua_State         *L,
                  const GtkTextIter *iter)
{
    push_offset (L, gtk_text_iter_get_offset (iter));
}

static int
check_offset (lua_State *L,
              int        arg)
{
    return luaL_checkinteger (L, arg) - 1;
}

static int
check_opt_offset (lua_State *L,
                  int        arg,
                  int        dflt)
{
    return luaL_optinteger (L, arg, dflt + 1) - 1;
}

METH (GtkTextView, get_cursor)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    push_iter_offset (L, &iter);
    return 1;
}

METH (GtkTextView, get_selection_bound)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_selection_bound (buffer));
    push_iter_offset (L, &iter);
    return 1;
}

METH (GtkTextView, get_selection)
{
    GET_BUFFER (L);
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        push_iter_offset (L, &start);
        push_iter_offset (L, &end);
        return 2;
    }
    else
    {
        lua_pushnil (L);
        return 1;
    }
}

METH (GtkTextView, set_cursor)
{
    GET_BUFFER (L);
    int pos = check_offset (L, 2);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    gtk_text_buffer_place_cursor (buffer, &iter);
    return 0;
}

METH (GtkTextView, select_range)
{
    GET_BUFFER (L);
    int sb_pos = check_offset (L, 2);
    int ins_pos = check_opt_offset (L, 3, -1);
    GtkTextIter sb, ins;
    gtk_text_buffer_get_iter_at_offset (buffer, &sb, sb_pos);
    gtk_text_buffer_get_iter_at_offset (buffer, &ins, ins_pos);
    gtk_text_buffer_select_range (buffer, &ins, &sb);
    return 0;
}

METH (GtkTextView, unselect)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    gtk_text_buffer_place_cursor (buffer, &iter);
    return 0;
}

METH (GtkTextView, scroll_to_cursor)
{
    GtkTextView *textview = GET_TEXT_VIEW (L);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (textview);
    gtk_text_view_scroll_mark_onscreen (textview,
                                        gtk_text_buffer_get_insert (buffer));
    return 0;
}

METH (GtkTextView, delete_selection)
{
    GET_BUFFER (L);
    gtk_text_buffer_delete_selection (buffer, FALSE, TRUE);
    return 0;
}

METH (GtkTextView, delete_range)
{
    GET_BUFFER (L);
    int start_pos = check_offset (L, 2);
    int end_pos = check_opt_offset (L, 3, -1);
    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_offset (buffer, &start, start_pos);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, end_pos);
    gtk_text_buffer_delete (buffer, &start, &end);
    return 0;
}

METH (GtkTextView, get_text)
{
    GET_BUFFER (L);
    int start_pos = check_opt_offset (L, 2, 0);
    int end_pos = check_opt_offset (L, 3, -1);
    char *text;
    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_offset (buffer, &start, start_pos);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, end_pos);
    text = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
    lua_take_utf8string (L, text);
    return 1;
}

METH (GtkTextView, get_selected_text)
{
    GET_BUFFER (L);
    GtkTextIter start, end;

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
    {
        char *text = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
        lua_take_utf8string (L, text);
    }
    else
    {
        lua_pushnil (L);
    }

    return 1;
}

METH (GtkTextView, replace_selection)
{
    GET_BUFFER (L);
    const char *text = lua_check_utf8string (L, 2, NULL);
    GtkTextIter start, end;
    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);
    gtk_text_buffer_insert (buffer, &start, text, -1);
    return 0;
}

METH (GtkTextView, insert_text)
{
    GET_BUFFER (L);
    const char *text;
    GtkTextIter iter;
    int pos;

    text = lua_check_utf8string (L, 2, NULL);
    pos = check_opt_offset (L, 3, G_MININT);
    if (pos == G_MININT)
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
    else
        gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);

    gtk_text_buffer_insert (buffer, &iter, text, -1);
    return 0;
}

METH (GtkTextView, set_text)
{
    GET_BUFFER (L);
    size_t len;
    const char *text = lua_check_utf8string (L, 2, &len);
    gtk_text_buffer_set_text (buffer, text, len);
    return 0;
}

METH (GtkTextView, get_char_count)
{
    GET_BUFFER (L);
    lua_pushinteger (L, gtk_text_buffer_get_char_count (buffer));
    return 1;
}

METH (GtkTextView, get_line_count)
{
    GET_BUFFER (L);
    lua_pushinteger (L, gtk_text_buffer_get_line_count (buffer));
    return 1;
}

METH (GtkTextView, get_pos_at_line)
{
    GET_BUFFER (L);
    GtkTextIter iter = {0};
    int line = check_offset (L, 2);
    gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
    push_iter_offset (L, &iter);
    if (!gtk_text_iter_ends_line (&iter))
        gtk_text_iter_forward_to_line_end (&iter);
    push_iter_offset (L, &iter);
    gtk_text_iter_forward_line (&iter);
    push_iter_offset (L, &iter);
    return 3;
}

METH (GtkTextView, get_line_at_pos)
{
    GET_BUFFER (L);
    GtkTextIter iter = {0};
    int pos = check_offset (L, 2);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    push_offset (L, gtk_text_iter_get_line (&iter));
    return 1;
}

METH (GtkTextView, get_line_at_cursor)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (buffer, &iter,
                                      gtk_text_buffer_get_insert (buffer));
    push_offset (L, gtk_text_iter_get_line (&iter));
    return 1;
}

METH (GtkTextView, get_line_text)
{
    GET_BUFFER (L);
    GtkTextIter start, end;
    char *text;
    int line = check_opt_offset (L, 2, G_MININT);
    if (line == G_MININT)
    {
        gtk_text_buffer_get_iter_at_mark (buffer, &start,
                                          gtk_text_buffer_get_insert (buffer));
        gtk_text_iter_set_line_offset (&start, 0);
    }
    else
        gtk_text_buffer_get_iter_at_line (buffer, &start, line);
    end = start;
    if (!gtk_text_iter_ends_line (&end))
        gtk_text_iter_forward_to_line_end (&end);
    text = gtk_text_buffer_get_slice (buffer, &start, &end, TRUE);
    lua_pushstring (L, text);
    g_free (text);
    return 1;
}

METH (GtkTextView, next_pos)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    int pos = check_offset (L, 2);
    gboolean ret;
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    ret = gtk_text_iter_forward_cursor_position (&iter);
    push_iter_offset (L, &iter);
    lua_pushboolean (L, ret);
    return 2;
}

METH (GtkTextView, prev_pos)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    int pos = check_offset (L, 2);
    gboolean ret;
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    ret = gtk_text_iter_backward_cursor_position (&iter);
    push_iter_offset (L, &iter);
    lua_pushboolean (L, ret);
    return 2;
}

METH (GtkTextView, get_end_pos)
{
    GET_BUFFER (L);
    push_offset (L, gtk_text_buffer_get_char_count (buffer));
    return 1;
}

METH (GtkTextView, is_end_pos)
{
    GET_BUFFER (L);
    int pos = check_offset (L, 2);
    lua_pushboolean (L, pos == gtk_text_buffer_get_char_count (buffer));
    return 1;
}

METH (GtkTextView, is_cursor_pos)
{
    GET_BUFFER (L);
    GtkTextIter iter;
    int pos = check_offset (L, 2);
    gtk_text_buffer_get_iter_at_offset (buffer, &iter, pos);
    lua_pushboolean (L, gtk_text_iter_is_cursor_position (&iter));
    return 1;
}

static const luaL_Reg meths_GtkTextView[] = {
    METH_LREG (GtkTextView, get_cursor),
    METH_LREG (GtkTextView, get_selection_bound),
    METH_LREG (GtkTextView, get_selection),
    METH_LREG (GtkTextView, set_cursor),
    METH_LREG (GtkTextView, select_range),
    METH_LREG (GtkTextView, unselect),
    METH_LREG (GtkTextView, scroll_to_cursor),

    METH_LREG (GtkTextView, next_pos),
    METH_LREG (GtkTextView, prev_pos),
    METH_LREG (GtkTextView, is_cursor_pos),
    METH_LREG (GtkTextView, is_end_pos),
    METH_LREG (GtkTextView, get_end_pos),

    METH_LREG (GtkTextView, get_char_count),
    METH_LREG (GtkTextView, get_line_count),
    METH_LREG (GtkTextView, get_pos_at_line),
    METH_LREG (GtkTextView, get_line_at_pos),
    METH_LREG (GtkTextView, get_line_at_cursor),
    METH_LREG (GtkTextView, get_line_text),

    METH_LREG (GtkTextView, delete_selection),
    METH_LREG (GtkTextView, delete_range),

    METH_LREG (GtkTextView, get_text),
    METH_LREG (GtkTextView, get_selected_text),

    METH_LREG (GtkTextView, replace_selection),
    METH_LREG (GtkTextView, insert_text),
    METH_LREG (GtkTextView, set_text),

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

    register_type (G_TYPE_OBJECT, meths_GObject);
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
    MOO_OBJECT_REF_SINK (object);
    *object_ptr = object;
#ifdef MOO_ENABLE_UNIT_TESTS
    object_count += 1;
#endif

    if (luaL_newmetatable (L, "medit.GObject"))
        luaL_register (L, NULL, meths_GObject);

    lua_setmetatable (L, -2);
}

#ifdef MOO_ENABLE_UNIT_TESTS

#include "moolua/moo-tests-lua.h"
#include <mooedit/mooedit-tests.h>

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

static void
test_moo_edit (void)
{
    TEST_ASSERT (object_count == 0);
    moo_test_run_lua_file ("mooedit.lua", add_test_api, NULL);
    TEST_ASSERT (object_count == 0);
}

void
moo_test_mooedit_lua_api (void)
{
    MooTestSuite *suite;

    suite = moo_test_suite_new ("mooedit/mooedit-lua-api.c", NULL, NULL, NULL);

    moo_test_suite_add_test (suite, "test of GObject",
                             (MooTestFunc) test_gobject, NULL);
    moo_test_suite_add_test (suite, "test of GtkTextView",
                             (MooTestFunc) test_gtk_text_view, NULL);
    moo_test_suite_add_test (suite, "test of the editor",
                             (MooTestFunc) test_moo_edit, NULL);
}

#endif
