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


#line 24 "pango.override"
#define NO_IMPORT_PYGOBJECT
#define PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_ENGINE
#include <pygobject.h>
#include <pango/pango.h>

typedef struct {
    PyObject *func, *data;
} PyGtkCustomNotify;

#ifndef PANGO_TYPE_LAYOUT_LINE
# define PANGO_TYPE_LAYOUT_LINE pypango_layout_line_get_type()

static PangoLayoutLine *
_layout_line_boxed_copy(PangoLayoutLine *line)
{
    pango_layout_line_ref(line);
    return line;
}

static GType
pypango_layout_line_get_type(void)
{
    static GType our_type = 0;
  
    if (our_type == 0)
        our_type = g_boxed_type_register_static("PangoLayoutLine",
                                                (GBoxedCopyFunc)_layout_line_boxed_copy,
                                                (GBoxedFreeFunc)pango_layout_line_unref);
    return our_type;
}
#endif /* #ifndef PANGO_TYPE_LAYOUT_LINE */

#ifndef PANGO_TYPE_ITEM
# define PANGO_TYPE_ITEM (pypango_item_get_type ())

static GType
pypango_item_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("PangoItem",
                                             (GBoxedCopyFunc) pango_item_copy,
                                             (GBoxedFreeFunc) pango_item_free);
  return our_type;
}
#endif /* #ifndef PANGO_TYPE_ITEM */

/* ------------- PangoAttribute ------------- */

typedef struct {
    PyObject_HEAD
    PangoAttribute *attr;
} PyPangoAttribute;
staticforward PyTypeObject PyPangoAttribute_Type;

static PyObject *
pypango_attr_new(PangoAttribute *attr, guint start, guint end)
{
    PyPangoAttribute *self;

    self = (PyPangoAttribute *)PyObject_NEW(PyPangoAttribute,
					    &PyPangoAttribute_Type);
    if (self == NULL)
	return NULL;
    self->attr = attr;
    attr->start_index = start;
    attr->end_index = end;

    return (PyObject *)self;
}

static void
pypango_attr_dealloc(PyPangoAttribute *self)
{
    pango_attribute_destroy(self->attr);
    PyObject_DEL(self);
}

static int
pypango_attr_compare(PyPangoAttribute *self, PyPangoAttribute *v)
{
    if (pango_attribute_equal(self->attr, v->attr))
	return 0;
    if (self->attr > v->attr)
	return -1;
    return 1;
}

static long
pypango_attr_hash(PyPangoAttribute *self)
{
    return (long)self->attr;
}

static PyObject *
pypango_attr_copy(PyPangoAttribute *self)
{
    return pypango_attr_new(pango_attribute_copy(self->attr),
			    self->attr->start_index, self->attr->end_index);
}

static PyMethodDef pypango_attr_methods[] = {
    { "copy", (PyCFunction)pypango_attr_copy, METH_NOARGS },
    { NULL, NULL, 0 }
};

static PyObject *
pypango_attr_get_index(PyPangoAttribute *self, void *closure)
{
    gboolean is_end = GPOINTER_TO_INT(closure) != 0;

    if (is_end)
	return PyInt_FromLong(self->attr->end_index);
    else
	return PyInt_FromLong(self->attr->start_index);
}

static int
pypango_attr_set_index(PyPangoAttribute *self, PyObject *value, void *closure)
{
    gboolean is_end = GPOINTER_TO_INT(closure) != 0;
    gint val;

    val = PyInt_AsLong(value);
    if (PyErr_Occurred()) {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError, "index must be an integer");
	return -1;
    }
    if (is_end)
	self->attr->end_index = val;
    else
	self->attr->start_index = val;
    return 0;
}

static PyObject *
pypango_attr_get_type(PyPangoAttribute *self, void *closure)
{
    return PyInt_FromLong(self->attr->klass->type);
}

static PyGetSetDef pypango_attr_getsets[] = {
    { "start_index", (getter)pypango_attr_get_index,
      (setter)pypango_attr_set_index, NULL, GINT_TO_POINTER(0) },
    { "end_index", (getter)pypango_attr_get_index,
      (setter)pypango_attr_set_index, NULL, GINT_TO_POINTER(1) },
    { "type", (getter)pypango_attr_get_type, (setter)0, NULL, NULL },
    { NULL, (getter)0, (setter)0, NULL, NULL }
};

static PyObject *
pypango_attr_tp_getattr(PyPangoAttribute *self, char *attr)
{
    PangoAttribute *attribute = self->attr;
    PyObject *name, *ret;

    switch (attribute->klass->type) {
    case PANGO_ATTR_LANGUAGE:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "value");
	if (!strcmp(attr, "value"))
	    return pyg_boxed_new(PANGO_TYPE_LANGUAGE,
				 ((PangoAttrLanguage *)attribute)->value,
				 TRUE, TRUE);
	break;
    case PANGO_ATTR_FAMILY:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "value");
	if (!strcmp(attr, "value"))
	    return PyString_FromString(((PangoAttrString *)attribute)->value);
	break;
    case PANGO_ATTR_STYLE:
    case PANGO_ATTR_WEIGHT:
    case PANGO_ATTR_VARIANT:
    case PANGO_ATTR_STRETCH:
    case PANGO_ATTR_SIZE:
    case PANGO_ATTR_UNDERLINE:
    case PANGO_ATTR_STRIKETHROUGH:
    case PANGO_ATTR_RISE:
    case PANGO_ATTR_FALLBACK:
    case PANGO_ATTR_LETTER_SPACING:
    case PANGO_ATTR_ABSOLUTE_SIZE:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "value");
	if (!strcmp(attr, "value"))
	    return PyInt_FromLong(((PangoAttrInt *)attribute)->value);
	break;
    case PANGO_ATTR_FONT_DESC:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "desc");
	if (!strcmp(attr, "desc"))
	    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION,
				 ((PangoAttrFontDesc *)attribute)->desc,
				 TRUE, TRUE);
	break;
    case PANGO_ATTR_FOREGROUND:
    case PANGO_ATTR_BACKGROUND:
    case PANGO_ATTR_UNDERLINE_COLOR:
    case PANGO_ATTR_STRIKETHROUGH_COLOR:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "color");
	if (!strcmp(attr, "color"))
	    return pyg_boxed_new(PANGO_TYPE_COLOR,
				 &((PangoAttrColor *)attribute)->color,
				 TRUE, TRUE);
	break;
    case PANGO_ATTR_SHAPE:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[ss]", "ink_rect", "logical_rect");
	if (!strcmp(attr, "ink_rect")) {
	    PangoRectangle rect = ((PangoAttrShape *)attribute)->ink_rect;

	    return Py_BuildValue("iiii", rect.x, rect.y,
				 rect.width, rect.height);
	}
	if (!strcmp(attr, "logical_rect")) {
	    PangoRectangle rect = ((PangoAttrShape *)attribute)->logical_rect;

	    return Py_BuildValue("iiii", rect.x, rect.y,
				 rect.width, rect.height);
	}
	break;
    case PANGO_ATTR_SCALE:
	if (!strcmp(attr, "__members__"))
	    return Py_BuildValue("[s]", "value");
	if (!strcmp(attr, "value"))
	    return PyFloat_FromDouble(((PangoAttrFloat *)attribute)->value);
	break;
    default:
	break;
    }

    name = PyString_FromString(attr);
    ret = PyObject_GenericGetAttr((PyObject *)self, name);
    Py_DECREF(name);
    return ret;
}

static PyTypeObject PyPangoAttribute_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "pango.Attribute",			/* tp_name */
    sizeof(PyPangoAttribute),		/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)pypango_attr_dealloc,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)pypango_attr_tp_getattr,	/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)pypango_attr_compare,	/* tp_compare */
    (reprfunc)0,			/* tp_repr */
    0,					/* tp_as_number */
    0,					/* tp_as_sequence */
    0,					/* tp_as_mapping */
    (hashfunc)pypango_attr_hash,	/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)0,			/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)0,			/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    pypango_attr_methods,		/* tp_methods */
    0,					/* tp_members */
    pypango_attr_getsets,		/* tp_getset */
    (PyTypeObject *)0,			/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)0,			/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    0,					/* tp_free */
    (inquiry)0,				/* tp_is_gc */
    (PyObject *)0,			/* tp_bases */
};

/* ------------- PangoAttrIterator ------------- */

typedef struct {
    PyObject_HEAD
    PangoAttrIterator *iter;
} PyPangoAttrIterator;
staticforward PyTypeObject PyPangoAttrIterator_Type;

static PyObject *
pypango_attr_iterator_new(PangoAttrIterator *iter)
{
    PyPangoAttrIterator *self;

    self = (PyPangoAttrIterator *)PyObject_NEW(PyPangoAttrIterator,
					   &PyPangoAttrIterator_Type);
    if (self == NULL)
	return NULL;
    self->iter = iter;

    return (PyObject *)self;
}

static void
pypango_attr_iterator_dealloc(PyPangoAttrIterator *self)
{
    pango_attr_iterator_destroy(self->iter);
    PyObject_DEL(self);
}

static int
pypango_attr_iterator_compare(PyPangoAttrIterator *self,
			      PyPangoAttrIterator *v)
{
    if (self->iter == v->iter)
	return 0;
    if (self->iter > v->iter)
	return -1;
    return 1;
}

static long
pypango_attr_iterator_hash(PyPangoAttrIterator *self)
{
    return (long)self->iter;
}

static PyObject *
pypango_attr_iterator_copy(PyPangoAttrIterator *self)
{
    return pypango_attr_iterator_new(pango_attr_iterator_copy(self->iter));
}

static PyObject *
pypango_attr_iterator_range(PyPangoAttrIterator *self)
{
    gint start, end;

    pango_attr_iterator_range(self->iter, &start, &end);
    return Py_BuildValue("ii", start, end);
}

static PyObject *
pypango_attr_iterator_next(PyPangoAttrIterator *self)
{
    return PyBool_FromLong(pango_attr_iterator_next(self->iter));
}

static PyObject *
pypango_attr_iterator_get(PyPangoAttrIterator *self, PyObject *args,
			  PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type;
    PangoAttrType type;
    PangoAttribute *attr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:pango.AttrIterator.get",
				     kwlist, &py_type))
	return NULL;

    if (pyg_enum_get_value(PANGO_TYPE_ATTR_TYPE, py_type, (gint*)&type))
	return NULL;

    if (!(attr = pango_attr_iterator_get(self->iter, type))) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    return pypango_attr_new(attr, attr->start_index, attr->end_index);
}

static PyObject *
pypango_attr_iterator_get_font(PyPangoAttrIterator *self)
{
    PangoFontDescription *desc;
    PangoLanguage *language;
    GSList *extra_attrs, *tmp;
    PyObject *py_desc, *py_language, *py_extra_attrs;

    if (!(desc = pango_font_description_new())) {
	PyErr_SetString(PyExc_RuntimeError, "can't get font info");
	return NULL;
    }
    pango_attr_iterator_get_font(self->iter, desc, &language, &extra_attrs);
    py_desc = pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, desc, TRUE, TRUE);
    py_language = pyg_boxed_new(PANGO_TYPE_LANGUAGE, language, TRUE, TRUE);

    py_extra_attrs = PyList_New(0);
    for (tmp = extra_attrs; tmp != NULL; tmp = tmp->next) {
	PangoAttribute *attr = (PangoAttribute *)tmp->data;
	PyObject *py_attr = pypango_attr_new(attr, attr->start_index,
					  attr->end_index);
	PyList_Append(py_extra_attrs, py_attr);
	Py_DECREF(py_attr);
    }
    g_slist_free(extra_attrs);

    return Py_BuildValue("NNN", py_desc, py_language, py_extra_attrs);
}

static PyObject *
pypango_attr_iterator_get_attrs(PyPangoAttrIterator *self)
{
    GSList *alist;
    PyObject *py_list;
    guint i, len;

    alist = pango_attr_iterator_get_attrs(self->iter);

    len = g_slist_length(alist);
    py_list = PyTuple_New(len);
    for (i = 0; i < len; i++) {
	PangoAttribute *attr = (PangoAttribute *)g_slist_nth_data(alist, i);
	
	PyTuple_SetItem(py_list, i, pypango_attr_new(attr, attr->start_index,
						     attr->end_index));
    }
    /* don't have to destroy attributes since we use them */
    g_slist_free(alist);
    return py_list;
}

static PyMethodDef pypango_attr_iterator_methods[] = {
    { "copy", (PyCFunction)pypango_attr_iterator_copy, METH_NOARGS },
    { "range", (PyCFunction)pypango_attr_iterator_range, METH_NOARGS },
    { "next", (PyCFunction)pypango_attr_iterator_next, METH_NOARGS },
    { "get", (PyCFunction)pypango_attr_iterator_get, METH_VARARGS|METH_KEYWORDS },
    { "get_font", (PyCFunction)pypango_attr_iterator_get_font, METH_NOARGS },
    { "get_attrs", (PyCFunction)pypango_attr_iterator_get_attrs, METH_NOARGS },
    { NULL, NULL, 0 }
};

static PyTypeObject PyPangoAttrIterator_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "pango.AttrIterator",			/* tp_name */
    sizeof(PyPangoAttrIterator),		/* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)pypango_attr_iterator_dealloc,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,			/* tp_getattr */
    (setattrfunc)0,			/* tp_setattr */
    (cmpfunc)pypango_attr_iterator_compare,	/* tp_compare */
    (reprfunc)0,			/* tp_repr */
    0,					/* tp_as_number */
    0,					/* tp_as_sequence */
    0,					/* tp_as_mapping */
    (hashfunc)pypango_attr_iterator_hash,	/* tp_hash */
    (ternaryfunc)0,			/* tp_call */
    (reprfunc)0,			/* tp_str */
    (getattrofunc)0,			/* tp_getattro */
    (setattrofunc)0,			/* tp_setattro */
    0,					/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,			/* tp_flags */
    NULL, /* Documentation string */
    (traverseproc)0,			/* tp_traverse */
    (inquiry)0,				/* tp_clear */
    (richcmpfunc)0,			/* tp_richcompare */
    0,					/* tp_weaklistoffset */
    (getiterfunc)0,			/* tp_iter */
    (iternextfunc)0,			/* tp_iternext */
    pypango_attr_iterator_methods,	/* tp_methods */
    0,					/* tp_members */
    0,					/* tp_getset */
    (PyTypeObject *)0,			/* tp_base */
    (PyObject *)0,			/* tp_dict */
    0,					/* tp_descr_get */
    0,					/* tp_descr_set */
    0,					/* tp_dictoffset */
    (initproc)0,			/* tp_init */
    (allocfunc)0,			/* tp_alloc */
    (newfunc)0,				/* tp_new */
    0,					/* tp_free */
    (inquiry)0,				/* tp_is_gc */
    (PyObject *)0,			/* tp_bases */
};

#line 506 "pango.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyPangoAttrList_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoColor_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontDescription_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontMetrics_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoGlyphString_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoItem_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoLanguage_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoLayoutIter_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoLayoutLine_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoMatrix_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoTabArray_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoContext_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoEngine_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoEngineLang_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoEngineShape_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFont_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontFace_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontFamily_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontMap_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontset_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoFontsetSimple_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoLayout_Type;
PyTypeObject G_GNUC_INTERNAL PyPangoRenderer_Type;

#line 539 "pango.c"



/* ----------- PangoAttrList ----------- */

static int
_wrap_pango_attr_list_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":Pango.AttrList.__init__", kwlist))
        return -1;
    self->gtype = PANGO_TYPE_ATTR_LIST;
    self->free_on_dealloc = FALSE;
    self->boxed = pango_attr_list_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoAttrList object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_pango_attr_list_copy(PyObject *self)
{
    PangoAttrList *ret;

    
    ret = pango_attr_list_copy(pyg_boxed_get(self, PangoAttrList));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_ATTR_LIST, ret, FALSE, TRUE);
}

#line 836 "pango.override"
static PyObject *
_wrap_pango_attr_list_insert(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attr", NULL };
    PyPangoAttribute *py_attr;
    PangoAttribute *attr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!:PangoAttrList.insert", kwlist,
				     &PyPangoAttribute_Type, &py_attr))
	return NULL;
    attr = pango_attribute_copy(py_attr->attr);

    pango_attr_list_insert(pyg_boxed_get(self, PangoAttrList), attr);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 595 "pango.c"


#line 856 "pango.override"
static PyObject *
_wrap_pango_attr_list_insert_before(PyObject *self, PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "attr", NULL };
    PyPangoAttribute *py_attr;
    PangoAttribute *attr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!:PangoAttrList.insert_before", kwlist,
				     &PyPangoAttribute_Type, &py_attr))
	return NULL;
    attr = pango_attribute_copy(py_attr->attr);

    pango_attr_list_insert_before(pyg_boxed_get(self, PangoAttrList), attr);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 618 "pango.c"


#line 877 "pango.override"
static PyObject *
_wrap_pango_attr_list_change(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attr", NULL };
    PyPangoAttribute *py_attr;
    PangoAttribute *attr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!:PangoAttrList.change", kwlist,
				     &PyPangoAttribute_Type, &py_attr))
	return NULL;
    attr = pango_attribute_copy(py_attr->attr);

    pango_attr_list_change(pyg_boxed_get(self, PangoAttrList), attr);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 640 "pango.c"


static PyObject *
_wrap_pango_attr_list_splice(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "other", "pos", "len", NULL };
    PyObject *py_other;
    int pos, len;
    PangoAttrList *other = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Pango.AttrList.splice", kwlist, &py_other, &pos, &len))
        return NULL;
    if (pyg_boxed_check(py_other, PANGO_TYPE_ATTR_LIST))
        other = pyg_boxed_get(py_other, PangoAttrList);
    else {
        PyErr_SetString(PyExc_TypeError, "other should be a PangoAttrList");
        return NULL;
    }
    
    pango_attr_list_splice(pyg_boxed_get(self, PangoAttrList), other, pos, len);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1558 "pango.override"
static gboolean
pypango_attr_list_filter_cb(PangoAttribute *attr, gpointer data)
{
    PyGILState_STATE state;
    PyGtkCustomNotify *cunote = data;
    PyObject *retobj, *py_attr;
    gboolean ret = FALSE;
    
    state = pyg_gil_state_ensure();

    py_attr = pypango_attr_new(pango_attribute_copy(attr),
			       attr->start_index, attr->end_index);

    if (cunote->data)
	retobj = PyObject_CallFunction(cunote->func, "NO", py_attr,
				       cunote->data);
    else
	retobj = PyObject_CallFunction(cunote->func, "N", py_attr);

    if (retobj != NULL) {
	ret = PyObject_IsTrue(retobj);
        Py_DECREF(retobj);
    } else {
        PyErr_Print();
    }

    pyg_gil_state_release(state);
    return ret;
}
static PyObject *
_wrap_pango_attr_list_filter(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "func", "data", NULL };
    PyObject *py_func, *py_data = NULL;
    PangoAttrList *attr_list, *filtered_list;
    PyGtkCustomNotify cunote;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O|O:pango.AttrList.filter",
				     kwlist, &py_func, &py_data))
	return NULL;

    if (!PyCallable_Check(py_func)) {
        PyErr_SetString(PyExc_TypeError, "func must be callable");
        return NULL;
    }

    cunote.func = py_func;
    cunote.data = py_data;
    Py_INCREF(cunote.func);
    Py_XINCREF(cunote.data);

    attr_list = (PangoAttrList *)pyg_boxed_get(self, PangoAttrList);
    filtered_list = pango_attr_list_filter(attr_list,
					   pypango_attr_list_filter_cb,
					   (gpointer)&cunote);

    Py_DECREF(cunote.func);
    Py_XDECREF(cunote.data);

    if (filtered_list)
	return pyg_boxed_new(PANGO_TYPE_ATTR_LIST, filtered_list, FALSE, TRUE);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 733 "pango.c"


#line 1448 "pango.override"
static PyObject *
_wrap_pango_attr_list_get_iterator(PyGBoxed *self)
{
    PangoAttrList *list = pyg_boxed_get(self, PangoAttrList);
    PangoAttrIterator *iter = pango_attr_list_get_iterator(list);

    return pypango_attr_iterator_new(iter);
}
#line 745 "pango.c"


static const PyMethodDef _PyPangoAttrList_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_attr_list_copy, METH_NOARGS,
      NULL },
    { "insert", (PyCFunction)_wrap_pango_attr_list_insert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "insert_before", (PyCFunction)_wrap_pango_attr_list_insert_before, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "change", (PyCFunction)_wrap_pango_attr_list_change, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "splice", (PyCFunction)_wrap_pango_attr_list_splice, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "filter", (PyCFunction)_wrap_pango_attr_list_filter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_iterator", (PyCFunction)_wrap_pango_attr_list_get_iterator, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoAttrList_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.AttrList",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoAttrList_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_attr_list_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoColor ----------- */

#line 1421 "pango.override"
static int
_wrap_pango_color_parse(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "spec", NULL };
    char *spec;
    PangoColor color;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:PangoColor.__init__",
				     kwlist, &spec))
        return -1;

    self->gtype = PANGO_TYPE_COLOR;
    self->free_on_dealloc = FALSE;

    if (pango_color_parse(&color, spec) != TRUE
	|| !(self->boxed = pango_color_copy(&color))) {
        PyErr_SetString(PyExc_RuntimeError,
			"could not create PangoColor object");
        return -1;
    }

    self->free_on_dealloc = TRUE;

    return 0;
}
#line 841 "pango.c"


static PyObject *
_wrap_pango_color_to_string(PyObject *self)
{
    gchar *ret;

    
    ret = pango_color_to_string(pyg_boxed_get(self, PangoColor));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyPangoColor_methods[] = {
    { "to_string", (PyCFunction)_wrap_pango_color_to_string, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_pango_color__get_red(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoColor)->red;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_color__get_green(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoColor)->green;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_color__get_blue(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoColor)->blue;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef pango_color_getsets[] = {
    { "red", (getter)_wrap_pango_color__get_red, (setter)0 },
    { "green", (getter)_wrap_pango_color__get_green, (setter)0 },
    { "blue", (getter)_wrap_pango_color__get_blue, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 2024 "pango.override"
static PyObject *
_wrap_pango_color_tp_str(PyObject *self)
{
    return _wrap_pango_color_to_string(self);
}
#line 907 "pango.c"


PyTypeObject G_GNUC_INTERNAL PyPangoColor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Color",                   /* tp_name */
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
    (reprfunc)_wrap_pango_color_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)_PyPangoColor_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)pango_color_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_color_parse,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoFontDescription ----------- */

#line 899 "pango.override"
static int
_wrap_pango_font_description_new(PyGBoxed *self, PyObject *args,
				 PyObject *kwargs)
{
    static char *kwlist[] = { "str", NULL };
    char *str = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|z:PangoFontDescription.__init__",
				     kwlist, &str))
	return -1;

    self->gtype = PANGO_TYPE_FONT_DESCRIPTION;
    self->free_on_dealloc = FALSE;
    if (str)
	self->boxed = pango_font_description_from_string(str);
    else
	self->boxed = pango_font_description_new();
    if (!self->boxed) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not create PangoFontDescription object");
	return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}
#line 986 "pango.c"


#line 948 "pango.override"
static PyObject *
_wrap_pango_font_description_copy(PyObject *self)
{
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION,
			 pyg_boxed_get(self, PangoFontDescription),
			 TRUE, TRUE);
}
#line 997 "pango.c"


static PyObject *
_wrap_pango_font_description_copy_static(PyObject *self)
{
    PangoFontDescription *ret;

    if (PyErr_Warn(PyExc_DeprecationWarning, "use copy pango.FontDescription.copy instead") < 0)
        return NULL;
    
    ret = pango_font_description_copy_static(pyg_boxed_get(self, PangoFontDescription));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_font_description_hash(PyObject *self)
{
    guint ret;

    
    ret = pango_font_description_hash(pyg_boxed_get(self, PangoFontDescription));
    
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_pango_font_description_set_family(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", NULL };
    char *family;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Pango.FontDescription.set_family", kwlist, &family))
        return NULL;
    
    pango_font_description_set_family(pyg_boxed_get(self, PangoFontDescription), family);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_set_family_static(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", NULL };
    char *family;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Pango.FontDescription.set_family_static", kwlist, &family))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use copy pango.FontDescription.set_family instead") < 0)
        return NULL;
    
    pango_font_description_set_family_static(pyg_boxed_get(self, PangoFontDescription), family);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_family(PyObject *self)
{
    const gchar *ret;

    
    ret = pango_font_description_get_family(pyg_boxed_get(self, PangoFontDescription));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_set_style(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "style", NULL };
    PangoStyle style;
    PyObject *py_style = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.set_style", kwlist, &py_style))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_STYLE, py_style, (gpointer)&style))
        return NULL;
    
    pango_font_description_set_style(pyg_boxed_get(self, PangoFontDescription), style);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_style(PyObject *self)
{
    gint ret;

    
    ret = pango_font_description_get_style(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_enum_from_gtype(PANGO_TYPE_STYLE, ret);
}

static PyObject *
_wrap_pango_font_description_set_variant(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "variant", NULL };
    PyObject *py_variant = NULL;
    PangoVariant variant;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.set_variant", kwlist, &py_variant))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_VARIANT, py_variant, (gpointer)&variant))
        return NULL;
    
    pango_font_description_set_variant(pyg_boxed_get(self, PangoFontDescription), variant);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_variant(PyObject *self)
{
    gint ret;

    
    ret = pango_font_description_get_variant(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_enum_from_gtype(PANGO_TYPE_VARIANT, ret);
}

static PyObject *
_wrap_pango_font_description_set_weight(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "weight", NULL };
    PangoWeight weight;
    PyObject *py_weight = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.set_weight", kwlist, &py_weight))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_WEIGHT, py_weight, (gpointer)&weight))
        return NULL;
    
    pango_font_description_set_weight(pyg_boxed_get(self, PangoFontDescription), weight);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_weight(PyObject *self)
{
    gint ret;

    
    ret = pango_font_description_get_weight(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_enum_from_gtype(PANGO_TYPE_WEIGHT, ret);
}

static PyObject *
_wrap_pango_font_description_set_stretch(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stretch", NULL };
    PyObject *py_stretch = NULL;
    PangoStretch stretch;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.set_stretch", kwlist, &py_stretch))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_STRETCH, py_stretch, (gpointer)&stretch))
        return NULL;
    
    pango_font_description_set_stretch(pyg_boxed_get(self, PangoFontDescription), stretch);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_stretch(PyObject *self)
{
    gint ret;

    
    ret = pango_font_description_get_stretch(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_enum_from_gtype(PANGO_TYPE_STRETCH, ret);
}

static PyObject *
_wrap_pango_font_description_set_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    int size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.FontDescription.set_size", kwlist, &size))
        return NULL;
    
    pango_font_description_set_size(pyg_boxed_get(self, PangoFontDescription), size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_size(PyObject *self)
{
    int ret;

    
    ret = pango_font_description_get_size(pyg_boxed_get(self, PangoFontDescription));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_description_set_absolute_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    double size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:Pango.FontDescription.set_absolute_size", kwlist, &size))
        return NULL;
    
    pango_font_description_set_absolute_size(pyg_boxed_get(self, PangoFontDescription), size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_size_is_absolute(PyObject *self)
{
    int ret;

    
    ret = pango_font_description_get_size_is_absolute(pyg_boxed_get(self, PangoFontDescription));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_font_description_set_gravity(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gravity", NULL };
    PyObject *py_gravity = NULL;
    PangoGravity gravity;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.set_gravity", kwlist, &py_gravity))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY, py_gravity, (gpointer)&gravity))
        return NULL;
    
    pango_font_description_set_gravity(pyg_boxed_get(self, PangoFontDescription), gravity);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_get_gravity(PyObject *self)
{
    gint ret;

    
    ret = pango_font_description_get_gravity(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY, ret);
}

static PyObject *
_wrap_pango_font_description_get_set_fields(PyObject *self)
{
    guint ret;

    
    ret = pango_font_description_get_set_fields(pyg_boxed_get(self, PangoFontDescription));
    
    return pyg_flags_from_gtype(PANGO_TYPE_FONT_MASK, ret);
}

static PyObject *
_wrap_pango_font_description_unset_fields(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "to_unset", NULL };
    PyObject *py_to_unset = NULL;
    PangoFontMask to_unset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontDescription.unset_fields", kwlist, &py_to_unset))
        return NULL;
    if (pyg_flags_get_value(PANGO_TYPE_FONT_MASK, py_to_unset, (gpointer)&to_unset))
        return NULL;
    
    pango_font_description_unset_fields(pyg_boxed_get(self, PangoFontDescription), to_unset);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_merge(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc_to_merge", "replace_existing", NULL };
    PyObject *py_desc_to_merge;
    int replace_existing;
    PangoFontDescription *desc_to_merge = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Pango.FontDescription.merge", kwlist, &py_desc_to_merge, &replace_existing))
        return NULL;
    if (pyg_boxed_check(py_desc_to_merge, PANGO_TYPE_FONT_DESCRIPTION))
        desc_to_merge = pyg_boxed_get(py_desc_to_merge, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc_to_merge should be a PangoFontDescription");
        return NULL;
    }
    
    pango_font_description_merge(pyg_boxed_get(self, PangoFontDescription), desc_to_merge, replace_existing);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_merge_static(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc_to_merge", "replace_existing", NULL };
    PyObject *py_desc_to_merge;
    int replace_existing;
    PangoFontDescription *desc_to_merge = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oi:Pango.FontDescription.merge_static", kwlist, &py_desc_to_merge, &replace_existing))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use copy pango.FontDescription.merge instead") < 0)
        return NULL;
    if (pyg_boxed_check(py_desc_to_merge, PANGO_TYPE_FONT_DESCRIPTION))
        desc_to_merge = pyg_boxed_get(py_desc_to_merge, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc_to_merge should be a PangoFontDescription");
        return NULL;
    }
    
    pango_font_description_merge_static(pyg_boxed_get(self, PangoFontDescription), desc_to_merge, replace_existing);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_better_match(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "old_match", "new_match", NULL };
    PyObject *py_old_match = Py_None, *py_new_match;
    int ret;
    PangoFontDescription *old_match = NULL, *new_match = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:Pango.FontDescription.better_match", kwlist, &py_old_match, &py_new_match))
        return NULL;
    if (pyg_boxed_check(py_old_match, PANGO_TYPE_FONT_DESCRIPTION))
        old_match = pyg_boxed_get(py_old_match, PangoFontDescription);
    else if (py_old_match != Py_None) {
        PyErr_SetString(PyExc_TypeError, "old_match should be a PangoFontDescription or None");
        return NULL;
    }
    if (pyg_boxed_check(py_new_match, PANGO_TYPE_FONT_DESCRIPTION))
        new_match = pyg_boxed_get(py_new_match, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "new_match should be a PangoFontDescription");
        return NULL;
    }
    
    ret = pango_font_description_better_match(pyg_boxed_get(self, PangoFontDescription), old_match, new_match);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_font_description_to_string(PyObject *self)
{
    gchar *ret;

    
    ret = pango_font_description_to_string(pyg_boxed_get(self, PangoFontDescription));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_description_to_filename(PyObject *self)
{
    gchar *ret;

    
    ret = pango_font_description_to_filename(pyg_boxed_get(self, PangoFontDescription));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyPangoFontDescription_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_font_description_copy, METH_NOARGS,
      NULL },
    { "copy_static", (PyCFunction)_wrap_pango_font_description_copy_static, METH_NOARGS,
      NULL },
    { "hash", (PyCFunction)_wrap_pango_font_description_hash, METH_NOARGS,
      NULL },
    { "set_family", (PyCFunction)_wrap_pango_font_description_set_family, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_family_static", (PyCFunction)_wrap_pango_font_description_set_family_static, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_family", (PyCFunction)_wrap_pango_font_description_get_family, METH_NOARGS,
      NULL },
    { "set_style", (PyCFunction)_wrap_pango_font_description_set_style, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_style", (PyCFunction)_wrap_pango_font_description_get_style, METH_NOARGS,
      NULL },
    { "set_variant", (PyCFunction)_wrap_pango_font_description_set_variant, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_variant", (PyCFunction)_wrap_pango_font_description_get_variant, METH_NOARGS,
      NULL },
    { "set_weight", (PyCFunction)_wrap_pango_font_description_set_weight, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_weight", (PyCFunction)_wrap_pango_font_description_get_weight, METH_NOARGS,
      NULL },
    { "set_stretch", (PyCFunction)_wrap_pango_font_description_set_stretch, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_stretch", (PyCFunction)_wrap_pango_font_description_get_stretch, METH_NOARGS,
      NULL },
    { "set_size", (PyCFunction)_wrap_pango_font_description_set_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_size", (PyCFunction)_wrap_pango_font_description_get_size, METH_NOARGS,
      NULL },
    { "set_absolute_size", (PyCFunction)_wrap_pango_font_description_set_absolute_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_size_is_absolute", (PyCFunction)_wrap_pango_font_description_get_size_is_absolute, METH_NOARGS,
      NULL },
    { "set_gravity", (PyCFunction)_wrap_pango_font_description_set_gravity, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_gravity", (PyCFunction)_wrap_pango_font_description_get_gravity, METH_NOARGS,
      NULL },
    { "get_set_fields", (PyCFunction)_wrap_pango_font_description_get_set_fields, METH_NOARGS,
      NULL },
    { "unset_fields", (PyCFunction)_wrap_pango_font_description_unset_fields, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "merge", (PyCFunction)_wrap_pango_font_description_merge, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "merge_static", (PyCFunction)_wrap_pango_font_description_merge_static, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "better_match", (PyCFunction)_wrap_pango_font_description_better_match, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_string", (PyCFunction)_wrap_pango_font_description_to_string, METH_NOARGS,
      NULL },
    { "to_filename", (PyCFunction)_wrap_pango_font_description_to_filename, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 2045 "pango.override"
static int
_wrap_pango_font_description_tp_compare(PyObject *self, PyObject *other)
{
    PangoFontDescription *font1, *font2;
    
    if (!pyg_boxed_check(other, PANGO_TYPE_FONT_DESCRIPTION))
	return -1;

    font1 = pyg_boxed_get(self, PangoFontDescription);
    font2 = pyg_boxed_get(other, PangoFontDescription);

    if (pango_font_description_equal(font1, font2))
	return 0;

    return -1;
}
#line 1484 "pango.c"


#line 2038 "pango.override"
static int
_wrap_pango_font_description_tp_hash(PyObject *self)
{
    return pango_font_description_hash(pyg_boxed_get(self, PangoFontDescription));
}
#line 1493 "pango.c"


#line 2031 "pango.override"
static PyObject *
_wrap_pango_font_description_tp_str(PyObject *self)
{
    return _wrap_pango_font_description_to_string(self);
}
#line 1502 "pango.c"


PyTypeObject G_GNUC_INTERNAL PyPangoFontDescription_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontDescription",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)_wrap_pango_font_description_tp_compare,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)_wrap_pango_font_description_tp_hash,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_pango_font_description_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)_PyPangoFontDescription_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_font_description_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoFontMetrics ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

static PyObject *
_wrap_pango_font_metrics_get_ascent(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_ascent(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_descent(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_descent(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_approximate_char_width(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_approximate_char_width(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_approximate_digit_width(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_approximate_digit_width(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_underline_position(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_underline_position(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_underline_thickness(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_underline_thickness(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_strikethrough_position(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_strikethrough_position(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_font_metrics_get_strikethrough_thickness(PyObject *self)
{
    int ret;

    
    ret = pango_font_metrics_get_strikethrough_thickness(pyg_boxed_get(self, PangoFontMetrics));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyPangoFontMetrics_methods[] = {
    { "get_ascent", (PyCFunction)_wrap_pango_font_metrics_get_ascent, METH_NOARGS,
      NULL },
    { "get_descent", (PyCFunction)_wrap_pango_font_metrics_get_descent, METH_NOARGS,
      NULL },
    { "get_approximate_char_width", (PyCFunction)_wrap_pango_font_metrics_get_approximate_char_width, METH_NOARGS,
      NULL },
    { "get_approximate_digit_width", (PyCFunction)_wrap_pango_font_metrics_get_approximate_digit_width, METH_NOARGS,
      NULL },
    { "get_underline_position", (PyCFunction)_wrap_pango_font_metrics_get_underline_position, METH_NOARGS,
      NULL },
    { "get_underline_thickness", (PyCFunction)_wrap_pango_font_metrics_get_underline_thickness, METH_NOARGS,
      NULL },
    { "get_strikethrough_position", (PyCFunction)_wrap_pango_font_metrics_get_strikethrough_position, METH_NOARGS,
      NULL },
    { "get_strikethrough_thickness", (PyCFunction)_wrap_pango_font_metrics_get_strikethrough_thickness, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontMetrics_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontMetrics",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontMetrics_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoGlyphString ----------- */

static int
_wrap_pango_glyph_string_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":Pango.GlyphString.__init__", kwlist))
        return -1;
    self->gtype = PANGO_TYPE_GLYPH_STRING;
    self->free_on_dealloc = FALSE;
    self->boxed = pango_glyph_string_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoGlyphString object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_pango_glyph_string_set_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "new_len", NULL };
    int new_len;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.GlyphString.set_size", kwlist, &new_len))
        return NULL;
    
    pango_glyph_string_set_size(pyg_boxed_get(self, PangoGlyphString), new_len);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_glyph_string_copy(PyObject *self)
{
    PangoGlyphString *ret;

    
    ret = pango_glyph_string_copy(pyg_boxed_get(self, PangoGlyphString));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_GLYPH_STRING, ret, FALSE, TRUE);
}

#line 1042 "pango.override"
static PyObject *
_wrap_pango_glyph_string_extents(PyObject *self, PyObject *args,
				 PyObject *kwargs)
{
    static char *kwlist[] = { "font", NULL };
    PyObject *font;
    PangoRectangle ink_rect, logical_rect;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O:PangoGlyphString.extents", kwlist,
				     &font))
	return NULL;
    if (!pygobject_check(font, &PyPangoFont_Type)) {
	PyErr_SetString(PyExc_TypeError, "font must be a PangoFont");
	return NULL;
    }

    pango_glyph_string_extents(pyg_boxed_get(self, PangoGlyphString),
			       PANGO_FONT(pygobject_get(font)),
			       &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 1795 "pango.c"


#line 1071 "pango.override"
static PyObject *
_wrap_pango_glyph_string_extents_range(PyObject *self, PyObject *args,
				       PyObject *kwargs)
{
    static char *kwlist[] = { "start", "end", "font", NULL };
    gint start, end;
    PyObject *font;
    PangoRectangle ink_rect, logical_rect;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iiO:PangoGlyphString.extents_range",
				     kwlist, &start, &end, &font))
	return NULL;
    if (!pygobject_check(font, &PyPangoFont_Type)) {
	PyErr_SetString(PyExc_TypeError, "font must be a PangoFont");
	return NULL;
    }

    pango_glyph_string_extents_range(pyg_boxed_get(self, PangoGlyphString),
				     start, end,
				     PANGO_FONT(pygobject_get(font)),
				     &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 1828 "pango.c"


#line 1102 "pango.override"
static PyObject *
_wrap_pango_glyph_string_get_logical_widths(PyObject *self, PyObject *args,
					    PyObject *kwargs)
{
    static char *kwlist[] = { "text", "embedding_level", NULL };
    const char *text;
    gint length, embedding_level, *logical_widths;
    Py_ssize_t i, slen;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s#i:PangoGlyphString.get_logical_widths",
				     kwlist, &text, &length, &embedding_level))
	return NULL;
    slen = g_utf8_strlen(text, length);
    logical_widths = g_new(int, slen);
    pango_glyph_string_get_logical_widths(pyg_boxed_get(self,PangoGlyphString),
					  text, length, embedding_level,
					  logical_widths);
    ret = PyTuple_New(slen);
    for (i = 0; i < slen; i++) {
	PyObject *item = PyInt_FromLong(logical_widths[i]);

	PyTuple_SetItem(ret, i, item);
    }
    g_free(logical_widths);
    return ret;
}
#line 1860 "pango.c"


static const PyMethodDef _PyPangoGlyphString_methods[] = {
    { "set_size", (PyCFunction)_wrap_pango_glyph_string_set_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy", (PyCFunction)_wrap_pango_glyph_string_copy, METH_NOARGS,
      NULL },
    { "extents", (PyCFunction)_wrap_pango_glyph_string_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "extents_range", (PyCFunction)_wrap_pango_glyph_string_extents_range, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_logical_widths", (PyCFunction)_wrap_pango_glyph_string_get_logical_widths, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_pango_glyph_string__get_num_glyphs(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoGlyphString)->num_glyphs;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef pango_glyph_string_getsets[] = {
    { "num_glyphs", (getter)_wrap_pango_glyph_string__get_num_glyphs, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyPangoGlyphString_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.GlyphString",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoGlyphString_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)pango_glyph_string_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_glyph_string_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoItem ----------- */

static int
_wrap_pango_item_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":Pango.Item.__init__", kwlist))
        return -1;
    self->gtype = PANGO_TYPE_ITEM;
    self->free_on_dealloc = FALSE;
    self->boxed = pango_item_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoItem object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_pango_item_copy(PyObject *self)
{
    PangoItem *ret;

    
    ret = pango_item_copy(pyg_boxed_get(self, PangoItem));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_ITEM, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_item_split(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "split_index", "split_offset", NULL };
    int split_index, split_offset;
    PangoItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Pango.Item.split", kwlist, &split_index, &split_offset))
        return NULL;
    
    ret = pango_item_split(pyg_boxed_get(self, PangoItem), split_index, split_offset);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_ITEM, ret, FALSE, TRUE);
}

static const PyMethodDef _PyPangoItem_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_item_copy, METH_NOARGS,
      NULL },
    { "split", (PyCFunction)_wrap_pango_item_split, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_pango_item__get_offset(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoItem)->offset;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_item__get_length(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoItem)->length;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_item__get_num_chars(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoItem)->num_chars;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_item__get_analysis_shape_engine(PyObject *self, void *closure)
{
    PangoEngineShape *ret;

    ret = pyg_boxed_get(self, PangoItem)->analysis.shape_engine;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_item__get_analysis_lang_engine(PyObject *self, void *closure)
{
    PangoEngineLang *ret;

    ret = pyg_boxed_get(self, PangoItem)->analysis.lang_engine;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_item__get_analysis_font(PyObject *self, void *closure)
{
    PangoFont *ret;

    ret = pyg_boxed_get(self, PangoItem)->analysis.font;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_item__get_analysis_level(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoItem)->analysis.level;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_item__get_analysis_language(PyObject *self, void *closure)
{
    PangoLanguage *ret;

    ret = pyg_boxed_get(self, PangoItem)->analysis.language;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LANGUAGE, ret, TRUE, TRUE);
}

static const PyGetSetDef pango_item_getsets[] = {
    { "offset", (getter)_wrap_pango_item__get_offset, (setter)0 },
    { "length", (getter)_wrap_pango_item__get_length, (setter)0 },
    { "num_chars", (getter)_wrap_pango_item__get_num_chars, (setter)0 },
    { "analysis_shape_engine", (getter)_wrap_pango_item__get_analysis_shape_engine, (setter)0 },
    { "analysis_lang_engine", (getter)_wrap_pango_item__get_analysis_lang_engine, (setter)0 },
    { "analysis_font", (getter)_wrap_pango_item__get_analysis_font, (setter)0 },
    { "analysis_level", (getter)_wrap_pango_item__get_analysis_level, (setter)0 },
    { "analysis_language", (getter)_wrap_pango_item__get_analysis_language, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyPangoItem_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Item",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoItem_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)pango_item_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_item_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoLanguage ----------- */

static int
_wrap_pango_language_from_string(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "language", NULL };
    char *language;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Pango.Language.__init__", kwlist, &language))
        return -1;
    self->gtype = PANGO_TYPE_LANGUAGE;
    self->free_on_dealloc = FALSE;
    self->boxed = pango_language_from_string(language);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoLanguage object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_pango_language_includes_script(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "script", NULL };
    PyObject *py_script = NULL;
    int ret;
    PangoScript script;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Language.includes_script", kwlist, &py_script))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_SCRIPT, py_script, (gpointer)&script))
        return NULL;
    
    ret = pango_language_includes_script(pyg_boxed_get(self, PangoLanguage), script);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_language_matches(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "range_list", NULL };
    char *range_list;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:Pango.Language.matches", kwlist, &range_list))
        return NULL;
    
    ret = pango_language_matches(pyg_boxed_get(self, PangoLanguage), range_list);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_language_to_string(PyObject *self)
{
    const gchar *ret;

    
    ret = pango_language_to_string(pyg_boxed_get(self, PangoLanguage));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyPangoLanguage_methods[] = {
    { "includes_script", (PyCFunction)_wrap_pango_language_includes_script, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "matches", (PyCFunction)_wrap_pango_language_matches, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_string", (PyCFunction)_wrap_pango_language_to_string, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 2063 "pango.override"
static PyObject *
_wrap_pango_language_tp_str(PyObject *self)
{
    return _wrap_pango_language_to_string(self);
}
#line 2217 "pango.c"


PyTypeObject G_GNUC_INTERNAL PyPangoLanguage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Language",                   /* tp_name */
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
    (reprfunc)_wrap_pango_language_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)_PyPangoLanguage_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_language_from_string,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoLayoutIter ----------- */

static PyObject *
_wrap_pango_layout_iter_get_index(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_get_index(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_iter_get_line(PyObject *self)
{
    PangoLayoutLine *ret;

    
    ret = pango_layout_iter_get_line(pyg_boxed_get(self, PangoLayoutIter));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LAYOUT_LINE, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_layout_iter_get_line_readonly(PyObject *self)
{
    PangoLayoutLine *ret;

    
    ret = pango_layout_iter_get_line_readonly(pyg_boxed_get(self, PangoLayoutIter));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LAYOUT_LINE, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_layout_iter_at_last_line(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_at_last_line(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_iter_next_char(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_next_char(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_iter_next_cluster(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_next_cluster(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_iter_next_run(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_next_run(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_iter_next_line(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_next_line(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyBool_FromLong(ret);

}

#line 1768 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_char_extents(PyGObject *self)
{
    PangoRectangle logical_rect;

    pango_layout_iter_get_char_extents(pyg_boxed_get(self, PangoLayoutIter),
                                       &logical_rect);
    return Py_BuildValue("(iiii)",
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2376 "pango.c"


#line 1781 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_cluster_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_iter_get_cluster_extents(pyg_boxed_get(self, PangoLayoutIter),
                                          &ink_rect, &logical_rect);
    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2393 "pango.c"


#line 1811 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_run_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_iter_get_run_extents(pyg_boxed_get(self, PangoLayoutIter),
                                      &ink_rect, &logical_rect);
    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2410 "pango.c"


#line 1796 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_line_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_iter_get_line_extents(pyg_boxed_get(self, PangoLayoutIter),
                                       &ink_rect, &logical_rect);
    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2427 "pango.c"


#line 1841 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_line_yrange(PyGObject *self)
{
    int start, end;

    pango_layout_iter_get_line_yrange(pyg_boxed_get(self, PangoLayoutIter),
                                      &start, &end);
    return Py_BuildValue("(ii)", start, end);
}
#line 2440 "pango.c"


#line 1826 "pango.override"
static PyObject *
_wrap_pango_layout_iter_get_layout_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_iter_get_layout_extents(pyg_boxed_get(self, PangoLayoutIter),
                                         &ink_rect, &logical_rect);
    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2457 "pango.c"


static PyObject *
_wrap_pango_layout_iter_get_baseline(PyObject *self)
{
    int ret;

    
    ret = pango_layout_iter_get_baseline(pyg_boxed_get(self, PangoLayoutIter));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyPangoLayoutIter_methods[] = {
    { "get_index", (PyCFunction)_wrap_pango_layout_iter_get_index, METH_NOARGS,
      NULL },
    { "get_line", (PyCFunction)_wrap_pango_layout_iter_get_line, METH_NOARGS,
      NULL },
    { "get_line_readonly", (PyCFunction)_wrap_pango_layout_iter_get_line_readonly, METH_NOARGS,
      NULL },
    { "at_last_line", (PyCFunction)_wrap_pango_layout_iter_at_last_line, METH_NOARGS,
      NULL },
    { "next_char", (PyCFunction)_wrap_pango_layout_iter_next_char, METH_NOARGS,
      NULL },
    { "next_cluster", (PyCFunction)_wrap_pango_layout_iter_next_cluster, METH_NOARGS,
      NULL },
    { "next_run", (PyCFunction)_wrap_pango_layout_iter_next_run, METH_NOARGS,
      NULL },
    { "next_line", (PyCFunction)_wrap_pango_layout_iter_next_line, METH_NOARGS,
      NULL },
    { "get_char_extents", (PyCFunction)_wrap_pango_layout_iter_get_char_extents, METH_NOARGS,
      NULL },
    { "get_cluster_extents", (PyCFunction)_wrap_pango_layout_iter_get_cluster_extents, METH_NOARGS,
      NULL },
    { "get_run_extents", (PyCFunction)_wrap_pango_layout_iter_get_run_extents, METH_NOARGS,
      NULL },
    { "get_line_extents", (PyCFunction)_wrap_pango_layout_iter_get_line_extents, METH_NOARGS,
      NULL },
    { "get_line_yrange", (PyCFunction)_wrap_pango_layout_iter_get_line_yrange, METH_NOARGS,
      NULL },
    { "get_layout_extents", (PyCFunction)_wrap_pango_layout_iter_get_layout_extents, METH_NOARGS,
      NULL },
    { "get_baseline", (PyCFunction)_wrap_pango_layout_iter_get_baseline, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoLayoutIter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.LayoutIter",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoLayoutIter_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoLayoutLine ----------- */

#line 1852 "pango.override"
static PyObject *
_wrap_pango_layout_line_x_to_index(PyObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "x_pos", NULL };
    gboolean inside;
    int x_pos, index, trailing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:PangoLayoutLine.x_to_index",
				     kwlist, &x_pos))
	return NULL;

    inside = pango_layout_line_x_to_index(pyg_boxed_get(self, PangoLayoutLine),
                                          x_pos, &index, &trailing);
    return Py_BuildValue("Nii", PyBool_FromLong(inside), index, trailing);
}
#line 2572 "pango.c"


#line 1871 "pango.override"
static PyObject *
_wrap_pango_layout_line_index_to_x(PyObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "index", "trailing", NULL };
    int x_pos, index;
    PyObject *trailing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:PangoLayoutLine.index_to_x",
				     kwlist, &index, &trailing))
	return NULL;
    pango_layout_line_index_to_x(pyg_boxed_get(self, PangoLayoutLine),
                                 index, PyObject_IsTrue(trailing),
                                 &x_pos);
    return PyInt_FromLong(x_pos);
}
#line 2593 "pango.c"


#line 1890 "pango.override"
static PyObject *
_wrap_pango_layout_line_get_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_line_get_extents(pyg_boxed_get(self, PangoLayoutLine),
                                  &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 2611 "pango.c"


#line 1906 "pango.override"
static PyObject *
_wrap_pango_layout_line_get_pixel_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_line_get_pixel_extents(pyg_boxed_get(self, PangoLayoutLine),
                                        &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}

#line 2630 "pango.c"


static const PyMethodDef _PyPangoLayoutLine_methods[] = {
    { "x_to_index", (PyCFunction)_wrap_pango_layout_line_x_to_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "index_to_x", (PyCFunction)_wrap_pango_layout_line_index_to_x, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_pango_layout_line_get_extents, METH_NOARGS,
      NULL },
    { "get_pixel_extents", (PyCFunction)_wrap_pango_layout_line_get_pixel_extents, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_pango_layout_line__get_start_index(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoLayoutLine)->start_index;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_line__get_length(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, PangoLayoutLine)->length;
    return PyInt_FromLong(ret);
}

#line 1923 "pango.override"

static inline PyObject *
pypango_glyph_item_new(PangoGlyphItem *gitem)
{
    return Py_BuildValue("NN", pyg_boxed_new(PANGO_TYPE_ITEM, gitem->item, TRUE, TRUE),
                         pyg_boxed_new(PANGO_TYPE_GLYPH_STRING, gitem->glyphs, TRUE, TRUE));
}

static PyObject *
_wrap_pango_layout_line__get_runs(PyGObject *self, void *closure)
{
    PangoLayoutLine *line = pyg_boxed_get(self, PangoLayoutLine);
    PyObject *list, *item;
    GSList *l;

    list = PyList_New(0);
    for (l = line->runs; l; l = l->next) {
        item = pypango_glyph_item_new((PangoGlyphItem *) l->data);
        PyList_Append(list, item);
        Py_DECREF(item);
    }
    return list;
}
#line 2687 "pango.c"


static PyObject *
_wrap_pango_layout_line__get_is_paragraph_start(PyObject *self, void *closure)
{
    guint ret;

    ret = pyg_boxed_get(self, PangoLayoutLine)->is_paragraph_start;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_pango_layout_line__get_resolved_dir(PyObject *self, void *closure)
{
    guint ret;

    ret = pyg_boxed_get(self, PangoLayoutLine)->resolved_dir;
    return PyLong_FromUnsignedLong(ret);
}

static const PyGetSetDef pango_layout_line_getsets[] = {
    { "start_index", (getter)_wrap_pango_layout_line__get_start_index, (setter)0 },
    { "length", (getter)_wrap_pango_layout_line__get_length, (setter)0 },
    { "runs", (getter)_wrap_pango_layout_line__get_runs, (setter)0 },
    { "is_paragraph_start", (getter)_wrap_pango_layout_line__get_is_paragraph_start, (setter)0 },
    { "resolved_dir", (getter)_wrap_pango_layout_line__get_resolved_dir, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyPangoLayoutLine_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.LayoutLine",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoLayoutLine_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)pango_layout_line_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoMatrix ----------- */

static PyObject *
_wrap_pango_matrix_copy(PyObject *self)
{
    PangoMatrix *ret;

    
    ret = pango_matrix_copy(pyg_boxed_get(self, PangoMatrix));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_MATRIX, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_matrix_translate(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tx", "ty", NULL };
    double tx, ty;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:Pango.Matrix.translate", kwlist, &tx, &ty))
        return NULL;
    
    pango_matrix_translate(pyg_boxed_get(self, PangoMatrix), tx, ty);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_matrix_scale(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "scale_x", "scale_y", NULL };
    double scale_x, scale_y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:Pango.Matrix.scale", kwlist, &scale_x, &scale_y))
        return NULL;
    
    pango_matrix_scale(pyg_boxed_get(self, PangoMatrix), scale_x, scale_y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_matrix_rotate(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", NULL };
    double degrees;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:Pango.Matrix.rotate", kwlist, &degrees))
        return NULL;
    
    pango_matrix_rotate(pyg_boxed_get(self, PangoMatrix), degrees);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_matrix_concat(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "new_matrix", NULL };
    PyObject *py_new_matrix;
    PangoMatrix *new_matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Matrix.concat", kwlist, &py_new_matrix))
        return NULL;
    if (pyg_boxed_check(py_new_matrix, PANGO_TYPE_MATRIX))
        new_matrix = pyg_boxed_get(py_new_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "new_matrix should be a PangoMatrix");
        return NULL;
    }
    
    pango_matrix_concat(pyg_boxed_get(self, PangoMatrix), new_matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyPangoMatrix_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_matrix_copy, METH_NOARGS,
      NULL },
    { "translate", (PyCFunction)_wrap_pango_matrix_translate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scale", (PyCFunction)_wrap_pango_matrix_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rotate", (PyCFunction)_wrap_pango_matrix_rotate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "concat", (PyCFunction)_wrap_pango_matrix_concat, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoMatrix_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Matrix",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoMatrix_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoTabArray ----------- */

static int
_wrap_pango_tab_array_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "initial_size", "positions_in_pixels", NULL };
    int initial_size, positions_in_pixels;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:Pango.TabArray.__init__", kwlist, &initial_size, &positions_in_pixels))
        return -1;
    self->gtype = PANGO_TYPE_TAB_ARRAY;
    self->free_on_dealloc = FALSE;
    self->boxed = pango_tab_array_new(initial_size, positions_in_pixels);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoTabArray object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_pango_tab_array_copy(PyObject *self)
{
    PangoTabArray *ret;

    
    ret = pango_tab_array_copy(pyg_boxed_get(self, PangoTabArray));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_TAB_ARRAY, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_tab_array_get_size(PyObject *self)
{
    int ret;

    
    ret = pango_tab_array_get_size(pyg_boxed_get(self, PangoTabArray));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_tab_array_resize(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "new_size", NULL };
    int new_size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.TabArray.resize", kwlist, &new_size))
        return NULL;
    
    pango_tab_array_resize(pyg_boxed_get(self, PangoTabArray), new_size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_tab_array_set_tab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tab_index", "alignment", "location", NULL };
    int tab_index, location;
    PyObject *py_alignment = NULL;
    PangoTabAlign alignment;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iOi:Pango.TabArray.set_tab", kwlist, &tab_index, &py_alignment, &location))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_TAB_ALIGN, py_alignment, (gpointer)&alignment))
        return NULL;
    
    pango_tab_array_set_tab(pyg_boxed_get(self, PangoTabArray), tab_index, alignment, location);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1365 "pango.override"
static PyObject *
_wrap_pango_tab_array_get_tab(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tab_index", NULL };
    gint tab_index, location;
    PangoTabAlign alignment;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:PangoTabArray.get_tab",
				     kwlist, &tab_index))
	return NULL;

    pango_tab_array_get_tab(pyg_boxed_get(self, PangoTabArray),
			    tab_index, &alignment, &location);
    return Py_BuildValue("(ii)", (int)alignment, location);
}
#line 3001 "pango.c"


#line 1382 "pango.override"
static PyObject *
_wrap_pango_tab_array_get_tabs(PyObject *self)
{
    PangoTabAlign *alignments;
    gint *locations, length, i;
    PyObject *ret;

    length = pango_tab_array_get_size(pyg_boxed_get(self, PangoTabArray));
    pango_tab_array_get_tabs(pyg_boxed_get(self, PangoTabArray),
			     &alignments, &locations);
    ret = PyTuple_New(length);
    for (i = 0; i < length; i++) {
	PyObject *item;

	item = Py_BuildValue("(ii)", (int)alignments[i], locations[i]);
	PyTuple_SetItem(ret, i, item);
    }
    g_free(alignments);
    g_free(locations);
    return ret;
}
#line 3026 "pango.c"


static PyObject *
_wrap_pango_tab_array_get_positions_in_pixels(PyObject *self)
{
    int ret;

    
    ret = pango_tab_array_get_positions_in_pixels(pyg_boxed_get(self, PangoTabArray));
    
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyPangoTabArray_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_tab_array_copy, METH_NOARGS,
      NULL },
    { "get_size", (PyCFunction)_wrap_pango_tab_array_get_size, METH_NOARGS,
      NULL },
    { "resize", (PyCFunction)_wrap_pango_tab_array_resize, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_tab", (PyCFunction)_wrap_pango_tab_array_set_tab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_tab", (PyCFunction)_wrap_pango_tab_array_get_tab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_tabs", (PyCFunction)_wrap_pango_tab_array_get_tabs, METH_NOARGS,
      NULL },
    { "get_positions_in_pixels", (PyCFunction)_wrap_pango_tab_array_get_positions_in_pixels, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoTabArray_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.TabArray",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoTabArray_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_pango_tab_array_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoContext ----------- */

static int
_wrap_pango_context_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":pango.Context.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create pango.Context object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_pango_context_set_font_map(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font_map", NULL };
    PyGObject *font_map;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Context.add_font_map", kwlist, &PyPangoFontMap_Type, &font_map))
        return NULL;
    
    pango_context_set_font_map(PANGO_CONTEXT(self->obj), PANGO_FONT_MAP(font_map->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_font_map(PyGObject *self)
{
    PangoFontMap *ret;

    
    ret = pango_context_get_font_map(PANGO_CONTEXT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 957 "pango.override"
static PyObject *
_wrap_pango_context_list_families(PyGObject *self)
{
    PangoFontFamily **families;
    gint n_families, i;
    PyObject *ret;

    pango_context_list_families(PANGO_CONTEXT(self->obj), &families,
				&n_families);
    ret = PyTuple_New(n_families);
    for (i = 0; i < n_families; i++) {
	PyObject *family;

	family = pygobject_new((GObject *)families[i]);
	PyTuple_SetItem(ret, i, family);
    }
    g_free(families);
    return ret;
}
#line 3175 "pango.c"


static PyObject *
_wrap_pango_context_load_font(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc", NULL };
    PyObject *py_desc, *py_ret;
    PangoFont *ret;
    PangoFontDescription *desc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.load_font", kwlist, &py_desc))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    
    ret = pango_context_load_font(PANGO_CONTEXT(self->obj), desc);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_pango_context_load_fontset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc", "language", NULL };
    PyObject *py_desc, *py_language, *py_ret;
    PangoFontset *ret;
    PangoFontDescription *desc = NULL;
    PangoLanguage *language = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Pango.Context.load_fontset", kwlist, &py_desc, &py_language))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return NULL;
    }
    
    ret = pango_context_load_fontset(PANGO_CONTEXT(self->obj), desc, language);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_pango_context_get_metrics(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc", "language", NULL };
    PyObject *py_desc, *py_language = Py_None;
    PangoFontDescription *desc = NULL;
    PangoLanguage *language = NULL;
    PangoFontMetrics *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O|O:Pango.Context.get_metrics", kwlist, &py_desc, &py_language))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else if (py_language != Py_None) {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage or None");
        return NULL;
    }
    
    ret = pango_context_get_metrics(PANGO_CONTEXT(self->obj), desc, language);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_METRICS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_context_set_font_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc", NULL };
    PyObject *py_desc;
    PangoFontDescription *desc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_font_description", kwlist, &py_desc))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    
    pango_context_set_font_description(PANGO_CONTEXT(self->obj), desc);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_font_description(PyGObject *self)
{
    PangoFontDescription *ret;

    
    ret = pango_context_get_font_description(PANGO_CONTEXT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_context_get_language(PyGObject *self)
{
    PangoLanguage *ret;

    
    ret = pango_context_get_language(PANGO_CONTEXT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LANGUAGE, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_context_set_language(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "language", NULL };
    PyObject *py_language;
    PangoLanguage *language = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_language", kwlist, &py_language))
        return NULL;
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return NULL;
    }
    
    pango_context_set_language(PANGO_CONTEXT(self->obj), language);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_set_base_dir(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "direction", NULL };
    PangoDirection direction;
    PyObject *py_direction = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_base_dir", kwlist, &py_direction))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_DIRECTION, py_direction, (gpointer)&direction))
        return NULL;
    
    pango_context_set_base_dir(PANGO_CONTEXT(self->obj), direction);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_base_dir(PyGObject *self)
{
    gint ret;

    
    ret = pango_context_get_base_dir(PANGO_CONTEXT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_DIRECTION, ret);
}

static PyObject *
_wrap_pango_context_set_matrix(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    PangoMatrix *matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_matrix", kwlist, &py_matrix))
        return NULL;
    if (pyg_boxed_check(py_matrix, PANGO_TYPE_MATRIX))
        matrix = pyg_boxed_get(py_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "matrix should be a PangoMatrix");
        return NULL;
    }
    
    pango_context_set_matrix(PANGO_CONTEXT(self->obj), matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_matrix(PyGObject *self)
{
    const PangoMatrix *ret;

    
    ret = pango_context_get_matrix(PANGO_CONTEXT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_MATRIX, (PangoMatrix*) ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_context_set_base_gravity(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gravity", NULL };
    PyObject *py_gravity = NULL;
    PangoGravity gravity;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_base_gravity", kwlist, &py_gravity))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY, py_gravity, (gpointer)&gravity))
        return NULL;
    
    pango_context_set_base_gravity(PANGO_CONTEXT(self->obj), gravity);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_base_gravity(PyGObject *self)
{
    gint ret;

    
    ret = pango_context_get_base_gravity(PANGO_CONTEXT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY, ret);
}

static PyObject *
_wrap_pango_context_get_gravity(PyGObject *self)
{
    gint ret;

    
    ret = pango_context_get_gravity(PANGO_CONTEXT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY, ret);
}

static PyObject *
_wrap_pango_context_set_gravity_hint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "hint", NULL };
    PangoGravityHint hint;
    PyObject *py_hint = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Context.set_gravity_hint", kwlist, &py_hint))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY_HINT, py_hint, (gpointer)&hint))
        return NULL;
    
    pango_context_set_gravity_hint(PANGO_CONTEXT(self->obj), hint);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_context_get_gravity_hint(PyGObject *self)
{
    gint ret;

    
    ret = pango_context_get_gravity_hint(PANGO_CONTEXT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY_HINT, ret);
}

static const PyMethodDef _PyPangoContext_methods[] = {
    { "add_font_map", (PyCFunction)_wrap_pango_context_set_font_map, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_font_map", (PyCFunction)_wrap_pango_context_get_font_map, METH_NOARGS,
      NULL },
    { "list_families", (PyCFunction)_wrap_pango_context_list_families, METH_NOARGS,
      NULL },
    { "load_font", (PyCFunction)_wrap_pango_context_load_font, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "load_fontset", (PyCFunction)_wrap_pango_context_load_fontset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_metrics", (PyCFunction)_wrap_pango_context_get_metrics, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_font_description", (PyCFunction)_wrap_pango_context_set_font_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_font_description", (PyCFunction)_wrap_pango_context_get_font_description, METH_NOARGS,
      NULL },
    { "get_language", (PyCFunction)_wrap_pango_context_get_language, METH_NOARGS,
      NULL },
    { "set_language", (PyCFunction)_wrap_pango_context_set_language, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_base_dir", (PyCFunction)_wrap_pango_context_set_base_dir, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_base_dir", (PyCFunction)_wrap_pango_context_get_base_dir, METH_NOARGS,
      NULL },
    { "set_matrix", (PyCFunction)_wrap_pango_context_set_matrix, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_matrix", (PyCFunction)_wrap_pango_context_get_matrix, METH_NOARGS,
      NULL },
    { "set_base_gravity", (PyCFunction)_wrap_pango_context_set_base_gravity, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_base_gravity", (PyCFunction)_wrap_pango_context_get_base_gravity, METH_NOARGS,
      NULL },
    { "get_gravity", (PyCFunction)_wrap_pango_context_get_gravity, METH_NOARGS,
      NULL },
    { "set_gravity_hint", (PyCFunction)_wrap_pango_context_set_gravity_hint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_gravity_hint", (PyCFunction)_wrap_pango_context_get_gravity_hint, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Context",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoContext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_pango_context_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoEngine ----------- */

PyTypeObject G_GNUC_INTERNAL PyPangoEngine_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Engine",                   /* tp_name */
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



/* ----------- PangoEngineLang ----------- */

PyTypeObject G_GNUC_INTERNAL PyPangoEngineLang_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.EngineLang",                   /* tp_name */
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



/* ----------- PangoEngineShape ----------- */

PyTypeObject G_GNUC_INTERNAL PyPangoEngineShape_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.EngineShape",                   /* tp_name */
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



/* ----------- PangoFont ----------- */

static PyObject *
_wrap_pango_font_describe(PyGObject *self)
{
    PangoFontDescription *ret;

    
    ret = pango_font_describe(PANGO_FONT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_font_get_metrics(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "language", NULL };
    PyObject *py_language = Py_None;
    PangoLanguage *language = NULL;
    PangoFontMetrics *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:Pango.Font.get_metrics", kwlist, &py_language))
        return NULL;
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else if (py_language != Py_None) {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage or None");
        return NULL;
    }
    
    ret = pango_font_get_metrics(PANGO_FONT(self->obj), language);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_METRICS, ret, FALSE, TRUE);
}

#line 978 "pango.override"
static PyObject *
_wrap_pango_font_get_glyph_extents(PyGObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "glyph", NULL };
    gint glyph;
    PangoRectangle ink_rect, logical_rect;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:PangoFont.get_glyph_extents", kwlist,
				     &glyph))
	return NULL;
    pango_font_get_glyph_extents(PANGO_FONT(self->obj), (PangoGlyph)glyph,
				 &ink_rect, &logical_rect);
    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 3759 "pango.c"


static PyObject *
_wrap_pango_font_get_font_map(PyGObject *self)
{
    PangoFontMap *ret;

    
    ret = pango_font_get_font_map(PANGO_FONT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_PangoFont__do_describe(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    PangoFontDescription *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Font.describe", kwlist, &PyPangoFont_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_CLASS(klass)->describe)
        ret = PANGO_FONT_CLASS(klass)->describe(PANGO_FONT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Font.describe not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, TRUE, TRUE);
}

static PyObject *
_wrap_PangoFont__do_find_shaper(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "lang", "ch", NULL };
    PyGObject *self;
    PyObject *py_lang;
    PangoLanguage *lang = NULL;
    PangoEngineShape *ret;
    unsigned long ch;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Ok:Pango.Font.find_shaper", kwlist, &PyPangoFont_Type, &self, &py_lang, &ch))
        return NULL;
    if (pyg_boxed_check(py_lang, PANGO_TYPE_LANGUAGE))
        lang = pyg_boxed_get(py_lang, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "lang should be a PangoLanguage");
        return NULL;
    }
    if (ch > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " ch parameter to unsigned 32 bit integer");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_CLASS(klass)->find_shaper)
        ret = PANGO_FONT_CLASS(klass)->find_shaper(PANGO_FONT(self->obj), lang, ch);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Font.find_shaper not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_PangoFont__do_get_metrics(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "language", NULL };
    PyGObject *self;
    PyObject *py_language;
    PangoLanguage *language = NULL;
    PangoFontMetrics *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Pango.Font.get_metrics", kwlist, &PyPangoFont_Type, &self, &py_language))
        return NULL;
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_CLASS(klass)->get_metrics)
        ret = PANGO_FONT_CLASS(klass)->get_metrics(PANGO_FONT(self->obj), language);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Font.get_metrics not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_METRICS, ret, TRUE, TRUE);
}

static PyObject *
_wrap_PangoFont__do_get_font_map(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    PangoFontMap *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Font.get_font_map", kwlist, &PyPangoFont_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_CLASS(klass)->get_font_map)
        ret = PANGO_FONT_CLASS(klass)->get_font_map(PANGO_FONT(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Font.get_font_map not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyPangoFont_methods[] = {
    { "describe", (PyCFunction)_wrap_pango_font_describe, METH_NOARGS,
      NULL },
    { "get_metrics", (PyCFunction)_wrap_pango_font_get_metrics, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_glyph_extents", (PyCFunction)_wrap_pango_font_get_glyph_extents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_font_map", (PyCFunction)_wrap_pango_font_get_font_map, METH_NOARGS,
      NULL },
    { "do_describe", (PyCFunction)_wrap_PangoFont__do_describe, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_find_shaper", (PyCFunction)_wrap_PangoFont__do_find_shaper, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_metrics", (PyCFunction)_wrap_PangoFont__do_get_metrics, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_font_map", (PyCFunction)_wrap_PangoFont__do_get_font_map, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFont_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Font",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFont_methods, /* tp_methods */
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

static PangoFontDescription*
_wrap_PangoFont__proxy_do_describe(PangoFont *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PangoFontDescription* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_describe");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    if (!pyg_boxed_check(py_retval, PANGO_TYPE_FONT_DESCRIPTION)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a PangoFontDescription");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    retval = pyg_boxed_get(py_retval, PangoFontDescription);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static PangoFontMetrics*
_wrap_PangoFont__proxy_do_get_metrics(PangoFont *self, PangoLanguage*language)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_language;
    PangoFontMetrics* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    py_language = pyg_boxed_new(PANGO_TYPE_LANGUAGE, language, FALSE, FALSE);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_language);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_metrics");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
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
        return pango_font_metrics_new();
    }
    if (!pyg_boxed_check(py_retval, PANGO_TYPE_FONT_METRICS)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a PangoFontMetrics");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    retval = pyg_boxed_get(py_retval, PangoFontMetrics);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static PangoFontMap*
_wrap_PangoFont__proxy_do_get_font_map(PangoFont *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PangoFontMap* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_font_map");
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
    retval = (PangoFontMap*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__PangoFont_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoFontClass *klass = PANGO_FONT_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_describe");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "describe")))
            klass->describe = _wrap_PangoFont__proxy_do_describe;
        Py_DECREF(o);
    }

    /* overriding do_get_coverage is currently not supported */

    /* overriding do_find_shaper is currently not supported */

    /* overriding do_get_glyph_extents is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_metrics");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_metrics")))
            klass->get_metrics = _wrap_PangoFont__proxy_do_get_metrics;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_font_map");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_font_map")))
            klass->get_font_map = _wrap_PangoFont__proxy_do_get_font_map;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- PangoFontFace ----------- */

static PyObject *
_wrap_pango_font_face_describe(PyGObject *self)
{
    PangoFontDescription *ret;

    
    ret = pango_font_face_describe(PANGO_FONT_FACE(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_font_face_get_face_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = pango_font_face_get_face_name(PANGO_FONT_FACE(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1626 "pango.override"
static PyObject *
_wrap_pango_font_face_list_sizes(PyGObject *self)
{
    PyObject *py_sizes;
    int *sizes, n_sizes, i;

    pango_font_face_list_sizes(PANGO_FONT_FACE(self->obj), &sizes, &n_sizes);


    if (!sizes) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    py_sizes = PyTuple_New(n_sizes);

    for (i = 0; i < n_sizes; i++)
	PyTuple_SetItem(py_sizes, i, PyInt_FromLong(sizes[i]));

    g_free(sizes);

    return py_sizes;
}
#line 4231 "pango.c"


static PyObject *
_wrap_PangoFontFace__do_get_face_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.FontFace.get_face_name", kwlist, &PyPangoFontFace_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_FACE_CLASS(klass)->get_face_name)
        ret = PANGO_FONT_FACE_CLASS(klass)->get_face_name(PANGO_FONT_FACE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontFace.get_face_name not implemented");
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
_wrap_PangoFontFace__do_describe(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    PangoFontDescription *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.FontFace.describe", kwlist, &PyPangoFontFace_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_FACE_CLASS(klass)->describe)
        ret = PANGO_FONT_FACE_CLASS(klass)->describe(PANGO_FONT_FACE(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontFace.describe not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, ret, TRUE, TRUE);
}

static const PyMethodDef _PyPangoFontFace_methods[] = {
    { "describe", (PyCFunction)_wrap_pango_font_face_describe, METH_NOARGS,
      NULL },
    { "get_face_name", (PyCFunction)_wrap_pango_font_face_get_face_name, METH_NOARGS,
      NULL },
    { "list_sizes", (PyCFunction)_wrap_pango_font_face_list_sizes, METH_NOARGS,
      NULL },
    { "do_get_face_name", (PyCFunction)_wrap_PangoFontFace__do_get_face_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_describe", (PyCFunction)_wrap_PangoFontFace__do_describe, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontFace_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontFace",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontFace_methods, /* tp_methods */
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

static PangoFontDescription*
_wrap_PangoFontFace__proxy_do_describe(PangoFontFace *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PangoFontDescription* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_describe");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    if (!pyg_boxed_check(py_retval, PANGO_TYPE_FONT_DESCRIPTION)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a PangoFontDescription");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_description_new();
    }
    retval = pyg_boxed_get(py_retval, PangoFontDescription);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__PangoFontFace_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoFontFaceClass *klass = PANGO_FONT_FACE_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    /* overriding do_get_face_name is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_describe");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "describe")))
            klass->describe = _wrap_PangoFontFace__proxy_do_describe;
        Py_DECREF(o);
    }

    /* overriding do_list_sizes is currently not supported */
    return 0;
}


/* ----------- PangoFontFamily ----------- */

#line 1000 "pango.override"
static PyObject *
_wrap_pango_font_family_list_faces(PyGObject *self)
{
    PangoFontFace **faces;
    gint n_faces, i;
    PyObject *ret;

    pango_font_family_list_faces(PANGO_FONT_FAMILY(self->obj),
				 &faces, &n_faces);
    ret = PyTuple_New(n_faces);
    for (i = 0; i < n_faces; i++) {
	PyObject *face;

	face = pygobject_new((GObject *)faces[i]);
	PyTuple_SetItem(ret, i, face);
    }
    g_free(faces);
    return ret;
}
#line 4444 "pango.c"


static PyObject *
_wrap_pango_font_family_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = pango_font_family_get_name(PANGO_FONT_FAMILY(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_family_is_monospace(PyGObject *self)
{
    int ret;

    
    ret = pango_font_family_is_monospace(PANGO_FONT_FAMILY(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_PangoFontFamily__do_get_name(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.FontFamily.get_name", kwlist, &PyPangoFontFamily_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_FAMILY_CLASS(klass)->get_name)
        ret = PANGO_FONT_FAMILY_CLASS(klass)->get_name(PANGO_FONT_FAMILY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontFamily.get_name not implemented");
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
_wrap_PangoFontFamily__do_is_monospace(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.FontFamily.is_monospace", kwlist, &PyPangoFontFamily_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_FAMILY_CLASS(klass)->is_monospace)
        ret = PANGO_FONT_FAMILY_CLASS(klass)->is_monospace(PANGO_FONT_FAMILY(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontFamily.is_monospace not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyPangoFontFamily_methods[] = {
    { "list_faces", (PyCFunction)_wrap_pango_font_family_list_faces, METH_NOARGS,
      NULL },
    { "get_name", (PyCFunction)_wrap_pango_font_family_get_name, METH_NOARGS,
      NULL },
    { "is_monospace", (PyCFunction)_wrap_pango_font_family_is_monospace, METH_NOARGS,
      NULL },
    { "do_get_name", (PyCFunction)_wrap_PangoFontFamily__do_get_name, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_monospace", (PyCFunction)_wrap_PangoFontFamily__do_is_monospace, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontFamily_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontFamily",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontFamily_methods, /* tp_methods */
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

static gboolean
_wrap_PangoFontFamily__proxy_do_is_monospace(PangoFontFamily *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_is_monospace");
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

static int
__PangoFontFamily_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoFontFamilyClass *klass = PANGO_FONT_FAMILY_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    /* overriding do_list_faces is currently not supported */

    /* overriding do_get_name is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_is_monospace");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "is_monospace")))
            klass->is_monospace = _wrap_PangoFontFamily__proxy_do_is_monospace;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- PangoFontMap ----------- */

static PyObject *
_wrap_pango_font_map_load_font(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", "desc", NULL };
    PyGObject *context;
    PyObject *py_desc, *py_ret;
    PangoFontDescription *desc = NULL;
    PangoFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Pango.FontMap.load_font", kwlist, &PyPangoContext_Type, &context, &py_desc))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    
    ret = pango_font_map_load_font(PANGO_FONT_MAP(self->obj), PANGO_CONTEXT(context->obj), desc);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_pango_font_map_load_fontset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", "desc", "language", NULL };
    PyGObject *context;
    PyObject *py_desc, *py_language, *py_ret;
    PangoFontDescription *desc = NULL;
    PangoLanguage *language = NULL;
    PangoFontset *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:Pango.FontMap.load_fontset", kwlist, &PyPangoContext_Type, &context, &py_desc, &py_language))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return NULL;
    }
    
    ret = pango_font_map_load_fontset(PANGO_FONT_MAP(self->obj), PANGO_CONTEXT(context->obj), desc, language);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1021 "pango.override"
static PyObject *
_wrap_pango_font_map_list_families(PyGObject *self)
{
    PangoFontFamily **families;
    gint n_families, i;
    PyObject *ret;

    pango_font_map_list_families(PANGO_FONT_MAP(self->obj), &families,
				 &n_families);
    ret = PyTuple_New(n_families);
    for (i = 0; i < n_families; i++) {
	PyObject *family;

	family = pygobject_new((GObject *)families[i]);
	PyTuple_SetItem(ret, i, family);
    }
    g_free(families);
    return ret;
}
#line 4744 "pango.c"


static PyObject *
_wrap_pango_font_map_get_shape_engine_type(PyGObject *self)
{
    const gchar *ret;

    
    ret = pango_font_map_get_shape_engine_type(PANGO_FONT_MAP(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_font_map_create_context(PyGObject *self)
{
    PangoContext *ret;
    PyObject *py_ret;

    
    ret = pango_font_map_create_context(PANGO_FONT_MAP(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_PangoFontMap__do_load_font(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "context", "desc", NULL };
    PyGObject *self, *context;
    PyObject *py_desc;
    PangoFontDescription *desc = NULL;
    PangoFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:Pango.FontMap.load_font", kwlist, &PyPangoFontMap_Type, &self, &PyPangoContext_Type, &context, &py_desc))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_MAP_CLASS(klass)->load_font)
        ret = PANGO_FONT_MAP_CLASS(klass)->load_font(PANGO_FONT_MAP(self->obj), PANGO_CONTEXT(context->obj), desc);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontMap.load_font not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_PangoFontMap__do_load_fontset(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "context", "desc", "language", NULL };
    PyGObject *self, *context;
    PyObject *py_desc, *py_language;
    PangoFontDescription *desc = NULL;
    PangoLanguage *language = NULL;
    PangoFontset *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!OO:Pango.FontMap.load_fontset", kwlist, &PyPangoFontMap_Type, &self, &PyPangoContext_Type, &context, &py_desc, &py_language))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription");
        return NULL;
    }
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONT_MAP_CLASS(klass)->load_fontset)
        ret = PANGO_FONT_MAP_CLASS(klass)->load_fontset(PANGO_FONT_MAP(self->obj), PANGO_CONTEXT(context->obj), desc, language);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.FontMap.load_fontset not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyPangoFontMap_methods[] = {
    { "load_font", (PyCFunction)_wrap_pango_font_map_load_font, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "load_fontset", (PyCFunction)_wrap_pango_font_map_load_fontset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "list_families", (PyCFunction)_wrap_pango_font_map_list_families, METH_NOARGS,
      NULL },
    { "get_shape_engine_type", (PyCFunction)_wrap_pango_font_map_get_shape_engine_type, METH_NOARGS,
      NULL },
    { "create_context", (PyCFunction)_wrap_pango_font_map_create_context, METH_NOARGS,
      NULL },
    { "do_load_font", (PyCFunction)_wrap_PangoFontMap__do_load_font, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_load_fontset", (PyCFunction)_wrap_PangoFontMap__do_load_fontset, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontMap_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontMap",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontMap_methods, /* tp_methods */
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

static PangoFont*
_wrap_PangoFontMap__proxy_do_load_font(PangoFontMap *self, PangoContext*context, const PangoFontDescription*desc)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_context = NULL;
    PyObject *py_desc;
    PangoFont* retval;
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
    if (context)
        py_context = pygobject_new((GObject *) context);
    else {
        Py_INCREF(Py_None);
        py_context = Py_None;
    }
    py_desc = pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, (PangoFontDescription*) desc, TRUE, TRUE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_context);
    PyTuple_SET_ITEM(py_args, 1, py_desc);
    
    py_method = PyObject_GetAttrString(py_self, "do_load_font");
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
    retval = (PangoFont*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static PangoFontset*
_wrap_PangoFontMap__proxy_do_load_fontset(PangoFontMap *self, PangoContext*context, const PangoFontDescription*desc, PangoLanguage*language)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_context = NULL;
    PyObject *py_desc;
    PyObject *py_language;
    PangoFontset* retval;
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
    if (context)
        py_context = pygobject_new((GObject *) context);
    else {
        Py_INCREF(Py_None);
        py_context = Py_None;
    }
    py_desc = pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, (PangoFontDescription*) desc, TRUE, TRUE);
    py_language = pyg_boxed_new(PANGO_TYPE_LANGUAGE, language, FALSE, FALSE);
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_context);
    PyTuple_SET_ITEM(py_args, 1, py_desc);
    PyTuple_SET_ITEM(py_args, 2, py_language);
    
    py_method = PyObject_GetAttrString(py_self, "do_load_fontset");
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
    retval = (PangoFontset*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__PangoFontMap_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoFontMapClass *klass = PANGO_FONT_MAP_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_load_font");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "load_font")))
            klass->load_font = _wrap_PangoFontMap__proxy_do_load_font;
        Py_DECREF(o);
    }

    /* overriding do_list_families is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_load_fontset");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "load_fontset")))
            klass->load_fontset = _wrap_PangoFontMap__proxy_do_load_fontset;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- PangoFontset ----------- */

static PyObject *
_wrap_pango_fontset_get_font(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "wc", NULL };
    PyObject *py_wc = NULL, *py_ret;
    PangoFont *ret;
    guint wc = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Fontset.get_font", kwlist, &py_wc))
        return NULL;
    if (py_wc) {
        if (PyLong_Check(py_wc))
            wc = PyLong_AsUnsignedLong(py_wc);
        else if (PyInt_Check(py_wc))
            wc = PyInt_AsLong(py_wc);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'wc' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = pango_fontset_get_font(PANGO_FONTSET(self->obj), wc);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_pango_fontset_get_metrics(PyGObject *self)
{
    PangoFontMetrics *ret;

    
    ret = pango_fontset_get_metrics(PANGO_FONTSET(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_METRICS, ret, FALSE, TRUE);
}

#line 1651 "pango.override"
static gboolean
pypango_fontset_foreach_cb(PangoFontset *fontset, PangoFont *font,
				 gpointer data)
{
    PyGILState_STATE state;
    PyGtkCustomNotify *cunote = data;
    PyObject *retobj, *py_font, *py_fontset;
    gboolean ret = FALSE;
    
    state = pyg_gil_state_ensure();

    py_fontset = pygobject_new((GObject *)fontset);
    py_font = pygobject_new((GObject *)font);

    if (cunote->data)
	retobj = PyObject_CallFunction(cunote->func, "NNO", py_fontset,
				       py_font, cunote->data);
    else
	retobj = PyObject_CallFunction(cunote->func, "NN", py_fontset,
				       py_font);

    if (retobj != NULL) {
	ret = PyObject_IsTrue(retobj);
        Py_DECREF(retobj);
    } else {
        PyErr_Print();
    }

    pyg_gil_state_release(state);
    return ret;
}
static PyObject *
_wrap_pango_fontset_foreach(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "func", "data", NULL };
    PyObject *py_func, *py_data = NULL;
    PyGtkCustomNotify cunote;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O|O:pango.Fontset.fforeach",
				     kwlist, &py_func, &py_data))
	return NULL;

    if (!PyCallable_Check(py_func)) {
        PyErr_SetString(PyExc_TypeError, "func must be callable");
        return NULL;
    }

    cunote.func = py_func;
    cunote.data = py_data;
    Py_INCREF(cunote.func);
    Py_XINCREF(cunote.data);

    pango_fontset_foreach(PANGO_FONTSET(self->obj),
			  pypango_fontset_foreach_cb,
			  (gpointer)&cunote);

    Py_DECREF(cunote.func);
    Py_XDECREF(cunote.data);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 5199 "pango.c"


static PyObject *
_wrap_PangoFontset__do_get_font(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "wc", NULL };
    PyGObject *self;
    PyObject *py_wc = NULL;
    guint wc = 0;
    PangoFont *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Pango.Fontset.get_font", kwlist, &PyPangoFontset_Type, &self, &py_wc))
        return NULL;
    if (py_wc) {
        if (PyLong_Check(py_wc))
            wc = PyLong_AsUnsignedLong(py_wc);
        else if (PyInt_Check(py_wc))
            wc = PyInt_AsLong(py_wc);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'wc' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONTSET_CLASS(klass)->get_font)
        ret = PANGO_FONTSET_CLASS(klass)->get_font(PANGO_FONTSET(self->obj), wc);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Fontset.get_font not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_PangoFontset__do_get_metrics(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    PangoFontMetrics *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Fontset.get_metrics", kwlist, &PyPangoFontset_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONTSET_CLASS(klass)->get_metrics)
        ret = PANGO_FONTSET_CLASS(klass)->get_metrics(PANGO_FONTSET(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Fontset.get_metrics not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_METRICS, ret, TRUE, TRUE);
}

static PyObject *
_wrap_PangoFontset__do_get_language(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    PangoLanguage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Fontset.get_language", kwlist, &PyPangoFontset_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_FONTSET_CLASS(klass)->get_language)
        ret = PANGO_FONTSET_CLASS(klass)->get_language(PANGO_FONTSET(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Fontset.get_language not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LANGUAGE, ret, TRUE, TRUE);
}

static const PyMethodDef _PyPangoFontset_methods[] = {
    { "get_font", (PyCFunction)_wrap_pango_fontset_get_font, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_metrics", (PyCFunction)_wrap_pango_fontset_get_metrics, METH_NOARGS,
      NULL },
    { "foreach", (PyCFunction)_wrap_pango_fontset_foreach, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_font", (PyCFunction)_wrap_PangoFontset__do_get_font, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_metrics", (PyCFunction)_wrap_PangoFontset__do_get_metrics, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_language", (PyCFunction)_wrap_PangoFontset__do_get_language, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontset_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Fontset",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontset_methods, /* tp_methods */
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

static PangoFont*
_wrap_PangoFontset__proxy_do_get_font(PangoFontset *self, guint wc)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_wc;
    PangoFont* retval;
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
    py_wc = PyInt_FromLong(wc);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_wc);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_font");
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
    retval = (PangoFont*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static PangoFontMetrics*
_wrap_PangoFontset__proxy_do_get_metrics(PangoFontset *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PangoFontMetrics* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_metrics");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    if (!pyg_boxed_check(py_retval, PANGO_TYPE_FONT_METRICS)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a PangoFontMetrics");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_font_metrics_new();
    }
    retval = pyg_boxed_get(py_retval, PangoFontMetrics);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static PangoLanguage*
_wrap_PangoFontset__proxy_do_get_language(PangoFontset *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PangoLanguage* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return pango_language_from_string("");
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_language");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_language_from_string("");
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_language_from_string("");
    }
    if (!pyg_boxed_check(py_retval, PANGO_TYPE_LANGUAGE)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a PangoLanguage");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return pango_language_from_string("");
    }
    retval = pyg_boxed_get(py_retval, PangoLanguage);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__PangoFontset_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoFontsetClass *klass = PANGO_FONTSET_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_font");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_font")))
            klass->get_font = _wrap_PangoFontset__proxy_do_get_font;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_metrics");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_metrics")))
            klass->get_metrics = _wrap_PangoFontset__proxy_do_get_metrics;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_language");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_language")))
            klass->get_language = _wrap_PangoFontset__proxy_do_get_language;
        Py_DECREF(o);
    }

    /* overriding do_foreach is currently not supported */
    return 0;
}


/* ----------- PangoFontsetSimple ----------- */

static int
_wrap_pango_fontset_simple_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "language", NULL };
    PyObject *py_language;
    PangoLanguage *language = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.FontsetSimple.__init__", kwlist, &py_language))
        return -1;
    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else {
        PyErr_SetString(PyExc_TypeError, "language should be a PangoLanguage");
        return -1;
    }
    self->obj = (GObject *)pango_fontset_simple_new(language);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoFontsetSimple object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_pango_fontset_simple_append(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", NULL };
    PyGObject *font;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.FontsetSimple.append", kwlist, &PyPangoFont_Type, &font))
        return NULL;
    
    pango_fontset_simple_append(PANGO_FONTSET_SIMPLE(self->obj), PANGO_FONT(font->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_fontset_simple_size(PyGObject *self)
{
    int ret;

    
    ret = pango_fontset_simple_size(PANGO_FONTSET_SIMPLE(self->obj));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyPangoFontsetSimple_methods[] = {
    { "append", (PyCFunction)_wrap_pango_fontset_simple_append, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "size", (PyCFunction)_wrap_pango_fontset_simple_size, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoFontsetSimple_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.FontsetSimple",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoFontsetSimple_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_pango_fontset_simple_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoLayout ----------- */

static int
_wrap_pango_layout_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", NULL };
    PyGObject *context;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Layout.__init__", kwlist, &PyPangoContext_Type, &context))
        return -1;
    self->obj = (GObject *)pango_layout_new(PANGO_CONTEXT(context->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create PangoLayout object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_pango_layout_copy(PyGObject *self)
{
    PyObject *py_ret;
    PangoLayout *ret;

    
    ret = pango_layout_copy(PANGO_LAYOUT(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_pango_layout_get_context(PyGObject *self)
{
    PangoContext *ret;

    
    ret = pango_layout_get_context(PANGO_LAYOUT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_layout_set_attributes(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attrs", NULL };
    PyObject *py_attrs;
    PangoAttrList *attrs = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_attributes", kwlist, &py_attrs))
        return NULL;
    if (pyg_boxed_check(py_attrs, PANGO_TYPE_ATTR_LIST))
        attrs = pyg_boxed_get(py_attrs, PangoAttrList);
    else {
        PyErr_SetString(PyExc_TypeError, "attrs should be a PangoAttrList");
        return NULL;
    }
    
    pango_layout_set_attributes(PANGO_LAYOUT(self->obj), attrs);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_attributes(PyGObject *self)
{
    PangoAttrList *ret;

    
    ret = pango_layout_get_attributes(PANGO_LAYOUT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_ATTR_LIST, ret, TRUE, TRUE);
}

#line 1405 "pango.override"
static PyObject *
_wrap_pango_layout_set_text(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", NULL };
    char *text;
    Py_ssize_t length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#:PangoLayout.set_text",
				     kwlist, &text, &length))
        return NULL;
    pango_layout_set_text(PANGO_LAYOUT(self->obj), text, length);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 5769 "pango.c"


static PyObject *
_wrap_pango_layout_get_text(PyGObject *self)
{
    const gchar *ret;

    
    ret = pango_layout_get_text(PANGO_LAYOUT(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1132 "pango.override"
static PyObject *
_wrap_pango_layout_set_markup(PyGObject *self, PyObject *args,PyObject *kwargs)
{
    static char *kwlist[] = { "markup", NULL };
    char *markup;
    Py_ssize_t length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#:PangoLayout.set_markup",
				     kwlist, &markup, &length))
	return NULL;

    pango_layout_set_markup(PANGO_LAYOUT(self->obj), markup, length);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 5803 "pango.c"


#line 1150 "pango.override"
static PyObject *
_wrap_pango_layout_set_markup_with_accel(PyGObject *self, PyObject *args,
					 PyObject *kwargs)
{
    static char *kwlist[] = { "markup", "accel_marker", NULL };
    char *markup;
    Py_ssize_t length, accel_length;
    Py_UNICODE *accel_marker, pychr;
    gunichar accel_char;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s#u#:PangoLayout.set_markup_with_accel",
				     kwlist, &markup, &length,
				     &accel_marker, &accel_length))
	return NULL;
    if (accel_length != 1) {
	PyErr_SetString(PyExc_TypeError, "accel_marker must be a unicode string of length 1");
	return NULL;
    }
    pango_layout_set_markup_with_accel(PANGO_LAYOUT(self->obj), markup, length,
				       (gunichar)accel_marker[0], &accel_char);

#if !defined(Py_UNICODE_SIZE) || Py_UNICODE_SIZE == 2
    if (accel_char >= 0xffff) {
	PyErr_SetString(PyExc_ValueError, "unicode character is too big to fit in a 16-bit unicode character");
	return NULL;
    }
#endif
    pychr = (Py_UNICODE)accel_char;
    return PyUnicode_FromUnicode(&pychr, 1);
}
#line 5838 "pango.c"


static PyObject *
_wrap_pango_layout_set_font_description(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "desc", NULL };
    PyObject *py_desc = Py_None;
    PangoFontDescription *desc = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_font_description", kwlist, &py_desc))
        return NULL;
    if (pyg_boxed_check(py_desc, PANGO_TYPE_FONT_DESCRIPTION))
        desc = pyg_boxed_get(py_desc, PangoFontDescription);
    else if (py_desc != Py_None) {
        PyErr_SetString(PyExc_TypeError, "desc should be a PangoFontDescription or None");
        return NULL;
    }
    
    pango_layout_set_font_description(PANGO_LAYOUT(self->obj), desc);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_font_description(PyGObject *self)
{
    const PangoFontDescription *ret;

    
    ret = pango_layout_get_font_description(PANGO_LAYOUT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_FONT_DESCRIPTION, (PangoFontDescription*) ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_layout_set_width(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", NULL };
    int width;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_width", kwlist, &width))
        return NULL;
    
    pango_layout_set_width(PANGO_LAYOUT(self->obj), width);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_width(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_width(PANGO_LAYOUT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_set_wrap(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "wrap", NULL };
    PyObject *py_wrap = NULL;
    PangoWrapMode wrap;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_wrap", kwlist, &py_wrap))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_WRAP_MODE, py_wrap, (gpointer)&wrap))
        return NULL;
    
    pango_layout_set_wrap(PANGO_LAYOUT(self->obj), wrap);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_wrap(PyGObject *self)
{
    gint ret;

    
    ret = pango_layout_get_wrap(PANGO_LAYOUT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_WRAP_MODE, ret);
}

static PyObject *
_wrap_pango_layout_is_wrapped(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_is_wrapped(PANGO_LAYOUT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_set_indent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "indent", NULL };
    int indent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_indent", kwlist, &indent))
        return NULL;
    
    pango_layout_set_indent(PANGO_LAYOUT(self->obj), indent);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_indent(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_indent(PANGO_LAYOUT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_set_spacing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "spacing", NULL };
    int spacing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_spacing", kwlist, &spacing))
        return NULL;
    
    pango_layout_set_spacing(PANGO_LAYOUT(self->obj), spacing);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_spacing(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_spacing(PANGO_LAYOUT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_set_justify(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "justify", NULL };
    int justify;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_justify", kwlist, &justify))
        return NULL;
    
    pango_layout_set_justify(PANGO_LAYOUT(self->obj), justify);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_justify(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_justify(PANGO_LAYOUT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_set_auto_dir(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "auto_dir", NULL };
    int auto_dir;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_auto_dir", kwlist, &auto_dir))
        return NULL;
    
    pango_layout_set_auto_dir(PANGO_LAYOUT(self->obj), auto_dir);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_auto_dir(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_auto_dir(PANGO_LAYOUT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_set_alignment(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "alignment", NULL };
    PyObject *py_alignment = NULL;
    PangoAlignment alignment;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_alignment", kwlist, &py_alignment))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_ALIGNMENT, py_alignment, (gpointer)&alignment))
        return NULL;
    
    pango_layout_set_alignment(PANGO_LAYOUT(self->obj), alignment);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_alignment(PyGObject *self)
{
    gint ret;

    
    ret = pango_layout_get_alignment(PANGO_LAYOUT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_ALIGNMENT, ret);
}

static PyObject *
_wrap_pango_layout_set_tabs(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tabs", NULL };
    PyObject *py_tabs = Py_None;
    PangoTabArray *tabs = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_tabs", kwlist, &py_tabs))
        return NULL;
    if (pyg_boxed_check(py_tabs, PANGO_TYPE_TAB_ARRAY))
        tabs = pyg_boxed_get(py_tabs, PangoTabArray);
    else if (py_tabs != Py_None) {
        PyErr_SetString(PyExc_TypeError, "tabs should be a PangoTabArray or None");
        return NULL;
    }
    
    pango_layout_set_tabs(PANGO_LAYOUT(self->obj), tabs);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_tabs(PyGObject *self)
{
    PangoTabArray *ret;

    
    ret = pango_layout_get_tabs(PANGO_LAYOUT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_TAB_ARRAY, ret, FALSE, TRUE);
}

static PyObject *
_wrap_pango_layout_set_single_paragraph_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "setting", NULL };
    int setting;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.set_single_paragraph_mode", kwlist, &setting))
        return NULL;
    
    pango_layout_set_single_paragraph_mode(PANGO_LAYOUT(self->obj), setting);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_single_paragraph_mode(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_single_paragraph_mode(PANGO_LAYOUT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_set_ellipsize(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ellipsize", NULL };
    PyObject *py_ellipsize = NULL;
    PangoEllipsizeMode ellipsize;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Layout.set_ellipsize", kwlist, &py_ellipsize))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_ELLIPSIZE_MODE, py_ellipsize, (gpointer)&ellipsize))
        return NULL;
    
    pango_layout_set_ellipsize(PANGO_LAYOUT(self->obj), ellipsize);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_layout_get_ellipsize(PyGObject *self)
{
    gint ret;

    
    ret = pango_layout_get_ellipsize(PANGO_LAYOUT(self->obj));
    
    return pyg_enum_from_gtype(PANGO_TYPE_ELLIPSIZE_MODE, ret);
}

static PyObject *
_wrap_pango_layout_is_ellipsized(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_is_ellipsized(PANGO_LAYOUT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_pango_layout_get_unknown_glyphs_count(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_unknown_glyphs_count(PANGO_LAYOUT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_context_changed(PyGObject *self)
{
    
    pango_layout_context_changed(PANGO_LAYOUT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1183 "pango.override"
static PyObject *
_wrap_pango_layout_index_to_pos(PyGObject *self, PyObject *args,
				PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    gint index;
    PangoRectangle pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:PangoLayout.index_to_pos", kwlist,
				     &index))
	return NULL;

    pango_layout_index_to_pos(PANGO_LAYOUT(self->obj), index, &pos);
    return Py_BuildValue("(iiii)", pos.x, pos.y, pos.width, pos.height);
}
#line 6217 "pango.c"


#line 1201 "pango.override"
static PyObject *
_wrap_pango_layout_get_cursor_pos(PyGObject *self, PyObject *args,
				  PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    gint index;
    PangoRectangle strong_pos, weak_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i:PangoLayout.get_cursor_pos", kwlist,
				     &index))
	return NULL;

    pango_layout_get_cursor_pos(PANGO_LAYOUT(self->obj), index,
				&strong_pos, &weak_pos);
    return Py_BuildValue("((iiii)(iiii))",
			 strong_pos.x, strong_pos.y,
			 strong_pos.width, strong_pos.height,
			 weak_pos.x, weak_pos.y,
			 weak_pos.width, weak_pos.height);
}
#line 6242 "pango.c"


#line 1224 "pango.override"
static PyObject *
_wrap_pango_layout_move_cursor_visually(PyGObject *self, PyObject *args,
					PyObject *kwargs)
{
    static char *kwlist[] = { "strong", "old_index", "old_trailing", "direction", NULL };
    gboolean strong;
    gint old_index, old_trailing, direction, new_index = 0, new_trailing = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iiii:PangoLayout.move_cursor_visually",
				     kwlist, &strong, &old_index,
				     &old_trailing, &direction))
	return NULL;

    pango_layout_move_cursor_visually(PANGO_LAYOUT(self->obj), strong,
				      old_index, old_trailing, direction,
				      &new_index, &new_trailing);
    return Py_BuildValue("(ii)", new_index, new_trailing);
}
#line 6265 "pango.c"


#line 1245 "pango.override"
static PyObject *
_wrap_pango_layout_xy_to_index(PyGObject *self, PyObject *args,
			       PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    gint x, y, index, trailing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "ii:PangoLayout.xy_to_index", kwlist,
				     &x, &y))
	return NULL;

    pango_layout_xy_to_index(PANGO_LAYOUT(self->obj), x, y, &index, &trailing);

    return Py_BuildValue("(ii)", index, trailing);
}
#line 6285 "pango.c"


#line 1263 "pango.override"
static PyObject *
_wrap_pango_layout_get_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_get_extents(PANGO_LAYOUT(self->obj),
			     &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 6303 "pango.c"


#line 1279 "pango.override"
static PyObject *
_wrap_pango_layout_get_pixel_extents(PyGObject *self)
{
    PangoRectangle ink_rect, logical_rect;

    pango_layout_get_pixel_extents(PANGO_LAYOUT(self->obj),
				   &ink_rect, &logical_rect);

    return Py_BuildValue("((iiii)(iiii))",
			 ink_rect.x, ink_rect.y,
			 ink_rect.width, ink_rect.height,
			 logical_rect.x, logical_rect.y,
			 logical_rect.width, logical_rect.height);
}
#line 6321 "pango.c"


#line 1295 "pango.override"
static PyObject *
_wrap_pango_layout_get_size(PyGObject *self)
{
    gint width, height;

    pango_layout_get_size(PANGO_LAYOUT(self->obj), &width, &height);

    return Py_BuildValue("(ii)", width, height);
}
#line 6334 "pango.c"


#line 1306 "pango.override"
static PyObject *
_wrap_pango_layout_get_pixel_size(PyGObject *self)
{
    gint width, height;

    pango_layout_get_pixel_size(PANGO_LAYOUT(self->obj), &width, &height);

    return Py_BuildValue("(ii)", width, height);
}
#line 6347 "pango.c"


static PyObject *
_wrap_pango_layout_get_line_count(PyGObject *self)
{
    int ret;

    
    ret = pango_layout_get_line_count(PANGO_LAYOUT(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_layout_get_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line", NULL };
    int line;
    PangoLayoutLine *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.get_line", kwlist, &line))
        return NULL;
    
    ret = pango_layout_get_line(PANGO_LAYOUT(self->obj), line);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LAYOUT_LINE, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_layout_get_line_readonly(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line", NULL };
    int line;
    PangoLayoutLine *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:Pango.Layout.get_line_readonly", kwlist, &line))
        return NULL;
    
    ret = pango_layout_get_line_readonly(PANGO_LAYOUT(self->obj), line);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LAYOUT_LINE, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_layout_get_iter(PyGObject *self)
{
    PangoLayoutIter *ret;

    
    ret = pango_layout_get_iter(PANGO_LAYOUT(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LAYOUT_ITER, ret, FALSE, TRUE);
}

static const PyMethodDef _PyPangoLayout_methods[] = {
    { "copy", (PyCFunction)_wrap_pango_layout_copy, METH_NOARGS,
      NULL },
    { "get_context", (PyCFunction)_wrap_pango_layout_get_context, METH_NOARGS,
      NULL },
    { "set_attributes", (PyCFunction)_wrap_pango_layout_set_attributes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attributes", (PyCFunction)_wrap_pango_layout_get_attributes, METH_NOARGS,
      NULL },
    { "set_text", (PyCFunction)_wrap_pango_layout_set_text, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_text", (PyCFunction)_wrap_pango_layout_get_text, METH_NOARGS,
      NULL },
    { "set_markup", (PyCFunction)_wrap_pango_layout_set_markup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_markup_with_accel", (PyCFunction)_wrap_pango_layout_set_markup_with_accel, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_font_description", (PyCFunction)_wrap_pango_layout_set_font_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_font_description", (PyCFunction)_wrap_pango_layout_get_font_description, METH_NOARGS,
      NULL },
    { "set_width", (PyCFunction)_wrap_pango_layout_set_width, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_width", (PyCFunction)_wrap_pango_layout_get_width, METH_NOARGS,
      NULL },
    { "set_wrap", (PyCFunction)_wrap_pango_layout_set_wrap, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_wrap", (PyCFunction)_wrap_pango_layout_get_wrap, METH_NOARGS,
      NULL },
    { "is_wrapped", (PyCFunction)_wrap_pango_layout_is_wrapped, METH_NOARGS,
      NULL },
    { "set_indent", (PyCFunction)_wrap_pango_layout_set_indent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_indent", (PyCFunction)_wrap_pango_layout_get_indent, METH_NOARGS,
      NULL },
    { "set_spacing", (PyCFunction)_wrap_pango_layout_set_spacing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_spacing", (PyCFunction)_wrap_pango_layout_get_spacing, METH_NOARGS,
      NULL },
    { "set_justify", (PyCFunction)_wrap_pango_layout_set_justify, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_justify", (PyCFunction)_wrap_pango_layout_get_justify, METH_NOARGS,
      NULL },
    { "set_auto_dir", (PyCFunction)_wrap_pango_layout_set_auto_dir, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_auto_dir", (PyCFunction)_wrap_pango_layout_get_auto_dir, METH_NOARGS,
      NULL },
    { "set_alignment", (PyCFunction)_wrap_pango_layout_set_alignment, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_alignment", (PyCFunction)_wrap_pango_layout_get_alignment, METH_NOARGS,
      NULL },
    { "set_tabs", (PyCFunction)_wrap_pango_layout_set_tabs, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_tabs", (PyCFunction)_wrap_pango_layout_get_tabs, METH_NOARGS,
      NULL },
    { "set_single_paragraph_mode", (PyCFunction)_wrap_pango_layout_set_single_paragraph_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_single_paragraph_mode", (PyCFunction)_wrap_pango_layout_get_single_paragraph_mode, METH_NOARGS,
      NULL },
    { "set_ellipsize", (PyCFunction)_wrap_pango_layout_set_ellipsize, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_ellipsize", (PyCFunction)_wrap_pango_layout_get_ellipsize, METH_NOARGS,
      NULL },
    { "is_ellipsized", (PyCFunction)_wrap_pango_layout_is_ellipsized, METH_NOARGS,
      NULL },
    { "get_unknown_glyphs_count", (PyCFunction)_wrap_pango_layout_get_unknown_glyphs_count, METH_NOARGS,
      NULL },
    { "context_changed", (PyCFunction)_wrap_pango_layout_context_changed, METH_NOARGS,
      NULL },
    { "index_to_pos", (PyCFunction)_wrap_pango_layout_index_to_pos, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_cursor_pos", (PyCFunction)_wrap_pango_layout_get_cursor_pos, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "move_cursor_visually", (PyCFunction)_wrap_pango_layout_move_cursor_visually, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "xy_to_index", (PyCFunction)_wrap_pango_layout_xy_to_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_extents", (PyCFunction)_wrap_pango_layout_get_extents, METH_NOARGS,
      NULL },
    { "get_pixel_extents", (PyCFunction)_wrap_pango_layout_get_pixel_extents, METH_NOARGS,
      NULL },
    { "get_size", (PyCFunction)_wrap_pango_layout_get_size, METH_NOARGS,
      NULL },
    { "get_pixel_size", (PyCFunction)_wrap_pango_layout_get_pixel_size, METH_NOARGS,
      NULL },
    { "get_line_count", (PyCFunction)_wrap_pango_layout_get_line_count, METH_NOARGS,
      NULL },
    { "get_line", (PyCFunction)_wrap_pango_layout_get_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_line_readonly", (PyCFunction)_wrap_pango_layout_get_line_readonly, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_iter", (PyCFunction)_wrap_pango_layout_get_iter, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoLayout_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Layout",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoLayout_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_pango_layout_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- PangoRenderer ----------- */

static PyObject *
_wrap_pango_renderer_draw_layout(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layout", "x", "y", NULL };
    PyGObject *layout;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:Pango.Renderer.draw_layout", kwlist, &PyPangoLayout_Type, &layout, &x, &y))
        return NULL;
    
    pango_renderer_draw_layout(PANGO_RENDERER(self->obj), PANGO_LAYOUT(layout->obj), x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_draw_layout_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line", "x", "y", NULL };
    PangoLayoutLine *line = NULL;
    PyObject *py_line;
    int x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oii:Pango.Renderer.draw_layout_line", kwlist, &py_line, &x, &y))
        return NULL;
    if (pyg_boxed_check(py_line, PANGO_TYPE_LAYOUT_LINE))
        line = pyg_boxed_get(py_line, PangoLayoutLine);
    else {
        PyErr_SetString(PyExc_TypeError, "line should be a PangoLayoutLine");
        return NULL;
    }
    
    pango_renderer_draw_layout_line(PANGO_RENDERER(self->obj), line, x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_draw_glyphs(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", "glyphs", "x", "y", NULL };
    PyGObject *font;
    PangoGlyphString *glyphs = NULL;
    int x, y;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Oii:Pango.Renderer.draw_glyphs", kwlist, &PyPangoFont_Type, &font, &py_glyphs, &x, &y))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    
    pango_renderer_draw_glyphs(PANGO_RENDERER(self->obj), PANGO_FONT(font->obj), glyphs, x, y);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_draw_rectangle(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", "x", "y", "width", "height", NULL };
    PyObject *py_part = NULL;
    int x, y, width, height;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Oiiii:Pango.Renderer.draw_rectangle", kwlist, &py_part, &x, &y, &width, &height))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    
    pango_renderer_draw_rectangle(PANGO_RENDERER(self->obj), part, x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_draw_error_underline(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "width", "height", NULL };
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iiii:Pango.Renderer.draw_error_underline", kwlist, &x, &y, &width, &height))
        return NULL;
    
    pango_renderer_draw_error_underline(PANGO_RENDERER(self->obj), x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_draw_trapezoid(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", "y1_", "x11", "x21", "y2", "x12", "x22", NULL };
    PyObject *py_part = NULL;
    double y1_, x11, x21, y2, x12, x22;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Odddddd:Pango.Renderer.draw_trapezoid", kwlist, &py_part, &y1_, &x11, &x21, &y2, &x12, &x22))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    
    pango_renderer_draw_trapezoid(PANGO_RENDERER(self->obj), part, y1_, x11, x21, y2, x12, x22);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_activate(PyGObject *self)
{
    
    pango_renderer_activate(PANGO_RENDERER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_deactivate(PyGObject *self)
{
    
    pango_renderer_deactivate(PANGO_RENDERER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_part_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", NULL };
    PyObject *py_part = NULL;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Renderer.part_changed", kwlist, &py_part))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    
    pango_renderer_part_changed(PANGO_RENDERER(self->obj), part);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_set_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", "color", NULL };
    PyObject *py_part = NULL, *py_color;
    PangoColor *color = NULL;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:Pango.Renderer.set_color", kwlist, &py_part, &py_color))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    if (pyg_boxed_check(py_color, PANGO_TYPE_COLOR))
        color = pyg_boxed_get(py_color, PangoColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a PangoColor");
        return NULL;
    }
    
    pango_renderer_set_color(PANGO_RENDERER(self->obj), part, color);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_get_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "part", NULL };
    PyObject *py_part = NULL;
    PangoColor *ret;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Renderer.get_color", kwlist, &py_part))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    
    ret = pango_renderer_get_color(PANGO_RENDERER(self->obj), part);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_COLOR, ret, TRUE, TRUE);
}

static PyObject *
_wrap_pango_renderer_set_matrix(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    PangoMatrix *matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:Pango.Renderer.set_matrix", kwlist, &py_matrix))
        return NULL;
    if (pyg_boxed_check(py_matrix, PANGO_TYPE_MATRIX))
        matrix = pyg_boxed_get(py_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "matrix should be a PangoMatrix");
        return NULL;
    }
    
    pango_renderer_set_matrix(PANGO_RENDERER(self->obj), matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_renderer_get_matrix(PyGObject *self)
{
    const PangoMatrix *ret;

    
    ret = pango_renderer_get_matrix(PANGO_RENDERER(self->obj));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_MATRIX, (PangoMatrix*) ret, TRUE, TRUE);
}

static PyObject *
_wrap_PangoRenderer__do_draw_glyphs(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "font", "glyphs", "x", "y", NULL };
    PyGObject *self, *font;
    PangoGlyphString *glyphs = NULL;
    int x, y;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!Oii:Pango.Renderer.draw_glyphs", kwlist, &PyPangoRenderer_Type, &self, &PyPangoFont_Type, &font, &py_glyphs, &x, &y))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->draw_glyphs)
        PANGO_RENDERER_CLASS(klass)->draw_glyphs(PANGO_RENDERER(self->obj), PANGO_FONT(font->obj), glyphs, x, y);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.draw_glyphs not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_draw_rectangle(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "part", "x", "y", "width", "height", NULL };
    PyGObject *self;
    PyObject *py_part = NULL;
    int x, y, width, height;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Oiiii:Pango.Renderer.draw_rectangle", kwlist, &PyPangoRenderer_Type, &self, &py_part, &x, &y, &width, &height))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->draw_rectangle)
        PANGO_RENDERER_CLASS(klass)->draw_rectangle(PANGO_RENDERER(self->obj), part, x, y, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.draw_rectangle not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_draw_error_underline(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "x", "y", "width", "height", NULL };
    PyGObject *self;
    int x, y, width, height;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iiii:Pango.Renderer.draw_error_underline", kwlist, &PyPangoRenderer_Type, &self, &x, &y, &width, &height))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->draw_error_underline)
        PANGO_RENDERER_CLASS(klass)->draw_error_underline(PANGO_RENDERER(self->obj), x, y, width, height);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.draw_error_underline not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_draw_trapezoid(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "part", "y1_", "x11", "x21", "y2", "x12", "x22", NULL };
    PyGObject *self;
    PyObject *py_part = NULL;
    PangoRenderPart part;
    double y1_, x11, x21, y2, x12, x22;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Odddddd:Pango.Renderer.draw_trapezoid", kwlist, &PyPangoRenderer_Type, &self, &py_part, &y1_, &x11, &x21, &y2, &x12, &x22))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->draw_trapezoid)
        PANGO_RENDERER_CLASS(klass)->draw_trapezoid(PANGO_RENDERER(self->obj), part, y1_, x11, x21, y2, x12, x22);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.draw_trapezoid not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_part_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "part", NULL };
    PyGObject *self;
    PyObject *py_part = NULL;
    PangoRenderPart part;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:Pango.Renderer.part_changed", kwlist, &PyPangoRenderer_Type, &self, &py_part))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_RENDER_PART, py_part, (gpointer)&part))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->part_changed)
        PANGO_RENDERER_CLASS(klass)->part_changed(PANGO_RENDERER(self->obj), part);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.part_changed not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_begin(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Renderer.begin", kwlist, &PyPangoRenderer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->begin)
        PANGO_RENDERER_CLASS(klass)->begin(PANGO_RENDERER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.begin not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_PangoRenderer__do_end(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:Pango.Renderer.end", kwlist, &PyPangoRenderer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (PANGO_RENDERER_CLASS(klass)->end)
        PANGO_RENDERER_CLASS(klass)->end(PANGO_RENDERER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method Pango.Renderer.end not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyPangoRenderer_methods[] = {
    { "draw_layout", (PyCFunction)_wrap_pango_renderer_draw_layout, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_layout_line", (PyCFunction)_wrap_pango_renderer_draw_layout_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_glyphs", (PyCFunction)_wrap_pango_renderer_draw_glyphs, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_rectangle", (PyCFunction)_wrap_pango_renderer_draw_rectangle, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_error_underline", (PyCFunction)_wrap_pango_renderer_draw_error_underline, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "draw_trapezoid", (PyCFunction)_wrap_pango_renderer_draw_trapezoid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "activate", (PyCFunction)_wrap_pango_renderer_activate, METH_NOARGS,
      NULL },
    { "deactivate", (PyCFunction)_wrap_pango_renderer_deactivate, METH_NOARGS,
      NULL },
    { "part_changed", (PyCFunction)_wrap_pango_renderer_part_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_color", (PyCFunction)_wrap_pango_renderer_set_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_color", (PyCFunction)_wrap_pango_renderer_get_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_matrix", (PyCFunction)_wrap_pango_renderer_set_matrix, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_matrix", (PyCFunction)_wrap_pango_renderer_get_matrix, METH_NOARGS,
      NULL },
    { "do_draw_glyphs", (PyCFunction)_wrap_PangoRenderer__do_draw_glyphs, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_rectangle", (PyCFunction)_wrap_PangoRenderer__do_draw_rectangle, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_error_underline", (PyCFunction)_wrap_PangoRenderer__do_draw_error_underline, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_draw_trapezoid", (PyCFunction)_wrap_PangoRenderer__do_draw_trapezoid, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_part_changed", (PyCFunction)_wrap_PangoRenderer__do_part_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_begin", (PyCFunction)_wrap_PangoRenderer__do_begin, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_end", (PyCFunction)_wrap_PangoRenderer__do_end, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoRenderer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "pango.Renderer",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoRenderer_methods, /* tp_methods */
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
_wrap_PangoRenderer__proxy_do_draw_glyphs(PangoRenderer *self, PangoFont*font, PangoGlyphString*glyphs, int x, int y)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_font = NULL;
    PyObject *py_glyphs;
    PyObject *py_x;
    PyObject *py_y;
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
    if (font)
        py_font = pygobject_new((GObject *) font);
    else {
        Py_INCREF(Py_None);
        py_font = Py_None;
    }
    py_glyphs = pyg_boxed_new(PANGO_TYPE_GLYPH_STRING, glyphs, FALSE, FALSE);
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    
    py_args = PyTuple_New(4);
    PyTuple_SET_ITEM(py_args, 0, py_font);
    PyTuple_SET_ITEM(py_args, 1, py_glyphs);
    PyTuple_SET_ITEM(py_args, 2, py_x);
    PyTuple_SET_ITEM(py_args, 3, py_y);
    
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
_wrap_PangoRenderer__proxy_do_draw_rectangle(PangoRenderer *self, PangoRenderPart part, int x, int y, int width, int height)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_part;
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
    py_part = pyg_enum_from_gtype(PANGO_TYPE_RENDER_PART, part);
    if (!py_part) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_x = PyInt_FromLong(x);
    py_y = PyInt_FromLong(y);
    py_width = PyInt_FromLong(width);
    py_height = PyInt_FromLong(height);
    
    py_args = PyTuple_New(5);
    PyTuple_SET_ITEM(py_args, 0, py_part);
    PyTuple_SET_ITEM(py_args, 1, py_x);
    PyTuple_SET_ITEM(py_args, 2, py_y);
    PyTuple_SET_ITEM(py_args, 3, py_width);
    PyTuple_SET_ITEM(py_args, 4, py_height);
    
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
_wrap_PangoRenderer__proxy_do_draw_error_underline(PangoRenderer *self, int x, int y, int width, int height)
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
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_error_underline");
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
_wrap_PangoRenderer__proxy_do_draw_trapezoid(PangoRenderer *self, PangoRenderPart part, double y1_, double x11, double x21, double y2, double x12, double x22)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_part;
    PyObject *py_y1_;
    PyObject *py_x11;
    PyObject *py_x21;
    PyObject *py_y2;
    PyObject *py_x12;
    PyObject *py_x22;
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
    py_part = pyg_enum_from_gtype(PANGO_TYPE_RENDER_PART, part);
    if (!py_part) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_y1_ = PyFloat_FromDouble(y1_);
    py_x11 = PyFloat_FromDouble(x11);
    py_x21 = PyFloat_FromDouble(x21);
    py_y2 = PyFloat_FromDouble(y2);
    py_x12 = PyFloat_FromDouble(x12);
    py_x22 = PyFloat_FromDouble(x22);
    
    py_args = PyTuple_New(7);
    PyTuple_SET_ITEM(py_args, 0, py_part);
    PyTuple_SET_ITEM(py_args, 1, py_y1_);
    PyTuple_SET_ITEM(py_args, 2, py_x11);
    PyTuple_SET_ITEM(py_args, 3, py_x21);
    PyTuple_SET_ITEM(py_args, 4, py_y2);
    PyTuple_SET_ITEM(py_args, 5, py_x12);
    PyTuple_SET_ITEM(py_args, 6, py_x22);
    
    py_method = PyObject_GetAttrString(py_self, "do_draw_trapezoid");
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
_wrap_PangoRenderer__proxy_do_part_changed(PangoRenderer *self, PangoRenderPart part)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_part;
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
    py_part = pyg_enum_from_gtype(PANGO_TYPE_RENDER_PART, part);
    if (!py_part) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_part);
    
    py_method = PyObject_GetAttrString(py_self, "do_part_changed");
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
_wrap_PangoRenderer__proxy_do_begin(PangoRenderer *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_begin");
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
_wrap_PangoRenderer__proxy_do_end(PangoRenderer *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_end");
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
__PangoRenderer_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    PangoRendererClass *klass = PANGO_RENDERER_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_glyphs");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_glyphs")))
            klass->draw_glyphs = _wrap_PangoRenderer__proxy_do_draw_glyphs;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_rectangle");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_rectangle")))
            klass->draw_rectangle = _wrap_PangoRenderer__proxy_do_draw_rectangle;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_error_underline");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_error_underline")))
            klass->draw_error_underline = _wrap_PangoRenderer__proxy_do_draw_error_underline;
        Py_DECREF(o);
    }

    /* overriding do_draw_shape is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_draw_trapezoid");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "draw_trapezoid")))
            klass->draw_trapezoid = _wrap_PangoRenderer__proxy_do_draw_trapezoid;
        Py_DECREF(o);
    }

    /* overriding do_draw_glyph is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_part_changed");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "part_changed")))
            klass->part_changed = _wrap_PangoRenderer__proxy_do_part_changed;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_begin");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "begin")))
            klass->begin = _wrap_PangoRenderer__proxy_do_begin;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_end");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "end")))
            klass->end = _wrap_PangoRenderer__proxy_do_end;
        Py_DECREF(o);
    }

    /* overriding do_prepare_run is currently not supported */
    return 0;
}


/* ----------- functions ----------- */

static PyObject *
_wrap_pango_attr_type_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:pango_attr_type_register", kwlist, &name))
        return NULL;
    
    ret = pango_attr_type_register(name);
    
    return pyg_enum_from_gtype(PANGO_TYPE_ATTR_TYPE, ret);
}

#line 550 "pango.override"
static PyObject *
_wrap_pango_attr_language_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "language", "start_index", "end_index", NULL };
    char *slanguage;
    PangoLanguage *language;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ii:PangoAttrLanguage",
				     kwlist, &slanguage, &start, &end))
	return NULL;

    language = pango_language_from_string(slanguage);

    return pypango_attr_new(pango_attr_language_new(language), start, end);
}
#line 7653 "pango.c"


#line 568 "pango.override"
static PyObject *
_wrap_pango_attr_family_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", "start_index", "end_index", NULL };
    char *family;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ii:PangoAttrFamily",
				     kwlist, &family, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_family_new(family), start, end);
}
#line 7670 "pango.c"


#line 583 "pango.override"
static PyObject *
_wrap_pango_attr_foreground_new(PyObject *self,PyObject *args,PyObject *kwargs)
{
    static char *kwlist[] = { "red", "green", "blue", "start_index",
			      "end_index", NULL };
    guint16 red, green, blue;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "HHH|ii:PangoAttrForeground",
				     kwlist, &red, &green, &blue,
				     &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_foreground_new(red, green, blue),
			    start, end);
}
#line 7691 "pango.c"


#line 602 "pango.override"
static PyObject *
_wrap_pango_attr_background_new(PyObject *self,PyObject *args,PyObject *kwargs)
{
    static char *kwlist[] = { "red", "green", "blue", "start_index",
			      "end_index", NULL };
    guint16 red, green, blue;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "HHH|ii:PangoAttrBackground",
				     kwlist, &red, &green, &blue,
				     &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_background_new(red, green, blue),
			    start, end);
}
#line 7712 "pango.c"


#line 621 "pango.override"
static PyObject *
_wrap_pango_attr_size_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", "start_index", "end_index", NULL };
    int size;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|ii:PangoAttrSize",
				     kwlist, &size, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_size_new(size), start, end);
}
#line 7729 "pango.c"


#line 1988 "pango.override"
static PyObject *
_wrap_pango_attr_size_new_absolute(PyObject *self, PyObject *args,
                                   PyObject *kwargs)
{
    static char *kwlist[] = { "size", "start_index", "end_index", NULL };
    int size;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i|ii:PangoAttrSizeAbsolute",
				     kwlist, &size, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_size_new_absolute(size), start, end);
}
#line 7748 "pango.c"


#line 636 "pango.override"
static PyObject *
_wrap_pango_attr_style_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "style", "start_index", "end_index", NULL };
    PyObject *py_style;
    PangoStyle style;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrStyle",
				     kwlist, &py_style, &start, &end))
	return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_STYLE, py_style, (gint *)&style))
	return NULL;

    return pypango_attr_new(pango_attr_style_new(style), start, end);
}
#line 7768 "pango.c"


#line 654 "pango.override"
static PyObject *
_wrap_pango_attr_weight_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "weight", "start_index", "end_index", NULL };
    PyObject *py_weight;
    PangoWeight weight;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrWeight",
				     kwlist, &py_weight, &start, &end))
	return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_WEIGHT, py_weight, (gint *)&weight))
	return NULL;

    return pypango_attr_new(pango_attr_weight_new(weight), start, end);
}
#line 7788 "pango.c"


#line 672 "pango.override"
static PyObject *
_wrap_pango_attr_variant_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "variant", "start_index", "end_index", NULL };
    PyObject *py_variant;
    PangoVariant variant;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrVariant",
				     kwlist, &py_variant, &start, &end))
	return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_VARIANT, py_variant, (gint *)&variant))
	return NULL;

    return pypango_attr_new(pango_attr_variant_new(variant), start, end);
}
#line 7808 "pango.c"


#line 690 "pango.override"
static PyObject *
_wrap_pango_attr_stretch_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "stretch", "start_index", "end_index", NULL };
    PyObject *py_stretch;
    PangoStretch stretch;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrStretch",
				     kwlist, &py_stretch, &start, &end))
	return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_STRETCH, py_stretch, (gint *)&stretch))
	return NULL;

    return pypango_attr_new(pango_attr_stretch_new(stretch), start, end);
}
#line 7828 "pango.c"


#line 708 "pango.override"
static PyObject *
_wrap_pango_attr_font_desc_new(PyObject *self, PyObject *args,PyObject *kwargs)
{
    static char *kwlist[] = { "desc", "start_index", "end_index", NULL };
    PyObject *font_desc;
    PangoFontDescription *desc;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrFontDesc",
				     kwlist, &font_desc, &start, &end))
	return NULL;
    if (!pyg_boxed_check(font_desc, PANGO_TYPE_FONT_DESCRIPTION)) {
	PyErr_SetString(PyExc_TypeError,"desc must be a PangoFontDescription");
	return NULL;
    }
    desc = pyg_boxed_get(font_desc, PangoFontDescription);
    return pypango_attr_new(pango_attr_font_desc_new(desc), start, end);
}
#line 7850 "pango.c"


#line 728 "pango.override"
static PyObject *
_wrap_pango_attr_underline_new(PyObject *self, PyObject *args,PyObject *kwargs)
{
    static char *kwlist[] = { "underline", "start_index", "end_index", NULL };
    PyObject *py_underline;
    PangoUnderline underline;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ii:PangoAttrUnderline",
				     kwlist, &py_underline, &start, &end))
	return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_UNDERLINE, py_underline,
			   (gint *)&underline))
	return NULL;

    return pypango_attr_new(pango_attr_underline_new(underline), start, end);
}
#line 7871 "pango.c"


#line 1948 "pango.override"
static PyObject *
_wrap_pango_attr_underline_color_new(PyObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "red", "green", "blue", "start_index",
			      "end_index", NULL };
    guint16 red, green, blue;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "HHH|ii:PangoAttrUnderlineColor",
				     kwlist, &red, &green, &blue,
				     &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_underline_color_new(red, green, blue),
			    start, end);
}
#line 7893 "pango.c"


#line 747 "pango.override"
static PyObject *
_wrap_pango_attr_strikethrough_new(PyObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "strikethrough", "start_index",
			      "end_index", NULL };
    gboolean strikethrough;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "i|ii:PangoAttrStrikethrough",
				     kwlist, &strikethrough, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_strikethrough_new(strikethrough),
			    start, end);
}
#line 7914 "pango.c"


#line 1968 "pango.override"
static PyObject *
_wrap_pango_attr_strikethrough_color_new(PyObject *self, PyObject *args,
                                         PyObject *kwargs)
{
    static char *kwlist[] = { "red", "green", "blue", "start_index",
			      "end_index", NULL };
    guint16 red, green, blue;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "HHH|ii:PangoAttrStrikethroughColor",
				     kwlist, &red, &green, &blue,
				     &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_strikethrough_color_new(red, green, blue),
			    start, end);
}
#line 7936 "pango.c"


#line 766 "pango.override"
static PyObject *
_wrap_pango_attr_rise_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rise", "start_index", "end_index", NULL };
    gint rise;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|ii:PangoAttrRise",
				     kwlist, &rise, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_rise_new(rise), start, end);
}
#line 7953 "pango.c"


#line 821 "pango.override"
static PyObject *
_wrap_pango_attr_scale_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "scale", "start_index", "end_index", NULL };
    double scale;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d|ii:PangoAttrScale",
				     kwlist, &scale, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_scale_new(scale), start, end);
}
#line 7970 "pango.c"


#line 1542 "pango.override"
static PyObject *
_wrap_pango_attr_fallback_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fallback", "start_index", "end_index", NULL };
    gboolean fallback;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i|ii:PangoAttrFallback",
				     kwlist, &fallback, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_fallback_new(fallback),
			    start, end);
}
#line 7988 "pango.c"


#line 2005 "pango.override"
static PyObject *
_wrap_pango_attr_letter_spacing_new(PyObject *self, PyObject *args,
                                    PyObject *kwargs)
{
    static char *kwlist[] = { "letter_spacing", "start_index", "end_index",
                              NULL };
    int spacing;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i|ii:PangoAttrLetterSpacing",
				     kwlist, &spacing, &start, &end))
	return NULL;

    return pypango_attr_new(pango_attr_letter_spacing_new(spacing),
                            start, end);
}
#line 8009 "pango.c"


#line 781 "pango.override"
static PyObject *
_wrap_pango_attr_shape_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ink_rect", "logical_rect", "start_index",
			      "end_index", NULL };
    PangoRectangle ink_rect, logical_rect;
    PyObject *py_ink_rect, *py_logical_rect;
    guint start = 0, end = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "OO|ii:PangoAttrShape", kwlist,
				     &py_ink_rect, &py_logical_rect,
				     &start, &end))
	return NULL;

    if (!PyTuple_Check(py_ink_rect)
	|| !PyArg_ParseTuple(py_ink_rect, "iiii",
			     &ink_rect.x, &ink_rect.y,
			     &ink_rect.width, &ink_rect.height)) {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError,
			"ink_rect must be a 4-tuple of integers");
	return NULL;
    }

    if (!PyTuple_Check(py_logical_rect)
	|| !PyArg_ParseTuple(py_logical_rect, "iiii",
			     &logical_rect.x, &logical_rect.y,
			     &logical_rect.width,&logical_rect.height)) {
	PyErr_Clear();
	PyErr_SetString(PyExc_TypeError,
			"logical_rect must be a 4-tuple of integers");
	return NULL;
    }

    return pypango_attr_new(pango_attr_shape_new(&ink_rect, &logical_rect),
			    start, end);
}
#line 8051 "pango.c"


#line 1317 "pango.override"
static PyObject *
_wrap_pango_parse_markup(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "markup_text", "accel_marker", NULL };
    char *markup_text, *text = NULL;
    Py_ssize_t length;
    Py_UNICODE *py_accel_marker = NULL, py_accel_char;
    Py_ssize_t py_accel_marker_len;
    gunichar accel_marker, accel_char = 0;
    PangoAttrList *attr_list = NULL;
    GError *error = NULL;
    gboolean ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s#|u#:pango.parse_markup",
				     kwlist, &markup_text, &length,
				     &py_accel_marker, &py_accel_marker_len))
	return NULL;
    if (py_accel_marker != NULL) {
	if (py_accel_marker_len != 1) {
	    PyErr_SetString(PyExc_TypeError, "accel_mark must be one character");
	    return NULL;
	}
	accel_marker = py_accel_marker[0];
    } else
	accel_marker = 0;

    ret = pango_parse_markup(markup_text, length, accel_marker,
			     &attr_list, &text, &accel_char, &error);
    if (pyg_error_check(&error))
	return NULL;

#if !defined(Py_UNICODE_SIZE) || Py_UNICODE_SIZE == 2
    if (accel_char >= 0xffff) {
	PyErr_SetString(PyExc_ValueError, "unicode character is too big to fit in a 16-bit unicode character");
	return NULL;
    }
#endif
    py_accel_char = (Py_UNICODE)accel_char;

    py_ret = Py_BuildValue("(Nsu#)", pyg_boxed_new(PANGO_TYPE_ATTR_LIST,
						   attr_list, FALSE, TRUE),
			   text, &py_accel_char, (Py_ssize_t) 1);
    g_free(text);
    return py_ret;
}
#line 8101 "pango.c"


static PyObject *
_wrap_pango_gravity_to_rotation(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gravity", NULL };
    PyObject *py_gravity = NULL;
    double ret;
    PangoGravity gravity;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gravity_to_rotation", kwlist, &py_gravity))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY, py_gravity, (gpointer)&gravity))
        return NULL;
    
    ret = pango_gravity_to_rotation(gravity);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_pango_gravity_get_for_matrix(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    gint ret;
    PangoMatrix *matrix = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gravity_get_for_matrix", kwlist, &py_matrix))
        return NULL;
    if (pyg_boxed_check(py_matrix, PANGO_TYPE_MATRIX))
        matrix = pyg_boxed_get(py_matrix, PangoMatrix);
    else {
        PyErr_SetString(PyExc_TypeError, "matrix should be a PangoMatrix");
        return NULL;
    }
    
    ret = pango_gravity_get_for_matrix(matrix);
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY, ret);
}

static PyObject *
_wrap_pango_gravity_get_for_script(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "script", "base_gravity", "hint", NULL };
    PyObject *py_script = NULL, *py_base_gravity = NULL, *py_hint = NULL;
    PangoGravityHint hint;
    PangoGravity base_gravity;
    PangoScript script;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOO:gravity_get_for_script", kwlist, &py_script, &py_base_gravity, &py_hint))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_SCRIPT, py_script, (gpointer)&script))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY, py_base_gravity, (gpointer)&base_gravity))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_GRAVITY_HINT, py_hint, (gpointer)&hint))
        return NULL;
    
    ret = pango_gravity_get_for_script(script, base_gravity, hint);
    
    return pyg_enum_from_gtype(PANGO_TYPE_GRAVITY, ret);
}

static PyObject *
_wrap_pango_script_for_unichar(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ch", NULL };
    gunichar ch;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O&:script_for_unichar", kwlist, pyg_pyobj_to_unichar_conv, &ch))
        return NULL;
    
    ret = pango_script_for_unichar(ch);
    
    return pyg_enum_from_gtype(PANGO_TYPE_SCRIPT, ret);
}

static PyObject *
_wrap_pango_script_get_sample_language(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "script", NULL };
    PyObject *py_script = NULL;
    PangoLanguage *ret;
    PangoScript script;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:get_sample_language", kwlist, &py_script))
        return NULL;
    if (pyg_enum_get_value(PANGO_TYPE_SCRIPT, py_script, (gpointer)&script))
        return NULL;
    
    ret = pango_script_get_sample_language(script);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LANGUAGE, ret, TRUE, TRUE);
}

#line 1716 "pango.override"
static PyObject *
_wrap_pango_language_from_string1(PyGObject *self, PyObject *args,
			       PyObject *kwargs)
{
    static char *kwlist[] = { "language", NULL };
    char *language;
    PangoLanguage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s:pango_language_from_string",
				     kwlist, &language))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning,
		   "use pango.Language instead") < 0)
        return NULL;

    ret = pango_language_from_string(language);
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(PANGO_TYPE_LANGUAGE, ret, TRUE, TRUE);
}

#line 8224 "pango.c"


#line 1739 "pango.override"
static PyObject *
_wrap_pango_language_matches1(PyGObject *self, PyObject *args,
			       PyObject *kwargs)
{
    static char *kwlist[] = { "language", "range_list", NULL };
    PyObject *py_language = Py_None;
    char *range_list;
    PangoLanguage *language = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "Os:pango_language_matches",
				     kwlist, &py_language, &range_list))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning,
		   "use pango.Language.matches instead") < 0)
        return NULL;

    if (pyg_boxed_check(py_language, PANGO_TYPE_LANGUAGE))
        language = pyg_boxed_get(py_language, PangoLanguage);
    else if (py_language != Py_None) {
        PyErr_SetString(PyExc_TypeError,
			"language should be a PangoLanguage or None");
        return NULL;
    }

    return PyBool_FromLong(pango_language_matches(language, range_list));
}
#line 8255 "pango.c"


static PyObject *
_wrap_pango_unichar_direction(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ch", NULL };
    gunichar ch;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O&:unichar_direction", kwlist, pyg_pyobj_to_unichar_conv, &ch))
        return NULL;
    
    ret = pango_unichar_direction(ch);
    
    return pyg_enum_from_gtype(PANGO_TYPE_DIRECTION, ret);
}

static PyObject *
_wrap_pango_find_base_dir(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "text", "length", NULL };
    char *text;
    int length;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:find_base_dir", kwlist, &text, &length))
        return NULL;
    
    ret = pango_find_base_dir(text, length);
    
    return pyg_enum_from_gtype(PANGO_TYPE_DIRECTION, ret);
}

static PyObject *
_wrap_pango_units_from_double(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "d", NULL };
    int ret;
    double d;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:units_from_double", kwlist, &d))
        return NULL;
    
    ret = pango_units_from_double(d);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_units_to_double(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    int i;
    double ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:units_to_double", kwlist, &i))
        return NULL;
    
    ret = pango_units_to_double(i);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_PANGO_PIXELS(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    int size, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:PIXELS", kwlist, &size))
        return NULL;
    
    ret = PANGO_PIXELS(size);
    
    return PyInt_FromLong(ret);
}

#line 1458 "pango.override"
static PyObject *
_wrap_PANGO_ASCENT(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    int ret;
    PangoRectangle rect;
    PyObject *py_rect;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!:ASCENT",
				     kwlist, &PyTuple_Type, &py_rect)
	&& PyArg_ParseTuple(py_rect, "iiii:ASCENT", &rect.x, &rect.y,
			     &rect.width, &rect.height)) {
	ret = PANGO_ASCENT(rect);
	return PyInt_FromLong(ret);
    }
    PyErr_Clear();
    PyErr_SetString(PyExc_ValueError, "rect must be a 4-tuple of integers");
    return NULL;
}
#line 8353 "pango.c"


#line 1479 "pango.override"
static PyObject *
_wrap_PANGO_DESCENT(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    int ret;
    PangoRectangle rect;
    PyObject *py_rect;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!:DESCENT",
				     kwlist, &PyTuple_Type, &py_rect)
	&& PyArg_ParseTuple(py_rect, "iiii:DESCENT", &rect.x, &rect.y,
			     &rect.width, &rect.height)) {
	ret = PANGO_DESCENT(rect);
	return PyInt_FromLong(ret);
    }
    PyErr_Clear();
    PyErr_SetString(PyExc_ValueError, "rect must be a 4-tuple of integers");
    return NULL;
}
#line 8376 "pango.c"


#line 1500 "pango.override"
static PyObject *
_wrap_PANGO_LBEARING(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    int ret;
    PangoRectangle rect;
    PyObject *py_rect;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!:LBEARING",
				     kwlist, &PyTuple_Type, &py_rect)
	&& PyArg_ParseTuple(py_rect, "iiii:LBEARING", &rect.x, &rect.y,
			     &rect.width, &rect.height)) {
	ret = PANGO_LBEARING(rect);
	return PyInt_FromLong(ret);
    }
    PyErr_Clear();
    PyErr_SetString(PyExc_ValueError, "rect must be a 4-tuple of integers");
    return NULL;
}
#line 8399 "pango.c"


#line 1521 "pango.override"
static PyObject *
_wrap_PANGO_RBEARING(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rect", NULL };
    int ret;
    PangoRectangle rect;
    PyObject *py_rect;

    if (PyArg_ParseTupleAndKeywords(args, kwargs, "O!:RBEARING",
				     kwlist, &PyTuple_Type, &py_rect)
	&& PyArg_ParseTuple(py_rect, "iiii:RBEARING", &rect.x, &rect.y,
			     &rect.width, &rect.height)) {
	ret = PANGO_RBEARING(rect);
	return PyInt_FromLong(ret);
    }
    PyErr_Clear();
    PyErr_SetString(PyExc_ValueError, "rect must be a 4-tuple of integers");
    return NULL;
}
#line 8422 "pango.c"


static PyObject *
_wrap_pango_version(PyObject *self)
{
    int ret;

    
    ret = pango_version();
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_pango_version_string(PyObject *self)
{
    const gchar *ret;

    
    ret = pango_version_string();
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_version_check(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "required_major", "required_minor", "required_micro", NULL };
    int required_major, required_minor, required_micro;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iii:version_check", kwlist, &required_major, &required_minor, &required_micro))
        return NULL;
    
    ret = pango_version_check(required_major, required_minor, required_micro);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

const PyMethodDef pypango_functions[] = {
    { "pango_attr_type_register", (PyCFunction)_wrap_pango_attr_type_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrLanguage", (PyCFunction)_wrap_pango_attr_language_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrFamily", (PyCFunction)_wrap_pango_attr_family_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrForeground", (PyCFunction)_wrap_pango_attr_foreground_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrBackground", (PyCFunction)_wrap_pango_attr_background_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrSize", (PyCFunction)_wrap_pango_attr_size_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrSizeAbsolute", (PyCFunction)_wrap_pango_attr_size_new_absolute, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrStyle", (PyCFunction)_wrap_pango_attr_style_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrWeight", (PyCFunction)_wrap_pango_attr_weight_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrVariant", (PyCFunction)_wrap_pango_attr_variant_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrStretch", (PyCFunction)_wrap_pango_attr_stretch_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrFontDesc", (PyCFunction)_wrap_pango_attr_font_desc_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrUnderline", (PyCFunction)_wrap_pango_attr_underline_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrUnderlineColor", (PyCFunction)_wrap_pango_attr_underline_color_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrStrikethrough", (PyCFunction)_wrap_pango_attr_strikethrough_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrStrikethroughColor", (PyCFunction)_wrap_pango_attr_strikethrough_color_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrRise", (PyCFunction)_wrap_pango_attr_rise_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrScale", (PyCFunction)_wrap_pango_attr_scale_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrFallback", (PyCFunction)_wrap_pango_attr_fallback_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrLetterSpacing", (PyCFunction)_wrap_pango_attr_letter_spacing_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "AttrShape", (PyCFunction)_wrap_pango_attr_shape_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_markup", (PyCFunction)_wrap_pango_parse_markup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gravity_to_rotation", (PyCFunction)_wrap_pango_gravity_to_rotation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gravity_get_for_matrix", (PyCFunction)_wrap_pango_gravity_get_for_matrix, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gravity_get_for_script", (PyCFunction)_wrap_pango_gravity_get_for_script, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "script_for_unichar", (PyCFunction)_wrap_pango_script_for_unichar, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_sample_language", (PyCFunction)_wrap_pango_script_get_sample_language, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pango_language_from_string", (PyCFunction)_wrap_pango_language_from_string1, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pango_language_matches", (PyCFunction)_wrap_pango_language_matches1, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unichar_direction", (PyCFunction)_wrap_pango_unichar_direction, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_base_dir", (PyCFunction)_wrap_pango_find_base_dir, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "units_from_double", (PyCFunction)_wrap_pango_units_from_double, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "units_to_double", (PyCFunction)_wrap_pango_units_to_double, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "PIXELS", (PyCFunction)_wrap_PANGO_PIXELS, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ASCENT", (PyCFunction)_wrap_PANGO_ASCENT, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "DESCENT", (PyCFunction)_wrap_PANGO_DESCENT, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "LBEARING", (PyCFunction)_wrap_PANGO_LBEARING, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "RBEARING", (PyCFunction)_wrap_PANGO_RBEARING, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "version", (PyCFunction)_wrap_pango_version, METH_NOARGS,
      NULL },
    { "version_string", (PyCFunction)_wrap_pango_version_string, METH_NOARGS,
      NULL },
    { "version_check", (PyCFunction)_wrap_pango_version_check, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pypango_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_enum_add(module, "Alignment", strip_prefix, PANGO_TYPE_ALIGNMENT);
  pyg_enum_add(module, "AttrType", strip_prefix, PANGO_TYPE_ATTR_TYPE);
  pyg_enum_add(module, "CoverageLevel", strip_prefix, PANGO_TYPE_COVERAGE_LEVEL);
  pyg_enum_add(module, "Direction", strip_prefix, PANGO_TYPE_DIRECTION);
  pyg_enum_add(module, "EllipsizeMode", strip_prefix, PANGO_TYPE_ELLIPSIZE_MODE);
  pyg_enum_add(module, "Gravity", strip_prefix, PANGO_TYPE_GRAVITY);
  pyg_enum_add(module, "GravityHint", strip_prefix, PANGO_TYPE_GRAVITY_HINT);
  pyg_enum_add(module, "RenderPart", strip_prefix, PANGO_TYPE_RENDER_PART);
  pyg_enum_add(module, "Script", strip_prefix, PANGO_TYPE_SCRIPT);
  pyg_enum_add(module, "Stretch", strip_prefix, PANGO_TYPE_STRETCH);
  pyg_enum_add(module, "Style", strip_prefix, PANGO_TYPE_STYLE);
  pyg_enum_add(module, "TabAlign", strip_prefix, PANGO_TYPE_TAB_ALIGN);
  pyg_enum_add(module, "Underline", strip_prefix, PANGO_TYPE_UNDERLINE);
  pyg_enum_add(module, "Variant", strip_prefix, PANGO_TYPE_VARIANT);
  pyg_enum_add(module, "Weight", strip_prefix, PANGO_TYPE_WEIGHT);
  pyg_enum_add(module, "WrapMode", strip_prefix, PANGO_TYPE_WRAP_MODE);
  pyg_flags_add(module, "FontMask", strip_prefix, PANGO_TYPE_FONT_MASK);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pypango_register_classes(PyObject *d)
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


#line 511 "pango.override"
    PyPangoAttribute_Type.tp_alloc = PyType_GenericAlloc;
    PyPangoAttribute_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPangoAttribute_Type) < 0)
        return;

    PyPangoAttrIterator_Type.tp_alloc = PyType_GenericAlloc;
    PyPangoAttrIterator_Type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyPangoAttrIterator_Type) < 0)
        return;

#line 8616 "pango.c"
    pyg_register_boxed(d, "AttrList", PANGO_TYPE_ATTR_LIST, &PyPangoAttrList_Type);
    pyg_register_boxed(d, "Color", PANGO_TYPE_COLOR, &PyPangoColor_Type);
    pyg_register_boxed(d, "FontDescription", PANGO_TYPE_FONT_DESCRIPTION, &PyPangoFontDescription_Type);
    pyg_register_boxed(d, "FontMetrics", PANGO_TYPE_FONT_METRICS, &PyPangoFontMetrics_Type);
    pyg_register_boxed(d, "GlyphString", PANGO_TYPE_GLYPH_STRING, &PyPangoGlyphString_Type);
    pyg_register_boxed(d, "Item", PANGO_TYPE_ITEM, &PyPangoItem_Type);
    pyg_register_boxed(d, "Language", PANGO_TYPE_LANGUAGE, &PyPangoLanguage_Type);
    pyg_register_boxed(d, "LayoutIter", PANGO_TYPE_LAYOUT_ITER, &PyPangoLayoutIter_Type);
    pyg_register_boxed(d, "LayoutLine", PANGO_TYPE_LAYOUT_LINE, &PyPangoLayoutLine_Type);
    pyg_register_boxed(d, "Matrix", PANGO_TYPE_MATRIX, &PyPangoMatrix_Type);
    pyg_register_boxed(d, "TabArray", PANGO_TYPE_TAB_ARRAY, &PyPangoTabArray_Type);
    pygobject_register_class(d, "PangoContext", PANGO_TYPE_CONTEXT, &PyPangoContext_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_CONTEXT);
    pygobject_register_class(d, "PangoEngine", PANGO_TYPE_ENGINE, &PyPangoEngine_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_ENGINE);
    pygobject_register_class(d, "PangoEngineLang", PANGO_TYPE_ENGINE_LANG, &PyPangoEngineLang_Type, Py_BuildValue("(O)", &PyPangoEngine_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_ENGINE_LANG);
    pygobject_register_class(d, "PangoEngineShape", PANGO_TYPE_ENGINE_SHAPE, &PyPangoEngineShape_Type, Py_BuildValue("(O)", &PyPangoEngine_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_ENGINE_SHAPE);
    pygobject_register_class(d, "PangoFont", PANGO_TYPE_FONT, &PyPangoFont_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_FONT);
    pyg_register_class_init(PANGO_TYPE_FONT, __PangoFont_class_init);
    pygobject_register_class(d, "PangoFontFace", PANGO_TYPE_FONT_FACE, &PyPangoFontFace_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_FONT_FACE);
    pyg_register_class_init(PANGO_TYPE_FONT_FACE, __PangoFontFace_class_init);
    pygobject_register_class(d, "PangoFontFamily", PANGO_TYPE_FONT_FAMILY, &PyPangoFontFamily_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_FONT_FAMILY);
    pyg_register_class_init(PANGO_TYPE_FONT_FAMILY, __PangoFontFamily_class_init);
    pygobject_register_class(d, "PangoFontMap", PANGO_TYPE_FONT_MAP, &PyPangoFontMap_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_FONT_MAP);
    pyg_register_class_init(PANGO_TYPE_FONT_MAP, __PangoFontMap_class_init);
    pygobject_register_class(d, "PangoFontset", PANGO_TYPE_FONTSET, &PyPangoFontset_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_FONTSET);
    pyg_register_class_init(PANGO_TYPE_FONTSET, __PangoFontset_class_init);
    pygobject_register_class(d, "PangoFontsetSimple", PANGO_TYPE_FONTSET_SIMPLE, &PyPangoFontsetSimple_Type, Py_BuildValue("(O)", &PyPangoFontset_Type));
    pygobject_register_class(d, "PangoLayout", PANGO_TYPE_LAYOUT, &PyPangoLayout_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "PangoRenderer", PANGO_TYPE_RENDERER, &PyPangoRenderer_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(PANGO_TYPE_RENDERER);
    pyg_register_class_init(PANGO_TYPE_RENDERER, __PangoRenderer_class_init);
}
