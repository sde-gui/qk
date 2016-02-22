/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#define PY_SSIZE_T_CLEAN
#include <Python.h>




#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
typedef inquiry lenfunc;
typedef intargfunc ssizeargfunc;
typedef intobjargproc ssizeobjargproc;
#endif


#line 24 "./gdk.override"
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "pygtk-private.h"

#if defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined(GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#endif

/* make GDK_DISPLAY() the case macro */
#undef GDK_DISPLAY
#define GDK_DISPLAY(object) (GDK_DISPLAY_OBJECT(object))

#ifdef HAVE_NUMPY
#  include <numpy/arrayobject.h>
static int have_numpy(void);
#endif

extern PyTypeObject PyGtkWidget_Type;

#ifdef HAVE_PYCAIRO
# include <pycairo.h>
extern Pycairo_CAPI_t *Pycairo_CAPI;
extern PyTypeObject PyGdkCairoContext_Type;
#endif

#ifndef GDK_TYPE_REGION
GType
pygdk_region_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("GdkRegion",
					     (GBoxedCopyFunc)gdk_region_copy,
					     (GBoxedFreeFunc)gdk_region_destroy);
  return our_type;
}
#endif

#ifndef GDK_TYPE_PIXBUF_SIMPLE_ANIM_ITER
GType gtk_print_capabilites_get_type (void) G_GNUC_CONST;
#define GDK_TYPE_PIXBUF_SIMPLE_ANIM_ITER (gdk_pixbuf_simple_anim_iter_get_type())
#endif

#line 75 "gdk.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyPangoContext_Type;
#define PyPangoContext_Type (*_PyPangoContext_Type)
static PyTypeObject *_PyPangoFont_Type;
#define PyPangoFont_Type (*_PyPangoFont_Type)
static PyTypeObject *_PyPangoLayout_Type;
#define PyPangoLayout_Type (*_PyPangoLayout_Type)
static PyTypeObject *_PyPangoRenderer_Type;
#define PyPangoRenderer_Type (*_PyPangoRenderer_Type)
static PyTypeObject *_PyGAppLaunchContext_Type;
#define PyGAppLaunchContext_Type (*_PyGAppLaunchContext_Type)
static PyTypeObject *_PyGIcon_Type;
#define PyGIcon_Type (*_PyGIcon_Type)
static PyTypeObject *_PyGInputStream_Type;
#define PyGInputStream_Type (*_PyGInputStream_Type)
static PyTypeObject *_PyGCancellable_Type;
#define PyGCancellable_Type (*_PyGCancellable_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyGdkEvent_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkFont_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkColor_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkCursor_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkRectangle_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkRegion_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkAppLaunchContext_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkColormap_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkDevice_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkDisplay_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkDisplayManager_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkDragContext_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkDrawable_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkWindow_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPangoRenderer_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixmap_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkGC_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkImage_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkKeymap_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbuf_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbufAnimation_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbufAnimationIter_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbufLoader_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbufSimpleAnim_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkPixbufSimpleAnimIter_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkScreen_Type;
PyTypeObject G_GNUC_INTERNAL PyGdkVisual_Type;


#line 24 "gdkcairo.override"

#ifdef HAVE_PYCAIRO


static PyTypeObject *_PyPangoCairoContext_Type;
#define PyPangoCairoContext_Type (*_PyPangoCairoContext_Type)

static PyObject *
_wrap_gdk_cairo_set_source_color(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gtk.gdk.CairoContext.set_source_color", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    gdk_cairo_set_source_color(PycairoContext_GET(self), color);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_cairo_set_source_pixbuf(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixbuf", "pixbuf_x", "pixbuf_y", NULL };
    PyGObject *pixbuf;
    double pixbuf_x, pixbuf_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!dd:gtk.gdk.CairoContext.set_source_pixbuf", kwlist,
                                     &PyGdkPixbuf_Type, &pixbuf, &pixbuf_x, &pixbuf_y))
        return NULL;
    gdk_cairo_set_source_pixbuf(PycairoContext_GET(self), GDK_PIXBUF(pixbuf->obj),
                                pixbuf_x, pixbuf_y);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_cairo_rectangle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rectangle", NULL };
    PyObject *py_rectangle;
    GdkRectangle rectangle;
    double x, y, width, height;

    if (PyArg_ParseTuple (args, "dddd:Context.rectangle",
			   &x, &y, &width, &height))
    {
        cairo_rectangle(PycairoContext_GET(self), x, y, width, height);
    } else {
        PyErr_Clear();
        if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gtk.gdk.CairoContext.rectangle", kwlist,
                                         &py_rectangle)) {
            PyErr_SetString(PyExc_TypeError, "gtk.gdk.CairoContext.rectangle accepts either"
                            " (x, y, width, height) or a single gtk.gdk.Rectangle parameter");
            return NULL;
        }
        if (!pyg_boxed_check(py_rectangle, GDK_TYPE_RECTANGLE)) {
            if (!pygdk_rectangle_from_pyobject(py_rectangle, &rectangle)) {
                PyErr_Clear();
                PyErr_SetString(PyExc_TypeError,
                                "rectangle must be a GdkRectangle or 4-tuple");
                return NULL;
            }
        } else {
            rectangle = *pyg_boxed_get(py_rectangle, GdkRectangle);
        }
 
        gdk_cairo_rectangle(PycairoContext_GET(self), &rectangle);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_cairo_region(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "region", NULL };
    PyGObject *py_region;
    GdkRegion *region;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gtk.gdk.CairoContext.region", kwlist,
                                     &py_region))
        return NULL;
 
    if (pyg_boxed_check(py_region, PYGDK_TYPE_REGION))
        region = pyg_boxed_get(py_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "region should be a gtk.gdk.Region");
        return NULL;
    }
     
    gdk_cairo_region(PycairoContext_GET(self), region);

    Py_INCREF(Py_None);
    return Py_None;
}

#if GTK_CHECK_VERSION(2, 18, 9)
static PyObject *
_wrap_gdk_cairo_reset_clip(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "drawable", NULL };
    PyGObject *drawable;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:gtk.gdk.CairoContext.reset_clip", kwlist,
				     &PycairoContext_Type, &cr, &PyGdkDrawable_Type,
				     &drawable))
        return NULL;
    
    gdk_cairo_reset_clip(cr->ctx, GDK_DRAWABLE(drawable->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}
#endif

#if GTK_CHECK_VERSION(2, 9, 3)
static PyObject *
_wrap_gdk_cairo_set_source_pixmap(PyObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "pixmap", "pixmap_x", "pixmap_y", NULL };
    PyGObject *pixmap;
    double pixmap_x, pixmap_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!dd:cairo_set_source_pixmap", kwlist,
                                     &PyGdkDrawable_Type, &pixmap,
                                     &pixmap_x, &pixmap_y))
        return NULL;
    gdk_cairo_set_source_pixmap(PycairoContext_GET(self),
                                GDK_DRAWABLE(pixmap->obj),
                                pixmap_x, pixmap_y);
    Py_INCREF(Py_None);
    return Py_None;
}
#endif

static PyMethodDef _PyGdkCairoContext_methods[] = {
    { "set_source_pixbuf", (PyCFunction)_wrap_gdk_cairo_set_source_pixbuf, METH_VARARGS|METH_KEYWORDS },
    { "set_source_color", (PyCFunction)_wrap_gdk_cairo_set_source_color, METH_VARARGS|METH_KEYWORDS },
    { "rectangle", (PyCFunction)_wrap_gdk_cairo_rectangle, METH_VARARGS|METH_KEYWORDS },
    { "region", (PyCFunction)_wrap_gdk_cairo_region, METH_VARARGS|METH_KEYWORDS },
#if GTK_CHECK_VERSION(2, 18, 9)
    { "reset_clip", (PyCFunction)_wrap_gdk_cairo_reset_clip, METH_VARARGS|METH_KEYWORDS },
#endif
#if GTK_CHECK_VERSION(2, 9, 3)
    { "set_source_pixmap", (PyCFunction)_wrap_gdk_cairo_set_source_pixmap, METH_VARARGS|METH_KEYWORDS },
#endif
    { NULL, NULL, 0 }
};


PyTypeObject PyGdkCairoContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "gtk.gdk.CairoContext",             /* tp_name */
    0,           			/* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)0,     			/* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)0,                        /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   		/* tp_flags */
    "A pangocairo.CairoContext enhanced with some additional gdk methods", /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    _PyGdkCairoContext_methods,         /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,          		/* tp_init */
    0,                			/* tp_alloc */
    0,         				/* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};

#endif /* #ifdef HAVE_PYCAIRO */

#line 339 "gdk.c"



/* ----------- GdkEvent ----------- */

static int
_wrap_gdk_event_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GdkEventType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Event.__init__", kwlist, &py_type))
        return -1;
    if (pyg_enum_get_value(GDK_TYPE_EVENT_TYPE, py_type, (gpointer)&type))
        return -1;
    self->gtype = GDK_TYPE_EVENT;
    self->free_on_dealloc = FALSE;
    self->boxed = gdk_event_new(type);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkEvent object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_gdk_event_send_client_message(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "winid", NULL };
    int ret;
    unsigned long winid;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:Gdk.Event.send_client_message", kwlist, &winid))
        return NULL;
    
    ret = gdk_event_send_client_message(pyg_boxed_get(self, GdkEvent), winid);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_event_send_clientmessage_toall(PyObject *self)
{
    
    gdk_event_send_clientmessage_toall(pyg_boxed_get(self, GdkEvent));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_event_put(PyObject *self)
{
    
    gdk_event_put(pyg_boxed_get(self, GdkEvent));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_event_copy(PyObject *self)
{
    GdkEvent *ret;

    
    ret = gdk_event_copy(pyg_boxed_get(self, GdkEvent));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

#line 919 "gdkevent.override"
static PyObject *
_wrap_gdk_event_free(PyObject *self)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "gtk.gdk.Event.free should not be used") < 0)
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}
#line 425 "gdk.c"


static PyObject *
_wrap_gdk_event_get_time(PyObject *self)
{
    guint32 ret;

    
    ret = gdk_event_get_time(pyg_boxed_get(self, GdkEvent));
    
    return PyLong_FromUnsignedLong(ret);

}

#line 908 "gdkevent.override"
static PyObject *
_wrap_gdk_event_get_state(PyGObject *self)
{
    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    GdkModifierType state = 0;

    gdk_event_get_state(event, &state);
    return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE, state);
}
#line 450 "gdk.c"


#line 882 "gdkevent.override"
static PyObject *
_wrap_gdk_event_get_coords(PyGObject *self)
{
    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    gdouble x, y;

    if (gdk_event_get_coords(event, &x, &y))
        return Py_BuildValue("(dd)", x, y);
    else
        return Py_BuildValue("()");
}
#line 465 "gdk.c"


#line 895 "gdkevent.override"
static PyObject *
_wrap_gdk_event_get_root_coords(PyGObject *self)
{
    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    gdouble x, y;

    if (gdk_event_get_root_coords(event, &x, &y))
        return Py_BuildValue("(dd)", x, y);
    else
        return Py_BuildValue("()");
}
#line 480 "gdk.c"


#line 862 "gdkevent.override"
static PyObject *
_wrap_gdk_event_get_axis(PyGObject *self, PyObject *args)
{
    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    gint axis_use;
    gdouble value;

    if (!PyArg_ParseTuple(args, "i:gdk.event_get_axis",
                          &axis_use))
        return NULL;
                                     
    if (gdk_event_get_axis(event, axis_use, &value)) {
        return PyFloat_FromDouble(value);
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 502 "gdk.c"


static PyObject *
_wrap_gdk_event_set_screen(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", NULL };
    PyGObject *screen;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Event.set_screen", kwlist, &PyGdkScreen_Type, &screen))
        return NULL;
    
    gdk_event_set_screen(pyg_boxed_get(self, GdkEvent), GDK_SCREEN(screen->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_event_get_screen(PyObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_event_get_screen(pyg_boxed_get(self, GdkEvent));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkEvent_methods[] = {
    { "send_client_message", (PyCFunction)_wrap_gdk_event_send_client_message, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_clientmessage_toall", (PyCFunction)_wrap_gdk_event_send_clientmessage_toall, METH_NOARGS,
      NULL },
    { "put", (PyCFunction)_wrap_gdk_event_put, METH_NOARGS,
      NULL },
    { "copy", (PyCFunction)_wrap_gdk_event_copy, METH_NOARGS,
      NULL },
    { "free", (PyCFunction)_wrap_gdk_event_free, METH_NOARGS,
      NULL },
    { "get_time", (PyCFunction)_wrap_gdk_event_get_time, METH_NOARGS,
      NULL },
    { "get_state", (PyCFunction)_wrap_gdk_event_get_state, METH_NOARGS,
      NULL },
    { "get_coords", (PyCFunction)_wrap_gdk_event_get_coords, METH_NOARGS,
      NULL },
    { "get_root_coords", (PyCFunction)_wrap_gdk_event_get_root_coords, METH_NOARGS,
      NULL },
    { "get_axis", (PyCFunction)_wrap_gdk_event_get_axis, METH_VARARGS,
      NULL },
    { "set_screen", (PyCFunction)_wrap_gdk_event_set_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_screen", (PyCFunction)_wrap_gdk_event_get_screen, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 475 "gdkevent.override"
static PyObject *
_wrap_gdk_event_tp_getattr(PyObject *self, char *attr)
{
    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    guint i;
    gchar *name;
    PyObject *ret;

    switch(event->type) {
    case GDK_NOTHING: break;
    case GDK_DELETE: break;
    case GDK_DESTROY: break;
    case GDK_EXPOSE:            /*GdkEventExpose            expose*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssss]",
                                 "type", "window", "send_event",
                                 "area", "region", "count");
        if (!strcmp(attr, "area"))
            return pyg_boxed_new(GDK_TYPE_RECTANGLE, &event->expose.area,
                                 TRUE, TRUE);
	if (!strcmp(attr, "region"))
            return pyg_boxed_new(PYGDK_TYPE_REGION, &event->expose.region,
                                 TRUE, TRUE);
        if (!strcmp(attr, "count"))
            return PyInt_FromLong(event->expose.count);
        break;
    case GDK_MOTION_NOTIFY:     /*GdkEventMotion            motion*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssssssssss]",
                                 "type", "window", "send_event",
                                 "time", "x", "y", "axes", "state",
                                 "is_hint", "device", "x_root", "y_root");
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->motion.time);
        if (!strcmp(attr, "x"))
            return PyFloat_FromDouble(event->motion.x);
        if (!strcmp(attr, "y"))
            return PyFloat_FromDouble(event->motion.y);
        if (!strcmp(attr, "axes")) {
            if (event->motion.axes) {
                PyObject *v = PyTuple_New(event->motion.device->num_axes);
                if (!v) return NULL;
                for (i = 0; i < event->motion.device->num_axes; i++)
                    PyTuple_SetItem(v, i,PyFloat_FromDouble(
                                                event->motion.axes[i]));
                return v;
            } else {
                Py_INCREF(Py_None);
                return Py_None;
            }
        }
        if (!strcmp(attr, "state"))
            return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                        event->motion.state);
        if (!strcmp(attr, "is_hint"))
            return PyInt_FromLong(event->motion.is_hint);
        if (!strcmp(attr, "device"))
            return pygobject_new((GObject *)event->motion.device);
        if (!strcmp(attr, "x_root"))
            return PyFloat_FromDouble(event->motion.x_root);
        if (!strcmp(attr, "y_root"))
            return PyFloat_FromDouble(event->motion.y_root);
        break;
    case GDK_BUTTON_PRESS:      /*GdkEventButton            button*/
    case GDK_2BUTTON_PRESS:     /*GdkEventButton            button*/
    case GDK_3BUTTON_PRESS:     /*GdkEventButton            button*/
    case GDK_BUTTON_RELEASE:    /*GdkEventButton            button*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssssssssss]",
                                 "type", "window", "send_event",
                                 "time", "x", "y", "axes", "state",
                                 "button", "device", "x_root", "y_root");
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->button.time);
        if (!strcmp(attr, "x"))
            return PyFloat_FromDouble(event->button.x);
        if (!strcmp(attr, "y"))
            return PyFloat_FromDouble(event->button.y);
        if (!strcmp(attr, "axes")) {
            if (event->button.axes) {
                PyObject *v = PyTuple_New(event->button.device->num_axes);
                if (!v) return NULL;
                for (i = 0; i < event->button.device->num_axes; i++)
                    PyTuple_SetItem(v, i,PyFloat_FromDouble(
                                                event->button.axes[i]));
                return v;
            } else {
                Py_INCREF(Py_None);
                return Py_None;
            }
        }
        if (!strcmp(attr, "state"))
            return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                        event->button.state);
        if (!strcmp(attr, "button"))
            return PyInt_FromLong(event->button.button);
        if (!strcmp(attr, "device"))
            return pygobject_new((GObject *)event->button.device);
        if (!strcmp(attr, "x_root"))
            return PyFloat_FromDouble(event->button.x_root);
        if (!strcmp(attr, "y_root"))
            return PyFloat_FromDouble(event->button.y_root);
        break;
    case GDK_KEY_PRESS:         /*GdkEventKey               key*/
    case GDK_KEY_RELEASE:       /*GdkEventKey               key*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssssssss]",
                                 "type", "window", "send_event",
                                 "time", "state", "keyval", "string",
				 "hardware_keycode", "group", "is_modifier");
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->key.time);
        if (!strcmp(attr, "state"))
            return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                        event->key.state);
        if (!strcmp(attr, "keyval"))
            return PyInt_FromLong(event->key.keyval);
        if (!strcmp(attr, "string"))
            return PyString_FromStringAndSize(event->key.string,
                                              event->key.length);
        if (!strcmp(attr, "hardware_keycode"))
            return PyInt_FromLong(event->key.hardware_keycode);
        if (!strcmp(attr, "group"))
            return PyInt_FromLong(event->key.group);
        if (!strcmp(attr, "is_modifier"))
            return PyInt_FromLong(event->key.is_modifier);
        break;
    case GDK_ENTER_NOTIFY:      /*GdkEventCrossing          crossing*/
    case GDK_LEAVE_NOTIFY:      /*GdkEventCrossing          crossing*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssssssssssss]",
                                 "type", "window", "send_event",
                                 "subwindow", "time", "x", "y",
                                 "x_root", "y_root", "mode", "detail",
                                 "focus", "state");
        if (!strcmp(attr, "subwindow"))
            return pygobject_new((GObject *)event->crossing.subwindow);
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->crossing.time);
        if (!strcmp(attr, "x"))
            return PyFloat_FromDouble(event->crossing.x);
        if (!strcmp(attr, "y"))
            return PyFloat_FromDouble(event->crossing.y);
        if (!strcmp(attr, "x_root"))
            return PyFloat_FromDouble(event->crossing.x_root);
        if (!strcmp(attr, "y_root"))
            return PyFloat_FromDouble(event->crossing.y_root);
        if (!strcmp(attr, "mode"))
            return pyg_enum_from_gtype(GDK_TYPE_CROSSING_MODE,
                                       event->crossing.mode);
        if (!strcmp(attr, "detail"))
            return pyg_enum_from_gtype(GDK_TYPE_NOTIFY_TYPE,
                                        event->crossing.detail);
        if (!strcmp(attr, "focus"))
            return PyInt_FromLong(event->crossing.focus);
        if (!strcmp(attr, "state"))
            return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                        event->crossing.state);
        break;
    case GDK_FOCUS_CHANGE:      /*GdkEventFocus             focus_change*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssss]",
                                 "type", "window", "send_event",
                                 "in_");
        if (!strcmp(attr, "in_"))
            return PyInt_FromLong(event->focus_change.in);
        break;
    case GDK_CONFIGURE:         /*GdkEventConfigure         configure*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssssss]",
                                 "type", "window", "send_event",
                                 "x", "y", "width", "height");
        if (!strcmp(attr, "x"))
            return PyInt_FromLong(event->configure.x);
        if (!strcmp(attr, "y"))
            return PyInt_FromLong(event->configure.y);
        if (!strcmp(attr, "width"))
            return PyInt_FromLong(event->configure.width);
        if (!strcmp(attr, "height"))
            return PyInt_FromLong(event->configure.height);
        break;
    case GDK_MAP: break;
    case GDK_UNMAP: break;
    case GDK_PROPERTY_NOTIFY:   /*GdkEventProperty          property*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssss]",
                                 "type", "window", "send_event",
                                 "atom", "time", "state");
        if (!strcmp(attr, "atom")) {
            name = gdk_atom_name(event->property.atom);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->property.time);
        if (!strcmp(attr, "state"))
            return pyg_enum_from_gtype(GDK_TYPE_PROPERTY_STATE, event->property.state);
        break;
    case GDK_SELECTION_CLEAR:   /*GdkEventSelection         selection*/
    case GDK_SELECTION_REQUEST: /*GdkEventSelection         selection*/
    case GDK_SELECTION_NOTIFY:  /*GdkEventSelection         selection*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssssss]",
                                 "type", "window", "send_event",
                                 "selection", "target", "property",
                                 "requestor", "time");
        if (!strcmp(attr, "selection")) {
            name = gdk_atom_name(event->selection.selection);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "target")) {
            name = gdk_atom_name(event->selection.target);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "property")) {
           name = gdk_atom_name(event->selection.property);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "requestor"))
            return PyInt_FromLong(event->selection.requestor);
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->selection.time);
        break;
    case GDK_PROXIMITY_IN:      /*GdkEventProximity         proximity*/
    case GDK_PROXIMITY_OUT:     /*GdkEventProximity         proximity*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssss]",
                                 "type", "window", "send_event",
                                 "time", "device");
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->proximity.time);
        if (!strcmp(attr, "device"))
            return pygobject_new((GObject *)event->proximity.device);
        break;
    case GDK_DRAG_ENTER:        /*GdkEventDND               dnd*/
    case GDK_DRAG_LEAVE:        /*GdkEventDND               dnd*/
    case GDK_DRAG_MOTION:       /*GdkEventDND               dnd*/
    case GDK_DRAG_STATUS:       /*GdkEventDND               dnd*/
    case GDK_DROP_START:        /*GdkEventDND               dnd*/
    case GDK_DROP_FINISHED:     /*GdkEventDND               dnd*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssssss]",
                                 "type", "window", "send_event",
                                 "context", "time", "x_root", "y_root");
        if (!strcmp(attr, "context"))
            return pygobject_new((GObject *)event->dnd.context);
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->dnd.time);
        if (!strcmp(attr, "x_root"))
            return PyFloat_FromDouble(event->dnd.x_root);
        if (!strcmp(attr, "y_root"))
            return PyFloat_FromDouble(event->dnd.y_root);
        break;
    case GDK_CLIENT_EVENT:      /*GdkEventClient            client*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssss]",
                                 "type", "window", "send_event",
                                 "message_type", "data_format", "data");
        if (!strcmp(attr, "message_type")) {
            name = gdk_atom_name(event->client.message_type);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "data_format"))
            return PyInt_FromLong(event->client.data_format);
        if (!strcmp(attr, "data"))
            return PyString_FromStringAndSize((char *) &event->client.data,
                                              sizeof(event->client.data));
        break;
    case GDK_VISIBILITY_NOTIFY: /*GdkEventVisibility        visibility*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssss]",
                                 "type", "window", "send_event",
                                 "state");
        if (!strcmp(attr, "state"))
            return pyg_enum_from_gtype(GDK_TYPE_VISIBILITY_STATE, event->visibility.state);
        break;
    case GDK_NO_EXPOSE:         /*GdkEventNoExpose          no_expose*/
        break;
    case GDK_SCROLL:            /*GdkEventScroll            scroll*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssssssssss]",
                                 "type", "window", "send_event",
                                 "time", "x", "y", "state", "direction",
                                 "device", "x_root", "y_root");
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->scroll.time);
        if (!strcmp(attr, "x"))
            return PyFloat_FromDouble(event->scroll.x);
        if (!strcmp(attr, "y"))
            return PyFloat_FromDouble(event->scroll.y);
        if (!strcmp(attr, "state"))
            return pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                        event->scroll.state);
        if (!strcmp(attr, "direction"))
            return pyg_enum_from_gtype(GDK_TYPE_SCROLL_DIRECTION, event->scroll.direction);
        if (!strcmp(attr, "device"))
            return pygobject_new((GObject *)event->scroll.device);
        if (!strcmp(attr, "x_root"))
            return PyFloat_FromDouble(event->scroll.x_root);
        if (!strcmp(attr, "y_root"))
            return PyFloat_FromDouble(event->scroll.y_root);
        break;
    case GDK_WINDOW_STATE:      /*GdkEventWindowState       window_state*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssss]",
                                 "type", "window", "send_event",
                                 "changed_mask", "new_window_state");
        if (!strcmp(attr, "changed_mask"))
            return pyg_flags_from_gtype(GDK_TYPE_WINDOW_STATE,
                                        event->window_state.changed_mask);
        if (!strcmp(attr, "new_window_state"))
            return pyg_flags_from_gtype(GDK_TYPE_WINDOW_STATE,
                                        event->window_state.new_window_state);
        break;
    case GDK_SETTING:           /*GdkEventSetting           setting*/
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[sssss]",
                                 "type", "window", "send_event",
                                 "action", "name");
        if (!strcmp(attr, "action"))
            return pyg_enum_from_gtype(GDK_TYPE_SETTING_ACTION, event->setting.action);
        if (!strcmp(attr, "name"))
            return PyString_FromString(event->setting.name);
        break;
    /* New in 2.8 */
    case GDK_OWNER_CHANGE:
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssssss]",
                                 "type", "window", "send_event", "owner",
                                 "reason", "selection", "time",
                                 "selection_time");
        if (!strcmp(attr, "owner"))
            return PyInt_FromLong(event->owner_change.owner);
        if (!strcmp(attr, "reason"))
            return pyg_enum_from_gtype(GDK_TYPE_OWNER_CHANGE,
				       event->owner_change.reason);
        if (!strcmp(attr, "selection")) {
            name = gdk_atom_name(event->owner_change.selection);
            ret = PyString_FromString(name);
            g_free(name);
            return ret;
        }
        if (!strcmp(attr, "time"))
            return PyInt_FromLong(event->owner_change.time);
        if (!strcmp(attr, "selection_time"))
            return PyInt_FromLong(event->owner_change.selection_time);
	break;
    case GDK_GRAB_BROKEN:
        if (!strcmp(attr, "__members__"))
            return Py_BuildValue("[ssssss]",
                                 "type", "window", "send_event",
                                 "keyboard", "implicit",
				 "grab_window");
        if (!strcmp(attr, "keyboard"))
            return PyBool_FromLong(event->grab_broken.keyboard);
        if (!strcmp(attr, "implicit"))
            return PyBool_FromLong(event->grab_broken.implicit);
        if (!strcmp(attr, "grab_window"))
            return pygobject_new((GObject *)event->grab_broken.grab_window);
        break;
	
    default:
        break;
    }
    
    if (!strcmp(attr, "type"))
        return pyg_enum_from_gtype(GDK_TYPE_EVENT_TYPE, event->type);
    if (!strcmp(attr, "window"))
        return pygobject_new((GObject *)event->any.window);
    if (!strcmp(attr, "send_event"))
        return PyInt_FromLong(event->any.send_event);
    if (!strcmp(attr, "__members__"))
        return Py_BuildValue("[sss]", "type", "window", "send_event");

    return Py_FindMethod((PyMethodDef*)_PyGdkEvent_methods, self, attr);
}
#line 946 "gdk.c"


#line 24 "gdkevent.override"
#define INT_CHECK() \
	    if (!PyInt_Check(value)) { \
		PyErr_Format(PyExc_TypeError, "%s must be an int", attr); \
		return -1; \
	    } 
#define INT_OR_LONG_CHECK() \
	    if (!PyInt_Check(value) && !PyLong_Check(value)) { \
		PyErr_Format(PyExc_TypeError, "%s must be an int or a long", attr); \
		return -1; \
	    }
#define FLOAT_CHECK() \
	    if (!PyFloat_Check(value)) { \
		PyErr_Format(PyExc_TypeError, "%s must be a float", attr); \
		return -1; \
	    } 
#define STRING_CHECK() \
	    if (!PyString_Check(value)) { \
		PyErr_Format(PyExc_TypeError, "%s must be a string", attr); \
		return -1; \
	    } 
static int
_wrap_gdk_event_tp_setattr(PyObject *self, char *attr, PyObject *value)
{
    GdkEvent *event;

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "can't delete attributes");
        return -1;
    }

    event = pyg_boxed_get(self, GdkEvent);

    /* Common members */
    if (attr[0] == 't' && !strcmp(attr, "type")) {
	PyErr_SetString(PyExc_AttributeError, "type is not writable");
	return -1;
    } else if (attr[0] == 'w' && !strcmp(attr, "window")) {
	if (!pygobject_check(value, &PyGdkWindow_Type)) {
	    PyErr_SetString(PyExc_TypeError, "window must be a GdkWindow");
	    return -1;
	}
	if (event->any.window)
            g_object_unref(event->any.window);
	event->any.window = g_object_ref(GDK_WINDOW(((PyGObject*)value)->obj));
	return 0;
    } else if (attr[0] == 's' && !strcmp(attr, "send_event")) {
	INT_CHECK();
	event->any.send_event = PyInt_AsLong(value);
	return 0;
    }
	
    switch(event->type) {
    case GDK_NOTHING: break;
    case GDK_DELETE: break;
    case GDK_DESTROY: break;
    case GDK_EXPOSE:            /*GdkEventExpose            expose*/
        if (!strcmp(attr, "area")) {
	    if (!pygdk_rectangle_from_pyobject(value, &event->expose.area))
		return -1;
            return 0;
	} else if (!strcmp(attr, "count")) {
	    INT_CHECK();
	    event->expose.count = PyInt_AsLong(value);
            return 0;
	}
        break;
    case GDK_MOTION_NOTIFY:     /*GdkEventMotion            motion*/
	if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->motion.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (attr[0] == 'x' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->motion.x = PyFloat_AsDouble(value);
	    return 0;
	} else if (attr[0] == 'y' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->motion.y = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "axes")) {
	    PyErr_SetString(PyExc_TypeError, "axes is not writable");
	    return -1;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->motion.state = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "is_hint")) {
	    INT_CHECK();
	    event->motion.is_hint = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "device")) {
	    PyErr_SetString(PyExc_TypeError, "device is not writable");
	    return -1;
	} else if (!strcmp(attr, "x_root")) {
	    FLOAT_CHECK()
	    event->motion.x_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "y_root")) {
	    FLOAT_CHECK()
	    event->motion.y_root = PyFloat_AsDouble(value);
	    return 0;
	}
        break;
    case GDK_BUTTON_PRESS:      /*GdkEventButton            button*/
    case GDK_2BUTTON_PRESS:     /*GdkEventButton            button*/
    case GDK_3BUTTON_PRESS:     /*GdkEventButton            button*/
    case GDK_BUTTON_RELEASE:    /*GdkEventButton            button*/
	if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->button.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (attr[0] == 'x' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->button.x = PyFloat_AsDouble(value);
	    return 0;
	} else if (attr[0] == 'y' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->button.y = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "axes")) {
	    PyErr_SetString(PyExc_TypeError, "axes is not writable");
	    return -1;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->button.state = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "button")) {
	    INT_CHECK();
	    event->button.button = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "device")) {
	    PyErr_SetString(PyExc_TypeError, "device is not writable");
	    return -1;
	} else if (!strcmp(attr, "x_root")) {
	    FLOAT_CHECK();
	    event->button.x_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "y_root")) {
	    FLOAT_CHECK();
	    event->button.y_root = PyFloat_AsDouble(value);
	    return 0;
	}
	break;
    case GDK_KEY_PRESS:         /*GdkEventKey               key*/
    case GDK_KEY_RELEASE:       /*GdkEventKey               key*/
	if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->key.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->key.state = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "keyval")) {
	    INT_CHECK();
	    event->key.keyval = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "string")) {
	    STRING_CHECK();
            g_free(event->key.string);
	    event->key.string = g_strdup(PyString_AsString(value));
	    return 0;
	} else if (!strcmp(attr, "hardware_keycode")) {
	    INT_CHECK();
	    event->key.hardware_keycode = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "group")) {
	    INT_CHECK();
	    event->key.group = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "is_modifier")) {
	    INT_CHECK();
	    event->key.is_modifier = PyInt_AsLong(value);
	    return 0;
	}
        break;
    case GDK_ENTER_NOTIFY:      /*GdkEventCrossing          crossing*/
    case GDK_LEAVE_NOTIFY:      /*GdkEventCrossing          crossing*/
        if (!strcmp(attr, "subwindow")) {
          if (!pygobject_check(value, &PyGdkWindow_Type)) {
              PyErr_SetString(PyExc_TypeError, "subwindow must be a GdkWindow");
              return -1;
          }
          if (event->crossing.subwindow)
              g_object_unref(event->crossing.subwindow);
          event->crossing.subwindow = g_object_ref(GDK_WINDOW(((PyGObject*)value)->obj));
          return 0;
	} else if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->crossing.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (attr[0] == 'x' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->crossing.x = PyFloat_AsDouble(value);
	    return 0;
	} else if (attr[0] == 'y' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->crossing.y = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "x_root")) {
	    FLOAT_CHECK();
	    event->crossing.x_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "y_root")) {
	    FLOAT_CHECK();
	    event->crossing.y_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "mode")) {
	    INT_CHECK();
	    event->crossing.mode = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "detail")) {
	    INT_CHECK();
	    event->crossing.detail = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "focus")) {
	    INT_CHECK();
	    event->crossing.focus = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->crossing.state = PyInt_AsLong(value);
	    return 0;
	}
        break;	
    case GDK_FOCUS_CHANGE:      /*GdkEventFocus             focus_change*/
	if (!strcmp(attr, "in_")) {
	    INT_CHECK();
	    event->focus_change.in = PyInt_AsLong(value);
	    return 0;
	}
	break;
    case GDK_CONFIGURE:
	if (!strcmp(attr, "x")) {
	    INT_CHECK();
	    event->configure.x = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "y")) {
	    INT_CHECK();
	    event->configure.y = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "width")) {
	    INT_CHECK();
	    event->configure.width = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "height")) {
	    INT_CHECK();
	    event->configure.height = PyInt_AsLong(value);
	    return 0;
	}
    case GDK_MAP: break;
    case GDK_UNMAP: break;
    case GDK_PROPERTY_NOTIFY:   /*GdkEventProperty          property*/
        if (!strcmp(attr, "atom")) {
	    PyErr_SetString(PyExc_TypeError, "atom is not writable");
	    return -1;
	} else if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->property.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->property.state = PyInt_AsLong(value);
	    return 0;
	}
        break;	
    case GDK_SELECTION_CLEAR:   /*GdkEventSelection         selection*/
    case GDK_SELECTION_REQUEST: /*GdkEventSelection         selection*/
    case GDK_SELECTION_NOTIFY:  /*GdkEventSelection         selection*/
        if (!strcmp(attr, "selection")) {
	    PyErr_SetString(PyExc_TypeError, "selection is not writable");
	    return -1;
	} else if (!strcmp(attr, "target")) {
	    PyErr_SetString(PyExc_TypeError, "target is not writable");
	    return -1;
	} else if (!strcmp(attr, "property")) {
	    PyErr_SetString(PyExc_TypeError, "property is not writable");
	    return -1;
	} else if (!strcmp(attr, "requestor")) {
	    INT_CHECK();
	    event->selection.requestor = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->selection.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	}
	
	break;
    case GDK_PROXIMITY_IN:      /*GdkEventProximity         proximity*/
    case GDK_PROXIMITY_OUT:     /*GdkEventProximity         proximity*/
	if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->proximity.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (!strcmp(attr, "device")) {
	    PyErr_SetString(PyExc_TypeError, "device is not writable");
	    return -1;
	}
	break;
    case GDK_DRAG_ENTER:        /*GdkEventDND               dnd*/
    case GDK_DRAG_LEAVE:        /*GdkEventDND               dnd*/
    case GDK_DRAG_MOTION:       /*GdkEventDND               dnd*/
    case GDK_DRAG_STATUS:       /*GdkEventDND               dnd*/
    case GDK_DROP_START:        /*GdkEventDND               dnd*/
    case GDK_DROP_FINISHED:     /*GdkEventDND               dnd*/
	if (!strcmp(attr, "context")) {
	    PyErr_SetString(PyExc_TypeError, "context is not writable");
	    return -1;
	} else if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->dnd.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (!strcmp(attr, "x_root")) {
	    FLOAT_CHECK();
	    event->dnd.x_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "y_root")) {
	    FLOAT_CHECK();
	    event->dnd.y_root = PyFloat_AsDouble(value);
	    return 0;
	}
	break;
    case GDK_CLIENT_EVENT:      /*GdkEventClient            client*/
        if (!strcmp(attr, "message_type")) {
            GdkAtom message_type = pygdk_atom_from_pyobject(value);
            if (PyErr_Occurred())
                return -1;
            event->client.message_type = message_type;
	    return 0;
	} else if (!strcmp(attr, "data_format")) {
	    INT_CHECK();
	    event->client.data_format = PyInt_AsLong(value);
            return 0;
	} else if (!strcmp(attr, "data")) {
	    char *tmp;
	    STRING_CHECK();
	    tmp = PyString_AsString(value);
	    strncpy((char *) &event->client.data, tmp,
                    sizeof(event->client.data));
	    return 0;
	}
	break;
    case GDK_VISIBILITY_NOTIFY: /*GdkEventVisibility        visibility*/
	if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->visibility.state = PyInt_AsLong(value);
	    return 0;
	}
	break;
    case GDK_NO_EXPOSE:         /*GdkEventNoExpose          no_expose*/
        break;
    case GDK_SCROLL:            /*GdkEventScroll            scroll*/
	if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->scroll.time = PyInt_AsUnsignedLongMask(value);
	    return 0;
	} else if (attr[0] == 'x' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->scroll.x = PyFloat_AsDouble(value);
	    return 0;
	} else if (attr[0] == 'y' && attr[1] == '\0') {
	    FLOAT_CHECK();
	    event->scroll.y = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "state")) {
	    INT_CHECK();
	    event->scroll.state = PyInt_AsLong(value);
	    return 0;
	} else if (!strcmp(attr, "direction")) {
	    INT_CHECK(); /* XXX: ENUM */
	    event->scroll.direction = PyInt_AsLong(value);
            return 0;
	} else if (!strcmp(attr, "device")) {
	    PyErr_SetString(PyExc_TypeError, "device is not writable");
	    return -1;
	} else if (!strcmp(attr, "x_root")) {
	    FLOAT_CHECK();
	    event->scroll.x_root = PyFloat_AsDouble(value);
	    return 0;
	} else if (!strcmp(attr, "y_root")) {
	    FLOAT_CHECK();
	    event->scroll.y_root = PyFloat_AsDouble(value);
	    return 0;
	}
	break;
    case GDK_WINDOW_STATE:      /*GdkEventWindowState       window_state*/
	if (!strcmp(attr, "changed_mask")) {
	    INT_CHECK();
	    event->window_state.changed_mask = PyInt_AsLong(value);
            return 0;
	} else if (!strcmp(attr, "new_window_state")) {
	    INT_CHECK();
	    event->window_state.new_window_state = PyInt_AsLong(value);
            return 0;
	}
	break;
    case GDK_SETTING:           /*GdkEventSetting           setting*/
        if (!strcmp(attr, "action")) {
	    INT_CHECK();
	    event->setting.action = PyInt_AsLong(value);
            return 0;
	} else if (!strcmp(attr, "name")) {
	    STRING_CHECK();
	    event->setting.name = PyString_AsString(value);
	    return 0;
	}
	break;
    case GDK_OWNER_CHANGE:
        if (!strcmp(attr, "owner")) {
	    INT_CHECK();
	    event->owner_change.owner = PyInt_AsLong(value);
	} else if (!strcmp(attr, "reason")) {
	    INT_CHECK();
	    event->owner_change.reason = PyInt_AsLong(value);
	} else if (!strcmp(attr, "selection")) {
	    PyErr_SetString(PyExc_TypeError, "selection is not writable");
	    return -1;
	} else if (!strcmp(attr, "time")) {
	    INT_OR_LONG_CHECK();
	    event->owner_change.time = PyInt_AsUnsignedLongMask(value);
	} else if (!strcmp(attr, "selection_time")) {
	    INT_CHECK();
	    event->owner_change.selection_time = PyInt_AsLong(value);
	}
	break;
    case GDK_GRAB_BROKEN:
        if (!strcmp(attr, "keyboard")) {
	    INT_CHECK();
	    event->grab_broken.keyboard = PyInt_AsLong(value);
	} else if (!strcmp(attr, "implicit")) {
	    INT_CHECK();
	    event->grab_broken.keyboard = PyInt_AsLong(value);
	} else if (!strcmp(attr, "grab_window")) {
	    PyErr_SetString(PyExc_TypeError, "grab_window is not writable");
	    return -1;
	}
        break;
    default:
	break;
    }
    
    PyErr_SetString(PyExc_AttributeError, "could not write attribute");
    return -1;
}
#undef INT_CHECK
#undef INT_OR_LONG_CHECK
#undef FLOAT_CHECK
#undef STRING_CHECK	
#line 1399 "gdk.c"


#line 929 "gdkevent.override"
static PyObject *
_wrap_gdk_event_tp_repr(PyGObject *self)
{
    static char buffer[0x400];
    int length = 0;

    GdkEvent *event = pyg_boxed_get(self, GdkEvent);
    GEnumValue *type = g_enum_get_value(g_type_class_peek(GDK_TYPE_EVENT_TYPE), event->type);

    length += g_snprintf(buffer + length, sizeof buffer - length, "<%s at %p: %s",
                         self->ob_type->tp_name, self, type ? type->value_name : "UNKNOWN TYPE");

    /* Depending on event type we will append values of some most important attributes to
     * representation string.  In any case, this information is not enough to reconstruct
     * the event object (and in any case gtk.gdk.Event is incapable of doing so),
     * therefore the <...> syntax.
     */
    switch(event->type) {
    case GDK_NOTHING: break;
    case GDK_DELETE: break;
    case GDK_DESTROY: break;
    case GDK_EXPOSE:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length, " area=[%d, %d, %d, %d]",
                                 event->expose.area.x, event->expose.area.y,
                                 event->expose.area.width, event->expose.area.height);
            break;
        }
    case GDK_MOTION_NOTIFY:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length, " x=%.2f, y=%.2f",
                                 event->motion.x, event->motion.y);
            break;
        }
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " x=%.2f, y=%.2f, button=%d",
                                 event->button.x, event->button.y, event->button.button);
            break;
        }
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
        {
            const gchar *key = gdk_keyval_name(event->key.keyval);

            if (key)
                length += g_snprintf(buffer + length, sizeof buffer - length, " keyval=%s", key);
            else
                length += g_snprintf(buffer + length, sizeof buffer - length,
                                     " keyval=%d", event->key.keyval);
            break;
        }
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
        {
            GEnumValue *mode = g_enum_get_value(g_type_class_peek(GDK_TYPE_CROSSING_MODE),
                                                event->crossing.mode);
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " x=%.2f, y=%.2f, mode=%s",
                                 event->crossing.x, event->crossing.y,
                                 mode ? mode->value_name : "UNKNOWN");
            break;
        }
    case GDK_FOCUS_CHANGE: break;
    case GDK_CONFIGURE:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " x=%d, y=%d, width=%d, height=%d",
                                 event->configure.x, event->configure.y,
                                 event->configure.width, event->configure.height);
            break;
        }
    case GDK_MAP: break;
    case GDK_UNMAP: break;
    case GDK_PROPERTY_NOTIFY:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length, " atom=%s",
                                 gdk_atom_name(event->property.atom));
            break;
        }
    case GDK_SELECTION_CLEAR:
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
        {
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " selection=%s, target=%s, property=%s",
                                 gdk_atom_name(event->selection.selection),
                                 gdk_atom_name(event->selection.target),
                                 gdk_atom_name(event->selection.property));
            break;
        }
    case GDK_PROXIMITY_IN: break;
    case GDK_PROXIMITY_OUT: break;
    case GDK_DRAG_ENTER: break;
    case GDK_DRAG_LEAVE: break;
    case GDK_DRAG_MOTION: break;
    case GDK_DRAG_STATUS: break;
    case GDK_DROP_START: break;
    case GDK_DROP_FINISHED: break;
    case GDK_CLIENT_EVENT: break;
    case GDK_VISIBILITY_NOTIFY:
        {
            GEnumValue *state = g_enum_get_value(g_type_class_peek(GDK_TYPE_VISIBILITY_STATE),
                                                event->visibility.state);
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " state=%s", state ? state->value_name : "UNKNOWN");
            break;
        }
    case GDK_NO_EXPOSE: break;
    case GDK_SCROLL:
        {
            GEnumValue *direction = g_enum_get_value(g_type_class_peek(GDK_TYPE_SCROLL_DIRECTION),
                                                     event->scroll.direction);
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " x=%.2f, y=%.2f, direction=%s",
                                 event->scroll.x, event->scroll.y,
                                 direction ? direction->value_name : "UNKNOWN");
            break;
        }
    case GDK_WINDOW_STATE: break;
    case GDK_SETTING:
        {
            GEnumValue *action = g_enum_get_value(g_type_class_peek(GDK_TYPE_SETTING_ACTION),
                                                  event->setting.action);
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " action=%s, name=%s",
                                 action ? action->value_name : "UNKNOWN",
                                 event->setting.name);
            break;
        }
    case GDK_OWNER_CHANGE:
        {
            GEnumValue *reason = g_enum_get_value(g_type_class_peek(GDK_TYPE_OWNER_CHANGE),
                                                  event->owner_change.reason);
            length += g_snprintf(buffer + length, sizeof buffer - length,
                                 " reason=%s, selection=%s",
                                 reason ? reason->value_name : "UNKNOWN",
                                 gdk_atom_name(event->owner_change.selection));
            break;
        }
    default:
        break;
    }

    length += g_snprintf(buffer + length, sizeof buffer - length, ">");
    return PyString_FromStringAndSize(buffer, length);
}
#line 1554 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkEvent_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Event",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)_wrap_gdk_event_tp_getattr,       /* tp_getattr */
    (setattrfunc)_wrap_gdk_event_tp_setattr,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gdk_event_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkEvent_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_event_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkFont ----------- */

static int
_wrap_gdk_font_load(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font_name", NULL };
    char *font_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Font.__init__", kwlist, &font_name))
        return -1;
    self->gtype = GDK_TYPE_FONT;
    self->free_on_dealloc = FALSE;
    self->boxed = gdk_font_load(font_name);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkFont object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_gdk_font_equal(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fontb", NULL };
    GdkFont *fontb = NULL;
    PyObject *py_fontb;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Font.equal", kwlist, &py_fontb))
        return NULL;
    if (pyg_boxed_check(py_fontb, GDK_TYPE_FONT))
        fontb = pyg_boxed_get(py_fontb, GdkFont);
    else {
        PyErr_SetString(PyExc_TypeError, "fontb should be a GdkFont");
        return NULL;
    }
    
    ret = gdk_font_equal(pyg_boxed_get(self, GdkFont), fontb);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_font_id(PyObject *self)
{
    int ret;

    
    ret = gdk_font_id(pyg_boxed_get(self, GdkFont));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_string_width(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Font.string_width", kwlist, &string))
        return NULL;
    
    ret = gdk_string_width(pyg_boxed_get(self, GdkFont), string);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_text_width(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", "text_length", NULL };
    char *text;
    int text_length = -1, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|i:Gdk.Font.width", kwlist, &text, &text_length))
        return NULL;
    
    ret = gdk_text_width(pyg_boxed_get(self, GdkFont), text, text_length);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_char_width(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "character", NULL };
    char character;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"c:Gdk.Font.char_width", kwlist, &character))
        return NULL;
    
    ret = gdk_char_width(pyg_boxed_get(self, GdkFont), character);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_string_measure(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Font.string_measure", kwlist, &string))
        return NULL;
    
    ret = gdk_string_measure(pyg_boxed_get(self, GdkFont), string);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_text_measure(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", "text_length", NULL };
    char *text;
    int text_length = -1, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|i:Gdk.Font.measure", kwlist, &text, &text_length))
        return NULL;
    
    ret = gdk_text_measure(pyg_boxed_get(self, GdkFont), text, text_length);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_char_measure(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "character", NULL };
    char character;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"c:Gdk.Font.char_measure", kwlist, &character))
        return NULL;
    
    ret = gdk_char_measure(pyg_boxed_get(self, GdkFont), character);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_string_height(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Font.string_height", kwlist, &string))
        return NULL;
    
    ret = gdk_string_height(pyg_boxed_get(self, GdkFont), string);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_text_height(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", "text_length", NULL };
    char *text;
    int text_length = -1, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|i:Gdk.Font.height", kwlist, &text, &text_length))
        return NULL;
    
    ret = gdk_text_height(pyg_boxed_get(self, GdkFont), text, text_length);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_char_height(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "character", NULL };
    char character;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"c:Gdk.Font.char_height", kwlist, &character))
        return NULL;
    
    ret = gdk_char_height(pyg_boxed_get(self, GdkFont), character);
    
    return PyInt_FromLong(ret);
}

#line 162 "./gdk.override"
static PyObject *
_wrap_gdk_text_extents(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", NULL };
    gchar *text;
    Py_ssize_t length;
    gint lbearing, rbearing, width, ascent, descent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#:GdkFont.extents",
                                     kwlist, &text, &length))
        return NULL;
    gdk_text_extents(pyg_boxed_get(self, GdkFont), text, length,
                     &lbearing, &rbearing, &width, &ascent, &descent);
    return Py_BuildValue("(iiiii)", lbearing, rbearing, width,
                         ascent, descent);
}
#line 1812 "gdk.c"


static PyObject *
_wrap_gdk_font_get_display(PyObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_font_get_display(pyg_boxed_get(self, GdkFont));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkFont_methods[] = {
    { "equal", (PyCFunction)_wrap_gdk_font_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "id", (PyCFunction)_wrap_gdk_font_id, METH_NOARGS,
      NULL },
    { "string_width", (PyCFunction)_wrap_gdk_string_width, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "width", (PyCFunction)_wrap_gdk_text_width, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "char_width", (PyCFunction)_wrap_gdk_char_width, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "string_measure", (PyCFunction)_wrap_gdk_string_measure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "measure", (PyCFunction)_wrap_gdk_text_measure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "char_measure", (PyCFunction)_wrap_gdk_char_measure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "string_height", (PyCFunction)_wrap_gdk_string_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "height", (PyCFunction)_wrap_gdk_text_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "char_height", (PyCFunction)_wrap_gdk_char_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "extents", (PyCFunction)_wrap_gdk_text_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_display", (PyCFunction)_wrap_gdk_font_get_display, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gdk_font__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_boxed_get(self, GdkFont)->type;
    return pyg_enum_from_gtype(GDK_TYPE_FONT_TYPE, ret);
}

static PyObject *
_wrap_gdk_font__get_ascent(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkFont)->ascent;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_font__get_descent(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkFont)->descent;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gdk_font_getsets[] = {
    { "type", (getter)_wrap_gdk_font__get_type, (setter)0 },
    { "ascent", (getter)_wrap_gdk_font__get_ascent, (setter)0 },
    { "descent", (getter)_wrap_gdk_font__get_descent, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkFont_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Font",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkFont_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_font_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_font_load,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkColor ----------- */

#line 28 "gdkcolor.override"
static int
_wrap_gdk_color_new(PyGBoxed *self,
                    PyObject *args,
                    PyObject *kwargs)
{
    static char *kwlist1[] = {"red", "green", "blue", "pixel", NULL };
    static char *kwlist2[] = { "spec", NULL };
    PyObject *red = Py_None, *green = Py_None, *blue = Py_None;
    const char *spec = NULL;
    GdkColor colour;

    /* Note: this constructor has become quite complicated, because it
     * is heavily overloaded.  Additionally, we try to optimize a
     * little. */

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "|s:gdk.Color", kwlist2,
                                    &spec)) {
        if (!spec)
            memset(&colour, 0, sizeof(colour));
        else if (!gdk_color_parse(spec, &colour)) {
            PyErr_SetString(PyExc_ValueError,
                            "unable to parse colour specification");
            return -1;
        }

        goto success;
    }

    PyErr_Clear();

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "|OOOk:gdk.Color", kwlist1,
                                    &red, &green, &blue, &colour.pixel)) {
        /* We don't allow mixing floats and non-floats as that is too
         * error-prone.  All non-floats are deemed integers in case
         * they have __int__() method. */
        int have_floats = 0;
        int have_nonfloats = 0;

        if (red == Py_None)
            colour.red = 0;
        else {
            if (PyFloat_Check(red)) {
                have_floats = 1;
                colour.red = MIN(MAX(0.0, PyFloat_AsDouble(red)), 1.0) * 65535.0;
            }
            else {
                have_nonfloats = 1;
                colour.red = PyInt_AsLong(red);
            }
        }

        if (PyErr_Occurred())
            return -1;

        if (green == Py_None)
            colour.green = 0;
        else {
            if (PyFloat_Check(green)) {
                if (have_nonfloats)
                    goto mixed_types_error;
                have_floats = 1;
                colour.green = MIN(MAX(0.0, PyFloat_AsDouble(green)), 1.0) * 65535.0;
            }
            else {
                if (have_floats)
                    goto mixed_types_error;
                have_nonfloats = 1;
                colour.green = PyInt_AsLong(green);
            }
        }

        if (PyErr_Occurred())
            return -1;

        if (blue == Py_None)
            colour.blue = 0;
        else {
            if (PyFloat_Check(blue)) {
                if (have_nonfloats)
                    goto mixed_types_error;
                colour.blue = MIN(MAX(0.0, PyFloat_AsDouble(blue)), 1.0) * 65535.0;
            }
            else {
                if (have_floats)
                    goto mixed_types_error;
                colour.blue = PyInt_AsLong(blue);
            }
        }

        if (PyErr_Occurred())
            return -1;

        goto success;

    mixed_types_error:
        PyErr_SetString(PyExc_TypeError, "arguments must either be all integers or all floats");
        return -1;
    }

    PyErr_Clear();
    PyErr_SetString(PyExc_TypeError, "Usage:\n"
                    "  gtk.gdk.Color(red, green, blue, pixel)  [all are optional]\n"
                    "  gtk.gdk.Color(spec)                     [see gtk.gdk.color_parse()]");
    return -1;

 success:
    self->boxed =  g_boxed_copy(GDK_TYPE_COLOR, &colour);
    self->free_on_dealloc = TRUE;
    self->gtype = GDK_TYPE_COLOR;
    
    return 0;
}

#line 2054 "gdk.c"


static PyObject *
_wrap_gdk_color_to_string(PyObject *self)
{
    gchar *ret;

    
    ret = gdk_color_to_string(pyg_boxed_get(self, GdkColor));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkColor_methods[] = {
    { "to_string", (PyCFunction)_wrap_gdk_color_to_string, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 220 "gdkcolor.override"
static int
_wrap_gdk_color__set_pixel(PyObject *self, PyObject *value, void *closure)
{
    long pixel = PyInt_AsLong(value);
    if (pixel == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->pixel = pixel;
        return 0;
    }
}
#line 2092 "gdk.c"


static PyObject *
_wrap_gdk_color__get_pixel(PyObject *self, void *closure)
{
    guint32 ret;

    ret = pyg_boxed_get(self, GdkColor)->pixel;
    return PyLong_FromUnsignedLong(ret);

}

#line 181 "gdkcolor.override"
static int
_wrap_gdk_color__set_red(PyObject *self, PyObject *value, void *closure)
{
    long red = PyInt_AsLong(value);
    if (red == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->red = red;
        return 0;
    }
}
#line 2117 "gdk.c"


static PyObject *
_wrap_gdk_color__get_red(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkColor)->red;
    return PyInt_FromLong(ret);
}

#line 207 "gdkcolor.override"
static int
_wrap_gdk_color__set_green(PyObject *self, PyObject *value, void *closure)
{
    long green = PyInt_AsLong(value);
    if (green == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->green = green;
        return 0;
    }
}
#line 2141 "gdk.c"


static PyObject *
_wrap_gdk_color__get_green(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkColor)->green;
    return PyInt_FromLong(ret);
}

#line 194 "gdkcolor.override"
static int
_wrap_gdk_color__set_blue(PyObject *self, PyObject *value, void *closure)
{
    long blue = PyInt_AsLong(value);
    if (blue == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->blue = blue;
        return 0;
    }
}
#line 2165 "gdk.c"


static PyObject *
_wrap_gdk_color__get_blue(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkColor)->blue;
    return PyInt_FromLong(ret);
}

#line 233 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_red_float(PyObject *self, void *closure)
{
    return PyFloat_FromDouble(pyg_boxed_get(self, GdkColor)->red / 65535.0);
}

static int
_wrap_gdk_color__set_red_float(PyObject *self, PyObject *value, void *closure)
{
    double red = PyFloat_AsDouble(value);
    if (red == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->red = MIN(MAX(0.0, red), 1.0) * 65535.0;
        return 0;
    }
}
#line 2196 "gdk.c"


#line 253 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_green_float(PyObject *self, void *closure)
{
    return PyFloat_FromDouble(pyg_boxed_get(self, GdkColor)->green / 65535.0);
}

static int
_wrap_gdk_color__set_green_float(PyObject *self, PyObject *value, void *closure)
{
    double green = PyFloat_AsDouble(value);
    if (green == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->green = MIN(MAX(0.0, green), 1.0) * 65535.0;
        return 0;
    }
}
#line 2218 "gdk.c"


#line 273 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_blue_float(PyObject *self, void *closure)
{
    return PyFloat_FromDouble(pyg_boxed_get(self, GdkColor)->blue / 65535.0);
}

static int
_wrap_gdk_color__set_blue_float(PyObject *self, PyObject *value, void *closure)
{
    double blue = PyFloat_AsDouble(value);
    if (blue == -1 && PyErr_Occurred())
        return -1;
    else {
        pyg_boxed_get(self, GdkColor)->blue = MIN(MAX(0.0, blue), 1.0) * 65535.0;
        return 0;
    }
}
#line 2240 "gdk.c"


#line 293 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_hue(PyObject *self, void *closure)
{
    GdkColor *color = pyg_boxed_get(self, GdkColor);
    gdouble red = color->red / 65535.0;
    gdouble green = color->green / 65535.0;
    gdouble blue = color->blue / 65535.0;
    gdouble hue;

    gtk_rgb_to_hsv(red, green, blue, &hue, NULL, NULL);
    return PyFloat_FromDouble(hue);
}
#line 2257 "gdk.c"


#line 308 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_saturation(PyObject *self, void *closure)
{
    GdkColor *color = pyg_boxed_get(self, GdkColor);
    gdouble red = color->red / 65535.0;
    gdouble green = color->green / 65535.0;
    gdouble blue = color->blue / 65535.0;
    gdouble saturation;

    gtk_rgb_to_hsv(red, green, blue, NULL, &saturation, NULL);
    return PyFloat_FromDouble(saturation);
}
#line 2274 "gdk.c"


#line 323 "gdkcolor.override"

static PyObject *
_wrap_gdk_color__get_value(PyObject *self, void *closure)
{
    GdkColor *color = pyg_boxed_get(self, GdkColor);
    gdouble red = color->red / 65535.0;
    gdouble green = color->green / 65535.0;
    gdouble blue = color->blue / 65535.0;
    gdouble value;

    gtk_rgb_to_hsv(red, green, blue, NULL, NULL, &value);
    return PyFloat_FromDouble(value);
}
#line 2291 "gdk.c"


static const PyGetSetDef gdk_color_getsets[] = {
    { "pixel", (getter)_wrap_gdk_color__get_pixel, (setter)_wrap_gdk_color__set_pixel },
    { "red", (getter)_wrap_gdk_color__get_red, (setter)_wrap_gdk_color__set_red },
    { "green", (getter)_wrap_gdk_color__get_green, (setter)_wrap_gdk_color__set_green },
    { "blue", (getter)_wrap_gdk_color__get_blue, (setter)_wrap_gdk_color__set_blue },
    { "red_float", (getter)_wrap_gdk_color__get_red_float, (setter)_wrap_gdk_color__set_red_float },
    { "green_float", (getter)_wrap_gdk_color__get_green_float, (setter)_wrap_gdk_color__set_green_float },
    { "blue_float", (getter)_wrap_gdk_color__get_blue_float, (setter)_wrap_gdk_color__set_blue_float },
    { "hue", (getter)_wrap_gdk_color__get_hue, (setter)0 },
    { "saturation", (getter)_wrap_gdk_color__get_saturation, (setter)0 },
    { "value", (getter)_wrap_gdk_color__get_value, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 456 "gdkcolor.override"
static int
pygdk_color_to_string_smart(char *buffer, int length, GdkColor *color)
{
    /* We use g_snprintf() because PyString_FromFormat() doesn't support %0Nx-like formats
     * (i.e. with leading zeros).
     *
     * Note that numbers here are used so that there is no off-by-one errors in
     * 'eval(repr(color))', i.e. so that 'eval(repr(color)) == color'.  See
     * pango_color_parse() for justification of these numbers.  Three-nibble color
     * components are deemed unimportant.
     */
    if (color->red % 0x1111 == 0 && color->green % 0x1111 == 0 && color->blue % 0x1111 == 0) {
        return g_snprintf(buffer, length, "#%01x%01x%01x",
                          color->red / 0x1111, color->green / 0x1111, color->blue / 0x1111);
    }
    else if (color->red % 0x0101 == 0 && color->green % 0x0101 == 0 && color->blue % 0x0101 == 0) {
        return g_snprintf(buffer, length, "#%02x%02x%02x",
                          color->red / 0x0101, color->green / 0x0101, color->blue / 0x0101);
    }
    else {
        return g_snprintf(buffer, length, "#%04x%04x%04x",
                          color->red, color->green, color->blue);
    }
}
static PyObject *
_wrap_gdk_color_tp_repr(PyGBoxed *self)
{
    static char buffer[0x40];
    int length = 0;

    length += g_snprintf(buffer + length, sizeof buffer - length, "%s('", self->ob_type->tp_name);
    length += pygdk_color_to_string_smart(buffer + length, sizeof buffer - length,
                                          pyg_boxed_get(self, GdkColor));
    length += g_snprintf(buffer + length, sizeof buffer - length, "')");

    return PyString_FromStringAndSize(buffer, length);
}
#line 2346 "gdk.c"


#line 495 "gdkcolor.override"
static PyObject *
_wrap_gdk_color_tp_str(PyGBoxed *self)
{
    /* gtk.gdk.Color has a meaningful informal representation, so we define both __repr__
     * and __str__, unlike for most other types.
     */
    static char buffer[1 + 4*3 + 1];  /* # + at most 4 digits per component + \0 */
    int length = pygdk_color_to_string_smart(buffer, sizeof buffer, pyg_boxed_get(self, GdkColor));

    return PyString_FromStringAndSize(buffer, length);
}
#line 2361 "gdk.c"


#line 425 "gdkcolor.override"
static PyObject *
_wrap_gdk_color_tp_richcompare(PyObject *self, PyObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGdkColor_Type)
        && PyObject_TypeCheck(other, &PyGdkColor_Type)) {
        GdkColor *color1 = pyg_boxed_get(self, GdkColor);
        GdkColor *color2 = pyg_boxed_get(other, GdkColor);

        switch (op) {
        case Py_EQ:
            result = (gdk_color_equal(color1, color2)
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (!gdk_color_equal(color1, color2)
                      ? Py_True : Py_False);
            break;
        default:
            result = Py_NotImplemented;
        }
    }
    else
        result = Py_NotImplemented;

    Py_INCREF(result);
    return result;
}
#line 2394 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkColor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Color",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gdk_color_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_gdk_color_tp_str,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_gdk_color_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkColor_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_color_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_color_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkCursor ----------- */

#line 395 "./gdk.override"
static int
_wrap_gdk_cursor_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist1[] = { "cursor_type", NULL };
    static char *kwlist2[] = { "display", "cursor_type", NULL };
    static char *kwlist3[] = { "display", "pixbuf", "x", "y", NULL };
    static char *kwlist4[] = { "source", "mask", "fg", "bg", "x", "y", NULL };
    PyObject *py_cursor_type;
    PyGObject *py_display;
    PyGObject *source, *mask, *pixbuf;
    PyObject *fg, *bg;
    gint x, y;

    self->gtype = GDK_TYPE_CURSOR;
    self->free_on_dealloc = FALSE;
    self->boxed = NULL;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkCursor.__init__",
				     kwlist1, &py_cursor_type)) {
	GdkCursorType cursor_type;

        if (pyg_enum_get_value(GDK_TYPE_CURSOR_TYPE, py_cursor_type,
                               (gint *)&cursor_type))
            return -1;
        self->boxed = gdk_cursor_new(cursor_type);
	goto done;
    }

    PyErr_Clear();

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "OO:GdkCursor.__init__",
				     kwlist2, &py_display, &py_cursor_type)) {
	
	GdkCursorType cursor_type;

	if (!pygobject_check(py_display, &PyGdkDisplay_Type)) {
	    PyErr_SetString(PyExc_TypeError,
			    "display should be a GdkDisplay");
	    return -1;
	}
        if (pyg_enum_get_value(GDK_TYPE_CURSOR_TYPE, py_cursor_type,
                               (gint *)&cursor_type))
            return -1;
        self->boxed = gdk_cursor_new_for_display(GDK_DISPLAY(py_display->obj),
						 cursor_type);
	goto done;
    }

    PyErr_Clear();

    if (PyArg_ParseTupleAndKeywords(args, kwargs,
				    "OOii:GdkCursor.__init__", kwlist3,
				    &py_display, &pixbuf, &x, &y)) {

	if (!pygobject_check(py_display, &PyGdkDisplay_Type)) {
	    PyErr_SetString(PyExc_TypeError,
			    "display should be a GdkDisplay");
	    return -1;
	}
	if (!pygobject_check(pixbuf, &PyGdkPixbuf_Type)) {
	    PyErr_SetString(PyExc_TypeError,
			    "pixbuf should be a GdkPixbuf");
	    return -1;
	}
	self->boxed = gdk_cursor_new_from_pixbuf(GDK_DISPLAY(py_display->obj),
						 GDK_PIXBUF(pixbuf->obj),
						 x, y);
	goto done;
     }

    PyErr_Clear();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				    "OOOOii:GdkCursor.__init__", kwlist4,
				     &source, &mask, &fg, &bg, &x, &y)) {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "Usage:\n"
			"  gtk.gdk.Cursor(cursor_type)\n"
			"  gtk.gdk.Cursor(display, cursor_type)\n"
			"  gtk.gdk.Cursor(display, pixbuf, x, y)\n"
			"  gtk.gdk.Cursor(source, mask, fg, bg, x, y)");
	return -1;
    }

    if (!pygobject_check(source, &PyGdkPixmap_Type)) {
	PyErr_SetString(PyExc_TypeError, "source should be a GdkPixmap");
	return -1;
    }
    if (!pygobject_check(mask, &PyGdkPixmap_Type)) {
	PyErr_SetString(PyExc_TypeError, "mask should be a GdkPixmap");
	return -1;
    }
    if (!pyg_boxed_check(fg, GDK_TYPE_COLOR)) {
	PyErr_SetString(PyExc_TypeError, "fg should be a GdkColor");
	return -1;
    }
    if (!pyg_boxed_check(bg, GDK_TYPE_COLOR)) {
	PyErr_SetString(PyExc_TypeError, "bg should be a GdkColor");
	return -1;
    }
    self->boxed = gdk_cursor_new_from_pixmap(GDK_PIXMAP(source->obj),
					     GDK_PIXMAP(mask->obj),
					     pyg_boxed_get(fg, GdkColor),
					     pyg_boxed_get(bg, GdkColor),
					     x, y);
 done:
    if (!self->boxed) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not create GdkCursor object");
	return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}
#line 2561 "gdk.c"


static PyObject *
_wrap_gdk_cursor_get_image(PyObject *self)
{
    GdkPixbuf *ret;

    
    ret = gdk_cursor_get_image(pyg_boxed_get(self, GdkCursor));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_cursor_get_display(PyObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_cursor_get_display(pyg_boxed_get(self, GdkCursor));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkCursor_methods[] = {
    { "get_image", (PyCFunction)_wrap_gdk_cursor_get_image, METH_NOARGS,
      NULL },
    { "get_display", (PyCFunction)_wrap_gdk_cursor_get_display, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gdk_cursor__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_boxed_get(self, GdkCursor)->type;
    return pyg_enum_from_gtype(GDK_TYPE_CURSOR_TYPE, ret);
}

static const PyGetSetDef gdk_cursor_getsets[] = {
    { "type", (getter)_wrap_gdk_cursor__get_type, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 511 "./gdk.override"
static PyObject *
_wrap_gdk_cursor_tp_repr(PyGObject *self)
{
    GdkCursor *cursor = pyg_boxed_get(self, GdkCursor);
    GEnumValue *type = g_enum_get_value(g_type_class_peek(GDK_TYPE_CURSOR_TYPE), cursor->type);

    /* We use <...> syntax because gtk.gdk.Cursor objects are generally impossible to
     * reconstruct with eval(repr(...))  round-trip. */
    return PyString_FromFormat("<%s at %p: %s>",
                               self->ob_type->tp_name, self,
                               type ? type->value_name : "UNKNOWN TYPE");
}
#line 2623 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkCursor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Cursor",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gdk_cursor_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkCursor_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_cursor_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_cursor_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkRectangle ----------- */

#line 24 "gdkrectangle.override"
static int
_wrap_gdk_rectangle_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    GdkRectangle rect = {0, 0, 0, 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|iiii:GdkRectangle.__init__",
				     kwlist, &(rect.x), &(rect.y),
				     &(rect.width), &(rect.height)))
	return -1;

    self->boxed =  g_boxed_copy(GDK_TYPE_RECTANGLE, &rect);
    self->free_on_dealloc = TRUE;
    self->gtype = GDK_TYPE_RECTANGLE;
    
    return 0;
}
#line 2694 "gdk.c"


#line 159 "gdkrectangle.override"
static PyObject *
_wrap_gdk_rectangle_intersect(PyGObject *self, PyObject *args,
			      PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyObject *py_src;
    GdkRectangle src, dest = {0, 0, 0, 0};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkRectangle.intersect",
				     kwlist, &py_src)) {
        return NULL;
    }
    
    if (!pyg_boxed_check(py_src, GDK_TYPE_RECTANGLE)) {
	if (!pygdk_rectangle_from_pyobject(py_src, &src)) {
	    PyErr_Clear();
	    PyErr_SetString(PyExc_TypeError,
			    "src must be a GdkRectangle or 4-tuple");
	    return NULL;
	}
    } else {
	src = *pyg_boxed_get(py_src, GdkRectangle);
    }

    gdk_rectangle_intersect(pyg_boxed_get(self, GdkRectangle), &src, &dest);
                            
    return pyg_boxed_new(GDK_TYPE_RECTANGLE, &dest, TRUE, TRUE);
}
#line 2726 "gdk.c"


#line 189 "gdkrectangle.override"
static PyObject *
_wrap_gdk_rectangle_union(PyGObject *self, PyObject *args,
			  PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyObject *py_src;
    GdkRectangle src, dest;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkRectangle.union",
				     kwlist, &py_src)) {
        return NULL;
    }
    
    if (!pyg_boxed_check(py_src, GDK_TYPE_RECTANGLE)) {
	if (!pygdk_rectangle_from_pyobject(py_src, &src)) {
	    PyErr_Clear();
	    PyErr_SetString(PyExc_TypeError,
			    "src must be a GdkRectangle or 4-tuple");
	    return NULL;
	}
    } else {
	src = *pyg_boxed_get(py_src, GdkRectangle);
    }

    gdk_rectangle_union(pyg_boxed_get(self, GdkRectangle), &src, &dest);
                            
    return pyg_boxed_new(GDK_TYPE_RECTANGLE, &dest, TRUE, TRUE);
}
#line 2758 "gdk.c"


static const PyMethodDef _PyGdkRectangle_methods[] = {
    { "intersect", (PyCFunction)_wrap_gdk_rectangle_intersect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "union", (PyCFunction)_wrap_gdk_rectangle_union, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 107 "gdkrectangle.override"
static int
_wrap_gdk_rectangle__set_x(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, GdkRectangle)->x = val;
    return 0;
}
#line 2781 "gdk.c"


static PyObject *
_wrap_gdk_rectangle__get_x(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkRectangle)->x;
    return PyInt_FromLong(ret);
}

#line 120 "gdkrectangle.override"
static int
_wrap_gdk_rectangle__set_y(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, GdkRectangle)->y = val;
    return 0;
}
#line 2805 "gdk.c"


static PyObject *
_wrap_gdk_rectangle__get_y(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkRectangle)->y;
    return PyInt_FromLong(ret);
}

#line 133 "gdkrectangle.override"
static int
_wrap_gdk_rectangle__set_width(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, GdkRectangle)->width = val;
    return 0;
}
#line 2829 "gdk.c"


static PyObject *
_wrap_gdk_rectangle__get_width(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkRectangle)->width;
    return PyInt_FromLong(ret);
}

#line 146 "gdkrectangle.override"
static int
_wrap_gdk_rectangle__set_height(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, GdkRectangle)->height = val;
    return 0;
}
#line 2853 "gdk.c"


static PyObject *
_wrap_gdk_rectangle__get_height(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GdkRectangle)->height;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gdk_rectangle_getsets[] = {
    { "x", (getter)_wrap_gdk_rectangle__get_x, (setter)_wrap_gdk_rectangle__set_x },
    { "y", (getter)_wrap_gdk_rectangle__get_y, (setter)_wrap_gdk_rectangle__set_y },
    { "width", (getter)_wrap_gdk_rectangle__get_width, (setter)_wrap_gdk_rectangle__set_width },
    { "height", (getter)_wrap_gdk_rectangle__get_height, (setter)_wrap_gdk_rectangle__set_height },
    { NULL, (getter)0, (setter)0 },
};

#line 252 "gdkrectangle.override"
static PyObject *
_wrap_gdk_rectangle_tp_repr(PyGBoxed *self)
{
    GdkRectangle *rect = pyg_boxed_get(self, GdkRectangle);
    return PyString_FromFormat("%s(%d, %d, %d, %d)", self->ob_type->tp_name,
                               rect->x, rect->y, rect->width, rect->height);
}
#line 2881 "gdk.c"


#line 44 "gdkrectangle.override"
static Py_ssize_t
_wrap_gdk_rectangle_length(PyGBoxed *self)
{
    return 4;
}
static PyObject *
_wrap_gdk_rectangle_getitem(PyGBoxed *self, Py_ssize_t pos)
{
    GdkRectangle *rect;

    if (pos < 0) pos += 4;
    if (pos < 0 || pos >= 4) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }
    rect = pyg_boxed_get(self, GdkRectangle);
    switch (pos) {
    case 0: return PyInt_FromLong(rect->x);
    case 1: return PyInt_FromLong(rect->y);
    case 2: return PyInt_FromLong(rect->width);
    case 3: return PyInt_FromLong(rect->height);
    default:
        g_assert_not_reached();
        return NULL;
    }
}
static int
_wrap_gdk_rectangle_setitem(PyGBoxed *self, Py_ssize_t pos, PyObject *value)
{
    GdkRectangle *rect;
    gint val;

    if (pos < 0) pos += 4;
    if (pos < 0 || pos >= 4) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }
    rect = pyg_boxed_get(self, GdkRectangle);
    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    switch(pos) {
    case 0: rect->x      = val; break;
    case 1: rect->y      = val; break;
    case 2: rect->width  = val; break;
    case 3: rect->height = val; break;
    default:
        g_assert_not_reached();
        return -1;
    }
    return 0;
}
static PySequenceMethods _wrap_gdk_rectangle_tp_as_sequence = {
    (lenfunc)_wrap_gdk_rectangle_length,
    0,
    0,
    (ssizeargfunc)_wrap_gdk_rectangle_getitem,
    0,
    (ssizeobjargproc)_wrap_gdk_rectangle_setitem,
    0,
};
#line 2946 "gdk.c"


#line 219 "gdkrectangle.override"
static PyObject *
_wrap_gdk_rectangle_tp_richcompare(PyObject *self, PyObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGdkRectangle_Type)
        && PyObject_TypeCheck(other, &PyGdkRectangle_Type)) {
        GdkRectangle *rect1 = pyg_boxed_get(self, GdkRectangle);
        GdkRectangle *rect2 = pyg_boxed_get(other, GdkRectangle);

        switch (op) {
        case Py_EQ:
            result = (rect1->x == rect2->x && rect1->y == rect2->y
                      && rect1->width == rect2->width && rect1->height == rect2->height
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (rect1->x != rect2->x || rect1->y != rect2->y
                      || rect1->width != rect2->width || rect1->height != rect2->height
                      ? Py_True : Py_False);
            break;
        default:
            result = Py_NotImplemented;
        }
    }
    else
        result = Py_NotImplemented;

    Py_INCREF(result);
    return result;
}
#line 2981 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkRectangle_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Rectangle",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gdk_rectangle_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)&_wrap_gdk_rectangle_tp_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_gdk_rectangle_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkRectangle_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_rectangle_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_rectangle_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkRegion ----------- */

static int
_wrap_gdk_region_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":Gdk.Region.__init__", kwlist))
        return -1;
    self->gtype = PYGDK_TYPE_REGION;
    self->free_on_dealloc = FALSE;
    self->boxed = gdk_region_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkRegion object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

#line 525 "./gdk.override"
static PyObject *
_wrap_gdk_region_get_clipbox(PyGObject *self)
{
    GdkRectangle rect = {0, 0, 0, 0};

    gdk_region_get_clipbox(pyg_boxed_get(self, GdkRegion), &rect);

    return pyg_boxed_new(GDK_TYPE_RECTANGLE, &rect, TRUE, TRUE);
}
#line 3062 "gdk.c"


static PyObject *
_wrap_gdk_region_empty(PyObject *self)
{
    int ret;

    
    ret = gdk_region_empty(pyg_boxed_get(self, GdkRegion));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_region_equal(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "other", NULL };
    PyObject *py_other;
    int ret;
    GdkRegion *other = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.equal", kwlist, &py_other))
        return NULL;
    if (pyg_boxed_check(py_other, PYGDK_TYPE_REGION))
        other = pyg_boxed_get(py_other, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "other should be a GdkRegion");
        return NULL;
    }
    
    ret = gdk_region_equal(pyg_boxed_get(self, GdkRegion), other);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_region_point_in(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Region.point_in", kwlist, &x, &y))
        return NULL;
    
    ret = gdk_region_point_in(pyg_boxed_get(self, GdkRegion), x, y);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_region_rect_in(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    PyObject *py_rect;
    GdkRectangle rect = { 0, 0, 0, 0 };
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.rect_in", kwlist, &py_rect))
        return NULL;
    if (!pygdk_rectangle_from_pyobject(py_rect, &rect))
        return NULL;
    
    ret = gdk_region_rect_in(pyg_boxed_get(self, GdkRegion), &rect);
    
    return pyg_enum_from_gtype(GDK_TYPE_OVERLAP_TYPE, ret);
}

static PyObject *
_wrap_gdk_region_copy(PyObject *self)
{
    GdkRegion *ret;

    
    ret = gdk_region_copy(pyg_boxed_get(self, GdkRegion));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, TRUE, TRUE);
}

#line 536 "./gdk.override"
static PyObject *
_wrap_gdk_region_get_rectangles(PyGObject *self)
{
    GdkRectangle    *rect;
    gint            n_rect, i;
    PyObject        *py_rects;

    gdk_region_get_rectangles(pyg_boxed_get(self, GdkRegion), &rect, &n_rect);

    py_rects = PyList_New(n_rect);    
    
    for (i = 0; i < n_rect; i++)
	PyList_SetItem(py_rects, i, pyg_boxed_new(GDK_TYPE_RECTANGLE,
						   &rect[i], TRUE, TRUE));

    g_free(rect);
    return py_rects;
}
#line 3164 "gdk.c"


static PyObject *
_wrap_gdk_region_offset(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dx", "dy", NULL };
    int dx, dy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Region.offset", kwlist, &dx, &dy))
        return NULL;
    
    gdk_region_offset(pyg_boxed_get(self, GdkRegion), dx, dy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_shrink(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dx", "dy", NULL };
    int dx, dy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Region.shrink", kwlist, &dx, &dy))
        return NULL;
    
    gdk_region_shrink(pyg_boxed_get(self, GdkRegion), dx, dy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_union_with_rect(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    PyObject *py_rect;
    GdkRectangle rect = { 0, 0, 0, 0 };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.union_with_rect", kwlist, &py_rect))
        return NULL;
    if (!pygdk_rectangle_from_pyobject(py_rect, &rect))
        return NULL;
    
    gdk_region_union_with_rect(pyg_boxed_get(self, GdkRegion), &rect);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_intersect(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source2", NULL };
    PyObject *py_source2;
    GdkRegion *source2 = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.intersect", kwlist, &py_source2))
        return NULL;
    if (pyg_boxed_check(py_source2, PYGDK_TYPE_REGION))
        source2 = pyg_boxed_get(py_source2, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "source2 should be a GdkRegion");
        return NULL;
    }
    
    gdk_region_intersect(pyg_boxed_get(self, GdkRegion), source2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_union(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source2", NULL };
    PyObject *py_source2;
    GdkRegion *source2 = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.union", kwlist, &py_source2))
        return NULL;
    if (pyg_boxed_check(py_source2, PYGDK_TYPE_REGION))
        source2 = pyg_boxed_get(py_source2, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "source2 should be a GdkRegion");
        return NULL;
    }
    
    gdk_region_union(pyg_boxed_get(self, GdkRegion), source2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_subtract(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source2", NULL };
    PyObject *py_source2;
    GdkRegion *source2 = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.subtract", kwlist, &py_source2))
        return NULL;
    if (pyg_boxed_check(py_source2, PYGDK_TYPE_REGION))
        source2 = pyg_boxed_get(py_source2, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "source2 should be a GdkRegion");
        return NULL;
    }
    
    gdk_region_subtract(pyg_boxed_get(self, GdkRegion), source2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_region_xor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source2", NULL };
    PyObject *py_source2;
    GdkRegion *source2 = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Region.xor", kwlist, &py_source2))
        return NULL;
    if (pyg_boxed_check(py_source2, PYGDK_TYPE_REGION))
        source2 = pyg_boxed_get(py_source2, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "source2 should be a GdkRegion");
        return NULL;
    }
    
    gdk_region_xor(pyg_boxed_get(self, GdkRegion), source2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkRegion_methods[] = {
    { "get_clipbox", (PyCFunction)_wrap_gdk_region_get_clipbox, METH_NOARGS,
      NULL },
    { "empty", (PyCFunction)_wrap_gdk_region_empty, METH_NOARGS,
      NULL },
    { "equal", (PyCFunction)_wrap_gdk_region_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "point_in", (PyCFunction)_wrap_gdk_region_point_in, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rect_in", (PyCFunction)_wrap_gdk_region_rect_in, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy", (PyCFunction)_wrap_gdk_region_copy, METH_NOARGS,
      NULL },
    { "get_rectangles", (PyCFunction)_wrap_gdk_region_get_rectangles, METH_NOARGS,
      NULL },
    { "offset", (PyCFunction)_wrap_gdk_region_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "shrink", (PyCFunction)_wrap_gdk_region_shrink, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "union_with_rect", (PyCFunction)_wrap_gdk_region_union_with_rect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "intersect", (PyCFunction)_wrap_gdk_region_intersect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "union", (PyCFunction)_wrap_gdk_region_union, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "subtract", (PyCFunction)_wrap_gdk_region_subtract, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "xor", (PyCFunction)_wrap_gdk_region_xor, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 556 "./gdk.override"
static PyObject *
_wrap_pygdk_region_tp_richcompare(PyObject *self, PyObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGdkRegion_Type)
        && PyObject_TypeCheck(other, &PyGdkRegion_Type)) {
        GdkRegion *region1 = pyg_boxed_get(self, GdkRegion);
        GdkRegion *region2 = pyg_boxed_get(other, GdkRegion);

        switch (op) {
        case Py_EQ:
            result = (gdk_region_equal(region1, region2)
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (!gdk_region_equal(region1, region2)
                      ? Py_True : Py_False);
            break;
        default:
            result = Py_NotImplemented;
        }
    }
    else
        result = Py_NotImplemented;

    Py_INCREF(result);
    return result;
}
#line 3365 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkRegion_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Region",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_pygdk_region_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkRegion_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gdk_region_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkAppLaunchContext ----------- */

static int
_wrap_gdk_app_launch_context_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gtk.gdk.AppLaunchContext.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gtk.gdk.AppLaunchContext object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gdk_app_launch_context_set_display(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", NULL };
    PyGObject *display;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.AppLaunchContext.set_display", kwlist, &PyGdkDisplay_Type, &display))
        return NULL;
    
    gdk_app_launch_context_set_display(GDK_APP_LAUNCH_CONTEXT(self->obj), GDK_DISPLAY(display->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_app_launch_context_set_screen(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", NULL };
    PyGObject *screen;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.AppLaunchContext.set_screen", kwlist, &PyGdkScreen_Type, &screen))
        return NULL;
    
    gdk_app_launch_context_set_screen(GDK_APP_LAUNCH_CONTEXT(self->obj), GDK_SCREEN(screen->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_app_launch_context_set_desktop(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desktop", NULL };
    int desktop;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.AppLaunchContext.set_desktop", kwlist, &desktop))
        return NULL;
    
    gdk_app_launch_context_set_desktop(GDK_APP_LAUNCH_CONTEXT(self->obj), desktop);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_app_launch_context_set_timestamp(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timestamp", NULL };
    unsigned long timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:Gdk.AppLaunchContext.set_timestamp", kwlist, &timestamp))
        return NULL;
    if (timestamp > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " timestamp parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_app_launch_context_set_timestamp(GDK_APP_LAUNCH_CONTEXT(self->obj), timestamp);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_app_launch_context_set_icon(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon", NULL };
    PyGObject *py_icon;
    GIcon *icon = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.AppLaunchContext.set_icon", kwlist, &py_icon))
        return NULL;
    if (py_icon && pygobject_check(py_icon, &PyGIcon_Type))
        icon = G_ICON(py_icon->obj);
    else if ((PyObject *)py_icon != Py_None) {
        PyErr_SetString(PyExc_TypeError, "icon should be a GIcon or None");
        return NULL;
    }
    
    gdk_app_launch_context_set_icon(GDK_APP_LAUNCH_CONTEXT(self->obj), (GIcon *) icon);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_app_launch_context_set_icon_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon_name", NULL };
    char *icon_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"z:Gdk.AppLaunchContext.set_icon_name", kwlist, &icon_name))
        return NULL;
    
    gdk_app_launch_context_set_icon_name(GDK_APP_LAUNCH_CONTEXT(self->obj), icon_name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkAppLaunchContext_methods[] = {
    { "set_display", (PyCFunction)_wrap_gdk_app_launch_context_set_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_screen", (PyCFunction)_wrap_gdk_app_launch_context_set_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_desktop", (PyCFunction)_wrap_gdk_app_launch_context_set_desktop, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_timestamp", (PyCFunction)_wrap_gdk_app_launch_context_set_timestamp, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon", (PyCFunction)_wrap_gdk_app_launch_context_set_icon, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_name", (PyCFunction)_wrap_gdk_app_launch_context_set_icon_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkAppLaunchContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.AppLaunchContext",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkAppLaunchContext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_app_launch_context_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkColormap ----------- */

static int
_wrap_gdk_colormap_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "visual", "allocate", NULL };
    PyGObject *visual;
    int allocate;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Gdk.Colormap.__init__", kwlist, &PyGdkVisual_Type, &visual, &allocate))
        return -1;
    self->obj = (GObject *)gdk_colormap_new(GDK_VISUAL(visual->obj), allocate);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkColormap object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_colormap_change(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ncolors", NULL };
    int ncolors;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Colormap.change", kwlist, &ncolors))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "deprecated") < 0)
        return NULL;
    
    gdk_colormap_change(GDK_COLORMAP(self->obj), ncolors);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_colormap_free_colors(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colors", "ncolors", NULL };
    PyObject *py_colors;
    int ncolors;
    GdkColor *colors = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Gdk.Colormap.free_colors", kwlist, &py_colors, &ncolors))
        return NULL;
    if (pyg_boxed_check(py_colors, GDK_TYPE_COLOR))
        colors = pyg_boxed_get(py_colors, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "colors should be a GdkColor");
        return NULL;
    }
    
    gdk_colormap_free_colors(GDK_COLORMAP(self->obj), colors, ncolors);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 360 "gdkcolor.override"
static PyObject *
_wrap_gdk_colormap_alloc_color(PyGObject *self, PyObject *args,
                              PyObject *kwargs)
{
    static char *kwlist1[] = { "red", "green", "blue", "writeable", "best_match", NULL };
    static char *kwlist2[] = { "spec", "writeable", "best_match", NULL };
    static char *kwlist3[] = { "color", "writeable", "best_match", NULL };
    GdkColor colour = { 0, 0, 0, 0 };
    gboolean writeable = FALSE;
    gboolean best_match = TRUE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "HHH|ii:GdkColormap.alloc_color", kwlist1,
                                     &colour.red, &colour.green, &colour.blue,
                                     &writeable, &best_match)) {

 	PyObject *pycolour;
	
        PyErr_Clear();
        if (!PyArg_ParseTupleAndKeywords(args, kwargs,
 					 "O|ii:GdkColormap.alloc_color",
 					 kwlist3, &pycolour,
 					 &writeable, &best_match)) {
            return NULL;
	}

 	if (!pyg_boxed_check(pycolour, GDK_TYPE_COLOR)) {
 	    gchar *color_name;
 
 	    PyErr_Clear();
 	    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
 					     "s|ii:GdkColormap.alloc_color",
 					     kwlist2, &color_name, &writeable,
 					     &best_match)) {
 		return NULL;
	    }
	    
 	    if (!gdk_color_parse(color_name, &colour)) {
 		PyErr_SetString(PyExc_ValueError,
 				"unable to parse colour specification");
 		return NULL;
	    }
 	} else {
 	    colour = *pyg_boxed_get(pycolour, GdkColor);
        }
    }
    
    if (!gdk_colormap_alloc_color(GDK_COLORMAP(self->obj),
                                  &colour, writeable, best_match)) {
        PyErr_SetString(PyExc_RuntimeError, "couldn't allocate colour");
        return NULL;
    }
    return pyg_boxed_new(GDK_TYPE_COLOR, &colour, TRUE, TRUE);
}
#line 3719 "gdk.c"


#line 508 "gdkcolor.override"
static PyObject *
_wrap_gdk_colormap_query_color(PyGObject *self, PyObject *args,
                               PyObject *kwargs)
{
    static char *kwlist[] = { "pixel", NULL };
    GdkColor colour = { 0, 0, 0, 0 };
    PyObject *py_pixel;
    gulong pixel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O:GdkColormap.query_color", kwlist,
                                     &py_pixel))
        return NULL;

    if (PyLong_Check(py_pixel)) {
        pixel = PyLong_AsUnsignedLong(py_pixel);
        if (PyErr_Occurred())
            return NULL;
    } else if (PyInt_Check(py_pixel))
        pixel = PyInt_AS_LONG(py_pixel);
    else {
        PyErr_SetString(PyExc_TypeError, "GdkColormap.query_color: pixel must be"
                        " either int or long");
        return NULL;
    }

    gdk_colormap_query_color(GDK_COLORMAP(self->obj), pixel, &colour);
    
    return pyg_boxed_new(GDK_TYPE_COLOR, &colour, TRUE, TRUE);
}
#line 3753 "gdk.c"


static PyObject *
_wrap_gdk_colormap_get_visual(PyGObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_colormap_get_visual(GDK_COLORMAP(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_colormap_get_screen(PyGObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_colormap_get_screen(GDK_COLORMAP(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_color_white(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    int ret;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Colormap.white", kwlist, &py_color))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "deprecated") < 0)
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    ret = gdk_color_white(GDK_COLORMAP(self->obj), color);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_color_black(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    int ret;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Colormap.black", kwlist, &py_color))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "deprecated") < 0)
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    ret = gdk_color_black(GDK_COLORMAP(self->obj), color);
    
    return PyInt_FromLong(ret);
}

#line 416 "gdkcolor.override"
static PyObject *
_wrap_gdk_color_alloc(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    if (!PyErr_Warn(PyExc_DeprecationWarning, "use GdkColormap.alloc_color")<0)
        return NULL;
    return _wrap_gdk_colormap_alloc_color(self, args, kwargs);
}
#line 3836 "gdk.c"


static const PyMethodDef _PyGdkColormap_methods[] = {
    { "change", (PyCFunction)_wrap_gdk_colormap_change, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "free_colors", (PyCFunction)_wrap_gdk_colormap_free_colors, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "alloc_color", (PyCFunction)_wrap_gdk_colormap_alloc_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_color", (PyCFunction)_wrap_gdk_colormap_query_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_visual", (PyCFunction)_wrap_gdk_colormap_get_visual, METH_NOARGS,
      NULL },
    { "get_screen", (PyCFunction)_wrap_gdk_colormap_get_screen, METH_NOARGS,
      NULL },
    { "white", (PyCFunction)_wrap_gdk_color_white, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "black", (PyCFunction)_wrap_gdk_color_black, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "alloc", (PyCFunction)_wrap_gdk_color_alloc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkColormap_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Colormap",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkColormap_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_colormap_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkDevice ----------- */

static PyObject *
_wrap_gdk_device_set_source(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source", NULL };
    PyObject *py_source = NULL;
    GdkInputSource source;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Device.set_source", kwlist, &py_source))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INPUT_SOURCE, py_source, (gpointer)&source))
        return NULL;
    
    gdk_device_set_source(GDK_DEVICE(self->obj), source);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_device_set_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", NULL };
    PyObject *py_mode = NULL;
    GdkInputMode mode;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Device.set_mode", kwlist, &py_mode))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INPUT_MODE, py_mode, (gpointer)&mode))
        return NULL;
    
    ret = gdk_device_set_mode(GDK_DEVICE(self->obj), mode);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_device_set_key(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", "keyval", "modifiers", NULL };
    PyObject *py_index = NULL, *py_keyval = NULL, *py_modifiers = NULL;
    GdkModifierType modifiers;
    guint index = 0, keyval = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOO:Gdk.Device.set_key", kwlist, &py_index, &py_keyval, &py_modifiers))
        return NULL;
    if (py_index) {
        if (PyLong_Check(py_index))
            index = PyLong_AsUnsignedLong(py_index);
        else if (PyInt_Check(py_index))
            index = PyInt_AsLong(py_index);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'index' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (pyg_flags_get_value(GDK_TYPE_MODIFIER_TYPE, py_modifiers, (gpointer)&modifiers))
        return NULL;
    
    gdk_device_set_key(GDK_DEVICE(self->obj), index, keyval, modifiers);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_device_set_axis_use(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", "use", NULL };
    PyObject *py_index = NULL, *py_use = NULL;
    GdkAxisUse use;
    guint index = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Gdk.Device.set_axis_use", kwlist, &py_index, &py_use))
        return NULL;
    if (py_index) {
        if (PyLong_Check(py_index))
            index = PyLong_AsUnsignedLong(py_index);
        else if (PyInt_Check(py_index))
            index = PyInt_AsLong(py_index);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'index' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (pyg_enum_get_value(GDK_TYPE_AXIS_USE, py_use, (gpointer)&use))
        return NULL;
    
    gdk_device_set_axis_use(GDK_DEVICE(self->obj), index, use);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 621 "./gdk.override"
static PyObject *
_wrap_gdk_device_get_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", NULL };
    GdkDevice *device = GDK_DEVICE(self->obj);
    PyGObject *window;
    gdouble *axes;
    GdkModifierType mask;
    PyObject *py_axes;
    guint i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkDevice.get_state",
                                     kwlist, &window))
        return NULL;
    if (!pygobject_check(window, &PyGdkWindow_Type)) {
        PyErr_SetString(PyExc_TypeError, "window should be a GdkWindow");
        return NULL;
    }
    axes = g_new0(gdouble, device->num_axes);
    gdk_device_get_state(device, GDK_WINDOW(window->obj), axes, &mask);
    py_axes = PyTuple_New(device->num_axes);
    for (i = 0; i < device->num_axes; i++)
        PyTuple_SetItem(py_axes, i, PyFloat_FromDouble(axes[i]));
    g_free(axes);
    return Py_BuildValue("(NN)", py_axes,
                         pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE, mask));
}
#line 4043 "gdk.c"


#line 652 "./gdk.override"
static PyObject *
_wrap_gdk_device_get_history(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", "start", "stop", NULL };
    GdkDevice *device = GDK_DEVICE(self->obj);
    PyGObject *window;
    guint start, stop;
    GdkTimeCoord **events;
    gint n_events;
    PyObject *pyevents;
    guint i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "Oii:GdkDevice.get_history", kwlist,
                                     &window, &start, &stop))
        return NULL;
    if (!pygobject_check(window, &PyGdkWindow_Type)) {
        PyErr_SetString(PyExc_TypeError, "window should be a GdkWindow");
        return NULL;
    }
    gdk_device_get_history(device, GDK_WINDOW(window->obj), start, stop,
                           &events, &n_events);
    pyevents = PyTuple_New(n_events);
    for (i = 0; i < n_events; i++) {
        PyObject *axes;
        gint j;

        axes = PyTuple_New(device->num_axes);
        for (j = 0; j < device->num_axes; j++)
            PyTuple_SetItem(axes, j, PyFloat_FromDouble(events[i]->axes[j]));
        PyTuple_SetItem(pyevents, i, Py_BuildValue("(iN)", events[i]->time,
                                                   axes));
    }
    gdk_device_free_history(events, n_events);
    return pyevents;
}
#line 4083 "gdk.c"


#line 690 "./gdk.override"
static PyObject *
_wrap_gdk_device_get_axis(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "axes", "use", NULL };
    GdkDevice *device = GDK_DEVICE(self->obj);
    PyObject *py_axes;
    gdouble *axes, value;
    GdkAxisUse use;
    gboolean ret;
    gint i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi:GdkDevice.get_axis",
                                     kwlist, &py_axes, &use))
        return NULL;
    if (!PySequence_Check(py_axes)) {
        PyErr_SetString(PyExc_TypeError, "axes must be a sequence");
        return NULL;
    }
    if (PySequence_Length(py_axes) != device->num_axes) {
        PyErr_SetString(PyExc_TypeError, "axes has the wrong length");
        return NULL;
    }
    axes = g_new(gdouble, device->num_axes);
    for (i = 0; i < device->num_axes; i++) {
        PyObject *item = PySequence_GetItem(py_axes, i);

        axes[i] = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (PyErr_Occurred()) {
            g_free(axes);
            return NULL;
       }
    }
    ret = gdk_device_get_axis(device, axes, use, &value);
    g_free(axes);
    if (ret)
        return PyFloat_FromDouble(value);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 4127 "gdk.c"


static const PyMethodDef _PyGdkDevice_methods[] = {
    { "set_source", (PyCFunction)_wrap_gdk_device_set_source, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_mode", (PyCFunction)_wrap_gdk_device_set_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_key", (PyCFunction)_wrap_gdk_device_set_key, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_axis_use", (PyCFunction)_wrap_gdk_device_set_axis_use, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_state", (PyCFunction)_wrap_gdk_device_get_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_history", (PyCFunction)_wrap_gdk_device_get_history, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_axis", (PyCFunction)_wrap_gdk_device_get_axis, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gdk_device__get_name(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GDK_DEVICE(pygobject_get(self))->name;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_device__get_source(PyObject *self, void *closure)
{
    gint ret;

    ret = GDK_DEVICE(pygobject_get(self))->source;
    return pyg_enum_from_gtype(GDK_TYPE_INPUT_SOURCE, ret);
}

static PyObject *
_wrap_gdk_device__get_mode(PyObject *self, void *closure)
{
    gint ret;

    ret = GDK_DEVICE(pygobject_get(self))->mode;
    return pyg_enum_from_gtype(GDK_TYPE_INPUT_MODE, ret);
}

static PyObject *
_wrap_gdk_device__get_has_cursor(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_DEVICE(pygobject_get(self))->has_cursor;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_device__get_num_axes(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_DEVICE(pygobject_get(self))->num_axes;
    return PyInt_FromLong(ret);
}

#line 587 "./gdk.override"
static PyObject *
_wrap_gdk_device__get_axes(PyGObject *self, void *closure)
{
    GdkDevice *device = GDK_DEVICE(self->obj);
    PyObject *ret;
    gint i;

    ret = PyTuple_New(device->num_axes);
    for (i = 0; i < device->num_axes; i++)
        PyTuple_SetItem(ret, i, Py_BuildValue("(idd)",
                                              device->axes[i].use,
                                              device->axes[i].min,
                                              device->axes[i].max));
    return ret;
}
#line 4213 "gdk.c"


static PyObject *
_wrap_gdk_device__get_num_keys(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_DEVICE(pygobject_get(self))->num_keys;
    return PyInt_FromLong(ret);
}

#line 604 "./gdk.override"
static PyObject *
_wrap_gdk_device__get_keys(PyGObject *self, void *closure)
{
    GdkDevice *device = GDK_DEVICE(self->obj);
    PyObject *ret;
    gint i;

    ret = PyTuple_New(device->num_keys);
    for (i = 0; i < device->num_keys; i++)
        PyTuple_SetItem(ret, i,
                        Py_BuildValue("(iN)", device->keys[i].keyval,
                                      pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                                           device->keys[i].modifiers)));
    return ret;
}
#line 4241 "gdk.c"


static const PyGetSetDef gdk_device_getsets[] = {
    { "name", (getter)_wrap_gdk_device__get_name, (setter)0 },
    { "source", (getter)_wrap_gdk_device__get_source, (setter)0 },
    { "mode", (getter)_wrap_gdk_device__get_mode, (setter)0 },
    { "has_cursor", (getter)_wrap_gdk_device__get_has_cursor, (setter)0 },
    { "num_axes", (getter)_wrap_gdk_device__get_num_axes, (setter)0 },
    { "axes", (getter)_wrap_gdk_device__get_axes, (setter)0 },
    { "num_keys", (getter)_wrap_gdk_device__get_num_keys, (setter)0 },
    { "keys", (getter)_wrap_gdk_device__get_keys, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkDevice_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Device",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkDevice_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_device_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkDisplay ----------- */

static int
_wrap_gdk_display_open(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display_name", NULL };
    char *display_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"z:Gdk.Display.__init__", kwlist, &display_name))
        return -1;
    self->obj = (GObject *)gdk_display_open(display_name);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkDisplay object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_display_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = gdk_display_get_name(GDK_DISPLAY(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_get_n_screens(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_get_n_screens(GDK_DISPLAY(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_display_get_screen(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen_num", NULL };
    int screen_num;
    GdkScreen *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Display.get_screen", kwlist, &screen_num))
        return NULL;
    
    ret = gdk_display_get_screen(GDK_DISPLAY(self->obj), screen_num);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_display_get_default_screen(PyGObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_display_get_default_screen(GDK_DISPLAY(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_display_pointer_ungrab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time_", NULL };
    unsigned long time_ = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|k:Gdk.Display.pointer_ungrab", kwlist, &time_))
        return NULL;
    if (time_ > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time_ parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_display_pointer_ungrab(GDK_DISPLAY(self->obj), time_);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_keyboard_ungrab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time_", NULL };
    unsigned long time_ = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|k:Gdk.Display.keyboard_ungrab", kwlist, &time_))
        return NULL;
    if (time_ > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time_ parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_display_keyboard_ungrab(GDK_DISPLAY(self->obj), time_);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_pointer_is_grabbed(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_pointer_is_grabbed(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_beep(PyGObject *self)
{
    
    gdk_display_beep(GDK_DISPLAY(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_sync(PyGObject *self)
{
    
    gdk_display_sync(GDK_DISPLAY(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1570 "./gdk.override"
static PyObject *
_wrap_gdk_display_close(PyGObject *self)
{
    GdkDisplay *display = GDK_DISPLAY(self->obj);
    if (self->obj && !display->closed) {
        g_object_ref(self->obj);
        gdk_display_close(display);
    }
    Py_INCREF(Py_None);
    return Py_None;
}
#line 4462 "gdk.c"


#line 929 "./gdk.override"
static PyObject *
_wrap_gdk_display_list_devices(PyGObject *self)
{
    GList * devlist,  *tmp;
    PyObject *list;

    devlist = gdk_display_list_devices(GDK_DISPLAY_OBJECT(self->obj));
    list = PyList_New(0);
    for (tmp = devlist; tmp != NULL; tmp = tmp->next) {
        PyObject *item = pygobject_new((GObject *)tmp->data);
        PyList_Append(list, item);
        Py_DECREF(item);
    }

    return list;
}
#line 4482 "gdk.c"


static PyObject *
_wrap_gdk_display_get_event(PyGObject *self)
{
    GdkEvent *ret;

    
    ret = gdk_display_get_event(GDK_DISPLAY(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_display_peek_event(PyGObject *self)
{
    GdkEvent *ret;

    
    ret = gdk_display_peek_event(GDK_DISPLAY(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_display_put_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    GdkEvent *event = NULL;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Display.put_event", kwlist, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    
    gdk_display_put_event(GDK_DISPLAY(self->obj), event);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_set_double_click_time(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "msec", NULL };
    PyObject *py_msec = NULL;
    guint msec = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Display.set_double_click_time", kwlist, &py_msec))
        return NULL;
    if (py_msec) {
        if (PyLong_Check(py_msec))
            msec = PyLong_AsUnsignedLong(py_msec);
        else if (PyInt_Check(py_msec))
            msec = PyInt_AsLong(py_msec);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'msec' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    gdk_display_set_double_click_time(GDK_DISPLAY(self->obj), msec);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_get_core_pointer(PyGObject *self)
{
    GdkDevice *ret;

    
    ret = gdk_display_get_core_pointer(GDK_DISPLAY(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 947 "./gdk.override"
static PyObject *
_wrap_gdk_display_get_pointer(PyGObject *self)
{
    GdkScreen *screen = NULL;
    gint x, y;
    GdkModifierType mask;

    gdk_display_get_pointer(GDK_DISPLAY_OBJECT(self->obj), &screen, &x, &y, &mask);
    return Py_BuildValue("(NiiN)", pygobject_new((GObject *)screen),
			 x, y,
                         pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE, mask));
}
#line 4582 "gdk.c"


#line 961 "./gdk.override"
static PyObject *
_wrap_gdk_display_get_window_at_pointer(PyGObject *self)
{
    GdkWindow *window;
    gint win_x, win_y;

    window = gdk_display_get_window_at_pointer(GDK_DISPLAY_OBJECT(self->obj),
					       &win_x, &win_y);
    if (window)
	return Py_BuildValue("(Nii)", pygobject_new((GObject *)window),
			     win_x, win_y);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 4600 "gdk.c"


#line 1341 "./gdk.override"
static PyObject *
_wrap_gdk_display_store_clipboard(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "clipboard_window", "time_", "targets", NULL };
    PyGObject *pyclipboard_window;
    guint32 time_;
    PyObject *pytargets;
    GdkAtom *targets;
    int tlen, i;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!iO:GdkDIsplay.store_clipboard", kwlist,
				     &PyGdkWindow_Type, &pyclipboard_window,
                                     &time_, &pytargets))
	return NULL;

    if (!pytargets || pytargets == Py_None) {
        tlen = 0;
        targets = NULL;
    } else {
        if (!PySequence_Check(pytargets)) {
            PyErr_SetString(PyExc_TypeError,
                            "targets must be a sequence of target strings"
                            "or GdkAtoms or None");
            return NULL;
        }
        tlen = PySequence_Size(pytargets);
        targets = g_new0(GdkAtom, tlen);
        for (i = 0; i < tlen; i++) {
            PyObject *item = PySequence_GetItem(pytargets, i);
            targets[i] = pygdk_atom_from_pyobject(item);
            if (PyErr_Occurred()) {
                Py_DECREF(item);
                g_free(targets);
                return NULL;
            }
            Py_DECREF(item);
        }
        g_free(targets);
    }
    gdk_display_store_clipboard(GDK_DISPLAY(self->obj),
                                GDK_WINDOW(pyclipboard_window->obj),
                                time_, targets, tlen);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 4651 "gdk.c"


static PyObject *
_wrap_gdk_display_supports_selection_notification(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_selection_notification(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_request_selection_notification(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "selection", NULL };
    PyObject *py_selection = NULL;
    int ret;
    GdkAtom selection;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Display.request_selection_notification", kwlist, &py_selection))
        return NULL;
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    
    ret = gdk_display_request_selection_notification(GDK_DISPLAY(self->obj), selection);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_supports_clipboard_persistence(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_clipboard_persistence(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_flush(PyGObject *self)
{
    
    gdk_display_flush(GDK_DISPLAY(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_set_double_click_distance(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "distance", NULL };
    PyObject *py_distance = NULL;
    guint distance = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Display.set_double_click_distance", kwlist, &py_distance))
        return NULL;
    if (py_distance) {
        if (PyLong_Check(py_distance))
            distance = PyLong_AsUnsignedLong(py_distance);
        else if (PyInt_Check(py_distance))
            distance = PyInt_AsLong(py_distance);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'distance' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    gdk_display_set_double_click_distance(GDK_DISPLAY(self->obj), distance);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_supports_cursor_alpha(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_cursor_alpha(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_supports_cursor_color(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_cursor_color(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_get_default_cursor_size(PyGObject *self)
{
    guint ret;

    
    ret = gdk_display_get_default_cursor_size(GDK_DISPLAY(self->obj));
    
    return PyLong_FromUnsignedLong(ret);
}

#line 1142 "./gdk.override"
static PyObject *
_wrap_gdk_display_get_maximal_cursor_size(PyGObject *self)
{
    guint width, height;

    gdk_display_get_maximal_cursor_size(GDK_DISPLAY(self->obj),
					&width, &height);

    return Py_BuildValue("(ii)", width, height);
}
#line 4780 "gdk.c"


static PyObject *
_wrap_gdk_display_get_default_group(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_display_get_default_group(GDK_DISPLAY(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_display_warp_pointer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", "x", "y", NULL };
    PyGObject *screen;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.Display.warp_pointer", kwlist, &PyGdkScreen_Type, &screen, &x, &y))
        return NULL;
    
    gdk_display_warp_pointer(GDK_DISPLAY(self->obj), GDK_SCREEN(screen->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_display_supports_shapes(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_shapes(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_supports_input_shapes(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_input_shapes(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_display_supports_composite(PyGObject *self)
{
    int ret;

    
    ret = gdk_display_supports_composite(GDK_DISPLAY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GdkDisplay__do_get_display_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Display.get_display_name", kwlist, &PyGdkDisplay_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_CLASS(klass)->get_display_name)
        ret = GDK_DISPLAY_CLASS(klass)->get_display_name(GDK_DISPLAY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Display.get_display_name not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDisplay__do_get_n_screens(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Display.get_n_screens", kwlist, &PyGdkDisplay_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_CLASS(klass)->get_n_screens)
        ret = GDK_DISPLAY_CLASS(klass)->get_n_screens(GDK_DISPLAY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Display.get_n_screens not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_GdkDisplay__do_get_screen(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "screen_num", NULL };
    PyGObject *self;
    int screen_num;
    GdkScreen *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Gdk.Display.get_screen", kwlist, &PyGdkDisplay_Type, &self, &screen_num))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_CLASS(klass)->get_screen)
        ret = GDK_DISPLAY_CLASS(klass)->get_screen(GDK_DISPLAY(self->obj), screen_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Display.get_screen not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDisplay__do_get_default_screen(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkScreen *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Display.get_default_screen", kwlist, &PyGdkDisplay_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_CLASS(klass)->get_default_screen)
        ret = GDK_DISPLAY_CLASS(klass)->get_default_screen(GDK_DISPLAY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Display.get_default_screen not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDisplay__do_closed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "is_error", NULL };
    PyGObject *self;
    int is_error;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Gdk.Display.closed", kwlist, &PyGdkDisplay_Type, &self, &is_error))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_CLASS(klass)->closed)
        GDK_DISPLAY_CLASS(klass)->closed(GDK_DISPLAY(self->obj), is_error);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Display.closed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkDisplay_methods[] = {
    { "get_name", (PyCFunction)_wrap_gdk_display_get_name, METH_NOARGS,
      NULL },
    { "get_n_screens", (PyCFunction)_wrap_gdk_display_get_n_screens, METH_NOARGS,
      NULL },
    { "get_screen", (PyCFunction)_wrap_gdk_display_get_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_default_screen", (PyCFunction)_wrap_gdk_display_get_default_screen, METH_NOARGS,
      NULL },
    { "pointer_ungrab", (PyCFunction)_wrap_gdk_display_pointer_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyboard_ungrab", (PyCFunction)_wrap_gdk_display_keyboard_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pointer_is_grabbed", (PyCFunction)_wrap_gdk_display_pointer_is_grabbed, METH_NOARGS,
      NULL },
    { "beep", (PyCFunction)_wrap_gdk_display_beep, METH_NOARGS,
      NULL },
    { "sync", (PyCFunction)_wrap_gdk_display_sync, METH_NOARGS,
      NULL },
    { "close", (PyCFunction)_wrap_gdk_display_close, METH_NOARGS,
      NULL },
    { "list_devices", (PyCFunction)_wrap_gdk_display_list_devices, METH_NOARGS,
      NULL },
    { "get_event", (PyCFunction)_wrap_gdk_display_get_event, METH_NOARGS,
      NULL },
    { "peek_event", (PyCFunction)_wrap_gdk_display_peek_event, METH_NOARGS,
      NULL },
    { "put_event", (PyCFunction)_wrap_gdk_display_put_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_double_click_time", (PyCFunction)_wrap_gdk_display_set_double_click_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_core_pointer", (PyCFunction)_wrap_gdk_display_get_core_pointer, METH_NOARGS,
      NULL },
    { "get_pointer", (PyCFunction)_wrap_gdk_display_get_pointer, METH_NOARGS,
      NULL },
    { "get_window_at_pointer", (PyCFunction)_wrap_gdk_display_get_window_at_pointer, METH_NOARGS,
      NULL },
    { "store_clipboard", (PyCFunction)_wrap_gdk_display_store_clipboard, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "supports_selection_notification", (PyCFunction)_wrap_gdk_display_supports_selection_notification, METH_NOARGS,
      NULL },
    { "request_selection_notification", (PyCFunction)_wrap_gdk_display_request_selection_notification, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "supports_clipboard_persistence", (PyCFunction)_wrap_gdk_display_supports_clipboard_persistence, METH_NOARGS,
      NULL },
    { "flush", (PyCFunction)_wrap_gdk_display_flush, METH_NOARGS,
      NULL },
    { "set_double_click_distance", (PyCFunction)_wrap_gdk_display_set_double_click_distance, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "supports_cursor_alpha", (PyCFunction)_wrap_gdk_display_supports_cursor_alpha, METH_NOARGS,
      NULL },
    { "supports_cursor_color", (PyCFunction)_wrap_gdk_display_supports_cursor_color, METH_NOARGS,
      NULL },
    { "get_default_cursor_size", (PyCFunction)_wrap_gdk_display_get_default_cursor_size, METH_NOARGS,
      NULL },
    { "get_maximal_cursor_size", (PyCFunction)_wrap_gdk_display_get_maximal_cursor_size, METH_NOARGS,
      NULL },
    { "get_default_group", (PyCFunction)_wrap_gdk_display_get_default_group, METH_NOARGS,
      NULL },
    { "warp_pointer", (PyCFunction)_wrap_gdk_display_warp_pointer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "supports_shapes", (PyCFunction)_wrap_gdk_display_supports_shapes, METH_NOARGS,
      NULL },
    { "supports_input_shapes", (PyCFunction)_wrap_gdk_display_supports_input_shapes, METH_NOARGS,
      NULL },
    { "supports_composite", (PyCFunction)_wrap_gdk_display_supports_composite, METH_NOARGS,
      NULL },
    { "do_get_display_name", (PyCFunction)_wrap_GdkDisplay__do_get_display_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_screens", (PyCFunction)_wrap_GdkDisplay__do_get_n_screens, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_screen", (PyCFunction)_wrap_GdkDisplay__do_get_screen, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_default_screen", (PyCFunction)_wrap_GdkDisplay__do_get_default_screen, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_closed", (PyCFunction)_wrap_GdkDisplay__do_closed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkDisplay_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Display",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkDisplay_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_display_open,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static const gchar*
_wrap_GdkDisplay__proxy_do_get_display_name(GdkDisplay *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    const gchar* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_display_name");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_GdkDisplay__proxy_do_get_n_screens(GdkDisplay *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_screens");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkScreen*
_wrap_GdkDisplay__proxy_do_get_screen(GdkDisplay *self, gint screen_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_screen_num;
    GdkScreen* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_screen_num = PyInt_FromLong(screen_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_screen_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_screen");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkScreen*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkScreen*
_wrap_GdkDisplay__proxy_do_get_default_screen(GdkDisplay *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkScreen* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_default_screen");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkScreen*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GdkDisplay__proxy_do_closed(GdkDisplay *self, gboolean is_error)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_is_error;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_is_error = is_error? Py_True : Py_False;
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_is_error);
    PyTuple_SET_ITEM(py_args, 0, py_is_error);
    
    py_method = PyObject_GetAttrString(py_self, "do_closed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkDisplay_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkDisplayClass *klass = GDK_DISPLAY_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_display_name");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_display_name")))
            klass->get_display_name = _wrap_GdkDisplay__proxy_do_get_display_name;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_n_screens");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_n_screens")))
            klass->get_n_screens = _wrap_GdkDisplay__proxy_do_get_n_screens;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_screen");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_screen")))
            klass->get_screen = _wrap_GdkDisplay__proxy_do_get_screen;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_default_screen");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_default_screen")))
            klass->get_default_screen = _wrap_GdkDisplay__proxy_do_get_default_screen;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_closed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "closed")))
            klass->closed = _wrap_GdkDisplay__proxy_do_closed;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GdkDisplayManager ----------- */

static PyObject *
_wrap_gdk_display_manager_get_default_display(PyGObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_display_manager_get_default_display(GDK_DISPLAY_MANAGER(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_display_manager_set_default_display(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", NULL };
    PyGObject *display;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.DisplayManager.set_default_display", kwlist, &PyGdkDisplay_Type, &display))
        return NULL;
    
    gdk_display_manager_set_default_display(GDK_DISPLAY_MANAGER(self->obj), GDK_DISPLAY(display->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 977 "./gdk.override"
static PyObject *
_wrap_gdk_display_manager_list_displays(PyGObject *self)
{
    GSList *list, *tmp;
    PyObject *py_list;

    list = gdk_display_manager_list_displays(GDK_DISPLAY_MANAGER(self->obj));
    py_list = PyList_New(0);
    for (tmp = list; tmp != NULL; tmp = tmp->next) {
        PyObject *item = pygobject_new((GObject *)tmp->data);
        PyList_Append(py_list, item);
        Py_DECREF(item);
    }
    g_slist_free(list);

    return py_list;
}
#line 5495 "gdk.c"


static PyObject *
_wrap_GdkDisplayManager__do_display_opened(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "display", NULL };
    PyGObject *self, *display;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:Gdk.DisplayManager.display_opened", kwlist, &PyGdkDisplayManager_Type, &self, &PyGdkDisplay_Type, &display))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DISPLAY_MANAGER_CLASS(klass)->display_opened)
        GDK_DISPLAY_MANAGER_CLASS(klass)->display_opened(GDK_DISPLAY_MANAGER(self->obj), GDK_DISPLAY(display->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.DisplayManager.display_opened not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkDisplayManager_methods[] = {
    { "get_default_display", (PyCFunction)_wrap_gdk_display_manager_get_default_display, METH_NOARGS,
      NULL },
    { "set_default_display", (PyCFunction)_wrap_gdk_display_manager_set_default_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "list_displays", (PyCFunction)_wrap_gdk_display_manager_list_displays, METH_NOARGS,
      NULL },
    { "do_display_opened", (PyCFunction)_wrap_GdkDisplayManager__do_display_opened, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkDisplayManager_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.DisplayManager",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkDisplayManager_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GdkDisplayManager__proxy_do_display_opened(GdkDisplayManager *self, GdkDisplay*display)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_display = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (display)
        py_display = pygobject_new((GObject *) display);
    else {
        Py_INCREF(Py_None);
        py_display = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_display);
    
    py_method = PyObject_GetAttrString(py_self, "do_display_opened");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkDisplayManager_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkDisplayManagerClass *klass = GDK_DISPLAY_MANAGER_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_display_opened");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "display_opened")))
            klass->display_opened = _wrap_GdkDisplayManager__proxy_do_display_opened;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GdkDragContext ----------- */

static int
_wrap_gdk_drag_context_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gtk.gdk.DragContext.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gtk.gdk.DragContext object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gdk_drag_status(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "action", "time", NULL };
    PyObject *py_action = NULL;
    unsigned long time = GDK_CURRENT_TIME;
    GdkDragAction action;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O|k:Gdk.DragContext.drag_status", kwlist, &py_action, &time))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_DRAG_ACTION, py_action, (gpointer)&action))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_drag_status(GDK_DRAG_CONTEXT(self->obj), action, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drop_reply(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ok", "time", NULL };
    int ok;
    unsigned long time = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|k:Gdk.DragContext.drop_reply", kwlist, &ok, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_drop_reply(GDK_DRAG_CONTEXT(self->obj), ok, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drop_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "success", "time", NULL };
    int success;
    unsigned long time = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|k:Gdk.DragContext.drop_finish", kwlist, &success, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_drop_finish(GDK_DRAG_CONTEXT(self->obj), success, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drag_get_selection(PyGObject *self)
{
    PyObject * py_ret;
    gchar * name;
    GdkAtom ret;

    
    ret = gdk_drag_get_selection(GDK_DRAG_CONTEXT(self->obj));
    
    name = gdk_atom_name(ret);
    py_ret = PyString_FromString(name);
    g_free(name);
    return py_ret;
}

#line 732 "./gdk.override"
static PyObject *
_wrap_gdk_drag_find_window(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drag_window", "x_root", "y_root", NULL };
    PyGObject *drag_window;
    gint x_root, y_root;
    GdkWindow *dest_window;
    GdkDragProtocol protocol;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "Oii:GdkDragContext.drag_find_window",
                                     kwlist, &drag_window, &x_root, &y_root))
        return NULL;
    if (!pygobject_check(drag_window, &PyGdkWindow_Type)) {
        PyErr_SetString(PyExc_TypeError, "drag_window must be a GdkWindow");
        return NULL;
    }
    gdk_drag_find_window(GDK_DRAG_CONTEXT(self->obj),
                         GDK_WINDOW(drag_window->obj), x_root, y_root,
                         &dest_window, &protocol);
    return Py_BuildValue("(Ni)", pygobject_new((GObject *)dest_window),
                         protocol);
}
#line 5795 "gdk.c"


#line 757 "./gdk.override"
static PyObject *
_wrap_gdk_drag_find_window_for_screen(PyGObject *self, PyObject *args,
				      PyObject *kwargs)
{
    static char *kwlist[] = { "drag_window", "screen", "x_root", "y_root", NULL };
    PyGObject *drag_window, *screen;
    gint x_root, y_root;
    GdkWindow *dest_window;
    GdkDragProtocol protocol;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
			"O!O!ii:GdkDragContext.drag_find_window_for_screen",
			kwlist, &PyGdkWindow_Type, &drag_window,
			&PyGdkScreen_Type, &screen,
			&x_root, &y_root))
        return NULL;
    gdk_drag_find_window_for_screen(GDK_DRAG_CONTEXT(self->obj),
				    GDK_WINDOW(drag_window->obj),
				    GDK_SCREEN(screen->obj),
				    x_root, y_root,
				    &dest_window, &protocol);
    return Py_BuildValue("(Ni)", pygobject_new((GObject *)dest_window),
                         protocol);
}
#line 5823 "gdk.c"


static PyObject *
_wrap_gdk_drag_motion(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest_window", "protocol", "x_root", "y_root", "suggested_action", "possible_actions", "time", NULL };
    int x_root, y_root, ret;
    PyObject *py_protocol = NULL, *py_suggested_action = NULL, *py_possible_actions = NULL;
    GdkDragProtocol protocol;
    unsigned long time;
    GdkDragAction suggested_action, possible_actions;
    PyGObject *dest_window;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OiiOOk:Gdk.DragContext.drag_motion", kwlist, &PyGdkWindow_Type, &dest_window, &py_protocol, &x_root, &y_root, &py_suggested_action, &py_possible_actions, &time))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_DRAG_PROTOCOL, py_protocol, (gpointer)&protocol))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_DRAG_ACTION, py_suggested_action, (gpointer)&suggested_action))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_DRAG_ACTION, py_possible_actions, (gpointer)&possible_actions))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_drag_motion(GDK_DRAG_CONTEXT(self->obj), GDK_WINDOW(dest_window->obj), protocol, x_root, y_root, suggested_action, possible_actions, time);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_drag_drop(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    unsigned long time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:Gdk.DragContext.drag_drop", kwlist, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_drag_drop(GDK_DRAG_CONTEXT(self->obj), time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drag_abort(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    unsigned long time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:Gdk.DragContext.drag_abort", kwlist, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_drag_abort(GDK_DRAG_CONTEXT(self->obj), time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drag_drop_succeeded(PyGObject *self)
{
    int ret;

    
    ret = gdk_drag_drop_succeeded(GDK_DRAG_CONTEXT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_drag_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "success", "del_", "time", NULL };
    int success, del;
    unsigned long time = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii|k:Gdk.DragContext.finish", kwlist, &success, &del, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gtk_drag_finish(GDK_DRAG_CONTEXT(self->obj), success, del, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_get_source_widget(PyGObject *self)
{
    GtkWidget *ret;

    
    ret = gtk_drag_get_source_widget(GDK_DRAG_CONTEXT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gtk_drag_set_icon_widget(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "widget", "hot_x", "hot_y", NULL };
    PyGObject *widget;
    int hot_x, hot_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.DragContext.set_icon_widget", kwlist, &PyGtkWidget_Type, &widget, &hot_x, &hot_y))
        return NULL;
    
    gtk_drag_set_icon_widget(GDK_DRAG_CONTEXT(self->obj), GTK_WIDGET(widget->obj), hot_x, hot_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_set_icon_pixmap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", "pixmap", "mask", "hot_x", "hot_y", NULL };
    PyGObject *colormap, *pixmap, *py_mask;
    int hot_x, hot_y;
    GdkPixmap *mask = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!Oii:Gdk.DragContext.set_icon_pixmap", kwlist, &PyGdkColormap_Type, &colormap, &PyGdkPixmap_Type, &pixmap, &py_mask, &hot_x, &hot_y))
        return NULL;
    if (py_mask && pygobject_check(py_mask, &PyGdkPixmap_Type))
        mask = GDK_PIXMAP(py_mask->obj);
    else if ((PyObject *)py_mask != Py_None) {
        PyErr_SetString(PyExc_TypeError, "mask should be a GdkPixmap or None");
        return NULL;
    }
    
    gtk_drag_set_icon_pixmap(GDK_DRAG_CONTEXT(self->obj), GDK_COLORMAP(colormap->obj), GDK_PIXMAP(pixmap->obj), (GdkBitmap *) mask, hot_x, hot_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_set_icon_pixbuf(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixbuf", "hot_x", "hot_y", NULL };
    PyGObject *pixbuf;
    int hot_x, hot_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.DragContext.set_icon_pixbuf", kwlist, &PyGdkPixbuf_Type, &pixbuf, &hot_x, &hot_y))
        return NULL;
    
    gtk_drag_set_icon_pixbuf(GDK_DRAG_CONTEXT(self->obj), GDK_PIXBUF(pixbuf->obj), hot_x, hot_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_set_icon_stock(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stock_id", "hot_x", "hot_y", NULL };
    char *stock_id;
    int hot_x, hot_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sii:Gdk.DragContext.set_icon_stock", kwlist, &stock_id, &hot_x, &hot_y))
        return NULL;
    
    gtk_drag_set_icon_stock(GDK_DRAG_CONTEXT(self->obj), stock_id, hot_x, hot_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_set_icon_default(PyGObject *self)
{
    
    gtk_drag_set_icon_default(GDK_DRAG_CONTEXT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_drag_set_icon_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon_name", "hot_x", "hot_y", NULL };
    char *icon_name;
    int hot_x, hot_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sii:Gdk.DragContext.set_icon_name", kwlist, &icon_name, &hot_x, &hot_y))
        return NULL;
    
    gtk_drag_set_icon_name(GDK_DRAG_CONTEXT(self->obj), icon_name, hot_x, hot_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkDragContext_methods[] = {
    { "drag_status", (PyCFunction)_wrap_gdk_drag_status, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drop_reply", (PyCFunction)_wrap_gdk_drop_reply, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drop_finish", (PyCFunction)_wrap_gdk_drop_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_get_selection", (PyCFunction)_wrap_gdk_drag_get_selection, METH_NOARGS,
      NULL },
    { "drag_find_window", (PyCFunction)_wrap_gdk_drag_find_window, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_find_window_for_screen", (PyCFunction)_wrap_gdk_drag_find_window_for_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_motion", (PyCFunction)_wrap_gdk_drag_motion, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_drop", (PyCFunction)_wrap_gdk_drag_drop, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_abort", (PyCFunction)_wrap_gdk_drag_abort, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drag_drop_succeeded", (PyCFunction)_wrap_gdk_drag_drop_succeeded, METH_NOARGS,
      NULL },
    { "finish", (PyCFunction)_wrap_gtk_drag_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_source_widget", (PyCFunction)_wrap_gtk_drag_get_source_widget, METH_NOARGS,
      NULL },
    { "set_icon_widget", (PyCFunction)_wrap_gtk_drag_set_icon_widget, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_pixmap", (PyCFunction)_wrap_gtk_drag_set_icon_pixmap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_pixbuf", (PyCFunction)_wrap_gtk_drag_set_icon_pixbuf, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_stock", (PyCFunction)_wrap_gtk_drag_set_icon_stock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_default", (PyCFunction)_wrap_gtk_drag_set_icon_default, METH_NOARGS,
      NULL },
    { "set_icon_name", (PyCFunction)_wrap_gtk_drag_set_icon_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gdk_drag_context__get_protocol(PyObject *self, void *closure)
{
    gint ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->protocol;
    return pyg_enum_from_gtype(GDK_TYPE_DRAG_PROTOCOL, ret);
}

static PyObject *
_wrap_gdk_drag_context__get_is_source(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->is_source;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_drag_context__get_source_window(PyObject *self, void *closure)
{
    GdkWindow *ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->source_window;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_drag_context__get_dest_window(PyObject *self, void *closure)
{
    GdkWindow *ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->dest_window;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 783 "./gdk.override"
static PyObject *
_wrap_gdk_drag_context__get_targets(PyGObject *self, void *closure)
{
    PyObject *atom, *ret = PyList_New(0);
    GList *tmp;
    if (ret == NULL)
        return NULL;
    for (tmp = GDK_DRAG_CONTEXT(self->obj)->targets; tmp; tmp = tmp->next) {
        gchar *name;

        name = gdk_atom_name(GDK_POINTER_TO_ATOM(tmp->data));
        if ((atom = PyString_FromString(name)) == NULL) {
            g_free(name);
            Py_DECREF(ret);
            return NULL;
        }
        PyList_Append(ret, atom);
        g_free(name);
        Py_DECREF(atom);
    }
    return ret;
}
#line 6145 "gdk.c"


static PyObject *
_wrap_gdk_drag_context__get_actions(PyObject *self, void *closure)
{
    guint ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->actions;
    return pyg_flags_from_gtype(GDK_TYPE_DRAG_ACTION, ret);
}

static PyObject *
_wrap_gdk_drag_context__get_suggested_action(PyObject *self, void *closure)
{
    guint ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->suggested_action;
    return pyg_flags_from_gtype(GDK_TYPE_DRAG_ACTION, ret);
}

static PyObject *
_wrap_gdk_drag_context__get_action(PyObject *self, void *closure)
{
    guint ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->action;
    return pyg_flags_from_gtype(GDK_TYPE_DRAG_ACTION, ret);
}

static PyObject *
_wrap_gdk_drag_context__get_start_time(PyObject *self, void *closure)
{
    guint32 ret;

    ret = GDK_DRAG_CONTEXT(pygobject_get(self))->start_time;
    return PyLong_FromUnsignedLong(ret);

}

static const PyGetSetDef gdk_drag_context_getsets[] = {
    { "protocol", (getter)_wrap_gdk_drag_context__get_protocol, (setter)0 },
    { "is_source", (getter)_wrap_gdk_drag_context__get_is_source, (setter)0 },
    { "source_window", (getter)_wrap_gdk_drag_context__get_source_window, (setter)0 },
    { "dest_window", (getter)_wrap_gdk_drag_context__get_dest_window, (setter)0 },
    { "targets", (getter)_wrap_gdk_drag_context__get_targets, (setter)0 },
    { "actions", (getter)_wrap_gdk_drag_context__get_actions, (setter)0 },
    { "suggested_action", (getter)_wrap_gdk_drag_context__get_suggested_action, (setter)0 },
    { "action", (getter)_wrap_gdk_drag_context__get_action, (setter)0 },
    { "start_time", (getter)_wrap_gdk_drag_context__get_start_time, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkDragContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.DragContext",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkDragContext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_drag_context_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_drag_context_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkDrawable ----------- */

static PyObject *
_wrap_gdk_drawable_copy_to_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "image", "src_x", "src_y", "dest_x", "dest_y", "width", "height", NULL };
    PyGObject *image;
    int src_x, src_y, dest_x, dest_y, width, height;
    GdkImage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiii:Gdk.Drawable.copy_to_image", kwlist, &PyGdkImage_Type, &image, &src_x, &src_y, &dest_x, &dest_y, &width, &height))
        return NULL;
    
    ret = gdk_drawable_copy_to_image(GDK_DRAWABLE(self->obj), GDK_IMAGE(image->obj), src_x, src_y, dest_x, dest_y, width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 807 "./gdk.override"
static PyObject *
_wrap_gdk_drawable_get_size(PyGObject *self)
{
    gint width;
    gint height;

    gdk_drawable_get_size(GDK_DRAWABLE(self->obj), &width, &height);
    return Py_BuildValue("(ii)", width, height);
}
#line 6274 "gdk.c"


static PyObject *
_wrap_gdk_drawable_set_colormap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", NULL };
    PyGObject *colormap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.set_colormap", kwlist, &PyGdkColormap_Type, &colormap))
        return NULL;
    
    gdk_drawable_set_colormap(GDK_DRAWABLE(self->obj), GDK_COLORMAP(colormap->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_drawable_get_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_drawable_get_colormap(GDK_DRAWABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_drawable_get_visual(PyGObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_drawable_get_visual(GDK_DRAWABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_drawable_get_depth(PyGObject *self)
{
    int ret;

    
    ret = gdk_drawable_get_depth(GDK_DRAWABLE(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_drawable_get_screen(PyGObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_drawable_get_screen(GDK_DRAWABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_drawable_get_display(PyGObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_drawable_get_display(GDK_DRAWABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_draw_point(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", NULL };
    PyGObject *gc;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.Drawable.draw_point", kwlist, &PyGdkGC_Type, &gc, &x, &y))
        return NULL;
    
    gdk_draw_point(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x1", "y1", "x2", "y2", NULL };
    PyGObject *gc;
    int x1, y1, x2, y2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiii:Gdk.Drawable.draw_line", kwlist, &PyGdkGC_Type, &gc, &x1, &y1, &x2, &y2))
        return NULL;
    
    gdk_draw_line(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), x1, y1, x2, y2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_rectangle(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "filled", "x", "y", "width", "height", NULL };
    PyGObject *gc;
    int filled, x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiii:Gdk.Drawable.draw_rectangle", kwlist, &PyGdkGC_Type, &gc, &filled, &x, &y, &width, &height))
        return NULL;
    
    gdk_draw_rectangle(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), filled, x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_arc(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "filled", "x", "y", "width", "height", "angle1", "angle2", NULL };
    PyGObject *gc;
    int filled, x, y, width, height, angle1, angle2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiiii:Gdk.Drawable.draw_arc", kwlist, &PyGdkGC_Type, &gc, &filled, &x, &y, &width, &height, &angle1, &angle2))
        return NULL;
    
    gdk_draw_arc(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), filled, x, y, width, height, angle1, angle2);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 28 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_polygon(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "filled", "points", NULL };
    PyGObject *gc;
    PyObject *py_points;
    gint filled, npoints, i;
    GdkPoint *points;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!iO:GdkDrawable.draw_polygon", kwlist,
                                     &PyGdkGC_Type, &gc, &filled, &py_points))
        return NULL;
    if (!PySequence_Check(py_points)) {
        PyErr_SetString(PyExc_TypeError, "points must be a sequence");
        return NULL;
    }
    npoints = PySequence_Length(py_points);
    points = g_new(GdkPoint, npoints);
    for (i = 0; i < npoints; i++) {
        PyObject *item = PySequence_GetItem(py_points, i);
        Py_DECREF(item);
        if (!PyArg_ParseTuple(item, "ii", &points[i].x, &points[i].y)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "sequence members must be 2-tuples");
            g_free(points);
            return NULL;
        }
    }
    gdk_draw_polygon(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), filled,
                     points, npoints);
    g_free(points);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6452 "gdk.c"


static PyObject *
_wrap_gdk_draw_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", "gc", "x", "y", "string", NULL };
    GdkFont *font = NULL;
    PyObject *py_font;
    int x, y;
    char *string;
    PyGObject *gc;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!iis:Gdk.Drawable.draw_string", kwlist, &py_font, &PyGdkGC_Type, &gc, &x, &y, &string))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use GdkDrawable.draw_layout") < 0)
        return NULL;
    if (pyg_boxed_check(py_font, GDK_TYPE_FONT))
        font = pyg_boxed_get(py_font, GdkFont);
    else {
        PyErr_SetString(PyExc_TypeError, "font should be a GdkFont");
        return NULL;
    }
    
    gdk_draw_string(GDK_DRAWABLE(self->obj), font, GDK_GC(gc->obj), x, y, string);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 66 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", "gc", "x", "y", "text",NULL};
    PyGObject *gc;
    PyObject *font;
    int x, y;
    Py_ssize_t len;
    char *text;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OO!iis#:GdkDrawable.draw_text", kwlist,
                                     &font, &PyGdkGC_Type, &gc,
                                     &x, &y, &text, &len))
        return NULL;
    if (!pyg_boxed_check(font, GDK_TYPE_FONT)) {
        PyErr_SetString(PyExc_TypeError, "font must be a GdkFont");
        return NULL;
    }
    gdk_draw_text(GDK_DRAWABLE(self->obj), pyg_boxed_get(font, GdkFont),
                  GDK_GC(gc->obj), x, y, text, len);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6506 "gdk.c"


static PyObject *
_wrap_gdk_draw_drawable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "src", "xsrc", "ysrc", "xdest", "ydest", "width", "height", NULL };
    PyGObject *gc, *src;
    int xsrc, ysrc, xdest, ydest, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiii:Gdk.Drawable.draw_drawable", kwlist, &PyGdkGC_Type, &gc, &PyGdkDrawable_Type, &src, &xsrc, &ysrc, &xdest, &ydest, &width, &height))
        return NULL;
    
    gdk_draw_drawable(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), GDK_DRAWABLE(src->obj), xsrc, ysrc, xdest, ydest, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "image", "xsrc", "ysrc", "xdest", "ydest", "width", "height", NULL };
    PyGObject *gc, *image;
    int xsrc, ysrc, xdest, ydest, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiii:Gdk.Drawable.draw_image", kwlist, &PyGdkGC_Type, &gc, &PyGdkImage_Type, &image, &xsrc, &ysrc, &xdest, &ydest, &width, &height))
        return NULL;
    
    gdk_draw_image(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), GDK_IMAGE(image->obj), xsrc, ysrc, xdest, ydest, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 91 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_points(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "points", NULL };
    PyGObject *gc;
    PyObject *py_points;
    gint npoints, i;
    GdkPoint *points;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O:GdkDrawable.draw_points", kwlist,
                                     &PyGdkGC_Type, &gc, &py_points))
        return NULL;
    if (!PySequence_Check(py_points)) {
        PyErr_SetString(PyExc_TypeError, "points must be a sequence");
        return NULL;
    }
    npoints = PySequence_Length(py_points);
    points = g_new(GdkPoint, npoints);
    for (i = 0; i < npoints; i++) {
        PyObject *item = PySequence_GetItem(py_points, i);
        Py_DECREF(item);
        if (!PyArg_ParseTuple(item, "ii", &points[i].x, &points[i].y)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "sequence members must be 2-tuples");
            g_free(points);
            return NULL;
        }
    }
    gdk_draw_points(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), points, npoints);
    g_free(points);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6577 "gdk.c"


#line 128 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_segments(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "segs", NULL };
    PyGObject *gc;
    PyObject *py_segs;
    gint nsegs, i;
    GdkSegment *segs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O:GdkDrawable.draw_segments", kwlist,
                                     &PyGdkGC_Type, &gc, &py_segs))
        return NULL;
    if (!PySequence_Check(py_segs)) {
        PyErr_SetString(PyExc_TypeError, "segs must be a sequence");
        return NULL;
    }
    nsegs = PySequence_Length(py_segs);
    segs = g_new(GdkSegment, nsegs);
    for (i = 0; i < nsegs; i++) {
        PyObject *item = PySequence_GetItem(py_segs, i);
        Py_DECREF(item);
        if (!PyArg_ParseTuple(item, "iiii", &segs[i].x1, &segs[i].y1,
                              &segs[i].x2, &segs[i].y2)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "sequence members must be 4-tuples");
            g_free(segs);
            return NULL;
        }
    }
    gdk_draw_segments(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), segs, nsegs);
    g_free(segs);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6617 "gdk.c"


#line 166 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_lines(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "points", NULL };
    PyGObject *gc;
    PyObject *py_points;
    gint npoints, i;
    GdkPoint *points;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O:GdkDrawable.draw_lines", kwlist,
                                     &PyGdkGC_Type, &gc, &py_points))
        return NULL;
    if (!PySequence_Check(py_points)) {
        PyErr_SetString(PyExc_TypeError, "points must be a sequence");
        return NULL;
    }
    npoints = PySequence_Length(py_points);
    points = g_new(GdkPoint, npoints);
    for (i = 0; i < npoints; i++) {
        PyObject *item = PySequence_GetItem(py_points, i);
        Py_DECREF(item);
        if (!PyArg_ParseTuple(item, "ii", &points[i].x, &points[i].y)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "sequence members must be 2-tuples");
            g_free(points);
            return NULL;
        }
    }
    gdk_draw_lines(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), points, npoints);
    g_free(points);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6656 "gdk.c"


static PyObject *
_wrap_gdk_draw_pixbuf(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "pixbuf", "src_x", "src_y", "dest_x", "dest_y", "width", "height", "dither", "x_dither", "y_dither", NULL };
    PyGObject *py_gc, *pixbuf;
    int src_x, src_y, dest_x, dest_y, width = -1, height = -1, x_dither = 0, y_dither = 0;
    GdkGC *gc = NULL;
    PyObject *py_dither = NULL;
    GdkRgbDither dither = GDK_RGB_DITHER_NORMAL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!iiii|iiOii:Gdk.Drawable.draw_pixbuf", kwlist, &py_gc, &PyGdkPixbuf_Type, &pixbuf, &src_x, &src_y, &dest_x, &dest_y, &width, &height, &py_dither, &x_dither, &y_dither))
        return NULL;
    if (py_gc && pygobject_check(py_gc, &PyGdkGC_Type))
        gc = GDK_GC(py_gc->obj);
    else if ((PyObject *)py_gc != Py_None) {
        PyErr_SetString(PyExc_TypeError, "gc should be a GdkGC or None");
        return NULL;
    }
    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dither, (gpointer)&dither))
        return NULL;
    
    gdk_draw_pixbuf(GDK_DRAWABLE(self->obj), (GdkGC *) gc, GDK_PIXBUF(pixbuf->obj), src_x, src_y, dest_x, dest_y, width, height, dither, x_dither, y_dither);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_glyphs(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "font", "x", "y", "glyphs", NULL };
    PyGObject *gc, *font;
    int x, y;
    PangoGlyphString *glyphs = NULL;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiO:Gdk.Drawable.draw_glyphs", kwlist, &PyGdkGC_Type, &gc, &PyPangoFont_Type, &font, &x, &y, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    
    gdk_draw_glyphs(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), PANGO_FONT(font->obj), x, y, glyphs);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 425 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_layout_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "line", "foreground",
			      "background", NULL };
    PyGObject *gc, *py_line;
    int x, y;
    GdkColor *foreground = NULL, *background = NULL;
    PyObject *py_foreground = Py_None, *py_background = Py_None;
    PangoLayoutLine *line;
 
    if (!PyArg_ParseTupleAndKeywords(
	    args, kwargs, "O!iiO|OO:GdkDrawable.draw_layout_line",
	    kwlist, &PyGdkGC_Type, &gc, &x, &y, &py_line,
	    &py_foreground, &py_background))
        return NULL;
    if (pyg_boxed_check(py_line, PANGO_TYPE_LAYOUT_LINE))
        line = pyg_boxed_get(py_line, PangoLayoutLine);
    else {
        PyErr_SetString(PyExc_TypeError, "line should be a PangoLayoutLine");
        return NULL;
    }
    if (pyg_boxed_check(py_foreground, GDK_TYPE_COLOR))
        foreground = pyg_boxed_get(py_foreground, GdkColor);
    else if (py_foreground != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"foreground should be a GdkColor or None");
        return NULL;
    }
    if (pyg_boxed_check(py_background, GDK_TYPE_COLOR))
        background = pyg_boxed_get(py_background, GdkColor);
    else if (py_background != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"background should be a GdkColor or None");
        return NULL;
    }
    gdk_draw_layout_line_with_colors(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj),
                                     x, y, line, foreground, background);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6752 "gdk.c"


#line 388 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_layout(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "layout", "foreground",
			      "background", NULL };
    PyGObject *gc, *layout;
    int x, y;
    GdkColor *foreground = NULL, *background = NULL;
    PyObject *py_foreground = Py_None, *py_background = Py_None;

    if (!PyArg_ParseTupleAndKeywords(
	    args, kwargs, "O!iiO!|OO:GdkDrawable.draw_layout",
	    kwlist, &PyGdkGC_Type, &gc, &x, &y, &PyPangoLayout_Type, &layout,
	    &py_foreground, &py_background))
        return NULL;
    if (pyg_boxed_check(py_foreground, GDK_TYPE_COLOR))
        foreground = pyg_boxed_get(py_foreground, GdkColor);
    else if (py_foreground != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"foreground should be a GdkColor or None");
        return NULL;
    }
    if (pyg_boxed_check(py_background, GDK_TYPE_COLOR))
        background = pyg_boxed_get(py_background, GdkColor);
    else if (py_background != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"background should be a GdkColor or None");
        return NULL;
    }
    gdk_draw_layout_with_colors(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), x, y,
				PANGO_LAYOUT(layout->obj), foreground,
				background);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 6791 "gdk.c"


static PyObject *
_wrap_gdk_drawable_get_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;
    PyObject *py_ret;
    GdkImage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Drawable.get_image", kwlist, &x, &y, &width, &height))
        return NULL;
    
    ret = gdk_drawable_get_image(GDK_DRAWABLE(self->obj), x, y, width, height);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_drawable_get_clip_region(PyGObject *self)
{
    GdkRegion *ret;

    
    ret = gdk_drawable_get_clip_region(GDK_DRAWABLE(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gdk_drawable_get_visible_region(PyGObject *self)
{
    GdkRegion *ret;

    
    ret = gdk_drawable_get_visible_region(GDK_DRAWABLE(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, FALSE, TRUE);
}

#line 157 "gdkgc.override"
static PyObject *
_wrap_gdk_gc_new_with_values(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "foreground", "background", "font", "function",
                              "fill", "tile", "stipple", "clip_mask",
                              "subwindow_mode", "ts_x_origin", "ts_y_origin",
                              "clip_x_origin", "clip_y_origin",
                              "graphics_exposures", "line_width", "line_style",
                              "cap_style", "join_style", NULL };
    PyObject *foreground = Py_None, *background = Py_None;
    PyObject *font = Py_None;
    gint function = -1, fill = -1;
    PyObject *tile = Py_None, *stipple = Py_None, *clip_mask = Py_None;
    gint subwindow_mode = -1, ts_x_origin = -1, ts_y_origin = -1;
    gint clip_x_origin = -1, clip_y_origin = -1, graphics_exposures = -1;
    gint line_width = -1, line_style = -1, cap_style = -1, join_style = -1;
    GdkGCValues values;
    GdkGCValuesMask mask = 0;
    GdkGC *gc;
    PyObject *pygc;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|OOOiiOOOiiiiiiiiii:GdkDrawable.new_gc",
                                     kwlist, &foreground, &background, &font,
                                     &function, &fill, &tile, &stipple,
                                     &clip_mask, &subwindow_mode,
                                     &ts_x_origin, &ts_y_origin,
                                     &clip_x_origin, &clip_y_origin,
                                     &graphics_exposures, &line_width,
                                     &line_style, &cap_style, &join_style))
        return NULL;
    if (pyg_boxed_check(foreground, GDK_TYPE_COLOR)) {
        mask |= GDK_GC_FOREGROUND;
        values.foreground = *pyg_boxed_get(foreground, GdkColor);
    } else if (foreground != Py_None) {
        PyErr_SetString(PyExc_TypeError, "foreground must be a GdkColor");
        return NULL;
    }
    if (pyg_boxed_check(background, GDK_TYPE_COLOR)) {
        mask |= GDK_GC_BACKGROUND;
        values.background = *pyg_boxed_get(background, GdkColor);
    } else if (background != Py_None) {
        PyErr_SetString(PyExc_TypeError, "background must be a GdkColor");
        return NULL;
    }
    if (pyg_boxed_check(font, GDK_TYPE_FONT)) {
        mask |= GDK_GC_FONT;
        values.font = pyg_boxed_get(font, GdkFont);
    } else if (font != Py_None) {
        PyErr_SetString(PyExc_TypeError, "font must be a GdkFont");
        return NULL;
    }
    if (function != -1) {
        mask |= GDK_GC_FUNCTION;
        values.function = function;
    }
    if (fill != -1) {
        mask |= GDK_GC_FILL;
        values.fill = fill;
    }
    if (pygobject_check(tile, &PyGdkPixmap_Type)) {
        mask |= GDK_GC_TILE;
        values.tile = GDK_PIXMAP(pygobject_get(tile));
    } else if (tile != Py_None) {
        PyErr_SetString(PyExc_TypeError, "tile must be a GdkPixmap");
        return NULL;
    }
    if (pygobject_check(stipple, &PyGdkPixmap_Type)) {
        mask |= GDK_GC_STIPPLE;
        values.stipple = GDK_PIXMAP(pygobject_get(stipple));
    } else if (stipple != Py_None) {
        PyErr_SetString(PyExc_TypeError, "stipple must be a GdkPixmap");
        return NULL;
    }
    if (pygobject_check(clip_mask, &PyGdkPixmap_Type)) {
        mask |= GDK_GC_CLIP_MASK;
        values.clip_mask = GDK_PIXMAP(pygobject_get(clip_mask));
    } else if (clip_mask != Py_None) {
        PyErr_SetString(PyExc_TypeError, "clip_mask must be a GdkPixmap");
        return NULL;
    }
    if (subwindow_mode != -1) {
        mask |= GDK_GC_SUBWINDOW;
        values.subwindow_mode = subwindow_mode;
    }
    if (ts_x_origin != -1) {
        mask |= GDK_GC_TS_X_ORIGIN;
        values.ts_x_origin = ts_x_origin;
    }
    if (ts_y_origin != -1) {
        mask |= GDK_GC_TS_Y_ORIGIN;
        values.ts_y_origin = ts_y_origin;
    }
    if (clip_x_origin != -1) {
        mask |= GDK_GC_CLIP_X_ORIGIN;
        values.clip_x_origin = clip_x_origin;
    }
    if (clip_y_origin != -1) {
        mask |= GDK_GC_CLIP_Y_ORIGIN;
        values.clip_y_origin = clip_y_origin;
    }
    if (graphics_exposures != -1) {
        mask |= GDK_GC_EXPOSURES;
        values.graphics_exposures = graphics_exposures;
    }
    if (line_width != -1) {
        mask |= GDK_GC_LINE_WIDTH;
        values.line_width = line_width;
    }
    if (line_style != -1) {
        mask |= GDK_GC_LINE_STYLE;
        values.line_style = line_style;
    }
    if (cap_style != -1) {
        mask |= GDK_GC_CAP_STYLE;
        values.cap_style = cap_style;
    }
    if (join_style != -1) {
        mask |= GDK_GC_JOIN_STYLE;
        values.join_style = join_style;
    }
    gc = gdk_gc_new_with_values(GDK_DRAWABLE(self->obj), &values, mask);
    pygc = pygobject_new((GObject *)gc);
    g_object_unref(gc);
    return pygc;
}
#line 6964 "gdk.c"


static PyObject *
_wrap_gdk_image_get(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;
    GdkImage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Drawable.image_get", kwlist, &x, &y, &width, &height))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use GdkDrawable.get_image") < 0)
        return NULL;
    
    ret = gdk_image_get(GDK_DRAWABLE(self->obj), x, y, width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 203 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_rgb_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "width", "height", "dith",
                              "rgb_buf", "rowstride", "xdith", "ydith", NULL };
    PyGObject *gc;
    PyObject *py_dith;
    gint x, y, width, height, rowstride = -1, xdith = 0, ydith = 0;
    Py_ssize_t len;
    GdkRgbDither dith;
    guchar *rgb_buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!iiiiOs#|iii:GdkDrawable.draw_rgb_image",
                                     kwlist, &PyGdkGC_Type, &gc, &x, &y,
                                     &width, &height, &py_dith,
                                     &rgb_buf, &len, &rowstride,
                                     &xdith, &ydith))
        return NULL;

    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dith, (gint *)&dith))
        return NULL;
    if (!(width > 0 && height > 0)) {
        PyErr_SetString(PyExc_ValueError,
                        "height and width must be greater than zero");
        return NULL;
    }
    if (rowstride == -1) rowstride = width * 3;
    if (len < rowstride * (height - 1) + width*3) {
        PyErr_SetString(PyExc_IndexError, "rgb_buf is not large enough");
        return NULL;
    }
      /* We always call gdk_draw_rgb_image_dithalign instead of
       * gdk_draw_rgb_image, since gdk_draw_rgb_image is strictly
       * equivalent to calling gdk_draw_rgb_image_dithalign with
       * xdith=0 and ydith=0, which are their default values. */
    gdk_draw_rgb_image_dithalign(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj),
                                 x, y, width, height, dith, rgb_buf,
                                 rowstride, xdith, ydith);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 7028 "gdk.c"


#line 249 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_rgb_32_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "width", "height", "dith",
                              "rgb_buf", "rowstride", "xdith", "ydith", NULL };
    PyGObject *gc;
    PyObject *py_dith;
    gint x, y, width, height, rowstride = -1, xdith = 0, ydith = 0;
    Py_ssize_t len;
    GdkRgbDither dith;
    guchar *rgb_buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O!iiiiOs#|iii:GdkDrawable.draw_rgb_32_image",
                                     kwlist, &PyGdkGC_Type, &gc, &x, &y,
                                     &width, &height, &py_dith,
                                     &rgb_buf, &len, &rowstride,
                                     &xdith, &ydith))
        return NULL;

    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dith, (gint *)&dith))
        return NULL;
    if (!(width > 0 && height > 0)) {
        PyErr_SetString(PyExc_ValueError,
                        "height and width must be greater than zero");
        return NULL;
    }
    if (rowstride == -1) rowstride = width * 4;
    if (len < rowstride * (height - 1) + width*4) {
        PyErr_SetString(PyExc_IndexError, "rgb_buf is not large enough");
        return NULL;
    }
      /* We always call gdk_draw_rgb_32_image_dithalign instead of
       * gdk_draw_rgb_32_image, since gdk_draw_rgb_32_image is strictly
       * equivalent to calling gdk_draw_rgb_32_image_dithalign with
       * xdith=0 and ydith=0, which are their default values. */
    gdk_draw_rgb_32_image_dithalign(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj),
                                    x, y, width, height, dith, rgb_buf,
                                    rowstride, xdith, ydith);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 7074 "gdk.c"


#line 295 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_gray_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "width", "height", "dith",
                              "buf", "rowstride", NULL };
    PyGObject *gc;
    PyObject *py_dith;
    gint x, y, width, height, rowstride = -1;
    Py_ssize_t len;
    GdkRgbDither dith;
    guchar *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!iiiiOs#|i:GdkDrawable.draw_gray_image",
                                     kwlist, &PyGdkGC_Type, &gc, &x, &y,
                                     &width, &height, &py_dith,
                                     &buf, &len, &rowstride))
        return NULL;

    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dith, (gint *)&dith))
        return NULL;
    if (!(width > 0 && height > 0)) {
        PyErr_SetString(PyExc_ValueError,
                        "height and width must be greater than zero");
        return NULL;
    }
    if (rowstride == -1) rowstride = width;
    if (len < rowstride * (height - 1) + width) {
        PyErr_SetString(PyExc_IndexError, "buf is not large enough");
        return NULL;
    }
    gdk_draw_gray_image(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), x, y,
                        width, height, dith, buf, rowstride);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 7114 "gdk.c"


#line 333 "gdkdraw.override"
static PyObject *
_wrap_gdk_draw_indexed_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "x", "y", "width", "height", "dith",
                              "buf", "rowstride", "colors", NULL };
    PyGObject *gc;
    PyObject *py_dith;
    gint x, y, width, height, rowstride = -1;
    Py_ssize_t len;
    GdkRgbDither dith;
    guchar *buf;
    PyObject *pycolors;
    GdkRgbCmap *cmap;
    guint32 *colors;
    int i, n_colors;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!iiiiOs#iO:GdkDrawable.draw_indexed_image",
                                     kwlist, &PyGdkGC_Type, &gc, &x, &y,
                                     &width, &height, &py_dith,
                                     &buf, &len, &rowstride,
				     &pycolors))
        return NULL;

    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dith, (gint *)&dith))
        return NULL;
    if (!(width > 0 && height > 0)) {
        PyErr_SetString(PyExc_ValueError,
                        "height and width must be greater than zero");
        return NULL;
    }
    if (rowstride == -1) rowstride = width;
    if (len < rowstride * (height - 1) + width) {
        PyErr_SetString(PyExc_IndexError, "buf is not large enough");
        return NULL;
    }
    if (!PyList_Check(pycolors)) {
        PyErr_SetString(PyExc_TypeError, "colors must be a list");
        return NULL;
    }
    n_colors = PyList_Size(pycolors);
    colors = g_new0(guint32, n_colors);
    for (i = 0; i < n_colors; i++)
	colors[i] = PyInt_AsLong(PyList_GetItem(pycolors, i));
    cmap = gdk_rgb_cmap_new(colors, n_colors);
    g_free(colors);
    gdk_draw_indexed_image(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), x, y,
			   width, height, dith, buf, rowstride,
			   cmap);
    gdk_rgb_cmap_free(cmap);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 7171 "gdk.c"


#line 269 "gdkcairo.override"
static PyObject *
_wrap_gdk_cairo_create(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    cairo_t *ret;

    ret = gdk_cairo_create(GDK_DRAWABLE(self->obj));
    return PycairoContext_FromContext(ret, &PyGdkCairoContext_Type, NULL);
}

#line 7184 "gdk.c"


static PyObject *
_wrap_GdkDrawable__do_draw_rectangle(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "filled", "x", "y", "width", "height", NULL };
    PyGObject *self, *gc;
    int filled, x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiii:Gdk.Drawable.draw_rectangle", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &filled, &x, &y, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_rectangle)
        GDK_DRAWABLE_CLASS(klass)->draw_rectangle(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), filled, x, y, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_rectangle not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_arc(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "filled", "x", "y", "width", "height", "angle1", "angle2", NULL };
    PyGObject *self, *gc;
    int filled, x, y, width, height, angle1, angle2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiiii:Gdk.Drawable.draw_arc", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &filled, &x, &y, &width, &height, &angle1, &angle2))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_arc)
        GDK_DRAWABLE_CLASS(klass)->draw_arc(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), filled, x, y, width, height, angle1, angle2);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_arc not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "font", "gc", "x", "y", "text", "text_length", NULL };
    PyGObject *self, *gc;
    GdkFont *font = NULL;
    int x, y, text_length;
    char *text;
    PyObject *py_font;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO!iisi:Gdk.Drawable.draw_text", kwlist, &PyGdkDrawable_Type, &self, &py_font, &PyGdkGC_Type, &gc, &x, &y, &text, &text_length))
        return NULL;
    if (pyg_boxed_check(py_font, GDK_TYPE_FONT))
        font = pyg_boxed_get(py_font, GdkFont);
    else {
        PyErr_SetString(PyExc_TypeError, "font should be a GdkFont");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_text)
        GDK_DRAWABLE_CLASS(klass)->draw_text(GDK_DRAWABLE(self->obj), font, GDK_GC(gc->obj), x, y, text, text_length);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_text not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_drawable(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "src", "xsrc", "ysrc", "xdest", "ydest", "width", "height", NULL };
    PyGObject *self, *gc, *src;
    int xsrc, ysrc, xdest, ydest, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!iiiiii:Gdk.Drawable.draw_drawable", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &PyGdkDrawable_Type, &src, &xsrc, &ysrc, &xdest, &ydest, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_drawable)
        GDK_DRAWABLE_CLASS(klass)->draw_drawable(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), GDK_DRAWABLE(src->obj), xsrc, ysrc, xdest, ydest, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_drawable not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_glyphs(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "font", "x", "y", "glyphs", NULL };
    PyGObject *self, *gc, *font;
    int x, y;
    PangoGlyphString *glyphs = NULL;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!iiO:Gdk.Drawable.draw_glyphs", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &PyPangoFont_Type, &font, &x, &y, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_glyphs)
        GDK_DRAWABLE_CLASS(klass)->draw_glyphs(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), PANGO_FONT(font->obj), x, y, glyphs);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_glyphs not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_image(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "image", "xsrc", "ysrc", "xdest", "ydest", "width", "height", NULL };
    PyGObject *self, *gc, *image;
    int xsrc, ysrc, xdest, ydest, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!iiiiii:Gdk.Drawable.draw_image", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &PyGdkImage_Type, &image, &xsrc, &ysrc, &xdest, &ydest, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_image)
        GDK_DRAWABLE_CLASS(klass)->draw_image(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), GDK_IMAGE(image->obj), xsrc, ysrc, xdest, ydest, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_image not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_get_depth(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_depth", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_depth)
        ret = GDK_DRAWABLE_CLASS(klass)->get_depth(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_depth not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_GdkDrawable__do_set_colormap(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "cmap", NULL };
    PyGObject *self, *cmap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:Gdk.Drawable.set_colormap", kwlist, &PyGdkDrawable_Type, &self, &PyGdkColormap_Type, &cmap))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->set_colormap)
        GDK_DRAWABLE_CLASS(klass)->set_colormap(GDK_DRAWABLE(self->obj), GDK_COLORMAP(cmap->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.set_colormap not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_get_colormap(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkColormap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_colormap", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_colormap)
        ret = GDK_DRAWABLE_CLASS(klass)->get_colormap(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_colormap not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDrawable__do_get_visual(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkVisual *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_visual", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_visual)
        ret = GDK_DRAWABLE_CLASS(klass)->get_visual(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_visual not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDrawable__do_get_screen(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkScreen *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_screen", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_screen)
        ret = GDK_DRAWABLE_CLASS(klass)->get_screen(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_screen not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDrawable__do_get_image(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "x", "y", "width", "height", NULL };
    PyGObject *self;
    int x, y, width, height;
    GdkImage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiii:Gdk.Drawable.get_image", kwlist, &PyGdkDrawable_Type, &self, &x, &y, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_image)
        ret = GDK_DRAWABLE_CLASS(klass)->get_image(GDK_DRAWABLE(self->obj), x, y, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_image not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GdkDrawable__do_get_clip_region(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkRegion *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_clip_region", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_clip_region)
        ret = GDK_DRAWABLE_CLASS(klass)->get_clip_region(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_clip_region not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_GdkDrawable__do_get_visible_region(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GdkRegion *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Drawable.get_visible_region", kwlist, &PyGdkDrawable_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->get_visible_region)
        ret = GDK_DRAWABLE_CLASS(klass)->get_visible_region(GDK_DRAWABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.get_visible_region not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_GdkDrawable__do_draw_pixbuf(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "pixbuf", "src_x", "src_y", "dest_x", "dest_y", "width", "height", "dither", "x_dither", "y_dither", NULL };
    PyGObject *self, *gc, *pixbuf;
    int src_x, src_y, dest_x, dest_y, width, height, x_dither, y_dither;
    PyObject *py_dither = NULL;
    GdkRgbDither dither;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!iiiiiiOii:Gdk.Drawable.draw_pixbuf", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &PyGdkPixbuf_Type, &pixbuf, &src_x, &src_y, &dest_x, &dest_y, &width, &height, &py_dither, &x_dither, &y_dither))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dither, (gpointer)&dither))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_pixbuf)
        GDK_DRAWABLE_CLASS(klass)->draw_pixbuf(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), GDK_PIXBUF(pixbuf->obj), src_x, src_y, dest_x, dest_y, width, height, dither, x_dither, y_dither);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_pixbuf not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkDrawable__do_draw_glyphs_transformed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "gc", "matrix", "font", "x", "y", "glyphs", NULL };
    PyGObject *self, *gc, *font;
    PyObject *py_matrix, *py_glyphs;
    int x, y;
    PangoGlyphString *glyphs = NULL;
    PangoMatrix *matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!OO!iiO:Gdk.Drawable.draw_glyphs_transformed", kwlist, &PyGdkDrawable_Type, &self, &PyGdkGC_Type, &gc, &py_matrix, &PyPangoFont_Type, &font, &x, &y, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_matrix, PANGO_TYPE_MATRIX))
        matrix = pyg_boxed_get(py_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "matrix should be a PangoMatrix");
        return NULL;
    }
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_DRAWABLE_CLASS(klass)->draw_glyphs_transformed)
        GDK_DRAWABLE_CLASS(klass)->draw_glyphs_transformed(GDK_DRAWABLE(self->obj), GDK_GC(gc->obj), matrix, PANGO_FONT(font->obj), x, y, glyphs);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Drawable.draw_glyphs_transformed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkDrawable_methods[] = {
    { "copy_to_image", (PyCFunction)_wrap_gdk_drawable_copy_to_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_size", (PyCFunction)_wrap_gdk_drawable_get_size, METH_NOARGS,
      NULL },
    { "set_colormap", (PyCFunction)_wrap_gdk_drawable_set_colormap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_colormap", (PyCFunction)_wrap_gdk_drawable_get_colormap, METH_NOARGS,
      NULL },
    { "get_visual", (PyCFunction)_wrap_gdk_drawable_get_visual, METH_NOARGS,
      NULL },
    { "get_depth", (PyCFunction)_wrap_gdk_drawable_get_depth, METH_NOARGS,
      NULL },
    { "get_screen", (PyCFunction)_wrap_gdk_drawable_get_screen, METH_NOARGS,
      NULL },
    { "get_display", (PyCFunction)_wrap_gdk_drawable_get_display, METH_NOARGS,
      NULL },
    { "draw_point", (PyCFunction)_wrap_gdk_draw_point, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_line", (PyCFunction)_wrap_gdk_draw_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_rectangle", (PyCFunction)_wrap_gdk_draw_rectangle, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_arc", (PyCFunction)_wrap_gdk_draw_arc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_polygon", (PyCFunction)_wrap_gdk_draw_polygon, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_string", (PyCFunction)_wrap_gdk_draw_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_text", (PyCFunction)_wrap_gdk_draw_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_drawable", (PyCFunction)_wrap_gdk_draw_drawable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_image", (PyCFunction)_wrap_gdk_draw_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_points", (PyCFunction)_wrap_gdk_draw_points, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_segments", (PyCFunction)_wrap_gdk_draw_segments, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_lines", (PyCFunction)_wrap_gdk_draw_lines, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_pixbuf", (PyCFunction)_wrap_gdk_draw_pixbuf, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_glyphs", (PyCFunction)_wrap_gdk_draw_glyphs, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_layout_line", (PyCFunction)_wrap_gdk_draw_layout_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_layout", (PyCFunction)_wrap_gdk_draw_layout, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_image", (PyCFunction)_wrap_gdk_drawable_get_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_clip_region", (PyCFunction)_wrap_gdk_drawable_get_clip_region, METH_NOARGS,
      NULL },
    { "get_visible_region", (PyCFunction)_wrap_gdk_drawable_get_visible_region, METH_NOARGS,
      NULL },
    { "new_gc", (PyCFunction)_wrap_gdk_gc_new_with_values, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "image_get", (PyCFunction)_wrap_gdk_image_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_rgb_image", (PyCFunction)_wrap_gdk_draw_rgb_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_rgb_32_image", (PyCFunction)_wrap_gdk_draw_rgb_32_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_gray_image", (PyCFunction)_wrap_gdk_draw_gray_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_indexed_image", (PyCFunction)_wrap_gdk_draw_indexed_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "cairo_create", (PyCFunction)_wrap_gdk_cairo_create, METH_NOARGS,
      NULL },
    { "do_draw_rectangle", (PyCFunction)_wrap_GdkDrawable__do_draw_rectangle, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_arc", (PyCFunction)_wrap_GdkDrawable__do_draw_arc, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_text", (PyCFunction)_wrap_GdkDrawable__do_draw_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_drawable", (PyCFunction)_wrap_GdkDrawable__do_draw_drawable, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_glyphs", (PyCFunction)_wrap_GdkDrawable__do_draw_glyphs, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_image", (PyCFunction)_wrap_GdkDrawable__do_draw_image, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_depth", (PyCFunction)_wrap_GdkDrawable__do_get_depth, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_colormap", (PyCFunction)_wrap_GdkDrawable__do_set_colormap, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_colormap", (PyCFunction)_wrap_GdkDrawable__do_get_colormap, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_visual", (PyCFunction)_wrap_GdkDrawable__do_get_visual, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_screen", (PyCFunction)_wrap_GdkDrawable__do_get_screen, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_image", (PyCFunction)_wrap_GdkDrawable__do_get_image, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_clip_region", (PyCFunction)_wrap_GdkDrawable__do_get_clip_region, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_visible_region", (PyCFunction)_wrap_GdkDrawable__do_get_visible_region, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_pixbuf", (PyCFunction)_wrap_GdkDrawable__do_draw_pixbuf, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_glyphs_transformed", (PyCFunction)_wrap_GdkDrawable__do_draw_glyphs_transformed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 889 "./gdk.override"
static PyObject *
_wrap_gdk_drawable__get_xid(PyGObject *self, void *closure)
{
#if defined(GDK_WINDOWING_X11)
    GdkDrawable *drawable = GDK_DRAWABLE(self->obj);
    return PyLong_FromUnsignedLong(GDK_DRAWABLE_XID(drawable));
#else
    PyErr_SetString(PyExc_AttributeError, "xid attribute not supported");
    return NULL;
#endif
}
#line 7706 "gdk.c"


#line 876 "./gdk.override"
static PyObject *
_wrap_gdk_drawable__get_handle(PyGObject *self, void *closure)
{
#if defined(GDK_WINDOWING_WIN32)
    GdkDrawable *drawable = GDK_DRAWABLE(self->obj);
    return PyLong_FromVoidPtr(GDK_WINDOW_HWND(drawable));
#else
    PyErr_SetString(PyExc_AttributeError, "handle attribute not supported");
    return NULL;
#endif
}
#line 7721 "gdk.c"


static const PyGetSetDef gdk_drawable_getsets[] = {
    { "xid", (getter)_wrap_gdk_drawable__get_xid, (setter)0 },
    { "handle", (getter)_wrap_gdk_drawable__get_handle, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkDrawable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Drawable",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkDrawable_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_drawable_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GdkDrawable__proxy_do_draw_rectangle(GdkDrawable *self, GdkGC*gc, gboolean filled, gint x, gint y, gint width, gint height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_filled;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    py_filled = filled? Py_True : Py_False;
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(6);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    Py_INCREF(py_filled);
    PyTuple_SET_ITEM(py_args, 1, py_filled);
    PyTuple_SET_ITEM(py_args, 2, py_x);
    PyTuple_SET_ITEM(py_args, 3, py_y);
    PyTuple_SET_ITEM(py_args, 4, py_width);
    PyTuple_SET_ITEM(py_args, 5, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_rectangle");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_arc(GdkDrawable *self, GdkGC*gc, gboolean filled, gint x, gint y, gint width, gint height, gint angle1, gint angle2)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_filled;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_angle1;
    PyObject *py_angle2;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    py_filled = filled? Py_True : Py_False;
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    py_angle1 = PyInt_FromLong(angle1);
    py_angle2 = PyInt_FromLong(angle2);
    
    py_args = PyTuple_New(8);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    Py_INCREF(py_filled);
    PyTuple_SET_ITEM(py_args, 1, py_filled);
    PyTuple_SET_ITEM(py_args, 2, py_x);
    PyTuple_SET_ITEM(py_args, 3, py_y);
    PyTuple_SET_ITEM(py_args, 4, py_width);
    PyTuple_SET_ITEM(py_args, 5, py_height);
    PyTuple_SET_ITEM(py_args, 6, py_angle1);
    PyTuple_SET_ITEM(py_args, 7, py_angle2);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_arc");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_text(GdkDrawable *self, GdkFont*font, GdkGC*gc, gint x, gint y, const gchar*text, gint text_length)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_font;
    PyObject *py_gc = NULL;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_text = NULL;
    PyObject *py_text_length;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_font = pyg_boxed_new(GDK_TYPE_FONT, font, FALSE, FALSE);
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    if (text)
        py_text = PyString_FromString(text);
    if (!py_text) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_gc);
        Py_DECREF(py_font);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_text_length = PyInt_FromLong(text_length);
    
    py_args = PyTuple_New(6);
    PyTuple_SET_ITEM(py_args, 0, py_font);
    PyTuple_SET_ITEM(py_args, 1, py_gc);
    PyTuple_SET_ITEM(py_args, 2, py_x);
    PyTuple_SET_ITEM(py_args, 3, py_y);
    PyTuple_SET_ITEM(py_args, 4, py_text);
    PyTuple_SET_ITEM(py_args, 5, py_text_length);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_text");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_drawable(GdkDrawable *self, GdkGC*gc, GdkDrawable*src, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_src = NULL;
    PyObject *py_xsrc;
    PyObject *py_ysrc;
    PyObject *py_xdest;
    PyObject *py_ydest;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    if (src)
        py_src = pygobject_new((GObject *) src);
    else {
        Py_INCREF(Py_None);
        py_src = Py_None;
    }
    py_xsrc = PyInt_FromLong(xsrc);
    py_ysrc = PyInt_FromLong(ysrc);
    py_xdest = PyInt_FromLong(xdest);
    py_ydest = PyInt_FromLong(ydest);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(8);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    PyTuple_SET_ITEM(py_args, 1, py_src);
    PyTuple_SET_ITEM(py_args, 2, py_xsrc);
    PyTuple_SET_ITEM(py_args, 3, py_ysrc);
    PyTuple_SET_ITEM(py_args, 4, py_xdest);
    PyTuple_SET_ITEM(py_args, 5, py_ydest);
    PyTuple_SET_ITEM(py_args, 6, py_width);
    PyTuple_SET_ITEM(py_args, 7, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_drawable");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_glyphs(GdkDrawable *self, GdkGC*gc, PangoFont*font, gint x, gint y, PangoGlyphString*glyphs)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_font = NULL;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_glyphs;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    if (font)
        py_font = pygobject_new((GObject *) font);
    else {
        Py_INCREF(Py_None);
        py_font = Py_None;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_glyphs = pyg_boxed_new(PANGO_TYPE_GLYPH_STRING, glyphs, FALSE, FALSE);
    
    py_args = PyTuple_New(5);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    PyTuple_SET_ITEM(py_args, 1, py_font);
    PyTuple_SET_ITEM(py_args, 2, py_x);
    PyTuple_SET_ITEM(py_args, 3, py_y);
    PyTuple_SET_ITEM(py_args, 4, py_glyphs);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_glyphs");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_image(GdkDrawable *self, GdkGC*gc, GdkImage*image, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_image = NULL;
    PyObject *py_xsrc;
    PyObject *py_ysrc;
    PyObject *py_xdest;
    PyObject *py_ydest;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    if (image)
        py_image = pygobject_new((GObject *) image);
    else {
        Py_INCREF(Py_None);
        py_image = Py_None;
    }
    py_xsrc = PyInt_FromLong(xsrc);
    py_ysrc = PyInt_FromLong(ysrc);
    py_xdest = PyInt_FromLong(xdest);
    py_ydest = PyInt_FromLong(ydest);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(8);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    PyTuple_SET_ITEM(py_args, 1, py_image);
    PyTuple_SET_ITEM(py_args, 2, py_xsrc);
    PyTuple_SET_ITEM(py_args, 3, py_ysrc);
    PyTuple_SET_ITEM(py_args, 4, py_xdest);
    PyTuple_SET_ITEM(py_args, 5, py_ydest);
    PyTuple_SET_ITEM(py_args, 6, py_width);
    PyTuple_SET_ITEM(py_args, 7, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_image");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gint
_wrap_GdkDrawable__proxy_do_get_depth(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_depth");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GdkDrawable__proxy_do_set_colormap(GdkDrawable *self, GdkColormap*cmap)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cmap = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (cmap)
        py_cmap = pygobject_new((GObject *) cmap);
    else {
        Py_INCREF(Py_None);
        py_cmap = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_cmap);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_colormap");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GdkColormap*
_wrap_GdkDrawable__proxy_do_get_colormap(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkColormap* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_colormap");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkColormap*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkVisual*
_wrap_GdkDrawable__proxy_do_get_visual(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkVisual* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_visual");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkVisual*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkScreen*
_wrap_GdkDrawable__proxy_do_get_screen(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkScreen* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_screen");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkScreen*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkImage*
_wrap_GdkDrawable__proxy_do_get_image(GdkDrawable *self, gint x, gint y, gint width, gint height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_width;
    PyObject *py_height;
    GdkImage* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(4);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_width);
    PyTuple_SET_ITEM(py_args, 3, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_image");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GdkImage*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkRegion*
_wrap_GdkDrawable__proxy_do_get_clip_region(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkRegion* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_clip_region");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    if (!pyg_boxed_check(py_retval, PYGDK_TYPE_REGION)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GdkRegion");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    retval = pyg_boxed_get(py_retval, GdkRegion);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GdkRegion*
_wrap_GdkDrawable__proxy_do_get_visible_region(GdkDrawable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GdkRegion* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_visible_region");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    if (!pyg_boxed_check(py_retval, PYGDK_TYPE_REGION)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GdkRegion");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return gdk_region_new();
    }
    retval = pyg_boxed_get(py_retval, GdkRegion);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GdkDrawable__proxy_do_draw_pixbuf(GdkDrawable *self, GdkGC*gc, GdkPixbuf*pixbuf, gint src_x, gint src_y, gint dest_x, gint dest_y, gint width, gint height, GdkRgbDither dither, gint x_dither, gint y_dither)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_pixbuf = NULL;
    PyObject *py_src_x;
    PyObject *py_src_y;
    PyObject *py_dest_x;
    PyObject *py_dest_y;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_dither;
    PyObject *py_x_dither;
    PyObject *py_y_dither;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    if (pixbuf)
        py_pixbuf = pygobject_new((GObject *) pixbuf);
    else {
        Py_INCREF(Py_None);
        py_pixbuf = Py_None;
    }
    py_src_x = PyInt_FromLong(src_x);
    py_src_y = PyInt_FromLong(src_y);
    py_dest_x = PyInt_FromLong(dest_x);
    py_dest_y = PyInt_FromLong(dest_y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    py_dither = pyg_enum_from_gtype(GDK_TYPE_RGB_DITHER, dither);
    if (!py_dither) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_height);
        Py_DECREF(py_width);
        Py_DECREF(py_dest_y);
        Py_DECREF(py_dest_x);
        Py_DECREF(py_src_y);
        Py_DECREF(py_src_x);
        Py_DECREF(py_pixbuf);
        Py_DECREF(py_gc);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_x_dither = PyInt_FromLong(x_dither);
    py_y_dither = PyInt_FromLong(y_dither);
    
    py_args = PyTuple_New(11);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    PyTuple_SET_ITEM(py_args, 1, py_pixbuf);
    PyTuple_SET_ITEM(py_args, 2, py_src_x);
    PyTuple_SET_ITEM(py_args, 3, py_src_y);
    PyTuple_SET_ITEM(py_args, 4, py_dest_x);
    PyTuple_SET_ITEM(py_args, 5, py_dest_y);
    PyTuple_SET_ITEM(py_args, 6, py_width);
    PyTuple_SET_ITEM(py_args, 7, py_height);
    PyTuple_SET_ITEM(py_args, 8, py_dither);
    PyTuple_SET_ITEM(py_args, 9, py_x_dither);
    PyTuple_SET_ITEM(py_args, 10, py_y_dither);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_pixbuf");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkDrawable__proxy_do_draw_glyphs_transformed(GdkDrawable *self, GdkGC*gc, PangoMatrix*matrix, PangoFont*font, gint x, gint y, PangoGlyphString*glyphs)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_gc = NULL;
    PyObject *py_matrix;
    PyObject *py_font = NULL;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_glyphs;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (gc)
        py_gc = pygobject_new((GObject *) gc);
    else {
        Py_INCREF(Py_None);
        py_gc = Py_None;
    }
    py_matrix = pyg_boxed_new(PANGO_TYPE_MATRIX, matrix, FALSE, FALSE);
    if (font)
        py_font = pygobject_new((GObject *) font);
    else {
        Py_INCREF(Py_None);
        py_font = Py_None;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_glyphs = pyg_boxed_new(PANGO_TYPE_GLYPH_STRING, glyphs, FALSE, FALSE);
    
    py_args = PyTuple_New(6);
    PyTuple_SET_ITEM(py_args, 0, py_gc);
    PyTuple_SET_ITEM(py_args, 1, py_matrix);
    PyTuple_SET_ITEM(py_args, 2, py_font);
    PyTuple_SET_ITEM(py_args, 3, py_x);
    PyTuple_SET_ITEM(py_args, 4, py_y);
    PyTuple_SET_ITEM(py_args, 5, py_glyphs);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_glyphs_transformed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkDrawable_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkDrawableClass *klass = GDK_DRAWABLE_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    /* overriding do_create_gc is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_rectangle");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_rectangle")))
            klass->draw_rectangle = _wrap_GdkDrawable__proxy_do_draw_rectangle;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_arc");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_arc")))
            klass->draw_arc = _wrap_GdkDrawable__proxy_do_draw_arc;
        Py_DECREF(o);
    }

    /* overriding do_draw_polygon is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_text");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_text")))
            klass->draw_text = _wrap_GdkDrawable__proxy_do_draw_text;
        Py_DECREF(o);
    }

    /* overriding do_draw_text_wc is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_drawable");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_drawable")))
            klass->draw_drawable = _wrap_GdkDrawable__proxy_do_draw_drawable;
        Py_DECREF(o);
    }

    /* overriding do_draw_points is currently not supported */

    /* overriding do_draw_segments is currently not supported */

    /* overriding do_draw_lines is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_glyphs");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_glyphs")))
            klass->draw_glyphs = _wrap_GdkDrawable__proxy_do_draw_glyphs;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_image");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_image")))
            klass->draw_image = _wrap_GdkDrawable__proxy_do_draw_image;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_depth");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_depth")))
            klass->get_depth = _wrap_GdkDrawable__proxy_do_get_depth;
        Py_DECREF(o);
    }

    /* overriding do_get_size is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_colormap");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_colormap")))
            klass->set_colormap = _wrap_GdkDrawable__proxy_do_set_colormap;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_colormap");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_colormap")))
            klass->get_colormap = _wrap_GdkDrawable__proxy_do_get_colormap;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_visual");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_visual")))
            klass->get_visual = _wrap_GdkDrawable__proxy_do_get_visual;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_screen");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_screen")))
            klass->get_screen = _wrap_GdkDrawable__proxy_do_get_screen;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_image");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_image")))
            klass->get_image = _wrap_GdkDrawable__proxy_do_get_image;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_clip_region");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_clip_region")))
            klass->get_clip_region = _wrap_GdkDrawable__proxy_do_get_clip_region;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_visible_region");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_visible_region")))
            klass->get_visible_region = _wrap_GdkDrawable__proxy_do_get_visible_region;
        Py_DECREF(o);
    }

    /* overriding do_get_composite_drawable is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_pixbuf");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_pixbuf")))
            klass->draw_pixbuf = _wrap_GdkDrawable__proxy_do_draw_pixbuf;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_glyphs_transformed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_glyphs_transformed")))
            klass->draw_glyphs_transformed = _wrap_GdkDrawable__proxy_do_draw_glyphs_transformed;
        Py_DECREF(o);
    }

    /* overriding do_draw_trapezoids is currently not supported */

    /* overriding do_ref_cairo_surface is currently not supported */
    return 0;
}


/* ----------- GdkWindow ----------- */

#line 23 "gdkwindow.override"
static int
_wrap_gdk_window_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", "width", "height", "window_type",
			      "event_mask", "wclass", "title", "x", "y",
			      "visual", "colormap", "cursor", "wmclass_name",
			      "wmclass_class", "override_redirect", NULL };
    GdkWindowAttr attr = { NULL, 0, -1, -1, 0, 0, -1, NULL, NULL, 0, NULL,
			   NULL, NULL, -1 };
    guint attr_mask = 0;
    GdkWindow *parent = NULL;
    PyGObject *py_parent = NULL;
    PyObject *py_window_type = NULL;
    PyObject *py_wclass = NULL;
    PyGObject *py_visual = NULL;
    PyGObject *py_colormap = NULL;
    PyGObject *py_cursor = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "OiiOiO|siiO!O!Ossb:GdkWindow.__init__",
				     kwlist, &py_parent, &attr.width,
				     &attr.height, &py_window_type,
				     &attr.event_mask, &py_wclass,
				     &attr.title, &attr.x, &attr.y,
				     &PyGdkVisual_Type, &py_visual,
				     &PyGdkColormap_Type, &py_colormap,
				     &py_cursor,
				     &attr.wmclass_name,
				     &attr.wmclass_class,
				     &attr.override_redirect))
	return -1;

    if (py_parent && pygobject_check(py_parent, &PyGdkWindow_Type))
        parent = GDK_WINDOW(py_parent->obj);
    else if ((PyObject *)py_parent != Py_None) {
        PyErr_SetString(PyExc_TypeError, "parent must be a GdkWindow or None");
        return -1;
    }
    
    if (pyg_enum_get_value(GDK_TYPE_WINDOW_CLASS,
			   py_wclass, (gint *)&attr.wclass))
        return -1;
    if (pyg_enum_get_value(GDK_TYPE_WINDOW_TYPE,
			   py_window_type, (gint *)&attr.window_type))
        return -1;
    
    if (attr.title)
	attr_mask |= GDK_WA_TITLE;
    if (attr.x != -1)
	attr_mask |= GDK_WA_X;
    if (attr.y != -1)
	attr_mask |= GDK_WA_Y;
    if (py_visual) {
	attr.visual = GDK_VISUAL(py_visual->obj);
	attr_mask |= GDK_WA_VISUAL;
    }
    if (py_colormap) {
	attr.colormap = GDK_COLORMAP(py_colormap->obj);
	attr_mask |= GDK_WA_COLORMAP;
    }
    if (py_cursor) {
	if (!pyg_boxed_check(py_cursor, GDK_TYPE_CURSOR)) {
	    PyErr_SetString(PyExc_TypeError, "cursor should be a GdkCursor");
	    return -1;
	}
	attr.cursor = pyg_boxed_get(py_cursor, GdkCursor);
	attr_mask |= GDK_WA_CURSOR;
    }
    if (attr.wmclass_name && attr.wmclass_class)
	attr_mask |= GDK_WA_WMCLASS;
    if (attr.override_redirect != -1)
	attr_mask |= GDK_WA_NOREDIR;

    self->obj = (GObject *)gdk_window_new(parent, &attr, attr_mask);
    if (!self->obj) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not create GdkWindow object");
	return -1;
    }

    g_object_ref(self->obj);

    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 9266 "gdk.c"


#line 818 "./gdk.override"
static PyObject *
_wrap_gdk_drag_begin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "targets", NULL };
    PyObject *py_targets;
    GList *targets = NULL;
    guint i, len;
    GdkDragContext *context;
    PyObject *py_context;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkWindow.drag_begin",
                                     kwlist, &py_targets))
        return NULL;
    if (!PySequence_Check(py_targets)) {
        PyErr_SetString(PyExc_TypeError, "targets must be a list of ints");
        return NULL;
    }
    len = PySequence_Length(py_targets);
    for (i = 0; i < len; i++) {
        PyObject *item = PySequence_GetItem(py_targets, i);

        if (PyInt_Check(item)) {
            targets = g_list_append(targets,
                                    GUINT_TO_POINTER(PyInt_AsLong(item)));
            Py_DECREF(item);
        } else {
            PyErr_SetString(PyExc_TypeError, "targets must be a list of ints");
            Py_DECREF(item);
            g_list_free(targets);
            return NULL;
        }
    }
    context = gdk_drag_begin(GDK_WINDOW(self->obj), targets);
    g_list_free(targets);
    py_context = pygobject_new((GObject *)context);
    gdk_drag_context_unref(context);
    return py_context;
}
#line 9308 "gdk.c"


static PyObject *
_wrap_gdk_input_set_extension_events(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mask", "mode", NULL };
    int mask;
    PyObject *py_mode = NULL;
    GdkExtensionMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO:Gdk.Window.input_set_extension_events", kwlist, &mask, &py_mode))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_EXTENSION_MODE, py_mode, (gpointer)&mode))
        return NULL;
    
    gdk_input_set_extension_events(GDK_WINDOW(self->obj), mask, mode);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_destroy_notify(PyGObject *self)
{
    
    gdk_window_destroy_notify(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 110 "gdkwindow.override"

static GdkAtom atom_type = GDK_NONE, atom_pair_type = GDK_NONE;

static PyObject *
_wrap_gdk_property_get(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property", "type", "pdelete", NULL };
    PyObject *py_property, *py_type = NULL;
    GdkAtom property, type;
    gint pdelete = FALSE;

    GdkAtom atype;
    gint aformat, alength;
    guchar *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|Oi:GdkWindow.property_get", kwlist,
                                     &py_property, &py_type, 
                                     &pdelete)) {
        return NULL;
    }
    property = pygdk_atom_from_pyobject(py_property);
    if (PyErr_Occurred())
        return NULL;

    type = pygdk_atom_from_pyobject(py_type);
    if (PyErr_Occurred())
        return NULL;

    if (gdk_property_get(GDK_WINDOW(self->obj), property, type, 0, 9999,
                         pdelete, &atype, &aformat, &alength, &data)) {
        /* success */
        PyObject *pdata = NULL;
        gint nelements, i;
        gchar *aname;
        PyObject *ret;

        switch (aformat) {
        case 8:
            if ((pdata = PyString_FromStringAndSize((char*)data,
						    alength)) == NULL)
                return NULL;
            break;
        case 16: {
            short *data16;

            data16 = (short *)data;
            nelements = alength / sizeof(short);
            if ((pdata = PyList_New(nelements)) == NULL)
                return NULL;
            for (i = 0; i < nelements; i++)
                PyList_SetItem(pdata, i, PyInt_FromLong(data16[i]));
            break;
        }
        case 32: {
            if (atom_type == GDK_NONE) {
                atom_type = gdk_atom_intern("ATOM", TRUE);
                atom_pair_type = gdk_atom_intern("ATOM_PAIR", TRUE);
            }

            /* special handling for atom types */
            if (atype == atom_type || atype == atom_pair_type) {
                GdkAtom *data32;

                data32 = (GdkAtom *)data;
                nelements = alength / sizeof(GdkAtom);
                if ((pdata = PyList_New(nelements)) == NULL)
                    return NULL;
                for (i = 0; i < nelements; i++) {
                    aname = gdk_atom_name(data32[i]);
                    PyList_SetItem(pdata, i, PyString_FromString(aname));
                    g_free(aname);
                }
            } else {
                long *data32;

                data32 = (long *)data;
                nelements  = alength / sizeof(long);
                if ((pdata = PyList_New(nelements)) == NULL)
                    return NULL;
                for (i = 0; i < nelements; i++)
                    PyList_SetItem(pdata, i, PyInt_FromLong(data32[i]));
            }
            break;
        }
        default:
            g_warning("got a property format != 8, 16 or 32");
            g_assert_not_reached();
        }
        g_free(data);
        aname = gdk_atom_name(atype);
        ret = Py_BuildValue("(NiN)", PyString_FromString(aname), aformat,
                            pdata);
        g_free(aname);
        return ret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 9441 "gdk.c"


#line 212 "gdkwindow.override"
static PyObject *
_wrap_gdk_property_change(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property","type","format","mode","data",NULL };
    PyObject *py_property, *py_type;
    GdkAtom property, type;
    gint format;
    PyObject *py_mode, *pdata;
    GdkPropMode mode;
    guchar *data = NULL;
    gint nelements;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OOiOO:GdkWindow.property_change", kwlist,
                                     &py_property, &py_type, &format, &py_mode,
                                     &pdata)) {
        return NULL;
    }
    property = pygdk_atom_from_pyobject(py_property);
    if (PyErr_Occurred())
        return NULL;

    type = pygdk_atom_from_pyobject(py_type);
    if (PyErr_Occurred())
        return NULL;
    
    if (pyg_enum_get_value(GDK_TYPE_PROP_MODE, py_mode, (gint *)&mode))
        return NULL;
    switch (format) {
    case 8:
        if (!PyString_Check(pdata)) {
            PyErr_SetString(PyExc_TypeError, "data not a string and format=8");
            return NULL;
        }
        data = (guchar*)PyString_AsString(pdata);
        nelements = PyString_Size(pdata);
        break;
    case 16: {
        short *data16;
        gint i;

        if (!PySequence_Check(pdata)) {
            PyErr_SetString(PyExc_TypeError,
                            "data not a sequence and format=16");
            return NULL;
        }
        nelements = PySequence_Length(pdata);
        data16 = g_new(short, nelements);
        data = (guchar *)data16;
        for (i = 0; i < nelements; i++) {
            PyObject *item = PySequence_GetItem(pdata, i);

            if (item) data16[i] = PyInt_AsLong(item);
            if (!item || PyErr_Occurred()) {
                Py_XDECREF(item);
                g_free(data16);
                PyErr_Clear();
                PyErr_SetString(PyExc_TypeError,"data element not an int");
                return NULL;
            }
            Py_DECREF(item);
        }
        break;
    }
    case 32: {
        gint i;

        if (!PySequence_Check(pdata)) {
            PyErr_SetString(PyExc_TypeError,
                            "data not a sequence and format=32");
            return NULL;
        }
        nelements = PySequence_Length(pdata);

        if (atom_type == GDK_NONE) {
            atom_type = gdk_atom_intern("ATOM", TRUE);
            atom_pair_type = gdk_atom_intern("ATOM_PAIR", TRUE);
        }

        /* special handling for atom types */
        if (type == atom_type || type == atom_pair_type) {
            GdkAtom *data32 = g_new(GdkAtom, nelements);

            data = (guchar *)data32;
            for (i = 0; i < nelements; i++) {
                PyObject *item = PySequence_GetItem(pdata, i);

                if (item) data32[i] = pygdk_atom_from_pyobject(item);
                if (!item || PyErr_Occurred()) {
                    Py_XDECREF(item);
                    g_free(data32);
                    PyErr_Clear();
                    PyErr_SetString(PyExc_TypeError,"data element not an atom");
                    return NULL;
                }
                Py_DECREF(item);
            }
        } else {
            long *data32 = g_new(long, nelements);

            data = (guchar *)data32;
            for (i = 0; i < nelements; i++) {
                PyObject *item = PySequence_GetItem(pdata, i);

                if (item) data32[i] = PyInt_AsLong(item);
                if (!item || PyErr_Occurred()) {
                    Py_XDECREF(item);
                    g_free(data32);
                    PyErr_Clear();
                    PyErr_SetString(PyExc_TypeError,"data element not an int");
                    return NULL;
                }
                Py_DECREF(item);
            }
        }
        break;
    }
    default:
        PyErr_SetString(PyExc_TypeError, "format must be 8, 16 or 32");
        return NULL;
        break;
    }
    gdk_property_change(GDK_WINDOW(self->obj), property, type, format, mode,
                        data, nelements);
    if (format != 8)
        g_free(data);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 9574 "gdk.c"


static PyObject *
_wrap_gdk_property_delete(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property", NULL };
    PyObject *py_property = NULL;
    GdkAtom property;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.property_delete", kwlist, &py_property))
        return NULL;
    property = pygdk_atom_from_pyobject(py_property);
    if (PyErr_Occurred())
        return NULL;
    
    gdk_property_delete(GDK_WINDOW(self->obj), property);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_selection_convert(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "selection", "target", "time", NULL };
    PyObject *py_selection = NULL, *py_target = NULL;
    unsigned long time;
    GdkAtom selection, target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOk:Gdk.Window.selection_convert", kwlist, &py_selection, &py_target, &time))
        return NULL;
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    target = pygdk_atom_from_pyobject(py_target);
    if (PyErr_Occurred())
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_selection_convert(GDK_WINDOW(self->obj), selection, target, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_keep_above(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "setting", NULL };
    int setting;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_keep_above", kwlist, &setting))
        return NULL;
    
    gdk_window_set_keep_above(GDK_WINDOW(self->obj), setting);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_keep_below(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "setting", NULL };
    int setting;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_keep_below", kwlist, &setting))
        return NULL;
    
    gdk_window_set_keep_below(GDK_WINDOW(self->obj), setting);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_destroy(PyGObject *self)
{
    
    gdk_window_destroy(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_get_window_type(PyGObject *self)
{
    gint ret;

    
    ret = gdk_window_get_window_type(GDK_WINDOW(self->obj));
    
    return pyg_enum_from_gtype(GDK_TYPE_WINDOW_TYPE, ret);
}

static PyObject *
_wrap_gdk_window_show(PyGObject *self)
{
    
    gdk_window_show(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_hide(PyGObject *self)
{
    
    gdk_window_hide(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_withdraw(PyGObject *self)
{
    
    gdk_window_withdraw(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_move(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Window.move", kwlist, &x, &y))
        return NULL;
    
    gdk_window_move(GDK_WINDOW(self->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_resize(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", NULL };
    int width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Window.resize", kwlist, &width, &height))
        return NULL;
    
    gdk_window_resize(GDK_WINDOW(self->obj), width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_move_resize(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Window.move_resize", kwlist, &x, &y, &width, &height))
        return NULL;
    
    gdk_window_move_resize(GDK_WINDOW(self->obj), x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_reparent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "new_parent", "x", "y", NULL };
    PyGObject *new_parent;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.Window.reparent", kwlist, &PyGdkWindow_Type, &new_parent, &x, &y))
        return NULL;
    
    gdk_window_reparent(GDK_WINDOW(self->obj), GDK_WINDOW(new_parent->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_clear(PyGObject *self)
{
    
    gdk_window_clear(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_clear_area(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Window.clear_area", kwlist, &x, &y, &width, &height))
        return NULL;
    
    gdk_window_clear_area(GDK_WINDOW(self->obj), x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_clear_area_e(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Window.clear_area_e", kwlist, &x, &y, &width, &height))
        return NULL;
    
    gdk_window_clear_area_e(GDK_WINDOW(self->obj), x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_raise(PyGObject *self)
{
    
    gdk_window_raise(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_lower(PyGObject *self)
{
    
    gdk_window_lower(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_focus(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timestamp", NULL };
    unsigned long timestamp = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|k:Gdk.Window.focus", kwlist, &timestamp))
        return NULL;
    if (timestamp > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " timestamp parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_window_focus(GDK_WINDOW(self->obj), timestamp);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 568 "gdkwindow.override"

static void __gdk_window_set_user_data_window_destroyed(gpointer widget,
                                                        GObject *window);

static void
__gdk_window_set_user_data_widget_destroyed(gpointer  window,
                                            GObject  *widget)
{
    gpointer old_user_data = NULL;
    gdk_window_get_user_data(GDK_WINDOW(window), &old_user_data);
    gdk_window_set_user_data(GDK_WINDOW(window), NULL);
    g_object_weak_unref(G_OBJECT(window), __gdk_window_set_user_data_window_destroyed,
                        widget);
}

static void
__gdk_window_set_user_data_window_destroyed(gpointer widget,
                                            GObject *window)
{
    g_object_weak_unref(G_OBJECT(widget), __gdk_window_set_user_data_widget_destroyed,
                        window);
}

static PyObject *
_wrap_gdk_window_set_user_data(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "user_data", NULL };
    PyGObject *user_data;
    gpointer old_user_data = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:GdkWindow.set_user_data",
				     kwlist, &user_data))
        return NULL;

    if ((PyObject *)user_data != Py_None &&
	!pygobject_check(user_data, &PyGtkWidget_Type)) {
	PyErr_SetString(PyExc_TypeError, "Only GtkWidgets are allowed as user data."
                        " Maybe you want to use GObject.set_data()");
	return NULL;
    }
      /* Remove any existing weak references */
    gdk_window_get_user_data(GDK_WINDOW(self->obj), &old_user_data);
    if (old_user_data) {
        g_object_weak_unref(old_user_data, __gdk_window_set_user_data_widget_destroyed, self->obj);
        g_object_weak_unref(self->obj, __gdk_window_set_user_data_window_destroyed, old_user_data);
    }

    if ((PyObject *)user_data != Py_None) {
	  /* Add new weak references */
	gdk_window_set_user_data(GDK_WINDOW(self->obj), GTK_WIDGET(user_data->obj));
	g_object_weak_ref(user_data->obj, __gdk_window_set_user_data_widget_destroyed, self->obj);
	g_object_weak_ref(self->obj, __gdk_window_set_user_data_window_destroyed, user_data->obj);
    } else {
	gdk_window_set_user_data(GDK_WINDOW(self->obj), NULL);
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 9909 "gdk.c"


#line 630 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_user_data(PyGObject *self)
{
    gpointer user_data = NULL;
    gdk_window_get_user_data(GDK_WINDOW(self->obj), &user_data);
    if (GTK_IS_WIDGET(user_data)) {
	return pygobject_new(G_OBJECT(user_data));
    } else {
	PyErr_SetString(PyExc_ValueError, "could not convert the user data");
	return NULL;
    }
}
#line 9925 "gdk.c"


static PyObject *
_wrap_gdk_window_set_override_redirect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "override_redirect", NULL };
    int override_redirect;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_override_redirect", kwlist, &override_redirect))
        return NULL;
    
    gdk_window_set_override_redirect(GDK_WINDOW(self->obj), override_redirect);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 644 "gdkwindow.override"
static int
pygdk_filter_func_marshal(GdkXEvent *xevent,
			  GdkEvent *event,
			  PyGtkCustomNotify *cunote)
{
    PyGILState_STATE state;
    PyObject *ret;
    PyObject *pyevent;
    int retval;
    
    state = pyg_gil_state_ensure();
    
    pyevent = pyg_boxed_new(GDK_TYPE_EVENT, event, TRUE, TRUE);

    if (cunote->data)
        ret = PyObject_CallFunction(cunote->func, "OO", pyevent, cunote->data);

    else
        ret = PyObject_CallFunction(cunote->func, "O", pyevent);
    
    if (ret == NULL) {
        PyErr_Print();
	retval = GDK_FILTER_CONTINUE;
    } else {
	retval = PyInt_AsLong(ret);
    }
    
    pyg_gil_state_release(state);

    return retval;
}
static PyObject*
_wrap_gdk_window_add_filter(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "entries", "callback_data", NULL };
    PyGtkCustomNotify *cunote;
    PyObject *pyfunc, *pydata = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|O:GdkWindow.add_filter", kwlist,
                                     &pyfunc, &pydata))
        return NULL;
     
    
    cunote = g_new0(PyGtkCustomNotify, 1);
    cunote->func = pyfunc;
    cunote->data = pydata;
    Py_INCREF(cunote->func);
    Py_XINCREF(cunote->data);

    g_object_weak_ref(G_OBJECT(self->obj),
		      (GWeakNotify) pygtk_custom_destroy_notify,
		      cunote);
    
    gdk_window_add_filter (GDK_WINDOW(self->obj),
			   (GdkFilterFunc)pygdk_filter_func_marshal,
			   cunote);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 10005 "gdk.c"


static PyObject *
_wrap_gdk_window_scroll(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dx", "dy", NULL };
    int dx, dy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Window.scroll", kwlist, &dx, &dy))
        return NULL;
    
    gdk_window_scroll(GDK_WINDOW(self->obj), dx, dy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_shape_combine_mask(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_mask", "offset_x", "offset_y", NULL };
    PyGObject *py_shape_mask;
    int offset_x, offset_y;
    GdkPixmap *shape_mask = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Gdk.Window.shape_combine_mask", kwlist, &py_shape_mask, &offset_x, &offset_y))
        return NULL;
    if (py_shape_mask && pygobject_check(py_shape_mask, &PyGdkPixmap_Type))
        shape_mask = GDK_PIXMAP(py_shape_mask->obj);
    else if ((PyObject *)py_shape_mask != Py_None) {
        PyErr_SetString(PyExc_TypeError, "shape_mask should be a GdkPixmap or None");
        return NULL;
    }
    
    gdk_window_shape_combine_mask(GDK_WINDOW(self->obj), (GdkBitmap *) shape_mask, offset_x, offset_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_child_shapes(PyGObject *self)
{
    
    gdk_window_set_child_shapes(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_merge_child_shapes(PyGObject *self)
{
    
    gdk_window_merge_child_shapes(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_is_visible(PyGObject *self)
{
    int ret;

    
    ret = gdk_window_is_visible(GDK_WINDOW(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_window_is_viewable(PyGObject *self)
{
    int ret;

    
    ret = gdk_window_is_viewable(GDK_WINDOW(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_window_get_state(PyGObject *self)
{
    guint ret;

    
    ret = gdk_window_get_state(GDK_WINDOW(self->obj));
    
    return pyg_flags_from_gtype(GDK_TYPE_WINDOW_STATE, ret);
}

static PyObject *
_wrap_gdk_window_set_static_gravities(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "use_static", NULL };
    int use_static, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_static_gravities", kwlist, &use_static))
        return NULL;
    
    ret = gdk_window_set_static_gravities(GDK_WINDOW(self->obj), use_static);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_window_set_hints(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "min_width", "min_height", "max_width", "max_height", "flags", NULL };
    int x, y, min_width, min_height, max_width, max_height, flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiiiiii:Gdk.Window.set_hints", kwlist, &x, &y, &min_width, &min_height, &max_width, &max_height, &flags))
        return NULL;
    
    gdk_window_set_hints(GDK_WINDOW(self->obj), x, y, min_width, min_height, max_width, max_height, flags);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_type_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "hint", NULL };
    PyObject *py_hint = NULL;
    GdkWindowTypeHint hint;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_type_hint", kwlist, &py_hint))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_WINDOW_TYPE_HINT, py_hint, (gpointer)&hint))
        return NULL;
    
    gdk_window_set_type_hint(GDK_WINDOW(self->obj), hint);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_modal_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "modal", NULL };
    int modal;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_modal_hint", kwlist, &modal))
        return NULL;
    
    gdk_window_set_modal_hint(GDK_WINDOW(self->obj), modal);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_skip_taskbar_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "modal", NULL };
    int modal;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_skip_taskbar_hint", kwlist, &modal))
        return NULL;
    
    gdk_window_set_skip_taskbar_hint(GDK_WINDOW(self->obj), modal);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_skip_pager_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "modal", NULL };
    int modal;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_skip_pager_hint", kwlist, &modal))
        return NULL;
    
    gdk_window_set_skip_pager_hint(GDK_WINDOW(self->obj), modal);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 343 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_set_geometry_hints(PyGObject *self, PyObject *args,
                                    PyObject *kwargs)
{
    static char *kwlist[] = { "min_width", "min_height",
                              "max_width", "max_height", "base_width",
                              "base_height", "width_inc", "height_inc",
                              "min_aspect", "max_aspect", NULL };
    gint min_width = -1, min_height = -1, max_width = -1, max_height = -1;
    gint base_width = -1, base_height = -1, width_inc = -1, height_inc = -1;
    gdouble min_aspect = -1.0, max_aspect = -1.0;
    GdkGeometry geometry = { 0 };
    GdkWindowHints geom_mask = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|iiiiiiiidd:GdkWindow.set_geometry_hints",
				     kwlist, &min_width, &min_height,
				     &max_width, &max_height,
                                     &base_width, &base_height, &width_inc,
                                     &height_inc, &min_aspect, &max_aspect))
        return NULL;
    if (min_width >= 0 || min_height >= 0) {
        geometry.min_width = MAX(min_width, 0);
        geometry.min_height = MAX(min_height, 0);
        geom_mask |= GDK_HINT_MIN_SIZE;
    }
    if (max_width >= 0 || max_height >= 0) {
        geometry.max_width = MAX(max_width, 0);
        geometry.max_height = MAX(max_height, 0);
        geom_mask |= GDK_HINT_MAX_SIZE;
    }
    if (base_width >= 0 || base_height >= 0) {
        geometry.base_width = MAX(base_width, 0);
        geometry.base_height = MAX(base_height, 0);
        geom_mask |= GDK_HINT_BASE_SIZE;
    }
    if (width_inc >= 0 || height_inc >= 0) {
        geometry.width_inc = MAX(width_inc, 0);
        geometry.height_inc = MAX(height_inc, 0);
        geom_mask |= GDK_HINT_RESIZE_INC;
    }
    if (min_aspect >= 0.0 || max_aspect >= 0.0) {
        if (min_aspect <= 0.0 || max_aspect <= 0.0) {
            PyErr_SetString(PyExc_TypeError, "aspect ratios must be positive");
            return NULL;
        }
        geometry.min_aspect = min_aspect;
        geometry.max_aspect = max_aspect;
        geom_mask |= GDK_HINT_ASPECT;
    }
    gdk_window_set_geometry_hints(GDK_WINDOW(self->obj), &geometry, geom_mask);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 10249 "gdk.c"


static PyObject *
_wrap_gdk_window_begin_paint_rect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rectangle", NULL };
    PyObject *py_rectangle;
    GdkRectangle rectangle = { 0, 0, 0, 0 };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.begin_paint_rect", kwlist, &py_rectangle))
        return NULL;
    if (!pygdk_rectangle_from_pyobject(py_rectangle, &rectangle))
        return NULL;
    
    gdk_window_begin_paint_rect(GDK_WINDOW(self->obj), &rectangle);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_begin_paint_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "region", NULL };
    PyObject *py_region;
    GdkRegion *region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.begin_paint_region", kwlist, &py_region))
        return NULL;
    if (pyg_boxed_check(py_region, PYGDK_TYPE_REGION))
        region = pyg_boxed_get(py_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "region should be a GdkRegion");
        return NULL;
    }
    
    gdk_window_begin_paint_region(GDK_WINDOW(self->obj), region);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_end_paint(PyGObject *self)
{
    
    gdk_window_end_paint(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_title(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "title", NULL };
    char *title;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Window.set_title", kwlist, &title))
        return NULL;
    
    gdk_window_set_title(GDK_WINDOW(self->obj), title);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_role(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "role", NULL };
    char *role;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Window.set_role", kwlist, &role))
        return NULL;
    
    gdk_window_set_role(GDK_WINDOW(self->obj), role);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_transient_for(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "leader", NULL };
    PyGObject *leader;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Window.set_transient_for", kwlist, &PyGdkWindow_Type, &leader))
        return NULL;
    
    gdk_window_set_transient_for(GDK_WINDOW(self->obj), GDK_WINDOW(leader->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_background(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_background", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_window_set_background(GDK_WINDOW(self->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_back_pixmap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixmap", "parent_relative", NULL };
    PyGObject *py_pixmap;
    int parent_relative;
    GdkPixmap *pixmap = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Gdk.Window.set_back_pixmap", kwlist, &py_pixmap, &parent_relative))
        return NULL;
    if (py_pixmap && pygobject_check(py_pixmap, &PyGdkPixmap_Type))
        pixmap = GDK_PIXMAP(py_pixmap->obj);
    else if ((PyObject *)py_pixmap != Py_None) {
        PyErr_SetString(PyExc_TypeError, "pixmap should be a GdkPixmap or None");
        return NULL;
    }
    
    gdk_window_set_back_pixmap(GDK_WINDOW(self->obj), (GdkPixmap *) pixmap, parent_relative);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_cursor(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cursor", NULL };
    PyObject *py_cursor = Py_None;
    GdkCursor *cursor = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_cursor", kwlist, &py_cursor))
        return NULL;
    if (pyg_boxed_check(py_cursor, GDK_TYPE_CURSOR))
        cursor = pyg_boxed_get(py_cursor, GdkCursor);
    else if (py_cursor != Py_None) {
        PyErr_SetString(PyExc_TypeError, "cursor should be a GdkCursor or None");
        return NULL;
    }
    
    gdk_window_set_cursor(GDK_WINDOW(self->obj), cursor);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 399 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_geometry(PyGObject *self)
{
    gint x, y, width, height, depth;

    gdk_window_get_geometry(GDK_WINDOW(self->obj), &x, &y, &width, &height,
                            &depth);
    return Py_BuildValue("(iiiii)", x, y, width, height, depth);
}
#line 10424 "gdk.c"


#line 410 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_position(PyGObject *self)
{
    gint x, y;

    gdk_window_get_position(GDK_WINDOW(self->obj), &x, &y);
    return Py_BuildValue("(ii)", x, y);
}
#line 10436 "gdk.c"


#line 420 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_origin(PyGObject *self)
{
    gint x, y;

    gdk_window_get_origin(GDK_WINDOW(self->obj), &x, &y);
    return Py_BuildValue("(ii)", x, y);
}
#line 10448 "gdk.c"


#line 430 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_deskrelative_origin(PyGObject *self)
{
    gint x, y;

    gdk_window_get_deskrelative_origin(GDK_WINDOW(self->obj), &x, &y);
    return Py_BuildValue("(ii)", x, y);
}
#line 10460 "gdk.c"


#line 440 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_root_origin(PyGObject *self)
{
    gint x, y;

    gdk_window_get_root_origin(GDK_WINDOW(self->obj), &x, &y);
    return Py_BuildValue("(ii)", x, y);
}
#line 10472 "gdk.c"


#line 519 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_frame_extents(PyGObject *self)
{
    GdkRectangle rect = {0, 0, 0, 0};

    gdk_window_get_frame_extents(GDK_WINDOW(self->obj), &rect);

    return pyg_boxed_new(GDK_TYPE_RECTANGLE, &rect, TRUE, TRUE);
}
#line 10485 "gdk.c"


#line 450 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_pointer(PyGObject *self)
{
    gint x, y;
    GdkModifierType mask;

    gdk_window_get_pointer(GDK_WINDOW(self->obj), &x, &y, &mask);
    return Py_BuildValue("(iiN)", x, y,
                         pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE, mask));
}
#line 10499 "gdk.c"


static PyObject *
_wrap_gdk_window_get_parent(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_window_get_parent(GDK_WINDOW(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_window_get_toplevel(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_window_get_toplevel(GDK_WINDOW(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 464 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_children(PyGObject *self)
{
    GList *children, *tmp;
    PyObject *list;

    children = gdk_window_get_children(GDK_WINDOW(self->obj));
    list = PyList_New(0);
    for (tmp = children; tmp != NULL; tmp = tmp->next) {
        PyObject *item = pygobject_new((GObject *)tmp->data);

        PyList_Append(list, item);
        Py_DECREF(item);
    }
    g_list_free(children);
    return list;
}
#line 10544 "gdk.c"


static PyObject *
_wrap_gdk_window_get_events(PyGObject *self)
{
    guint ret;

    
    ret = gdk_window_get_events(GDK_WINDOW(self->obj));
    
    return pyg_flags_from_gtype(GDK_TYPE_EVENT_MASK, ret);
}

static PyObject *
_wrap_gdk_window_set_events(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event_mask", NULL };
    PyObject *py_event_mask = NULL;
    GdkEventMask event_mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_events", kwlist, &py_event_mask))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_EVENT_MASK, py_event_mask, (gpointer)&event_mask))
        return NULL;
    
    gdk_window_set_events(GDK_WINDOW(self->obj), event_mask);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 483 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_set_icon_list(PyGObject *self, PyObject *args,
			       PyObject *kwargs)
{
    static char *kwlist[] = { "pixbufs", NULL };
    PyObject *py_pixbufs;
    gint length, i;
    GList *pixbufs = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkWindow.set_icon_list",
				     kwlist, &py_pixbufs))
	return NULL;
    if (!(py_pixbufs = PySequence_Fast(py_pixbufs,
				"pixbufs must be a sequence of pixbufs")))
	return NULL;
    length = PySequence_Fast_GET_SIZE(py_pixbufs);
    for (i = length - 1; i >= 0; i--) {
	PyObject *item = PySequence_Fast_GET_ITEM(py_pixbufs, i);

	if (!pygobject_check(item, &PyGdkPixbuf_Type)) {
	    g_list_free(pixbufs);
	    Py_DECREF(py_pixbufs);
	    PyErr_SetString(PyExc_TypeError,
			    "pixbufs must be a sequence of pixbufs");
	    return NULL;
	}
	pixbufs = g_list_prepend(pixbufs, GDK_PIXBUF(pygobject_get(item)));
    }
    gdk_window_set_icon_list(GDK_WINDOW(self->obj), pixbufs);
    g_list_free(pixbufs);
    Py_DECREF(py_pixbufs);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 10611 "gdk.c"


static PyObject *
_wrap_gdk_window_set_icon(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon_window", "pixmap", "mask", NULL };
    PyGObject *icon_window, *pixmap, *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:Gdk.Window.set_icon", kwlist, &PyGdkWindow_Type, &icon_window, &PyGdkPixmap_Type, &pixmap, &PyGdkPixmap_Type, &mask))
        return NULL;
    
    gdk_window_set_icon(GDK_WINDOW(self->obj), GDK_WINDOW(icon_window->obj), GDK_PIXMAP(pixmap->obj), GDK_PIXMAP(mask->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_icon_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Window.set_icon_name", kwlist, &name))
        return NULL;
    
    gdk_window_set_icon_name(GDK_WINDOW(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_group(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "leader", NULL };
    PyGObject *leader;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Window.set_group", kwlist, &PyGdkWindow_Type, &leader))
        return NULL;
    
    gdk_window_set_group(GDK_WINDOW(self->obj), GDK_WINDOW(leader->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_get_group(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_window_get_group(GDK_WINDOW(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_window_set_decorations(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "decorations", NULL };
    PyObject *py_decorations = NULL;
    GdkWMDecoration decorations;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_decorations", kwlist, &py_decorations))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_WM_DECORATION, py_decorations, (gpointer)&decorations))
        return NULL;
    
    gdk_window_set_decorations(GDK_WINDOW(self->obj), decorations);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 530 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_decorations(PyGObject *self)
{
    GdkWMDecoration decor = 0; 

    gdk_window_get_decorations(GDK_WINDOW(self->obj), &decor);

    return pyg_flags_from_gtype(GDK_TYPE_WM_DECORATION, decor);
}
#line 10699 "gdk.c"


static PyObject *
_wrap_gdk_window_set_functions(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "functions", NULL };
    PyObject *py_functions = NULL;
    GdkWMFunction functions;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Window.set_functions", kwlist, &py_functions))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_WM_FUNCTION, py_functions, (gpointer)&functions))
        return NULL;
    
    gdk_window_set_functions(GDK_WINDOW(self->obj), functions);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_iconify(PyGObject *self)
{
    
    gdk_window_iconify(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_deiconify(PyGObject *self)
{
    
    gdk_window_deiconify(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_stick(PyGObject *self)
{
    
    gdk_window_stick(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_unstick(PyGObject *self)
{
    
    gdk_window_unstick(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_maximize(PyGObject *self)
{
    
    gdk_window_maximize(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_unmaximize(PyGObject *self)
{
    
    gdk_window_unmaximize(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_fullscreen(PyGObject *self)
{
    
    gdk_window_fullscreen(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_unfullscreen(PyGObject *self)
{
    
    gdk_window_unfullscreen(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_register_dnd(PyGObject *self)
{
    
    gdk_window_register_dnd(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_begin_resize_drag(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "edge", "button", "root_x", "root_y", "timestamp", NULL };
    PyObject *py_edge = NULL;
    int button, root_x, root_y;
    unsigned long timestamp;
    GdkWindowEdge edge;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oiiik:Gdk.Window.begin_resize_drag", kwlist, &py_edge, &button, &root_x, &root_y, &timestamp))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_WINDOW_EDGE, py_edge, (gpointer)&edge))
        return NULL;
    if (timestamp > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " timestamp parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_window_begin_resize_drag(GDK_WINDOW(self->obj), edge, button, root_x, root_y, timestamp);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_begin_move_drag(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "button", "root_x", "root_y", "timestamp", NULL };
    int button, root_x, root_y;
    unsigned long timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiik:Gdk.Window.begin_move_drag", kwlist, &button, &root_x, &root_y, &timestamp))
        return NULL;
    if (timestamp > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " timestamp parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_window_begin_move_drag(GDK_WINDOW(self->obj), button, root_x, root_y, timestamp);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_invalidate_rect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", "invalidate_children", NULL };
    PyObject *py_rect = Py_None;
    int invalidate_children;
    GdkRectangle rect_rect = { 0, 0, 0, 0 }, *rect;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Gdk.Window.invalidate_rect", kwlist, &py_rect, &invalidate_children))
        return NULL;
    if (py_rect == Py_None)
        rect = NULL;
    else if (pygdk_rectangle_from_pyobject(py_rect, &rect_rect))
        rect = &rect_rect;
    else
            return NULL;
    
    gdk_window_invalidate_rect(GDK_WINDOW(self->obj), rect, invalidate_children);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_invalidate_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "region", "invalidate_children", NULL };
    PyObject *py_region;
    int invalidate_children;
    GdkRegion *region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Gdk.Window.invalidate_region", kwlist, &py_region, &invalidate_children))
        return NULL;
    if (pyg_boxed_check(py_region, PYGDK_TYPE_REGION))
        region = pyg_boxed_get(py_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "region should be a GdkRegion");
        return NULL;
    }
    
    gdk_window_invalidate_region(GDK_WINDOW(self->obj), region, invalidate_children);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_get_update_area(PyGObject *self)
{
    GdkRegion *ret;

    
    ret = gdk_window_get_update_area(GDK_WINDOW(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gdk_window_freeze_updates(PyGObject *self)
{
    
    gdk_window_freeze_updates(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_thaw_updates(PyGObject *self)
{
    
    gdk_window_thaw_updates(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_process_updates(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "update_children", NULL };
    int update_children;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.process_updates", kwlist, &update_children))
        return NULL;
    
    gdk_window_process_updates(GDK_WINDOW(self->obj), update_children);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_accept_focus(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "accept_focus", NULL };
    int accept_focus;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_accept_focus", kwlist, &accept_focus))
        return NULL;
    
    gdk_window_set_accept_focus(GDK_WINDOW(self->obj), accept_focus);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_focus_on_map(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "focus_on_map", NULL };
    int focus_on_map;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_focus_on_map", kwlist, &focus_on_map))
        return NULL;
    
    gdk_window_set_focus_on_map(GDK_WINDOW(self->obj), focus_on_map);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_enable_synchronized_configure(PyGObject *self)
{
    
    gdk_window_enable_synchronized_configure(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_configure_finished(PyGObject *self)
{
    
    gdk_window_configure_finished(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_urgency_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "urgent", NULL };
    int urgent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_urgency_hint", kwlist, &urgent))
        return NULL;
    
    gdk_window_set_urgency_hint(GDK_WINDOW(self->obj), urgent);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_show_unraised(PyGObject *self)
{
    
    gdk_window_show_unraised(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_move_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "region", "dx", "dy", NULL };
    PyObject *py_region;
    int dx, dy;
    GdkRegion *region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Gdk.Window.move_region", kwlist, &py_region, &dx, &dy))
        return NULL;
    if (pyg_boxed_check(py_region, PYGDK_TYPE_REGION))
        region = pyg_boxed_get(py_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "region should be a GdkRegion");
        return NULL;
    }
    
    gdk_window_move_region(GDK_WINDOW(self->obj), region, dx, dy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_shape_combine_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_region", "offset_x", "offset_y", NULL };
    PyObject *py_shape_region = Py_None;
    int offset_x, offset_y;
    GdkRegion *shape_region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Gdk.Window.shape_combine_region", kwlist, &py_shape_region, &offset_x, &offset_y))
        return NULL;
    if (pyg_boxed_check(py_shape_region, PYGDK_TYPE_REGION))
        shape_region = pyg_boxed_get(py_shape_region, GdkRegion);
    else if (py_shape_region != Py_None) {
        PyErr_SetString(PyExc_TypeError, "shape_region should be a GdkRegion or None");
        return NULL;
    }
    
    gdk_window_shape_combine_region(GDK_WINDOW(self->obj), shape_region, offset_x, offset_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_input_shape_combine_mask(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mask", "x", "y", NULL };
    PyGObject *mask;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.Window.input_shape_combine_mask", kwlist, &PyGdkPixmap_Type, &mask, &x, &y))
        return NULL;
    
    gdk_window_input_shape_combine_mask(GDK_WINDOW(self->obj), GDK_PIXMAP(mask->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_input_shape_combine_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shape_region", "offset_x", "offset_y", NULL };
    PyObject *py_shape_region;
    int offset_x, offset_y;
    GdkRegion *shape_region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Gdk.Window.input_shape_combine_region", kwlist, &py_shape_region, &offset_x, &offset_y))
        return NULL;
    if (pyg_boxed_check(py_shape_region, PYGDK_TYPE_REGION))
        shape_region = pyg_boxed_get(py_shape_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "shape_region should be a GdkRegion");
        return NULL;
    }
    
    gdk_window_input_shape_combine_region(GDK_WINDOW(self->obj), shape_region, offset_x, offset_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_child_input_shapes(PyGObject *self)
{
    
    gdk_window_set_child_input_shapes(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_merge_child_input_shapes(PyGObject *self)
{
    
    gdk_window_merge_child_input_shapes(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_get_type_hint(PyGObject *self)
{
    gint ret;

    
    ret = gdk_window_get_type_hint(GDK_WINDOW(self->obj));
    
    return pyg_enum_from_gtype(GDK_TYPE_WINDOW_TYPE_HINT, ret);
}

static PyObject *
_wrap_gdk_window_set_composited(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "composited", NULL };
    int composited;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Window.set_composited", kwlist, &composited))
        return NULL;
    
    gdk_window_set_composited(GDK_WINDOW(self->obj), composited);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_startup_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "startup_id", NULL };
    char *startup_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Window.set_startup_id", kwlist, &startup_id))
        return NULL;
    
    gdk_window_set_startup_id(GDK_WINDOW(self->obj), startup_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_beep(PyGObject *self)
{
    
    gdk_window_beep(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_set_opacity(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "opacity", NULL };
    double opacity;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:Gdk.Window.set_opacity", kwlist, &opacity))
        return NULL;
    
    gdk_window_set_opacity(GDK_WINDOW(self->obj), opacity);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_redirect_to_drawable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "src_x", "src_y", "dest_x", "dest_y", "width", "height", NULL };
    PyGObject *drawable;
    int src_x, src_y, dest_x, dest_y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiii:Gdk.Window.redirect_to_drawable", kwlist, &PyGdkDrawable_Type, &drawable, &src_x, &src_y, &dest_x, &dest_y, &width, &height))
        return NULL;
    
    gdk_window_redirect_to_drawable(GDK_WINDOW(self->obj), GDK_DRAWABLE(drawable->obj), src_x, src_y, dest_x, dest_y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_window_remove_redirection(PyGObject *self)
{
    
    gdk_window_remove_redirection(GDK_WINDOW(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkWindow_methods[] = {
    { "drag_begin", (PyCFunction)_wrap_gdk_drag_begin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "input_set_extension_events", (PyCFunction)_wrap_gdk_input_set_extension_events, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "destroy_notify", (PyCFunction)_wrap_gdk_window_destroy_notify, METH_NOARGS,
      NULL },
    { "property_get", (PyCFunction)_wrap_gdk_property_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "property_change", (PyCFunction)_wrap_gdk_property_change, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "property_delete", (PyCFunction)_wrap_gdk_property_delete, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_convert", (PyCFunction)_wrap_gdk_selection_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_keep_above", (PyCFunction)_wrap_gdk_window_set_keep_above, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_keep_below", (PyCFunction)_wrap_gdk_window_set_keep_below, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "destroy", (PyCFunction)_wrap_gdk_window_destroy, METH_NOARGS,
      NULL },
    { "get_window_type", (PyCFunction)_wrap_gdk_window_get_window_type, METH_NOARGS,
      NULL },
    { "show", (PyCFunction)_wrap_gdk_window_show, METH_NOARGS,
      NULL },
    { "hide", (PyCFunction)_wrap_gdk_window_hide, METH_NOARGS,
      NULL },
    { "withdraw", (PyCFunction)_wrap_gdk_window_withdraw, METH_NOARGS,
      NULL },
    { "move", (PyCFunction)_wrap_gdk_window_move, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "resize", (PyCFunction)_wrap_gdk_window_resize, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "move_resize", (PyCFunction)_wrap_gdk_window_move_resize, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "reparent", (PyCFunction)_wrap_gdk_window_reparent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear", (PyCFunction)_wrap_gdk_window_clear, METH_NOARGS,
      NULL },
    { "clear_area", (PyCFunction)_wrap_gdk_window_clear_area, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_area_e", (PyCFunction)_wrap_gdk_window_clear_area_e, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "raise_", (PyCFunction)_wrap_gdk_window_raise, METH_NOARGS,
      NULL },
    { "lower", (PyCFunction)_wrap_gdk_window_lower, METH_NOARGS,
      NULL },
    { "focus", (PyCFunction)_wrap_gdk_window_focus, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_user_data", (PyCFunction)_wrap_gdk_window_set_user_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_user_data", (PyCFunction)_wrap_gdk_window_get_user_data, METH_NOARGS,
      NULL },
    { "set_override_redirect", (PyCFunction)_wrap_gdk_window_set_override_redirect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_filter", (PyCFunction)_wrap_gdk_window_add_filter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scroll", (PyCFunction)_wrap_gdk_window_scroll, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "shape_combine_mask", (PyCFunction)_wrap_gdk_window_shape_combine_mask, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_child_shapes", (PyCFunction)_wrap_gdk_window_set_child_shapes, METH_NOARGS,
      NULL },
    { "merge_child_shapes", (PyCFunction)_wrap_gdk_window_merge_child_shapes, METH_NOARGS,
      NULL },
    { "is_visible", (PyCFunction)_wrap_gdk_window_is_visible, METH_NOARGS,
      NULL },
    { "is_viewable", (PyCFunction)_wrap_gdk_window_is_viewable, METH_NOARGS,
      NULL },
    { "get_state", (PyCFunction)_wrap_gdk_window_get_state, METH_NOARGS,
      NULL },
    { "set_static_gravities", (PyCFunction)_wrap_gdk_window_set_static_gravities, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_hints", (PyCFunction)_wrap_gdk_window_set_hints, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_type_hint", (PyCFunction)_wrap_gdk_window_set_type_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_modal_hint", (PyCFunction)_wrap_gdk_window_set_modal_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_skip_taskbar_hint", (PyCFunction)_wrap_gdk_window_set_skip_taskbar_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_skip_pager_hint", (PyCFunction)_wrap_gdk_window_set_skip_pager_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_geometry_hints", (PyCFunction)_wrap_gdk_window_set_geometry_hints, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "begin_paint_rect", (PyCFunction)_wrap_gdk_window_begin_paint_rect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "begin_paint_region", (PyCFunction)_wrap_gdk_window_begin_paint_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "end_paint", (PyCFunction)_wrap_gdk_window_end_paint, METH_NOARGS,
      NULL },
    { "set_title", (PyCFunction)_wrap_gdk_window_set_title, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_role", (PyCFunction)_wrap_gdk_window_set_role, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_transient_for", (PyCFunction)_wrap_gdk_window_set_transient_for, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_background", (PyCFunction)_wrap_gdk_window_set_background, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_back_pixmap", (PyCFunction)_wrap_gdk_window_set_back_pixmap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_cursor", (PyCFunction)_wrap_gdk_window_set_cursor, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_geometry", (PyCFunction)_wrap_gdk_window_get_geometry, METH_NOARGS,
      NULL },
    { "get_position", (PyCFunction)_wrap_gdk_window_get_position, METH_NOARGS,
      NULL },
    { "get_origin", (PyCFunction)_wrap_gdk_window_get_origin, METH_NOARGS,
      NULL },
    { "get_deskrelative_origin", (PyCFunction)_wrap_gdk_window_get_deskrelative_origin, METH_NOARGS,
      NULL },
    { "get_root_origin", (PyCFunction)_wrap_gdk_window_get_root_origin, METH_NOARGS,
      NULL },
    { "get_frame_extents", (PyCFunction)_wrap_gdk_window_get_frame_extents, METH_NOARGS,
      NULL },
    { "get_pointer", (PyCFunction)_wrap_gdk_window_get_pointer, METH_NOARGS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_gdk_window_get_parent, METH_NOARGS,
      NULL },
    { "get_toplevel", (PyCFunction)_wrap_gdk_window_get_toplevel, METH_NOARGS,
      NULL },
    { "get_children", (PyCFunction)_wrap_gdk_window_get_children, METH_NOARGS,
      NULL },
    { "get_events", (PyCFunction)_wrap_gdk_window_get_events, METH_NOARGS,
      NULL },
    { "set_events", (PyCFunction)_wrap_gdk_window_set_events, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_list", (PyCFunction)_wrap_gdk_window_set_icon_list, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon", (PyCFunction)_wrap_gdk_window_set_icon, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon_name", (PyCFunction)_wrap_gdk_window_set_icon_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_group", (PyCFunction)_wrap_gdk_window_set_group, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_group", (PyCFunction)_wrap_gdk_window_get_group, METH_NOARGS,
      NULL },
    { "set_decorations", (PyCFunction)_wrap_gdk_window_set_decorations, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_decorations", (PyCFunction)_wrap_gdk_window_get_decorations, METH_NOARGS,
      NULL },
    { "set_functions", (PyCFunction)_wrap_gdk_window_set_functions, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "iconify", (PyCFunction)_wrap_gdk_window_iconify, METH_NOARGS,
      NULL },
    { "deiconify", (PyCFunction)_wrap_gdk_window_deiconify, METH_NOARGS,
      NULL },
    { "stick", (PyCFunction)_wrap_gdk_window_stick, METH_NOARGS,
      NULL },
    { "unstick", (PyCFunction)_wrap_gdk_window_unstick, METH_NOARGS,
      NULL },
    { "maximize", (PyCFunction)_wrap_gdk_window_maximize, METH_NOARGS,
      NULL },
    { "unmaximize", (PyCFunction)_wrap_gdk_window_unmaximize, METH_NOARGS,
      NULL },
    { "fullscreen", (PyCFunction)_wrap_gdk_window_fullscreen, METH_NOARGS,
      NULL },
    { "unfullscreen", (PyCFunction)_wrap_gdk_window_unfullscreen, METH_NOARGS,
      NULL },
    { "register_dnd", (PyCFunction)_wrap_gdk_window_register_dnd, METH_NOARGS,
      NULL },
    { "begin_resize_drag", (PyCFunction)_wrap_gdk_window_begin_resize_drag, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "begin_move_drag", (PyCFunction)_wrap_gdk_window_begin_move_drag, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "invalidate_rect", (PyCFunction)_wrap_gdk_window_invalidate_rect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "invalidate_region", (PyCFunction)_wrap_gdk_window_invalidate_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_update_area", (PyCFunction)_wrap_gdk_window_get_update_area, METH_NOARGS,
      NULL },
    { "freeze_updates", (PyCFunction)_wrap_gdk_window_freeze_updates, METH_NOARGS,
      NULL },
    { "thaw_updates", (PyCFunction)_wrap_gdk_window_thaw_updates, METH_NOARGS,
      NULL },
    { "process_updates", (PyCFunction)_wrap_gdk_window_process_updates, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_accept_focus", (PyCFunction)_wrap_gdk_window_set_accept_focus, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_focus_on_map", (PyCFunction)_wrap_gdk_window_set_focus_on_map, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enable_synchronized_configure", (PyCFunction)_wrap_gdk_window_enable_synchronized_configure, METH_NOARGS,
      NULL },
    { "configure_finished", (PyCFunction)_wrap_gdk_window_configure_finished, METH_NOARGS,
      NULL },
    { "set_urgency_hint", (PyCFunction)_wrap_gdk_window_set_urgency_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "show_unraised", (PyCFunction)_wrap_gdk_window_show_unraised, METH_NOARGS,
      NULL },
    { "move_region", (PyCFunction)_wrap_gdk_window_move_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "shape_combine_region", (PyCFunction)_wrap_gdk_window_shape_combine_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "input_shape_combine_mask", (PyCFunction)_wrap_gdk_window_input_shape_combine_mask, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "input_shape_combine_region", (PyCFunction)_wrap_gdk_window_input_shape_combine_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_child_input_shapes", (PyCFunction)_wrap_gdk_window_set_child_input_shapes, METH_NOARGS,
      NULL },
    { "merge_child_input_shapes", (PyCFunction)_wrap_gdk_window_merge_child_input_shapes, METH_NOARGS,
      NULL },
    { "get_type_hint", (PyCFunction)_wrap_gdk_window_get_type_hint, METH_NOARGS,
      NULL },
    { "set_composited", (PyCFunction)_wrap_gdk_window_set_composited, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_startup_id", (PyCFunction)_wrap_gdk_window_set_startup_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "beep", (PyCFunction)_wrap_gdk_window_beep, METH_NOARGS,
      NULL },
    { "set_opacity", (PyCFunction)_wrap_gdk_window_set_opacity, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "redirect_to_drawable", (PyCFunction)_wrap_gdk_window_redirect_to_drawable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_redirection", (PyCFunction)_wrap_gdk_window_remove_redirection, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkWindow_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Window",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkWindow_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_window_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPangoRenderer ----------- */

static int
_wrap_gdk_pango_renderer_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", NULL };
    PyGObject *screen;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.PangoRenderer.__init__", kwlist, &PyGdkScreen_Type, &screen))
        return -1;
    self->obj = (GObject *)gdk_pango_renderer_new(GDK_SCREEN(screen->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkPangoRenderer object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_pango_renderer_set_drawable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", NULL };
    PyGObject *py_drawable;
    GdkDrawable *drawable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.PangoRenderer.set_drawable", kwlist, &py_drawable))
        return NULL;
    if (py_drawable && pygobject_check(py_drawable, &PyGdkDrawable_Type))
        drawable = GDK_DRAWABLE(py_drawable->obj);
    else if ((PyObject *)py_drawable != Py_None) {
        PyErr_SetString(PyExc_TypeError, "drawable should be a GdkDrawable or None");
        return NULL;
    }
    
    gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(self->obj), (GdkDrawable *) drawable);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pango_renderer_set_gc(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", NULL };
    PyGObject *py_gc;
    GdkGC *gc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.PangoRenderer.set_gc", kwlist, &py_gc))
        return NULL;
    if (py_gc && pygobject_check(py_gc, &PyGdkGC_Type))
        gc = GDK_GC(py_gc->obj);
    else if ((PyObject *)py_gc != Py_None) {
        PyErr_SetString(PyExc_TypeError, "gc should be a GdkGC or None");
        return NULL;
    }
    
    gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(self->obj), (GdkGC *) gc);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pango_renderer_set_stipple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", "stipple", NULL };
    PyObject *py_part = NULL;
    PyGObject *py_stipple;
    PangoRenderPart part;
    GdkPixmap *stipple = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Gdk.PangoRenderer.set_stipple", kwlist, &py_part, &py_stipple))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    if (py_stipple && pygobject_check(py_stipple, &PyGdkPixmap_Type))
        stipple = GDK_PIXMAP(py_stipple->obj);
    else if ((PyObject *)py_stipple != Py_None) {
        PyErr_SetString(PyExc_TypeError, "stipple should be a GdkPixmap or None");
        return NULL;
    }
    
    gdk_pango_renderer_set_stipple(GDK_PANGO_RENDERER(self->obj), part, (GdkBitmap *) stipple);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pango_renderer_set_override_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", "color", NULL };
    PyObject *py_part = NULL, *py_color = Py_None;
    PangoRenderPart part;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Gdk.PangoRenderer.set_override_color", kwlist, &py_part, &py_color))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else if (py_color != Py_None) {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor or None");
        return NULL;
    }
    
    gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(self->obj), part, color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkPangoRenderer_methods[] = {
    { "set_drawable", (PyCFunction)_wrap_gdk_pango_renderer_set_drawable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_gc", (PyCFunction)_wrap_gdk_pango_renderer_set_gc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_stipple", (PyCFunction)_wrap_gdk_pango_renderer_set_stipple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_override_color", (PyCFunction)_wrap_gdk_pango_renderer_set_override_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkPangoRenderer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PangoRenderer",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPangoRenderer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pango_renderer_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPixmap ----------- */

static int
_wrap_gdk_pixmap_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "width", "height", "depth", NULL };
    PyGObject *py_drawable;
    GdkDrawable *drawable = NULL;
    int width, height, depth = -1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii|i:Gdk.Pixmap.__init__", kwlist, &py_drawable, &width, &height, &depth))
        return -1;
    if (py_drawable && pygobject_check(py_drawable, &PyGdkDrawable_Type))
        drawable = GDK_DRAWABLE(py_drawable->obj);
    else if ((PyObject *)py_drawable != Py_None) {
        PyErr_SetString(PyExc_TypeError, "drawable should be a GdkDrawable or None");
        return -1;
    }
    self->obj = (GObject *)gdk_pixmap_new((GdkDrawable *) drawable, width, height, depth);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkPixmap object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

PyTypeObject G_GNUC_INTERNAL PyGdkPixmap_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Pixmap",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pixmap_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkGC ----------- */

#line 24 "gdkgc.override"
static int
_wrap_gdk_gc_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "foreground", "background", "font",
			      "function", "fill", "tile", "stipple",
			      "clip_mask", "subwindow_mode", "ts_x_origin",
			      "ts_y_origin", "clip_x_origin", "clip_y_origin",
			      "graphics_exposures", "line_width", "line_style",
			      "cap_style", "join_style", NULL };
    PyGObject *drawable;
    PyObject *foreground = Py_None, *background = Py_None;
    PyObject *font = Py_None;
    PyObject *tile = Py_None, *stipple = Py_None, *clip_mask = Py_None;
    gint function = -1, fill = -1;
    gint subwindow_mode = -1, ts_x_origin = -1, ts_y_origin = -1;
    gint clip_x_origin = -1, clip_y_origin = -1, graphics_exposures = -1;
    gint line_width = -1, line_style = -1, cap_style = -1, join_style = -1;
    GdkGCValues values;
    GdkGCValuesMask mask = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!|OOOiiOOOiiiiiiiiii:GdkGC.__init__",
				     kwlist, &PyGdkDrawable_Type, &drawable,
				     &foreground, &background, &font,
				     &function, &fill, &tile, &stipple,
				     &clip_mask, &subwindow_mode,
				     &ts_x_origin, &ts_y_origin,
				     &clip_x_origin, &clip_y_origin,
				     &graphics_exposures, &line_width,
				     &line_style, &cap_style, &join_style))
	return -1;
    if (pyg_boxed_check(foreground, GDK_TYPE_COLOR)) {
	mask |= GDK_GC_FOREGROUND;
	values.foreground = *pyg_boxed_get(foreground, GdkColor);
    } else if (foreground != Py_None) {
	PyErr_SetString(PyExc_TypeError, "foreground must be a GdkColor");
	return -1;
    }
    if (pyg_boxed_check(background, GDK_TYPE_COLOR)) {
	mask |= GDK_GC_BACKGROUND;
	values.background = *pyg_boxed_get(background, GdkColor);
    } else if (background != Py_None) {
	PyErr_SetString(PyExc_TypeError, "background must be a GdkColor");
	return -1;
    }
    if (pyg_boxed_check(font, GDK_TYPE_FONT)) {
	mask |= GDK_GC_FONT;
	values.font = pyg_boxed_get(font, GdkFont);
    } else if (font != Py_None) {
	PyErr_SetString(PyExc_TypeError, "font must be a GdkFont");
	return -1;
    }
    if (function != -1) {
	mask |= GDK_GC_FUNCTION;
	values.function = function;
    }
    if (fill != -1) {
	mask |= GDK_GC_FILL;
	values.fill = fill;
    }
    if (pygobject_check(tile, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_TILE;
	values.tile = GDK_PIXMAP(pygobject_get(tile));
    } else if (tile != Py_None) {
	PyErr_SetString(PyExc_TypeError, "tile must be a GdkPixmap");
	return -1;
    }
    if (pygobject_check(stipple, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_STIPPLE;
	values.stipple = GDK_PIXMAP(pygobject_get(stipple));
    } else if (stipple != Py_None) {
	PyErr_SetString(PyExc_TypeError, "stipple must be a GdkPixmap");
	return -1;
    }
    if (pygobject_check(clip_mask, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_CLIP_MASK;
	values.clip_mask = GDK_PIXMAP(pygobject_get(clip_mask));
    } else if (clip_mask != Py_None) {
	PyErr_SetString(PyExc_TypeError, "clip_mask must be a GdkPixmap");
	return -1;
    }
    if (subwindow_mode != -1) {
	mask |= GDK_GC_SUBWINDOW;
	values.subwindow_mode = subwindow_mode;
    }
    if (ts_x_origin != -1) {
	mask |= GDK_GC_TS_X_ORIGIN;
	values.ts_x_origin = ts_x_origin;
    }
    if (ts_y_origin != -1) {
	mask |= GDK_GC_TS_Y_ORIGIN;
	values.ts_y_origin = ts_y_origin;
    }
    if (clip_x_origin != -1) {
	mask |= GDK_GC_CLIP_X_ORIGIN;
	values.clip_x_origin = clip_x_origin;
    }
    if (clip_y_origin != -1) {
	mask |= GDK_GC_CLIP_Y_ORIGIN;
	values.clip_y_origin = clip_y_origin;
    }
    if (graphics_exposures != -1) {
	mask |= GDK_GC_EXPOSURES;
	values.graphics_exposures = graphics_exposures;
    }
    if (line_width != -1) {
	mask |= GDK_GC_LINE_WIDTH;
	values.line_width = line_width;
    }
    if (line_style != -1) {
	mask |= GDK_GC_LINE_STYLE;
	values.line_style = line_style;
    }
    if (cap_style != -1) {
	mask |= GDK_GC_CAP_STYLE;
	values.cap_style = cap_style;
    }
    if (join_style != -1) {
	mask |= GDK_GC_JOIN_STYLE;
	values.join_style = join_style;
    }
    self->obj = (GObject *)gdk_gc_new_with_values(GDK_DRAWABLE(drawable->obj),
						  &values, mask);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkGC object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 11871 "gdk.c"


#line 302 "gdkgc.override"
static PyObject *
_wrap_gdk_gc_set_values(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "foreground", "background", "font", "function",
			      "fill", "tile", "stipple", "clip_mask",
			      "subwindow_mode", "ts_x_origin", "ts_y_origin",
			      "clip_x_origin", "clip_y_origin",
			      "graphics_exposures", "line_width", "line_style",
			      "cap_style", "join_style", NULL };
    PyObject *foreground = Py_None, *background = Py_None;
    PyObject *font = Py_None;
    gint function = -1, fill = -1;
    PyObject *tile = Py_None, *stipple = Py_None, *clip_mask = Py_None;
    gint subwindow_mode = -1, ts_x_origin = -1, ts_y_origin = -1;
    gint clip_x_origin = -1, clip_y_origin = -1, graphics_exposures = -1;
    gint line_width = -1, line_style = -1, cap_style = -1, join_style = -1;
    GdkGCValues values;
    GdkGCValuesMask mask = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|OOOiiOOOiiiiiiiiii:GdkGC.set_values",
				     kwlist, &foreground, &background, &font,
				     &function, &fill, &tile, &stipple,
				     &clip_mask, &subwindow_mode,
				     &ts_x_origin, &ts_y_origin,
				     &clip_x_origin, &clip_y_origin,
				     &graphics_exposures, &line_width,
				     &line_style, &cap_style, &join_style))
	return NULL;
    if (pyg_boxed_check(foreground, GDK_TYPE_COLOR)) {
	mask |= GDK_GC_FOREGROUND;
	values.foreground = *pyg_boxed_get(foreground, GdkColor);
    } else if (foreground != Py_None) {
	PyErr_SetString(PyExc_TypeError, "foreground must be a GdkColor");
	return NULL;
    }
    if (pyg_boxed_check(background, GDK_TYPE_COLOR)) {
	mask |= GDK_GC_BACKGROUND;
	values.background = *pyg_boxed_get(background, GdkColor);
    } else if (background != Py_None) {
	PyErr_SetString(PyExc_TypeError, "background must be a GdkColor");
	return NULL;
    }
    if (pyg_boxed_check(font, GDK_TYPE_FONT)) {
	mask |= GDK_GC_FONT;
	values.font = pyg_boxed_get(font, GdkFont);
    } else if (font != Py_None) {
	PyErr_SetString(PyExc_TypeError, "font must be a GdkFont");
	return NULL;
    }
    if (function != -1) {
	mask |= GDK_GC_FUNCTION;
	values.function = function;
    }
    if (fill != -1) {
	mask |= GDK_GC_FILL;
	values.fill = fill;
    }
    if (pygobject_check(tile, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_TILE;
	values.tile = GDK_PIXMAP(pygobject_get(tile));
    } else if (tile != Py_None) {
	PyErr_SetString(PyExc_TypeError, "tile must be a GdkPixmap");
	return NULL;
    }
    if (pygobject_check(stipple, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_STIPPLE;
	values.stipple = GDK_PIXMAP(pygobject_get(stipple));
    } else if (stipple != Py_None) {
	PyErr_SetString(PyExc_TypeError, "stipple must be a GdkPixmap");
	return NULL;
    }
    if (pygobject_check(clip_mask, &PyGdkPixmap_Type)) {
	mask |= GDK_GC_CLIP_MASK;
	values.clip_mask = GDK_PIXMAP(pygobject_get(clip_mask));
    } else if (clip_mask != Py_None) {
	PyErr_SetString(PyExc_TypeError, "clip_mask must be a GdkPixmap");
	return NULL;
    }
    if (subwindow_mode != -1) {
	mask |= GDK_GC_SUBWINDOW;
	values.subwindow_mode = subwindow_mode;
    }
    if (ts_x_origin != -1) {
	mask |= GDK_GC_TS_X_ORIGIN;
	values.ts_x_origin = ts_x_origin;
    }
    if (ts_y_origin != -1) {
	mask |= GDK_GC_TS_Y_ORIGIN;
	values.ts_y_origin = ts_y_origin;
    }
    if (clip_x_origin != -1) {
	mask |= GDK_GC_CLIP_X_ORIGIN;
	values.clip_x_origin = clip_x_origin;
    }
    if (clip_y_origin != -1) {
	mask |= GDK_GC_CLIP_Y_ORIGIN;
	values.clip_y_origin = clip_y_origin;
    }
    if (graphics_exposures != -1) {
	mask |= GDK_GC_EXPOSURES;
	values.graphics_exposures = graphics_exposures;
    }
    if (line_width != -1) {
	mask |= GDK_GC_LINE_WIDTH;
	values.line_width = line_width;
    }
    if (line_style != -1) {
	mask |= GDK_GC_LINE_STYLE;
	values.line_style = line_style;
    }
    if (cap_style != -1) {
	mask |= GDK_GC_CAP_STYLE;
	values.cap_style = cap_style;
    }
    if (join_style != -1) {
	mask |= GDK_GC_JOIN_STYLE;
	values.join_style = join_style;
    }
    gdk_gc_set_values(GDK_GC(self->obj), &values, mask);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 11999 "gdk.c"


static PyObject *
_wrap_gdk_gc_set_foreground(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_foreground", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_gc_set_foreground(GDK_GC(self->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_background(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_background", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_gc_set_background(GDK_GC(self->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_font(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", NULL };
    GdkFont *font = NULL;
    PyObject *py_font;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_font", kwlist, &py_font))
        return NULL;
    if (pyg_boxed_check(py_font, GDK_TYPE_FONT))
        font = pyg_boxed_get(py_font, GdkFont);
    else {
        PyErr_SetString(PyExc_TypeError, "font should be a GdkFont");
        return NULL;
    }
    
    gdk_gc_set_font(GDK_GC(self->obj), font);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_function(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "function", NULL };
    PyObject *py_function = NULL;
    GdkFunction function;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_function", kwlist, &py_function))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_FUNCTION, py_function, (gpointer)&function))
        return NULL;
    
    gdk_gc_set_function(GDK_GC(self->obj), function);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_fill(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fill", NULL };
    PyObject *py_fill = NULL;
    GdkFill fill;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_fill", kwlist, &py_fill))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_FILL, py_fill, (gpointer)&fill))
        return NULL;
    
    gdk_gc_set_fill(GDK_GC(self->obj), fill);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_tile(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tile", NULL };
    PyGObject *tile;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.GC.set_tile", kwlist, &PyGdkPixmap_Type, &tile))
        return NULL;
    
    gdk_gc_set_tile(GDK_GC(self->obj), GDK_PIXMAP(tile->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_stipple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stipple", NULL };
    PyGObject *stipple;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.GC.set_stipple", kwlist, &PyGdkPixmap_Type, &stipple))
        return NULL;
    
    gdk_gc_set_stipple(GDK_GC(self->obj), GDK_PIXMAP(stipple->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_ts_origin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.GC.set_ts_origin", kwlist, &x, &y))
        return NULL;
    
    gdk_gc_set_ts_origin(GDK_GC(self->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_clip_origin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.GC.set_clip_origin", kwlist, &x, &y))
        return NULL;
    
    gdk_gc_set_clip_origin(GDK_GC(self->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_clip_mask(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mask", NULL };
    PyGObject *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.GC.set_clip_mask", kwlist, &PyGdkPixmap_Type, &mask))
        return NULL;
    
    gdk_gc_set_clip_mask(GDK_GC(self->obj), GDK_PIXMAP(mask->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_clip_rectangle(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rectangle", NULL };
    PyObject *py_rectangle;
    GdkRectangle rectangle = { 0, 0, 0, 0 };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_clip_rectangle", kwlist, &py_rectangle))
        return NULL;
    if (!pygdk_rectangle_from_pyobject(py_rectangle, &rectangle))
        return NULL;
    
    gdk_gc_set_clip_rectangle(GDK_GC(self->obj), &rectangle);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_clip_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "region", NULL };
    PyObject *py_region;
    GdkRegion *region = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_clip_region", kwlist, &py_region))
        return NULL;
    if (pyg_boxed_check(py_region, PYGDK_TYPE_REGION))
        region = pyg_boxed_get(py_region, GdkRegion);
    else {
        PyErr_SetString(PyExc_TypeError, "region should be a GdkRegion");
        return NULL;
    }
    
    gdk_gc_set_clip_region(GDK_GC(self->obj), region);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_subwindow(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", NULL };
    PyObject *py_mode = NULL;
    GdkSubwindowMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_subwindow", kwlist, &py_mode))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_SUBWINDOW_MODE, py_mode, (gpointer)&mode))
        return NULL;
    
    gdk_gc_set_subwindow(GDK_GC(self->obj), mode);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_exposures(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "exposures", NULL };
    int exposures;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.GC.set_exposures", kwlist, &exposures))
        return NULL;
    
    gdk_gc_set_exposures(GDK_GC(self->obj), exposures);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_line_attributes(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line_width", "line_style", "cap_style", "join_style", NULL };
    int line_width;
    GdkLineStyle line_style;
    GdkJoinStyle join_style;
    GdkCapStyle cap_style;
    PyObject *py_line_style = NULL, *py_cap_style = NULL, *py_join_style = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iOOO:Gdk.GC.set_line_attributes", kwlist, &line_width, &py_line_style, &py_cap_style, &py_join_style))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_LINE_STYLE, py_line_style, (gpointer)&line_style))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_CAP_STYLE, py_cap_style, (gpointer)&cap_style))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_JOIN_STYLE, py_join_style, (gpointer)&join_style))
        return NULL;
    
    gdk_gc_set_line_attributes(GDK_GC(self->obj), line_width, line_style, cap_style, join_style);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 590 "gdkgc.override"
static PyObject *
_wrap_gdk_gc_set_dashes(PyGObject *self, PyObject *args)
{
    gint dash_offset, n, i;
    PyObject *list;
    gint8 *dash_list;

    if (!PyArg_ParseTuple(args, "iO:GdkGC.set_dashes", &dash_offset, &list))
        return NULL;
    if (!PySequence_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "second argument must be a sequence");
        return NULL;
    }
    n = PySequence_Length(list);
    dash_list = g_new(gint8, n);
    for (i = 0; i < n; i++) {
        PyObject *item = PySequence_GetItem(list, i);
        Py_DECREF(item);

        if (!PyInt_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "sequence member must be an int");
            g_free(dash_list);
            return NULL;
        }
        dash_list[i] = (gint8)PyInt_AsLong(item);
        if (dash_list[i] == 0) {
            PyErr_SetString(PyExc_TypeError, "sequence member must not be 0");
            g_free(dash_list);
            return NULL;
        }
    }
    gdk_gc_set_dashes(GDK_GC(self->obj), dash_offset, dash_list, n);
    g_free(dash_list);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 12314 "gdk.c"


static PyObject *
_wrap_gdk_gc_offset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x_offset", "y_offset", NULL };
    int x_offset, y_offset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.GC.offset", kwlist, &x_offset, &y_offset))
        return NULL;
    
    gdk_gc_offset(GDK_GC(self->obj), x_offset, y_offset);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_copy(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src_gc", NULL };
    PyGObject *src_gc;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.GC.copy", kwlist, &PyGdkGC_Type, &src_gc))
        return NULL;
    
    gdk_gc_copy(GDK_GC(self->obj), GDK_GC(src_gc->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_colormap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", NULL };
    PyGObject *colormap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.GC.set_colormap", kwlist, &PyGdkColormap_Type, &colormap))
        return NULL;
    
    gdk_gc_set_colormap(GDK_GC(self->obj), GDK_COLORMAP(colormap->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_get_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_gc_get_colormap(GDK_GC(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_gc_set_rgb_fg_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_rgb_fg_color", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_gc_set_rgb_fg_color(GDK_GC(self->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_set_rgb_bg_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "color", NULL };
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.GC.set_rgb_bg_color", kwlist, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_gc_set_rgb_bg_color(GDK_GC(self->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_gc_get_screen(PyGObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_gc_get_screen(GDK_GC(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkGC_methods[] = {
    { "set_values", (PyCFunction)_wrap_gdk_gc_set_values, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_foreground", (PyCFunction)_wrap_gdk_gc_set_foreground, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_background", (PyCFunction)_wrap_gdk_gc_set_background, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_font", (PyCFunction)_wrap_gdk_gc_set_font, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_function", (PyCFunction)_wrap_gdk_gc_set_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_fill", (PyCFunction)_wrap_gdk_gc_set_fill, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_tile", (PyCFunction)_wrap_gdk_gc_set_tile, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_stipple", (PyCFunction)_wrap_gdk_gc_set_stipple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_ts_origin", (PyCFunction)_wrap_gdk_gc_set_ts_origin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_clip_origin", (PyCFunction)_wrap_gdk_gc_set_clip_origin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_clip_mask", (PyCFunction)_wrap_gdk_gc_set_clip_mask, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_clip_rectangle", (PyCFunction)_wrap_gdk_gc_set_clip_rectangle, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_clip_region", (PyCFunction)_wrap_gdk_gc_set_clip_region, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_subwindow", (PyCFunction)_wrap_gdk_gc_set_subwindow, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_exposures", (PyCFunction)_wrap_gdk_gc_set_exposures, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_line_attributes", (PyCFunction)_wrap_gdk_gc_set_line_attributes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_dashes", (PyCFunction)_wrap_gdk_gc_set_dashes, METH_VARARGS,
      NULL },
    { "offset", (PyCFunction)_wrap_gdk_gc_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy", (PyCFunction)_wrap_gdk_gc_copy, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_colormap", (PyCFunction)_wrap_gdk_gc_set_colormap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_colormap", (PyCFunction)_wrap_gdk_gc_get_colormap, METH_NOARGS,
      NULL },
    { "set_rgb_fg_color", (PyCFunction)_wrap_gdk_gc_set_rgb_fg_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_rgb_bg_color", (PyCFunction)_wrap_gdk_gc_set_rgb_bg_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_screen", (PyCFunction)_wrap_gdk_gc_get_screen, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 428 "gdkgc.override"
static PyObject *
_wrap_gdk_gc_tp_getattro(PyGObject *self, PyObject *py_attr)
{
    GdkGCValues gc;
    char *attr;

    if (PyString_Check(py_attr))
        attr = PyString_AsString(py_attr);
    else
        return PyObject_GenericGetAttr((PyObject *)self, py_attr);

    if (!strcmp(attr, "__members__"))
        return Py_BuildValue("[ssssssssssssssssss]", "background", "cap_style",
                             "clip_mask", "clip_x_origin", "clip_y_origin",
                             "fill", "font", "foreground", "function",
                             "graphics_exposures", "join_style", "line_style",
                             "line_width", "stipple", "sub_window", "tile",
                             "ts_x_origin", "ts_y_origin");
    gdk_gc_get_values(GDK_GC(self->obj), &gc);
    if (!strcmp(attr, "foreground")) 
        return pyg_boxed_new(GDK_TYPE_COLOR, &(gc.foreground), TRUE, TRUE);
    if (!strcmp(attr, "background")) 
        return pyg_boxed_new(GDK_TYPE_COLOR, &(gc.background), TRUE, TRUE);
    if (!strcmp(attr, "font")) 
        return pyg_boxed_new(GDK_TYPE_FONT, gc.font, TRUE, TRUE);
    if (!strcmp(attr, "function")) 
        return PyInt_FromLong(gc.function);
    if (!strcmp(attr, "fill")) 
        return PyInt_FromLong(gc.fill);
    if (!strcmp(attr, "tile")) 
        return pygobject_new((GObject *)gc.tile);
    if (!strcmp(attr, "stipple")) 
        return pygobject_new((GObject *)gc.stipple);
    if (!strcmp(attr, "clip_mask")) 
        return pygobject_new((GObject *)gc.clip_mask);
    if (!strcmp(attr, "subwindow_mode"))
        return pyg_enum_from_gtype(GDK_TYPE_SUBWINDOW_MODE, gc.subwindow_mode);
    if (!strcmp(attr, "ts_x_origin")) 
        return PyInt_FromLong(gc.ts_x_origin);
    if (!strcmp(attr, "ts_y_origin")) 
        return PyInt_FromLong(gc.ts_y_origin);
    if (!strcmp(attr, "clip_x_origin")) 
        return PyInt_FromLong(gc.clip_x_origin);
    if (!strcmp(attr, "clip_y_origin")) 
        return PyInt_FromLong(gc.clip_y_origin);
    if (!strcmp(attr, "graphics_exposures"))
        return PyInt_FromLong(gc.graphics_exposures);
    if (!strcmp(attr, "line_width")) 
        return PyInt_FromLong(gc.line_width);
    if (!strcmp(attr, "line_style")) 
        return pyg_enum_from_gtype(GDK_TYPE_LINE_STYLE, gc.line_style);
    if (!strcmp(attr, "cap_style")) 
        return pyg_enum_from_gtype(GDK_TYPE_CAP_STYLE, gc.cap_style);
    if (!strcmp(attr, "join_style")) 
        return pyg_enum_from_gtype(GDK_TYPE_JOIN_STYLE, gc.join_style);

    {
        PyObject *name = PyString_FromString(attr);
        PyObject *ret = PyObject_GenericGetAttr((PyObject *)self, name);
        Py_DECREF(name);
        return ret;
    }
}
#line 12546 "gdk.c"


#line 493 "gdkgc.override"
static int
_wrap_gdk_gc_tp_setattro(PyGObject *self, PyObject *py_attr, PyObject *value)
{
    GdkGC *gc = GDK_GC(self->obj);
    char *attr;

    if (value == NULL) {
        PyErr_SetString(PyExc_TypeError, "can't delete attributes");
        return -1;
    }

    if (PyString_Check(py_attr))
        attr = PyString_AsString(py_attr);
    else {
        PyErr_SetString(PyExc_AttributeError, "could not write attribute");
        return -1;
    }

    if (PyInt_Check(value)) {
        int i = PyInt_AsLong(value);
        GdkGCValues v;
        gdk_gc_get_values(gc, &v);
        if (!strcmp(attr, "function")) {
            gdk_gc_set_function(gc, i);
            return 0;
        } else if (!strcmp(attr, "fill")) {
            gdk_gc_set_fill(gc, i);
            return 0;
        } else if (!strcmp(attr, "subwindow_mode")) {
            gdk_gc_set_subwindow(gc, i);
            return 0;
        } else if (!strcmp(attr, "ts_x_origin")) {
            gdk_gc_set_ts_origin(gc, i, v.ts_y_origin);
            return 0;
        } else if (!strcmp(attr, "ts_y_origin")) {
            gdk_gc_set_ts_origin(gc, v.ts_x_origin, i);
            return 0;
        } else if (!strcmp(attr, "clip_x_origin")) {
            gdk_gc_set_clip_origin(gc, i, v.clip_y_origin);
            return 0;
        } else if (!strcmp(attr, "clip_y_origin")) {
            gdk_gc_set_clip_origin(gc, v.clip_x_origin, i);
            return 0;
        } else if (!strcmp(attr, "graphics_exposures")) {
            gdk_gc_set_exposures(gc, i);
            return 0;
        } else if (!strcmp(attr, "line_width")) {
            gdk_gc_set_line_attributes(gc, i, v.line_style,
                                       v.cap_style, v.join_style);
            return 0;
        } else if (!strcmp(attr, "line_style")) {
            gdk_gc_set_line_attributes(gc, v.line_width, i,
                                       v.cap_style, v.join_style);
            return 0;
        } else if (!strcmp(attr, "cap_style")) {
            gdk_gc_set_line_attributes(gc, v.line_width, v.line_style,
                                       i, v.join_style);
            return 0;
        } else if (!strcmp(attr, "join_style")) {
            gdk_gc_set_line_attributes(gc, v.line_width, v.line_style,
                                       v.cap_style, i);
            return 0;
        }
    } else if (pyg_boxed_check(value, GDK_TYPE_COLOR)) {
        GdkColor *c = pyg_boxed_get(value, GdkColor);
        if (!strcmp(attr, "foreground")) {
            gdk_gc_set_foreground(gc, c);
            return 0;
        } else if (!strcmp(attr, "background")) {
            gdk_gc_set_background(gc, c);
            return 0;
        }
    } else if (pyg_boxed_check(value, GDK_TYPE_FONT)) {
        if (!strcmp(attr, "font")) {
            gdk_gc_set_font(gc, pyg_boxed_get(value, GdkFont));
            return 0;
        }
    } else if (pygobject_check(value, &PyGdkWindow_Type) || value == Py_None) {
        PyGObject *window = (PyGObject *)value;
        GdkWindow *w = (value==Py_None)?NULL:GDK_WINDOW(window->obj);
        if (!strcmp(attr, "tile")) {
            gdk_gc_set_tile(gc, w);
            return 0;
        } else if (!strcmp(attr, "stipple")) {
            gdk_gc_set_stipple(gc, w);
            return 0;
        } else if (!strcmp(attr, "clip_mask")) {
            gdk_gc_set_clip_mask(gc, w);
            return 0;
        }
    }

    PyErr_SetString(PyExc_AttributeError, "could not write attribute");
    return -1;
}
#line 12645 "gdk.c"


PyTypeObject G_GNUC_INTERNAL PyGdkGC_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.GC",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)_wrap_gdk_gc_tp_getattro,     /* tp_getattro */
    (setattrofunc)_wrap_gdk_gc_tp_setattro,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkGC_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_gc_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};


static int
__GdkGC_class_init(gpointer gclass, PyTypeObject *pyclass)
{

    /* overriding do_get_values is currently not supported */

    /* overriding do_set_values is currently not supported */

    /* overriding do_set_dashes is currently not supported */
    return 0;
}


/* ----------- GdkImage ----------- */

static int
_wrap_gdk_image_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "visual", "width", "height", NULL };
    PyObject *py_type = NULL;
    PyGObject *visual;
    int width, height;
    GdkImageType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!ii:Gdk.Image.__init__", kwlist, &py_type, &PyGdkVisual_Type, &visual, &width, &height))
        return -1;
    if (pyg_enum_get_value(GDK_TYPE_IMAGE_TYPE, py_type, (gpointer)&type))
        return -1;
    self->obj = (GObject *)gdk_image_new(type, GDK_VISUAL(visual->obj), width, height);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkImage object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_image_put_pixel(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "pixel", NULL };
    int x, y;
    unsigned long pixel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iik:Gdk.Image.put_pixel", kwlist, &x, &y, &pixel))
        return NULL;
    if (pixel > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " pixel parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_image_put_pixel(GDK_IMAGE(self->obj), x, y, pixel);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_image_get_pixel(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y;
    guint32 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Image.get_pixel", kwlist, &x, &y))
        return NULL;
    
    ret = gdk_image_get_pixel(GDK_IMAGE(self->obj), x, y);
    
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_image_set_colormap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", NULL };
    PyGObject *colormap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Image.set_colormap", kwlist, &PyGdkColormap_Type, &colormap))
        return NULL;
    
    gdk_image_set_colormap(GDK_IMAGE(self->obj), GDK_COLORMAP(colormap->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_image_get_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_image_get_colormap(GDK_IMAGE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkImage_methods[] = {
    { "put_pixel", (PyCFunction)_wrap_gdk_image_put_pixel, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_pixel", (PyCFunction)_wrap_gdk_image_get_pixel, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_colormap", (PyCFunction)_wrap_gdk_image_set_colormap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_colormap", (PyCFunction)_wrap_gdk_image_get_colormap, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkImage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Image",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkImage_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_image_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkKeymap ----------- */

#line 1240 "./gdk.override"
static PyObject *
_wrap_gdk_keymap_lookup_key(PyGObject *self, PyObject *args,
			    PyObject *kwargs)
{
    static char *kwlist[] = { "keycode", "group", "level", NULL };
    GdkKeymapKey key;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iii:gtk.gdk.Keymap.lookup_key",
				     kwlist, &key.keycode, &key.group,
				     &key.level))
	return NULL;

    return PyInt_FromLong(gdk_keymap_lookup_key(GDK_KEYMAP(self->obj), &key));
}
#line 12874 "gdk.c"


#line 1257 "./gdk.override"
static PyObject *
_wrap_gdk_keymap_translate_keyboard_state(PyGObject *self, PyObject *args,
					  PyObject *kwargs)
{
    static char *kwlist[] = { "keycode", "state", "group", NULL };
    guint keycode, keyval;
    gint group, effective_group, level;
    GdkModifierType state, modifiers;
    PyObject *py_state;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iOi:gtk.gdk.Keymap.translate_keyboard_state",
				     kwlist, &keycode, &py_state,
				     &group))
	return NULL;

    if (pyg_flags_get_value(GDK_TYPE_MODIFIER_TYPE, py_state,
			    (gint *)&state))
        return NULL;

    if (gdk_keymap_translate_keyboard_state(GDK_KEYMAP(self->obj), keycode,
					    state, group, &keyval,
					    &effective_group, &level,
					    &modifiers))
	return Py_BuildValue("(iiiN)", keyval, effective_group, level,
			     pyg_flags_from_gtype(GDK_TYPE_MODIFIER_TYPE,
                                                  modifiers));
    Py_INCREF(Py_None);
    return Py_None;
}
#line 12908 "gdk.c"


#line 1170 "./gdk.override"
static PyObject *
_wrap_gdk_keymap_get_entries_for_keyval(PyGObject *self, PyObject *args,
					PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    guint keyval;
    GdkKeymapKey *keys;
    gint n_keys;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:gtk.gdk.Keymap.get_entries_for_keyval",
				     kwlist, &keyval))
	return NULL;

    if (gdk_keymap_get_entries_for_keyval(GDK_KEYMAP(self->obj), keyval,
					  &keys, &n_keys)) {
	int i;
	PyObject * list = PyTuple_New(n_keys);

	for (i = 0; i < n_keys; i++) {
	    PyTuple_SetItem(list, i,
			    Py_BuildValue("(iii)", keys[i].keycode,
					  keys[i].group, keys[i].level));
	}
	g_free(keys);

	return list;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 12944 "gdk.c"


#line 1204 "./gdk.override"
static PyObject *
_wrap_gdk_keymap_get_entries_for_keycode(PyGObject *self, PyObject *args,
					 PyObject *kwargs)
{
    static char *kwlist[] = { "hardware_keycode", NULL };
    guint keycode;
    GdkKeymapKey *keys;
    guint *keyvals;
    gint n_keys;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:gtk.gdk.Keymap.get_entries_for_keycode",
				     kwlist, &keycode))
	return NULL;

    if (gdk_keymap_get_entries_for_keycode(GDK_KEYMAP(self->obj), keycode,
					  &keys, &keyvals, &n_keys)) {
	int i;
	PyObject * list = PyTuple_New(n_keys);

	for (i = 0; i < n_keys; i++) {
	    PyTuple_SetItem(list, i,
			    Py_BuildValue("(iiii)", keyvals[i],
					  keys[i].keycode,
					  keys[i].group, keys[i].level));
	}
	g_free(keys);

	return list;
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 12982 "gdk.c"


static PyObject *
_wrap_gdk_keymap_get_direction(PyGObject *self)
{
    gint ret;

    
    ret = gdk_keymap_get_direction(GDK_KEYMAP(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_DIRECTION, ret);
}

static PyObject *
_wrap_gdk_keymap_have_bidi_layouts(PyGObject *self)
{
    int ret;

    
    ret = gdk_keymap_have_bidi_layouts(GDK_KEYMAP(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_keymap_get_caps_lock_state(PyGObject *self)
{
    int ret;

    
    ret = gdk_keymap_get_caps_lock_state(GDK_KEYMAP(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GdkKeymap__do_direction_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Keymap.direction_changed", kwlist, &PyGdkKeymap_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_KEYMAP_CLASS(klass)->direction_changed)
        GDK_KEYMAP_CLASS(klass)->direction_changed(GDK_KEYMAP(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Keymap.direction_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkKeymap__do_keys_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Keymap.keys_changed", kwlist, &PyGdkKeymap_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_KEYMAP_CLASS(klass)->keys_changed)
        GDK_KEYMAP_CLASS(klass)->keys_changed(GDK_KEYMAP(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Keymap.keys_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkKeymap_methods[] = {
    { "lookup_key", (PyCFunction)_wrap_gdk_keymap_lookup_key, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "translate_keyboard_state", (PyCFunction)_wrap_gdk_keymap_translate_keyboard_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_entries_for_keyval", (PyCFunction)_wrap_gdk_keymap_get_entries_for_keyval, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_entries_for_keycode", (PyCFunction)_wrap_gdk_keymap_get_entries_for_keycode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_direction", (PyCFunction)_wrap_gdk_keymap_get_direction, METH_NOARGS,
      NULL },
    { "have_bidi_layouts", (PyCFunction)_wrap_gdk_keymap_have_bidi_layouts, METH_NOARGS,
      NULL },
    { "get_caps_lock_state", (PyCFunction)_wrap_gdk_keymap_get_caps_lock_state, METH_NOARGS,
      NULL },
    { "do_direction_changed", (PyCFunction)_wrap_GdkKeymap__do_direction_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_keys_changed", (PyCFunction)_wrap_GdkKeymap__do_keys_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkKeymap_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Keymap",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkKeymap_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GdkKeymap__proxy_do_direction_changed(GdkKeymap *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_direction_changed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkKeymap__proxy_do_keys_changed(GdkKeymap *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_keys_changed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkKeymap_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkKeymapClass *klass = GDK_KEYMAP_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_direction_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "direction_changed")))
            klass->direction_changed = _wrap_GdkKeymap__proxy_do_direction_changed;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_keys_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "keys_changed")))
            klass->keys_changed = _wrap_GdkKeymap__proxy_do_keys_changed;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GdkPixbuf ----------- */

static int
_wrap_gdk_pixbuf_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colorspace", "has_alpha", "bits_per_sample", "width", "height", NULL };
    PyObject *py_colorspace = NULL;
    int has_alpha, bits_per_sample, width, height;
    GdkColorspace colorspace;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oiiii:Gdk.Pixbuf.__init__", kwlist, &py_colorspace, &has_alpha, &bits_per_sample, &width, &height))
        return -1;
    if (pyg_enum_get_value(GDK_TYPE_COLORSPACE, py_colorspace, (gpointer)&colorspace))
        return -1;
    self->obj = (GObject *)gdk_pixbuf_new(colorspace, has_alpha, bits_per_sample, width, height);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkPixbuf object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_pixbuf_render_threshold_alpha(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bitmap", "src_x", "src_y", "dest_x", "dest_y", "width", "height", "alpha_threshold", NULL };
    PyGObject *bitmap;
    int src_x, src_y, dest_x, dest_y, width, height, alpha_threshold;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiiii:Gdk.Pixbuf.render_threshold_alpha", kwlist, &PyGdkPixmap_Type, &bitmap, &src_x, &src_y, &dest_x, &dest_y, &width, &height, &alpha_threshold))
        return NULL;
    
    gdk_pixbuf_render_threshold_alpha(GDK_PIXBUF(self->obj), GDK_PIXMAP(bitmap->obj), src_x, src_y, dest_x, dest_y, width, height, alpha_threshold);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_render_to_drawable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "gc", "src_x", "src_y", "dest_x", "dest_y", "width", "height", "dither", "x_dither", "y_dither", NULL };
    PyGObject *drawable, *gc;
    int src_x, src_y, dest_x, dest_y, width, height, x_dither = 0, y_dither = 0;
    PyObject *py_dither = NULL;
    GdkRgbDither dither = GDK_RGB_DITHER_NORMAL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiii|Oii:Gdk.Pixbuf.render_to_drawable", kwlist, &PyGdkDrawable_Type, &drawable, &PyGdkGC_Type, &gc, &src_x, &src_y, &dest_x, &dest_y, &width, &height, &py_dither, &x_dither, &y_dither))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use gtk.gdk.Drawable.draw_pixbuf") < 0)
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dither, (gpointer)&dither))
        return NULL;
    
    gdk_pixbuf_render_to_drawable(GDK_PIXBUF(self->obj), GDK_DRAWABLE(drawable->obj), GDK_GC(gc->obj), src_x, src_y, dest_x, dest_y, width, height, dither, x_dither, y_dither);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_render_to_drawable_alpha(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "src_x", "src_y", "dest_x", "dest_y", "width", "height", "alpha_mode", "alpha_threshold", "dither", "x_dither", "y_dither", NULL };
    PyGObject *drawable;
    int src_x, src_y, dest_x, dest_y, width, height, alpha_threshold, x_dither = 0, y_dither = 0;
    GdkPixbufAlphaMode alpha_mode;
    PyObject *py_alpha_mode = NULL, *py_dither = NULL;
    GdkRgbDither dither = GDK_RGB_DITHER_NORMAL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiiiOi|Oii:Gdk.Pixbuf.render_to_drawable_alpha", kwlist, &PyGdkDrawable_Type, &drawable, &src_x, &src_y, &dest_x, &dest_y, &width, &height, &py_alpha_mode, &alpha_threshold, &py_dither, &x_dither, &y_dither))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use gtk.gdk.Drawable.draw_pixbuf") < 0)
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_PIXBUF_ALPHA_MODE, py_alpha_mode, (gpointer)&alpha_mode))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dither, (gpointer)&dither))
        return NULL;
    
    gdk_pixbuf_render_to_drawable_alpha(GDK_PIXBUF(self->obj), GDK_DRAWABLE(drawable->obj), src_x, src_y, dest_x, dest_y, width, height, alpha_mode, alpha_threshold, dither, x_dither, y_dither);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 24 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_render_pixmap_and_mask(PyGObject *self, PyObject *args,
                                        PyObject *kwargs)
{
    static char *kwlist[] = { "alpha_threshold", NULL};
    int alpha_threshold = 127;
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    PyObject *pypixmap, *pymask, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|i:GdkPixbuf.render_pixmap_and_mask", kwlist,
                                     &alpha_threshold))
        return NULL;
    gdk_pixbuf_render_pixmap_and_mask(GDK_PIXBUF(self->obj), &pixmap, &mask,
                                      alpha_threshold);

    if (pixmap) {
        pypixmap = pygobject_new((GObject *) pixmap);
        g_object_unref(pixmap);
    } else {
        Py_INCREF(Py_None);
        pypixmap = Py_None;
    }

    if (mask) {
        pymask = pygobject_new((GObject *) mask);
        gdk_bitmap_unref(mask);
    } else {
        Py_INCREF(Py_None);
        pymask = Py_None;
    }
    
    ret = Py_BuildValue("(NN)", pypixmap, pymask);
    return ret;
}
#line 13390 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_get_from_drawable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "cmap", "src_x", "src_y", "dest_x", "dest_y", "width", "height", NULL };
    PyGObject *src, *cmap;
    int src_x, src_y, dest_x, dest_y, width, height;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiii:Gdk.Pixbuf.get_from_drawable", kwlist, &PyGdkDrawable_Type, &src, &PyGdkColormap_Type, &cmap, &src_x, &src_y, &dest_x, &dest_y, &width, &height))
        return NULL;
    
    ret = gdk_pixbuf_get_from_drawable(GDK_PIXBUF(self->obj), GDK_DRAWABLE(src->obj), GDK_COLORMAP(cmap->obj), src_x, src_y, dest_x, dest_y, width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_get_from_image(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "cmap", "src_x", "src_y", "dest_x", "dest_y", "width", "height", NULL };
    PyGObject *src, *cmap;
    int src_x, src_y, dest_x, dest_y, width, height;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiiiii:Gdk.Pixbuf.get_from_image", kwlist, &PyGdkImage_Type, &src, &PyGdkColormap_Type, &cmap, &src_x, &src_y, &dest_x, &dest_y, &width, &height))
        return NULL;
    
    ret = gdk_pixbuf_get_from_image(GDK_PIXBUF(self->obj), GDK_IMAGE(src->obj), GDK_COLORMAP(cmap->obj), src_x, src_y, dest_x, dest_y, width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_rotate_simple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "angle", NULL };
    int angle;
    PyObject *py_ret;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Pixbuf.rotate_simple", kwlist, &angle))
        return NULL;
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_rotate_simple(GDK_PIXBUF(self->obj), angle);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_get_colorspace(PyGObject *self)
{
    gint ret;

    
    ret = gdk_pixbuf_get_colorspace(GDK_PIXBUF(self->obj));
    
    return pyg_enum_from_gtype(GDK_TYPE_COLORSPACE, ret);
}

static PyObject *
_wrap_gdk_pixbuf_get_n_channels(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_n_channels(GDK_PIXBUF(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_get_has_alpha(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_has_alpha(GDK_PIXBUF(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_pixbuf_get_bits_per_sample(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_bits_per_sample(GDK_PIXBUF(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 132 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_get_pixels(PyGObject *self)
{
    GdkPixbuf *pixbuf;
    guchar *pixels;
    gint rowstride, height;

    pixbuf = GDK_PIXBUF(self->obj);
    pixels = gdk_pixbuf_get_pixels(pixbuf);
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);

    if (pixels == NULL) {
        PyErr_SetString(PyExc_ValueError, "could not get pixel data");
        return NULL;
    }

    return PyString_FromStringAndSize((char*)pixels, rowstride*height);
}
#line 13511 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_get_width(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_width(GDK_PIXBUF(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_get_height(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_height(GDK_PIXBUF(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_get_rowstride(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_get_rowstride(GDK_PIXBUF(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_copy(PyGObject *self)
{
    PyObject *py_ret;
    GdkPixbuf *ret;

    
    ret = gdk_pixbuf_copy(GDK_PIXBUF(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_new_subpixbuf(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src_x", "src_y", "width", "height", NULL };
    int src_x, src_y, width, height;
    PyObject *py_ret;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Gdk.Pixbuf.subpixbuf", kwlist, &src_x, &src_y, &width, &height))
        return NULL;
    
    ret = gdk_pixbuf_new_subpixbuf(GDK_PIXBUF(self->obj), src_x, src_y, width, height);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 661 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_fill(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixel", NULL };
    PyObject *py_pixel = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GdkPixbuf.fill", kwlist, &py_pixel))
        return NULL;
    
    if (PyInt_Check(py_pixel))
	gdk_pixbuf_fill(GDK_PIXBUF(self->obj), PyInt_AsLong(py_pixel));
    else if (PyLong_Check(py_pixel))
	gdk_pixbuf_fill(GDK_PIXBUF(self->obj), PyLong_AsUnsignedLong(py_pixel));
    else {
	PyErr_SetString(PyExc_TypeError, "pixel must be an int or a long");
	return NULL;
    }
    
    Py_INCREF(Py_None);
    return Py_None;
}
#line 13603 "gdk.c"


#line 153 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_save(PyGObject *self, PyObject *args,
                      PyObject *kwargs)
{
    static char *kwlist[] = {"filename", "type", "options", NULL};
    gchar *filename, *type, **option_keys = NULL, **option_values = NULL;
    PyObject *py_options = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "ss|O!:GdkPixbuf.save", kwlist,
                                     &filename, &type,
                                     &PyDict_Type, &py_options))
        return NULL;

    if (py_options != NULL) {
        guint len;
        Py_ssize_t pos = 0, i = 0;
        PyObject *key, *value;
        
        len = PyDict_Size(py_options);
        option_keys = g_new(gchar *, len + 1);
        option_values = g_new(gchar *, len + 1);

        while (PyDict_Next(py_options, &pos, &key, &value)) {
            if (!PyString_Check(key) || !PyString_Check(value)) {
                g_free(option_keys);
                g_free(option_values);
                PyErr_SetString(PyExc_TypeError,
                                "keys and values must be strings");
                return NULL;
            }

            option_keys[i]   = PyString_AsString(key);
            option_values[i] = PyString_AsString(value);
            i++;
        }
        
        option_keys[len] = NULL;
        option_values[len] = NULL;
    }

    pyg_begin_allow_threads;
    gdk_pixbuf_savev(GDK_PIXBUF(self->obj), filename, type,
                     option_keys, option_values, &error);
    pyg_end_allow_threads;

    g_free(option_keys);
    g_free(option_values);
 
    if (pyg_error_check(&error))
        return NULL;
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 13664 "gdk.c"


#line 212 "gdkpixbuf.override"

typedef struct {
    PyObject *callback;
    PyObject *user_data;
} py_pixbuf_save_callback_data_t;

static gboolean
pixbuf_save_func(const gchar *buf,
                 gsize count,
                 GError **error,
                 py_pixbuf_save_callback_data_t *data)
{
    PyObject *ret;

    if (data->user_data)
        ret = PyObject_CallFunction(data->callback, "s#O", buf, (Py_ssize_t) count, data->user_data);

    else
        ret = PyObject_CallFunction(data->callback, "s#", buf, (Py_ssize_t) count);
    
    if (ret == NULL) {
        PyErr_Print();
        return FALSE;
    }
    Py_DECREF(ret);
    return TRUE;
}

static PyObject *
_wrap_gdk_pixbuf_save_to_callback(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = {"save_func", "type", "options", "user_data", NULL};
    gchar *type, **option_keys = NULL, **option_values = NULL;
    PyObject *py_options = NULL;
    GError *error = NULL;
    py_pixbuf_save_callback_data_t data = { NULL, NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "Os|O!O:GdkPixbuf.save", kwlist,
                                     &data.callback, &type,
                                     &PyDict_Type, &py_options, &data.user_data))
        return NULL;

    if (!PyCallable_Check(data.callback)) {
        PyErr_SetString(PyExc_TypeError, "first argument must be callable");
        return NULL;
    }

    if (py_options != NULL) {
        Py_ssize_t len;
        Py_ssize_t pos = 0, i = 0;
        PyObject *key, *value;
        
        len = PyDict_Size(py_options);
        option_keys = g_new(gchar *, len + 1);
        option_values = g_new(gchar *, len + 1);

        while (PyDict_Next(py_options, &pos, &key, &value)) {
            if (!PyString_Check(key) || !PyString_Check(value)) {
                g_free(option_keys);
                g_free(option_values);
                PyErr_SetString(PyExc_TypeError,
                                "keys and values must be strings");
                return NULL;
            }

            option_keys[i]   = PyString_AsString(key);
            option_values[i] = PyString_AsString(value);
            i++;
        }
        
        option_keys[len] = NULL;
        option_values[len] = NULL;
    }
    gdk_pixbuf_save_to_callbackv(GDK_PIXBUF(self->obj),
                                 (GdkPixbufSaveFunc) pixbuf_save_func, &data,
                                 type, option_keys, option_values, &error);
    g_free(option_keys);
    g_free(option_values);
 
    if (pyg_error_check(&error))
        return NULL;
    
    Py_INCREF(Py_None);
    return Py_None;
}
#line 13755 "gdk.c"


#line 773 "gdkpixbuf.override"
/* Old declaration accepted characters as 'r', 'g' and 'b' arguments.
 * However, that is flawed, as GTK+ in this case meant '8-bit
 * integers', not 'characters'.  Override is needed to provide
 * backward compatibility for already existing uses. */
static PyObject *
_wrap_gdk_pixbuf_add_alpha(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "substitute_color", "r", "g", "b", NULL };
    int substitute_color, r, g, b;
    PyObject *py_ret;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:gdk.Pixbuf.add_alpha", kwlist,
                                     &substitute_color, &r, &g, &b)) {
        /* Backward compatibility. */
        PyObject *exc_type, *exc_value, *exc_traceback;

        PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);

        if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iccc:gdk.Pixbuf.add_alpha", kwlist,
                                         &substitute_color, &r, &g, &b)) {
            PyErr_Restore(exc_type, exc_value, exc_traceback);
            return NULL;
        }

        Py_XDECREF(exc_type);
        Py_XDECREF(exc_value);
        Py_XDECREF(exc_traceback);
    }
    
    ret = gdk_pixbuf_add_alpha(GDK_PIXBUF(self->obj), substitute_color, r, g, b);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}
#line 13796 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_copy_area(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src_x", "src_y", "width", "height", "dest_pixbuf", "dest_x", "dest_y", NULL };
    int src_x, src_y, width, height, dest_x, dest_y;
    PyGObject *dest_pixbuf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiiiO!ii:Gdk.Pixbuf.copy_area", kwlist, &src_x, &src_y, &width, &height, &PyGdkPixbuf_Type, &dest_pixbuf, &dest_x, &dest_y))
        return NULL;
    
    gdk_pixbuf_copy_area(GDK_PIXBUF(self->obj), src_x, src_y, width, height, GDK_PIXBUF(dest_pixbuf->obj), dest_x, dest_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_saturate_and_pixelate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", "saturation", "pixelate", NULL };
    PyGObject *dest;
    double saturation;
    int pixelate;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!di:Gdk.Pixbuf.saturate_and_pixelate", kwlist, &PyGdkPixbuf_Type, &dest, &saturation, &pixelate))
        return NULL;
    pyg_begin_allow_threads;
    gdk_pixbuf_saturate_and_pixelate(GDK_PIXBUF(self->obj), GDK_PIXBUF(dest->obj), saturation, pixelate);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_scale(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", "dest_x", "dest_y", "dest_width", "dest_height", "offset_x", "offset_y", "scale_x", "scale_y", "interp_type", NULL };
    PyGObject *dest;
    int dest_x, dest_y, dest_width, dest_height;
    GdkInterpType interp_type;
    PyObject *py_interp_type = NULL;
    double offset_x, offset_y, scale_x, scale_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiddddO:Gdk.Pixbuf.scale", kwlist, &PyGdkPixbuf_Type, &dest, &dest_x, &dest_y, &dest_width, &dest_height, &offset_x, &offset_y, &scale_x, &scale_y, &py_interp_type))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INTERP_TYPE, py_interp_type, (gpointer)&interp_type))
        return NULL;
    pyg_begin_allow_threads;
    gdk_pixbuf_scale(GDK_PIXBUF(self->obj), GDK_PIXBUF(dest->obj), dest_x, dest_y, dest_width, dest_height, offset_x, offset_y, scale_x, scale_y, interp_type);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_composite(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", "dest_x", "dest_y", "dest_width", "dest_height", "offset_x", "offset_y", "scale_x", "scale_y", "interp_type", "overall_alpha", NULL };
    PyGObject *dest;
    int dest_x, dest_y, dest_width, dest_height, overall_alpha;
    GdkInterpType interp_type;
    PyObject *py_interp_type = NULL;
    double offset_x, offset_y, scale_x, scale_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiddddOi:Gdk.Pixbuf.composite", kwlist, &PyGdkPixbuf_Type, &dest, &dest_x, &dest_y, &dest_width, &dest_height, &offset_x, &offset_y, &scale_x, &scale_y, &py_interp_type, &overall_alpha))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INTERP_TYPE, py_interp_type, (gpointer)&interp_type))
        return NULL;
    pyg_begin_allow_threads;
    gdk_pixbuf_composite(GDK_PIXBUF(self->obj), GDK_PIXBUF(dest->obj), dest_x, dest_y, dest_width, dest_height, offset_x, offset_y, scale_x, scale_y, interp_type, overall_alpha);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_composite_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", "dest_x", "dest_y", "dest_width", "dest_height", "offset_x", "offset_y", "scale_x", "scale_y", "interp_type", "overall_alpha", "check_x", "check_y", "check_size", "color1", "color2", NULL };
    int dest_x, dest_y, dest_width, dest_height, overall_alpha, check_x, check_y, check_size;
    double offset_x, offset_y, scale_x, scale_y;
    PyObject *py_interp_type = NULL;
    unsigned long color1, color2;
    PyGObject *dest;
    GdkInterpType interp_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiddddOiiiikk:Gdk.Pixbuf.composite_color", kwlist, &PyGdkPixbuf_Type, &dest, &dest_x, &dest_y, &dest_width, &dest_height, &offset_x, &offset_y, &scale_x, &scale_y, &py_interp_type, &overall_alpha, &check_x, &check_y, &check_size, &color1, &color2))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INTERP_TYPE, py_interp_type, (gpointer)&interp_type))
        return NULL;
    if (color1 > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " color1 parameter to unsigned 32 bit integer");
        return NULL;
    }
    if (color2 > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " color2 parameter to unsigned 32 bit integer");
        return NULL;
    }
    pyg_begin_allow_threads;
    gdk_pixbuf_composite_color(GDK_PIXBUF(self->obj), GDK_PIXBUF(dest->obj), dest_x, dest_y, dest_width, dest_height, offset_x, offset_y, scale_x, scale_y, interp_type, overall_alpha, check_x, check_y, check_size, color1, color2);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_scale_simple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest_width", "dest_height", "interp_type", NULL };
    int dest_width, dest_height;
    PyObject *py_interp_type = NULL, *py_ret;
    GdkPixbuf *ret;
    GdkInterpType interp_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiO:Gdk.Pixbuf.scale_simple", kwlist, &dest_width, &dest_height, &py_interp_type))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INTERP_TYPE, py_interp_type, (gpointer)&interp_type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_scale_simple(GDK_PIXBUF(self->obj), dest_width, dest_height, interp_type);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_composite_color_simple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest_width", "dest_height", "interp_type", "overall_alpha", "check_size", "color1", "color2", NULL };
    int dest_width, dest_height, overall_alpha, check_size;
    PyObject *py_interp_type = NULL, *py_ret;
    GdkPixbuf *ret;
    unsigned long color1, color2;
    GdkInterpType interp_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiOiikk:Gdk.Pixbuf.composite_color_simple", kwlist, &dest_width, &dest_height, &py_interp_type, &overall_alpha, &check_size, &color1, &color2))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_INTERP_TYPE, py_interp_type, (gpointer)&interp_type))
        return NULL;
    if (color1 > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " color1 parameter to unsigned 32 bit integer");
        return NULL;
    }
    if (color2 > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " color2 parameter to unsigned 32 bit integer");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_composite_color_simple(GDK_PIXBUF(self->obj), dest_width, dest_height, interp_type, overall_alpha, check_size, color1, color2);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_apply_embedded_orientation(PyGObject *self)
{
    PyObject *py_ret;
    GdkPixbuf *ret;

    
    ret = gdk_pixbuf_apply_embedded_orientation(GDK_PIXBUF(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_get_option(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "key", NULL };
    char *key;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.Pixbuf.get_option", kwlist, &key))
        return NULL;
    
    ret = gdk_pixbuf_get_option(GDK_PIXBUF(self->obj), key);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_flip(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "horizontal", NULL };
    int horizontal;
    PyObject *py_ret;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Pixbuf.flip", kwlist, &horizontal))
        return NULL;
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_flip(GDK_PIXBUF(self->obj), horizontal);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 98 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_get_pixels_array(PyGObject *self)
{


#ifdef HAVE_NUMPY
    GdkPixbuf *pixbuf = GDK_PIXBUF(self->obj);
    PyArrayObject *array;
    int dims[3] = { 0, 0, 3 };

    if (!have_numpy())
	return NULL;

    dims[0] = gdk_pixbuf_get_height(pixbuf);
    dims[1] = gdk_pixbuf_get_width(pixbuf);
    if (gdk_pixbuf_get_has_alpha(pixbuf))
        dims[2] = 4;
    array = (PyArrayObject *)PyArray_FromDimsAndData(3, dims, PyArray_UBYTE,
                                        (char *)gdk_pixbuf_get_pixels(pixbuf));
    if (array == NULL)
        return NULL;

    array->strides[0] = gdk_pixbuf_get_rowstride(pixbuf);
    /* the array holds a ref to the pixbuf pixels through this wrapper*/
    Py_INCREF(self);
    array->base = (PyObject *)self;
    return PyArray_Return(array);
#else
    PyErr_SetString(PyExc_RuntimeError, "pygtk was not compiled with Numeric Python support");
    return NULL;
#endif
}
#line 14050 "gdk.c"


static const PyMethodDef _PyGdkPixbuf_methods[] = {
    { "render_threshold_alpha", (PyCFunction)_wrap_gdk_pixbuf_render_threshold_alpha, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "render_to_drawable", (PyCFunction)_wrap_gdk_pixbuf_render_to_drawable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "render_to_drawable_alpha", (PyCFunction)_wrap_gdk_pixbuf_render_to_drawable_alpha, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "render_pixmap_and_mask", (PyCFunction)_wrap_gdk_pixbuf_render_pixmap_and_mask, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_from_drawable", (PyCFunction)_wrap_gdk_pixbuf_get_from_drawable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_from_image", (PyCFunction)_wrap_gdk_pixbuf_get_from_image, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rotate_simple", (PyCFunction)_wrap_gdk_pixbuf_rotate_simple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_colorspace", (PyCFunction)_wrap_gdk_pixbuf_get_colorspace, METH_NOARGS,
      NULL },
    { "get_n_channels", (PyCFunction)_wrap_gdk_pixbuf_get_n_channels, METH_NOARGS,
      NULL },
    { "get_has_alpha", (PyCFunction)_wrap_gdk_pixbuf_get_has_alpha, METH_NOARGS,
      NULL },
    { "get_bits_per_sample", (PyCFunction)_wrap_gdk_pixbuf_get_bits_per_sample, METH_NOARGS,
      NULL },
    { "get_pixels", (PyCFunction)_wrap_gdk_pixbuf_get_pixels, METH_NOARGS,
      NULL },
    { "get_width", (PyCFunction)_wrap_gdk_pixbuf_get_width, METH_NOARGS,
      NULL },
    { "get_height", (PyCFunction)_wrap_gdk_pixbuf_get_height, METH_NOARGS,
      NULL },
    { "get_rowstride", (PyCFunction)_wrap_gdk_pixbuf_get_rowstride, METH_NOARGS,
      NULL },
    { "copy", (PyCFunction)_wrap_gdk_pixbuf_copy, METH_NOARGS,
      NULL },
    { "subpixbuf", (PyCFunction)_wrap_gdk_pixbuf_new_subpixbuf, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fill", (PyCFunction)_wrap_gdk_pixbuf_fill, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "save", (PyCFunction)_wrap_gdk_pixbuf_save, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "save_to_callback", (PyCFunction)_wrap_gdk_pixbuf_save_to_callback, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_alpha", (PyCFunction)_wrap_gdk_pixbuf_add_alpha, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy_area", (PyCFunction)_wrap_gdk_pixbuf_copy_area, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "saturate_and_pixelate", (PyCFunction)_wrap_gdk_pixbuf_saturate_and_pixelate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scale", (PyCFunction)_wrap_gdk_pixbuf_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "composite", (PyCFunction)_wrap_gdk_pixbuf_composite, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "composite_color", (PyCFunction)_wrap_gdk_pixbuf_composite_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scale_simple", (PyCFunction)_wrap_gdk_pixbuf_scale_simple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "composite_color_simple", (PyCFunction)_wrap_gdk_pixbuf_composite_color_simple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "apply_embedded_orientation", (PyCFunction)_wrap_gdk_pixbuf_apply_embedded_orientation, METH_NOARGS,
      NULL },
    { "get_option", (PyCFunction)_wrap_gdk_pixbuf_get_option, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flip", (PyCFunction)_wrap_gdk_pixbuf_flip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_pixels_array", (PyCFunction)_wrap_gdk_pixbuf_get_pixels_array, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 62 "gdkpixbuf.override"
#ifdef HAVE_NUMPY
static int
have_numpy(void)
{
    /* Return 1 if Numeric is available,
       return 0 and set and exception if it is not. */
    static int import_done = 0;
    static PyObject *exc_type=NULL, *exc_value=NULL;
    PyObject *exc_tb=NULL;

    if (exc_type != NULL) {
	PyErr_Restore(exc_type, exc_value, NULL);
	return 0;
    }
    if (!import_done) {
	import_done = 1;
	import_array1(1);
	if (PyErr_Occurred()) {
	    PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
	    Py_INCREF(exc_type);
	    Py_XINCREF(exc_value);
	    PyErr_Restore(exc_type, exc_value, exc_tb);
	    return 0;
	}
    }
    return 1;
}
#endif
static PyObject *
_wrap_gdk_pixbuf__get_pixel_array(PyGObject *self, void *closure)
{
    PyErr_Warn(PyExc_DeprecationWarning, "use pixbuf.get_pixels_array() instead of the pixbuf.pixel_array attribute");
    return _wrap_gdk_pixbuf_get_pixels_array(self);
}
#line 14156 "gdk.c"


static const PyGetSetDef gdk_pixbuf_getsets[] = {
    { "pixel_array", (getter)_wrap_gdk_pixbuf__get_pixel_array, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkPixbuf_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Pixbuf",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPixbuf_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_pixbuf_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pixbuf_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPixbufAnimation ----------- */

static int
_wrap_gdk_pixbuf_animation_new_from_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Gdk.PixbufAnimation.__init__", kwlist, &filename))
        return -1;
    self->obj = (GObject *)gdk_pixbuf_animation_new_from_file(filename, &error);
    if (pyg_error_check(&error))
        return -1;

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkPixbufAnimation object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_pixbuf_animation_get_width(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_animation_get_width(GDK_PIXBUF_ANIMATION(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_animation_get_height(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_animation_get_height(GDK_PIXBUF_ANIMATION(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_animation_is_static_image(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_animation_is_static_image(GDK_PIXBUF_ANIMATION(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_pixbuf_animation_get_static_image(PyGObject *self)
{
    GdkPixbuf *ret;

    
    ret = gdk_pixbuf_animation_get_static_image(GDK_PIXBUF_ANIMATION(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 519 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_animation_get_iter(PyGObject *self, PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "start_time", NULL };
    GdkPixbufAnimationIter *ret;
    double start_time = 0.0;
    GTimeVal starttime, *starttimep;
    PyObject *pyretval;

    if (!PyArg_ParseTupleAndKeywords(
	    args, kwargs, "|d:GdkPixbufAnimation.get_iter",
	    kwlist, &start_time))
        return NULL;

    if (start_time > 0.0) {
	starttime.tv_sec = (glong) start_time;
	starttime.tv_usec = (glong)((start_time - starttime.tv_sec)
				    * G_USEC_PER_SEC);
	starttimep = &starttime;
    } else if (start_time == 0.0) {
	starttimep = NULL;
    } else {
        PyErr_SetString(PyExc_ValueError, "start_time must be >= 0.0");
        return NULL;
    }

    ret = gdk_pixbuf_animation_get_iter(GDK_PIXBUF_ANIMATION(self->obj),
					starttimep);
    /* pygobject_new handles NULL checking */
    pyretval = pygobject_new((GObject *)ret);
    if (ret)
        g_object_unref(ret);
    return pyretval;
}
#line 14316 "gdk.c"


static const PyMethodDef _PyGdkPixbufAnimation_methods[] = {
    { "get_width", (PyCFunction)_wrap_gdk_pixbuf_animation_get_width, METH_NOARGS,
      NULL },
    { "get_height", (PyCFunction)_wrap_gdk_pixbuf_animation_get_height, METH_NOARGS,
      NULL },
    { "is_static_image", (PyCFunction)_wrap_gdk_pixbuf_animation_is_static_image, METH_NOARGS,
      NULL },
    { "get_static_image", (PyCFunction)_wrap_gdk_pixbuf_animation_get_static_image, METH_NOARGS,
      NULL },
    { "get_iter", (PyCFunction)_wrap_gdk_pixbuf_animation_get_iter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkPixbufAnimation_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PixbufAnimation",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPixbufAnimation_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pixbuf_animation_new_from_file,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPixbufAnimationIter ----------- */

static PyObject *
_wrap_gdk_pixbuf_animation_iter_get_delay_time(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_animation_iter_get_delay_time(GDK_PIXBUF_ANIMATION_ITER(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_pixbuf_animation_iter_get_pixbuf(PyGObject *self)
{
    GdkPixbuf *ret;

    
    ret = gdk_pixbuf_animation_iter_get_pixbuf(GDK_PIXBUF_ANIMATION_ITER(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_animation_iter_on_currently_loading_frame(PyGObject *self)
{
    int ret;

    
    ret = gdk_pixbuf_animation_iter_on_currently_loading_frame(GDK_PIXBUF_ANIMATION_ITER(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 556 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_animation_iter_advance(PyGObject *self, PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "current_time", NULL };
    gboolean ret;
    double current_time = 0.0;
    GTimeVal currenttime, *currenttimep;

    if (!PyArg_ParseTupleAndKeywords(
	    args, kwargs, "|d:GdkPixbufAnimationIter.advance",
	    kwlist, &current_time))
        return NULL;

    if (current_time > 0.0) {
	currenttime.tv_sec = (glong) current_time;
	currenttime.tv_usec = (glong)((current_time - currenttime.tv_sec)
				    * G_USEC_PER_SEC);
	currenttimep = &currenttime;
    } else if (current_time == 0.0) {
	currenttimep = NULL;
    } else {
        PyErr_SetString(PyExc_ValueError, "current_time must be >= 0.0");
        return NULL;
    }

    ret = gdk_pixbuf_animation_iter_advance(GDK_PIXBUF_ANIMATION_ITER(self->obj),
					    currenttimep);

    return PyBool_FromLong(ret);
}
#line 14449 "gdk.c"


static const PyMethodDef _PyGdkPixbufAnimationIter_methods[] = {
    { "get_delay_time", (PyCFunction)_wrap_gdk_pixbuf_animation_iter_get_delay_time, METH_NOARGS,
      NULL },
    { "get_pixbuf", (PyCFunction)_wrap_gdk_pixbuf_animation_iter_get_pixbuf, METH_NOARGS,
      NULL },
    { "on_currently_loading_frame", (PyCFunction)_wrap_gdk_pixbuf_animation_iter_on_currently_loading_frame, METH_NOARGS,
      NULL },
    { "advance", (PyCFunction)_wrap_gdk_pixbuf_animation_iter_advance, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkPixbufAnimationIter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PixbufAnimationIter",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPixbufAnimationIter_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPixbufLoader ----------- */

#line 456 "gdkpixbuf.override"
static int
_wrap_gdk_pixbuf_loader_new_with_type(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "image_type", NULL };
    char *image_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|s:GdkPixbufLoader.__init__",
				     kwlist, &image_type))
        return -1;

    if (image_type == NULL) {
	self->obj = (GObject *)gdk_pixbuf_loader_new();
    } else {
	GError *error = NULL;
	self->obj = (GObject *)gdk_pixbuf_loader_new_with_type(image_type,
							       &error);
	if (pyg_error_check(&error))
	    return -1;
    }

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError,
			"could not create GdkPixbufLoader object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 14545 "gdk.c"


#line 489 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_loader_write(PyGObject *self,
                              PyObject *args,
                              PyObject *kwargs)
{
    static char *kwlist[] = { "buf", "count", NULL };
    Py_ssize_t buf_len;
    int count = -1, ret;
    guchar *buf;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s#|i:GdkPixbufLoader.write",
                                     kwlist, &buf, &buf_len, &count))
        return NULL;

    if (count < 0 || count > buf_len) {
        count = buf_len;
    }
    
    ret = gdk_pixbuf_loader_write(GDK_PIXBUF_LOADER(self->obj),
                                  buf, count, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    
    return PyBool_FromLong(ret);
}
#line 14577 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_loader_get_pixbuf(PyGObject *self)
{
    GdkPixbuf *ret;

    
    ret = gdk_pixbuf_loader_get_pixbuf(GDK_PIXBUF_LOADER(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_loader_get_animation(PyGObject *self)
{
    GdkPixbufAnimation *ret;

    
    ret = gdk_pixbuf_loader_get_animation(GDK_PIXBUF_LOADER(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_loader_close(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = gdk_pixbuf_loader_close(GDK_PIXBUF_LOADER(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_pixbuf_loader_set_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", NULL };
    int width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.PixbufLoader.set_size", kwlist, &width, &height))
        return NULL;
    
    gdk_pixbuf_loader_set_size(GDK_PIXBUF_LOADER(self->obj), width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 713 "gdkpixbuf.override"
static PyObject *pygtk_make_pixbuf_format_dict(GdkPixbufFormat *format);
static PyObject *
_wrap_gdk_pixbuf_loader_get_format(PyGObject *self)
{
    GdkPixbufFormat *format;

    format = gdk_pixbuf_loader_get_format(GDK_PIXBUF_LOADER(self->obj));

    if (format)
	return pygtk_make_pixbuf_format_dict(format);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 14649 "gdk.c"


static PyObject *
_wrap_GdkPixbufLoader__do_size_prepared(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "width", "height", NULL };
    PyGObject *self;
    int width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Gdk.PixbufLoader.size_prepared", kwlist, &PyGdkPixbufLoader_Type, &self, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_PIXBUF_LOADER_CLASS(klass)->size_prepared)
        GDK_PIXBUF_LOADER_CLASS(klass)->size_prepared(GDK_PIXBUF_LOADER(self->obj), width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.PixbufLoader.size_prepared not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkPixbufLoader__do_area_prepared(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.PixbufLoader.area_prepared", kwlist, &PyGdkPixbufLoader_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_PIXBUF_LOADER_CLASS(klass)->area_prepared)
        GDK_PIXBUF_LOADER_CLASS(klass)->area_prepared(GDK_PIXBUF_LOADER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.PixbufLoader.area_prepared not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkPixbufLoader__do_area_updated(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "x", "y", "width", "height", NULL };
    PyGObject *self;
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiii:Gdk.PixbufLoader.area_updated", kwlist, &PyGdkPixbufLoader_Type, &self, &x, &y, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_PIXBUF_LOADER_CLASS(klass)->area_updated)
        GDK_PIXBUF_LOADER_CLASS(klass)->area_updated(GDK_PIXBUF_LOADER(self->obj), x, y, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.PixbufLoader.area_updated not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkPixbufLoader__do_closed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.PixbufLoader.closed", kwlist, &PyGdkPixbufLoader_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_PIXBUF_LOADER_CLASS(klass)->closed)
        GDK_PIXBUF_LOADER_CLASS(klass)->closed(GDK_PIXBUF_LOADER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.PixbufLoader.closed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkPixbufLoader_methods[] = {
    { "write", (PyCFunction)_wrap_gdk_pixbuf_loader_write, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_pixbuf", (PyCFunction)_wrap_gdk_pixbuf_loader_get_pixbuf, METH_NOARGS,
      NULL },
    { "get_animation", (PyCFunction)_wrap_gdk_pixbuf_loader_get_animation, METH_NOARGS,
      NULL },
    { "close", (PyCFunction)_wrap_gdk_pixbuf_loader_close, METH_NOARGS,
      NULL },
    { "set_size", (PyCFunction)_wrap_gdk_pixbuf_loader_set_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_format", (PyCFunction)_wrap_gdk_pixbuf_loader_get_format, METH_NOARGS,
      NULL },
    { "do_size_prepared", (PyCFunction)_wrap_GdkPixbufLoader__do_size_prepared, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_area_prepared", (PyCFunction)_wrap_GdkPixbufLoader__do_area_prepared, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_area_updated", (PyCFunction)_wrap_GdkPixbufLoader__do_area_updated, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_closed", (PyCFunction)_wrap_GdkPixbufLoader__do_closed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkPixbufLoader_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PixbufLoader",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPixbufLoader_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pixbuf_loader_new_with_type,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GdkPixbufLoader__proxy_do_size_prepared(GdkPixbufLoader *self, int width, int height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_width);
    PyTuple_SET_ITEM(py_args, 1, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_size_prepared");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkPixbufLoader__proxy_do_area_prepared(GdkPixbufLoader *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_area_prepared");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkPixbufLoader__proxy_do_area_updated(GdkPixbufLoader *self, int x, int y, int width, int height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(4);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_width);
    PyTuple_SET_ITEM(py_args, 3, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_area_updated");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkPixbufLoader__proxy_do_closed(GdkPixbufLoader *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_closed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkPixbufLoader_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkPixbufLoaderClass *klass = GDK_PIXBUF_LOADER_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_size_prepared");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "size_prepared")))
            klass->size_prepared = _wrap_GdkPixbufLoader__proxy_do_size_prepared;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_area_prepared");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "area_prepared")))
            klass->area_prepared = _wrap_GdkPixbufLoader__proxy_do_area_prepared;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_area_updated");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "area_updated")))
            klass->area_updated = _wrap_GdkPixbufLoader__proxy_do_area_updated;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_closed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "closed")))
            klass->closed = _wrap_GdkPixbufLoader__proxy_do_closed;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GdkPixbufSimpleAnim ----------- */

static int
_wrap_gdk_pixbuf_simple_anim_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", "rate", NULL };
    int width, height;
    double rate;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iid:Gdk.PixbufSimpleAnim.__init__", kwlist, &width, &height, &rate))
        return -1;
    self->obj = (GObject *)gdk_pixbuf_simple_anim_new(width, height, rate);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkPixbufSimpleAnim object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_pixbuf_simple_anim_add_frame(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixbuf", NULL };
    PyGObject *pixbuf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.PixbufSimpleAnim.add_frame", kwlist, &PyGdkPixbuf_Type, &pixbuf))
        return NULL;
    
    gdk_pixbuf_simple_anim_add_frame(GDK_PIXBUF_SIMPLE_ANIM(self->obj), GDK_PIXBUF(pixbuf->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkPixbufSimpleAnim_methods[] = {
    { "add_frame", (PyCFunction)_wrap_gdk_pixbuf_simple_anim_add_frame, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkPixbufSimpleAnim_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PixbufSimpleAnim",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkPixbufSimpleAnim_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_pixbuf_simple_anim_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkPixbufSimpleAnimIter ----------- */

PyTypeObject G_GNUC_INTERNAL PyGdkPixbufSimpleAnimIter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.PixbufSimpleAnimIter",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GdkScreen ----------- */

#line 996 "./gdk.override"
static int
_wrap_gdk_screen_new (PyGObject *self)
{
    self->obj = (GObject *)gdk_screen_get_default();
    if (!self->obj) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get default display");
	return -1;
    }

    g_object_ref(self->obj);

    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 15256 "gdk.c"


static PyObject *
_wrap_gdk_screen_get_default_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_screen_get_default_colormap(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_set_default_colormap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", NULL };
    PyGObject *colormap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Screen.set_default_colormap", kwlist, &PyGdkColormap_Type, &colormap))
        return NULL;
    
    gdk_screen_set_default_colormap(GDK_SCREEN(self->obj), GDK_COLORMAP(colormap->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_screen_get_system_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_screen_get_system_colormap(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_system_visual(PyGObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_screen_get_system_visual(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_rgb_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_screen_get_rgb_colormap(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_rgba_colormap(PyGObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_screen_get_rgba_colormap(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_rgb_visual(PyGObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_screen_get_rgb_visual(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_rgba_visual(PyGObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_screen_get_rgba_visual(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_root_window(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_screen_get_root_window(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_display(PyGObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_screen_get_display(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_number(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_number(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_width(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_width(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_height(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_height(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_width_mm(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_width_mm(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_height_mm(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_height_mm(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 1013 "./gdk.override"
static PyObject *
_wrap_gdk_screen_list_visuals(PyGObject *self)
{
    GList *visl;
    guint nvisl;
    int i;
    PyObject *list;

    visl = gdk_screen_list_visuals(GDK_SCREEN(self->obj));
    nvisl = g_list_length(visl);

    if ((list = PyList_New(nvisl)) == NULL)
	return NULL;

    for (i = 0; i < nvisl; i++) {
	PyObject *item;

	item = pygobject_new((GObject *)g_list_nth_data(visl, i));
	PyList_SetItem(list, i, item);
    }

    g_list_free(visl);

    return list;
}
#line 15463 "gdk.c"


#line 1040 "./gdk.override"
static PyObject *
_wrap_gdk_screen_get_toplevel_windows(PyGObject *self)
{
    GList *topl;
    guint ntopl;
    int i;
    PyObject *list;

    topl = gdk_screen_get_toplevel_windows(GDK_SCREEN(self->obj));
    ntopl = g_list_length(topl);

    if ((list = PyList_New(ntopl)) == NULL)
	return NULL;

    for (i = 0; i < ntopl; i++) {
	PyObject *item;

	item = pygobject_new((GObject *)g_list_nth_data(topl, i));
	PyList_SetItem(list, i, item);
    }

    g_list_free(topl);

    return list;
}
#line 15492 "gdk.c"


static PyObject *
_wrap_gdk_screen_make_display_name(PyGObject *self)
{
    gchar *ret;

    
    ret = gdk_screen_make_display_name(GDK_SCREEN(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_screen_get_n_monitors(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_get_n_monitors(GDK_SCREEN(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 1067 "./gdk.override"
static PyObject *
_wrap_gdk_screen_get_monitor_geometry(PyGObject *self, PyObject *args,
				      PyObject *kwargs)
{
    static char *kwlist[] = { "monitor_num", NULL };
    int monitor_num;
    GdkRectangle dest = { 0, 0, 0, 0 };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:GdkScreen.get_monitor_geometry",
				     kwlist, &monitor_num))
        return NULL;

    gdk_screen_get_monitor_geometry(GDK_SCREEN(self->obj), monitor_num, &dest);

    return pyg_boxed_new(GDK_TYPE_RECTANGLE, &dest, TRUE, TRUE);
}
#line 15541 "gdk.c"


static PyObject *
_wrap_gdk_screen_get_monitor_at_point(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    int x, y, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Gdk.Screen.get_monitor_at_point", kwlist, &x, &y))
        return NULL;
    
    ret = gdk_screen_get_monitor_at_point(GDK_SCREEN(self->obj), x, y);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_monitor_at_window(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", NULL };
    PyGObject *window;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Screen.get_monitor_at_window", kwlist, &PyGdkWindow_Type, &window))
        return NULL;
    
    ret = gdk_screen_get_monitor_at_window(GDK_SCREEN(self->obj), GDK_WINDOW(window->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_broadcast_client_message(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    GdkEvent *event = NULL;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Gdk.Screen.broadcast_client_message", kwlist, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    
    gdk_screen_broadcast_client_message(GDK_SCREEN(self->obj), event);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1086 "./gdk.override"
static PyObject *
_wrap_gdk_screen_get_setting(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    GValue value = { 0, };
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:GdkScreen.get_setting",
				     kwlist, &name))
	return NULL;

    /* this is a bit of a hack.  We don't know what type of setting it is */
    g_value_init(&value, GDK_TYPE_COLOR);
    if (gdk_screen_get_setting(GDK_SCREEN(self->obj), name, &value))
	goto got_setting;
    g_value_unset(&value);
    g_value_init(&value, G_TYPE_INT);
    if (gdk_screen_get_setting(GDK_SCREEN(self->obj), name, &value))
	goto got_setting;
    g_value_unset(&value);
    g_value_init(&value, G_TYPE_STRING);
    if (gdk_screen_get_setting(GDK_SCREEN(self->obj), name, &value))
	goto got_setting;
    g_value_unset(&value);
    /* setting not found */
    PyErr_SetString(PyExc_ValueError, "could not find setting");
    return NULL;

 got_setting:
    ret = pyg_value_as_pyobject(&value, TRUE);
    g_value_unset(&value);
    return ret;
}
#line 15630 "gdk.c"


static PyObject *
_wrap_gtk_alternative_dialog_button_order(PyGObject *self)
{
    int ret;

    
    ret = gtk_alternative_dialog_button_order(GDK_SCREEN(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_screen_is_composited(PyGObject *self)
{
    int ret;

    
    ret = gdk_screen_is_composited(GDK_SCREEN(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 1541 "./gdk.override"
static PyObject *
_wrap_gdk_screen_set_font_options(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "options", NULL };
    PyGObject *py_options;
    const cairo_font_options_t *options;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O:GdkScreen.set_font_options",
                                     kwlist, &py_options))
        return NULL;
    if ((PyObject*)py_options == Py_None)
        options = NULL;
    else if (pygobject_check(py_options, &PycairoFontOptions_Type))
        options = ((PycairoFontOptions *)py_options)->font_options;
    else {
        PyErr_SetString(PyExc_TypeError,
                        "options must be a cairo.FontOptions or None");
        return NULL;
    }

    gdk_screen_set_font_options(GDK_SCREEN(self->obj), options);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 15685 "gdk.c"


#line 1527 "./gdk.override"
static PyObject *
_wrap_gdk_screen_get_font_options(PyGObject *self)
{
    const cairo_font_options_t *options;

    options = gdk_screen_get_font_options(GDK_SCREEN(self->obj));
    if (!options) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    return PycairoFontOptions_FromFontOptions(cairo_font_options_copy(options));
}
#line 15701 "gdk.c"


static PyObject *
_wrap_gdk_screen_set_resolution(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dpi", NULL };
    double dpi;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:Gdk.Screen.set_resolution", kwlist, &dpi))
        return NULL;
    
    gdk_screen_set_resolution(GDK_SCREEN(self->obj), dpi);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_screen_get_resolution(PyGObject *self)
{
    double ret;

    
    ret = gdk_screen_get_resolution(GDK_SCREEN(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gdk_screen_get_active_window(PyGObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_screen_get_active_window(GDK_SCREEN(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_screen_get_monitor_width_mm(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "monitor_num", NULL };
    int monitor_num, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Screen.get_monitor_width_mm", kwlist, &monitor_num))
        return NULL;
    
    ret = gdk_screen_get_monitor_width_mm(GDK_SCREEN(self->obj), monitor_num);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_monitor_height_mm(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "monitor_num", NULL };
    int monitor_num, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Screen.get_monitor_height_mm", kwlist, &monitor_num))
        return NULL;
    
    ret = gdk_screen_get_monitor_height_mm(GDK_SCREEN(self->obj), monitor_num);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_get_monitor_plug_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "monitor_num", NULL };
    int monitor_num;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Gdk.Screen.get_monitor_plug_name", kwlist, &monitor_num))
        return NULL;
    
    ret = gdk_screen_get_monitor_plug_name(GDK_SCREEN(self->obj), monitor_num);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkScreen__do_size_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Screen.size_changed", kwlist, &PyGdkScreen_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_SCREEN_CLASS(klass)->size_changed)
        GDK_SCREEN_CLASS(klass)->size_changed(GDK_SCREEN(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Screen.size_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GdkScreen__do_composited_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Gdk.Screen.composited_changed", kwlist, &PyGdkScreen_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GDK_SCREEN_CLASS(klass)->composited_changed)
        GDK_SCREEN_CLASS(klass)->composited_changed(GDK_SCREEN(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Gdk.Screen.composited_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGdkScreen_methods[] = {
    { "get_default_colormap", (PyCFunction)_wrap_gdk_screen_get_default_colormap, METH_NOARGS,
      NULL },
    { "set_default_colormap", (PyCFunction)_wrap_gdk_screen_set_default_colormap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_system_colormap", (PyCFunction)_wrap_gdk_screen_get_system_colormap, METH_NOARGS,
      NULL },
    { "get_system_visual", (PyCFunction)_wrap_gdk_screen_get_system_visual, METH_NOARGS,
      NULL },
    { "get_rgb_colormap", (PyCFunction)_wrap_gdk_screen_get_rgb_colormap, METH_NOARGS,
      NULL },
    { "get_rgba_colormap", (PyCFunction)_wrap_gdk_screen_get_rgba_colormap, METH_NOARGS,
      NULL },
    { "get_rgb_visual", (PyCFunction)_wrap_gdk_screen_get_rgb_visual, METH_NOARGS,
      NULL },
    { "get_rgba_visual", (PyCFunction)_wrap_gdk_screen_get_rgba_visual, METH_NOARGS,
      NULL },
    { "get_root_window", (PyCFunction)_wrap_gdk_screen_get_root_window, METH_NOARGS,
      NULL },
    { "get_display", (PyCFunction)_wrap_gdk_screen_get_display, METH_NOARGS,
      NULL },
    { "get_number", (PyCFunction)_wrap_gdk_screen_get_number, METH_NOARGS,
      NULL },
    { "get_width", (PyCFunction)_wrap_gdk_screen_get_width, METH_NOARGS,
      NULL },
    { "get_height", (PyCFunction)_wrap_gdk_screen_get_height, METH_NOARGS,
      NULL },
    { "get_width_mm", (PyCFunction)_wrap_gdk_screen_get_width_mm, METH_NOARGS,
      NULL },
    { "get_height_mm", (PyCFunction)_wrap_gdk_screen_get_height_mm, METH_NOARGS,
      NULL },
    { "list_visuals", (PyCFunction)_wrap_gdk_screen_list_visuals, METH_NOARGS,
      NULL },
    { "get_toplevel_windows", (PyCFunction)_wrap_gdk_screen_get_toplevel_windows, METH_NOARGS,
      NULL },
    { "make_display_name", (PyCFunction)_wrap_gdk_screen_make_display_name, METH_NOARGS,
      NULL },
    { "get_n_monitors", (PyCFunction)_wrap_gdk_screen_get_n_monitors, METH_NOARGS,
      NULL },
    { "get_monitor_geometry", (PyCFunction)_wrap_gdk_screen_get_monitor_geometry, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_monitor_at_point", (PyCFunction)_wrap_gdk_screen_get_monitor_at_point, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_monitor_at_window", (PyCFunction)_wrap_gdk_screen_get_monitor_at_window, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "broadcast_client_message", (PyCFunction)_wrap_gdk_screen_broadcast_client_message, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_setting", (PyCFunction)_wrap_gdk_screen_get_setting, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "alternative_dialog_button_order", (PyCFunction)_wrap_gtk_alternative_dialog_button_order, METH_NOARGS,
      NULL },
    { "is_composited", (PyCFunction)_wrap_gdk_screen_is_composited, METH_NOARGS,
      NULL },
    { "set_font_options", (PyCFunction)_wrap_gdk_screen_set_font_options, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_font_options", (PyCFunction)_wrap_gdk_screen_get_font_options, METH_NOARGS,
      NULL },
    { "set_resolution", (PyCFunction)_wrap_gdk_screen_set_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_resolution", (PyCFunction)_wrap_gdk_screen_get_resolution, METH_NOARGS,
      NULL },
    { "get_active_window", (PyCFunction)_wrap_gdk_screen_get_active_window, METH_NOARGS,
      NULL },
    { "get_monitor_width_mm", (PyCFunction)_wrap_gdk_screen_get_monitor_width_mm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_monitor_height_mm", (PyCFunction)_wrap_gdk_screen_get_monitor_height_mm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_monitor_plug_name", (PyCFunction)_wrap_gdk_screen_get_monitor_plug_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_size_changed", (PyCFunction)_wrap_GdkScreen__do_size_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_composited_changed", (PyCFunction)_wrap_GdkScreen__do_composited_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGdkScreen_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Screen",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkScreen_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_screen_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GdkScreen__proxy_do_size_changed(GdkScreen *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_size_changed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GdkScreen__proxy_do_composited_changed(GdkScreen *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_composited_changed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GdkScreen_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GdkScreenClass *klass = GDK_SCREEN_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_size_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "size_changed")))
            klass->size_changed = _wrap_GdkScreen__proxy_do_size_changed;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_composited_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "composited_changed")))
            klass->composited_changed = _wrap_GdkScreen__proxy_do_composited_changed;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GdkVisual ----------- */

static int
_wrap_gdk_visual_get_best_with_both(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "depth", "visual_type", NULL };
    int depth;
    GdkVisualType visual_type;
    PyObject *py_visual_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO:Gdk.Visual.__init__", kwlist, &depth, &py_visual_type))
        return -1;
    if (pyg_enum_get_value(GDK_TYPE_VISUAL_TYPE, py_visual_type, (gpointer)&visual_type))
        return -1;
    self->obj = (GObject *)gdk_visual_get_best_with_both(depth, visual_type);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GdkVisual object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gdk_visual_get_screen(PyGObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_visual_get_screen(GDK_VISUAL(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGdkVisual_methods[] = {
    { "get_screen", (PyCFunction)_wrap_gdk_visual_get_screen, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gdk_visual__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = GDK_VISUAL(pygobject_get(self))->type;
    return pyg_enum_from_gtype(GDK_TYPE_VISUAL_TYPE, ret);
}

static PyObject *
_wrap_gdk_visual__get_depth(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->depth;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_byte_order(PyObject *self, void *closure)
{
    gint ret;

    ret = GDK_VISUAL(pygobject_get(self))->byte_order;
    return pyg_enum_from_gtype(GDK_TYPE_BYTE_ORDER, ret);
}

static PyObject *
_wrap_gdk_visual__get_colormap_size(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->colormap_size;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_bits_per_rgb(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->bits_per_rgb;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_red_mask(PyObject *self, void *closure)
{
    guint32 ret;

    ret = GDK_VISUAL(pygobject_get(self))->red_mask;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_visual__get_red_shift(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->red_shift;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_red_prec(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->red_prec;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_green_mask(PyObject *self, void *closure)
{
    guint32 ret;

    ret = GDK_VISUAL(pygobject_get(self))->green_mask;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_visual__get_green_shift(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->green_shift;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_green_prec(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->green_prec;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_blue_mask(PyObject *self, void *closure)
{
    guint32 ret;

    ret = GDK_VISUAL(pygobject_get(self))->blue_mask;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_visual__get_blue_shift(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->blue_shift;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual__get_blue_prec(PyObject *self, void *closure)
{
    int ret;

    ret = GDK_VISUAL(pygobject_get(self))->blue_prec;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gdk_visual_getsets[] = {
    { "type", (getter)_wrap_gdk_visual__get_type, (setter)0 },
    { "depth", (getter)_wrap_gdk_visual__get_depth, (setter)0 },
    { "byte_order", (getter)_wrap_gdk_visual__get_byte_order, (setter)0 },
    { "colormap_size", (getter)_wrap_gdk_visual__get_colormap_size, (setter)0 },
    { "bits_per_rgb", (getter)_wrap_gdk_visual__get_bits_per_rgb, (setter)0 },
    { "red_mask", (getter)_wrap_gdk_visual__get_red_mask, (setter)0 },
    { "red_shift", (getter)_wrap_gdk_visual__get_red_shift, (setter)0 },
    { "red_prec", (getter)_wrap_gdk_visual__get_red_prec, (setter)0 },
    { "green_mask", (getter)_wrap_gdk_visual__get_green_mask, (setter)0 },
    { "green_shift", (getter)_wrap_gdk_visual__get_green_shift, (setter)0 },
    { "green_prec", (getter)_wrap_gdk_visual__get_green_prec, (setter)0 },
    { "blue_mask", (getter)_wrap_gdk_visual__get_blue_mask, (setter)0 },
    { "blue_shift", (getter)_wrap_gdk_visual__get_blue_shift, (setter)0 },
    { "blue_prec", (getter)_wrap_gdk_visual__get_blue_prec, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGdkVisual_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gtk.gdk.Visual",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGdkVisual_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gdk_visual_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gdk_visual_get_best_with_both,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};





/* ----------- functions ----------- */

static PyObject *
_wrap_gdk_notify_startup_complete(PyObject *self)
{
    
    gdk_notify_startup_complete();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_display_arg_name(PyObject *self)
{
    const gchar *ret;

    
    ret = gdk_get_display_arg_name();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_program_class(PyObject *self)
{
    const gchar *ret;

    
    ret = gdk_get_program_class();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_set_program_class(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "program_class", NULL };
    char *program_class;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:set_program_class", kwlist, &program_class))
        return NULL;
    
    gdk_set_program_class(program_class);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_display(PyObject *self)
{
    const gchar *ret;

    
    ret = gdk_get_display();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pointer_grab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", "owner_events", "event_mask", "confine_to", "cursor", "time", NULL };
    int owner_events = FALSE;
    PyObject *py_event_mask = NULL, *py_cursor = Py_None;
    GdkEventMask event_mask = 0;
    GdkWindow *confine_to = NULL;
    GdkCursor *cursor = NULL;
    PyGObject *window, *py_confine_to = NULL;
    unsigned long time = GDK_CURRENT_TIME;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|iOOOk:pointer_grab", kwlist, &PyGdkWindow_Type, &window, &owner_events, &py_event_mask, &py_confine_to, &py_cursor, &time))
        return NULL;
    if (py_event_mask && pyg_flags_get_value(GDK_TYPE_EVENT_MASK, py_event_mask, (gpointer)&event_mask))
        return NULL;
    if ((PyObject *)py_confine_to == Py_None)
        confine_to = NULL;
    else if (py_confine_to && pygobject_check(py_confine_to, &PyGdkWindow_Type))
        confine_to = GDK_WINDOW(py_confine_to->obj);
    else if (py_confine_to) {
        PyErr_SetString(PyExc_TypeError, "confine_to should be a GdkWindow or None");
        return NULL;
    }
    if (pyg_boxed_check(py_cursor, GDK_TYPE_CURSOR))
        cursor = pyg_boxed_get(py_cursor, GdkCursor);
    else if (py_cursor != Py_None) {
        PyErr_SetString(PyExc_TypeError, "cursor should be a GdkCursor or None");
        return NULL;
    }
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_pointer_grab(GDK_WINDOW(window->obj), owner_events, event_mask, (GdkWindow *) confine_to, cursor, time);
    
    return pyg_enum_from_gtype(GDK_TYPE_GRAB_STATUS, ret);
}

static PyObject *
_wrap_gdk_pointer_ungrab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    unsigned long time = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|k:pointer_ungrab", kwlist, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_pointer_ungrab(time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_keyboard_grab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", "owner_events", "time", NULL };
    PyGObject *window;
    int owner_events = FALSE;
    unsigned long time = GDK_CURRENT_TIME;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|ik:keyboard_grab", kwlist, &PyGdkWindow_Type, &window, &owner_events, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_keyboard_grab(GDK_WINDOW(window->obj), owner_events, time);
    
    return pyg_enum_from_gtype(GDK_TYPE_GRAB_STATUS, ret);
}

static PyObject *
_wrap_gdk_keyboard_ungrab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    unsigned long time = GDK_CURRENT_TIME;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|k:keyboard_ungrab", kwlist, &time))
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_keyboard_ungrab(time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pointer_is_grabbed(PyObject *self)
{
    int ret;

    
    ret = gdk_pointer_is_grabbed();
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_screen_width(PyObject *self)
{
    int ret;

    
    ret = gdk_screen_width();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_height(PyObject *self)
{
    int ret;

    
    ret = gdk_screen_height();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_width_mm(PyObject *self)
{
    int ret;

    
    ret = gdk_screen_width_mm();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_screen_height_mm(PyObject *self)
{
    int ret;

    
    ret = gdk_screen_height_mm();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_flush(PyObject *self)
{
    
    gdk_flush();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_beep(PyObject *self)
{
    
    gdk_beep();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_set_double_click_time(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "msec", NULL };
    PyObject *py_msec = NULL;
    guint msec = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:set_double_click_time", kwlist, &py_msec))
        return NULL;
    if (py_msec) {
        if (PyLong_Check(py_msec))
            msec = PyLong_AsUnsignedLong(py_msec);
        else if (PyInt_Check(py_msec))
            msec = PyInt_AsLong(py_msec);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'msec' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    gdk_set_double_click_time(msec);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 132 "./gdk.override"
static PyObject *
_wrap_gdk_threads_enter(PyObject *self)
{
    /* must allow threads while acquiring lock, or no other python
     * code will execute while we wait! */
    pyg_begin_allow_threads;
    gdk_threads_enter();
    pyg_end_allow_threads;

    Py_INCREF(Py_None);
    return Py_None;
}
#line 16621 "gdk.c"


static PyObject *
_wrap_gdk_threads_leave(PyObject *self)
{
    
    gdk_threads_leave();
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 121 "./gdk.override"
static PyObject *
_wrap_gdk_threads_init(PyObject *self)
{
    if (pyg_enable_threads())
        return NULL;
    gdk_threads_init();
    Py_INCREF(Py_None);
    return Py_None;
}
#line 16644 "gdk.c"


static PyObject *
_wrap_gdk_exit(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "error_code", NULL };
    int error_code;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:exit", kwlist, &error_code))
        return NULL;
    
    gdk_exit(error_code);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_set_locale(PyObject *self)
{
    gchar *ret;

    
    ret = gdk_set_locale();
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_error_trap_push(PyObject *self)
{
    
    gdk_error_trap_push();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_error_trap_pop(PyObject *self)
{
    int ret;

    
    ret = gdk_error_trap_pop();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_set_use_xshm(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "use_xshm", NULL };
    int use_xshm;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:set_use_xshm", kwlist, &use_xshm))
        return NULL;
    
    gdk_set_use_xshm(use_xshm);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_use_xshm(PyObject *self)
{
    int ret;

    
    ret = gdk_get_use_xshm();
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_input_remove(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    int tag;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:input_remove", kwlist, &tag))
        return NULL;
    
    gdk_input_remove(tag);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_event_send_client_message_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "event", "winid", NULL };
    PyGObject *display;
    GdkEvent *event = NULL;
    int ret;
    unsigned long winid;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Ok:event_send_client_message_for_display", kwlist, &PyGdkDisplay_Type, &display, &py_event, &winid))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    
    ret = gdk_event_send_client_message_for_display(GDK_DISPLAY(display->obj), event, winid);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_colors_store(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", "colors", "ncolors", NULL };
    PyGObject *colormap;
    PyObject *py_colors;
    int ncolors;
    GdkColor *colors = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Oi:colors_store", kwlist, &PyGdkColormap_Type, &colormap, &py_colors, &ncolors))
        return NULL;
    if (pyg_boxed_check(py_colors, GDK_TYPE_COLOR))
        colors = pyg_boxed_get(py_colors, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "colors should be a GdkColor");
        return NULL;
    }
    
    gdk_colors_store(GDK_COLORMAP(colormap->obj), colors, ncolors);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_colormap_get_system(PyObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_colormap_get_system();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_colormap_get_system_size(PyObject *self)
{
    int ret;

    if (PyErr_Warn(PyExc_DeprecationWarning, "deprecated") < 0)
        return NULL;
    
    ret = gdk_colormap_get_system_size();
    
    return PyInt_FromLong(ret);
}

#line 338 "gdkcolor.override"
static PyObject *
_wrap_gdk_color_parse(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "spec", NULL };
    const char *spec;
    GdkColor colour = { 0, };
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:color_parse", kwlist,
                                     &spec))
        return NULL;

    if (!gdk_color_parse(spec, &colour)) {
        PyErr_SetString(PyExc_ValueError,
                        "unable to parse colour specification");
        return NULL;
    }
    
    /* pyg_boxed_new handles NULL checking */    
    return pyg_boxed_new (GDK_TYPE_COLOR, &colour, TRUE, TRUE);
}
#line 16837 "gdk.c"


static PyObject *
_wrap_gdk_color_change(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", "color", NULL };
    PyGObject *colormap;
    PyObject *py_color;
    int ret;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:color_change", kwlist, &PyGdkColormap_Type, &colormap, &py_color))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "deprecated") < 0)
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    ret = gdk_color_change(GDK_COLORMAP(colormap->obj), color);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_cursor_new_from_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "name", NULL };
    PyGObject *display;
    char *name;
    GdkCursor *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:cursor_new_from_name", kwlist, &PyGdkDisplay_Type, &display, &name))
        return NULL;
    
    ret = gdk_cursor_new_from_name(GDK_DISPLAY(display->obj), name);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_CURSOR, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_display_get_default(PyObject *self)
{
    GdkDisplay *ret;

    
    ret = gdk_display_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_display_manager_get(PyObject *self)
{
    GdkDisplayManager *ret;

    
    ret = gdk_display_manager_get();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_draw_layout_with_colors(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "gc", "x", "y", "layout", "foreground", "background", NULL };
    PyGObject *drawable, *gc, *layout;
    int x, y;
    GdkColor *foreground = NULL, *background = NULL;
    PyObject *py_foreground, *py_background;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!iiO!OO:draw_layout_with_colors", kwlist, &PyGdkDrawable_Type, &drawable, &PyGdkGC_Type, &gc, &x, &y, &PyPangoLayout_Type, &layout, &py_foreground, &py_background))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use gtk.gdk.Drawable.draw_layout instead") < 0)
        return NULL;
    if (pyg_boxed_check(py_foreground, GDK_TYPE_COLOR))
        foreground = pyg_boxed_get(py_foreground, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "foreground should be a GdkColor");
        return NULL;
    }
    if (pyg_boxed_check(py_background, GDK_TYPE_COLOR))
        background = pyg_boxed_get(py_background, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "background should be a GdkColor");
        return NULL;
    }
    
    gdk_draw_layout_with_colors(GDK_DRAWABLE(drawable->obj), GDK_GC(gc->obj), x, y, PANGO_LAYOUT(layout->obj), foreground, background);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_draw_glyphs_transformed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "gc", "matrix", "font", "x", "y", "glyphs", NULL };
    PyGObject *drawable, *gc, *font;
    PyObject *py_matrix, *py_glyphs;
    int x, y;
    PangoGlyphString *glyphs = NULL;
    PangoMatrix *matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!OO!iiO:draw_glyphs_transformed", kwlist, &PyGdkDrawable_Type, &drawable, &PyGdkGC_Type, &gc, &py_matrix, &PyPangoFont_Type, &font, &x, &y, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_matrix, PANGO_TYPE_MATRIX))
        matrix = pyg_boxed_get(py_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "matrix should be a PangoMatrix");
        return NULL;
    }
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    
    gdk_draw_glyphs_transformed(GDK_DRAWABLE(drawable->obj), GDK_GC(gc->obj), matrix, PANGO_FONT(font->obj), x, y, glyphs);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_events_pending(PyObject *self)
{
    int ret;

    
    ret = gdk_events_pending();
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_event_get(PyObject *self)
{
    GdkEvent *ret;

    
    ret = gdk_event_get();
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_event_peek(PyObject *self)
{
    GdkEvent *ret;

    
    ret = gdk_event_peek();
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_event_get_graphics_expose(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", NULL };
    PyGObject *window;
    GdkEvent *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:event_get_graphics_expose", kwlist, &PyGdkWindow_Type, &window))
        return NULL;
    
    ret = gdk_event_get_graphics_expose(GDK_WINDOW(window->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_EVENT, ret, TRUE, TRUE);
}

#line 1390 "./gdk.override"
static void
pygdk_event_handler_marshal(GdkEvent *event, gpointer data)
{
    PyGILState_STATE state;
    PyGtkCustomNotify *cunote = data;
    PyObject *retobj;
    PyObject *pyevent;

    g_assert (cunote->func);

    state = pyg_gil_state_ensure();

    pyevent = pyg_boxed_new(GDK_TYPE_EVENT, event, TRUE, TRUE);
    if (cunote->data)
        retobj = PyEval_CallFunction(cunote->func, "(NO)",
				     pyevent, cunote->data);
    else
        retobj = PyEval_CallFunction(cunote->func, "(N)", pyevent);

    if (retobj == NULL) {
        PyErr_Print();
    } else
        Py_DECREF(retobj);

    pyg_gil_state_release(state);
}

static PyObject *
_wrap_gdk_event_handler_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *pyfunc, *pyarg = NULL;
    PyGtkCustomNotify *cunote;

    if (!PyArg_ParseTuple(args, "O|O:event_handler_set",
                          &pyfunc, &pyarg))
        return NULL;

    if (pyfunc == Py_None) {
	gdk_event_handler_set(NULL, NULL, NULL);
    } else {
	cunote = g_new0(PyGtkCustomNotify, 1);
	cunote->func = pyfunc;
	cunote->data = pyarg;
	Py_INCREF(cunote->func);
	Py_XINCREF(cunote->data);

	gdk_event_handler_set(pygdk_event_handler_marshal,
			      cunote,
			      pygtk_custom_destroy_notify);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#line 17077 "gdk.c"


static PyObject *
_wrap_gdk_set_show_events(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "show_events", NULL };
    int show_events;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:set_show_events", kwlist, &show_events))
        return NULL;
    
    gdk_set_show_events(show_events);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_show_events(PyObject *self)
{
    int ret;

    
    ret = gdk_get_show_events();
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_font_load_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "font_name", NULL };
    PyGObject *display;
    char *font_name;
    GdkFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:font_load_for_display", kwlist, &PyGdkDisplay_Type, &display, &font_name))
        return NULL;
    
    ret = gdk_font_load_for_display(GDK_DISPLAY(display->obj), font_name);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_FONT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_fontset_load_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "fontset_name", NULL };
    PyGObject *display;
    char *fontset_name;
    GdkFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:fontset_load_for_display", kwlist, &PyGdkDisplay_Type, &display, &fontset_name))
        return NULL;
    
    ret = gdk_fontset_load_for_display(GDK_DISPLAY(display->obj), fontset_name);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_FONT, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_font_from_description_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "font_desc", NULL };
    PyGObject *display;
    PyObject *py_font_desc;
    PangoFontDescription *font_desc = NULL;
    GdkFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:font_from_description_for_display", kwlist, &PyGdkDisplay_Type, &display, &py_font_desc))
        return NULL;
    if (pyg_boxed_check(py_font_desc, PANGO_TYPE_FONT_DESCRIPTION))
        font_desc = pyg_boxed_get(py_font_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "font_desc should be a PangoFontDescription");
        return NULL;
    }
    
    ret = gdk_font_from_description_for_display(GDK_DISPLAY(display->obj), font_desc);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_FONT, ret, TRUE, TRUE);
}

#line 146 "./gdk.override"
static PyObject *
_wrap_gdk_fontset_load(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fontset_name", NULL };
    char *fontset_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:fontset_load", kwlist,
                                     &fontset_name))
        return NULL;

    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_FONT, gdk_fontset_load(fontset_name),
                         FALSE, TRUE);
}
#line 17180 "gdk.c"


static PyObject *
_wrap_gdk_font_from_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font_desc", NULL };
    PyObject *py_font_desc;
    GdkFont *ret;
    PangoFontDescription *font_desc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:font_from_description", kwlist, &py_font_desc))
        return NULL;
    if (pyg_boxed_check(py_font_desc, PANGO_TYPE_FONT_DESCRIPTION))
        font_desc = pyg_boxed_get(py_font_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "font_desc should be a PangoFontDescription");
        return NULL;
    }
    
    ret = gdk_font_from_description(font_desc);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GDK_TYPE_FONT, ret, TRUE, TRUE);
}

#line 285 "gdkgc.override"
static PyObject *
_wrap_gdk_gc_new2(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", NULL };
    PyGObject *drawable;
    GdkGC *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:gc_new", kwlist, &PyGdkDrawable_Type, &drawable))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use gdk.GC or drawable.new_gc") < 0)
        return NULL;
    ret = gdk_gc_new(GDK_DRAWABLE(drawable->obj));
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}
#line 17222 "gdk.c"


#line 858 "./gdk.override"
static PyObject *
_wrap_gdk_devices_list(PyObject *self)
{
    GList * devlist,  *tmp;
    PyObject *list;

    devlist = gdk_devices_list();
    list = PyList_New(0);
    for (tmp = devlist; tmp != NULL; tmp = tmp->next) {
        PyObject *item = pygobject_new((GObject *)tmp->data);
        PyList_Append(list, item);
        Py_DECREF(item);
    }

    return list;
}
#line 17242 "gdk.c"


static PyObject *
_wrap_gdk_device_get_core_pointer(PyObject *self)
{
    GdkDevice *ret;

    
    ret = gdk_device_get_core_pointer();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_keymap_get_default(PyObject *self)
{
    GdkKeymap *ret;

    
    ret = gdk_keymap_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_keymap_get_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", NULL };
    PyGObject *display;
    GdkKeymap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:keymap_get_for_display", kwlist, &PyGdkDisplay_Type, &display))
        return NULL;
    
    ret = gdk_keymap_get_for_display(GDK_DISPLAY(display->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_keyval_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    const gchar *ret;
    guint keyval = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_name", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_name(keyval);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_keyval_from_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval_name", NULL };
    char *keyval_name;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:keyval_from_name", kwlist, &keyval_name))
        return NULL;
    
    ret = gdk_keyval_from_name(keyval_name);
    
    return PyLong_FromUnsignedLong(ret);
}

#line 1154 "./gdk.override"
static PyObject *
_wrap_gdk_keyval_convert_case(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "symbol", NULL };
    guint symbol, upper, lower;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:keyval_convert_case",
				     kwlist, &symbol))
	return NULL;

    gdk_keyval_convert_case(symbol, &lower, &upper);

    return Py_BuildValue("(ii)", lower, upper);
}
#line 17344 "gdk.c"


static PyObject *
_wrap_gdk_keyval_to_upper(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    guint keyval = 0, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_to_upper", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_to_upper(keyval);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gdk_keyval_to_lower(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    guint keyval = 0, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_to_lower", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_to_lower(keyval);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gdk_keyval_is_upper(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    int ret;
    guint keyval = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_is_upper", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_is_upper(keyval);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_keyval_is_lower(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    int ret;
    guint keyval = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_is_lower", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_is_lower(keyval);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_keyval_to_unicode(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keyval", NULL };
    PyObject *py_keyval = NULL;
    guint32 ret;
    guint keyval = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:keyval_to_unicode", kwlist, &py_keyval))
        return NULL;
    if (py_keyval) {
        if (PyLong_Check(py_keyval))
            keyval = PyLong_AsUnsignedLong(py_keyval);
        else if (PyInt_Check(py_keyval))
            keyval = PyInt_AsLong(py_keyval);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'keyval' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = gdk_keyval_to_unicode(keyval);
    
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_unicode_to_keyval(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "wc", NULL };
    unsigned long wc;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:unicode_to_keyval", kwlist, &wc))
        return NULL;
    if (wc > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " wc parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_unicode_to_keyval(wc);
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gdk_pango_renderer_get_default(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", NULL };
    PyGObject *screen;
    PangoRenderer *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:pango_renderer_get_default", kwlist, &PyGdkScreen_Type, &screen))
        return NULL;
    
    ret = gdk_pango_renderer_get_default(GDK_SCREEN(screen->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pango_context_get_for_screen(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", NULL };
    PyGObject *screen;
    PangoContext *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:pango_context_get_for_screen", kwlist, &PyGdkScreen_Type, &screen))
        return NULL;
    
    ret = gdk_pango_context_get_for_screen(GDK_SCREEN(screen->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pango_context_get(PyObject *self)
{
    PangoContext *ret;

    
    ret = gdk_pango_context_get();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pango_context_set_colormap(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", "colormap", NULL };
    PyGObject *context, *colormap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:pango_context_set_colormap", kwlist, &PyPangoContext_Type, &context, &PyGdkColormap_Type, &colormap))
        return NULL;
    
    gdk_pango_context_set_colormap(PANGO_CONTEXT(context->obj), GDK_COLORMAP(colormap->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 729 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_get_from_drawable2(PyObject *self,
				    PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "pixbuf", "src", "cmap",
			      "src_x", "src_y",
			      "dest_x", "dest_y",
			      "width", "height", NULL };
    PyGObject *py_pixbuf, *src, *cmap;
    int src_x, src_y, dest_x, dest_y, width, height;
    GdkPixbuf *pixbuf = NULL, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "OO!O!iiiiii:pixbuf_get_from_drawable",
				     kwlist,
				     &py_pixbuf,
				     &PyGdkDrawable_Type, &src,
				     &PyGdkColormap_Type, &cmap,
				     &src_x, &src_y,
				     &dest_x, &dest_y,
				     &width, &height))
        return NULL;
    
    if (py_pixbuf && pygobject_check(py_pixbuf, &PyGdkPixbuf_Type))
        pixbuf = GDK_PIXBUF(py_pixbuf->obj);
    else if ((PyObject *)py_pixbuf != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"pixbuf should be a GdkPixbuf or None");
        return NULL;
    }
    
    ret = gdk_pixbuf_get_from_drawable(pixbuf,
				       GDK_DRAWABLE(src->obj),
				       GDK_COLORMAP(cmap->obj),
				       src_x, src_y,
				       dest_x, dest_y,
				       width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}
#line 17601 "gdk.c"


#line 1447 "./gdk.override"
static PyObject *
_wrap_gdk_bitmap_create_from_data(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "data", "width", "height", NULL };
    PyGObject *py_drawable;
    GdkDrawable *drawable = NULL;
    gchar *data;
    GdkBitmap *ret;
    Py_ssize_t data_len;
    int width, height, rowstride;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os#ii:bitmap_create_from_data", kwlist, &py_drawable, &data, &data_len, &width, &height))
        return NULL;
    if (py_drawable && pygobject_check(py_drawable, &PyGdkDrawable_Type))
        drawable = GDK_DRAWABLE(py_drawable->obj);
    else if ((PyObject *)py_drawable != Py_None) {
        PyErr_SetString(PyExc_TypeError, "drawable should be a GdkDrawable or None");
        return NULL;
    }

    rowstride = (width&(~7))/8 + 1;
    if (data_len*8 < rowstride*height) {
        PyErr_SetString(PyExc_ValueError, "data size is insufficient for the given width, height, and depth");
        return NULL;
    }
    
    ret = gdk_bitmap_create_from_data((GdkDrawable *) drawable, data, width, height);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 17637 "gdk.c"


#line 1481 "./gdk.override"
static PyObject *
_wrap_gdk_pixmap_create_from_data(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "drawable", "data", "width", "height", "depth", "fg", "bg", NULL };
    Py_ssize_t data_len;
    int width, height, depth;
    GdkPixmap *ret;
    PyObject *py_fg, *py_bg;
    GdkColor *fg = NULL, *bg = NULL;
    GdkDrawable *drawable = NULL;
    gchar *data;
    PyGObject *py_drawable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os#iiiOO:pixmap_create_from_data", kwlist, &py_drawable, &data, &data_len, &width, &height, &depth, &py_fg, &py_bg))
        return NULL;
    if (py_drawable && pygobject_check(py_drawable, &PyGdkDrawable_Type))
        drawable = GDK_DRAWABLE(py_drawable->obj);
    else if ((PyObject *)py_drawable != Py_None) {
        PyErr_SetString(PyExc_TypeError, "drawable should be a GdkDrawable or None");
        return NULL;
    }
    if (pyg_boxed_check(py_fg, GDK_TYPE_COLOR))
        fg = pyg_boxed_get(py_fg, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "fg should be a GdkColor");
        return NULL;
    }
    if (pyg_boxed_check(py_bg, GDK_TYPE_COLOR))
        bg = pyg_boxed_get(py_bg, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "bg should be a GdkColor");
        return NULL;
    }

    if (data_len < width*height*(depth>>3)) {
        PyErr_SetString(PyExc_ValueError, "data size is insufficient for the given width, height, and depth");
        return NULL;
    }
    
    ret = gdk_pixmap_create_from_data((GdkDrawable *) drawable, data, width, height, depth, fg, bg);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}
#line 17685 "gdk.c"


#line 180 "./gdk.override"
static PyObject *
_wrap_gdk_pixmap_create_from_xpm(PyObject *self, PyObject *args,
                                 PyObject *kwargs)
{
    static char *kwlist[] = { "window", "transparent_color", "filename", NULL};
    PyGObject *window;
    PyObject *py_trans_color, *ret;
    GdkColor *trans_color = NULL;
    gchar *filename;
    GdkPixmap *pixmap;
    GdkBitmap *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!Os:pixmap_create_from_xpm", kwlist,
                                     &PyGdkDrawable_Type, &window,
                                     &py_trans_color, &filename))
        return NULL;
    if (pyg_boxed_check(py_trans_color, GDK_TYPE_COLOR))
        trans_color = pyg_boxed_get(py_trans_color, GdkColor);
    else if (py_trans_color != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "transparent_color must be a colour or None");
        return NULL;
    }
    pixmap = gdk_pixmap_create_from_xpm(GDK_DRAWABLE(window->obj), &mask,
                                        trans_color, filename);
    if (pixmap == NULL) {
        PyErr_SetString(PyExc_IOError, "can't load pixmap");
        return NULL;
    }
    ret = Py_BuildValue("(NN)",
                        pygobject_new((GObject *)pixmap),
                        pygobject_new((GObject *)mask));
    g_object_unref(pixmap);
    g_object_unref(mask);
    return ret;
}
#line 17726 "gdk.c"


#line 219 "./gdk.override"
static PyObject *
_wrap_gdk_pixmap_colormap_create_from_xpm(PyObject *self, PyObject *args,
                                          PyObject *kwargs)
{
    static char *kwlist[] = { "window", "colormap", "transparent_color",
                              "filename", NULL };
    PyObject *py_window, *py_colormap, *py_trans_color, *ret;
    GdkDrawable *window = NULL;
    GdkColormap *colormap = NULL;
    GdkColor *trans_color = NULL;
    gchar *filename;
    GdkPixmap *pixmap;
    GdkBitmap *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OOOs:pixmap_colormap_create_from_xpm",
                                     kwlist, &py_window, &py_colormap,
                                     &py_trans_color, &filename))
        return NULL;
    if (pygobject_check(py_window, &PyGdkDrawable_Type))
        window = GDK_DRAWABLE(pygobject_get(py_window));
    else if (py_window != Py_None) {
        PyErr_SetString(PyExc_TypeError, "window must be a GdkDrawable or None");
        return NULL;
    }
    if (pygobject_check(py_colormap, &PyGdkColormap_Type))
        colormap = GDK_COLORMAP(pygobject_get(py_colormap));
    else if (py_colormap != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "colormap must be a GdkColormap or None");
        return NULL;
    }
    if (pyg_boxed_check(py_trans_color, GDK_TYPE_COLOR))
        trans_color = pyg_boxed_get(py_trans_color, GdkColor);
    else if (py_trans_color != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "transparent_color must be a colour or None");
        return NULL;
    }
    pixmap = gdk_pixmap_colormap_create_from_xpm(window, colormap, &mask,
                                                 trans_color, filename);
    if (pixmap == NULL) {
        PyErr_SetString(PyExc_IOError, "can't load pixmap");
        return NULL;
    }
    ret = Py_BuildValue("(NN)",
                        pygobject_new((GObject *)pixmap),
                        pygobject_new((GObject *)mask));
    g_object_unref(pixmap);
    g_object_unref(mask);
    return ret;
}
#line 17782 "gdk.c"


#line 273 "./gdk.override"
static PyObject *
_wrap_gdk_pixmap_create_from_xpm_d(PyObject *self, PyObject *args,
                                   PyObject *kwargs)
{
    static char *kwlist[] = { "window", "transparent_color", "data", NULL };
    PyGObject *window;
    PyObject *py_trans_color, *py_data, *ret;
    GdkColor *trans_color = NULL;
    gchar **data;
    int len, i;
    GdkPixmap *pixmap;
    GdkBitmap *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!OO!:pixmap_create_from_xpm_d", kwlist,
                                     &PyGdkDrawable_Type, &window,
                                     &py_trans_color, &PyList_Type, &py_data))
        return NULL;
    if (pyg_boxed_check(py_trans_color, GDK_TYPE_COLOR))
        trans_color = pyg_boxed_get(py_trans_color, GdkColor);
    else if (py_trans_color != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "transparent_color must be a colour or None");
        return NULL;
    }
    len = PyList_Size(py_data);
    data = g_new(gchar *, len);
    for (i = 0; i < len; i ++) {
        PyObject *item = PyList_GetItem(py_data, i);
        if (!PyString_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "data items must be strings");
            g_free(data);
            return NULL;
        }
        data[i] = PyString_AsString(item);
    }
    pixmap = gdk_pixmap_create_from_xpm_d(GDK_DRAWABLE(window->obj), &mask,
                                          trans_color, data);
    g_free(data);
    if (pixmap == NULL) {
        PyErr_SetString(PyExc_IOError, "can't load pixmap");
        return NULL;
    }
    ret = Py_BuildValue("(NN)",
                        pygobject_new((GObject *)pixmap),
                        pygobject_new((GObject *)mask));
    g_object_unref(pixmap);
    g_object_unref(mask);
    return ret;
}
#line 17836 "gdk.c"


#line 325 "./gdk.override"
static PyObject *
_wrap_gdk_pixmap_colormap_create_from_xpm_d(PyObject *self, PyObject *args,
                                            PyObject *kwargs)
{
    static char *kwlist[] = { "window", "colormap", "transparent_color",
                              "data", NULL };
    PyObject *py_window, *py_colormap, *py_trans_color, *py_data, *ret;
    GdkDrawable *window = NULL;
    GdkColormap *colormap = NULL;
    GdkColor *trans_color = NULL;
    gchar **data;
    int len, i;
    GdkPixmap *pixmap;
    GdkBitmap *mask;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OOOO!:pixmap_colormap_create_from_xpm_d",
                                     kwlist, &py_window, &py_colormap,
                                     &py_trans_color,
                                     &PyList_Type, &py_data))
        return NULL;
    if (pygobject_check(py_window, &PyGdkDrawable_Type))
        window = GDK_DRAWABLE(pygobject_get(py_window));
    else if (py_window != Py_None) {
        PyErr_SetString(PyExc_TypeError, "window must be a GdkDrawable or None");
        return NULL;
    }
    if (pygobject_check(py_colormap, &PyGdkColormap_Type))
        colormap = GDK_COLORMAP(pygobject_get(py_colormap));
    else if (py_colormap != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "colormap must be a GdkColormap or None");
        return NULL;
    }
    if (pyg_boxed_check(py_trans_color, GDK_TYPE_COLOR))
        trans_color = pyg_boxed_get(py_trans_color, GdkColor);
    else if (py_trans_color != Py_None) {
        PyErr_SetString(PyExc_TypeError,
                        "transparent_color must be a colour or None");
        return NULL;
    }
    len = PyList_Size(py_data);
    data = g_new(gchar *, len);
    for (i = 0; i < len; i ++) {
        PyObject *item = PyList_GetItem(py_data, i);
        if (!PyString_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "data items must be strings");
            g_free(data);
            return NULL;
        }
        data[i] = PyString_AsString(item);
    }
    pixmap = gdk_pixmap_colormap_create_from_xpm_d(window, colormap, &mask,
                                                   trans_color, data);
    g_free(data);
    if (pixmap == NULL) {
        PyErr_SetString(PyExc_IOError, "can't load pixmap");
        return NULL;
    }
    ret = Py_BuildValue("(NN)",
                        pygobject_new((GObject *)pixmap),
                        pygobject_new((GObject *)mask));
    g_object_unref(pixmap);
    g_object_unref(mask);
    return ret;
}
#line 17906 "gdk.c"


static PyObject *
_wrap_gdk_pixmap_foreign_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "anid", NULL };
    PyObject *py_ret;
    unsigned long anid;
    GdkPixmap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:pixmap_foreign_new", kwlist, &anid))
        return NULL;
    
    ret = gdk_pixmap_foreign_new(anid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixmap_lookup(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "anid", NULL };
    unsigned long anid;
    GdkPixmap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:pixmap_lookup", kwlist, &anid))
        return NULL;
    
    ret = gdk_pixmap_lookup(anid);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixmap_foreign_new_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "anid", NULL };
    PyGObject *display;
    PyObject *py_ret;
    unsigned long anid;
    GdkPixmap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:pixmap_foreign_new_for_display", kwlist, &PyGdkDisplay_Type, &display, &anid))
        return NULL;
    
    ret = gdk_pixmap_foreign_new_for_display(GDK_DISPLAY(display->obj), anid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixmap_lookup_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "anid", NULL };
    PyGObject *display;
    unsigned long anid;
    GdkPixmap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:pixmap_lookup_for_display", kwlist, &PyGdkDisplay_Type, &display, &anid))
        return NULL;
    
    ret = gdk_pixmap_lookup_for_display(GDK_DISPLAY(display->obj), anid);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_synthesize_window_state(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "window", "unset_flags", "set_flags", NULL };
    PyGObject *window;
    PyObject *py_unset_flags = NULL, *py_set_flags = NULL;
    GdkWindowState unset_flags, set_flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:synthesize_window_state", kwlist, &PyGdkWindow_Type, &window, &py_unset_flags, &py_set_flags))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_WINDOW_STATE, py_unset_flags, (gpointer)&unset_flags))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_WINDOW_STATE, py_set_flags, (gpointer)&set_flags))
        return NULL;
    
    gdk_synthesize_window_state(GDK_WINDOW(window->obj), unset_flags, set_flags);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_utf8_to_string_target(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "str", NULL };
    char *str;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:utf8_to_string_target", kwlist, &str))
        return NULL;
    
    ret = gdk_utf8_to_string_target(str);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_free_compound_text(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ctext", NULL };
    guchar *ctext;
    Py_ssize_t ctext_len;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s#:free_compound_text", kwlist, &ctext, &ctext_len))
        return NULL;
    
    gdk_free_compound_text(ctext);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1122 "./gdk.override"
static PyObject *
_wrap_gdk_atom_intern(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "atom_name", "only_if_exists", NULL };
    char *atom_name;
    int only_if_exists = FALSE;
    GdkAtom ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|i:atom_intern",
				     kwlist, &atom_name, &only_if_exists))
        return NULL;
    ret = gdk_atom_intern(atom_name, only_if_exists);
    if (ret == GDK_NONE) {
	Py_INCREF(Py_None);
	return Py_None;
    }
    return PyGdkAtom_New(ret);
}
#line 18058 "gdk.c"


static PyObject *
_wrap_gdk_rgb_xpixel_from_rgb(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rgb", NULL };
    gulong ret;
    unsigned long rgb;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:rgb_xpixel_from_rgb", kwlist, &rgb))
        return NULL;
    if (rgb > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " rgb parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_rgb_xpixel_from_rgb(rgb);
    
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gdk_rgb_gc_set_foreground(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "rgb", NULL };
    PyGObject *gc;
    unsigned long rgb;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:rgb_gc_set_foreground", kwlist, &PyGdkGC_Type, &gc, &rgb))
        return NULL;
    if (rgb > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " rgb parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_rgb_gc_set_foreground(GDK_GC(gc->obj), rgb);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_gc_set_background(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gc", "rgb", NULL };
    PyGObject *gc;
    unsigned long rgb;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:rgb_gc_set_background", kwlist, &PyGdkGC_Type, &gc, &rgb))
        return NULL;
    if (rgb > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " rgb parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_rgb_gc_set_background(GDK_GC(gc->obj), rgb);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_ditherable(PyObject *self)
{
    int ret;

    
    ret = gdk_rgb_ditherable();
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_rgb_set_verbose(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "verbose", NULL };
    int verbose;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:rgb_set_verbose", kwlist, &verbose))
        return NULL;
    
    gdk_rgb_set_verbose(verbose);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_set_install(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "install", NULL };
    int install;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:rgb_set_install", kwlist, &install))
        return NULL;
    
    gdk_rgb_set_install(install);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_set_min_colors(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "min_colors", NULL };
    int min_colors;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:rgb_set_min_colors", kwlist, &min_colors))
        return NULL;
    
    gdk_rgb_set_min_colors(min_colors);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_get_colormap(PyObject *self)
{
    GdkColormap *ret;

    
    ret = gdk_rgb_get_colormap();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_rgb_get_cmap(PyObject *self)
{
    GdkColormap *ret;

    if (PyErr_Warn(PyExc_DeprecationWarning, "use gtk.gdk.rgb_get_colormap instead") < 0)
        return NULL;
    
    ret = gdk_rgb_get_cmap();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_rgb_get_visual(PyObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_rgb_get_visual();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_rgb_init(PyObject *self)
{
    
    gdk_rgb_init();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_find_color(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colormap", "color", NULL };
    PyGObject *colormap;
    PyObject *py_color;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:rgb_find_color", kwlist, &PyGdkColormap_Type, &colormap, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    
    gdk_rgb_find_color(GDK_COLORMAP(colormap->obj), color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_rgb_colormap_ditherable(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cmap", NULL };
    PyGObject *cmap;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:rgb_colormap_ditherable", kwlist, &PyGdkColormap_Type, &cmap))
        return NULL;
    
    ret = gdk_rgb_colormap_ditherable(GDK_COLORMAP(cmap->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_screen_get_default(PyObject *self)
{
    GdkScreen *ret;

    
    ret = gdk_screen_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_selection_owner_set(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "owner", "selection", "time", "send_event", NULL };
    PyGObject *owner;
    PyObject *py_selection = NULL;
    int send_event, ret;
    GdkAtom selection;
    unsigned long time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Oki:selection_owner_set", kwlist, &PyGdkWindow_Type, &owner, &py_selection, &time, &send_event))
        return NULL;
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_selection_owner_set(GDK_WINDOW(owner->obj), selection, time, send_event);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_selection_owner_get(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "selection", NULL };
    PyObject *py_selection = NULL;
    GdkAtom selection;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:selection_owner_get", kwlist, &py_selection))
        return NULL;
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    
    ret = gdk_selection_owner_get(selection);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_selection_owner_set_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "owner", "selection", "time", "send_event", NULL };
    PyObject *py_selection = NULL;
    GdkAtom selection;
    int send_event, ret;
    unsigned long time;
    GdkWindow *owner = NULL;
    PyGObject *display, *py_owner;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OOki:selection_owner_set_for_display", kwlist, &PyGdkDisplay_Type, &display, &py_owner, &py_selection, &time, &send_event))
        return NULL;
    if (py_owner && pygobject_check(py_owner, &PyGdkWindow_Type))
        owner = GDK_WINDOW(py_owner->obj);
    else if ((PyObject *)py_owner != Py_None) {
        PyErr_SetString(PyExc_TypeError, "owner should be a GdkWindow or None");
        return NULL;
    }
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    ret = gdk_selection_owner_set_for_display(GDK_DISPLAY(display->obj), (GdkWindow *) owner, selection, time, send_event);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gdk_selection_owner_get_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "selection", NULL };
    PyGObject *display;
    PyObject *py_selection = NULL;
    GdkAtom selection;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:selection_owner_get_for_display", kwlist, &PyGdkDisplay_Type, &display, &py_selection))
        return NULL;
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    
    ret = gdk_selection_owner_get_for_display(GDK_DISPLAY(display->obj), selection);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_selection_send_notify(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "requestor", "selection", "target", "property", "time", NULL };
    PyObject *py_selection = NULL, *py_target = NULL, *py_property = NULL;
    GdkAtom selection, target, property;
    unsigned long requestor, time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"kOOOk:selection_send_notify", kwlist, &requestor, &py_selection, &py_target, &py_property, &time))
        return NULL;
    if (requestor > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " requestor parameter to unsigned 32 bit integer");
        return NULL;
    }
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    target = pygdk_atom_from_pyobject(py_target);
    if (PyErr_Occurred())
        return NULL;
    property = pygdk_atom_from_pyobject(py_property);
    if (PyErr_Occurred())
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_selection_send_notify(requestor, selection, target, property, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_selection_send_notify_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "requestor", "selection", "target", "property", "time", NULL };
    PyGObject *display;
    GdkAtom selection, target, property;
    unsigned long requestor, time;
    PyObject *py_selection = NULL, *py_target = NULL, *py_property = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!kOOOk:selection_send_notify_for_display", kwlist, &PyGdkDisplay_Type, &display, &requestor, &py_selection, &py_target, &py_property, &time))
        return NULL;
    if (requestor > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " requestor parameter to unsigned 32 bit integer");
        return NULL;
    }
    selection = pygdk_atom_from_pyobject(py_selection);
    if (PyErr_Occurred())
        return NULL;
    target = pygdk_atom_from_pyobject(py_target);
    if (PyErr_Occurred())
        return NULL;
    property = pygdk_atom_from_pyobject(py_property);
    if (PyErr_Occurred())
        return NULL;
    if (time > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " time parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    gdk_selection_send_notify_for_display(GDK_DISPLAY(display->obj), requestor, selection, target, property, time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_visual_get_best_depth(PyObject *self)
{
    int ret;

    
    ret = gdk_visual_get_best_depth();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gdk_visual_get_best_type(PyObject *self)
{
    gint ret;

    
    ret = gdk_visual_get_best_type();
    
    return pyg_enum_from_gtype(GDK_TYPE_VISUAL_TYPE, ret);
}

static PyObject *
_wrap_gdk_visual_get_system(PyObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_visual_get_system();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_visual_get_best(PyObject *self)
{
    GdkVisual *ret;

    
    ret = gdk_visual_get_best();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_visual_get_best_with_depth(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "depth", NULL };
    int depth;
    GdkVisual *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:visual_get_best_with_depth", kwlist, &depth))
        return NULL;
    
    ret = gdk_visual_get_best_with_depth(depth);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_visual_get_best_with_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "visual_type", NULL };
    PyObject *py_visual_type = NULL;
    GdkVisualType visual_type;
    GdkVisual *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:visual_get_best_with_type", kwlist, &py_visual_type))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_VISUAL_TYPE, py_visual_type, (gpointer)&visual_type))
        return NULL;
    
    ret = gdk_visual_get_best_with_type(visual_type);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1289 "./gdk.override"
static PyObject *
_wrap_gdk_query_depths(PyObject *self)
{
    gint count, i;
    gint *depths;
    PyObject *py_depths;

    gdk_query_depths(&depths, &count);

    py_depths = PyTuple_New(count);

    for (i = 0; i < count; i++)
	PyTuple_SetItem(py_depths, i, PyInt_FromLong(depths[i]));

    return py_depths;
}
#line 18562 "gdk.c"


#line 1307 "./gdk.override"
static PyObject *
_wrap_gdk_query_visual_types(PyObject *self)
{
    gint count, i;
    GdkVisualType *types;
    PyObject *py_types;

    gdk_query_visual_types(&types, &count);

    py_types = PyTuple_New(count);

    for (i = 0; i < count; i++)
	PyTuple_SetItem(py_types, i, PyInt_FromLong(types[i]));

    return py_types;
}
#line 18582 "gdk.c"


#line 902 "./gdk.override"
static PyObject *
_wrap_gdk_list_visuals(PyGObject *self)
{
    GList *visl;
    guint nvisl;
    int i;
    PyObject *list;

    visl = gdk_list_visuals();
    nvisl = g_list_length(visl);

    if ((list = PyList_New(nvisl)) == NULL)
	return NULL;

    for (i = 0; i < nvisl; i++) {
	PyObject *item;

	item = pygobject_new((GObject *)g_list_nth_data(visl, i));
	PyList_SetItem(list, i, item);
    }

    g_list_free(visl);

    return list;
}
#line 18611 "gdk.c"


#line 1325 "./gdk.override"
static PyObject *
_wrap_gdk_window_at_pointer(PyObject *self)
{
    GdkWindow *window;
    gint x, y;

    window = gdk_window_at_pointer(&x, &y);

    if (window)
	return Py_BuildValue("(Nii)", pygobject_new((GObject *)window), x, y);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 18629 "gdk.c"


static PyObject *
_wrap_gdk_window_foreign_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "anid", NULL };
    PyObject *py_ret;
    unsigned long anid;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:window_foreign_new", kwlist, &anid))
        return NULL;
    
    ret = gdk_window_foreign_new(anid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_window_lookup(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "anid", NULL };
    unsigned long anid;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:window_lookup", kwlist, &anid))
        return NULL;
    
    ret = gdk_window_lookup(anid);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_window_foreign_new_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "anid", NULL };
    PyGObject *display;
    PyObject *py_ret;
    unsigned long anid;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:window_foreign_new_for_display", kwlist, &PyGdkDisplay_Type, &display, &anid))
        return NULL;
    
    ret = gdk_window_foreign_new_for_display(GDK_DISPLAY(display->obj), anid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_window_lookup_for_display(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display", "anid", NULL };
    PyGObject *display;
    unsigned long anid;
    GdkWindow *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:window_lookup_for_display", kwlist, &PyGdkDisplay_Type, &display, &anid))
        return NULL;
    
    ret = gdk_window_lookup_for_display(GDK_DISPLAY(display->obj), anid);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_set_sm_client_id(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sm_client_id", NULL };
    char *sm_client_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:set_sm_client_id", kwlist, &sm_client_id))
        return NULL;
    
    gdk_set_sm_client_id(sm_client_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 541 "gdkwindow.override"
static PyObject *
_wrap_gdk_window_get_toplevels(PyGObject *self)
{
    GList *topl;
    guint ntopl;
    int i;
    PyObject *list;

    topl = gdk_window_get_toplevels();
    ntopl = g_list_length(topl);

    if ((list = PyList_New(ntopl)) == NULL)
	return NULL;

    for (i = 0; i < ntopl; i++) {
	PyObject *item;

	item = pygobject_new((GObject *)g_list_nth_data(topl, i));
	PyList_SetItem(list, i, item);
    }

    g_list_free(topl);

    return list;
}
#line 18745 "gdk.c"


static PyObject *
_wrap_gdk_window_process_all_updates(PyObject *self)
{
    
    gdk_window_process_all_updates();
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_get_default_root_window(PyObject *self)
{
    GdkWindow *ret;

    
    ret = gdk_get_default_root_window();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_new_from_file(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    PyObject *py_ret;
    GdkPixbuf *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:pixbuf_new_from_file", kwlist, &filename))
        return NULL;
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_new_from_file(filename, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_new_from_file_at_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", "width", "height", NULL };
    char *filename;
    int width, height;
    GdkPixbuf *ret;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sii:pixbuf_new_from_file_at_size", kwlist, &filename, &width, &height))
        return NULL;
    pyg_begin_allow_threads;
    ret = gdk_pixbuf_new_from_file_at_size(filename, width, height, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 301 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_new_from_data(PyObject *self, PyObject *args,
			       PyObject *kwargs)
{
    static char *kwlist[] = { "data", "colorspace", "has_alpha",
			      "bits_per_sample", "width", "height",
			      "rowstride", NULL };
    const gchar *data;
    GdkColorspace colorspace;
    gboolean has_alpha;
    Py_ssize_t data_len;
    gint bits_per_sample, width, height, rowstride;
    Py_ssize_t required_len;
    guchar *data_copy;
    GdkPixbuf *pixbuf;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, 
				     "s#iiiiii:pixbuf_new_from_data", kwlist, 
				     &data, &data_len, &colorspace, 
				     &has_alpha, &bits_per_sample, 
				     &width, &height, &rowstride))
	return NULL;
    required_len = height*rowstride;
    if (data_len < required_len) {
	PyErr_Format(PyExc_ValueError, 
		      "data length (%ld) is less then required by the other parameters (%ld)", 
		      (long int) data_len, (long int) required_len);
	return NULL;
    }
    data_copy = g_try_malloc(required_len);
    if (!data_copy)
	return PyErr_NoMemory();
    memcpy(data_copy, data, required_len);
    pixbuf = gdk_pixbuf_new_from_data(data_copy, colorspace, has_alpha, 
				      bits_per_sample, width, height, 
				      rowstride, 
				      (GdkPixbufDestroyNotify )g_free, NULL);
    ret = pygobject_new((GObject *)pixbuf);
    g_object_unref(pixbuf);
    return ret;
}
#line 18858 "gdk.c"


#line 345 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_new_from_array(PyObject *self, PyObject *args, 
				PyObject *kwargs)
{
#ifdef HAVE_NUMPY
    static char *kwlist[] = { "array", "colorspace", "bits_per_sample", NULL };
    PyArrayObject *array;
    GdkColorspace colorspace;
    gint bits_per_sample ;
    GdkPixbuf *pixbuf;
    PyObject *ret;
    gboolean has_alpha;
    gint width, height;
    gint i, j, nc, rs, st0, st1, st2;
    guchar *src, *dst;

    if (!have_numpy())
	return NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, 
				      "O!ii:pixbuf_new_from_array", kwlist, 
				      &PyArray_Type, &array, &colorspace, 
				      &bits_per_sample))
	return NULL;
    if (array->descr->type_num != PyArray_CHAR
	&& array->descr->type_num != PyArray_UBYTE) {
	PyErr_SetString(PyExc_TypeError, "array type must be 'c' or 'b'");
	return NULL;
    }
    if (array->nd != 3 || array->dimensions[2] < 3 || array->dimensions[2] > 4) {
	PyErr_SetString(PyExc_TypeError, "array shape must be (n,m,3) or (n,m,4)");
	return NULL;
    }
    height = array->dimensions[0];
    width = array->dimensions[1];
    has_alpha = array->dimensions[2] == 4;
    pixbuf = gdk_pixbuf_new(colorspace, has_alpha, bits_per_sample, 
			    width, height);
    src = (guchar*) array->data;
    dst = gdk_pixbuf_get_pixels(pixbuf);
    nc = gdk_pixbuf_get_n_channels(pixbuf);
    rs = gdk_pixbuf_get_rowstride(pixbuf);
    st0 = array->strides[0];
    st1 = array->strides[1];
    st2 = array->strides[2];
    if (has_alpha) {
	for (j = 0; j < height; j++) {
	    for (i = 0; i < width; i++) {
		dst[j*rs + i*4 + 0] = src[j*st0 + i*st1 + 0*st2];
		dst[j*rs + i*4 + 1] = src[j*st0 + i*st1 + 1*st2];
		dst[j*rs + i*4 + 2] = src[j*st0 + i*st1 + 2*st2];
		dst[j*rs + i*4 + 3] = src[j*st0 + i*st1 + 3*st2];
	    }
	}
    } else {
	for (j = 0; j < height; j++) {
	    for (i = 0; i < width; i++) {
		dst[j*rs + i*3 + 0] = src[j*st0 + i*st1 + 0*st2];
		dst[j*rs + i*3 + 1] = src[j*st0 + i*st1 + 1*st2];
		dst[j*rs + i*3 + 2] = src[j*st0 + i*st1 + 2*st2];
	    }
	}
    }
    ret = pygobject_new((GObject *)pixbuf);
    g_object_unref(pixbuf);
    return ret;

#else
    PyErr_SetString(PyExc_RuntimeError, "pygtk was not compiled with Numeric Python support");
    return NULL;
#endif
}
#line 18934 "gdk.c"


#line 419 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_new_from_xpm_data(PyObject *self, PyObject *args,
                                   PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject *py_data, *ret;
    gchar **data;
    int len, i;
    GdkPixbuf *pixbuf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:pixbuf_new_from_xpm_data", kwlist,
                                     &PyList_Type, &py_data))
        return NULL;
    len = PyList_Size(py_data);
    data = g_new(gchar *, len);
    for (i = 0; i < len; i ++) {
        PyObject *item = PyList_GetItem(py_data, i);
        if (!PyString_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "data items must be strings");
            g_free(data);
            return NULL;
        }
        data[i] = PyString_AsString(item);
    }
    pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)data);
    g_free(data);
    if (pixbuf == NULL) {
        PyErr_SetString(PyExc_IOError, "can't load pixbuf");
        return NULL;
    }
    ret = pygobject_new((GObject *)pixbuf);
    g_object_unref(pixbuf);
    return ret;
}
#line 18973 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_new_from_inline(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data_length", "data", "copy_pixels", NULL };
    int data_length, copy_pixels;
    GdkPixbuf *ret;
    GError *error = NULL;
    PyObject *py_ret;
    guchar *data;
    Py_ssize_t data_len;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"is#i:pixbuf_new_from_inline", kwlist, &data_length, &data, &data_len, &copy_pixels))
        return NULL;
    
    ret = gdk_pixbuf_new_from_inline(data_length, data, copy_pixels, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_pixbuf_new_from_file_at_scale(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", "width", "height", "preserve_aspect_ratio", NULL };
    char *filename;
    int width, height, preserve_aspect_ratio;
    GdkPixbuf *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"siii:pixbuf_new_from_file_at_scale", kwlist, &filename, &width, &height, &preserve_aspect_ratio))
        return NULL;
    
    ret = gdk_pixbuf_new_from_file_at_scale(filename, width, height, preserve_aspect_ratio, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 589 "gdkpixbuf.override"
static PyObject *
pygtk_make_pixbuf_format_dict(GdkPixbufFormat *format)
{
    PyObject *item, *value;
    gchar *sval, **aval;
    gint i;

    item = PyDict_New();

    sval = gdk_pixbuf_format_get_name(format);
    value = PyString_FromString(sval);
    PyDict_SetItemString(item, "name", value);
    Py_DECREF(value);
    g_free(sval);

    sval = gdk_pixbuf_format_get_description(format);
    value = PyString_FromString(sval);
    PyDict_SetItemString(item, "description", value);
    Py_DECREF(value);
    g_free(sval);

    aval = gdk_pixbuf_format_get_mime_types(format);
    value = PyList_New(0);
    for (i = 0; aval[i] != NULL; i++) {
	PyObject *string_item = PyString_FromString(aval[i]);
	    
	PyList_Append(value, string_item);
	Py_DECREF(string_item);
    }
    PyDict_SetItemString(item, "mime_types", value);
    Py_DECREF(value);
    g_strfreev(aval);

    aval = gdk_pixbuf_format_get_extensions(format);
    value = PyList_New(0);
    for (i = 0; aval[i] != NULL; i++) {
	PyObject *string_item = PyString_FromString(aval[i]);
	    
	PyList_Append(value, string_item);
	Py_DECREF(string_item);
    }
    PyDict_SetItemString(item, "extensions", value);
    Py_DECREF(value);
    g_strfreev(aval);

    PyDict_SetItemString(item, "is_writable",
			 PyBool_FromLong(gdk_pixbuf_format_is_writable(format)));

    return item;
}
static PyObject *
_wrap_gdk_pixbuf_get_formats(PyObject *self)
{
    GSList *list, *tmp;
    PyObject *ret;

    list = gdk_pixbuf_get_formats();
    ret = PyList_New(0);
    for (tmp = list; tmp != NULL; tmp = tmp->next) {
	PyObject *item;
	item = pygtk_make_pixbuf_format_dict((GdkPixbufFormat *)(tmp->data));

	PyList_Append(ret, item);
	Py_DECREF(item);
    }

    g_slist_free(list);

    return ret;
}
#line 19091 "gdk.c"


#line 684 "gdkpixbuf.override"
static PyObject *
_wrap_gdk_pixbuf_get_file_info(PyObject *self, PyObject *args,
			       PyObject *kwargs)
{
    PyObject *pygtk_make_pixbuf_format_dict(GdkPixbufFormat *format);
    static char *kwlist[] = { "filename", NULL };
    gint width, height;
    GdkPixbufFormat *format;
    gchar *filename;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s:gtk.gdk.pixbuf_get_file_info", kwlist,
				     &filename))
	return NULL;

    format = gdk_pixbuf_get_file_info(filename, &width, &height);

    if (format) {
	PyObject *py_format;

	py_format = pygtk_make_pixbuf_format_dict(format);
	return Py_BuildValue("(Nii)", py_format, width, height);
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 19122 "gdk.c"


static PyObject *
_wrap_gdk_pixbuf_loader_new(PyObject *self)
{
    GdkPixbufLoader *ret;

    if (PyErr_Warn(PyExc_DeprecationWarning, "use gtk.gdk.PixbufLoader instead") < 0)
        return NULL;
    
    ret = gdk_pixbuf_loader_new();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_loader_new_with_mime_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mime_type", NULL };
    char *mime_type;
    PyObject *py_ret;
    GdkPixbufLoader *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:pixbuf_loader_new_with_mime_type", kwlist, &mime_type))
        return NULL;
    
    ret = gdk_pixbuf_loader_new_with_mime_type(mime_type, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_gdk_region_rectangle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rectangle", NULL };
    PyObject *py_rectangle;
    GdkRectangle rectangle = { 0, 0, 0, 0 };
    GdkRegion *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:region_rectangle", kwlist, &py_rectangle))
        return NULL;
    if (!pygdk_rectangle_from_pyobject(py_rectangle, &rectangle))
        return NULL;
    
    ret = gdk_region_rectangle(&rectangle);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PYGDK_TYPE_REGION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gdk_pixmap_foreign_new_for_screen(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "screen", "anid", "width", "height", "depth", NULL };
    PyGObject *screen;
    int width, height, depth;
    unsigned long anid;
    GdkPixmap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!kiii:pixmap_foreign_new_for_screen", kwlist, &PyGdkScreen_Type, &screen, &anid, &width, &height, &depth))
        return NULL;
    
    ret = gdk_pixmap_foreign_new_for_screen(GDK_SCREEN(screen->obj), anid, width, height, depth);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_event_request_motions(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    GdkEvent *event = NULL;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:event_request_motions", kwlist, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    
    gdk_event_request_motions((GdkEventMotion *)event);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_notify_startup_complete_with_id(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "startup_id", NULL };
    char *startup_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:notify_startup_complete_with_id", kwlist, &startup_id))
        return NULL;
    
    gdk_notify_startup_complete_with_id(startup_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gdk_pixbuf_new_from_stream(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stream", "cancellable", NULL };
    PyGObject *stream, *py_cancellable = NULL;
    GdkPixbuf *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:pixbuf_new_from_stream", kwlist, &PyGInputStream_Type, &stream, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(stream->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gdk_pixbuf_new_from_stream_at_scale(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stream", "width", "height", "preserve_aspect_ratio", "cancellable", NULL };
    PyGObject *stream, *py_cancellable = NULL;
    int width = -1, height = -1, preserve_aspect_ratio = 1;
    GdkPixbuf *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|iiiO:pixbuf_new_from_stream_at_scale", kwlist, &PyGInputStream_Type, &stream, &width, &height, &preserve_aspect_ratio, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = gdk_pixbuf_new_from_stream_at_scale(G_INPUT_STREAM(stream->obj), width, height, preserve_aspect_ratio, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 143 "gdkcolor.override"
static PyObject *
_wrap_color_from_hsv (PyObject *ignored, PyObject *args, PyObject*kwargs)
{
    static char *kwlist[] = { "hue", "saturation", "value", NULL };
    gdouble hue, saturation, value;
    gdouble red, green, blue;
    GdkColor color;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ddd:gtk.gdk.color_from_hsv", kwlist,
                                     &hue, &saturation, &value))
        return NULL;

    /* See documentation of the class for rationale. */

    hue -= floor(hue);

    if (saturation > 1.0)
        saturation = 1.0;
    else if (saturation < 0.0)
        saturation = 0.0;

    if (value > 1.0)
        value = 1.0;
    else if (value < 0.0)
        value = 0.0;

    gtk_hsv_to_rgb(hue, saturation, value,
                   &red, &green, &blue);

    color.red = red * 65535.0;
    color.green = green * 65535.0;
    color.blue = blue * 65535.0;

    return pyg_boxed_new(GDK_TYPE_COLOR, &color, TRUE, TRUE);
}

#line 19329 "gdk.c"


const PyMethodDef pygdk_functions[] = {
    { "notify_startup_complete", (PyCFunction)_wrap_gdk_notify_startup_complete, METH_NOARGS,
      NULL },
    { "get_display_arg_name", (PyCFunction)_wrap_gdk_get_display_arg_name, METH_NOARGS,
      NULL },
    { "get_program_class", (PyCFunction)_wrap_gdk_get_program_class, METH_NOARGS,
      NULL },
    { "set_program_class", (PyCFunction)_wrap_gdk_set_program_class, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_display", (PyCFunction)_wrap_gdk_get_display, METH_NOARGS,
      NULL },
    { "pointer_grab", (PyCFunction)_wrap_gdk_pointer_grab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pointer_ungrab", (PyCFunction)_wrap_gdk_pointer_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyboard_grab", (PyCFunction)_wrap_gdk_keyboard_grab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyboard_ungrab", (PyCFunction)_wrap_gdk_keyboard_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pointer_is_grabbed", (PyCFunction)_wrap_gdk_pointer_is_grabbed, METH_NOARGS,
      NULL },
    { "screen_width", (PyCFunction)_wrap_gdk_screen_width, METH_NOARGS,
      NULL },
    { "screen_height", (PyCFunction)_wrap_gdk_screen_height, METH_NOARGS,
      NULL },
    { "screen_width_mm", (PyCFunction)_wrap_gdk_screen_width_mm, METH_NOARGS,
      NULL },
    { "screen_height_mm", (PyCFunction)_wrap_gdk_screen_height_mm, METH_NOARGS,
      NULL },
    { "flush", (PyCFunction)_wrap_gdk_flush, METH_NOARGS,
      NULL },
    { "beep", (PyCFunction)_wrap_gdk_beep, METH_NOARGS,
      NULL },
    { "set_double_click_time", (PyCFunction)_wrap_gdk_set_double_click_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "threads_enter", (PyCFunction)_wrap_gdk_threads_enter, METH_NOARGS,
      NULL },
    { "threads_leave", (PyCFunction)_wrap_gdk_threads_leave, METH_NOARGS,
      NULL },
    { "threads_init", (PyCFunction)_wrap_gdk_threads_init, METH_NOARGS,
      NULL },
    { "exit", (PyCFunction)_wrap_gdk_exit, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_locale", (PyCFunction)_wrap_gdk_set_locale, METH_NOARGS,
      NULL },
    { "error_trap_push", (PyCFunction)_wrap_gdk_error_trap_push, METH_NOARGS,
      NULL },
    { "error_trap_pop", (PyCFunction)_wrap_gdk_error_trap_pop, METH_NOARGS,
      NULL },
    { "set_use_xshm", (PyCFunction)_wrap_gdk_set_use_xshm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_use_xshm", (PyCFunction)_wrap_gdk_get_use_xshm, METH_NOARGS,
      NULL },
    { "input_remove", (PyCFunction)_wrap_gdk_input_remove, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_send_client_message_for_display", (PyCFunction)_wrap_gdk_event_send_client_message_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "colors_store", (PyCFunction)_wrap_gdk_colors_store, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "colormap_get_system", (PyCFunction)_wrap_gdk_colormap_get_system, METH_NOARGS,
      NULL },
    { "colormap_get_system_size", (PyCFunction)_wrap_gdk_colormap_get_system_size, METH_NOARGS,
      NULL },
    { "color_parse", (PyCFunction)_wrap_gdk_color_parse, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "color_change", (PyCFunction)_wrap_gdk_color_change, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "cursor_new_from_name", (PyCFunction)_wrap_gdk_cursor_new_from_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "display_get_default", (PyCFunction)_wrap_gdk_display_get_default, METH_NOARGS,
      NULL },
    { "display_manager_get", (PyCFunction)_wrap_gdk_display_manager_get, METH_NOARGS,
      NULL },
    { "draw_layout_with_colors", (PyCFunction)_wrap_gdk_draw_layout_with_colors, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_glyphs_transformed", (PyCFunction)_wrap_gdk_draw_glyphs_transformed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "events_pending", (PyCFunction)_wrap_gdk_events_pending, METH_NOARGS,
      NULL },
    { "event_get", (PyCFunction)_wrap_gdk_event_get, METH_NOARGS,
      NULL },
    { "event_peek", (PyCFunction)_wrap_gdk_event_peek, METH_NOARGS,
      NULL },
    { "event_get_graphics_expose", (PyCFunction)_wrap_gdk_event_get_graphics_expose, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_handler_set", (PyCFunction)_wrap_gdk_event_handler_set, METH_VARARGS,
      NULL },
    { "set_show_events", (PyCFunction)_wrap_gdk_set_show_events, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_show_events", (PyCFunction)_wrap_gdk_get_show_events, METH_NOARGS,
      NULL },
    { "font_load_for_display", (PyCFunction)_wrap_gdk_font_load_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fontset_load_for_display", (PyCFunction)_wrap_gdk_fontset_load_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "font_from_description_for_display", (PyCFunction)_wrap_gdk_font_from_description_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fontset_load", (PyCFunction)_wrap_gdk_fontset_load, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "font_from_description", (PyCFunction)_wrap_gdk_font_from_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gc_new", (PyCFunction)_wrap_gdk_gc_new2, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "devices_list", (PyCFunction)_wrap_gdk_devices_list, METH_NOARGS,
      NULL },
    { "device_get_core_pointer", (PyCFunction)_wrap_gdk_device_get_core_pointer, METH_NOARGS,
      NULL },
    { "keymap_get_default", (PyCFunction)_wrap_gdk_keymap_get_default, METH_NOARGS,
      NULL },
    { "keymap_get_for_display", (PyCFunction)_wrap_gdk_keymap_get_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_name", (PyCFunction)_wrap_gdk_keyval_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_from_name", (PyCFunction)_wrap_gdk_keyval_from_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_convert_case", (PyCFunction)_wrap_gdk_keyval_convert_case, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_to_upper", (PyCFunction)_wrap_gdk_keyval_to_upper, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_to_lower", (PyCFunction)_wrap_gdk_keyval_to_lower, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_is_upper", (PyCFunction)_wrap_gdk_keyval_is_upper, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_is_lower", (PyCFunction)_wrap_gdk_keyval_is_lower, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyval_to_unicode", (PyCFunction)_wrap_gdk_keyval_to_unicode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unicode_to_keyval", (PyCFunction)_wrap_gdk_unicode_to_keyval, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pango_renderer_get_default", (PyCFunction)_wrap_gdk_pango_renderer_get_default, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pango_context_get_for_screen", (PyCFunction)_wrap_gdk_pango_context_get_for_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pango_context_get", (PyCFunction)_wrap_gdk_pango_context_get, METH_NOARGS,
      NULL },
    { "pango_context_set_colormap", (PyCFunction)_wrap_gdk_pango_context_set_colormap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_get_from_drawable", (PyCFunction)_wrap_gdk_pixbuf_get_from_drawable2, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "bitmap_create_from_data", (PyCFunction)_wrap_gdk_bitmap_create_from_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_create_from_data", (PyCFunction)_wrap_gdk_pixmap_create_from_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_create_from_xpm", (PyCFunction)_wrap_gdk_pixmap_create_from_xpm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_colormap_create_from_xpm", (PyCFunction)_wrap_gdk_pixmap_colormap_create_from_xpm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_create_from_xpm_d", (PyCFunction)_wrap_gdk_pixmap_create_from_xpm_d, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_colormap_create_from_xpm_d", (PyCFunction)_wrap_gdk_pixmap_colormap_create_from_xpm_d, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_foreign_new", (PyCFunction)_wrap_gdk_pixmap_foreign_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_lookup", (PyCFunction)_wrap_gdk_pixmap_lookup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_foreign_new_for_display", (PyCFunction)_wrap_gdk_pixmap_foreign_new_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_lookup_for_display", (PyCFunction)_wrap_gdk_pixmap_lookup_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "synthesize_window_state", (PyCFunction)_wrap_gdk_synthesize_window_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "utf8_to_string_target", (PyCFunction)_wrap_gdk_utf8_to_string_target, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "free_compound_text", (PyCFunction)_wrap_gdk_free_compound_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "atom_intern", (PyCFunction)_wrap_gdk_atom_intern, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_xpixel_from_rgb", (PyCFunction)_wrap_gdk_rgb_xpixel_from_rgb, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_gc_set_foreground", (PyCFunction)_wrap_gdk_rgb_gc_set_foreground, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_gc_set_background", (PyCFunction)_wrap_gdk_rgb_gc_set_background, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_ditherable", (PyCFunction)_wrap_gdk_rgb_ditherable, METH_NOARGS,
      NULL },
    { "rgb_set_verbose", (PyCFunction)_wrap_gdk_rgb_set_verbose, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_set_install", (PyCFunction)_wrap_gdk_rgb_set_install, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_set_min_colors", (PyCFunction)_wrap_gdk_rgb_set_min_colors, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_get_colormap", (PyCFunction)_wrap_gdk_rgb_get_colormap, METH_NOARGS,
      NULL },
    { "rgb_get_cmap", (PyCFunction)_wrap_gdk_rgb_get_cmap, METH_NOARGS,
      NULL },
    { "rgb_get_visual", (PyCFunction)_wrap_gdk_rgb_get_visual, METH_NOARGS,
      NULL },
    { "rgb_init", (PyCFunction)_wrap_gdk_rgb_init, METH_NOARGS,
      NULL },
    { "rgb_find_color", (PyCFunction)_wrap_gdk_rgb_find_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rgb_colormap_ditherable", (PyCFunction)_wrap_gdk_rgb_colormap_ditherable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "screen_get_default", (PyCFunction)_wrap_gdk_screen_get_default, METH_NOARGS,
      NULL },
    { "selection_owner_set", (PyCFunction)_wrap_gdk_selection_owner_set, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_owner_get", (PyCFunction)_wrap_gdk_selection_owner_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_owner_set_for_display", (PyCFunction)_wrap_gdk_selection_owner_set_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_owner_get_for_display", (PyCFunction)_wrap_gdk_selection_owner_get_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_send_notify", (PyCFunction)_wrap_gdk_selection_send_notify, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "selection_send_notify_for_display", (PyCFunction)_wrap_gdk_selection_send_notify_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "visual_get_best_depth", (PyCFunction)_wrap_gdk_visual_get_best_depth, METH_NOARGS,
      NULL },
    { "visual_get_best_type", (PyCFunction)_wrap_gdk_visual_get_best_type, METH_NOARGS,
      NULL },
    { "visual_get_system", (PyCFunction)_wrap_gdk_visual_get_system, METH_NOARGS,
      NULL },
    { "visual_get_best", (PyCFunction)_wrap_gdk_visual_get_best, METH_NOARGS,
      NULL },
    { "visual_get_best_with_depth", (PyCFunction)_wrap_gdk_visual_get_best_with_depth, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "visual_get_best_with_type", (PyCFunction)_wrap_gdk_visual_get_best_with_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_depths", (PyCFunction)_wrap_gdk_query_depths, METH_NOARGS,
      NULL },
    { "query_visual_types", (PyCFunction)_wrap_gdk_query_visual_types, METH_NOARGS,
      NULL },
    { "list_visuals", (PyCFunction)_wrap_gdk_list_visuals, METH_NOARGS,
      NULL },
    { "window_at_pointer", (PyCFunction)_wrap_gdk_window_at_pointer, METH_NOARGS,
      NULL },
    { "window_foreign_new", (PyCFunction)_wrap_gdk_window_foreign_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "window_lookup", (PyCFunction)_wrap_gdk_window_lookup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "window_foreign_new_for_display", (PyCFunction)_wrap_gdk_window_foreign_new_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "window_lookup_for_display", (PyCFunction)_wrap_gdk_window_lookup_for_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_sm_client_id", (PyCFunction)_wrap_gdk_set_sm_client_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "window_get_toplevels", (PyCFunction)_wrap_gdk_window_get_toplevels, METH_NOARGS,
      NULL },
    { "window_process_all_updates", (PyCFunction)_wrap_gdk_window_process_all_updates, METH_NOARGS,
      NULL },
    { "get_default_root_window", (PyCFunction)_wrap_gdk_get_default_root_window, METH_NOARGS,
      NULL },
    { "pixbuf_new_from_file", (PyCFunction)_wrap_gdk_pixbuf_new_from_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_file_at_size", (PyCFunction)_wrap_gdk_pixbuf_new_from_file_at_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_data", (PyCFunction)_wrap_gdk_pixbuf_new_from_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_array", (PyCFunction)_wrap_gdk_pixbuf_new_from_array, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_xpm_data", (PyCFunction)_wrap_gdk_pixbuf_new_from_xpm_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_inline", (PyCFunction)_wrap_gdk_pixbuf_new_from_inline, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_file_at_scale", (PyCFunction)_wrap_gdk_pixbuf_new_from_file_at_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_get_formats", (PyCFunction)_wrap_gdk_pixbuf_get_formats, METH_NOARGS,
      NULL },
    { "pixbuf_get_file_info", (PyCFunction)_wrap_gdk_pixbuf_get_file_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_loader_new", (PyCFunction)_wrap_gdk_pixbuf_loader_new, METH_NOARGS,
      NULL },
    { "pixbuf_loader_new_with_mime_type", (PyCFunction)_wrap_gdk_pixbuf_loader_new_with_mime_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "region_rectangle", (PyCFunction)_wrap_gdk_region_rectangle, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixmap_foreign_new_for_screen", (PyCFunction)_wrap_gdk_pixmap_foreign_new_for_screen, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_request_motions", (PyCFunction)_wrap_gdk_event_request_motions, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "notify_startup_complete_with_id", (PyCFunction)_wrap_gdk_notify_startup_complete_with_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_stream", (PyCFunction)_wrap_gdk_pixbuf_new_from_stream, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pixbuf_new_from_stream_at_scale", (PyCFunction)_wrap_gdk_pixbuf_new_from_stream_at_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "color_from_hsv", (PyCFunction)_wrap_color_from_hsv, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pygdk_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_enum_add(module, "CursorType", strip_prefix, GDK_TYPE_CURSOR_TYPE);
  pyg_flags_add(module, "DragAction", strip_prefix, GDK_TYPE_DRAG_ACTION);
  pyg_enum_add(module, "DragProtocol", strip_prefix, GDK_TYPE_DRAG_PROTOCOL);
  pyg_enum_add(module, "FilterReturn", strip_prefix, GDK_TYPE_FILTER_RETURN);
  pyg_enum_add(module, "EventType", strip_prefix, GDK_TYPE_EVENT_TYPE);
  pyg_flags_add(module, "EventMask", strip_prefix, GDK_TYPE_EVENT_MASK);
  pyg_enum_add(module, "VisibilityState", strip_prefix, GDK_TYPE_VISIBILITY_STATE);
  pyg_enum_add(module, "ScrollDirection", strip_prefix, GDK_TYPE_SCROLL_DIRECTION);
  pyg_enum_add(module, "NotifyType", strip_prefix, GDK_TYPE_NOTIFY_TYPE);
  pyg_enum_add(module, "CrossingMode", strip_prefix, GDK_TYPE_CROSSING_MODE);
  pyg_enum_add(module, "PropertyState", strip_prefix, GDK_TYPE_PROPERTY_STATE);
  pyg_flags_add(module, "WindowState", strip_prefix, GDK_TYPE_WINDOW_STATE);
  pyg_enum_add(module, "SettingAction", strip_prefix, GDK_TYPE_SETTING_ACTION);
  pyg_enum_add(module, "FontType", strip_prefix, GDK_TYPE_FONT_TYPE);
  pyg_enum_add(module, "CapStyle", strip_prefix, GDK_TYPE_CAP_STYLE);
  pyg_enum_add(module, "Fill", strip_prefix, GDK_TYPE_FILL);
  pyg_enum_add(module, "Function", strip_prefix, GDK_TYPE_FUNCTION);
  pyg_enum_add(module, "JoinStyle", strip_prefix, GDK_TYPE_JOIN_STYLE);
  pyg_enum_add(module, "LineStyle", strip_prefix, GDK_TYPE_LINE_STYLE);
  pyg_enum_add(module, "SubwindowMode", strip_prefix, GDK_TYPE_SUBWINDOW_MODE);
  pyg_flags_add(module, "GCValuesMask", strip_prefix, GDK_TYPE_GC_VALUES_MASK);
  pyg_enum_add(module, "ImageType", strip_prefix, GDK_TYPE_IMAGE_TYPE);
  pyg_enum_add(module, "ExtensionMode", strip_prefix, GDK_TYPE_EXTENSION_MODE);
  pyg_enum_add(module, "InputSource", strip_prefix, GDK_TYPE_INPUT_SOURCE);
  pyg_enum_add(module, "InputMode", strip_prefix, GDK_TYPE_INPUT_MODE);
  pyg_enum_add(module, "AxisUse", strip_prefix, GDK_TYPE_AXIS_USE);
  pyg_enum_add(module, "PropMode", strip_prefix, GDK_TYPE_PROP_MODE);
  pyg_enum_add(module, "FillRule", strip_prefix, GDK_TYPE_FILL_RULE);
  pyg_enum_add(module, "OverlapType", strip_prefix, GDK_TYPE_OVERLAP_TYPE);
  pyg_enum_add(module, "RgbDither", strip_prefix, GDK_TYPE_RGB_DITHER);
  pyg_enum_add(module, "ByteOrder", strip_prefix, GDK_TYPE_BYTE_ORDER);
  pyg_flags_add(module, "ModifierType", strip_prefix, GDK_TYPE_MODIFIER_TYPE);
  pyg_flags_add(module, "InputCondition", strip_prefix, GDK_TYPE_INPUT_CONDITION);
  pyg_enum_add(module, "Status", strip_prefix, GDK_TYPE_STATUS);
  pyg_enum_add(module, "GrabStatus", strip_prefix, GDK_TYPE_GRAB_STATUS);
  pyg_enum_add(module, "VisualType", strip_prefix, GDK_TYPE_VISUAL_TYPE);
  pyg_enum_add(module, "WindowClass", strip_prefix, GDK_TYPE_WINDOW_CLASS);
  pyg_enum_add(module, "WindowType", strip_prefix, GDK_TYPE_WINDOW_TYPE);
  pyg_flags_add(module, "WindowAttributesType", strip_prefix, GDK_TYPE_WINDOW_ATTRIBUTES_TYPE);
  pyg_flags_add(module, "WindowHints", strip_prefix, GDK_TYPE_WINDOW_HINTS);
  pyg_enum_add(module, "WindowTypeHint", strip_prefix, GDK_TYPE_WINDOW_TYPE_HINT);
  pyg_flags_add(module, "WMDecoration", strip_prefix, GDK_TYPE_WM_DECORATION);
  pyg_flags_add(module, "WMFunction", strip_prefix, GDK_TYPE_WM_FUNCTION);
  pyg_enum_add(module, "Gravity", strip_prefix, GDK_TYPE_GRAVITY);
  pyg_enum_add(module, "WindowEdge", strip_prefix, GDK_TYPE_WINDOW_EDGE);
  pyg_enum_add(module, "PixbufAlphaMode", strip_prefix, GDK_TYPE_PIXBUF_ALPHA_MODE);
  pyg_enum_add(module, "Colorspace", strip_prefix, GDK_TYPE_COLORSPACE);
  pyg_enum_add(module, "PixbufError", strip_prefix, GDK_TYPE_PIXBUF_ERROR);
  pyg_enum_add(module, "PixbufRotation", strip_prefix, GDK_TYPE_PIXBUF_ROTATION);
  pyg_enum_add(module, "InterpType", strip_prefix, GDK_TYPE_INTERP_TYPE);
  pyg_enum_add(module, "OwnerChange", strip_prefix, GDK_TYPE_OWNER_CHANGE);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pygdk_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gio")) != NULL) {
        _PyGAppLaunchContext_Type = (PyTypeObject *)PyObject_GetAttrString(module, "AppLaunchContext");
        if (_PyGAppLaunchContext_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name AppLaunchContext from gio");
            return ;
        }
        _PyGIcon_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Icon");
        if (_PyGIcon_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Icon from gio");
            return ;
        }
        _PyGInputStream_Type = (PyTypeObject *)PyObject_GetAttrString(module, "InputStream");
        if (_PyGInputStream_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name InputStream from gio");
            return ;
        }
        _PyGCancellable_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Cancellable");
        if (_PyGCancellable_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Cancellable from gio");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gio");
        return ;
    }
    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        _PyGObject_Type = (PyTypeObject *)PyObject_GetAttrString(module, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return ;
    }
    if ((module = PyImport_ImportModule("pango")) != NULL) {
        _PyPangoContext_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Context");
        if (_PyPangoContext_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Context from pango");
            return ;
        }
        _PyPangoFont_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Font");
        if (_PyPangoFont_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Font from pango");
            return ;
        }
        _PyPangoLayout_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Layout");
        if (_PyPangoLayout_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Layout from pango");
            return ;
        }
        _PyPangoRenderer_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Renderer");
        if (_PyPangoRenderer_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Renderer from pango");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import pango");
        return ;
    }


#line 244 "gdkcairo.override"
#ifdef HAVE_PYCAIRO
    if ((module = PyImport_ImportModule("pangocairo")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyPangoCairoContext_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "CairoContext");
        if (_PyPangoCairoContext_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name CairoContext from pangocairo");
            return;
        }
    } else {
        return;
    }

    PyGdkCairoContext_Type.tp_base = &PyPangoCairoContext_Type;
    if (PyType_Ready(&PyGdkCairoContext_Type) < 0) {
        g_return_if_reached();
    }
    if (PyDict_SetItemString(d, "CairoContext", (PyObject *)&PyGdkCairoContext_Type) < 0) {
        g_return_if_reached();
    }
#endif


#line 19784 "gdk.c"
    pyg_register_boxed(d, "Event", GDK_TYPE_EVENT, &PyGdkEvent_Type);
    pyg_register_boxed(d, "Font", GDK_TYPE_FONT, &PyGdkFont_Type);
    pyg_register_boxed(d, "Color", GDK_TYPE_COLOR, &PyGdkColor_Type);
    pyg_register_boxed(d, "Cursor", GDK_TYPE_CURSOR, &PyGdkCursor_Type);
    pyg_register_boxed(d, "Rectangle", GDK_TYPE_RECTANGLE, &PyGdkRectangle_Type);
    pyg_register_boxed(d, "Region", PYGDK_TYPE_REGION, &PyGdkRegion_Type);
    pygobject_register_class(d, "GdkAppLaunchContext", GDK_TYPE_APP_LAUNCH_CONTEXT, &PyGdkAppLaunchContext_Type, Py_BuildValue("(O)", &PyGAppLaunchContext_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_APP_LAUNCH_CONTEXT);
    pygobject_register_class(d, "GdkColormap", GDK_TYPE_COLORMAP, &PyGdkColormap_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GdkDevice", GDK_TYPE_DEVICE, &PyGdkDevice_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_DEVICE);
    pygobject_register_class(d, "GdkDisplay", GDK_TYPE_DISPLAY, &PyGdkDisplay_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_register_class_init(GDK_TYPE_DISPLAY, __GdkDisplay_class_init);
    pygobject_register_class(d, "GdkDisplayManager", GDK_TYPE_DISPLAY_MANAGER, &PyGdkDisplayManager_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_DISPLAY_MANAGER);
    pyg_register_class_init(GDK_TYPE_DISPLAY_MANAGER, __GdkDisplayManager_class_init);
    pygobject_register_class(d, "GdkDragContext", GDK_TYPE_DRAG_CONTEXT, &PyGdkDragContext_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_DRAG_CONTEXT);
    pygobject_register_class(d, "GdkDrawable", GDK_TYPE_DRAWABLE, &PyGdkDrawable_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_DRAWABLE);
    pyg_register_class_init(GDK_TYPE_DRAWABLE, __GdkDrawable_class_init);
    pygobject_register_class(d, "GdkWindow", GDK_TYPE_WINDOW, &PyGdkWindow_Type, Py_BuildValue("(O)", &PyGdkDrawable_Type));
    pygobject_register_class(d, "GdkPangoRenderer", GDK_TYPE_PANGO_RENDERER, &PyGdkPangoRenderer_Type, Py_BuildValue("(O)", &PyPangoRenderer_Type));
    pygobject_register_class(d, "GdkPixmap", GDK_TYPE_PIXMAP, &PyGdkPixmap_Type, Py_BuildValue("(O)", &PyGdkDrawable_Type));
    pygobject_register_class(d, "GdkGC", GDK_TYPE_GC, &PyGdkGC_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_register_class_init(GDK_TYPE_GC, __GdkGC_class_init);
    pygobject_register_class(d, "GdkImage", GDK_TYPE_IMAGE, &PyGdkImage_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GdkKeymap", GDK_TYPE_KEYMAP, &PyGdkKeymap_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_KEYMAP);
    pyg_register_class_init(GDK_TYPE_KEYMAP, __GdkKeymap_class_init);
    pygobject_register_class(d, "GdkPixbuf", GDK_TYPE_PIXBUF, &PyGdkPixbuf_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GdkPixbufAnimation", GDK_TYPE_PIXBUF_ANIMATION, &PyGdkPixbufAnimation_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GdkPixbufAnimationIter", GDK_TYPE_PIXBUF_ANIMATION_ITER, &PyGdkPixbufAnimationIter_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_PIXBUF_ANIMATION_ITER);
    pygobject_register_class(d, "GdkPixbufLoader", GDK_TYPE_PIXBUF_LOADER, &PyGdkPixbufLoader_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_register_class_init(GDK_TYPE_PIXBUF_LOADER, __GdkPixbufLoader_class_init);
    pygobject_register_class(d, "GdkPixbufSimpleAnim", GDK_TYPE_PIXBUF_SIMPLE_ANIM, &PyGdkPixbufSimpleAnim_Type, Py_BuildValue("(O)", &PyGdkPixbufAnimation_Type));
    pygobject_register_class(d, "GdkPixbufSimpleAnimIter", GDK_TYPE_PIXBUF_SIMPLE_ANIM_ITER, &PyGdkPixbufSimpleAnimIter_Type, Py_BuildValue("(O)", &PyGdkPixbufAnimationIter_Type));
    pyg_set_object_has_new_constructor(GDK_TYPE_PIXBUF_SIMPLE_ANIM_ITER);
    pygobject_register_class(d, "GdkScreen", GDK_TYPE_SCREEN, &PyGdkScreen_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_register_class_init(GDK_TYPE_SCREEN, __GdkScreen_class_init);
    pygobject_register_class(d, "GdkVisual", GDK_TYPE_VISUAL, &PyGdkVisual_Type, Py_BuildValue("(O)", &PyGObject_Type));
}
