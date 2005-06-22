/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 3 "/home/muntyan/projects/moo/moo/mooedit/gtksourceview/gtksourceview-pygtk.override"
#include <Python.h>
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include "gtksourceview/gtksourceview.h"
#line 13 "/home/muntyan/projects/moo/moo/mooedit/gtksourceview/gtksourceview-pygtk.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGdkPixbuf_Type;
#define PyGdkPixbuf_Type (*_PyGdkPixbuf_Type)
static PyTypeObject *_PyGtkTextView_Type;
#define PyGtkTextView_Type (*_PyGtkTextView_Type)
static PyTypeObject *_PyGtkTextBuffer_Type;
#define PyGtkTextBuffer_Type (*_PyGtkTextBuffer_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGtkSourceBuffer_Type;
PyTypeObject PyGtkSourceView_Type;


/* ----------- GtkSourceBuffer ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

static PyObject *
_wrap_gtk_source_buffer_get_check_brackets(PyGObject *self)
{
    int ret;

    ret = gtk_source_buffer_get_check_brackets(GTK_SOURCE_BUFFER(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_buffer_set_check_brackets(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "check_brackets", NULL };
    int check_brackets;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceBuffer.set_check_brackets", kwlist, &check_brackets))
        return NULL;
    gtk_source_buffer_set_check_brackets(GTK_SOURCE_BUFFER(self->obj), check_brackets);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_get_highlight(PyGObject *self)
{
    int ret;

    ret = gtk_source_buffer_get_highlight(GTK_SOURCE_BUFFER(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_buffer_set_highlight(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "highlight", NULL };
    int highlight;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceBuffer.set_highlight", kwlist, &highlight))
        return NULL;
    gtk_source_buffer_set_highlight(GTK_SOURCE_BUFFER(self->obj), highlight);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_get_max_undo_levels(PyGObject *self)
{
    int ret;

    ret = gtk_source_buffer_get_max_undo_levels(GTK_SOURCE_BUFFER(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gtk_source_buffer_set_max_undo_levels(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "max_undo_levels", NULL };
    int max_undo_levels;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceBuffer.set_max_undo_levels", kwlist, &max_undo_levels))
        return NULL;
    gtk_source_buffer_set_max_undo_levels(GTK_SOURCE_BUFFER(self->obj), max_undo_levels);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_get_escape_char(PyGObject *self)
{
    gunichar ret;
    Py_UNICODE py_ret;

    ret = gtk_source_buffer_get_escape_char(GTK_SOURCE_BUFFER(self->obj));
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
_wrap_gtk_source_buffer_set_escape_char(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "escape_char", NULL };
    gunichar escape_char;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:GtkSourceBuffer.set_escape_char", kwlist, pyg_pyobj_to_unichar_conv, &escape_char))
        return NULL;
    gtk_source_buffer_set_escape_char(GTK_SOURCE_BUFFER(self->obj), escape_char);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_can_undo(PyGObject *self)
{
    int ret;

    ret = gtk_source_buffer_can_undo(GTK_SOURCE_BUFFER(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_buffer_can_redo(PyGObject *self)
{
    int ret;

    ret = gtk_source_buffer_can_redo(GTK_SOURCE_BUFFER(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_buffer_undo(PyGObject *self)
{
    gtk_source_buffer_undo(GTK_SOURCE_BUFFER(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_redo(PyGObject *self)
{
    gtk_source_buffer_redo(GTK_SOURCE_BUFFER(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_begin_not_undoable_action(PyGObject *self)
{
    gtk_source_buffer_begin_not_undoable_action(GTK_SOURCE_BUFFER(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_buffer_end_not_undoable_action(PyGObject *self)
{
    gtk_source_buffer_end_not_undoable_action(GTK_SOURCE_BUFFER(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _PyGtkSourceBuffer_methods[] = {
    { "get_check_brackets", (PyCFunction)_wrap_gtk_source_buffer_get_check_brackets, METH_NOARGS },
    { "set_check_brackets", (PyCFunction)_wrap_gtk_source_buffer_set_check_brackets, METH_VARARGS|METH_KEYWORDS },
    { "get_highlight", (PyCFunction)_wrap_gtk_source_buffer_get_highlight, METH_NOARGS },
    { "set_highlight", (PyCFunction)_wrap_gtk_source_buffer_set_highlight, METH_VARARGS|METH_KEYWORDS },
    { "get_max_undo_levels", (PyCFunction)_wrap_gtk_source_buffer_get_max_undo_levels, METH_NOARGS },
    { "set_max_undo_levels", (PyCFunction)_wrap_gtk_source_buffer_set_max_undo_levels, METH_VARARGS|METH_KEYWORDS },
    { "get_escape_char", (PyCFunction)_wrap_gtk_source_buffer_get_escape_char, METH_NOARGS },
    { "set_escape_char", (PyCFunction)_wrap_gtk_source_buffer_set_escape_char, METH_VARARGS|METH_KEYWORDS },
    { "can_undo", (PyCFunction)_wrap_gtk_source_buffer_can_undo, METH_NOARGS },
    { "can_redo", (PyCFunction)_wrap_gtk_source_buffer_can_redo, METH_NOARGS },
    { "undo", (PyCFunction)_wrap_gtk_source_buffer_undo, METH_NOARGS },
    { "redo", (PyCFunction)_wrap_gtk_source_buffer_redo, METH_NOARGS },
    { "begin_not_undoable_action", (PyCFunction)_wrap_gtk_source_buffer_begin_not_undoable_action, METH_NOARGS },
    { "end_not_undoable_action", (PyCFunction)_wrap_gtk_source_buffer_end_not_undoable_action, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyGtkSourceBuffer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "moo_gtksourceview.SourceBuffer",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGtkSourceBuffer_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GtkSourceView ----------- */

static int
_wrap_gtk_source_view_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, ":moo_gtksourceview.SourceView.__init__", kwlist))
        return -1;

    self->obj = g_object_newv(obj_type, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create %(typename)s object");
        return -1;
    }

    pygobject_register_wrapper((PyObject *)self);
    return 0;
}


static PyObject *
_wrap_gtk_source_view_set_show_line_numbers(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "show", NULL };
    int show;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_show_line_numbers", kwlist, &show))
        return NULL;
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(self->obj), show);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_show_line_numbers(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_show_line_numbers(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_show_line_markers(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "show", NULL };
    int show;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_show_line_markers", kwlist, &show))
        return NULL;
    gtk_source_view_set_show_line_markers(GTK_SOURCE_VIEW(self->obj), show);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_show_line_markers(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_show_line_markers(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_tabs_width(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "width", NULL };
    int width;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_tabs_width", kwlist, &width))
        return NULL;
    gtk_source_view_set_tabs_width(GTK_SOURCE_VIEW(self->obj), width);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_tabs_width(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_tabs_width(GTK_SOURCE_VIEW(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gtk_source_view_set_auto_indent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enable", NULL };
    int enable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_auto_indent", kwlist, &enable))
        return NULL;
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(self->obj), enable);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_auto_indent(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_auto_indent(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_insert_spaces_instead_of_tabs(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enable", NULL };
    int enable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_insert_spaces_instead_of_tabs", kwlist, &enable))
        return NULL;
    gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(self->obj), enable);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_insert_spaces_instead_of_tabs(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_show_margin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "show", NULL };
    int show;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_show_margin", kwlist, &show))
        return NULL;
    gtk_source_view_set_show_margin(GTK_SOURCE_VIEW(self->obj), show);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_show_margin(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_show_margin(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_highlight_current_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "show", NULL };
    int show;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_highlight_current_line", kwlist, &show))
        return NULL;
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(self->obj), show);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_highlight_current_line(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_highlight_current_line(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gtk_source_view_set_margin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "margin", NULL };
    int margin;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_margin", kwlist, &margin))
        return NULL;
    gtk_source_view_set_margin(GTK_SOURCE_VIEW(self->obj), margin);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_margin(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_margin(GTK_SOURCE_VIEW(self->obj));
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gtk_source_view_set_marker_pixbuf(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "marker_type", "pixbuf", NULL };
    char *marker_type;
    PyGObject *pixbuf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO!:GtkSourceView.set_marker_pixbuf", kwlist, &marker_type, &PyGdkPixbuf_Type, &pixbuf))
        return NULL;
    gtk_source_view_set_marker_pixbuf(GTK_SOURCE_VIEW(self->obj), marker_type, GDK_PIXBUF(pixbuf->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_marker_pixbuf(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "marker_type", NULL };
    char *marker_type;
    GdkPixbuf *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:GtkSourceView.get_marker_pixbuf", kwlist, &marker_type))
        return NULL;
    ret = gtk_source_view_get_marker_pixbuf(GTK_SOURCE_VIEW(self->obj), marker_type);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gtk_source_view_set_smart_home_end(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enable", NULL };
    int enable;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GtkSourceView.set_smart_home_end", kwlist, &enable))
        return NULL;
    gtk_source_view_set_smart_home_end(GTK_SOURCE_VIEW(self->obj), enable);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gtk_source_view_get_smart_home_end(PyGObject *self)
{
    int ret;

    ret = gtk_source_view_get_smart_home_end(GTK_SOURCE_VIEW(self->obj));
    return PyBool_FromLong(ret);

}

static PyMethodDef _PyGtkSourceView_methods[] = {
    { "set_show_line_numbers", (PyCFunction)_wrap_gtk_source_view_set_show_line_numbers, METH_VARARGS|METH_KEYWORDS },
    { "get_show_line_numbers", (PyCFunction)_wrap_gtk_source_view_get_show_line_numbers, METH_NOARGS },
    { "set_show_line_markers", (PyCFunction)_wrap_gtk_source_view_set_show_line_markers, METH_VARARGS|METH_KEYWORDS },
    { "get_show_line_markers", (PyCFunction)_wrap_gtk_source_view_get_show_line_markers, METH_NOARGS },
    { "set_tabs_width", (PyCFunction)_wrap_gtk_source_view_set_tabs_width, METH_VARARGS|METH_KEYWORDS },
    { "get_tabs_width", (PyCFunction)_wrap_gtk_source_view_get_tabs_width, METH_NOARGS },
    { "set_auto_indent", (PyCFunction)_wrap_gtk_source_view_set_auto_indent, METH_VARARGS|METH_KEYWORDS },
    { "get_auto_indent", (PyCFunction)_wrap_gtk_source_view_get_auto_indent, METH_NOARGS },
    { "set_insert_spaces_instead_of_tabs", (PyCFunction)_wrap_gtk_source_view_set_insert_spaces_instead_of_tabs, METH_VARARGS|METH_KEYWORDS },
    { "get_insert_spaces_instead_of_tabs", (PyCFunction)_wrap_gtk_source_view_get_insert_spaces_instead_of_tabs, METH_NOARGS },
    { "set_show_margin", (PyCFunction)_wrap_gtk_source_view_set_show_margin, METH_VARARGS|METH_KEYWORDS },
    { "get_show_margin", (PyCFunction)_wrap_gtk_source_view_get_show_margin, METH_NOARGS },
    { "set_highlight_current_line", (PyCFunction)_wrap_gtk_source_view_set_highlight_current_line, METH_VARARGS|METH_KEYWORDS },
    { "get_highlight_current_line", (PyCFunction)_wrap_gtk_source_view_get_highlight_current_line, METH_NOARGS },
    { "set_margin", (PyCFunction)_wrap_gtk_source_view_set_margin, METH_VARARGS|METH_KEYWORDS },
    { "get_margin", (PyCFunction)_wrap_gtk_source_view_get_margin, METH_NOARGS },
    { "set_marker_pixbuf", (PyCFunction)_wrap_gtk_source_view_set_marker_pixbuf, METH_VARARGS|METH_KEYWORDS },
    { "get_marker_pixbuf", (PyCFunction)_wrap_gtk_source_view_get_marker_pixbuf, METH_VARARGS|METH_KEYWORDS },
    { "set_smart_home_end", (PyCFunction)_wrap_gtk_source_view_set_smart_home_end, METH_VARARGS|METH_KEYWORDS },
    { "get_smart_home_end", (PyCFunction)_wrap_gtk_source_view_get_smart_home_end, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyGtkSourceView_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "moo_gtksourceview.SourceView",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGtkSourceView_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gtk_source_view_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_gtk_source_view_new_with_buffer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", NULL };
    PyGObject *buffer;
    GtkWidget *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:gtk_source_view_new_with_buffer", kwlist, &PyGtkSourceBuffer_Type, &buffer))
        return NULL;
    ret = gtk_source_view_new_with_buffer(GTK_SOURCE_BUFFER(buffer->obj));
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

PyMethodDef moo_gtksourceview_functions[] = {
    { "gtk_source_view_new_with_buffer", (PyCFunction)_wrap_gtk_source_view_new_with_buffer, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

/* initialise stuff extension classes */
void
moo_gtksourceview_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkTextView_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "TextView");
        if (_PyGtkTextView_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name TextView from gtk");
            return;
        }
        _PyGtkTextBuffer_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "TextBuffer");
        if (_PyGtkTextBuffer_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name TextBuffer from gtk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return;
    }
    if ((module = PyImport_ImportModule("gtk.gdk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGdkPixbuf_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Pixbuf");
        if (_PyGdkPixbuf_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Pixbuf from gtk.gdk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk.gdk");
        return;
    }


#line 647 "/home/muntyan/projects/moo/moo/mooedit/gtksourceview/gtksourceview-pygtk.c"
    pygobject_register_class(d, "GtkSourceBuffer", GTK_TYPE_SOURCE_BUFFER, &PyGtkSourceBuffer_Type, Py_BuildValue("(O)", &PyGtkTextBuffer_Type));
    pygobject_register_class(d, "GtkSourceView", GTK_TYPE_SOURCE_VIEW, &PyGtkSourceView_Type, Py_BuildValue("(O)", &PyGtkTextView_Type));
}
