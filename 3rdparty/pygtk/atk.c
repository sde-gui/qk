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


#line 24 "atk.override"
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include <atk/atk.h>
#include <atk/atk-enum-types.h>
#include <atk/atknoopobjectfactory.h>
#include <atk/atknoopobject.h>


#line 24 "atkrectangle.override"

static gboolean
pyatk_rectangle_from_pyobject(PyObject *object, AtkRectangle *rectangle)
{
    g_return_val_if_fail(rectangle != NULL, FALSE);
 
    if (pyg_boxed_check(object, ATK_TYPE_RECTANGLE)) {
        *rectangle = *pyg_boxed_get(object, AtkRectangle);
        return TRUE;
    }
    if (PyArg_ParseTuple(object, "iiii", &rectangle->x, &rectangle->y,
                                &rectangle->width, &rectangle->height)) {
        return TRUE;
    }
    PyErr_Clear();
    PyErr_SetString(PyExc_TypeError, "could not convert to AtkRectangle");
    return FALSE;
}

static PyObject *
PyAtkRectangle_from_value(const GValue *value)
{
    AtkRectangle *rect = (AtkRectangle *)g_value_get_boxed(value);

    return pyg_boxed_new(ATK_TYPE_RECTANGLE, rect, TRUE, TRUE);
}
static int
PyAtkRectangle_to_value(GValue *value, PyObject *object)
{
    AtkRectangle rect;

    if (!pyatk_rectangle_from_pyobject(object, &rect))
        return -1;

    g_value_set_boxed(value, &rect);
    return 0;
}

void
_pyatk_register_boxed_types(void)
{
    pyg_register_boxed_custom(ATK_TYPE_RECTANGLE,
                              PyAtkRectangle_from_value,
                              PyAtkRectangle_to_value);
}
#line 75 "atk.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyAtkRectangle_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkHyperlink_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkGObjectAccessible_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkObject_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkNoOpObject_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkObjectFactory_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkNoOpObjectFactory_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkRegistry_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkRelation_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkRelationSet_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkStateSet_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkUtil_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkAction_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkComponent_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkDocument_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkEditableText_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkHypertext_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkImage_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkImplementorIface_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkSelection_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkStreamableContent_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkTable_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkText_Type;
PyTypeObject G_GNUC_INTERNAL PyAtkValue_Type;

#line 109 "atk.c"



/* ----------- AtkRectangle ----------- */

#line 71 "atkrectangle.override"
static int
_wrap_atk_rectangle_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    AtkRectangle rect = {0, 0, 0, 0};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|iiii:AtkRectangle.__init__",
				     kwlist, &(rect.x), &(rect.y),
				     &(rect.width), &(rect.height)))
	return -1;

    self->boxed =  g_boxed_copy(ATK_TYPE_RECTANGLE, &rect);
    self->free_on_dealloc = TRUE;
    self->gtype = ATK_TYPE_RECTANGLE;
    
    return 0;
}
#line 134 "atk.c"


#line 156 "atkrectangle.override"
static int
_wrap_atk_rectangle__set_x(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, AtkRectangle)->x = val;
    return 0;
}
#line 149 "atk.c"


static PyObject *
_wrap_atk_rectangle__get_x(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, AtkRectangle)->x;
    return PyInt_FromLong(ret);
}

#line 169 "atkrectangle.override"
static int
_wrap_atk_rectangle__set_y(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, AtkRectangle)->y = val;
    return 0;
}
#line 173 "atk.c"


static PyObject *
_wrap_atk_rectangle__get_y(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, AtkRectangle)->y;
    return PyInt_FromLong(ret);
}

#line 182 "atkrectangle.override"
static int
_wrap_atk_rectangle__set_width(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, AtkRectangle)->width = val;
    return 0;
}
#line 197 "atk.c"


static PyObject *
_wrap_atk_rectangle__get_width(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, AtkRectangle)->width;
    return PyInt_FromLong(ret);
}

#line 195 "atkrectangle.override"
static int
_wrap_atk_rectangle__set_height(PyGBoxed *self, PyObject *value, void *closure)
{
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred())
        return -1;
    pyg_boxed_get(self, AtkRectangle)->height = val;
    return 0;
}
#line 221 "atk.c"


static PyObject *
_wrap_atk_rectangle__get_height(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, AtkRectangle)->height;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef atk_rectangle_getsets[] = {
    { "x", (getter)_wrap_atk_rectangle__get_x, (setter)_wrap_atk_rectangle__set_x },
    { "y", (getter)_wrap_atk_rectangle__get_y, (setter)_wrap_atk_rectangle__set_y },
    { "width", (getter)_wrap_atk_rectangle__get_width, (setter)_wrap_atk_rectangle__set_width },
    { "height", (getter)_wrap_atk_rectangle__get_height, (setter)_wrap_atk_rectangle__set_height },
    { NULL, (getter)0, (setter)0 },
};

#line 91 "atkrectangle.override"
static Py_ssize_t
_wrap_atk_rectangle_length(PyGBoxed *self)
{
    return 4;
}
static PyObject *
_wrap_atk_rectangle_getitem(PyGBoxed *self, Py_ssize_t pos)
{
    AtkRectangle *rect;

    if (pos < 0) pos += 4;
    if (pos < 0 || pos >= 4) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return NULL;
    }
    rect = pyg_boxed_get(self, AtkRectangle);
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
_wrap_atk_rectangle_setitem(PyGBoxed *self, Py_ssize_t pos, PyObject *value)
{
    AtkRectangle *rect;
    gint val;

    if (pos < 0) pos += 4;
    if (pos < 0 || pos >= 4) {
        PyErr_SetString(PyExc_IndexError, "index out of range");
        return -1;
    }
    rect = pyg_boxed_get(self, AtkRectangle);
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
static PySequenceMethods _wrap_atk_rectangle_tp_as_sequence = {
    (lenfunc)_wrap_atk_rectangle_length, /* sq_length */
    0,                          /* sq_concat */
    0,                          /* sq_repeat */
    (ssizeargfunc)_wrap_atk_rectangle_getitem, /* sq_item */
    0,                          /* sq_slice */
    (ssizeobjargproc)_wrap_atk_rectangle_setitem, /* sq_ass_item */
    0,                          /* sq_contains */
    0,                          /* sq_inplace_concat */
    0                           /* sq_inplace_repeat */
};
#line 305 "atk.c"


PyTypeObject G_GNUC_INTERNAL PyAtkRectangle_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Rectangle",                   /* tp_name */
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
    (PySequenceMethods*)&_wrap_atk_rectangle_tp_as_sequence, /* tp_as_sequence */
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
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)atk_rectangle_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_atk_rectangle_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkHyperlink ----------- */

static PyObject *
_wrap_atk_hyperlink_get_uri(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Hyperlink.get_uri", kwlist, &i))
        return NULL;
    
    ret = atk_hyperlink_get_uri(ATK_HYPERLINK(self->obj), i);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_hyperlink_get_object(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Hyperlink.get_object", kwlist, &i))
        return NULL;
    
    ret = atk_hyperlink_get_object(ATK_HYPERLINK(self->obj), i);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_hyperlink_get_end_index(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_get_end_index(ATK_HYPERLINK(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_hyperlink_get_start_index(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_get_start_index(ATK_HYPERLINK(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_hyperlink_is_valid(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_is_valid(ATK_HYPERLINK(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_hyperlink_get_n_anchors(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_get_n_anchors(ATK_HYPERLINK(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_hyperlink_is_inline(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_is_inline(ATK_HYPERLINK(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_hyperlink_is_selected_link(PyGObject *self)
{
    int ret;

    
    ret = atk_hyperlink_is_selected_link(ATK_HYPERLINK(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkHyperlink__do_get_uri(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Hyperlink.get_uri", kwlist, &PyAtkHyperlink_Type, &self, &i))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->get_uri)
        ret = ATK_HYPERLINK_CLASS(klass)->get_uri(ATK_HYPERLINK(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.get_uri not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkHyperlink__do_get_object(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Hyperlink.get_object", kwlist, &PyAtkHyperlink_Type, &self, &i))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->get_object)
        ret = ATK_HYPERLINK_CLASS(klass)->get_object(ATK_HYPERLINK(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.get_object not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkHyperlink__do_get_end_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.get_end_index", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->get_end_index)
        ret = ATK_HYPERLINK_CLASS(klass)->get_end_index(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.get_end_index not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHyperlink__do_get_start_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.get_start_index", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->get_start_index)
        ret = ATK_HYPERLINK_CLASS(klass)->get_start_index(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.get_start_index not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHyperlink__do_is_valid(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.is_valid", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->is_valid)
        ret = ATK_HYPERLINK_CLASS(klass)->is_valid(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.is_valid not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkHyperlink__do_get_n_anchors(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.get_n_anchors", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->get_n_anchors)
        ret = ATK_HYPERLINK_CLASS(klass)->get_n_anchors(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.get_n_anchors not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHyperlink__do_link_state(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.link_state", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->link_state)
        ret = ATK_HYPERLINK_CLASS(klass)->link_state(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.link_state not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_AtkHyperlink__do_is_selected_link(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.is_selected_link", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->is_selected_link)
        ret = ATK_HYPERLINK_CLASS(klass)->is_selected_link(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.is_selected_link not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkHyperlink__do_link_activated(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hyperlink.link_activated", kwlist, &PyAtkHyperlink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_HYPERLINK_CLASS(klass)->link_activated)
        ATK_HYPERLINK_CLASS(klass)->link_activated(ATK_HYPERLINK(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Hyperlink.link_activated not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkHyperlink_methods[] = {
    { "get_uri", (PyCFunction)_wrap_atk_hyperlink_get_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_object", (PyCFunction)_wrap_atk_hyperlink_get_object, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_end_index", (PyCFunction)_wrap_atk_hyperlink_get_end_index, METH_NOARGS,
      NULL },
    { "get_start_index", (PyCFunction)_wrap_atk_hyperlink_get_start_index, METH_NOARGS,
      NULL },
    { "is_valid", (PyCFunction)_wrap_atk_hyperlink_is_valid, METH_NOARGS,
      NULL },
    { "get_n_anchors", (PyCFunction)_wrap_atk_hyperlink_get_n_anchors, METH_NOARGS,
      NULL },
    { "is_inline", (PyCFunction)_wrap_atk_hyperlink_is_inline, METH_NOARGS,
      NULL },
    { "is_selected_link", (PyCFunction)_wrap_atk_hyperlink_is_selected_link, METH_NOARGS,
      NULL },
    { "do_get_uri", (PyCFunction)_wrap_AtkHyperlink__do_get_uri, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_object", (PyCFunction)_wrap_AtkHyperlink__do_get_object, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_end_index", (PyCFunction)_wrap_AtkHyperlink__do_get_end_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_start_index", (PyCFunction)_wrap_AtkHyperlink__do_get_start_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_valid", (PyCFunction)_wrap_AtkHyperlink__do_is_valid, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_anchors", (PyCFunction)_wrap_AtkHyperlink__do_get_n_anchors, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_link_state", (PyCFunction)_wrap_AtkHyperlink__do_link_state, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_selected_link", (PyCFunction)_wrap_AtkHyperlink__do_is_selected_link, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_link_activated", (PyCFunction)_wrap_AtkHyperlink__do_link_activated, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkHyperlink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Hyperlink",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkHyperlink_methods, /* tp_methods */
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

static gchar*
_wrap_AtkHyperlink__proxy_do_get_uri(AtkHyperlink *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_uri");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkHyperlink__proxy_do_get_object(AtkHyperlink *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    AtkObject* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_object");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkHyperlink__proxy_do_get_end_index(AtkHyperlink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_end_index");
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
static gint
_wrap_AtkHyperlink__proxy_do_get_start_index(AtkHyperlink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_start_index");
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
static gboolean
_wrap_AtkHyperlink__proxy_do_is_valid(AtkHyperlink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_is_valid");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkHyperlink__proxy_do_get_n_anchors(AtkHyperlink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_anchors");
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
static guint
_wrap_AtkHyperlink__proxy_do_link_state(AtkHyperlink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    guint retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_link_state");
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
static gboolean
_wrap_AtkHyperlink__proxy_do_is_selected_link(AtkHyperlink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_is_selected_link");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkHyperlink__proxy_do_link_activated(AtkHyperlink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_link_activated");
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
__AtkHyperlink_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    AtkHyperlinkClass *klass = ATK_HYPERLINK_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_uri");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_uri")))
            klass->get_uri = _wrap_AtkHyperlink__proxy_do_get_uri;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_object");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_object")))
            klass->get_object = _wrap_AtkHyperlink__proxy_do_get_object;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_end_index");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_end_index")))
            klass->get_end_index = _wrap_AtkHyperlink__proxy_do_get_end_index;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_start_index");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_start_index")))
            klass->get_start_index = _wrap_AtkHyperlink__proxy_do_get_start_index;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_is_valid");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "is_valid")))
            klass->is_valid = _wrap_AtkHyperlink__proxy_do_is_valid;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_n_anchors");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_n_anchors")))
            klass->get_n_anchors = _wrap_AtkHyperlink__proxy_do_get_n_anchors;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_link_state");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "link_state")))
            klass->link_state = _wrap_AtkHyperlink__proxy_do_link_state;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_is_selected_link");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "is_selected_link")))
            klass->is_selected_link = _wrap_AtkHyperlink__proxy_do_is_selected_link;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_link_activated");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "link_activated")))
            klass->link_activated = _wrap_AtkHyperlink__proxy_do_link_activated;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- AtkObject ----------- */

static PyObject *
_wrap_atk_object_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = atk_object_get_name(ATK_OBJECT(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_get_description(PyGObject *self)
{
    const gchar *ret;

    
    ret = atk_object_get_description(ATK_OBJECT(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_get_parent(PyGObject *self)
{
    AtkObject *ret;

    
    ret = atk_object_get_parent(ATK_OBJECT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_object_get_n_accessible_children(PyGObject *self)
{
    int ret;

    
    ret = atk_object_get_n_accessible_children(ATK_OBJECT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_object_ref_accessible_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Object.ref_accessible_child", kwlist, &i))
        return NULL;
    
    ret = atk_object_ref_accessible_child(ATK_OBJECT(self->obj), i);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_object_ref_relation_set(PyGObject *self)
{
    AtkRelationSet *ret;

    
    ret = atk_object_ref_relation_set(ATK_OBJECT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_object_get_role(PyGObject *self)
{
    gint ret;

    
    ret = atk_object_get_role(ATK_OBJECT(self->obj));
    
    return pyg_enum_from_gtype(ATK_TYPE_ROLE, ret);
}

static PyObject *
_wrap_atk_object_get_layer(PyGObject *self)
{
    gint ret;

    
    ret = atk_object_get_layer(ATK_OBJECT(self->obj));
    
    return pyg_enum_from_gtype(ATK_TYPE_LAYER, ret);
}

static PyObject *
_wrap_atk_object_get_mdi_zorder(PyGObject *self)
{
    int ret;

    
    ret = atk_object_get_mdi_zorder(ATK_OBJECT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_object_ref_state_set(PyGObject *self)
{
    AtkStateSet *ret;

    
    ret = atk_object_ref_state_set(ATK_OBJECT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_object_get_index_in_parent(PyGObject *self)
{
    int ret;

    
    ret = atk_object_get_index_in_parent(ATK_OBJECT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_object_set_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Atk.Object.set_name", kwlist, &name))
        return NULL;
    
    atk_object_set_name(ATK_OBJECT(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_set_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "description", NULL };
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Atk.Object.set_description", kwlist, &description))
        return NULL;
    
    atk_object_set_description(ATK_OBJECT(self->obj), description);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.set_parent", kwlist, &PyAtkObject_Type, &parent))
        return NULL;
    
    atk_object_set_parent(ATK_OBJECT(self->obj), ATK_OBJECT(parent->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_set_role(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "role", NULL };
    PyObject *py_role = NULL;
    AtkRole role;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.Object.set_role", kwlist, &py_role))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_ROLE, py_role, (gpointer)&role))
        return NULL;
    
    atk_object_set_role(ATK_OBJECT(self->obj), role);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_remove_property_change_handler(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.Object.remove_property_change_handler", kwlist, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    atk_object_remove_property_change_handler(ATK_OBJECT(self->obj), handler_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_add_relationship(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", "target", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    int ret;
    PyGObject *target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!:Atk.Object.add_relationship", kwlist, &py_relationship, &PyAtkObject_Type, &target))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    ret = atk_object_add_relationship(ATK_OBJECT(self->obj), relationship, ATK_OBJECT(target->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_object_remove_relationship(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", "target", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    int ret;
    PyGObject *target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!:Atk.Object.remove_relationship", kwlist, &py_relationship, &PyAtkObject_Type, &target))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    ret = atk_object_remove_relationship(ATK_OBJECT(self->obj), relationship, ATK_OBJECT(target->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkObject__do_get_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_name", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_name)
        ret = ATK_OBJECT_CLASS(klass)->get_name(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_name not implemented");
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
_wrap_AtkObject__do_get_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_description", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_description)
        ret = ATK_OBJECT_CLASS(klass)->get_description(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_description not implemented");
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
_wrap_AtkObject__do_get_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_parent", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_parent)
        ret = ATK_OBJECT_CLASS(klass)->get_parent(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_parent not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkObject__do_get_n_children(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_n_children", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_n_children)
        ret = ATK_OBJECT_CLASS(klass)->get_n_children(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_n_children not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkObject__do_ref_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Object.ref_child", kwlist, &PyAtkObject_Type, &self, &i))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->ref_child)
        ret = ATK_OBJECT_CLASS(klass)->ref_child(ATK_OBJECT(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.ref_child not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkObject__do_get_index_in_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_index_in_parent", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_index_in_parent)
        ret = ATK_OBJECT_CLASS(klass)->get_index_in_parent(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_index_in_parent not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkObject__do_ref_relation_set(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    AtkRelationSet *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.ref_relation_set", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->ref_relation_set)
        ret = ATK_OBJECT_CLASS(klass)->ref_relation_set(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.ref_relation_set not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkObject__do_get_role(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_role", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_role)
        ret = ATK_OBJECT_CLASS(klass)->get_role(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_role not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(ATK_TYPE_ROLE, ret);
}

static PyObject *
_wrap_AtkObject__do_get_layer(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_layer", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_layer)
        ret = ATK_OBJECT_CLASS(klass)->get_layer(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_layer not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(ATK_TYPE_LAYER, ret);
}

static PyObject *
_wrap_AtkObject__do_get_mdi_zorder(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.get_mdi_zorder", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->get_mdi_zorder)
        ret = ATK_OBJECT_CLASS(klass)->get_mdi_zorder(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.get_mdi_zorder not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkObject__do_ref_state_set(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    AtkStateSet *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.ref_state_set", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->ref_state_set)
        ret = ATK_OBJECT_CLASS(klass)->ref_state_set(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.ref_state_set not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkObject__do_set_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "name", NULL };
    PyGObject *self;
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:Atk.Object.set_name", kwlist, &PyAtkObject_Type, &self, &name))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->set_name)
        ATK_OBJECT_CLASS(klass)->set_name(ATK_OBJECT(self->obj), name);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.set_name not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_set_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "description", NULL };
    PyGObject *self;
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:Atk.Object.set_description", kwlist, &PyAtkObject_Type, &self, &description))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->set_description)
        ATK_OBJECT_CLASS(klass)->set_description(ATK_OBJECT(self->obj), description);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.set_description not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_set_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "parent", NULL };
    PyGObject *self, *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:Atk.Object.set_parent", kwlist, &PyAtkObject_Type, &self, &PyAtkObject_Type, &parent))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->set_parent)
        ATK_OBJECT_CLASS(klass)->set_parent(ATK_OBJECT(self->obj), ATK_OBJECT(parent->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.set_parent not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_set_role(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "role", NULL };
    PyGObject *self;
    AtkRole role;
    PyObject *py_role = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Atk.Object.set_role", kwlist, &PyAtkObject_Type, &self, &py_role))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_ROLE, py_role, (gpointer)&role))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->set_role)
        ATK_OBJECT_CLASS(klass)->set_role(ATK_OBJECT(self->obj), role);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.set_role not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_remove_property_change_handler(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "handler_id", NULL };
    PyGObject *self;
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Atk.Object.remove_property_change_handler", kwlist, &PyAtkObject_Type, &self, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->remove_property_change_handler)
        ATK_OBJECT_CLASS(klass)->remove_property_change_handler(ATK_OBJECT(self->obj), handler_id);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.remove_property_change_handler not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_focus_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "focus_in", NULL };
    PyGObject *self;
    int focus_in;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Object.focus_event", kwlist, &PyAtkObject_Type, &self, &focus_in))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->focus_event)
        ATK_OBJECT_CLASS(klass)->focus_event(ATK_OBJECT(self->obj), focus_in);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.focus_event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_state_change(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "name", "state_set", NULL };
    PyGObject *self;
    char *name;
    int state_set;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!si:Atk.Object.state_change", kwlist, &PyAtkObject_Type, &self, &name, &state_set))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->state_change)
        ATK_OBJECT_CLASS(klass)->state_change(ATK_OBJECT(self->obj), name, state_set);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.state_change not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkObject__do_visible_data_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Object.visible_data_changed", kwlist, &PyAtkObject_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_CLASS(klass)->visible_data_changed)
        ATK_OBJECT_CLASS(klass)->visible_data_changed(ATK_OBJECT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.Object.visible_data_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkObject_methods[] = {
    { "get_name", (PyCFunction)_wrap_atk_object_get_name, METH_NOARGS,
      NULL },
    { "get_description", (PyCFunction)_wrap_atk_object_get_description, METH_NOARGS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_atk_object_get_parent, METH_NOARGS,
      NULL },
    { "get_n_accessible_children", (PyCFunction)_wrap_atk_object_get_n_accessible_children, METH_NOARGS,
      NULL },
    { "ref_accessible_child", (PyCFunction)_wrap_atk_object_ref_accessible_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ref_relation_set", (PyCFunction)_wrap_atk_object_ref_relation_set, METH_NOARGS,
      NULL },
    { "get_role", (PyCFunction)_wrap_atk_object_get_role, METH_NOARGS,
      NULL },
    { "get_layer", (PyCFunction)_wrap_atk_object_get_layer, METH_NOARGS,
      NULL },
    { "get_mdi_zorder", (PyCFunction)_wrap_atk_object_get_mdi_zorder, METH_NOARGS,
      NULL },
    { "ref_state_set", (PyCFunction)_wrap_atk_object_ref_state_set, METH_NOARGS,
      NULL },
    { "get_index_in_parent", (PyCFunction)_wrap_atk_object_get_index_in_parent, METH_NOARGS,
      NULL },
    { "set_name", (PyCFunction)_wrap_atk_object_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_description", (PyCFunction)_wrap_atk_object_set_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_parent", (PyCFunction)_wrap_atk_object_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_role", (PyCFunction)_wrap_atk_object_set_role, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_property_change_handler", (PyCFunction)_wrap_atk_object_remove_property_change_handler, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_relationship", (PyCFunction)_wrap_atk_object_add_relationship, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_relationship", (PyCFunction)_wrap_atk_object_remove_relationship, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_name", (PyCFunction)_wrap_AtkObject__do_get_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_description", (PyCFunction)_wrap_AtkObject__do_get_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_parent", (PyCFunction)_wrap_AtkObject__do_get_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_children", (PyCFunction)_wrap_AtkObject__do_get_n_children, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_ref_child", (PyCFunction)_wrap_AtkObject__do_ref_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_index_in_parent", (PyCFunction)_wrap_AtkObject__do_get_index_in_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_ref_relation_set", (PyCFunction)_wrap_AtkObject__do_ref_relation_set, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_role", (PyCFunction)_wrap_AtkObject__do_get_role, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_layer", (PyCFunction)_wrap_AtkObject__do_get_layer, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_mdi_zorder", (PyCFunction)_wrap_AtkObject__do_get_mdi_zorder, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_ref_state_set", (PyCFunction)_wrap_AtkObject__do_ref_state_set, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_name", (PyCFunction)_wrap_AtkObject__do_set_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_description", (PyCFunction)_wrap_AtkObject__do_set_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_parent", (PyCFunction)_wrap_AtkObject__do_set_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_role", (PyCFunction)_wrap_AtkObject__do_set_role, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_property_change_handler", (PyCFunction)_wrap_AtkObject__do_remove_property_change_handler, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_focus_event", (PyCFunction)_wrap_AtkObject__do_focus_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_state_change", (PyCFunction)_wrap_AtkObject__do_state_change, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_visible_data_changed", (PyCFunction)_wrap_AtkObject__do_visible_data_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Object",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkObject_methods, /* tp_methods */
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

static const gchar*
_wrap_AtkObject__proxy_do_get_name(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_name");
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
static const gchar*
_wrap_AtkObject__proxy_do_get_description(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_description");
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
static AtkObject*
_wrap_AtkObject__proxy_do_get_parent(AtkObject *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkObject* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_parent");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkObject__proxy_do_get_n_children(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_children");
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
static AtkObject*
_wrap_AtkObject__proxy_do_ref_child(AtkObject *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    AtkObject* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_child");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkObject__proxy_do_get_index_in_parent(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_index_in_parent");
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
static AtkRelationSet*
_wrap_AtkObject__proxy_do_ref_relation_set(AtkObject *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkRelationSet* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_relation_set");
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
    retval = (AtkRelationSet*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkRole
_wrap_AtkObject__proxy_do_get_role(AtkObject *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkRole retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_role");
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
    if (pyg_enum_get_value(ATK_TYPE_ROLE, py_retval, (gint *)&retval)) {
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
static AtkLayer
_wrap_AtkObject__proxy_do_get_layer(AtkObject *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkLayer retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_layer");
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
    if (pyg_enum_get_value(ATK_TYPE_LAYER, py_retval, (gint *)&retval)) {
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
static gint
_wrap_AtkObject__proxy_do_get_mdi_zorder(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_mdi_zorder");
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
static AtkStateSet*
_wrap_AtkObject__proxy_do_ref_state_set(AtkObject *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkStateSet* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_state_set");
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
    retval = (AtkStateSet*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkObject__proxy_do_set_name(AtkObject *self, const gchar*name)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_name = NULL;
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
    if (name)
        py_name = PyString_FromString(name);
    if (!py_name) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_name);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_name");
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
_wrap_AtkObject__proxy_do_set_description(AtkObject *self, const gchar*description)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_description = NULL;
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
    if (description)
        py_description = PyString_FromString(description);
    if (!py_description) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_description);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_description");
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
_wrap_AtkObject__proxy_do_set_parent(AtkObject *self, AtkObject*parent)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_parent = NULL;
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
    if (parent)
        py_parent = pygobject_new((GObject *) parent);
    else {
        Py_INCREF(Py_None);
        py_parent = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_parent);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_parent");
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
_wrap_AtkObject__proxy_do_set_role(AtkObject *self, AtkRole role)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_role;
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
    py_role = pyg_enum_from_gtype(ATK_TYPE_ROLE, role);
    if (!py_role) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_role);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_role");
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
_wrap_AtkObject__proxy_do_remove_property_change_handler(AtkObject *self, guint handler_id)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_handler_id;
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
    py_handler_id = PyInt_FromLong(handler_id);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_handler_id);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_property_change_handler");
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
_wrap_AtkObject__proxy_do_focus_event(AtkObject *self, gboolean focus_in)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_focus_in;
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
    py_focus_in = focus_in? Py_True : Py_False;
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_focus_in);
    PyTuple_SET_ITEM(py_args, 0, py_focus_in);
    
    py_method = PyObject_GetAttrString(py_self, "do_focus_event");
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
_wrap_AtkObject__proxy_do_state_change(AtkObject *self, const gchar*name, gboolean state_set)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_name = NULL;
    PyObject *py_state_set;
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
    if (name)
        py_name = PyString_FromString(name);
    if (!py_name) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_state_set = state_set? Py_True : Py_False;
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_name);
    Py_INCREF(py_state_set);
    PyTuple_SET_ITEM(py_args, 1, py_state_set);
    
    py_method = PyObject_GetAttrString(py_self, "do_state_change");
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
_wrap_AtkObject__proxy_do_visible_data_changed(AtkObject *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_visible_data_changed");
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
__AtkObject_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    AtkObjectClass *klass = ATK_OBJECT_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    /* overriding do_get_attributes is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_name");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_name")))
            klass->get_name = _wrap_AtkObject__proxy_do_get_name;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_description");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_description")))
            klass->get_description = _wrap_AtkObject__proxy_do_get_description;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_parent");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_parent")))
            klass->get_parent = _wrap_AtkObject__proxy_do_get_parent;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_n_children");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_n_children")))
            klass->get_n_children = _wrap_AtkObject__proxy_do_get_n_children;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_ref_child");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "ref_child")))
            klass->ref_child = _wrap_AtkObject__proxy_do_ref_child;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_index_in_parent");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_index_in_parent")))
            klass->get_index_in_parent = _wrap_AtkObject__proxy_do_get_index_in_parent;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_ref_relation_set");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "ref_relation_set")))
            klass->ref_relation_set = _wrap_AtkObject__proxy_do_ref_relation_set;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_role");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_role")))
            klass->get_role = _wrap_AtkObject__proxy_do_get_role;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_layer");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_layer")))
            klass->get_layer = _wrap_AtkObject__proxy_do_get_layer;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_mdi_zorder");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_mdi_zorder")))
            klass->get_mdi_zorder = _wrap_AtkObject__proxy_do_get_mdi_zorder;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_ref_state_set");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "ref_state_set")))
            klass->ref_state_set = _wrap_AtkObject__proxy_do_ref_state_set;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_name");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_name")))
            klass->set_name = _wrap_AtkObject__proxy_do_set_name;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_description");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_description")))
            klass->set_description = _wrap_AtkObject__proxy_do_set_description;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_parent");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_parent")))
            klass->set_parent = _wrap_AtkObject__proxy_do_set_parent;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_role");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_role")))
            klass->set_role = _wrap_AtkObject__proxy_do_set_role;
        Py_DECREF(o);
    }

    /* overriding do_connect_property_change_handler is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_remove_property_change_handler");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "remove_property_change_handler")))
            klass->remove_property_change_handler = _wrap_AtkObject__proxy_do_remove_property_change_handler;
        Py_DECREF(o);
    }

    /* overriding do_initialize is currently not supported */

    /* overriding do_children_changed is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_focus_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "focus_event")))
            klass->focus_event = _wrap_AtkObject__proxy_do_focus_event;
        Py_DECREF(o);
    }

    /* overriding do_property_change is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_state_change");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "state_change")))
            klass->state_change = _wrap_AtkObject__proxy_do_state_change;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_visible_data_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "visible_data_changed")))
            klass->visible_data_changed = _wrap_AtkObject__proxy_do_visible_data_changed;
        Py_DECREF(o);
    }

    /* overriding do_active_descendant_changed is currently not supported */
    return 0;
}


/* ----------- AtkGObjectAccessible ----------- */

static PyObject *
_wrap_atk_gobject_accessible_get_object(PyGObject *self)
{
    GObject *ret;

    
    ret = atk_gobject_accessible_get_object(ATK_GOBJECT_ACCESSIBLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyAtkGObjectAccessible_methods[] = {
    { "get_object", (PyCFunction)_wrap_atk_gobject_accessible_get_object, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkGObjectAccessible_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.GObjectAccessible",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkGObjectAccessible_methods, /* tp_methods */
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



/* ----------- AtkNoOpObject ----------- */

static int
_wrap_atk_no_op_object_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "obj", NULL };
    PyGObject *obj;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.NoOpObject.__init__", kwlist, &PyGObject_Type, &obj))
        return -1;
    self->obj = (GObject *)atk_no_op_object_new(G_OBJECT(obj->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create AtkNoOpObject object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

PyTypeObject G_GNUC_INTERNAL PyAtkNoOpObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.NoOpObject",                   /* tp_name */
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
    (initproc)_wrap_atk_no_op_object_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkObjectFactory ----------- */

static PyObject *
_wrap_atk_object_factory_create_accessible(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "obj", NULL };
    PyGObject *obj;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.ObjectFactory.create_accessible", kwlist, &PyGObject_Type, &obj))
        return NULL;
    
    ret = atk_object_factory_create_accessible(ATK_OBJECT_FACTORY(self->obj), G_OBJECT(obj->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_object_factory_invalidate(PyGObject *self)
{
    
    atk_object_factory_invalidate(ATK_OBJECT_FACTORY(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_object_factory_get_accessible_type(PyGObject *self)
{
    GType ret;

    
    ret = atk_object_factory_get_accessible_type(ATK_OBJECT_FACTORY(self->obj));
    
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_AtkObjectFactory__do_invalidate(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.ObjectFactory.invalidate", kwlist, &PyAtkObjectFactory_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (ATK_OBJECT_FACTORY_CLASS(klass)->invalidate)
        ATK_OBJECT_FACTORY_CLASS(klass)->invalidate(ATK_OBJECT_FACTORY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Atk.ObjectFactory.invalidate not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkObjectFactory_methods[] = {
    { "create_accessible", (PyCFunction)_wrap_atk_object_factory_create_accessible, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "invalidate", (PyCFunction)_wrap_atk_object_factory_invalidate, METH_NOARGS,
      NULL },
    { "get_accessible_type", (PyCFunction)_wrap_atk_object_factory_get_accessible_type, METH_NOARGS,
      NULL },
    { "do_invalidate", (PyCFunction)_wrap_AtkObjectFactory__do_invalidate, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkObjectFactory_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.ObjectFactory",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkObjectFactory_methods, /* tp_methods */
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
_wrap_AtkObjectFactory__proxy_do_invalidate(AtkObjectFactory *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_invalidate");
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
__AtkObjectFactory_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    AtkObjectFactoryClass *klass = ATK_OBJECT_FACTORY_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_invalidate");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "invalidate")))
            klass->invalidate = _wrap_AtkObjectFactory__proxy_do_invalidate;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- AtkNoOpObjectFactory ----------- */

static int
_wrap_atk_no_op_object_factory_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":atk.NoOpObjectFactory.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create atk.NoOpObjectFactory object");
        return -1;
    }
    return 0;
}

PyTypeObject G_GNUC_INTERNAL PyAtkNoOpObjectFactory_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.NoOpObjectFactory",                   /* tp_name */
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
    (initproc)_wrap_atk_no_op_object_factory_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkRegistry ----------- */

static PyObject *
_wrap_atk_registry_set_factory_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "factory_type", NULL };
    PyObject *py_type = NULL, *py_factory_type = NULL;
    GType type, factory_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Atk.Registry.set_factory_type", kwlist, &py_type, &py_factory_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    if ((factory_type = pyg_type_from_object(py_factory_type)) == 0)
        return NULL;
    
    atk_registry_set_factory_type(ATK_REGISTRY(self->obj), type, factory_type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_registry_get_factory_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GType type, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.Registry.get_factory_type", kwlist, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    
    ret = atk_registry_get_factory_type(ATK_REGISTRY(self->obj), type);
    
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_atk_registry_get_factory(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GType type;
    AtkObjectFactory *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.Registry.get_factory", kwlist, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    
    ret = atk_registry_get_factory(ATK_REGISTRY(self->obj), type);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyAtkRegistry_methods[] = {
    { "set_factory_type", (PyCFunction)_wrap_atk_registry_set_factory_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_factory_type", (PyCFunction)_wrap_atk_registry_get_factory_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_factory", (PyCFunction)_wrap_atk_registry_get_factory, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkRegistry_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Registry",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkRegistry_methods, /* tp_methods */
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



/* ----------- AtkRelation ----------- */

#line 56 "atk.override"
static int
_wrap_atk_relation_new (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "targets", "relationship", NULL };
    AtkObject **targets;
    int relationship, count, i;
    PyObject *py_targets;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "Oi:relation_new", kwlist,
				     &py_targets, &relationship))
	return -1;

    if (!PySequence_Check(py_targets)) {
	PyErr_SetString(PyExc_TypeError,
                        "targets argument must be a non-empty sequence");
	return -1;
    }

    if ((count = PySequence_Length(py_targets)) == 0) {
	PyErr_SetString(PyExc_ValueError,
                        "targets argument must be a non-empty sequence");
	return -1;
    }
    targets = g_new(AtkObject *, count);
    for (i = 0; i < count; i++) {
	PyObject *item = PySequence_GetItem(py_targets, i);
	Py_DECREF(item); /* PySequence_GetItem INCREF's */
	if (!pygobject_check(item, &PyAtkObject_Type)) {
	    PyErr_SetString(PyExc_TypeError, "targets argument must be a sequence of AtkObjects.");
	    g_free(targets);
	    return -1;
	}
	    
	targets[i] = (AtkObject *) pygobject_get(item);
    }

    self->obj = (GObject *) atk_relation_new(targets, count, relationship);
    
    g_free(targets);

    pygobject_register_wrapper((PyObject *) self);
    return 0;
}
#line 4137 "atk.c"


static PyObject *
_wrap_atk_relation_get_relation_type(PyGObject *self)
{
    gint ret;

    
    ret = atk_relation_get_relation_type(ATK_RELATION(self->obj));
    
    return pyg_enum_from_gtype(ATK_TYPE_RELATION_TYPE, ret);
}

#line 256 "atk.override"
static PyObject *
_wrap_atk_relation_get_target (PyGObject *self)
{
    GPtrArray * trgtarray;
    AtkObject **targets;
    int i;
    PyObject *py_targets;
    
    trgtarray = atk_relation_get_target(ATK_RELATION(self->obj));
    py_targets = PyList_New(trgtarray->len);
    targets = (AtkObject**)trgtarray->pdata;
    for (i = 0; i < trgtarray->len; i++) {
	PyObject *item = pygobject_new((GObject*)targets[i]);
	PyList_SetItem(py_targets, i, item);
    }
    return py_targets;
}
#line 4169 "atk.c"


static PyObject *
_wrap_atk_relation_add_target(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "target", NULL };
    PyGObject *target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Relation.add_target", kwlist, &PyAtkObject_Type, &target))
        return NULL;
    
    atk_relation_add_target(ATK_RELATION(self->obj), ATK_OBJECT(target->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkRelation_methods[] = {
    { "get_relation_type", (PyCFunction)_wrap_atk_relation_get_relation_type, METH_NOARGS,
      NULL },
    { "get_target", (PyCFunction)_wrap_atk_relation_get_target, METH_NOARGS,
      NULL },
    { "add_target", (PyCFunction)_wrap_atk_relation_add_target, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkRelation_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Relation",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkRelation_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_atk_relation_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkRelationSet ----------- */

static int
_wrap_atk_relation_set_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":atk.RelationSet.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create atk.RelationSet object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_atk_relation_set_contains(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.RelationSet.contains", kwlist, &py_relationship))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    ret = atk_relation_set_contains(ATK_RELATION_SET(self->obj), relationship);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_relation_set_remove(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relation", NULL };
    PyGObject *relation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.RelationSet.remove", kwlist, &PyAtkRelation_Type, &relation))
        return NULL;
    
    atk_relation_set_remove(ATK_RELATION_SET(self->obj), ATK_RELATION(relation->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_relation_set_add(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relation", NULL };
    PyGObject *relation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.RelationSet.add", kwlist, &PyAtkRelation_Type, &relation))
        return NULL;
    
    atk_relation_set_add(ATK_RELATION_SET(self->obj), ATK_RELATION(relation->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_relation_set_get_n_relations(PyGObject *self)
{
    int ret;

    
    ret = atk_relation_set_get_n_relations(ATK_RELATION_SET(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_relation_set_get_relation(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    AtkRelation *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.RelationSet.get_relation", kwlist, &i))
        return NULL;
    
    ret = atk_relation_set_get_relation(ATK_RELATION_SET(self->obj), i);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_relation_set_get_relation_by_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    AtkRelation *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.RelationSet.get_relation_by_type", kwlist, &py_relationship))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    ret = atk_relation_set_get_relation_by_type(ATK_RELATION_SET(self->obj), relationship);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_relation_set_add_relation_by_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", "target", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    PyGObject *target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!:Atk.RelationSet.add_relation_by_type", kwlist, &py_relationship, &PyAtkObject_Type, &target))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    atk_relation_set_add_relation_by_type(ATK_RELATION_SET(self->obj), relationship, ATK_OBJECT(target->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkRelationSet_methods[] = {
    { "contains", (PyCFunction)_wrap_atk_relation_set_contains, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove", (PyCFunction)_wrap_atk_relation_set_remove, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add", (PyCFunction)_wrap_atk_relation_set_add, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_n_relations", (PyCFunction)_wrap_atk_relation_set_get_n_relations, METH_NOARGS,
      NULL },
    { "get_relation", (PyCFunction)_wrap_atk_relation_set_get_relation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_relation_by_type", (PyCFunction)_wrap_atk_relation_set_get_relation_by_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_relation_by_type", (PyCFunction)_wrap_atk_relation_set_add_relation_by_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkRelationSet_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.RelationSet",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkRelationSet_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_atk_relation_set_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkStateSet ----------- */

static int
_wrap_atk_state_set_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":atk.StateSet.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create atk.StateSet object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_atk_state_set_is_empty(PyGObject *self)
{
    int ret;

    
    ret = atk_state_set_is_empty(ATK_STATE_SET(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_state_set_add_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    AtkStateType type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.StateSet.add_state", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    ret = atk_state_set_add_state(ATK_STATE_SET(self->obj), type);
    
    return PyBool_FromLong(ret);

}

#line 275 "atk.override"
static PyObject *
_wrap_atk_state_set_add_states(PyGObject *self, PyObject *args,
                               PyObject *kwargs)
{
    static char *kwlist[] = { "types", NULL };
    AtkStateType *types;
    gint count, i;
    PyObject *py_types;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.StateSet.add_states",
                                     kwlist, &py_types))
	return NULL;

    if (!(py_types = PySequence_Fast(py_types, "types must be a sequence")))
        return NULL;

    count = PySequence_Fast_GET_SIZE(py_types);
    types = g_new0(AtkStateType, count);
    for (i = 0; i < count; i++) {
        if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE,
                               PySequence_Fast_GET_ITEM(py_types, i),
                               (gpointer)&types[i])) {
            Py_DECREF(py_types);
            return NULL;
        }
    }
    atk_state_set_add_states(ATK_STATE_SET(self->obj), types, count);

    Py_DECREF(py_types);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 4532 "atk.c"


static PyObject *
_wrap_atk_state_set_clear_states(PyGObject *self)
{
    
    atk_state_set_clear_states(ATK_STATE_SET(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_state_set_contains_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    AtkStateType type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.StateSet.contains_state", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    ret = atk_state_set_contains_state(ATK_STATE_SET(self->obj), type);
    
    return PyBool_FromLong(ret);

}

#line 310 "atk.override"
static PyObject *
_wrap_atk_state_set_contains_states(PyGObject *self, PyObject *args,
                               PyObject *kwargs)
{
    static char *kwlist[] = { "types", NULL };
    AtkStateType *types;
    gint count, i;
    gboolean ret;
    PyObject *py_types, *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.StateSet.contains_states",
                                     kwlist, &py_types))
	return NULL;

    if (!(py_types = PySequence_Fast(py_types, "types must be a sequence")))
        return NULL;

    count = PySequence_Fast_GET_SIZE(py_types);
    types = g_new0(AtkStateType, count);
    for (i = 0; i < count; i++) {
        if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE,
                               PySequence_Fast_GET_ITEM(py_types, i),
                               (gpointer)&types[i])) {
            Py_DECREF(py_types);
            return NULL;
        }
    }
    ret = atk_state_set_contains_states(ATK_STATE_SET(self->obj), types,
                                        count);
    if (ret)
        py_ret = Py_True;
    else
        py_ret = Py_False;
    
    Py_DECREF(py_types);
    Py_INCREF(py_ret);
    return py_ret;
}
#line 4604 "atk.c"


static PyObject *
_wrap_atk_state_set_remove_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    AtkStateType type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.StateSet.remove_state", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    ret = atk_state_set_remove_state(ATK_STATE_SET(self->obj), type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_state_set_and_sets(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "compare_set", NULL };
    PyGObject *compare_set;
    AtkStateSet *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.StateSet.and_sets", kwlist, &PyAtkStateSet_Type, &compare_set))
        return NULL;
    
    ret = atk_state_set_and_sets(ATK_STATE_SET(self->obj), ATK_STATE_SET(compare_set->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_state_set_or_sets(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "compare_set", NULL };
    PyGObject *compare_set;
    AtkStateSet *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.StateSet.or_sets", kwlist, &PyAtkStateSet_Type, &compare_set))
        return NULL;
    
    ret = atk_state_set_or_sets(ATK_STATE_SET(self->obj), ATK_STATE_SET(compare_set->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_state_set_xor_sets(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "compare_set", NULL };
    PyGObject *compare_set;
    AtkStateSet *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.StateSet.xor_sets", kwlist, &PyAtkStateSet_Type, &compare_set))
        return NULL;
    
    ret = atk_state_set_xor_sets(ATK_STATE_SET(self->obj), ATK_STATE_SET(compare_set->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyAtkStateSet_methods[] = {
    { "is_empty", (PyCFunction)_wrap_atk_state_set_is_empty, METH_NOARGS,
      NULL },
    { "add_state", (PyCFunction)_wrap_atk_state_set_add_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_states", (PyCFunction)_wrap_atk_state_set_add_states, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_states", (PyCFunction)_wrap_atk_state_set_clear_states, METH_NOARGS,
      NULL },
    { "contains_state", (PyCFunction)_wrap_atk_state_set_contains_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "contains_states", (PyCFunction)_wrap_atk_state_set_contains_states, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_state", (PyCFunction)_wrap_atk_state_set_remove_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "and_sets", (PyCFunction)_wrap_atk_state_set_and_sets, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "or_sets", (PyCFunction)_wrap_atk_state_set_or_sets, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "xor_sets", (PyCFunction)_wrap_atk_state_set_xor_sets, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkStateSet_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.StateSet",                   /* tp_name */
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
    (struct PyMethodDef*)_PyAtkStateSet_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_atk_state_set_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkUtil ----------- */

PyTypeObject G_GNUC_INTERNAL PyAtkUtil_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Util",                   /* tp_name */
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



/* ----------- AtkAction ----------- */

static PyObject *
_wrap_atk_action_do_action(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Action.do_action", kwlist, &i))
        return NULL;
    
    ret = atk_action_do_action(ATK_ACTION(self->obj), i);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_action_get_n_actions(PyGObject *self)
{
    int ret;

    
    ret = atk_action_get_n_actions(ATK_ACTION(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_action_get_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Action.get_description", kwlist, &i))
        return NULL;
    
    ret = atk_action_get_description(ATK_ACTION(self->obj), i);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_action_get_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Action.get_name", kwlist, &i))
        return NULL;
    
    ret = atk_action_get_name(ATK_ACTION(self->obj), i);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_action_get_keybinding(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Action.get_keybinding", kwlist, &i))
        return NULL;
    
    ret = atk_action_get_keybinding(ATK_ACTION(self->obj), i);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_action_set_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", "desc", NULL };
    int i, ret;
    char *desc;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"is:Atk.Action.set_description", kwlist, &i, &desc))
        return NULL;
    
    ret = atk_action_set_description(ATK_ACTION(self->obj), i, desc);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_action_get_localized_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Action.get_localized_name", kwlist, &i))
        return NULL;
    
    ret = atk_action_get_localized_name(ATK_ACTION(self->obj), i);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkAction__do_do_action(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Action.do_action", kwlist, &PyAtkAction_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->do_action)
        ret = iface->do_action(ATK_ACTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.do_action not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkAction__do_get_n_actions(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Action.get_n_actions", kwlist, &PyAtkAction_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->get_n_actions)
        ret = iface->get_n_actions(ATK_ACTION(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.get_n_actions not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkAction__do_get_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Action.get_description", kwlist, &PyAtkAction_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->get_description)
        ret = iface->get_description(ATK_ACTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.get_description not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkAction__do_get_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Action.get_name", kwlist, &PyAtkAction_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->get_name)
        ret = iface->get_name(ATK_ACTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.get_name not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkAction__do_get_keybinding(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Action.get_keybinding", kwlist, &PyAtkAction_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->get_keybinding)
        ret = iface->get_keybinding(ATK_ACTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.get_keybinding not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkAction__do_set_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", "desc", NULL };
    PyGObject *self;
    int i, ret;
    char *desc;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!is:Atk.Action.set_description", kwlist, &PyAtkAction_Type, &self, &i, &desc))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->set_description)
        ret = iface->set_description(ATK_ACTION(self->obj), i, desc);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.set_description not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkAction__do_get_localized_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkActionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Action.get_localized_name", kwlist, &PyAtkAction_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_ACTION);
    if (iface->get_localized_name)
        ret = iface->get_localized_name(ATK_ACTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Action.get_localized_name not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkAction_methods[] = {
    { "do_action", (PyCFunction)_wrap_atk_action_do_action, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_n_actions", (PyCFunction)_wrap_atk_action_get_n_actions, METH_NOARGS,
      NULL },
    { "get_description", (PyCFunction)_wrap_atk_action_get_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_name", (PyCFunction)_wrap_atk_action_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_keybinding", (PyCFunction)_wrap_atk_action_get_keybinding, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_description", (PyCFunction)_wrap_atk_action_set_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_localized_name", (PyCFunction)_wrap_atk_action_get_localized_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_do_action", (PyCFunction)_wrap_AtkAction__do_do_action, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_actions", (PyCFunction)_wrap_AtkAction__do_get_n_actions, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_description", (PyCFunction)_wrap_AtkAction__do_get_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_name", (PyCFunction)_wrap_AtkAction__do_get_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_keybinding", (PyCFunction)_wrap_AtkAction__do_get_keybinding, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_description", (PyCFunction)_wrap_AtkAction__do_set_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_localized_name", (PyCFunction)_wrap_AtkAction__do_get_localized_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkAction_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Action",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkAction_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gboolean
_wrap_AtkAction__proxy_do_do_action(AtkAction *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_do_action");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkAction__proxy_do_get_n_actions(AtkAction *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_actions");
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
static const gchar*
_wrap_AtkAction__proxy_do_get_description(AtkAction *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    const gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_description");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static const gchar*
_wrap_AtkAction__proxy_do_get_name(AtkAction *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    const gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_name");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static const gchar*
_wrap_AtkAction__proxy_do_get_keybinding(AtkAction *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    const gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_keybinding");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkAction__proxy_do_set_description(AtkAction *self, gint i, const gchar*desc)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    PyObject *py_desc = NULL;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_i = PyInt_FromLong(i);
    if (desc)
        py_desc = PyString_FromString(desc);
    if (!py_desc) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_i);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    PyTuple_SET_ITEM(py_args, 1, py_desc);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_description");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static const gchar*
_wrap_AtkAction__proxy_do_get_localized_name(AtkAction *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    const gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_localized_name");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__AtkAction__interface_init(AtkActionIface *iface, PyTypeObject *pytype)
{
    AtkActionIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_do_action") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->do_action = _wrap_AtkAction__proxy_do_do_action;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->do_action = parent_iface->do_action;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_actions") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_actions = _wrap_AtkAction__proxy_do_get_n_actions;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_actions = parent_iface->get_n_actions;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_description = _wrap_AtkAction__proxy_do_get_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_description = parent_iface->get_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_name") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_name = _wrap_AtkAction__proxy_do_get_name;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_name = parent_iface->get_name;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_keybinding") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_keybinding = _wrap_AtkAction__proxy_do_get_keybinding;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_keybinding = parent_iface->get_keybinding;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_description = _wrap_AtkAction__proxy_do_set_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_description = parent_iface->set_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_localized_name") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_localized_name = _wrap_AtkAction__proxy_do_get_localized_name;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_localized_name = parent_iface->get_localized_name;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkAction__iinfo = {
    (GInterfaceInitFunc) __AtkAction__interface_init,
    NULL,
    NULL
};


/* ----------- AtkComponent ----------- */

static PyObject *
_wrap_atk_component_contains(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "coord_type", NULL };
    int x, y, ret;
    AtkCoordType coord_type;
    PyObject *py_coord_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiO:Atk.Component.contains", kwlist, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    
    ret = atk_component_contains(ATK_COMPONENT(self->obj), x, y, coord_type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_component_ref_accessible_at_point(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "coord_type", NULL };
    int x, y;
    AtkCoordType coord_type;
    AtkObject *ret;
    PyObject *py_coord_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiO:Atk.Component.ref_accessible_at_point", kwlist, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    
    ret = atk_component_ref_accessible_at_point(ATK_COMPONENT(self->obj), x, y, coord_type);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 102 "atk.override"
static PyObject *
_wrap_atk_component_get_extents(PyGObject *self, PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "coord_type", NULL };
    gint x, y, width, height;
    AtkCoordType coord_type;
    PyObject *py_coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.Component.get_extents", kwlist,
				     &py_coord_type))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type,
                           (gint *)&coord_type))
        return NULL;
    atk_component_get_extents(ATK_COMPONENT(self->obj), &x, &y, &width,
                              &height, coord_type);
    return Py_BuildValue("(iiii)", x, y, width, height);
}
#line 5762 "atk.c"


#line 125 "atk.override"
static PyObject *
_wrap_atk_component_get_position(PyGObject *self, PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "coord_type", NULL };
    gint x, y;
    AtkCoordType coord_type;
    PyObject *py_coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.Component.get_position", kwlist,
				     &py_coord_type))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type,
                           (gint *)&coord_type))
        return NULL;
    atk_component_get_position(ATK_COMPONENT(self->obj), &x, &y, coord_type);
    return Py_BuildValue("(ii)", x, y);
}
#line 5786 "atk.c"


#line 147 "atk.override"
static PyObject *
_wrap_atk_component_get_size(PyGObject *self)
{
    gint width, height;

    atk_component_get_size(ATK_COMPONENT(self->obj), &width, &height);
    return Py_BuildValue("(ii)", width, height);
}
#line 5798 "atk.c"


static PyObject *
_wrap_atk_component_grab_focus(PyGObject *self)
{
    int ret;

    
    ret = atk_component_grab_focus(ATK_COMPONENT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_component_remove_focus_handler(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Atk.Component.remove_focus_handler", kwlist, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    atk_component_remove_focus_handler(ATK_COMPONENT(self->obj), handler_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_component_set_extents(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", "coord_type", NULL };
    int x, y, width, height, ret;
    AtkCoordType coord_type;
    PyObject *py_coord_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiiiO:Atk.Component.set_extents", kwlist, &x, &y, &width, &height, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    
    ret = atk_component_set_extents(ATK_COMPONENT(self->obj), x, y, width, height, coord_type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_component_set_position(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "coord_type", NULL };
    int x, y, ret;
    AtkCoordType coord_type;
    PyObject *py_coord_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiO:Atk.Component.set_position", kwlist, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    
    ret = atk_component_set_position(ATK_COMPONENT(self->obj), x, y, coord_type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_component_set_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", "height", NULL };
    int width, height, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Component.set_size", kwlist, &width, &height))
        return NULL;
    
    ret = atk_component_set_size(ATK_COMPONENT(self->obj), width, height);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_component_get_layer(PyGObject *self)
{
    gint ret;

    
    ret = atk_component_get_layer(ATK_COMPONENT(self->obj));
    
    return pyg_enum_from_gtype(ATK_TYPE_LAYER, ret);
}

static PyObject *
_wrap_atk_component_get_mdi_zorder(PyGObject *self)
{
    int ret;

    
    ret = atk_component_get_mdi_zorder(ATK_COMPONENT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_component_get_alpha(PyGObject *self)
{
    double ret;

    
    ret = atk_component_get_alpha(ATK_COMPONENT(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_AtkComponent__do_contains(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "x", "y", "coord_type", NULL };
    PyGObject *self;
    int x, y, ret;
    PyObject *py_coord_type = NULL;
    AtkCoordType coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiO:Atk.Component.contains", kwlist, &PyAtkComponent_Type, &self, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->contains)
        ret = iface->contains(ATK_COMPONENT(self->obj), x, y, coord_type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.contains not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkComponent__do_ref_accessible_at_point(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "x", "y", "coord_type", NULL };
    PyGObject *self;
    int x, y;
    AtkObject *ret;
    PyObject *py_coord_type = NULL;
    AtkCoordType coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiO:Atk.Component.ref_accessible_at_point", kwlist, &PyAtkComponent_Type, &self, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->ref_accessible_at_point)
        ret = iface->ref_accessible_at_point(ATK_COMPONENT(self->obj), x, y, coord_type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.ref_accessible_at_point not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkComponent__do_grab_focus(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Component.grab_focus", kwlist, &PyAtkComponent_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->grab_focus)
        ret = iface->grab_focus(ATK_COMPONENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.grab_focus not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkComponent__do_remove_focus_handler(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "handler_id", NULL };
    PyGObject *self;
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Atk.Component.remove_focus_handler", kwlist, &PyAtkComponent_Type, &self, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->remove_focus_handler)
        iface->remove_focus_handler(ATK_COMPONENT(self->obj), handler_id);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.remove_focus_handler not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkComponent__do_set_extents(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "x", "y", "width", "height", "coord_type", NULL };
    PyGObject *self;
    int x, y, width, height, ret;
    PyObject *py_coord_type = NULL;
    AtkCoordType coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiiiO:Atk.Component.set_extents", kwlist, &PyAtkComponent_Type, &self, &x, &y, &width, &height, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->set_extents)
        ret = iface->set_extents(ATK_COMPONENT(self->obj), x, y, width, height, coord_type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.set_extents not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkComponent__do_set_position(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "x", "y", "coord_type", NULL };
    PyGObject *self;
    int x, y, ret;
    PyObject *py_coord_type = NULL;
    AtkCoordType coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiO:Atk.Component.set_position", kwlist, &PyAtkComponent_Type, &self, &x, &y, &py_coord_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type, (gpointer)&coord_type))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->set_position)
        ret = iface->set_position(ATK_COMPONENT(self->obj), x, y, coord_type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.set_position not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkComponent__do_set_size(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "width", "height", NULL };
    PyGObject *self;
    int width, height, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Component.set_size", kwlist, &PyAtkComponent_Type, &self, &width, &height))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->set_size)
        ret = iface->set_size(ATK_COMPONENT(self->obj), width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.set_size not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkComponent__do_get_layer(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Component.get_layer", kwlist, &PyAtkComponent_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->get_layer)
        ret = iface->get_layer(ATK_COMPONENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.get_layer not implemented");
        return NULL;
    }
    return pyg_enum_from_gtype(ATK_TYPE_LAYER, ret);
}

static PyObject *
_wrap_AtkComponent__do_get_mdi_zorder(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Component.get_mdi_zorder", kwlist, &PyAtkComponent_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->get_mdi_zorder)
        ret = iface->get_mdi_zorder(ATK_COMPONENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.get_mdi_zorder not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkComponent__do_bounds_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", "bounds", NULL };
    PyGObject *self;
    PyObject *py_bounds;
    AtkRectangle *bounds = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Atk.Component.bounds_changed", kwlist, &PyAtkComponent_Type, &self, &py_bounds))
        return NULL;
    if (pyg_boxed_check(py_bounds, ATK_TYPE_RECTANGLE))
        bounds = pyg_boxed_get(py_bounds, AtkRectangle);
    else {
        PyErr_SetString(PyExc_TypeError, "bounds should be a AtkRectangle");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->bounds_changed)
        iface->bounds_changed(ATK_COMPONENT(self->obj), bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.bounds_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkComponent__do_get_alpha(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkComponentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    double ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Component.get_alpha", kwlist, &PyAtkComponent_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_COMPONENT);
    if (iface->get_alpha)
        ret = iface->get_alpha(ATK_COMPONENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Component.get_alpha not implemented");
        return NULL;
    }
    return PyFloat_FromDouble(ret);
}

static const PyMethodDef _PyAtkComponent_methods[] = {
    { "contains", (PyCFunction)_wrap_atk_component_contains, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ref_accessible_at_point", (PyCFunction)_wrap_atk_component_ref_accessible_at_point, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_atk_component_get_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_position", (PyCFunction)_wrap_atk_component_get_position, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_size", (PyCFunction)_wrap_atk_component_get_size, METH_NOARGS,
      NULL },
    { "grab_focus", (PyCFunction)_wrap_atk_component_grab_focus, METH_NOARGS,
      NULL },
    { "remove_focus_handler", (PyCFunction)_wrap_atk_component_remove_focus_handler, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_extents", (PyCFunction)_wrap_atk_component_set_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_position", (PyCFunction)_wrap_atk_component_set_position, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_size", (PyCFunction)_wrap_atk_component_set_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_layer", (PyCFunction)_wrap_atk_component_get_layer, METH_NOARGS,
      NULL },
    { "get_mdi_zorder", (PyCFunction)_wrap_atk_component_get_mdi_zorder, METH_NOARGS,
      NULL },
    { "get_alpha", (PyCFunction)_wrap_atk_component_get_alpha, METH_NOARGS,
      NULL },
    { "do_contains", (PyCFunction)_wrap_AtkComponent__do_contains, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_ref_accessible_at_point", (PyCFunction)_wrap_AtkComponent__do_ref_accessible_at_point, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_grab_focus", (PyCFunction)_wrap_AtkComponent__do_grab_focus, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_focus_handler", (PyCFunction)_wrap_AtkComponent__do_remove_focus_handler, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_extents", (PyCFunction)_wrap_AtkComponent__do_set_extents, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_position", (PyCFunction)_wrap_AtkComponent__do_set_position, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_size", (PyCFunction)_wrap_AtkComponent__do_set_size, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_layer", (PyCFunction)_wrap_AtkComponent__do_get_layer, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_mdi_zorder", (PyCFunction)_wrap_AtkComponent__do_get_mdi_zorder, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_bounds_changed", (PyCFunction)_wrap_AtkComponent__do_bounds_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_alpha", (PyCFunction)_wrap_AtkComponent__do_get_alpha, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkComponent_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Component",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkComponent_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gboolean
_wrap_AtkComponent__proxy_do_contains(AtkComponent *self, gint x, gint y, AtkCoordType coord_type)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_coord_type;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_coord_type = pyg_enum_from_gtype(ATK_TYPE_COORD_TYPE, coord_type);
    if (!py_coord_type) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_coord_type);
    
    py_method = PyObject_GetAttrString(py_self, "do_contains");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkComponent__proxy_do_ref_accessible_at_point(AtkComponent *self, gint x, gint y, AtkCoordType coord_type)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_coord_type;
    AtkObject* retval;
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
    py_coord_type = pyg_enum_from_gtype(ATK_TYPE_COORD_TYPE, coord_type);
    if (!py_coord_type) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_coord_type);
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_accessible_at_point");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkComponent__proxy_do_grab_focus(AtkComponent *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_grab_focus");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkComponent__proxy_do_remove_focus_handler(AtkComponent *self, guint handler_id)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_handler_id;
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
    py_handler_id = PyInt_FromLong(handler_id);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_handler_id);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_focus_handler");
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
static gboolean
_wrap_AtkComponent__proxy_do_set_extents(AtkComponent *self, gint x, gint y, gint width, gint height, AtkCoordType coord_type)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_width;
    PyObject *py_height;
    PyObject *py_coord_type;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    py_coord_type = pyg_enum_from_gtype(ATK_TYPE_COORD_TYPE, coord_type);
    if (!py_coord_type) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_height);
        Py_DECREF(py_width);
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(5);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_width);
    PyTuple_SET_ITEM(py_args, 3, py_height);
    PyTuple_SET_ITEM(py_args, 4, py_coord_type);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_extents");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkComponent__proxy_do_set_position(AtkComponent *self, gint x, gint y, AtkCoordType coord_type)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_coord_type;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_coord_type = pyg_enum_from_gtype(ATK_TYPE_COORD_TYPE, coord_type);
    if (!py_coord_type) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_coord_type);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_position");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkComponent__proxy_do_set_size(AtkComponent *self, gint width, gint height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_width;
    PyObject *py_height;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_width);
    PyTuple_SET_ITEM(py_args, 1, py_height);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_size");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkLayer
_wrap_AtkComponent__proxy_do_get_layer(AtkComponent *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkLayer retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_layer");
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
    if (pyg_enum_get_value(ATK_TYPE_LAYER, py_retval, (gint *)&retval)) {
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
static gint
_wrap_AtkComponent__proxy_do_get_mdi_zorder(AtkComponent *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_mdi_zorder");
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
_wrap_AtkComponent__proxy_do_bounds_changed(AtkComponent *self, AtkRectangle*bounds)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_bounds;
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
    py_bounds = pyg_boxed_new(ATK_TYPE_RECTANGLE, bounds, FALSE, FALSE);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_bounds);
    
    py_method = PyObject_GetAttrString(py_self, "do_bounds_changed");
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
static gdouble
_wrap_AtkComponent__proxy_do_get_alpha(AtkComponent *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gdouble retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_alpha");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "d", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__AtkComponent__interface_init(AtkComponentIface *iface, PyTypeObject *pytype)
{
    AtkComponentIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_contains") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->contains = _wrap_AtkComponent__proxy_do_contains;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->contains = parent_iface->contains;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_ref_accessible_at_point") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->ref_accessible_at_point = _wrap_AtkComponent__proxy_do_ref_accessible_at_point;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->ref_accessible_at_point = parent_iface->ref_accessible_at_point;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_grab_focus") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->grab_focus = _wrap_AtkComponent__proxy_do_grab_focus;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->grab_focus = parent_iface->grab_focus;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_focus_handler") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_focus_handler = _wrap_AtkComponent__proxy_do_remove_focus_handler;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_focus_handler = parent_iface->remove_focus_handler;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_extents") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_extents = _wrap_AtkComponent__proxy_do_set_extents;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_extents = parent_iface->set_extents;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_position") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_position = _wrap_AtkComponent__proxy_do_set_position;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_position = parent_iface->set_position;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_size") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_size = _wrap_AtkComponent__proxy_do_set_size;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_size = parent_iface->set_size;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_layer") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_layer = _wrap_AtkComponent__proxy_do_get_layer;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_layer = parent_iface->get_layer;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_mdi_zorder") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_mdi_zorder = _wrap_AtkComponent__proxy_do_get_mdi_zorder;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_mdi_zorder = parent_iface->get_mdi_zorder;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_bounds_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->bounds_changed = _wrap_AtkComponent__proxy_do_bounds_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->bounds_changed = parent_iface->bounds_changed;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_alpha") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_alpha = _wrap_AtkComponent__proxy_do_get_alpha;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_alpha = parent_iface->get_alpha;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkComponent__iinfo = {
    (GInterfaceInitFunc) __AtkComponent__interface_init,
    NULL,
    NULL
};


/* ----------- AtkDocument ----------- */

static PyObject *
_wrap_atk_document_get_document_type(PyGObject *self)
{
    const gchar *ret;

    
    ret = atk_document_get_document_type(ATK_DOCUMENT(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_document_get_attribute_value(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute_name", NULL };
    char *attribute_name;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Atk.Document.get_attribute_value", kwlist, &attribute_name))
        return NULL;
    
    ret = atk_document_get_attribute_value(ATK_DOCUMENT(self->obj), attribute_name);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_document_set_attribute_value(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute_name", "attribute_value", NULL };
    char *attribute_name, *attribute_value;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:Atk.Document.set_attribute_value", kwlist, &attribute_name, &attribute_value))
        return NULL;
    
    ret = atk_document_set_attribute_value(ATK_DOCUMENT(self->obj), attribute_name, attribute_value);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkDocument__do_get_document_type(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkDocumentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Document.get_document_type", kwlist, &PyAtkDocument_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_DOCUMENT);
    if (iface->get_document_type)
        ret = iface->get_document_type(ATK_DOCUMENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Document.get_document_type not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkDocument__do_set_document_attribute(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkDocumentIface *iface;
    static char *kwlist[] = { "self", "attribute_name", "attribute_value", NULL };
    PyGObject *self;
    char *attribute_name, *attribute_value;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ss:Atk.Document.set_document_attribute", kwlist, &PyAtkDocument_Type, &self, &attribute_name, &attribute_value))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_DOCUMENT);
    if (iface->set_document_attribute)
        ret = iface->set_document_attribute(ATK_DOCUMENT(self->obj), attribute_name, attribute_value);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Document.set_document_attribute not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyAtkDocument_methods[] = {
    { "get_document_type", (PyCFunction)_wrap_atk_document_get_document_type, METH_NOARGS,
      NULL },
    { "get_attribute_value", (PyCFunction)_wrap_atk_document_get_attribute_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_value", (PyCFunction)_wrap_atk_document_set_attribute_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_document_type", (PyCFunction)_wrap_AtkDocument__do_get_document_type, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_document_attribute", (PyCFunction)_wrap_AtkDocument__do_set_document_attribute, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkDocument_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Document",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkDocument_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static const gchar*
_wrap_AtkDocument__proxy_do_get_document_type(AtkDocument *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_document_type");
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
static gboolean
_wrap_AtkDocument__proxy_do_set_document_attribute(AtkDocument *self, const gchar*attribute_name, const gchar*attribute_value)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_attribute_name = NULL;
    PyObject *py_attribute_value = NULL;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (attribute_name)
        py_attribute_name = PyString_FromString(attribute_name);
    if (!py_attribute_name) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (attribute_value)
        py_attribute_value = PyString_FromString(attribute_value);
    if (!py_attribute_value) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_attribute_name);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_attribute_name);
    PyTuple_SET_ITEM(py_args, 1, py_attribute_value);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_document_attribute");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__AtkDocument__interface_init(AtkDocumentIface *iface, PyTypeObject *pytype)
{
    AtkDocumentIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_document_type") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_document_type = _wrap_AtkDocument__proxy_do_get_document_type;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_document_type = parent_iface->get_document_type;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_document_attribute") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_document_attribute = _wrap_AtkDocument__proxy_do_set_document_attribute;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_document_attribute = parent_iface->set_document_attribute;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkDocument__iinfo = {
    (GInterfaceInitFunc) __AtkDocument__interface_init,
    NULL,
    NULL
};


/* ----------- AtkEditableText ----------- */

#line 157 "atk.override"
static PyObject *
_wrap_atk_editable_text_set_run_attributes(PyGObject *self, PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "attrib_set", "start_offset", "end_offset",
                              NULL };
    gint start, end, n_attrs, i;
    gboolean ret;
    AtkAttributeSet *attrib_set = NULL;
    PyObject *py_attrib_set;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "Oii:atk.EditableText.set_run_attributes",
                                     kwlist, &py_attrib_set, &start, &end))
	return NULL;

    
    if (!(py_attrib_set = PySequence_Fast(py_attrib_set,
                                          "attrib_set must be a sequence")))
        return NULL;

    n_attrs = PySequence_Fast_GET_SIZE(py_attrib_set);
    for (i = 0; i < n_attrs; i++) {
        PyObject *item = PySequence_Fast_GET_ITEM(py_attrib_set, i);
        AtkAttribute *attr = g_new0(AtkAttribute, 1);
        if (!PyArg_ParseTuple(item, "ss", &attr->name, &attr->value)) {
            PyErr_Clear();
            PyErr_SetString(PyExc_TypeError,
                            "attrib_set items should be (string,string)");
            g_free(attr);
            g_slist_foreach(attrib_set, (GFunc)g_free, NULL);
            g_slist_free(attrib_set);
            Py_DECREF(py_attrib_set);
            return NULL;
        }
        attrib_set = g_slist_append(attrib_set, attr);
    }
    ret = atk_editable_text_set_run_attributes(ATK_EDITABLE_TEXT(self->obj),
                                               attrib_set, start, end);
    
    g_slist_foreach(attrib_set, (GFunc)g_free, NULL);
    g_slist_free(attrib_set);
    Py_DECREF(py_attrib_set);
    return PyBool_FromLong(ret);
}
#line 7547 "atk.c"


static PyObject *
_wrap_atk_editable_text_set_text_contents(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Atk.EditableText.set_text_contents", kwlist, &string))
        return NULL;
    
    atk_editable_text_set_text_contents(ATK_EDITABLE_TEXT(self->obj), string);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 204 "atk.override"
static PyObject *
_wrap_atk_editable_text_insert_text(PyGObject *self, PyObject *args,
                                    PyObject *kwargs)
{
    static char *kwlist[] = { "string", "length", "position",
                              NULL };
    gint length, position;
    const gchar *string;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sii:atk.EditableText.insert_text",
                                     kwlist, &string, &length, &position))
	return NULL;

    atk_editable_text_insert_text(ATK_EDITABLE_TEXT(self->obj),
                                  string, length, &position);
    return PyInt_FromLong(position);
}
#line 7584 "atk.c"


static PyObject *
_wrap_atk_editable_text_copy_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_pos", "end_pos", NULL };
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.EditableText.copy_text", kwlist, &start_pos, &end_pos))
        return NULL;
    
    atk_editable_text_copy_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_editable_text_cut_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_pos", "end_pos", NULL };
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.EditableText.cut_text", kwlist, &start_pos, &end_pos))
        return NULL;
    
    atk_editable_text_cut_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_editable_text_delete_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_pos", "end_pos", NULL };
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.EditableText.delete_text", kwlist, &start_pos, &end_pos))
        return NULL;
    
    atk_editable_text_delete_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_editable_text_paste_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "position", NULL };
    int position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.EditableText.paste_text", kwlist, &position))
        return NULL;
    
    atk_editable_text_paste_text(ATK_EDITABLE_TEXT(self->obj), position);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkEditableText__do_set_text_contents(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkEditableTextIface *iface;
    static char *kwlist[] = { "self", "string", NULL };
    PyGObject *self;
    char *string;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:Atk.EditableText.set_text_contents", kwlist, &PyAtkEditableText_Type, &self, &string))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_EDITABLE_TEXT);
    if (iface->set_text_contents)
        iface->set_text_contents(ATK_EDITABLE_TEXT(self->obj), string);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.EditableText.set_text_contents not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkEditableText__do_copy_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkEditableTextIface *iface;
    static char *kwlist[] = { "self", "start_pos", "end_pos", NULL };
    PyGObject *self;
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.EditableText.copy_text", kwlist, &PyAtkEditableText_Type, &self, &start_pos, &end_pos))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_EDITABLE_TEXT);
    if (iface->copy_text)
        iface->copy_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.EditableText.copy_text not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkEditableText__do_cut_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkEditableTextIface *iface;
    static char *kwlist[] = { "self", "start_pos", "end_pos", NULL };
    PyGObject *self;
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.EditableText.cut_text", kwlist, &PyAtkEditableText_Type, &self, &start_pos, &end_pos))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_EDITABLE_TEXT);
    if (iface->cut_text)
        iface->cut_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.EditableText.cut_text not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkEditableText__do_delete_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkEditableTextIface *iface;
    static char *kwlist[] = { "self", "start_pos", "end_pos", NULL };
    PyGObject *self;
    int start_pos, end_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.EditableText.delete_text", kwlist, &PyAtkEditableText_Type, &self, &start_pos, &end_pos))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_EDITABLE_TEXT);
    if (iface->delete_text)
        iface->delete_text(ATK_EDITABLE_TEXT(self->obj), start_pos, end_pos);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.EditableText.delete_text not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkEditableText__do_paste_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkEditableTextIface *iface;
    static char *kwlist[] = { "self", "position", NULL };
    PyGObject *self;
    int position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.EditableText.paste_text", kwlist, &PyAtkEditableText_Type, &self, &position))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_EDITABLE_TEXT);
    if (iface->paste_text)
        iface->paste_text(ATK_EDITABLE_TEXT(self->obj), position);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.EditableText.paste_text not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkEditableText_methods[] = {
    { "set_run_attributes", (PyCFunction)_wrap_atk_editable_text_set_run_attributes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_text_contents", (PyCFunction)_wrap_atk_editable_text_set_text_contents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "insert_text", (PyCFunction)_wrap_atk_editable_text_insert_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy_text", (PyCFunction)_wrap_atk_editable_text_copy_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "cut_text", (PyCFunction)_wrap_atk_editable_text_cut_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "delete_text", (PyCFunction)_wrap_atk_editable_text_delete_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "paste_text", (PyCFunction)_wrap_atk_editable_text_paste_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_set_text_contents", (PyCFunction)_wrap_AtkEditableText__do_set_text_contents, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_copy_text", (PyCFunction)_wrap_AtkEditableText__do_copy_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_cut_text", (PyCFunction)_wrap_AtkEditableText__do_cut_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_delete_text", (PyCFunction)_wrap_AtkEditableText__do_delete_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_paste_text", (PyCFunction)_wrap_AtkEditableText__do_paste_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkEditableText_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.EditableText",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkEditableText_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_AtkEditableText__proxy_do_set_text_contents(AtkEditableText *self, const gchar*string)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_string = NULL;
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
    if (string)
        py_string = PyString_FromString(string);
    if (!py_string) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_string);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_text_contents");
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
_wrap_AtkEditableText__proxy_do_copy_text(AtkEditableText *self, gint start_pos, gint end_pos)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_start_pos;
    PyObject *py_end_pos;
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
    py_start_pos = PyInt_FromLong(start_pos);
    py_end_pos = PyInt_FromLong(end_pos);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_start_pos);
    PyTuple_SET_ITEM(py_args, 1, py_end_pos);
    
    py_method = PyObject_GetAttrString(py_self, "do_copy_text");
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
_wrap_AtkEditableText__proxy_do_cut_text(AtkEditableText *self, gint start_pos, gint end_pos)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_start_pos;
    PyObject *py_end_pos;
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
    py_start_pos = PyInt_FromLong(start_pos);
    py_end_pos = PyInt_FromLong(end_pos);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_start_pos);
    PyTuple_SET_ITEM(py_args, 1, py_end_pos);
    
    py_method = PyObject_GetAttrString(py_self, "do_cut_text");
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
_wrap_AtkEditableText__proxy_do_delete_text(AtkEditableText *self, gint start_pos, gint end_pos)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_start_pos;
    PyObject *py_end_pos;
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
    py_start_pos = PyInt_FromLong(start_pos);
    py_end_pos = PyInt_FromLong(end_pos);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_start_pos);
    PyTuple_SET_ITEM(py_args, 1, py_end_pos);
    
    py_method = PyObject_GetAttrString(py_self, "do_delete_text");
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
_wrap_AtkEditableText__proxy_do_paste_text(AtkEditableText *self, gint position)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_position;
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
    py_position = PyInt_FromLong(position);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_position);
    
    py_method = PyObject_GetAttrString(py_self, "do_paste_text");
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
__AtkEditableText__interface_init(AtkEditableTextIface *iface, PyTypeObject *pytype)
{
    AtkEditableTextIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_text_contents") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_text_contents = _wrap_AtkEditableText__proxy_do_set_text_contents;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_text_contents = parent_iface->set_text_contents;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_copy_text") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->copy_text = _wrap_AtkEditableText__proxy_do_copy_text;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->copy_text = parent_iface->copy_text;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_cut_text") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->cut_text = _wrap_AtkEditableText__proxy_do_cut_text;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->cut_text = parent_iface->cut_text;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_delete_text") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->delete_text = _wrap_AtkEditableText__proxy_do_delete_text;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->delete_text = parent_iface->delete_text;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_paste_text") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->paste_text = _wrap_AtkEditableText__proxy_do_paste_text;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->paste_text = parent_iface->paste_text;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkEditableText__iinfo = {
    (GInterfaceInitFunc) __AtkEditableText__interface_init,
    NULL,
    NULL
};


/* ----------- AtkHypertext ----------- */

static PyObject *
_wrap_atk_hypertext_get_link(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "link_index", NULL };
    int link_index;
    AtkHyperlink *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Hypertext.get_link", kwlist, &link_index))
        return NULL;
    
    ret = atk_hypertext_get_link(ATK_HYPERTEXT(self->obj), link_index);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_hypertext_get_n_links(PyGObject *self)
{
    int ret;

    
    ret = atk_hypertext_get_n_links(ATK_HYPERTEXT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_hypertext_get_link_index(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "char_index", NULL };
    int char_index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Hypertext.get_link_index", kwlist, &char_index))
        return NULL;
    
    ret = atk_hypertext_get_link_index(ATK_HYPERTEXT(self->obj), char_index);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHypertext__do_get_link(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkHypertextIface *iface;
    static char *kwlist[] = { "self", "link_index", NULL };
    PyGObject *self;
    int link_index;
    AtkHyperlink *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Hypertext.get_link", kwlist, &PyAtkHypertext_Type, &self, &link_index))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_HYPERTEXT);
    if (iface->get_link)
        ret = iface->get_link(ATK_HYPERTEXT(self->obj), link_index);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Hypertext.get_link not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkHypertext__do_get_n_links(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkHypertextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Hypertext.get_n_links", kwlist, &PyAtkHypertext_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_HYPERTEXT);
    if (iface->get_n_links)
        ret = iface->get_n_links(ATK_HYPERTEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Hypertext.get_n_links not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHypertext__do_get_link_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkHypertextIface *iface;
    static char *kwlist[] = { "self", "char_index", NULL };
    PyGObject *self;
    int char_index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Hypertext.get_link_index", kwlist, &PyAtkHypertext_Type, &self, &char_index))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_HYPERTEXT);
    if (iface->get_link_index)
        ret = iface->get_link_index(ATK_HYPERTEXT(self->obj), char_index);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Hypertext.get_link_index not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkHypertext__do_link_selected(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkHypertextIface *iface;
    static char *kwlist[] = { "self", "link_index", NULL };
    PyGObject *self;
    int link_index;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Hypertext.link_selected", kwlist, &PyAtkHypertext_Type, &self, &link_index))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_HYPERTEXT);
    if (iface->link_selected)
        iface->link_selected(ATK_HYPERTEXT(self->obj), link_index);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Hypertext.link_selected not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkHypertext_methods[] = {
    { "get_link", (PyCFunction)_wrap_atk_hypertext_get_link, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_n_links", (PyCFunction)_wrap_atk_hypertext_get_n_links, METH_NOARGS,
      NULL },
    { "get_link_index", (PyCFunction)_wrap_atk_hypertext_get_link_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_link", (PyCFunction)_wrap_AtkHypertext__do_get_link, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_links", (PyCFunction)_wrap_AtkHypertext__do_get_n_links, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_link_index", (PyCFunction)_wrap_AtkHypertext__do_get_link_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_link_selected", (PyCFunction)_wrap_AtkHypertext__do_link_selected, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkHypertext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Hypertext",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkHypertext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static AtkHyperlink*
_wrap_AtkHypertext__proxy_do_get_link(AtkHypertext *self, gint link_index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_link_index;
    AtkHyperlink* retval;
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
    py_link_index = PyInt_FromLong(link_index);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_link_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_link");
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
    retval = (AtkHyperlink*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkHypertext__proxy_do_get_n_links(AtkHypertext *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_links");
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
static gint
_wrap_AtkHypertext__proxy_do_get_link_index(AtkHypertext *self, gint char_index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_char_index;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_char_index = PyInt_FromLong(char_index);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_char_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_link_index");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkHypertext__proxy_do_link_selected(AtkHypertext *self, gint link_index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_link_index;
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
    py_link_index = PyInt_FromLong(link_index);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_link_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_link_selected");
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
__AtkHypertext__interface_init(AtkHypertextIface *iface, PyTypeObject *pytype)
{
    AtkHypertextIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_link") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_link = _wrap_AtkHypertext__proxy_do_get_link;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_link = parent_iface->get_link;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_links") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_links = _wrap_AtkHypertext__proxy_do_get_n_links;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_links = parent_iface->get_n_links;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_link_index") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_link_index = _wrap_AtkHypertext__proxy_do_get_link_index;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_link_index = parent_iface->get_link_index;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_link_selected") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->link_selected = _wrap_AtkHypertext__proxy_do_link_selected;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->link_selected = parent_iface->link_selected;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkHypertext__iinfo = {
    (GInterfaceInitFunc) __AtkHypertext__interface_init,
    NULL,
    NULL
};


/* ----------- AtkImage ----------- */

static PyObject *
_wrap_atk_image_get_image_description(PyGObject *self)
{
    const gchar *ret;

    
    ret = atk_image_get_image_description(ATK_IMAGE(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 224 "atk.override"
static PyObject *
_wrap_atk_image_get_image_size(PyGObject *self)
{
    gint width, height;

    atk_image_get_image_size(ATK_IMAGE(self->obj), &width, &height);
    return Py_BuildValue("(ii)", width, height);
}
#line 8733 "atk.c"


static PyObject *
_wrap_atk_image_set_image_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "description", NULL };
    char *description;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Atk.Image.set_image_description", kwlist, &description))
        return NULL;
    
    ret = atk_image_set_image_description(ATK_IMAGE(self->obj), description);
    
    return PyBool_FromLong(ret);

}

#line 234 "atk.override"
static PyObject *
_wrap_atk_image_get_image_position(PyGObject *self, PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "coord_type", NULL };
    gint x, y;
    AtkCoordType coord_type;
    PyObject *py_coord_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.Image.get_image_position", kwlist,
				     &py_coord_type))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord_type,
                           (gint *)&coord_type))
        return NULL;
    atk_image_get_image_position(ATK_IMAGE(self->obj), &x, &y, coord_type);
    return Py_BuildValue("(ii)", x, y);
}
#line 8773 "atk.c"


static PyObject *
_wrap_AtkImage__do_get_image_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkImageIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Image.get_image_description", kwlist, &PyAtkImage_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_IMAGE);
    if (iface->get_image_description)
        ret = iface->get_image_description(ATK_IMAGE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Image.get_image_description not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkImage__do_set_image_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkImageIface *iface;
    static char *kwlist[] = { "self", "description", NULL };
    PyGObject *self;
    char *description;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:Atk.Image.set_image_description", kwlist, &PyAtkImage_Type, &self, &description))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_IMAGE);
    if (iface->set_image_description)
        ret = iface->set_image_description(ATK_IMAGE(self->obj), description);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Image.set_image_description not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyAtkImage_methods[] = {
    { "get_image_description", (PyCFunction)_wrap_atk_image_get_image_description, METH_NOARGS,
      NULL },
    { "get_image_size", (PyCFunction)_wrap_atk_image_get_image_size, METH_NOARGS,
      NULL },
    { "set_image_description", (PyCFunction)_wrap_atk_image_set_image_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_image_position", (PyCFunction)_wrap_atk_image_get_image_position, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_image_description", (PyCFunction)_wrap_AtkImage__do_get_image_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_image_description", (PyCFunction)_wrap_AtkImage__do_set_image_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkImage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Image",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkImage_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static const gchar*
_wrap_AtkImage__proxy_do_get_image_description(AtkImage *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_image_description");
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
static gboolean
_wrap_AtkImage__proxy_do_set_image_description(AtkImage *self, const gchar*description)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_description = NULL;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (description)
        py_description = PyString_FromString(description);
    if (!py_description) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_description);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_image_description");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__AtkImage__interface_init(AtkImageIface *iface, PyTypeObject *pytype)
{
    AtkImageIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_image_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_image_description = _wrap_AtkImage__proxy_do_get_image_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_image_description = parent_iface->get_image_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_image_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_image_description = _wrap_AtkImage__proxy_do_set_image_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_image_description = parent_iface->set_image_description;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkImage__iinfo = {
    (GInterfaceInitFunc) __AtkImage__interface_init,
    NULL,
    NULL
};


/* ----------- AtkImplementorIface ----------- */

static PyObject *
_wrap_atk_implementor_ref_accessible(PyGObject *self)
{
    AtkObject *ret;

    
    ret = atk_implementor_ref_accessible(ATK_IMPLEMENTOR(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyAtkImplementorIface_methods[] = {
    { "ref_accessible", (PyCFunction)_wrap_atk_implementor_ref_accessible, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkImplementorIface_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.ImplementorIface",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkImplementorIface_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- AtkSelection ----------- */

static PyObject *
_wrap_atk_selection_add_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Selection.add_selection", kwlist, &i))
        return NULL;
    
    ret = atk_selection_add_selection(ATK_SELECTION(self->obj), i);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_selection_clear_selection(PyGObject *self)
{
    int ret;

    
    ret = atk_selection_clear_selection(ATK_SELECTION(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_selection_ref_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Selection.ref_selection", kwlist, &i))
        return NULL;
    
    ret = atk_selection_ref_selection(ATK_SELECTION(self->obj), i);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_selection_get_selection_count(PyGObject *self)
{
    int ret;

    
    ret = atk_selection_get_selection_count(ATK_SELECTION(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_selection_is_child_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Selection.is_child_selected", kwlist, &i))
        return NULL;
    
    ret = atk_selection_is_child_selected(ATK_SELECTION(self->obj), i);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_selection_remove_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Selection.remove_selection", kwlist, &i))
        return NULL;
    
    ret = atk_selection_remove_selection(ATK_SELECTION(self->obj), i);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_selection_select_all_selection(PyGObject *self)
{
    int ret;

    
    ret = atk_selection_select_all_selection(ATK_SELECTION(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_add_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Selection.add_selection", kwlist, &PyAtkSelection_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->add_selection)
        ret = iface->add_selection(ATK_SELECTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.add_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_clear_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Selection.clear_selection", kwlist, &PyAtkSelection_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->clear_selection)
        ret = iface->clear_selection(ATK_SELECTION(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.clear_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_ref_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Selection.ref_selection", kwlist, &PyAtkSelection_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->ref_selection)
        ret = iface->ref_selection(ATK_SELECTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.ref_selection not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkSelection__do_get_selection_count(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Selection.get_selection_count", kwlist, &PyAtkSelection_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->get_selection_count)
        ret = iface->get_selection_count(ATK_SELECTION(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.get_selection_count not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkSelection__do_is_child_selected(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Selection.is_child_selected", kwlist, &PyAtkSelection_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->is_child_selected)
        ret = iface->is_child_selected(ATK_SELECTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.is_child_selected not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_remove_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Selection.remove_selection", kwlist, &PyAtkSelection_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->remove_selection)
        ret = iface->remove_selection(ATK_SELECTION(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.remove_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_select_all_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Selection.select_all_selection", kwlist, &PyAtkSelection_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->select_all_selection)
        ret = iface->select_all_selection(ATK_SELECTION(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.select_all_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkSelection__do_selection_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkSelectionIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Selection.selection_changed", kwlist, &PyAtkSelection_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_SELECTION);
    if (iface->selection_changed)
        iface->selection_changed(ATK_SELECTION(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Selection.selection_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkSelection_methods[] = {
    { "add_selection", (PyCFunction)_wrap_atk_selection_add_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_selection", (PyCFunction)_wrap_atk_selection_clear_selection, METH_NOARGS,
      NULL },
    { "ref_selection", (PyCFunction)_wrap_atk_selection_ref_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_selection_count", (PyCFunction)_wrap_atk_selection_get_selection_count, METH_NOARGS,
      NULL },
    { "is_child_selected", (PyCFunction)_wrap_atk_selection_is_child_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_selection", (PyCFunction)_wrap_atk_selection_remove_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "select_all_selection", (PyCFunction)_wrap_atk_selection_select_all_selection, METH_NOARGS,
      NULL },
    { "do_add_selection", (PyCFunction)_wrap_AtkSelection__do_add_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_clear_selection", (PyCFunction)_wrap_AtkSelection__do_clear_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_ref_selection", (PyCFunction)_wrap_AtkSelection__do_ref_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_selection_count", (PyCFunction)_wrap_AtkSelection__do_get_selection_count, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_child_selected", (PyCFunction)_wrap_AtkSelection__do_is_child_selected, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_selection", (PyCFunction)_wrap_AtkSelection__do_remove_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_select_all_selection", (PyCFunction)_wrap_AtkSelection__do_select_all_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_selection_changed", (PyCFunction)_wrap_AtkSelection__do_selection_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkSelection_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Selection",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkSelection_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gboolean
_wrap_AtkSelection__proxy_do_add_selection(AtkSelection *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkSelection__proxy_do_clear_selection(AtkSelection *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_clear_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkSelection__proxy_do_ref_selection(AtkSelection *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    AtkObject* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_selection");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkSelection__proxy_do_get_selection_count(AtkSelection *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_selection_count");
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
static gboolean
_wrap_AtkSelection__proxy_do_is_child_selected(AtkSelection *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_is_child_selected");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkSelection__proxy_do_remove_selection(AtkSelection *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkSelection__proxy_do_select_all_selection(AtkSelection *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_select_all_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkSelection__proxy_do_selection_changed(AtkSelection *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_selection_changed");
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
__AtkSelection__interface_init(AtkSelectionIface *iface, PyTypeObject *pytype)
{
    AtkSelectionIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_selection = _wrap_AtkSelection__proxy_do_add_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_selection = parent_iface->add_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_clear_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->clear_selection = _wrap_AtkSelection__proxy_do_clear_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->clear_selection = parent_iface->clear_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_ref_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->ref_selection = _wrap_AtkSelection__proxy_do_ref_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->ref_selection = parent_iface->ref_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_selection_count") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_selection_count = _wrap_AtkSelection__proxy_do_get_selection_count;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_selection_count = parent_iface->get_selection_count;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_is_child_selected") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->is_child_selected = _wrap_AtkSelection__proxy_do_is_child_selected;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->is_child_selected = parent_iface->is_child_selected;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_selection = _wrap_AtkSelection__proxy_do_remove_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_selection = parent_iface->remove_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_select_all_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->select_all_selection = _wrap_AtkSelection__proxy_do_select_all_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->select_all_selection = parent_iface->select_all_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_selection_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->selection_changed = _wrap_AtkSelection__proxy_do_selection_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->selection_changed = parent_iface->selection_changed;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkSelection__iinfo = {
    (GInterfaceInitFunc) __AtkSelection__interface_init,
    NULL,
    NULL
};


/* ----------- AtkStreamableContent ----------- */

static PyObject *
_wrap_atk_streamable_content_get_n_mime_types(PyGObject *self)
{
    int ret;

    
    ret = atk_streamable_content_get_n_mime_types(ATK_STREAMABLE_CONTENT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_streamable_content_get_mime_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.StreamableContent.get_mime_type", kwlist, &i))
        return NULL;
    
    ret = atk_streamable_content_get_mime_type(ATK_STREAMABLE_CONTENT(self->obj), i);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkStreamableContent__do_get_n_mime_types(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkStreamableContentIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.StreamableContent.get_n_mime_types", kwlist, &PyAtkStreamableContent_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_STREAMABLE_CONTENT);
    if (iface->get_n_mime_types)
        ret = iface->get_n_mime_types(ATK_STREAMABLE_CONTENT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.StreamableContent.get_n_mime_types not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkStreamableContent__do_get_mime_type(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkStreamableContentIface *iface;
    static char *kwlist[] = { "self", "i", NULL };
    PyGObject *self;
    int i;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.StreamableContent.get_mime_type", kwlist, &PyAtkStreamableContent_Type, &self, &i))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_STREAMABLE_CONTENT);
    if (iface->get_mime_type)
        ret = iface->get_mime_type(ATK_STREAMABLE_CONTENT(self->obj), i);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.StreamableContent.get_mime_type not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkStreamableContent_methods[] = {
    { "get_n_mime_types", (PyCFunction)_wrap_atk_streamable_content_get_n_mime_types, METH_NOARGS,
      NULL },
    { "get_mime_type", (PyCFunction)_wrap_atk_streamable_content_get_mime_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_n_mime_types", (PyCFunction)_wrap_AtkStreamableContent__do_get_n_mime_types, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_mime_type", (PyCFunction)_wrap_AtkStreamableContent__do_get_mime_type, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkStreamableContent_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.StreamableContent",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkStreamableContent_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gint
_wrap_AtkStreamableContent__proxy_do_get_n_mime_types(AtkStreamableContent *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_mime_types");
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
static const gchar*
_wrap_AtkStreamableContent__proxy_do_get_mime_type(AtkStreamableContent *self, gint i)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_i;
    const gchar* retval;
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
    py_i = PyInt_FromLong(i);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_i);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_mime_type");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__AtkStreamableContent__interface_init(AtkStreamableContentIface *iface, PyTypeObject *pytype)
{
    AtkStreamableContentIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_mime_types") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_mime_types = _wrap_AtkStreamableContent__proxy_do_get_n_mime_types;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_mime_types = parent_iface->get_n_mime_types;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_mime_type") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_mime_type = _wrap_AtkStreamableContent__proxy_do_get_mime_type;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_mime_type = parent_iface->get_mime_type;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkStreamableContent__iinfo = {
    (GInterfaceInitFunc) __AtkStreamableContent__interface_init,
    NULL,
    NULL
};


/* ----------- AtkTable ----------- */

static PyObject *
_wrap_atk_table_ref_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "column", NULL };
    int row, column;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Table.ref_at", kwlist, &row, &column))
        return NULL;
    
    ret = atk_table_ref_at(ATK_TABLE(self->obj), row, column);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_table_get_index_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "column", NULL };
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Table.get_index_at", kwlist, &row, &column))
        return NULL;
    
    ret = atk_table_get_index_at(ATK_TABLE(self->obj), row, column);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_column_at_index(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    int index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_column_at_index", kwlist, &index))
        return NULL;
    
    ret = atk_table_get_column_at_index(ATK_TABLE(self->obj), index);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_row_at_index(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    int index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_row_at_index", kwlist, &index))
        return NULL;
    
    ret = atk_table_get_row_at_index(ATK_TABLE(self->obj), index);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_n_columns(PyGObject *self)
{
    int ret;

    
    ret = atk_table_get_n_columns(ATK_TABLE(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_n_rows(PyGObject *self)
{
    int ret;

    
    ret = atk_table_get_n_rows(ATK_TABLE(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_column_extent_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "column", NULL };
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Table.get_column_extent_at", kwlist, &row, &column))
        return NULL;
    
    ret = atk_table_get_column_extent_at(ATK_TABLE(self->obj), row, column);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_row_extent_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "column", NULL };
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Table.get_row_extent_at", kwlist, &row, &column))
        return NULL;
    
    ret = atk_table_get_row_extent_at(ATK_TABLE(self->obj), row, column);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_table_get_caption(PyGObject *self)
{
    AtkObject *ret;

    
    ret = atk_table_get_caption(ATK_TABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_table_get_column_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", NULL };
    int column;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_column_description", kwlist, &column))
        return NULL;
    
    ret = atk_table_get_column_description(ATK_TABLE(self->obj), column);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_get_column_header(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", NULL };
    int column;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_column_header", kwlist, &column))
        return NULL;
    
    ret = atk_table_get_column_header(ATK_TABLE(self->obj), column);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_table_get_row_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", NULL };
    int row;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_row_description", kwlist, &row))
        return NULL;
    
    ret = atk_table_get_row_description(ATK_TABLE(self->obj), row);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_get_row_header(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", NULL };
    int row;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.get_row_header", kwlist, &row))
        return NULL;
    
    ret = atk_table_get_row_header(ATK_TABLE(self->obj), row);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_table_get_summary(PyGObject *self)
{
    AtkObject *ret;

    
    ret = atk_table_get_summary(ATK_TABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_table_set_caption(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caption", NULL };
    PyGObject *caption;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.set_caption", kwlist, &PyAtkObject_Type, &caption))
        return NULL;
    
    atk_table_set_caption(ATK_TABLE(self->obj), ATK_OBJECT(caption->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_set_column_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", "description", NULL };
    int column;
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"is:Atk.Table.set_column_description", kwlist, &column, &description))
        return NULL;
    
    atk_table_set_column_description(ATK_TABLE(self->obj), column, description);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_set_column_header(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", "header", NULL };
    int column;
    PyGObject *header;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO!:Atk.Table.set_column_header", kwlist, &column, &PyAtkObject_Type, &header))
        return NULL;
    
    atk_table_set_column_header(ATK_TABLE(self->obj), column, ATK_OBJECT(header->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_set_row_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "description", NULL };
    int row;
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"is:Atk.Table.set_row_description", kwlist, &row, &description))
        return NULL;
    
    atk_table_set_row_description(ATK_TABLE(self->obj), row, description);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_set_row_header(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "header", NULL };
    int row;
    PyGObject *header;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO!:Atk.Table.set_row_header", kwlist, &row, &PyAtkObject_Type, &header))
        return NULL;
    
    atk_table_set_row_header(ATK_TABLE(self->obj), row, ATK_OBJECT(header->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_table_set_summary(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "accessible", NULL };
    PyGObject *accessible;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.set_summary", kwlist, &PyAtkObject_Type, &accessible))
        return NULL;
    
    atk_table_set_summary(ATK_TABLE(self->obj), ATK_OBJECT(accessible->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 351 "atk.override"
static PyObject *
_wrap_atk_table_get_selected_columns(PyGObject *self)
{
    gint *selected, ret, i;
    PyObject *py_selected;

    ret = atk_table_get_selected_columns(ATK_TABLE(self->obj), &selected);
    py_selected = PyTuple_New(ret);
    for (i = 0; i < ret; i++) {
        PyTuple_SetItem(py_selected, i, PyInt_FromLong(selected[i]));
    }

    g_free(selected);
    return py_selected;
}
#line 10653 "atk.c"


#line 368 "atk.override"
static PyObject *
_wrap_atk_table_get_selected_rows(PyGObject *self)
{
    gint *selected, ret, i;
    PyObject *py_selected;

    ret = atk_table_get_selected_rows(ATK_TABLE(self->obj), &selected);
    py_selected = PyTuple_New(ret);
    for (i = 0; i < ret; i++) {
        PyTuple_SetItem(py_selected, i, PyInt_FromLong(selected[i]));
    }

    g_free(selected);
    return py_selected;
}
#line 10672 "atk.c"


static PyObject *
_wrap_atk_table_is_column_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", NULL };
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.is_column_selected", kwlist, &column))
        return NULL;
    
    ret = atk_table_is_column_selected(ATK_TABLE(self->obj), column);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_is_row_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", NULL };
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.is_row_selected", kwlist, &row))
        return NULL;
    
    ret = atk_table_is_row_selected(ATK_TABLE(self->obj), row);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_is_selected(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", "column", NULL };
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Table.is_selected", kwlist, &row, &column))
        return NULL;
    
    ret = atk_table_is_selected(ATK_TABLE(self->obj), row, column);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_add_row_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", NULL };
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.add_row_selection", kwlist, &row))
        return NULL;
    
    ret = atk_table_add_row_selection(ATK_TABLE(self->obj), row);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_remove_row_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "row", NULL };
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.remove_row_selection", kwlist, &row))
        return NULL;
    
    ret = atk_table_remove_row_selection(ATK_TABLE(self->obj), row);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_add_column_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", NULL };
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.add_column_selection", kwlist, &column))
        return NULL;
    
    ret = atk_table_add_column_selection(ATK_TABLE(self->obj), column);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_table_remove_column_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "column", NULL };
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Table.remove_column_selection", kwlist, &column))
        return NULL;
    
    ret = atk_table_remove_column_selection(ATK_TABLE(self->obj), column);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_ref_at(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "column", NULL };
    PyGObject *self;
    int row, column;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.ref_at", kwlist, &PyAtkTable_Type, &self, &row, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->ref_at)
        ret = iface->ref_at(ATK_TABLE(self->obj), row, column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.ref_at not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkTable__do_get_index_at(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "column", NULL };
    PyGObject *self;
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.get_index_at", kwlist, &PyAtkTable_Type, &self, &row, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_index_at)
        ret = iface->get_index_at(ATK_TABLE(self->obj), row, column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_index_at not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_column_at_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "index", NULL };
    PyGObject *self;
    int index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_column_at_index", kwlist, &PyAtkTable_Type, &self, &index))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_column_at_index)
        ret = iface->get_column_at_index(ATK_TABLE(self->obj), index);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_column_at_index not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_row_at_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "index", NULL };
    PyGObject *self;
    int index, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_row_at_index", kwlist, &PyAtkTable_Type, &self, &index))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_row_at_index)
        ret = iface->get_row_at_index(ATK_TABLE(self->obj), index);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_row_at_index not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_n_columns(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.get_n_columns", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_n_columns)
        ret = iface->get_n_columns(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_n_columns not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_n_rows(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.get_n_rows", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_n_rows)
        ret = iface->get_n_rows(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_n_rows not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_column_extent_at(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "column", NULL };
    PyGObject *self;
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.get_column_extent_at", kwlist, &PyAtkTable_Type, &self, &row, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_column_extent_at)
        ret = iface->get_column_extent_at(ATK_TABLE(self->obj), row, column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_column_extent_at not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_row_extent_at(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "column", NULL };
    PyGObject *self;
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.get_row_extent_at", kwlist, &PyAtkTable_Type, &self, &row, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_row_extent_at)
        ret = iface->get_row_extent_at(ATK_TABLE(self->obj), row, column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_row_extent_at not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkTable__do_get_caption(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.get_caption", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_caption)
        ret = iface->get_caption(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_caption not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkTable__do_get_column_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", NULL };
    PyGObject *self;
    int column;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_column_description", kwlist, &PyAtkTable_Type, &self, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_column_description)
        ret = iface->get_column_description(ATK_TABLE(self->obj), column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_column_description not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_get_column_header(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", NULL };
    PyGObject *self;
    int column;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_column_header", kwlist, &PyAtkTable_Type, &self, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_column_header)
        ret = iface->get_column_header(ATK_TABLE(self->obj), column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_column_header not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkTable__do_get_row_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", NULL };
    PyGObject *self;
    int row;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_row_description", kwlist, &PyAtkTable_Type, &self, &row))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_row_description)
        ret = iface->get_row_description(ATK_TABLE(self->obj), row);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_row_description not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_get_row_header(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", NULL };
    PyGObject *self;
    int row;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.get_row_header", kwlist, &PyAtkTable_Type, &self, &row))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_row_header)
        ret = iface->get_row_header(ATK_TABLE(self->obj), row);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_row_header not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkTable__do_get_summary(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.get_summary", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->get_summary)
        ret = iface->get_summary(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.get_summary not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_AtkTable__do_set_caption(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "caption", NULL };
    PyGObject *self, *caption;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:Atk.Table.set_caption", kwlist, &PyAtkTable_Type, &self, &PyAtkObject_Type, &caption))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_caption)
        iface->set_caption(ATK_TABLE(self->obj), ATK_OBJECT(caption->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_caption not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_set_column_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", "description", NULL };
    PyGObject *self;
    int column;
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!is:Atk.Table.set_column_description", kwlist, &PyAtkTable_Type, &self, &column, &description))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_column_description)
        iface->set_column_description(ATK_TABLE(self->obj), column, description);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_column_description not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_set_column_header(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", "header", NULL };
    PyGObject *self, *header;
    int column;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iO!:Atk.Table.set_column_header", kwlist, &PyAtkTable_Type, &self, &column, &PyAtkObject_Type, &header))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_column_header)
        iface->set_column_header(ATK_TABLE(self->obj), column, ATK_OBJECT(header->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_column_header not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_set_row_description(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "description", NULL };
    PyGObject *self;
    int row;
    char *description;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!is:Atk.Table.set_row_description", kwlist, &PyAtkTable_Type, &self, &row, &description))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_row_description)
        iface->set_row_description(ATK_TABLE(self->obj), row, description);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_row_description not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_set_row_header(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "header", NULL };
    PyGObject *self, *header;
    int row;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iO!:Atk.Table.set_row_header", kwlist, &PyAtkTable_Type, &self, &row, &PyAtkObject_Type, &header))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_row_header)
        iface->set_row_header(ATK_TABLE(self->obj), row, ATK_OBJECT(header->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_row_header not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_set_summary(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "accessible", NULL };
    PyGObject *self, *accessible;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:Atk.Table.set_summary", kwlist, &PyAtkTable_Type, &self, &PyAtkObject_Type, &accessible))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->set_summary)
        iface->set_summary(ATK_TABLE(self->obj), ATK_OBJECT(accessible->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.set_summary not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_is_column_selected(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", NULL };
    PyGObject *self;
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.is_column_selected", kwlist, &PyAtkTable_Type, &self, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->is_column_selected)
        ret = iface->is_column_selected(ATK_TABLE(self->obj), column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.is_column_selected not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_is_row_selected(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", NULL };
    PyGObject *self;
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.is_row_selected", kwlist, &PyAtkTable_Type, &self, &row))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->is_row_selected)
        ret = iface->is_row_selected(ATK_TABLE(self->obj), row);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.is_row_selected not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_is_selected(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "column", NULL };
    PyGObject *self;
    int row, column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.is_selected", kwlist, &PyAtkTable_Type, &self, &row, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->is_selected)
        ret = iface->is_selected(ATK_TABLE(self->obj), row, column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.is_selected not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_add_row_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", NULL };
    PyGObject *self;
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.add_row_selection", kwlist, &PyAtkTable_Type, &self, &row))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->add_row_selection)
        ret = iface->add_row_selection(ATK_TABLE(self->obj), row);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.add_row_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_remove_row_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", NULL };
    PyGObject *self;
    int row, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.remove_row_selection", kwlist, &PyAtkTable_Type, &self, &row))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->remove_row_selection)
        ret = iface->remove_row_selection(ATK_TABLE(self->obj), row);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.remove_row_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_add_column_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", NULL };
    PyGObject *self;
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.add_column_selection", kwlist, &PyAtkTable_Type, &self, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->add_column_selection)
        ret = iface->add_column_selection(ATK_TABLE(self->obj), column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.add_column_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_remove_column_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", NULL };
    PyGObject *self;
    int column, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Table.remove_column_selection", kwlist, &PyAtkTable_Type, &self, &column))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->remove_column_selection)
        ret = iface->remove_column_selection(ATK_TABLE(self->obj), column);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.remove_column_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkTable__do_row_inserted(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "num_inserted", NULL };
    PyGObject *self;
    int row, num_inserted;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.row_inserted", kwlist, &PyAtkTable_Type, &self, &row, &num_inserted))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->row_inserted)
        iface->row_inserted(ATK_TABLE(self->obj), row, num_inserted);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.row_inserted not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_column_inserted(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", "num_inserted", NULL };
    PyGObject *self;
    int column, num_inserted;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.column_inserted", kwlist, &PyAtkTable_Type, &self, &column, &num_inserted))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->column_inserted)
        iface->column_inserted(ATK_TABLE(self->obj), column, num_inserted);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.column_inserted not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_row_deleted(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "row", "num_deleted", NULL };
    PyGObject *self;
    int row, num_deleted;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.row_deleted", kwlist, &PyAtkTable_Type, &self, &row, &num_deleted))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->row_deleted)
        iface->row_deleted(ATK_TABLE(self->obj), row, num_deleted);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.row_deleted not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_column_deleted(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", "column", "num_deleted", NULL };
    PyGObject *self;
    int column, num_deleted;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Table.column_deleted", kwlist, &PyAtkTable_Type, &self, &column, &num_deleted))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->column_deleted)
        iface->column_deleted(ATK_TABLE(self->obj), column, num_deleted);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.column_deleted not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_row_reordered(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.row_reordered", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->row_reordered)
        iface->row_reordered(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.row_reordered not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_column_reordered(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.column_reordered", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->column_reordered)
        iface->column_reordered(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.column_reordered not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkTable__do_model_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTableIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Table.model_changed", kwlist, &PyAtkTable_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TABLE);
    if (iface->model_changed)
        iface->model_changed(ATK_TABLE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Table.model_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkTable_methods[] = {
    { "ref_at", (PyCFunction)_wrap_atk_table_ref_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_index_at", (PyCFunction)_wrap_atk_table_get_index_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_column_at_index", (PyCFunction)_wrap_atk_table_get_column_at_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_row_at_index", (PyCFunction)_wrap_atk_table_get_row_at_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_n_columns", (PyCFunction)_wrap_atk_table_get_n_columns, METH_NOARGS,
      NULL },
    { "get_n_rows", (PyCFunction)_wrap_atk_table_get_n_rows, METH_NOARGS,
      NULL },
    { "get_column_extent_at", (PyCFunction)_wrap_atk_table_get_column_extent_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_row_extent_at", (PyCFunction)_wrap_atk_table_get_row_extent_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_caption", (PyCFunction)_wrap_atk_table_get_caption, METH_NOARGS,
      NULL },
    { "get_column_description", (PyCFunction)_wrap_atk_table_get_column_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_column_header", (PyCFunction)_wrap_atk_table_get_column_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_row_description", (PyCFunction)_wrap_atk_table_get_row_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_row_header", (PyCFunction)_wrap_atk_table_get_row_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_summary", (PyCFunction)_wrap_atk_table_get_summary, METH_NOARGS,
      NULL },
    { "set_caption", (PyCFunction)_wrap_atk_table_set_caption, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_column_description", (PyCFunction)_wrap_atk_table_set_column_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_column_header", (PyCFunction)_wrap_atk_table_set_column_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_row_description", (PyCFunction)_wrap_atk_table_set_row_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_row_header", (PyCFunction)_wrap_atk_table_set_row_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_summary", (PyCFunction)_wrap_atk_table_set_summary, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_selected_columns", (PyCFunction)_wrap_atk_table_get_selected_columns, METH_NOARGS,
      NULL },
    { "get_selected_rows", (PyCFunction)_wrap_atk_table_get_selected_rows, METH_NOARGS,
      NULL },
    { "is_column_selected", (PyCFunction)_wrap_atk_table_is_column_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_row_selected", (PyCFunction)_wrap_atk_table_is_row_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_selected", (PyCFunction)_wrap_atk_table_is_selected, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_row_selection", (PyCFunction)_wrap_atk_table_add_row_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_row_selection", (PyCFunction)_wrap_atk_table_remove_row_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_column_selection", (PyCFunction)_wrap_atk_table_add_column_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_column_selection", (PyCFunction)_wrap_atk_table_remove_column_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_ref_at", (PyCFunction)_wrap_AtkTable__do_ref_at, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_index_at", (PyCFunction)_wrap_AtkTable__do_get_index_at, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_column_at_index", (PyCFunction)_wrap_AtkTable__do_get_column_at_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_row_at_index", (PyCFunction)_wrap_AtkTable__do_get_row_at_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_columns", (PyCFunction)_wrap_AtkTable__do_get_n_columns, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_rows", (PyCFunction)_wrap_AtkTable__do_get_n_rows, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_column_extent_at", (PyCFunction)_wrap_AtkTable__do_get_column_extent_at, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_row_extent_at", (PyCFunction)_wrap_AtkTable__do_get_row_extent_at, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_caption", (PyCFunction)_wrap_AtkTable__do_get_caption, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_column_description", (PyCFunction)_wrap_AtkTable__do_get_column_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_column_header", (PyCFunction)_wrap_AtkTable__do_get_column_header, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_row_description", (PyCFunction)_wrap_AtkTable__do_get_row_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_row_header", (PyCFunction)_wrap_AtkTable__do_get_row_header, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_summary", (PyCFunction)_wrap_AtkTable__do_get_summary, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_caption", (PyCFunction)_wrap_AtkTable__do_set_caption, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_column_description", (PyCFunction)_wrap_AtkTable__do_set_column_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_column_header", (PyCFunction)_wrap_AtkTable__do_set_column_header, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_row_description", (PyCFunction)_wrap_AtkTable__do_set_row_description, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_row_header", (PyCFunction)_wrap_AtkTable__do_set_row_header, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_summary", (PyCFunction)_wrap_AtkTable__do_set_summary, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_column_selected", (PyCFunction)_wrap_AtkTable__do_is_column_selected, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_row_selected", (PyCFunction)_wrap_AtkTable__do_is_row_selected, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_selected", (PyCFunction)_wrap_AtkTable__do_is_selected, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_add_row_selection", (PyCFunction)_wrap_AtkTable__do_add_row_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_row_selection", (PyCFunction)_wrap_AtkTable__do_remove_row_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_add_column_selection", (PyCFunction)_wrap_AtkTable__do_add_column_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_column_selection", (PyCFunction)_wrap_AtkTable__do_remove_column_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_row_inserted", (PyCFunction)_wrap_AtkTable__do_row_inserted, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_column_inserted", (PyCFunction)_wrap_AtkTable__do_column_inserted, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_row_deleted", (PyCFunction)_wrap_AtkTable__do_row_deleted, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_column_deleted", (PyCFunction)_wrap_AtkTable__do_column_deleted, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_row_reordered", (PyCFunction)_wrap_AtkTable__do_row_reordered, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_column_reordered", (PyCFunction)_wrap_AtkTable__do_column_reordered, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_model_changed", (PyCFunction)_wrap_AtkTable__do_model_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkTable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Table",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkTable_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static AtkObject*
_wrap_AtkTable__proxy_do_ref_at(AtkTable *self, gint row, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_column;
    AtkObject* retval;
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
    py_row = PyInt_FromLong(row);
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_ref_at");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkTable__proxy_do_get_index_at(AtkTable *self, gint row, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_column;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_row = PyInt_FromLong(row);
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_index_at");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkTable__proxy_do_get_column_at_index(AtkTable *self, gint index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_index;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_index = PyInt_FromLong(index);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_column_at_index");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkTable__proxy_do_get_row_at_index(AtkTable *self, gint index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_index;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_index = PyInt_FromLong(index);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_row_at_index");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkTable__proxy_do_get_n_columns(AtkTable *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_columns");
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
static gint
_wrap_AtkTable__proxy_do_get_n_rows(AtkTable *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_rows");
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
static gint
_wrap_AtkTable__proxy_do_get_column_extent_at(AtkTable *self, gint row, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_column;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_row = PyInt_FromLong(row);
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_column_extent_at");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkTable__proxy_do_get_row_extent_at(AtkTable *self, gint row, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_column;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_row = PyInt_FromLong(row);
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_row_extent_at");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkTable__proxy_do_get_caption(AtkTable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkObject* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_caption");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static const gchar*
_wrap_AtkTable__proxy_do_get_column_description(AtkTable *self, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    const gchar* retval;
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
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_column_description");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkTable__proxy_do_get_column_header(AtkTable *self, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    AtkObject* retval;
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
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_column_header");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static const gchar*
_wrap_AtkTable__proxy_do_get_row_description(AtkTable *self, gint row)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    const gchar* retval;
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
    py_row = PyInt_FromLong(row);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_row_description");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkTable__proxy_do_get_row_header(AtkTable *self, gint row)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    AtkObject* retval;
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
    py_row = PyInt_FromLong(row);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_row_header");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static AtkObject*
_wrap_AtkTable__proxy_do_get_summary(AtkTable *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    AtkObject* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_summary");
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
    retval = (AtkObject*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkTable__proxy_do_set_caption(AtkTable *self, AtkObject*caption)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_caption = NULL;
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
    if (caption)
        py_caption = pygobject_new((GObject *) caption);
    else {
        Py_INCREF(Py_None);
        py_caption = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_caption);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_caption");
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
_wrap_AtkTable__proxy_do_set_column_description(AtkTable *self, gint column, const gchar*description)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    PyObject *py_description = NULL;
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
    py_column = PyInt_FromLong(column);
    if (description)
        py_description = PyString_FromString(description);
    if (!py_description) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_column);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    PyTuple_SET_ITEM(py_args, 1, py_description);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_column_description");
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
_wrap_AtkTable__proxy_do_set_column_header(AtkTable *self, gint column, AtkObject*header)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    PyObject *py_header = NULL;
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
    py_column = PyInt_FromLong(column);
    if (header)
        py_header = pygobject_new((GObject *) header);
    else {
        Py_INCREF(Py_None);
        py_header = Py_None;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    PyTuple_SET_ITEM(py_args, 1, py_header);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_column_header");
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
_wrap_AtkTable__proxy_do_set_row_description(AtkTable *self, gint row, const gchar*description)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_description = NULL;
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
    py_row = PyInt_FromLong(row);
    if (description)
        py_description = PyString_FromString(description);
    if (!py_description) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_row);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_description);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_row_description");
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
_wrap_AtkTable__proxy_do_set_row_header(AtkTable *self, gint row, AtkObject*header)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_header = NULL;
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
    py_row = PyInt_FromLong(row);
    if (header)
        py_header = pygobject_new((GObject *) header);
    else {
        Py_INCREF(Py_None);
        py_header = Py_None;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_header);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_row_header");
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
_wrap_AtkTable__proxy_do_set_summary(AtkTable *self, AtkObject*accessible)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_accessible = NULL;
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
    if (accessible)
        py_accessible = pygobject_new((GObject *) accessible);
    else {
        Py_INCREF(Py_None);
        py_accessible = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_accessible);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_summary");
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
static gboolean
_wrap_AtkTable__proxy_do_is_column_selected(AtkTable *self, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_is_column_selected");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_is_row_selected(AtkTable *self, gint row)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_row = PyInt_FromLong(row);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    
    py_method = PyObject_GetAttrString(py_self, "do_is_row_selected");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_is_selected(AtkTable *self, gint row, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_column;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_row = PyInt_FromLong(row);
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_is_selected");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_add_row_selection(AtkTable *self, gint row)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_row = PyInt_FromLong(row);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_row_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_remove_row_selection(AtkTable *self, gint row)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_row = PyInt_FromLong(row);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_row_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_add_column_selection(AtkTable *self, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_column_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkTable__proxy_do_remove_column_selection(AtkTable *self, gint column)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_column = PyInt_FromLong(column);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_column_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkTable__proxy_do_row_inserted(AtkTable *self, gint row, gint num_inserted)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_num_inserted;
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
    py_row = PyInt_FromLong(row);
    py_num_inserted = PyInt_FromLong(num_inserted);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_num_inserted);
    
    py_method = PyObject_GetAttrString(py_self, "do_row_inserted");
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
_wrap_AtkTable__proxy_do_column_inserted(AtkTable *self, gint column, gint num_inserted)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    PyObject *py_num_inserted;
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
    py_column = PyInt_FromLong(column);
    py_num_inserted = PyInt_FromLong(num_inserted);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    PyTuple_SET_ITEM(py_args, 1, py_num_inserted);
    
    py_method = PyObject_GetAttrString(py_self, "do_column_inserted");
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
_wrap_AtkTable__proxy_do_row_deleted(AtkTable *self, gint row, gint num_deleted)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_row;
    PyObject *py_num_deleted;
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
    py_row = PyInt_FromLong(row);
    py_num_deleted = PyInt_FromLong(num_deleted);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_row);
    PyTuple_SET_ITEM(py_args, 1, py_num_deleted);
    
    py_method = PyObject_GetAttrString(py_self, "do_row_deleted");
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
_wrap_AtkTable__proxy_do_column_deleted(AtkTable *self, gint column, gint num_deleted)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_column;
    PyObject *py_num_deleted;
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
    py_column = PyInt_FromLong(column);
    py_num_deleted = PyInt_FromLong(num_deleted);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_column);
    PyTuple_SET_ITEM(py_args, 1, py_num_deleted);
    
    py_method = PyObject_GetAttrString(py_self, "do_column_deleted");
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
_wrap_AtkTable__proxy_do_row_reordered(AtkTable *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_row_reordered");
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
_wrap_AtkTable__proxy_do_column_reordered(AtkTable *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_column_reordered");
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
_wrap_AtkTable__proxy_do_model_changed(AtkTable *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_model_changed");
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
__AtkTable__interface_init(AtkTableIface *iface, PyTypeObject *pytype)
{
    AtkTableIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_ref_at") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->ref_at = _wrap_AtkTable__proxy_do_ref_at;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->ref_at = parent_iface->ref_at;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_index_at") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_index_at = _wrap_AtkTable__proxy_do_get_index_at;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_index_at = parent_iface->get_index_at;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_column_at_index") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_column_at_index = _wrap_AtkTable__proxy_do_get_column_at_index;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_column_at_index = parent_iface->get_column_at_index;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_row_at_index") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_row_at_index = _wrap_AtkTable__proxy_do_get_row_at_index;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_row_at_index = parent_iface->get_row_at_index;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_columns") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_columns = _wrap_AtkTable__proxy_do_get_n_columns;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_columns = parent_iface->get_n_columns;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_rows") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_rows = _wrap_AtkTable__proxy_do_get_n_rows;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_rows = parent_iface->get_n_rows;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_column_extent_at") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_column_extent_at = _wrap_AtkTable__proxy_do_get_column_extent_at;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_column_extent_at = parent_iface->get_column_extent_at;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_row_extent_at") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_row_extent_at = _wrap_AtkTable__proxy_do_get_row_extent_at;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_row_extent_at = parent_iface->get_row_extent_at;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_caption") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_caption = _wrap_AtkTable__proxy_do_get_caption;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_caption = parent_iface->get_caption;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_column_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_column_description = _wrap_AtkTable__proxy_do_get_column_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_column_description = parent_iface->get_column_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_column_header") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_column_header = _wrap_AtkTable__proxy_do_get_column_header;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_column_header = parent_iface->get_column_header;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_row_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_row_description = _wrap_AtkTable__proxy_do_get_row_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_row_description = parent_iface->get_row_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_row_header") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_row_header = _wrap_AtkTable__proxy_do_get_row_header;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_row_header = parent_iface->get_row_header;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_summary") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_summary = _wrap_AtkTable__proxy_do_get_summary;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_summary = parent_iface->get_summary;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_caption") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_caption = _wrap_AtkTable__proxy_do_set_caption;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_caption = parent_iface->set_caption;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_column_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_column_description = _wrap_AtkTable__proxy_do_set_column_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_column_description = parent_iface->set_column_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_column_header") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_column_header = _wrap_AtkTable__proxy_do_set_column_header;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_column_header = parent_iface->set_column_header;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_row_description") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_row_description = _wrap_AtkTable__proxy_do_set_row_description;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_row_description = parent_iface->set_row_description;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_row_header") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_row_header = _wrap_AtkTable__proxy_do_set_row_header;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_row_header = parent_iface->set_row_header;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_summary") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_summary = _wrap_AtkTable__proxy_do_set_summary;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_summary = parent_iface->set_summary;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_is_column_selected") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->is_column_selected = _wrap_AtkTable__proxy_do_is_column_selected;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->is_column_selected = parent_iface->is_column_selected;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_is_row_selected") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->is_row_selected = _wrap_AtkTable__proxy_do_is_row_selected;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->is_row_selected = parent_iface->is_row_selected;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_is_selected") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->is_selected = _wrap_AtkTable__proxy_do_is_selected;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->is_selected = parent_iface->is_selected;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_row_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_row_selection = _wrap_AtkTable__proxy_do_add_row_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_row_selection = parent_iface->add_row_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_row_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_row_selection = _wrap_AtkTable__proxy_do_remove_row_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_row_selection = parent_iface->remove_row_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_column_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_column_selection = _wrap_AtkTable__proxy_do_add_column_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_column_selection = parent_iface->add_column_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_column_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_column_selection = _wrap_AtkTable__proxy_do_remove_column_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_column_selection = parent_iface->remove_column_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_row_inserted") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->row_inserted = _wrap_AtkTable__proxy_do_row_inserted;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->row_inserted = parent_iface->row_inserted;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_column_inserted") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->column_inserted = _wrap_AtkTable__proxy_do_column_inserted;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->column_inserted = parent_iface->column_inserted;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_row_deleted") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->row_deleted = _wrap_AtkTable__proxy_do_row_deleted;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->row_deleted = parent_iface->row_deleted;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_column_deleted") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->column_deleted = _wrap_AtkTable__proxy_do_column_deleted;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->column_deleted = parent_iface->column_deleted;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_row_reordered") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->row_reordered = _wrap_AtkTable__proxy_do_row_reordered;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->row_reordered = parent_iface->row_reordered;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_column_reordered") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->column_reordered = _wrap_AtkTable__proxy_do_column_reordered;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->column_reordered = parent_iface->column_reordered;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_model_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->model_changed = _wrap_AtkTable__proxy_do_model_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->model_changed = parent_iface->model_changed;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkTable__iinfo = {
    (GInterfaceInitFunc) __AtkTable__interface_init,
    NULL,
    NULL
};


/* ----------- AtkText ----------- */

static PyObject *
_wrap_atk_text_get_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_offset", "end_offset", NULL };
    int start_offset, end_offset;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Text.get_text", kwlist, &start_offset, &end_offset))
        return NULL;
    
    ret = atk_text_get_text(ATK_TEXT(self->obj), start_offset, end_offset);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_text_get_character_at_offset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", NULL };
    int offset;
    gunichar ret;
    Py_UNICODE py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Text.get_character_at_offset", kwlist, &offset))
        return NULL;
    
    ret = atk_text_get_character_at_offset(ATK_TEXT(self->obj), offset);
    
#if !defined(Py_UNICODE_SIZE) || Py_UNICODE_SIZE == 2
    if (ret > 0xffff) {
        PyErr_SetString(PyExc_RuntimeError, "returned character can not be represented in 16-bit unicode");
        return NULL;
    }
#endif
    py_ret = (Py_UNICODE)ret;
    return PyUnicode_FromUnicode(&py_ret, 1);

}

#line 385 "atk.override"
static PyObject *
_wrap_atk_text_get_text_after_offset(PyGObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "boundary_type", NULL };
    gchar *text;
    gint start, end, offset;
    AtkTextBoundary boundary;
    PyObject *py_boundary;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_text_after_offset",
                                     kwlist, &offset, &py_boundary))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_TEXT_BOUNDARY, py_boundary,
                           (gpointer)&boundary))
        return NULL;

    text = atk_text_get_text_after_offset(ATK_TEXT(self->obj),
                                          offset, boundary, &start, &end);
    return Py_BuildValue("(s#ii)", text, (Py_ssize_t) (end - start), start, end);
}
#line 14289 "atk.c"


#line 410 "atk.override"
static PyObject *
_wrap_atk_text_get_text_at_offset(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "boundary_type", NULL };
    gchar *text;
    gint start, end, offset;
    AtkTextBoundary boundary;
    PyObject *py_boundary;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_text_at_offset",
                                     kwlist, &offset, &py_boundary))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_TEXT_BOUNDARY, py_boundary,
                           (gpointer)&boundary))
        return NULL;

    text = atk_text_get_text_at_offset(ATK_TEXT(self->obj),
                                       offset, boundary, &start, &end);
    return Py_BuildValue("(s#ii)", text, (Py_ssize_t) (end - start), start, end);
}
#line 14316 "atk.c"


#line 435 "atk.override"
static PyObject *
_wrap_atk_text_get_text_before_offset(PyGObject *self, PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "boundary_type", NULL };
    gchar *text;
    gint start, end, offset;
    AtkTextBoundary boundary;
    PyObject *py_boundary;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_text_before_offset",
                                     kwlist, &offset, &py_boundary))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_TEXT_BOUNDARY, py_boundary,
                           (gpointer)&boundary))
        return NULL;

    text = atk_text_get_text_before_offset(ATK_TEXT(self->obj),
                                           offset, boundary, &start, &end);
    return Py_BuildValue("(s#ii)", text, (Py_ssize_t) (end - start), start, end);
}
#line 14343 "atk.c"


static PyObject *
_wrap_atk_text_get_caret_offset(PyGObject *self)
{
    int ret;

    
    ret = atk_text_get_caret_offset(ATK_TEXT(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 460 "atk.override"
static PyObject *
_wrap_atk_text_get_character_extents(PyGObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "coords", NULL };
    gint offset, x, y, width, height;
    AtkCoordType coords;
    PyObject *py_coords;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_character_extents",
                                     kwlist, &offset, &py_coords))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coords,
                           (gpointer)&coords))
        return NULL;

    atk_text_get_character_extents(ATK_TEXT(self->obj), offset,
                                   &x, &y, &width, &height, coords);
    return Py_BuildValue("(iiii)", x, y, width, height);
}
#line 14380 "atk.c"


#line 484 "atk.override"
static PyObject *
_wrap_atk_text_get_run_attributes(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "offset", NULL };
    gint offset, start, end;
    AtkAttributeSet *attribs;
    PyObject *py_attribs;
    gint count, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_run_attributes",
                                     kwlist, &offset))
	return NULL;

    attribs = atk_text_get_run_attributes(ATK_TEXT(self->obj), offset,
                                          &start, &end);
    count = g_slist_length(attribs);
    py_attribs = PyTuple_New(count);
    for (i = 0; i < count; i++) {
        AtkAttribute *attr = g_slist_nth_data(attribs, i);
        PyTuple_SetItem(py_attribs, i, Py_BuildValue("(ss)", attr->name,
                                                     attr->value));
    }
    atk_attribute_set_free(attribs);
    return Py_BuildValue("(Nii)", py_attribs, start, end);
}
#line 14411 "atk.c"


#line 513 "atk.override"
static PyObject *
_wrap_atk_text_get_default_attributes(PyGObject *self)
{
    AtkAttributeSet *attribs;
    PyObject *py_attribs;
    gint count, i;

    attribs = atk_text_get_default_attributes(ATK_TEXT(self->obj));
    count = g_slist_length(attribs);
    py_attribs = PyTuple_New(count);
    for (i = 0; i < count; i++) {
        AtkAttribute *attr = g_slist_nth_data(attribs, i);
        PyTuple_SetItem(py_attribs, i, Py_BuildValue("(ss)", attr->name,
                                                     attr->value));
    }
    atk_attribute_set_free(attribs);
    return py_attribs;
}
#line 14433 "atk.c"


static PyObject *
_wrap_atk_text_get_character_count(PyGObject *self)
{
    int ret;

    
    ret = atk_text_get_character_count(ATK_TEXT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_text_get_offset_at_point(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "coords", NULL };
    int x, y, ret;
    AtkCoordType coords;
    PyObject *py_coords = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiO:Atk.Text.get_offset_at_point", kwlist, &x, &y, &py_coords))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coords, (gpointer)&coords))
        return NULL;
    
    ret = atk_text_get_offset_at_point(ATK_TEXT(self->obj), x, y, coords);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_atk_text_get_n_selections(PyGObject *self)
{
    int ret;

    
    ret = atk_text_get_n_selections(ATK_TEXT(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 581 "atk.override"
static PyObject *
_wrap_atk_text_get_selection(PyGObject *self, PyObject *args,
                             PyObject *kwargs)
{
    static char *kwlist[] = { "selection_num", NULL };
    gchar *text;
    gint start, end, selection_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:atk.Text.get_selection",
                                     kwlist, &selection_num))
	return NULL;

    text = atk_text_get_selection(ATK_TEXT(self->obj),
                                  selection_num, &start, &end);
    return Py_BuildValue("(s#ii)", text, (Py_ssize_t) (end - start), start, end);
}
#line 14494 "atk.c"


static PyObject *
_wrap_atk_text_add_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_offset", "end_offset", NULL };
    int start_offset, end_offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Atk.Text.add_selection", kwlist, &start_offset, &end_offset))
        return NULL;
    
    ret = atk_text_add_selection(ATK_TEXT(self->obj), start_offset, end_offset);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_text_remove_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "selection_num", NULL };
    int selection_num, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Text.remove_selection", kwlist, &selection_num))
        return NULL;
    
    ret = atk_text_remove_selection(ATK_TEXT(self->obj), selection_num);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_text_set_selection(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "selection_num", "start_offset", "end_offset", NULL };
    int selection_num, start_offset, end_offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iii:Atk.Text.set_selection", kwlist, &selection_num, &start_offset, &end_offset))
        return NULL;
    
    ret = atk_text_set_selection(ATK_TEXT(self->obj), selection_num, start_offset, end_offset);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_atk_text_set_caret_offset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", NULL };
    int offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Atk.Text.set_caret_offset", kwlist, &offset))
        return NULL;
    
    ret = atk_text_set_caret_offset(ATK_TEXT(self->obj), offset);
    
    return PyBool_FromLong(ret);

}

#line 600 "atk.override"
static PyObject *
_wrap_atk_text_get_range_extents(PyGObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "start_offset", "end_offset", "coord_type",
                              NULL };
    gint start, end;
    AtkCoordType coord;
    AtkTextRectangle rect;
    PyObject *py_coord;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iiO:atk.Text.get_range_extents",
                                     kwlist, &start, &end, &py_coord))
	return NULL;

    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord,
                           (gpointer)&coord))
        return NULL;

    atk_text_get_range_extents(ATK_TEXT(self->obj), start, end, coord, &rect);
    return pyg_boxed_new(ATK_TYPE_RECTANGLE, (AtkRectangle*)&rect, TRUE, TRUE);
}
#line 14581 "atk.c"


#line 533 "atk.override"
static PyObject *
_wrap_atk_text_get_bounded_ranges(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "rect", "coord_type", "x_clip_type",
                              "y_clip_type", NULL };
    AtkRectangle rect;
    AtkCoordType coord;
    AtkTextClipType x_clip, y_clip;
    AtkTextRange **ranges, *range;
    PyObject *py_rect, *py_coord, *py_x_clip, *py_y_clip, *py_ranges;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "OOOO:atk.Text.get_bounded_ranges",
                                     kwlist, &py_rect, &py_coord, &py_x_clip,
                                     &py_y_clip))
	return NULL;

    if (pyatk_rectangle_from_pyobject(py_rect, &rect))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coord, (gpointer)&coord))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_TEXT_CLIP_TYPE, py_x_clip,
                           (gpointer)&x_clip))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_TEXT_CLIP_TYPE, py_y_clip,
                           (gpointer)&y_clip))
        return NULL;

    ranges = atk_text_get_bounded_ranges(ATK_TEXT(self->obj),
                                         (AtkTextRectangle*)&rect, coord,
                                         x_clip, y_clip);
    py_ranges = PyList_New(0);
    while ((range = *ranges++)) {
        PyObject *py_range;
        py_rect = pyg_boxed_new(ATK_TYPE_RECTANGLE,
                                (AtkRectangle*)&range->bounds, TRUE, TRUE);
        py_range = Py_BuildValue("(Niis)", py_rect, range->start_offset,
                                 range->end_offset, range->content);
        PyList_Append(py_ranges, py_range);
        Py_DECREF(py_range);
    }
    atk_text_free_ranges(ranges);

    return py_ranges;
}
#line 14631 "atk.c"


static PyObject *
_wrap_AtkText__do_get_text(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "start_offset", "end_offset", NULL };
    PyGObject *self;
    int start_offset, end_offset;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Text.get_text", kwlist, &PyAtkText_Type, &self, &start_offset, &end_offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_text)
        ret = iface->get_text(ATK_TEXT(self->obj), start_offset, end_offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_text not implemented");
        return NULL;
    }
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkText__do_get_character_at_offset(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "offset", NULL };
    PyGObject *self;
    int offset;
    gunichar ret;
    Py_UNICODE py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Text.get_character_at_offset", kwlist, &PyAtkText_Type, &self, &offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_character_at_offset)
        ret = iface->get_character_at_offset(ATK_TEXT(self->obj), offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_character_at_offset not implemented");
        return NULL;
    }
#if !defined(Py_UNICODE_SIZE) || Py_UNICODE_SIZE == 2
    if (ret > 0xffff) {
        PyErr_SetString(PyExc_RuntimeError, "returned character can not be represented in 16-bit unicode");
        return NULL;
    }
#endif
    py_ret = (Py_UNICODE)ret;
    return PyUnicode_FromUnicode(&py_ret, 1);

}

static PyObject *
_wrap_AtkText__do_get_caret_offset(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Text.get_caret_offset", kwlist, &PyAtkText_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_caret_offset)
        ret = iface->get_caret_offset(ATK_TEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_caret_offset not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkText__do_get_character_count(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Text.get_character_count", kwlist, &PyAtkText_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_character_count)
        ret = iface->get_character_count(ATK_TEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_character_count not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkText__do_get_offset_at_point(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "x", "y", "coords", NULL };
    PyGObject *self;
    int x, y, ret;
    PyObject *py_coords = NULL;
    AtkCoordType coords;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiO:Atk.Text.get_offset_at_point", kwlist, &PyAtkText_Type, &self, &x, &y, &py_coords))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_COORD_TYPE, py_coords, (gpointer)&coords))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_offset_at_point)
        ret = iface->get_offset_at_point(ATK_TEXT(self->obj), x, y, coords);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_offset_at_point not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkText__do_get_n_selections(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Text.get_n_selections", kwlist, &PyAtkText_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->get_n_selections)
        ret = iface->get_n_selections(ATK_TEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.get_n_selections not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_AtkText__do_add_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "start_offset", "end_offset", NULL };
    PyGObject *self;
    int start_offset, end_offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Text.add_selection", kwlist, &PyAtkText_Type, &self, &start_offset, &end_offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->add_selection)
        ret = iface->add_selection(ATK_TEXT(self->obj), start_offset, end_offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.add_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkText__do_remove_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "selection_num", NULL };
    PyGObject *self;
    int selection_num, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Text.remove_selection", kwlist, &PyAtkText_Type, &self, &selection_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->remove_selection)
        ret = iface->remove_selection(ATK_TEXT(self->obj), selection_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.remove_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkText__do_set_selection(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "selection_num", "start_offset", "end_offset", NULL };
    PyGObject *self;
    int selection_num, start_offset, end_offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iii:Atk.Text.set_selection", kwlist, &PyAtkText_Type, &self, &selection_num, &start_offset, &end_offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->set_selection)
        ret = iface->set_selection(ATK_TEXT(self->obj), selection_num, start_offset, end_offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.set_selection not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkText__do_set_caret_offset(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "offset", NULL };
    PyGObject *self;
    int offset, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Text.set_caret_offset", kwlist, &PyAtkText_Type, &self, &offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->set_caret_offset)
        ret = iface->set_caret_offset(ATK_TEXT(self->obj), offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.set_caret_offset not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_AtkText__do_text_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "position", "length", NULL };
    PyGObject *self;
    int position, length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Atk.Text.text_changed", kwlist, &PyAtkText_Type, &self, &position, &length))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->text_changed)
        iface->text_changed(ATK_TEXT(self->obj), position, length);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.text_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkText__do_text_caret_moved(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", "location", NULL };
    PyGObject *self;
    int location;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:Atk.Text.text_caret_moved", kwlist, &PyAtkText_Type, &self, &location))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->text_caret_moved)
        iface->text_caret_moved(ATK_TEXT(self->obj), location);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.text_caret_moved not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkText__do_text_selection_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Text.text_selection_changed", kwlist, &PyAtkText_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->text_selection_changed)
        iface->text_selection_changed(ATK_TEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.text_selection_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_AtkText__do_text_attributes_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    AtkTextIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Atk.Text.text_attributes_changed", kwlist, &PyAtkText_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), ATK_TYPE_TEXT);
    if (iface->text_attributes_changed)
        iface->text_attributes_changed(ATK_TEXT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method Atk.Text.text_attributes_changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyAtkText_methods[] = {
    { "get_text", (PyCFunction)_wrap_atk_text_get_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_character_at_offset", (PyCFunction)_wrap_atk_text_get_character_at_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_text_after_offset", (PyCFunction)_wrap_atk_text_get_text_after_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_text_at_offset", (PyCFunction)_wrap_atk_text_get_text_at_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_text_before_offset", (PyCFunction)_wrap_atk_text_get_text_before_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_caret_offset", (PyCFunction)_wrap_atk_text_get_caret_offset, METH_NOARGS,
      NULL },
    { "get_character_extents", (PyCFunction)_wrap_atk_text_get_character_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_run_attributes", (PyCFunction)_wrap_atk_text_get_run_attributes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_default_attributes", (PyCFunction)_wrap_atk_text_get_default_attributes, METH_NOARGS,
      NULL },
    { "get_character_count", (PyCFunction)_wrap_atk_text_get_character_count, METH_NOARGS,
      NULL },
    { "get_offset_at_point", (PyCFunction)_wrap_atk_text_get_offset_at_point, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_n_selections", (PyCFunction)_wrap_atk_text_get_n_selections, METH_NOARGS,
      NULL },
    { "get_selection", (PyCFunction)_wrap_atk_text_get_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_selection", (PyCFunction)_wrap_atk_text_add_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_selection", (PyCFunction)_wrap_atk_text_remove_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_selection", (PyCFunction)_wrap_atk_text_set_selection, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_caret_offset", (PyCFunction)_wrap_atk_text_set_caret_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_range_extents", (PyCFunction)_wrap_atk_text_get_range_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_bounded_ranges", (PyCFunction)_wrap_atk_text_get_bounded_ranges, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_text", (PyCFunction)_wrap_AtkText__do_get_text, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_character_at_offset", (PyCFunction)_wrap_AtkText__do_get_character_at_offset, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_caret_offset", (PyCFunction)_wrap_AtkText__do_get_caret_offset, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_character_count", (PyCFunction)_wrap_AtkText__do_get_character_count, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_offset_at_point", (PyCFunction)_wrap_AtkText__do_get_offset_at_point, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_selections", (PyCFunction)_wrap_AtkText__do_get_n_selections, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_add_selection", (PyCFunction)_wrap_AtkText__do_add_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_selection", (PyCFunction)_wrap_AtkText__do_remove_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_selection", (PyCFunction)_wrap_AtkText__do_set_selection, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_caret_offset", (PyCFunction)_wrap_AtkText__do_set_caret_offset, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_text_changed", (PyCFunction)_wrap_AtkText__do_text_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_text_caret_moved", (PyCFunction)_wrap_AtkText__do_text_caret_moved, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_text_selection_changed", (PyCFunction)_wrap_AtkText__do_text_selection_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_text_attributes_changed", (PyCFunction)_wrap_AtkText__do_text_attributes_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkText_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Text",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkText_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gchar*
_wrap_AtkText__proxy_do_get_text(AtkText *self, gint start_offset, gint end_offset)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_start_offset;
    PyObject *py_end_offset;
    gchar* retval;
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
    py_start_offset = PyInt_FromLong(start_offset);
    py_end_offset = PyInt_FromLong(end_offset);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_start_offset);
    PyTuple_SET_ITEM(py_args, 1, py_end_offset);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_text");
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
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "s", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    retval = g_strdup(retval);
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkText__proxy_do_get_caret_offset(AtkText *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_caret_offset");
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
static gint
_wrap_AtkText__proxy_do_get_character_count(AtkText *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_character_count");
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
static gint
_wrap_AtkText__proxy_do_get_offset_at_point(AtkText *self, gint x, gint y, AtkCoordType coords)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_coords;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_coords = pyg_enum_from_gtype(ATK_TYPE_COORD_TYPE, coords);
    if (!py_coords) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_y);
        Py_DECREF(py_x);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_coords);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_offset_at_point");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
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
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_AtkText__proxy_do_get_n_selections(AtkText *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_selections");
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
static gboolean
_wrap_AtkText__proxy_do_add_selection(AtkText *self, gint start_offset, gint end_offset)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_start_offset;
    PyObject *py_end_offset;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_start_offset = PyInt_FromLong(start_offset);
    py_end_offset = PyInt_FromLong(end_offset);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_start_offset);
    PyTuple_SET_ITEM(py_args, 1, py_end_offset);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkText__proxy_do_remove_selection(AtkText *self, gint selection_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_selection_num;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_selection_num = PyInt_FromLong(selection_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_selection_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkText__proxy_do_set_selection(AtkText *self, gint selection_num, gint start_offset, gint end_offset)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_selection_num;
    PyObject *py_start_offset;
    PyObject *py_end_offset;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_selection_num = PyInt_FromLong(selection_num);
    py_start_offset = PyInt_FromLong(start_offset);
    py_end_offset = PyInt_FromLong(end_offset);
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_selection_num);
    PyTuple_SET_ITEM(py_args, 1, py_start_offset);
    PyTuple_SET_ITEM(py_args, 2, py_end_offset);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_selection");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_AtkText__proxy_do_set_caret_offset(AtkText *self, gint offset)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_offset;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_offset = PyInt_FromLong(offset);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_offset);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_caret_offset");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
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
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_AtkText__proxy_do_text_changed(AtkText *self, gint position, gint length)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_position;
    PyObject *py_length;
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
    py_position = PyInt_FromLong(position);
    py_length = PyInt_FromLong(length);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_position);
    PyTuple_SET_ITEM(py_args, 1, py_length);
    
    py_method = PyObject_GetAttrString(py_self, "do_text_changed");
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
_wrap_AtkText__proxy_do_text_caret_moved(AtkText *self, gint location)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_location;
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
    py_location = PyInt_FromLong(location);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_location);
    
    py_method = PyObject_GetAttrString(py_self, "do_text_caret_moved");
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
_wrap_AtkText__proxy_do_text_selection_changed(AtkText *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_text_selection_changed");
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
_wrap_AtkText__proxy_do_text_attributes_changed(AtkText *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_text_attributes_changed");
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
__AtkText__interface_init(AtkTextIface *iface, PyTypeObject *pytype)
{
    AtkTextIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_text") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_text = _wrap_AtkText__proxy_do_get_text;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_text = parent_iface->get_text;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_caret_offset") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_caret_offset = _wrap_AtkText__proxy_do_get_caret_offset;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_caret_offset = parent_iface->get_caret_offset;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_character_count") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_character_count = _wrap_AtkText__proxy_do_get_character_count;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_character_count = parent_iface->get_character_count;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_offset_at_point") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_offset_at_point = _wrap_AtkText__proxy_do_get_offset_at_point;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_offset_at_point = parent_iface->get_offset_at_point;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_selections") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_selections = _wrap_AtkText__proxy_do_get_n_selections;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_selections = parent_iface->get_n_selections;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_selection = _wrap_AtkText__proxy_do_add_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_selection = parent_iface->add_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_selection = _wrap_AtkText__proxy_do_remove_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_selection = parent_iface->remove_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_selection") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_selection = _wrap_AtkText__proxy_do_set_selection;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_selection = parent_iface->set_selection;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_caret_offset") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_caret_offset = _wrap_AtkText__proxy_do_set_caret_offset;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_caret_offset = parent_iface->set_caret_offset;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_text_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->text_changed = _wrap_AtkText__proxy_do_text_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->text_changed = parent_iface->text_changed;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_text_caret_moved") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->text_caret_moved = _wrap_AtkText__proxy_do_text_caret_moved;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->text_caret_moved = parent_iface->text_caret_moved;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_text_selection_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->text_selection_changed = _wrap_AtkText__proxy_do_text_selection_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->text_selection_changed = parent_iface->text_selection_changed;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_text_attributes_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->text_attributes_changed = _wrap_AtkText__proxy_do_text_attributes_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->text_attributes_changed = parent_iface->text_attributes_changed;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __AtkText__iinfo = {
    (GInterfaceInitFunc) __AtkText__interface_init,
    NULL,
    NULL
};


/* ----------- AtkValue ----------- */

#line 625 "atk.override"
static PyObject *
_wrap_atk_value_get_current_value(PyGObject *self)
{
    GValue value = { 0, };
    PyObject *ret;

    atk_value_get_current_value(ATK_VALUE(self->obj), &value);
    ret = pyg_value_as_pyobject(&value, TRUE);
    g_value_unset(&value);
    return ret;
}
#line 16040 "atk.c"


#line 638 "atk.override"
static PyObject *
_wrap_atk_value_get_maximum_value(PyGObject *self)
{
    GValue value = { 0, };
    PyObject *ret;

    atk_value_get_maximum_value(ATK_VALUE(self->obj), &value);
    ret = pyg_value_as_pyobject(&value, TRUE);
    g_value_unset(&value);
    return ret;
}
#line 16055 "atk.c"


#line 651 "atk.override"
static PyObject *
_wrap_atk_value_get_minimum_value(PyGObject *self)
{
    GValue value = { 0, };
    PyObject *ret;

    atk_value_get_minimum_value(ATK_VALUE(self->obj), &value);
    ret = pyg_value_as_pyobject(&value, TRUE);
    g_value_unset(&value);
    return ret;
}
#line 16070 "atk.c"


#line 664 "atk.override"
static PyObject *
_wrap_atk_value_set_current_value(PyGObject *self, PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "value", NULL };
    GValue value = { 0, };
    PyObject *ret, *py_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:atk.Value.set_current_value",
                                     kwlist, &py_value))
	return NULL;

    if (pyg_value_from_pyobject(&value, py_value)) {
        PyErr_SetString(PyExc_TypeError, "unable to convert value");
        return NULL;
    }

    if (atk_value_set_current_value(ATK_VALUE(self->obj), &value))
        ret = Py_True;
    else
        ret = Py_False;

    g_value_unset(&value);
    Py_INCREF(ret);
    return ret;
}
#line 16101 "atk.c"


static const PyMethodDef _PyAtkValue_methods[] = {
    { "get_current_value", (PyCFunction)_wrap_atk_value_get_current_value, METH_NOARGS,
      NULL },
    { "get_maximum_value", (PyCFunction)_wrap_atk_value_get_maximum_value, METH_NOARGS,
      NULL },
    { "get_minimum_value", (PyCFunction)_wrap_atk_value_get_minimum_value, METH_NOARGS,
      NULL },
    { "set_current_value", (PyCFunction)_wrap_atk_value_set_current_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyAtkValue_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "atk.Value",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyAtkValue_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_atk_gobject_accessible_for_object(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "obj", NULL };
    PyGObject *obj;
    AtkObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gobject_accessible_for_object", kwlist, &PyGObject_Type, &obj))
        return NULL;
    
    ret = atk_gobject_accessible_for_object(G_OBJECT(obj->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_role_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:role_register", kwlist, &name))
        return NULL;
    
    ret = atk_role_register(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_ROLE, ret);
}

static PyObject *
_wrap_atk_role_get_localized_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "role", NULL };
    PyObject *py_role = NULL;
    AtkRole role;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:role_get_localized_name", kwlist, &py_role))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_ROLE, py_role, (gpointer)&role))
        return NULL;
    
    ret = atk_role_get_localized_name(role);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_get_default_registry(PyObject *self)
{
    AtkRegistry *ret;

    
    ret = atk_get_default_registry();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_relation_type_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:relation_type_register", kwlist, &name))
        return NULL;
    
    ret = atk_relation_type_register(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_RELATION_TYPE, ret);
}

static PyObject *
_wrap_atk_relation_type_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relationship", NULL };
    PyObject *py_relationship = NULL;
    AtkRelationType relationship;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:relation_type_get_name", kwlist, &py_relationship))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_RELATION_TYPE, py_relationship, (gpointer)&relationship))
        return NULL;
    
    ret = atk_relation_type_get_name(relationship);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_relation_type_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:relation_type_for_name", kwlist, &name))
        return NULL;
    
    ret = atk_relation_type_for_name(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_RELATION_TYPE, ret);
}

static PyObject *
_wrap_atk_state_type_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:state_type_register", kwlist, &name))
        return NULL;
    
    ret = atk_state_type_register(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_STATE_TYPE, ret);
}

static PyObject *
_wrap_atk_state_type_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    AtkStateType type;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:state_type_get_name", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_STATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    ret = atk_state_type_get_name(type);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_state_type_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:state_type_for_name", kwlist, &name))
        return NULL;
    
    ret = atk_state_type_for_name(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_STATE_TYPE, ret);
}

static PyObject *
_wrap_atk_text_attribute_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attr", NULL };
    PyObject *py_attr = NULL;
    AtkTextAttribute attr;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:text_attribute_get_name", kwlist, &py_attr))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_TEXT_ATTRIBUTE, py_attr, (gpointer)&attr))
        return NULL;
    
    ret = atk_text_attribute_get_name(attr);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_text_attribute_get_value(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attr", "index", NULL };
    PyObject *py_attr = NULL;
    AtkTextAttribute attr;
    int index;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:text_attribute_get_value", kwlist, &py_attr, &index))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_TEXT_ATTRIBUTE, py_attr, (gpointer)&attr))
        return NULL;
    
    ret = atk_text_attribute_get_value(attr, index);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_text_attribute_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:text_attribute_register", kwlist, &name))
        return NULL;
    
    ret = atk_text_attribute_register(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_TEXT_ATTRIBUTE, ret);
}

static PyObject *
_wrap_atk_text_attribute_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:text_attribute_for_name", kwlist, &name))
        return NULL;
    
    ret = atk_text_attribute_for_name(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_TEXT_ATTRIBUTE, ret);
}

static PyObject *
_wrap_atk_remove_focus_tracker(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tracker_id", NULL };
    PyObject *py_tracker_id = NULL;
    guint tracker_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:remove_focus_tracker", kwlist, &py_tracker_id))
        return NULL;
    if (py_tracker_id) {
        if (PyLong_Check(py_tracker_id))
            tracker_id = PyLong_AsUnsignedLong(py_tracker_id);
        else if (PyInt_Check(py_tracker_id))
            tracker_id = PyInt_AsLong(py_tracker_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'tracker_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    atk_remove_focus_tracker(tracker_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_focus_tracker_notify(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", NULL };
    PyGObject *object;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:focus_tracker_notify", kwlist, &PyAtkObject_Type, &object))
        return NULL;
    
    atk_focus_tracker_notify(ATK_OBJECT(object->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_remove_global_event_listener(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "listener_id", NULL };
    PyObject *py_listener_id = NULL;
    guint listener_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:remove_global_event_listener", kwlist, &py_listener_id))
        return NULL;
    if (py_listener_id) {
        if (PyLong_Check(py_listener_id))
            listener_id = PyLong_AsUnsignedLong(py_listener_id);
        else if (PyInt_Check(py_listener_id))
            listener_id = PyInt_AsLong(py_listener_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'listener_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    atk_remove_global_event_listener(listener_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_remove_key_event_listener(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "listener_id", NULL };
    PyObject *py_listener_id = NULL;
    guint listener_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:remove_key_event_listener", kwlist, &py_listener_id))
        return NULL;
    if (py_listener_id) {
        if (PyLong_Check(py_listener_id))
            listener_id = PyLong_AsUnsignedLong(py_listener_id);
        else if (PyInt_Check(py_listener_id))
            listener_id = PyInt_AsLong(py_listener_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'listener_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    atk_remove_key_event_listener(listener_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_get_root(PyObject *self)
{
    AtkObject *ret;

    
    ret = atk_get_root();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_get_toolkit_name(PyObject *self)
{
    const gchar *ret;

    
    ret = atk_get_toolkit_name();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_get_toolkit_version(PyObject *self)
{
    const gchar *ret;

    
    ret = atk_get_toolkit_version();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_get_focus_object(PyObject *self)
{
    AtkObject *ret;

    
    ret = atk_get_focus_object();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_atk_role_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "role", NULL };
    PyObject *py_role = NULL;
    AtkRole role;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:role_get_name", kwlist, &py_role))
        return NULL;
    if (pyg_enum_get_value(ATK_TYPE_ROLE, py_role, (gpointer)&role))
        return NULL;
    
    ret = atk_role_get_name(role);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_atk_role_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:role_for_name", kwlist, &name))
        return NULL;
    
    ret = atk_role_for_name(name);
    
    return pyg_enum_from_gtype(ATK_TYPE_ROLE, ret);
}

const PyMethodDef pyatk_functions[] = {
    { "gobject_accessible_for_object", (PyCFunction)_wrap_atk_gobject_accessible_for_object, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "role_register", (PyCFunction)_wrap_atk_role_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "role_get_localized_name", (PyCFunction)_wrap_atk_role_get_localized_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_default_registry", (PyCFunction)_wrap_atk_get_default_registry, METH_NOARGS,
      NULL },
    { "relation_type_register", (PyCFunction)_wrap_atk_relation_type_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "relation_type_get_name", (PyCFunction)_wrap_atk_relation_type_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "relation_type_for_name", (PyCFunction)_wrap_atk_relation_type_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "state_type_register", (PyCFunction)_wrap_atk_state_type_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "state_type_get_name", (PyCFunction)_wrap_atk_state_type_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "state_type_for_name", (PyCFunction)_wrap_atk_state_type_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "text_attribute_get_name", (PyCFunction)_wrap_atk_text_attribute_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "text_attribute_get_value", (PyCFunction)_wrap_atk_text_attribute_get_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "text_attribute_register", (PyCFunction)_wrap_atk_text_attribute_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "text_attribute_for_name", (PyCFunction)_wrap_atk_text_attribute_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_focus_tracker", (PyCFunction)_wrap_atk_remove_focus_tracker, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "focus_tracker_notify", (PyCFunction)_wrap_atk_focus_tracker_notify, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_global_event_listener", (PyCFunction)_wrap_atk_remove_global_event_listener, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_key_event_listener", (PyCFunction)_wrap_atk_remove_key_event_listener, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_root", (PyCFunction)_wrap_atk_get_root, METH_NOARGS,
      NULL },
    { "get_toolkit_name", (PyCFunction)_wrap_atk_get_toolkit_name, METH_NOARGS,
      NULL },
    { "get_toolkit_version", (PyCFunction)_wrap_atk_get_toolkit_version, METH_NOARGS,
      NULL },
    { "get_focus_object", (PyCFunction)_wrap_atk_get_focus_object, METH_NOARGS,
      NULL },
    { "role_get_name", (PyCFunction)_wrap_atk_role_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "role_for_name", (PyCFunction)_wrap_atk_role_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pyatk_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_enum_add(module, "Role", strip_prefix, ATK_TYPE_ROLE);
  pyg_enum_add(module, "Layer", strip_prefix, ATK_TYPE_LAYER);
  pyg_enum_add(module, "RelationType", strip_prefix, ATK_TYPE_RELATION_TYPE);
  pyg_enum_add(module, "StateType", strip_prefix, ATK_TYPE_STATE_TYPE);
  pyg_enum_add(module, "TextAttribute", strip_prefix, ATK_TYPE_TEXT_ATTRIBUTE);
  pyg_enum_add(module, "TextBoundary", strip_prefix, ATK_TYPE_TEXT_BOUNDARY);
  pyg_enum_add(module, "KeyEventType", strip_prefix, ATK_TYPE_KEY_EVENT_TYPE);
  pyg_enum_add(module, "CoordType", strip_prefix, ATK_TYPE_COORD_TYPE);
  pyg_flags_add(module, "HyperlinkStateFlags", strip_prefix, ATK_TYPE_HYPERLINK_STATE_FLAGS);
  pyg_enum_add(module, "TextClipType", strip_prefix, ATK_TYPE_TEXT_CLIP_TYPE);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pyatk_register_classes(PyObject *d)
{
    PyObject *module;

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


#line 16681 "atk.c"
    pyg_register_boxed(d, "Rectangle", ATK_TYPE_RECTANGLE, &PyAtkRectangle_Type);
    pyg_register_interface(d, "Action", ATK_TYPE_ACTION, &PyAtkAction_Type);
    pyg_register_interface_info(ATK_TYPE_ACTION, &__AtkAction__iinfo);
    pyg_register_interface(d, "Component", ATK_TYPE_COMPONENT, &PyAtkComponent_Type);
    pyg_register_interface_info(ATK_TYPE_COMPONENT, &__AtkComponent__iinfo);
    pyg_register_interface(d, "Document", ATK_TYPE_DOCUMENT, &PyAtkDocument_Type);
    pyg_register_interface_info(ATK_TYPE_DOCUMENT, &__AtkDocument__iinfo);
    pyg_register_interface(d, "EditableText", ATK_TYPE_EDITABLE_TEXT, &PyAtkEditableText_Type);
    pyg_register_interface_info(ATK_TYPE_EDITABLE_TEXT, &__AtkEditableText__iinfo);
    pyg_register_interface(d, "Hypertext", ATK_TYPE_HYPERTEXT, &PyAtkHypertext_Type);
    pyg_register_interface_info(ATK_TYPE_HYPERTEXT, &__AtkHypertext__iinfo);
    pyg_register_interface(d, "Image", ATK_TYPE_IMAGE, &PyAtkImage_Type);
    pyg_register_interface_info(ATK_TYPE_IMAGE, &__AtkImage__iinfo);
    pyg_register_interface(d, "ImplementorIface", ATK_TYPE_IMPLEMENTOR, &PyAtkImplementorIface_Type);
    pyg_register_interface(d, "Selection", ATK_TYPE_SELECTION, &PyAtkSelection_Type);
    pyg_register_interface_info(ATK_TYPE_SELECTION, &__AtkSelection__iinfo);
    pyg_register_interface(d, "StreamableContent", ATK_TYPE_STREAMABLE_CONTENT, &PyAtkStreamableContent_Type);
    pyg_register_interface_info(ATK_TYPE_STREAMABLE_CONTENT, &__AtkStreamableContent__iinfo);
    pyg_register_interface(d, "Table", ATK_TYPE_TABLE, &PyAtkTable_Type);
    pyg_register_interface_info(ATK_TYPE_TABLE, &__AtkTable__iinfo);
    pyg_register_interface(d, "Text", ATK_TYPE_TEXT, &PyAtkText_Type);
    pyg_register_interface_info(ATK_TYPE_TEXT, &__AtkText__iinfo);
    pyg_register_interface(d, "Value", ATK_TYPE_VALUE, &PyAtkValue_Type);
    pygobject_register_class(d, "AtkHyperlink", ATK_TYPE_HYPERLINK, &PyAtkHyperlink_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_HYPERLINK);
    pyg_register_class_init(ATK_TYPE_HYPERLINK, __AtkHyperlink_class_init);
    pygobject_register_class(d, "AtkObject", ATK_TYPE_OBJECT, &PyAtkObject_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_OBJECT);
    pyg_register_class_init(ATK_TYPE_OBJECT, __AtkObject_class_init);
    pygobject_register_class(d, "AtkGObjectAccessible", ATK_TYPE_GOBJECT_ACCESSIBLE, &PyAtkGObjectAccessible_Type, Py_BuildValue("(O)", &PyAtkObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_GOBJECT_ACCESSIBLE);
    pygobject_register_class(d, "AtkNoOpObject", ATK_TYPE_NO_OP_OBJECT, &PyAtkNoOpObject_Type, Py_BuildValue("(O)", &PyAtkObject_Type));
    pygobject_register_class(d, "AtkObjectFactory", ATK_TYPE_OBJECT_FACTORY, &PyAtkObjectFactory_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_OBJECT_FACTORY);
    pyg_register_class_init(ATK_TYPE_OBJECT_FACTORY, __AtkObjectFactory_class_init);
    pygobject_register_class(d, "AtkNoOpObjectFactory", ATK_TYPE_NO_OP_OBJECT_FACTORY, &PyAtkNoOpObjectFactory_Type, Py_BuildValue("(O)", &PyAtkObjectFactory_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_NO_OP_OBJECT_FACTORY);
    pygobject_register_class(d, "AtkRegistry", ATK_TYPE_REGISTRY, &PyAtkRegistry_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_REGISTRY);
    pygobject_register_class(d, "AtkRelation", ATK_TYPE_RELATION, &PyAtkRelation_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "AtkRelationSet", ATK_TYPE_RELATION_SET, &PyAtkRelationSet_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_RELATION_SET);
    pygobject_register_class(d, "AtkStateSet", ATK_TYPE_STATE_SET, &PyAtkStateSet_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_STATE_SET);
    pygobject_register_class(d, "AtkUtil", ATK_TYPE_UTIL, &PyAtkUtil_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(ATK_TYPE_UTIL);
}
