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


#line 4 "pangocairo.override"
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <pango/pangocairo.h>
#include <pycairo.h>


extern Pycairo_CAPI_t *Pycairo_CAPI;

GType pypango_layout_line_type; /*  See bug 305975 */

#ifndef PANGO_TYPE_LAYOUT_LINE
# define PANGO_TYPE_LAYOUT_LINE pypango_layout_line_type
#endif

#line 35 "pangocairo.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyPangoFontMap_Type;
#define PyPangoFontMap_Type (*_PyPangoFontMap_Type)
static PyTypeObject *_PyPangoContext_Type;
#define PyPangoContext_Type (*_PyPangoContext_Type)
static PyTypeObject *_PyPangoLayout_Type;
#define PyPangoLayout_Type (*_PyPangoLayout_Type)
static PyTypeObject *_PyPangoFont_Type;
#define PyPangoFont_Type (*_PyPangoFont_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyPangoCairoFontMap_Type;


#line 88 "pangocairo.override"

static PyObject *
pypango_cairo_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyObject *o;
    PycairoContext *ctx;

    if (!PyArg_ParseTuple(args, "O!:CairoContext.__new__", 
			  &PycairoContext_Type, &ctx))
	return NULL;

    cairo_reference(ctx->ctx);
    o = PycairoContext_FromContext(ctx->ctx, type, NULL);
    return o;
}

static PyObject *
_wrap_pango_cairo_update_context(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", NULL };
    PyGObject *context;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:CairoContext.update_context", kwlist,
                                     &PyPangoContext_Type, &context))
        return NULL;
    pango_cairo_update_context(PycairoContext_GET(self), PANGO_CONTEXT(context->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_create_layout(PyGObject *self)
{
    PangoLayout *ret;

    ret = pango_cairo_create_layout(PycairoContext_GET(self));
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_cairo_update_layout(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layout", NULL };
    PyGObject *layout;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:CairoContext.update_layout",
                                     kwlist, &PyPangoLayout_Type, &layout))
        return NULL;
    pango_cairo_update_layout(PycairoContext_GET(self), PANGO_LAYOUT(layout->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_show_glyph_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", "glyphs", NULL };
    PyGObject *font;
    PangoGlyphString *glyphs = NULL;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O:CairoContext.show_glyph_string",
                                     kwlist, &PyPangoFont_Type, &font, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    pango_cairo_show_glyph_string(PycairoContext_GET(self), PANGO_FONT(font->obj), glyphs);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_show_layout_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line", NULL };
    PangoLayoutLine *line = NULL;
    PyObject *py_line;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CairoContext.show_layout_line",
                                     kwlist, &py_line))
        return NULL;
    if (pyg_boxed_check(py_line, PANGO_TYPE_LAYOUT_LINE))
        line = pyg_boxed_get(py_line, PangoLayoutLine);
    else {
        PyErr_SetString(PyExc_TypeError, "line should be a PangoLayoutLine");
        return NULL;
    }
    pango_cairo_show_layout_line(PycairoContext_GET(self), line);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_show_layout(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layout", NULL };
    PyGObject *layout;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:CairoContext.show_layout",
                                     kwlist, &PyPangoLayout_Type, &layout))
        return NULL;
    pango_cairo_show_layout(PycairoContext_GET(self), PANGO_LAYOUT(layout->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_glyph_string_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "font", "glyphs", NULL };
    PyGObject *font;
    PangoGlyphString *glyphs = NULL;
    PyObject *py_glyphs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O:CairoContext.glyph_string_path",
                                     kwlist, &PyPangoFont_Type, &font, &py_glyphs))
        return NULL;
    if (pyg_boxed_check(py_glyphs, PANGO_TYPE_GLYPH_STRING))
        glyphs = pyg_boxed_get(py_glyphs, PangoGlyphString);
    else {
        PyErr_SetString(PyExc_TypeError, "glyphs should be a PangoGlyphString");
        return NULL;
    }
    pango_cairo_glyph_string_path(PycairoContext_GET(self), PANGO_FONT(font->obj), glyphs);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_layout_line_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "line", NULL };
    PangoLayoutLine *line = NULL;
    PyObject *py_line;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:CairoContext.layout_line_path",
                                     kwlist, &py_line))
        return NULL;
    if (pyg_boxed_check(py_line, PANGO_TYPE_LAYOUT_LINE))
        line = pyg_boxed_get(py_line, PangoLayoutLine);
    else {
        PyErr_SetString(PyExc_TypeError, "line should be a PangoLayoutLine");
        return NULL;
    }
    pango_cairo_layout_line_path(PycairoContext_GET(self), line);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_layout_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "layout", NULL };
    PyGObject *layout;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:CairoContext.layout_path",
                                     kwlist, &PyPangoLayout_Type, &layout))
        return NULL;
    pango_cairo_layout_path(PycairoContext_GET(self), PANGO_LAYOUT(layout->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _PyCairoContext_methods[] = {
    { "update_context", (PyCFunction)_wrap_pango_cairo_update_context, METH_VARARGS|METH_KEYWORDS },
    { "create_layout", (PyCFunction)_wrap_pango_cairo_create_layout, METH_NOARGS },
    { "update_layout", (PyCFunction)_wrap_pango_cairo_update_layout, METH_VARARGS|METH_KEYWORDS },
    { "show_glyph_string", (PyCFunction)_wrap_pango_cairo_show_glyph_string, METH_VARARGS|METH_KEYWORDS },
    { "show_layout_line", (PyCFunction)_wrap_pango_cairo_show_layout_line, METH_VARARGS|METH_KEYWORDS },
    { "show_layout", (PyCFunction)_wrap_pango_cairo_show_layout, METH_VARARGS|METH_KEYWORDS },
    { "glyph_string_path", (PyCFunction)_wrap_pango_cairo_glyph_string_path, METH_VARARGS|METH_KEYWORDS },
    { "layout_line_path", (PyCFunction)_wrap_pango_cairo_layout_line_path, METH_VARARGS|METH_KEYWORDS },
    { "layout_path", (PyCFunction)_wrap_pango_cairo_layout_path, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};


PyTypeObject PyPangoCairoContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "pangocairo.CairoContext",          /* tp_name */
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
    "A cairo.Context enhanced with some additional pango methods", /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    _PyCairoContext_methods,            /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,          		/* tp_init */
    0,                			/* tp_alloc */
    pypango_cairo_new,         		/* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};

#line 282 "pangocairo.c"



/* ----------- PangoCairoFontMap ----------- */

static PyObject *
_wrap_pango_cairo_font_map_set_resolution(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dpi", NULL };
    double dpi;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:Pango.CairoFontMap.set_resolution", kwlist, &dpi))
        return NULL;
    
    pango_cairo_font_map_set_resolution(PANGO_CAIRO_FONT_MAP(self->obj), dpi);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_font_map_get_resolution(PyGObject *self)
{
    double ret;

    
    ret = pango_cairo_font_map_get_resolution(PANGO_CAIRO_FONT_MAP(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_pango_cairo_font_map_create_context(PyGObject *self)
{
    PangoContext *ret;
    PyObject *py_ret;

    
    ret = pango_cairo_font_map_create_context(PANGO_CAIRO_FONT_MAP(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static const PyMethodDef _PyPangoCairoFontMap_methods[] = {
    { "set_resolution", (PyCFunction)_wrap_pango_cairo_font_map_set_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_resolution", (PyCFunction)_wrap_pango_cairo_font_map_get_resolution, METH_NOARGS,
      NULL },
    { "create_context", (PyCFunction)_wrap_pango_cairo_font_map_create_context, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyPangoCairoFontMap_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "CairoFontMap",                   /* tp_name */
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
    (struct PyMethodDef*)_PyPangoCairoFontMap_methods, /* tp_methods */
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
_wrap_pango_cairo_font_map_new(PyObject *self)
{
    PangoFontMap *ret;

    
    ret = pango_cairo_font_map_new();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_cairo_font_map_get_default(PyObject *self)
{
    PangoFontMap *ret;

    
    ret = pango_cairo_font_map_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_pango_cairo_context_set_resolution(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", "dpi", NULL };
    PyGObject *context;
    double dpi;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!d:context_set_resolution", kwlist, &PyPangoContext_Type, &context, &dpi))
        return NULL;
    
    pango_cairo_context_set_resolution(PANGO_CONTEXT(context->obj), dpi);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_context_get_resolution(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", NULL };
    PyGObject *context;
    double ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:context_get_resolution", kwlist, &PyPangoContext_Type, &context))
        return NULL;
    
    ret = pango_cairo_context_get_resolution(PANGO_CONTEXT(context->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_pango_cairo_error_underline_path(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "x", "y", "width", "height", NULL };
    double x, y, width, height;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!dddd:error_underline_path", kwlist, &PycairoContext_Type, &cr, &x, &y, &width, &height))
        return NULL;
    
    pango_cairo_error_underline_path(cr->ctx, x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_pango_cairo_show_error_underline(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "x", "y", "width", "height", NULL };
    double x, y, width, height;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!dddd:show_error_underline", kwlist, &PycairoContext_Type, &cr, &x, &y, &width, &height))
        return NULL;
    
    pango_cairo_show_error_underline(cr->ctx, x, y, width, height);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 60 "pangocairo.override"
static PyObject *
_wrap_context_set_font_options(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", "font_options", NULL };
    PyGObject *context;
    PyGObject *py_options;
    const cairo_font_options_t *options;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O:pangocairo.context_set_font_options",
                                     kwlist, &PyPangoContext_Type, &context,
				     &py_options))
        return NULL;
    if ((PyObject*)py_options == Py_None)
        options = NULL;
    else if (pygobject_check(py_options, &PycairoFontOptions_Type))
        options = ((PycairoFontOptions *)py_options)->font_options;
    else {
        PyErr_SetString(PyExc_TypeError, "font_options must be a cairo.FontOptions or None");
        return NULL;
    }
    pango_cairo_context_set_font_options(PANGO_CONTEXT(context->obj), options);
    Py_INCREF(Py_None);
    return Py_None;
}


#line 502 "pangocairo.c"


#line 40 "pangocairo.override"
static PyObject *
_wrap_context_get_font_options(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "context", NULL };
    PyGObject *context;
    const cairo_font_options_t *font_options;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:pangocairo.context_get_font_options",
                                     kwlist, &PyPangoContext_Type, &context))
        return NULL;
    font_options = pango_cairo_context_get_font_options(PANGO_CONTEXT(context->obj));
    if (!font_options) {
	Py_INCREF(Py_None);
	return Py_None;
    }
    return PycairoFontOptions_FromFontOptions(cairo_font_options_copy(font_options));
}

#line 524 "pangocairo.c"


const PyMethodDef pypangocairo_functions[] = {
    { "cairo_font_map_new", (PyCFunction)_wrap_pango_cairo_font_map_new, METH_NOARGS,
      NULL },
    { "cairo_font_map_get_default", (PyCFunction)_wrap_pango_cairo_font_map_get_default, METH_NOARGS,
      NULL },
    { "context_set_resolution", (PyCFunction)_wrap_pango_cairo_context_set_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "context_get_resolution", (PyCFunction)_wrap_pango_cairo_context_get_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "error_underline_path", (PyCFunction)_wrap_pango_cairo_error_underline_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "show_error_underline", (PyCFunction)_wrap_pango_cairo_show_error_underline, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "context_set_font_options", (PyCFunction)_wrap_context_set_font_options, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "context_get_font_options", (PyCFunction)_wrap_context_get_font_options, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

/* initialise stuff extension classes */
void
pypangocairo_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("pango")) != NULL) {
        _PyPangoFontMap_Type = (PyTypeObject *)PyObject_GetAttrString(module, "FontMap");
        if (_PyPangoFontMap_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name FontMap from pango");
            return ;
        }
        _PyPangoContext_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Context");
        if (_PyPangoContext_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Context from pango");
            return ;
        }
        _PyPangoLayout_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Layout");
        if (_PyPangoLayout_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Layout from pango");
            return ;
        }
        _PyPangoFont_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Font");
        if (_PyPangoFont_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Font from pango");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import pango");
        return ;
    }


#line 585 "pangocairo.c"
    pyg_register_interface(d, "CairoFontMap", PANGO_TYPE_CAIRO_FONT_MAP, &PyPangoCairoFontMap_Type);
}
