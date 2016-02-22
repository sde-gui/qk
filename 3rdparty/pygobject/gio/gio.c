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


#line 29 "gio.override"
#define NO_IMPORT_PYGOBJECT
#include <pygobject.h>
#include <gio/gio.h>
#include "pygio-utils.h"
#include "pyglib.h"
#include "pygsource.h"

#define BUFSIZE 8192

typedef struct _PyGIONotify PyGIONotify;

struct _PyGIONotify {
    gboolean  referenced;
    PyObject *callback;
    PyObject *data;
    gboolean  attach_self;
    gpointer  buffer;
    gsize     buffer_size;

    /* If a structure has any 'slaves', those will reference their
     * callbacks and be freed together with the 'master'. */
    PyGIONotify *slaves;
};

static GQuark
pygio_notify_get_internal_quark(void)
{
    static GQuark quark = 0;
    if (!quark)
        quark = g_quark_from_string("pygio::notify");
    return quark;
}

static PyGIONotify *
pygio_notify_new(void)
{
    return g_slice_new0(PyGIONotify);
}

static PyGIONotify *
pygio_notify_new_slave(PyGIONotify* master)
{
    PyGIONotify *slave = pygio_notify_new();

    while (master->slaves)
        master = master->slaves;
    master->slaves = slave;

    return slave;
}

static gboolean
pygio_notify_using_optional_callback(PyGIONotify *notify)
{
    if (notify->callback)
        return TRUE;
    else {
        notify->data = NULL;
        return FALSE;
    }
}

static gboolean
pygio_notify_callback_is_valid_full(PyGIONotify *notify, const gchar *name)
{
    if (!notify->callback) {
        PyErr_SetString(PyExc_RuntimeError, "internal error: callback is not set");
        return FALSE;
    }

    if (!PyCallable_Check(notify->callback)) {
        gchar *error_message = g_strdup_printf("%s argument not callable", name);

	PyErr_SetString(PyExc_TypeError, error_message);
        g_free(error_message);
	return FALSE;
    }

    return TRUE;
}

static gboolean
pygio_notify_callback_is_valid(PyGIONotify *notify)
{
    return pygio_notify_callback_is_valid_full(notify, "callback");
}

static void
pygio_notify_reference_callback(PyGIONotify *notify)
{
    if (!notify->referenced) {
        notify->referenced = TRUE;
        Py_XINCREF(notify->callback);
        Py_XINCREF(notify->data);

        if (notify->slaves)
            pygio_notify_reference_callback(notify->slaves);
    }
}

static void
pygio_notify_copy_buffer(PyGIONotify *notify, gpointer buffer, gsize buffer_size)
{
    if (buffer_size > 0) {
	notify->buffer = g_slice_copy(buffer_size, buffer);
	notify->buffer_size = buffer_size;
    }
}

static gboolean
pygio_notify_allocate_buffer(PyGIONotify *notify, gsize buffer_size)
{
    if (buffer_size > 0) {
        notify->buffer = g_slice_alloc(buffer_size);
        if (!notify->buffer) {
            PyErr_Format(PyExc_MemoryError, "failed to allocate %" G_GSIZE_FORMAT " bytes", buffer_size);
            return FALSE;
        }

        notify->buffer_size = buffer_size;
    }

    return TRUE;
}

static void
pygio_notify_attach_to_result(PyGIONotify *notify)
{
    notify->attach_self = TRUE;
}

static PyGIONotify *
pygio_notify_get_attached(PyGObject *result)
{
    return g_object_get_qdata(G_OBJECT(result->obj), pygio_notify_get_internal_quark());
}

static void
pygio_notify_free(PyGIONotify *notify)
{
    if (notify) {
        if (notify->slaves)
            pygio_notify_free(notify->slaves);

        if (notify->referenced) {
            PyGILState_STATE state;

            state = pyg_gil_state_ensure();
            Py_XDECREF(notify->callback);
            Py_XDECREF(notify->data);
            pyg_gil_state_release(state);
        }

        if (notify->buffer)
            g_slice_free1(notify->buffer_size, notify->buffer);

        g_slice_free(PyGIONotify, notify);
    }
}

static void
async_result_callback_marshal(GObject *source_object,
			      GAsyncResult *result,
			      PyGIONotify *notify)
{
    PyObject *ret;
    PyGILState_STATE state;

    state = pyg_gil_state_ensure();

    if (!notify->referenced)
        g_warning("pygio_notify_reference_callback() hasn't been called before using the structure");

    if (notify->attach_self) {
        g_object_set_qdata_full(G_OBJECT(result), pygio_notify_get_internal_quark(),
                                notify, (GDestroyNotify) pygio_notify_free);
    }

    if (notify->data)
	ret = PyEval_CallFunction(notify->callback, "NNO",
				  pygobject_new(source_object),
				  pygobject_new((GObject *)result),
				  notify->data);
    else
	ret = PyObject_CallFunction(notify->callback, "NN",
				    pygobject_new(source_object),
				    pygobject_new((GObject *)result));

    if (ret == NULL) {
	PyErr_Print();
	PyErr_Clear();
    }

    Py_XDECREF(ret);

    /* Otherwise the structure is attached to 'result' and will be
     * freed when that object dies. */
    if (!notify->attach_self)
        pygio_notify_free(notify);

    pyg_gil_state_release(state);
}

#line 24 "gfile.override"

static void
file_progress_callback_marshal(goffset current_num_bytes,
			       goffset total_num_bytes,
			       PyGIONotify *notify)
{
    PyObject *ret;
    PyGILState_STATE state;

    state = pyg_gil_state_ensure();

    if (notify->data)
	ret = PyObject_CallFunction(notify->callback, "(KKO)",
				    current_num_bytes,
				    total_num_bytes,
				    notify->data);
    else
	ret = PyObject_CallFunction(notify->callback, "(KK)",
				    current_num_bytes,
				    total_num_bytes);

    if (ret == NULL)
      {
	PyErr_Print();
	PyErr_Clear();
      }

    Py_XDECREF(ret);
    pyg_gil_state_release(state);
}

#line 24 "gfileattribute.override"

extern PyTypeObject PyGFileAttributeInfo_Type;

typedef struct {
    PyObject_HEAD
    const GFileAttributeInfo *info;
} PyGFileAttributeInfo;

static PyObject *
pygio_file_attribute_info_tp_new(PyTypeObject *type)
{
    PyGFileAttributeInfo *self;
    GFileAttributeInfo *info = NULL;

    self = (PyGFileAttributeInfo *) PyObject_NEW(PyGFileAttributeInfo,
                                              &PyGFileAttributeInfo_Type);
    self->info = info;
    return (PyObject *) self;
}

static PyMethodDef pyg_file_attribute_info_methods[] = {
    { NULL,  0, 0 }
};

static PyObject *
pyg_file_attribute_info__get_name(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = ((PyGFileAttributeInfo*)self)->info->name;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
pyg_file_attribute_info__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = ((PyGFileAttributeInfo*)self)->info->type;
    return pyg_enum_from_gtype(G_TYPE_FILE_ATTRIBUTE_TYPE, ret);
}

static PyObject *
pyg_file_attribute_info__get_flags(PyObject *self, void *closure)
{
    guint ret;

    ret = ((PyGFileAttributeInfo*)self)->info->flags;
    return pyg_flags_from_gtype(G_TYPE_FILE_ATTRIBUTE_INFO_FLAGS, ret);
}

static const PyGetSetDef pyg_file_attribute_info_getsets[] = {
    { "name", (getter)pyg_file_attribute_info__get_name, (setter)0 },
    { "type", (getter)pyg_file_attribute_info__get_type, (setter)0 },
    { "flags", (getter)pyg_file_attribute_info__get_flags, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGFileAttributeInfo_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "gio.FileAttributeInfo",            /* tp_name */
    sizeof(PyGFileAttributeInfo),      /* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)0,                      /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    "Holds information about an attribute", /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    (struct PyMethodDef*)pyg_file_attribute_info_methods,    /* tp_methods */
    0,                                  /* tp_members */
    (struct PyGetSetDef*)pyg_file_attribute_info_getsets,    /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    0,                                  /* tp_alloc */
    (newfunc)pygio_file_attribute_info_tp_new,   /* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};

PyObject*
pyg_file_attribute_info_new(const GFileAttributeInfo *info)
{
    PyGFileAttributeInfo *self;

    self = (PyGFileAttributeInfo *)PyObject_NEW(PyGFileAttributeInfo,
                                             &PyGFileAttributeInfo_Type);
    if (G_UNLIKELY(self == NULL))
        return NULL;
    if (info)
        self->info = info;
    return (PyObject *)self;
}


#line 26 "gfileinfo.override"

#ifndef G_TYPE_FILE_ATTRIBUTE_MATCHER
#define G_TYPE_FILE_ATTRIBUTE_MATCHER (_g_file_attribute_matcher_get_type ())

static GType _g_file_attribute_matcher_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("GFileAttributeMatcher",
                                (GBoxedCopyFunc)g_file_attribute_matcher_ref,
                                (GBoxedFreeFunc)g_file_attribute_matcher_unref);

  return our_type;
}
#endif


#line 24 "ginputstream.override"
#define BUFSIZE 8192

#line 401 "gio.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGPollFD_Type;
#define PyGPollFD_Type (*_PyGPollFD_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyGFileAttributeMatcher_Type;
PyTypeObject G_GNUC_INTERNAL PyGSrvTarget_Type;
PyTypeObject G_GNUC_INTERNAL PyGAppLaunchContext_Type;
PyTypeObject G_GNUC_INTERNAL PyGCancellable_Type;
PyTypeObject G_GNUC_INTERNAL PyGEmblem_Type;
PyTypeObject G_GNUC_INTERNAL PyGEmblemedIcon_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileEnumerator_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileInfo_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileMonitor_Type;
PyTypeObject G_GNUC_INTERNAL PyGInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileIOStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGFilterInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGBufferedInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGDataInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGMemoryInputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGMountOperation_Type;
PyTypeObject G_GNUC_INTERNAL PyGInetAddress_Type;
PyTypeObject G_GNUC_INTERNAL PyGInetSocketAddress_Type;
PyTypeObject G_GNUC_INTERNAL PyGNetworkAddress_Type;
PyTypeObject G_GNUC_INTERNAL PyGNetworkService_Type;
PyTypeObject G_GNUC_INTERNAL PyGResolver_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocket_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketAddress_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketAddressEnumerator_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketClient_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketConnection_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketControlMessage_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketListener_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketService_Type;
PyTypeObject G_GNUC_INTERNAL PyGTcpConnection_Type;
PyTypeObject G_GNUC_INTERNAL PyGThreadedSocketService_Type;
PyTypeObject G_GNUC_INTERNAL PyGIOStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGMemoryOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGFilterOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGBufferedOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGDataOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileOutputStream_Type;
PyTypeObject G_GNUC_INTERNAL PyGSimpleAsyncResult_Type;
PyTypeObject G_GNUC_INTERNAL PyGVfs_Type;
PyTypeObject G_GNUC_INTERNAL PyGVolumeMonitor_Type;
PyTypeObject G_GNUC_INTERNAL PyGNativeVolumeMonitor_Type;
PyTypeObject G_GNUC_INTERNAL PyGFileIcon_Type;
PyTypeObject G_GNUC_INTERNAL PyGThemedIcon_Type;
PyTypeObject G_GNUC_INTERNAL PyGAppInfo_Type;
PyTypeObject G_GNUC_INTERNAL PyGAsyncInitable_Type;
PyTypeObject G_GNUC_INTERNAL PyGAsyncResult_Type;
PyTypeObject G_GNUC_INTERNAL PyGDrive_Type;
PyTypeObject G_GNUC_INTERNAL PyGFile_Type;
PyTypeObject G_GNUC_INTERNAL PyGIcon_Type;
PyTypeObject G_GNUC_INTERNAL PyGInitable_Type;
PyTypeObject G_GNUC_INTERNAL PyGLoadableIcon_Type;
PyTypeObject G_GNUC_INTERNAL PyGMount_Type;
PyTypeObject G_GNUC_INTERNAL PyGSeekable_Type;
PyTypeObject G_GNUC_INTERNAL PyGSocketConnectable_Type;
PyTypeObject G_GNUC_INTERNAL PyGVolume_Type;

#line 470 "gio.c"



/* ----------- GFileAttributeMatcher ----------- */

static int
_wrap_g_file_attribute_matcher_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", NULL };
    char *attributes;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileAttributeMatcher.__init__", kwlist, &attributes))
        return -1;
    self->gtype = G_TYPE_FILE_ATTRIBUTE_MATCHER;
    self->free_on_dealloc = FALSE;
    self->boxed = g_file_attribute_matcher_new(attributes);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GFileAttributeMatcher object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_g_file_attribute_matcher_matches(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileAttributeMatcher.matches", kwlist, &attribute))
        return NULL;
    
    ret = g_file_attribute_matcher_matches(pyg_boxed_get(self, GFileAttributeMatcher), attribute);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_attribute_matcher_matches_only(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileAttributeMatcher.matches_only", kwlist, &attribute))
        return NULL;
    
    ret = g_file_attribute_matcher_matches_only(pyg_boxed_get(self, GFileAttributeMatcher), attribute);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_attribute_matcher_enumerate_namespace(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ns", NULL };
    char *ns;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileAttributeMatcher.enumerate_namespace", kwlist, &ns))
        return NULL;
    
    ret = g_file_attribute_matcher_enumerate_namespace(pyg_boxed_get(self, GFileAttributeMatcher), ns);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_attribute_matcher_enumerate_next(PyObject *self)
{
    const gchar *ret;

    
    ret = g_file_attribute_matcher_enumerate_next(pyg_boxed_get(self, GFileAttributeMatcher));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFileAttributeMatcher_methods[] = {
    { "matches", (PyCFunction)_wrap_g_file_attribute_matcher_matches, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "matches_only", (PyCFunction)_wrap_g_file_attribute_matcher_matches_only, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enumerate_namespace", (PyCFunction)_wrap_g_file_attribute_matcher_enumerate_namespace, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enumerate_next", (PyCFunction)_wrap_g_file_attribute_matcher_enumerate_next, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileAttributeMatcher_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileAttributeMatcher",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileAttributeMatcher_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_g_file_attribute_matcher_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GSrvTarget ----------- */

static int
_wrap_g_srv_target_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "hostname", "port", "priority", "weight", NULL };
    char *hostname;
    int port, priority, weight;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"siii:gio.SrvTarget.__init__", kwlist, &hostname, &port, &priority, &weight))
        return -1;
    self->gtype = G_TYPE_SRV_TARGET;
    self->free_on_dealloc = FALSE;
    self->boxed = g_srv_target_new(hostname, port, priority, weight);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GSrvTarget object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_g_srv_target_copy(PyObject *self)
{
    GSrvTarget *ret;

    
    ret = g_srv_target_copy(pyg_boxed_get(self, GSrvTarget));
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(G_TYPE_SRV_TARGET, ret, TRUE, TRUE);
}

static PyObject *
_wrap_g_srv_target_get_hostname(PyObject *self)
{
    const gchar *ret;

    
    ret = g_srv_target_get_hostname(pyg_boxed_get(self, GSrvTarget));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_srv_target_get_port(PyObject *self)
{
    int ret;

    
    ret = g_srv_target_get_port(pyg_boxed_get(self, GSrvTarget));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_srv_target_get_priority(PyObject *self)
{
    int ret;

    
    ret = g_srv_target_get_priority(pyg_boxed_get(self, GSrvTarget));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_srv_target_get_weight(PyObject *self)
{
    int ret;

    
    ret = g_srv_target_get_weight(pyg_boxed_get(self, GSrvTarget));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGSrvTarget_methods[] = {
    { "copy", (PyCFunction)_wrap_g_srv_target_copy, METH_NOARGS,
      NULL },
    { "get_hostname", (PyCFunction)_wrap_g_srv_target_get_hostname, METH_NOARGS,
      NULL },
    { "get_port", (PyCFunction)_wrap_g_srv_target_get_port, METH_NOARGS,
      NULL },
    { "get_priority", (PyCFunction)_wrap_g_srv_target_get_priority, METH_NOARGS,
      NULL },
    { "get_weight", (PyCFunction)_wrap_g_srv_target_get_weight, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSrvTarget_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SrvTarget",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSrvTarget_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_g_srv_target_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GAppLaunchContext ----------- */

static int
_wrap_g_app_launch_context_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.AppLaunchContext.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.AppLaunchContext object");
        return -1;
    }
    return 0;
}

#line 25 "gapplaunchcontext.override"
static PyObject *
_wrap_g_app_launch_context_get_display(PyGObject *self,
                                       PyObject *args,
                                       PyObject *kwargs)
{
    static char *kwlist[] = { "info", "files", NULL };

    GList *file_list = NULL;
    PyGObject *py_info;
    PyObject *pyfile_list;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
			    "O!O:gio.AppLaunchContext.get_display",
			    kwlist,
			    &PyGAppInfo_Type, &py_info, &pyfile_list))
        return NULL;

    if (!PySequence_Check (pyfile_list)) {
        PyErr_Format (PyExc_TypeError,
                      "argument must be a list or tuple of GFile objects");
        return NULL;
    }

    file_list = pygio_pylist_to_gfile_glist(pyfile_list);

    ret = g_app_launch_context_get_display(G_APP_LAUNCH_CONTEXT(self->obj),
                                           G_APP_INFO(py_info->obj), file_list);
    g_list_free(file_list);

    if (ret)
        return PyString_FromString(ret);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 819 "gio.c"


#line 63 "gapplaunchcontext.override"
static PyObject *
_wrap_g_app_launch_context_get_startup_notify_id(PyGObject *self,
                                                 PyObject *args,
                                                 PyObject *kwargs)
{
    static char *kwlist[] = { "info", "files", NULL };

    GList       *file_list = NULL;
    PyGObject   *py_info;
    PyObject    *pyfile_list;
    gchar       *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
			    "O!O:gio.AppLaunchContext.get_startup_notify_id",
			    kwlist,
			    &PyGAppInfo_Type, &py_info, &pyfile_list))
        return NULL;

    if (!PySequence_Check (pyfile_list)) {
        PyErr_Format (PyExc_TypeError,
                      "argument must be a list or tuple of GFile objects");
        return NULL;
    }

    file_list = pygio_pylist_to_gfile_glist(pyfile_list);

    ret = g_app_launch_context_get_startup_notify_id(
                                        G_APP_LAUNCH_CONTEXT(self->obj),
                                        G_APP_INFO(py_info->obj), file_list);
    g_list_free(file_list);

    if (ret)
        return PyString_FromString(ret);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 860 "gio.c"


static PyObject *
_wrap_g_app_launch_context_launch_failed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "startup_notify_id", NULL };
    char *startup_notify_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.AppLaunchContext.launch_failed", kwlist, &startup_notify_id))
        return NULL;
    
    g_app_launch_context_launch_failed(G_APP_LAUNCH_CONTEXT(self->obj), startup_notify_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGAppLaunchContext_methods[] = {
    { "get_display", (PyCFunction)_wrap_g_app_launch_context_get_display, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_startup_notify_id", (PyCFunction)_wrap_g_app_launch_context_get_startup_notify_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "launch_failed", (PyCFunction)_wrap_g_app_launch_context_launch_failed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGAppLaunchContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.AppLaunchContext",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGAppLaunchContext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_app_launch_context_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GCancellable ----------- */

static int
_wrap_g_cancellable_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.Cancellable.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.Cancellable object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_cancellable_is_cancelled(PyGObject *self)
{
    int ret;

    
    ret = g_cancellable_is_cancelled(G_CANCELLABLE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_cancellable_set_error_if_cancelled(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_cancellable_set_error_if_cancelled(G_CANCELLABLE(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_cancellable_get_fd(PyGObject *self)
{
    int ret;

    
    ret = g_cancellable_get_fd(G_CANCELLABLE(self->obj));
    
    return PyInt_FromLong(ret);
}

#line 25 "gcancellable.override"
static PyObject *
_wrap_g_cancellable_make_pollfd (PyGObject *self)
{
    GPollFD pollfd;
    gboolean ret;
    PyGPollFD *pypollfd;

    ret = g_cancellable_make_pollfd(G_CANCELLABLE(self->obj), &pollfd);
  
    pypollfd = PyObject_NEW(PyGPollFD, &PyGPollFD_Type);
    pypollfd->fd_obj = NULL;
    pypollfd->pollfd = pollfd;
    return (PyObject *) pypollfd;
}
#line 1010 "gio.c"


static PyObject *
_wrap_g_cancellable_push_current(PyGObject *self)
{
    
    g_cancellable_push_current(G_CANCELLABLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_cancellable_pop_current(PyGObject *self)
{
    
    g_cancellable_pop_current(G_CANCELLABLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_cancellable_reset(PyGObject *self)
{
    
    g_cancellable_reset(G_CANCELLABLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_cancellable_cancel(PyGObject *self)
{
    
    g_cancellable_cancel(G_CANCELLABLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_cancellable_disconnect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    unsigned long handler_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:gio.Cancellable.disconnect", kwlist, &handler_id))
        return NULL;
    
    g_cancellable_disconnect(G_CANCELLABLE(self->obj), handler_id);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_cancellable_release_fd(PyGObject *self)
{
    
    g_cancellable_release_fd(G_CANCELLABLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGCancellable_methods[] = {
    { "is_cancelled", (PyCFunction)_wrap_g_cancellable_is_cancelled, METH_NOARGS,
      NULL },
    { "set_error_if_cancelled", (PyCFunction)_wrap_g_cancellable_set_error_if_cancelled, METH_NOARGS,
      NULL },
    { "get_fd", (PyCFunction)_wrap_g_cancellable_get_fd, METH_NOARGS,
      NULL },
    { "make_pollfd", (PyCFunction)_wrap_g_cancellable_make_pollfd, METH_NOARGS,
      NULL },
    { "push_current", (PyCFunction)_wrap_g_cancellable_push_current, METH_NOARGS,
      NULL },
    { "pop_current", (PyCFunction)_wrap_g_cancellable_pop_current, METH_NOARGS,
      NULL },
    { "reset", (PyCFunction)_wrap_g_cancellable_reset, METH_NOARGS,
      NULL },
    { "cancel", (PyCFunction)_wrap_g_cancellable_cancel, METH_NOARGS,
      NULL },
    { "disconnect", (PyCFunction)_wrap_g_cancellable_disconnect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "release_fd", (PyCFunction)_wrap_g_cancellable_release_fd, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGCancellable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Cancellable",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGCancellable_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_cancellable_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GEmblem ----------- */

static int
_wrap_g_emblem_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[2];
    PyObject *parsed_args[2] = {NULL, };
    char *arg_names[] = {"icon", "origin", NULL };
    char *prop_names[] = {"icon", "origin", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|O:gio.Emblem.__init__" , arg_names , &parsed_args[0] , &parsed_args[1]))
        return -1;

    memset(params, 0, sizeof(GParameter)*2);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.Emblem object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_emblem_get_icon(PyGObject *self)
{
    GIcon *ret;

    
    ret = g_emblem_get_icon(G_EMBLEM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_emblem_get_origin(PyGObject *self)
{
    gint ret;

    
    ret = g_emblem_get_origin(G_EMBLEM(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_EMBLEM_ORIGIN, ret);
}

static const PyMethodDef _PyGEmblem_methods[] = {
    { "get_icon", (PyCFunction)_wrap_g_emblem_get_icon, METH_NOARGS,
      NULL },
    { "get_origin", (PyCFunction)_wrap_g_emblem_get_origin, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGEmblem_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Emblem",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGEmblem_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_emblem_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GEmblemedIcon ----------- */

static int
_wrap_g_emblemed_icon_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon", "emblem", NULL };
    PyGObject *icon, *emblem;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:gio.EmblemedIcon.__init__", kwlist, &PyGIcon_Type, &icon, &PyGEmblem_Type, &emblem))
        return -1;
    self->obj = (GObject *)g_emblemed_icon_new(G_ICON(icon->obj), G_EMBLEM(emblem->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GEmblemedIcon object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_emblemed_icon_get_icon(PyGObject *self)
{
    GIcon *ret;

    
    ret = g_emblemed_icon_get_icon(G_EMBLEMED_ICON(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 299 "gicon.override"
static PyObject *
_wrap_g_emblemed_icon_get_emblems(PyGObject *self)
{
    GList *list;
    PyObject *ret;

    list = g_emblemed_icon_get_emblems(G_EMBLEMED_ICON(self->obj));
    
    PYLIST_FROMGLIST(ret, list, pygobject_new(list_item), NULL, NULL);

    return ret;
}
#line 1304 "gio.c"


static PyObject *
_wrap_g_emblemed_icon_add_emblem(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "emblem", NULL };
    PyGObject *emblem;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.EmblemedIcon.add_emblem", kwlist, &PyGEmblem_Type, &emblem))
        return NULL;
    
    g_emblemed_icon_add_emblem(G_EMBLEMED_ICON(self->obj), G_EMBLEM(emblem->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGEmblemedIcon_methods[] = {
    { "get_icon", (PyCFunction)_wrap_g_emblemed_icon_get_icon, METH_NOARGS,
      NULL },
    { "get_emblems", (PyCFunction)_wrap_g_emblemed_icon_get_emblems, METH_NOARGS,
      NULL },
    { "add_emblem", (PyCFunction)_wrap_g_emblemed_icon_add_emblem, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGEmblemedIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.EmblemedIcon",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGEmblemedIcon_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_emblemed_icon_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GFileEnumerator ----------- */

static PyObject *
_wrap_g_file_enumerator_next_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    PyObject *py_ret;
    GCancellable *cancellable = NULL;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.FileEnumerator.next_file", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_enumerator_next_file(G_FILE_ENUMERATOR(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_enumerator_close(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.FileEnumerator.close", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_enumerator_close(G_FILE_ENUMERATOR(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 59 "gfileenumerator.override"
static PyObject *
_wrap_g_file_enumerator_next_files_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "num_files", "callback",
			      "io_priority", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    int num_files;
    int io_priority = G_PRIORITY_DEFAULT;
    GCancellable *cancellable = NULL;
    PyGObject *py_cancellable = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO|iOO:gio.FileEnumerator.enumerate_next_files_async",
				     kwlist,
				     &num_files,
				     &notify->callback,
				     &io_priority,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);  
    
    g_file_enumerator_next_files_async(G_FILE_ENUMERATOR(self->obj),
				       num_files,
				       io_priority,
				       (GCancellable *) cancellable,
				       (GAsyncReadyCallback)async_result_callback_marshal,
				       notify);
    
    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 1486 "gio.c"


#line 106 "gfileenumerator.override"
static PyObject *
_wrap_g_file_enumerator_next_files_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GList *next_files, *l;
    GError *error = NULL;
    PyObject *ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!:gio.FileEnumerator.next_files_finish",
				     kwlist,
				     &PyGAsyncResult_Type, &result))
        return NULL;
    
    next_files = g_file_enumerator_next_files_finish(G_FILE_ENUMERATOR(self->obj),
						     G_ASYNC_RESULT(result->obj),
						     &error);
    if (pyg_error_check(&error))
        return NULL;

    ret = PyList_New(0);
    for (l = next_files; l; l = l->next) {
	GFileInfo *file_info = l->data;
	PyObject *item = pygobject_new((GObject *)file_info);
	PyList_Append(ret, item);
	Py_DECREF(item);
	g_object_unref(file_info);
    }
    g_list_free(next_files);

    return ret;
}
#line 1523 "gio.c"


#line 141 "gfileenumerator.override"
static PyObject *
_wrap_g_file_enumerator_close_async(PyGObject *self,
                                    PyObject *args,
                                    PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority", "cancellable",
                              "user_data", NULL };
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:gio.FileEnumerator.close_async",
                                     kwlist,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_enumerator_close_async(G_FILE_ENUMERATOR(self->obj),
			    io_priority,
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 1571 "gio.c"


static PyObject *
_wrap_g_file_enumerator_close_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileEnumerator.close_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_enumerator_close_finish(G_FILE_ENUMERATOR(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_enumerator_is_closed(PyGObject *self)
{
    int ret;

    
    ret = g_file_enumerator_is_closed(G_FILE_ENUMERATOR(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_enumerator_has_pending(PyGObject *self)
{
    int ret;

    
    ret = g_file_enumerator_has_pending(G_FILE_ENUMERATOR(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_enumerator_set_pending(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pending", NULL };
    int pending;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FileEnumerator.set_pending", kwlist, &pending))
        return NULL;
    
    g_file_enumerator_set_pending(G_FILE_ENUMERATOR(self->obj), pending);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_enumerator_get_container(PyGObject *self)
{
    GFile *ret;

    
    ret = g_file_enumerator_get_container(G_FILE_ENUMERATOR(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGFileEnumerator_methods[] = {
    { "next_file", (PyCFunction)_wrap_g_file_enumerator_next_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close", (PyCFunction)_wrap_g_file_enumerator_close, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "next_files_async", (PyCFunction)_wrap_g_file_enumerator_next_files_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "FE.next_files_async(num_files, callback, [io_priority, cancellable,\n"
"                    user_data])\n"
"Request information for a number of files from the enumerator\n"
"asynchronously. When all i/o for the operation is finished the callback\n"
"will be called with the requested information.\n"
"\n"
"The callback can be called with less than num_files files in case of error\n"
"or at the end of the enumerator. In case of a partial error the callback\n"
"will be called with any succeeding items and no error, and on the next\n"
"request the error will be reported. If a request is cancelled the callback\n"
"will be called with gio.ERROR_CANCELLED.\n"
"\n"
"During an async request no other sync and async calls are allowed, and will\n"
"result in gio.ERROR_PENDING errors.\n"
"\n"
"Any outstanding i/o request with higher priority (lower numerical value)\n"
"will be executed before an outstanding request with lower priority.\n"
"Default priority is gobject.PRIORITY_DEFAULT." },
    { "next_files_finish", (PyCFunction)_wrap_g_file_enumerator_next_files_finish, METH_VARARGS|METH_KEYWORDS,
      (char *) "FE.next_files_finish(result) -> a list of gio.FileInfos\n"
"Finishes the asynchronous operation started with\n"
"gio.FileEnumerator.next_files_async()." },
    { "close_async", (PyCFunction)_wrap_g_file_enumerator_close_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_finish", (PyCFunction)_wrap_g_file_enumerator_close_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_closed", (PyCFunction)_wrap_g_file_enumerator_is_closed, METH_NOARGS,
      NULL },
    { "has_pending", (PyCFunction)_wrap_g_file_enumerator_has_pending, METH_NOARGS,
      NULL },
    { "set_pending", (PyCFunction)_wrap_g_file_enumerator_set_pending, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_container", (PyCFunction)_wrap_g_file_enumerator_get_container, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 24 "gfileenumerator.override"
static PyObject*
_wrap_g_file_enumerator_tp_iter(PyGObject *self)
{
    Py_INCREF (self);
    return (PyObject *) self;
}
#line 1694 "gio.c"


#line 32 "gfileenumerator.override"
static PyObject*
_wrap_g_file_enumerator_tp_iternext(PyGObject *iter)
{
    GFileInfo *file_info;
    GError *error = NULL;

    if (!iter->obj) {
	PyErr_SetNone(PyExc_StopIteration);
	return NULL;
    }

    file_info = g_file_enumerator_next_file(G_FILE_ENUMERATOR(iter->obj),
					    NULL,
					    &error);
    if (pyg_error_check(&error)) {
        return NULL;
    }
    
    if (!file_info) {
	PyErr_SetNone(PyExc_StopIteration);
	return NULL;
    }

    return pygobject_new((GObject*)file_info);
}
#line 1723 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGFileEnumerator_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileEnumerator",                   /* tp_name */
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
    (getiterfunc)_wrap_g_file_enumerator_tp_iter,          /* tp_iter */
    (iternextfunc)_wrap_g_file_enumerator_tp_iternext,     /* tp_iternext */
    (struct PyMethodDef*)_PyGFileEnumerator_methods, /* tp_methods */
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



/* ----------- GFileInfo ----------- */

 static int
_wrap_g_file_info_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.FileInfo.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.FileInfo object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_file_info_dup(PyGObject *self)
{
    PyObject *py_ret;
    GFileInfo *ret;

    
    ret = g_file_info_dup(G_FILE_INFO(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_info_copy_into(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest_info", NULL };
    PyGObject *dest_info;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileInfo.copy_into", kwlist, &PyGFileInfo_Type, &dest_info))
        return NULL;
    
    g_file_info_copy_into(G_FILE_INFO(self->obj), G_FILE_INFO(dest_info->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_has_attribute(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.has_attribute", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_has_attribute(G_FILE_INFO(self->obj), attribute);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_has_namespace(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name_space", NULL };
    char *name_space;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.has_namespace", kwlist, &name_space))
        return NULL;
    
    ret = g_file_info_has_namespace(G_FILE_INFO(self->obj), name_space);
    
    return PyBool_FromLong(ret);

}

#line 45 "gfileinfo.override"
static PyObject *
_wrap_g_file_info_list_attributes(PyGObject *self, 
                                  PyObject  *args, 
				  PyObject  *kwargs)
{
    char *kwlist[] = { "name_space", NULL};
    gchar *name_space = NULL;
    gchar **names;
    gchar **n;
    PyObject *ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|z:gio.FileInfo.list_attributes",
				     kwlist, &name_space))
	return NULL;

    names = g_file_info_list_attributes(G_FILE_INFO(self->obj),
					name_space);

    ret = PyList_New(0);
    n = names;
    while (n && *n) {
        PyObject *item = PyString_FromString(n[0]);
        PyList_Append(ret, item);
        Py_DECREF(item);

        n++;
    }
    
    g_strfreev(names);
    return ret;
}
#line 1890 "gio.c"


static PyObject *
_wrap_g_file_info_get_attribute_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_type", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_type(G_FILE_INFO(self->obj), attribute);
    
    return pyg_enum_from_gtype(G_TYPE_FILE_ATTRIBUTE_TYPE, ret);
}

static PyObject *
_wrap_g_file_info_remove_attribute(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.remove_attribute", kwlist, &attribute))
        return NULL;
    
    g_file_info_remove_attribute(G_FILE_INFO(self->obj), attribute);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_attribute_status(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_status", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_status(G_FILE_INFO(self->obj), attribute);
    
    return pyg_enum_from_gtype(G_TYPE_FILE_ATTRIBUTE_STATUS, ret);
}

static PyObject *
_wrap_g_file_info_set_attribute_status(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "status", NULL };
    char *attribute;
    PyObject *py_status = NULL;
    int ret;
    GFileAttributeStatus status;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:gio.FileInfo.set_attribute_status", kwlist, &attribute, &py_status))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_FILE_ATTRIBUTE_STATUS, py_status, (gpointer)&status))
        return NULL;
    
    ret = g_file_info_set_attribute_status(G_FILE_INFO(self->obj), attribute, status);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_get_attribute_as_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_as_string", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_as_string(G_FILE_INFO(self->obj), attribute);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_attribute_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_string", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_string(G_FILE_INFO(self->obj), attribute);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_attribute_byte_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_byte_string", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_byte_string(G_FILE_INFO(self->obj), attribute);
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_attribute_boolean(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_boolean", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_boolean(G_FILE_INFO(self->obj), attribute);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_get_attribute_uint32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    guint32 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_uint32", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_uint32(G_FILE_INFO(self->obj), attribute);
    
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_g_file_info_get_attribute_int32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_int32", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_int32(G_FILE_INFO(self->obj), attribute);
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_file_info_get_attribute_uint64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_uint64", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_uint64(G_FILE_INFO(self->obj), attribute);
    
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_g_file_info_get_attribute_int64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    gint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_int64", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_int64(G_FILE_INFO(self->obj), attribute);
    
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_g_file_info_get_attribute_object(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", NULL };
    char *attribute;
    GObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.get_attribute_object", kwlist, &attribute))
        return NULL;
    
    ret = g_file_info_get_attribute_object(G_FILE_INFO(self->obj), attribute);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_info_set_attribute_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute, *attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:gio.FileInfo.set_attribute_string", kwlist, &attribute, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_string(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_byte_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute, *attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:gio.FileInfo.set_attribute_byte_string", kwlist, &attribute, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_byte_string(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_boolean(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    int attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:gio.FileInfo.set_attribute_boolean", kwlist, &attribute, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_boolean(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_uint32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    unsigned long attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sk:gio.FileInfo.set_attribute_uint32", kwlist, &attribute, &attr_value))
        return NULL;
    if (attr_value > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " attr_value parameter to unsigned 32 bit integer");
        return NULL;
    }
    
    g_file_info_set_attribute_uint32(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_int32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    int attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:gio.FileInfo.set_attribute_int32", kwlist, &attribute, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_int32(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_uint64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    PyObject *py_attr_value = NULL;
    guint64 attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:gio.FileInfo.set_attribute_uint64", kwlist, &attribute, &PyLong_Type, &py_attr_value))
        return NULL;
    attr_value = PyLong_AsUnsignedLongLong(py_attr_value);
    
    g_file_info_set_attribute_uint64(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_int64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    gint64 attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sL:gio.FileInfo.set_attribute_int64", kwlist, &attribute, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_int64(G_FILE_INFO(self->obj), attribute, attr_value);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_attribute_object(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "attr_value", NULL };
    char *attribute;
    PyGObject *attr_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:gio.FileInfo.set_attribute_object", kwlist, &attribute, &PyGObject_Type, &attr_value))
        return NULL;
    
    g_file_info_set_attribute_object(G_FILE_INFO(self->obj), attribute, G_OBJECT(attr_value->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_clear_status(PyGObject *self)
{
    
    g_file_info_clear_status(G_FILE_INFO(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_file_type(PyGObject *self)
{
    gint ret;

    
    ret = g_file_info_get_file_type(G_FILE_INFO(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_FILE_TYPE, ret);
}

static PyObject *
_wrap_g_file_info_get_is_hidden(PyGObject *self)
{
    int ret;

    
    ret = g_file_info_get_is_hidden(G_FILE_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_get_is_backup(PyGObject *self)
{
    int ret;

    
    ret = g_file_info_get_is_backup(G_FILE_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_get_is_symlink(PyGObject *self)
{
    int ret;

    
    ret = g_file_info_get_is_symlink(G_FILE_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_info_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_name(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_display_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_display_name(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_edit_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_edit_name(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_icon(PyGObject *self)
{
    GIcon *ret;

    
    ret = g_file_info_get_icon(G_FILE_INFO(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_info_get_content_type(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_content_type(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_size(PyGObject *self)
{
    gint64 ret;

    
    ret = g_file_info_get_size(G_FILE_INFO(self->obj));
    
    return PyLong_FromLongLong(ret);
}

#line 79 "gfileinfo.override"
static PyObject *
_wrap_g_file_info_get_modification_time(PyGObject *self, PyObject *unused)
{
    GTimeVal timeval;

    g_file_info_get_modification_time(G_FILE_INFO(self->obj), &timeval);
    return pyglib_float_from_timeval(timeval);
}
#line 2387 "gio.c"


static PyObject *
_wrap_g_file_info_get_symlink_target(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_symlink_target(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_etag(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_file_info_get_etag(G_FILE_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_get_sort_order(PyGObject *self)
{
    int ret;

    
    ret = g_file_info_get_sort_order(G_FILE_INFO(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_file_info_set_attribute_mask(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mask", NULL };
    PyObject *py_mask;
    GFileAttributeMatcher *mask = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.FileInfo.set_attribute_mask", kwlist, &py_mask))
        return NULL;
    if (pyg_boxed_check(py_mask, G_TYPE_FILE_ATTRIBUTE_MATCHER))
        mask = pyg_boxed_get(py_mask, GFileAttributeMatcher);
    else {
        PyErr_SetString(PyExc_TypeError, "mask should be a GFileAttributeMatcher");
        return NULL;
    }
    
    g_file_info_set_attribute_mask(G_FILE_INFO(self->obj), mask);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_unset_attribute_mask(PyGObject *self)
{
    
    g_file_info_unset_attribute_mask(G_FILE_INFO(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_file_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    GFileType type;
    PyObject *py_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.FileInfo.set_file_type", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_FILE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    g_file_info_set_file_type(G_FILE_INFO(self->obj), type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_is_hidden(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "is_hidden", NULL };
    int is_hidden;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FileInfo.set_is_hidden", kwlist, &is_hidden))
        return NULL;
    
    g_file_info_set_is_hidden(G_FILE_INFO(self->obj), is_hidden);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_is_symlink(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "is_symlink", NULL };
    int is_symlink;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FileInfo.set_is_symlink", kwlist, &is_symlink))
        return NULL;
    
    g_file_info_set_is_symlink(G_FILE_INFO(self->obj), is_symlink);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.set_name", kwlist, &name))
        return NULL;
    
    g_file_info_set_name(G_FILE_INFO(self->obj), name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_display_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display_name", NULL };
    char *display_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.set_display_name", kwlist, &display_name))
        return NULL;
    
    g_file_info_set_display_name(G_FILE_INFO(self->obj), display_name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_edit_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "edit_name", NULL };
    char *edit_name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.set_edit_name", kwlist, &edit_name))
        return NULL;
    
    g_file_info_set_edit_name(G_FILE_INFO(self->obj), edit_name);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_icon(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon", NULL };
    PyGObject *icon;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileInfo.set_icon", kwlist, &PyGIcon_Type, &icon))
        return NULL;
    
    g_file_info_set_icon(G_FILE_INFO(self->obj), G_ICON(icon->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_content_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", NULL };
    char *content_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.set_content_type", kwlist, &content_type))
        return NULL;
    
    g_file_info_set_content_type(G_FILE_INFO(self->obj), content_type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    gint64 size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L:gio.FileInfo.set_size", kwlist, &size))
        return NULL;
    
    g_file_info_set_size(G_FILE_INFO(self->obj), size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 89 "gfileinfo.override"
static PyObject *
_wrap_g_file_info_set_modification_time(PyGObject *self, 
                                        PyObject  *args, 
                                        PyObject  *kwargs)
{
    char *kwlist[] = { "mtime", NULL};
    double py_mtime = 0.0;
    GTimeVal ttime, *mtime;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "d:gio.FileInfo.set_modification_time",
                                     kwlist, &py_mtime))
        return NULL;

    if (py_mtime > 0.0) {
	ttime.tv_sec = (glong) py_mtime;
	ttime.tv_usec = (glong)((py_mtime - ttime.tv_sec) * G_USEC_PER_SEC);
        mtime = &ttime;
    } else if (py_mtime == 0.0) {
	mtime = NULL;
    } else {
        PyErr_SetString(PyExc_ValueError, "mtime must be >= 0.0");
        return NULL;
    }
    
    g_file_info_set_modification_time(G_FILE_INFO(self->obj), mtime);
    
    Py_INCREF(Py_None);
    return Py_None;
}

/* GFileInfo.get_attribute_data: No ArgType for GFileAttributeType* */
/* GFileInfo.set_attribute: No ArgType for gpointer */
#line 2633 "gio.c"


static PyObject *
_wrap_g_file_info_set_symlink_target(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "symlink_target", NULL };
    char *symlink_target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.FileInfo.set_symlink_target", kwlist, &symlink_target))
        return NULL;
    
    g_file_info_set_symlink_target(G_FILE_INFO(self->obj), symlink_target);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_info_set_sort_order(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sort_order", NULL };
    int sort_order;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FileInfo.set_sort_order", kwlist, &sort_order))
        return NULL;
    
    g_file_info_set_sort_order(G_FILE_INFO(self->obj), sort_order);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFileInfo_methods[] = {
    { "dup", (PyCFunction)_wrap_g_file_info_dup, METH_NOARGS,
      NULL },
    { "copy_into", (PyCFunction)_wrap_g_file_info_copy_into, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_attribute", (PyCFunction)_wrap_g_file_info_has_attribute, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_namespace", (PyCFunction)_wrap_g_file_info_has_namespace, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "list_attributes", (PyCFunction)_wrap_g_file_info_list_attributes, METH_VARARGS|METH_KEYWORDS,
      (char *) "INFO.list_attributes(name_space) -> Attribute list\n\n"
"Lists the file info structure's attributes." },
    { "get_attribute_type", (PyCFunction)_wrap_g_file_info_get_attribute_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_attribute", (PyCFunction)_wrap_g_file_info_remove_attribute, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_status", (PyCFunction)_wrap_g_file_info_get_attribute_status, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_status", (PyCFunction)_wrap_g_file_info_set_attribute_status, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_as_string", (PyCFunction)_wrap_g_file_info_get_attribute_as_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_string", (PyCFunction)_wrap_g_file_info_get_attribute_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_byte_string", (PyCFunction)_wrap_g_file_info_get_attribute_byte_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_boolean", (PyCFunction)_wrap_g_file_info_get_attribute_boolean, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_uint32", (PyCFunction)_wrap_g_file_info_get_attribute_uint32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_int32", (PyCFunction)_wrap_g_file_info_get_attribute_int32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_uint64", (PyCFunction)_wrap_g_file_info_get_attribute_uint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_int64", (PyCFunction)_wrap_g_file_info_get_attribute_int64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_attribute_object", (PyCFunction)_wrap_g_file_info_get_attribute_object, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_string", (PyCFunction)_wrap_g_file_info_set_attribute_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_byte_string", (PyCFunction)_wrap_g_file_info_set_attribute_byte_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_boolean", (PyCFunction)_wrap_g_file_info_set_attribute_boolean, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_uint32", (PyCFunction)_wrap_g_file_info_set_attribute_uint32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_int32", (PyCFunction)_wrap_g_file_info_set_attribute_int32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_uint64", (PyCFunction)_wrap_g_file_info_set_attribute_uint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_int64", (PyCFunction)_wrap_g_file_info_set_attribute_int64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_object", (PyCFunction)_wrap_g_file_info_set_attribute_object, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_status", (PyCFunction)_wrap_g_file_info_clear_status, METH_NOARGS,
      NULL },
    { "get_file_type", (PyCFunction)_wrap_g_file_info_get_file_type, METH_NOARGS,
      NULL },
    { "get_is_hidden", (PyCFunction)_wrap_g_file_info_get_is_hidden, METH_NOARGS,
      NULL },
    { "get_is_backup", (PyCFunction)_wrap_g_file_info_get_is_backup, METH_NOARGS,
      NULL },
    { "get_is_symlink", (PyCFunction)_wrap_g_file_info_get_is_symlink, METH_NOARGS,
      NULL },
    { "get_name", (PyCFunction)_wrap_g_file_info_get_name, METH_NOARGS,
      NULL },
    { "get_display_name", (PyCFunction)_wrap_g_file_info_get_display_name, METH_NOARGS,
      NULL },
    { "get_edit_name", (PyCFunction)_wrap_g_file_info_get_edit_name, METH_NOARGS,
      NULL },
    { "get_icon", (PyCFunction)_wrap_g_file_info_get_icon, METH_NOARGS,
      NULL },
    { "get_content_type", (PyCFunction)_wrap_g_file_info_get_content_type, METH_NOARGS,
      NULL },
    { "get_size", (PyCFunction)_wrap_g_file_info_get_size, METH_NOARGS,
      NULL },
    { "get_modification_time", (PyCFunction)_wrap_g_file_info_get_modification_time, METH_NOARGS,
      (char *) "INFO.get_modification_time() -> modification time\n"
"Returns the modification time, in UNIX time format\n" },
    { "get_symlink_target", (PyCFunction)_wrap_g_file_info_get_symlink_target, METH_NOARGS,
      NULL },
    { "get_etag", (PyCFunction)_wrap_g_file_info_get_etag, METH_NOARGS,
      NULL },
    { "get_sort_order", (PyCFunction)_wrap_g_file_info_get_sort_order, METH_NOARGS,
      NULL },
    { "set_attribute_mask", (PyCFunction)_wrap_g_file_info_set_attribute_mask, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unset_attribute_mask", (PyCFunction)_wrap_g_file_info_unset_attribute_mask, METH_NOARGS,
      NULL },
    { "set_file_type", (PyCFunction)_wrap_g_file_info_set_file_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_is_hidden", (PyCFunction)_wrap_g_file_info_set_is_hidden, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_is_symlink", (PyCFunction)_wrap_g_file_info_set_is_symlink, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_name", (PyCFunction)_wrap_g_file_info_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_display_name", (PyCFunction)_wrap_g_file_info_set_display_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_edit_name", (PyCFunction)_wrap_g_file_info_set_edit_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_icon", (PyCFunction)_wrap_g_file_info_set_icon, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_content_type", (PyCFunction)_wrap_g_file_info_set_content_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_size", (PyCFunction)_wrap_g_file_info_set_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_modification_time", (PyCFunction)_wrap_g_file_info_set_modification_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_symlink_target", (PyCFunction)_wrap_g_file_info_set_symlink_target, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_sort_order", (PyCFunction)_wrap_g_file_info_set_sort_order, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileInfo_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileInfo",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileInfo_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_file_info_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GFileMonitor ----------- */

static PyObject *
_wrap_g_file_monitor_cancel(PyGObject *self)
{
    int ret;

    
    ret = g_file_monitor_cancel(G_FILE_MONITOR(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_monitor_is_cancelled(PyGObject *self)
{
    int ret;

    
    ret = g_file_monitor_is_cancelled(G_FILE_MONITOR(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_monitor_set_rate_limit(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "limit_msecs", NULL };
    int limit_msecs;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FileMonitor.set_rate_limit", kwlist, &limit_msecs))
        return NULL;
    
    g_file_monitor_set_rate_limit(G_FILE_MONITOR(self->obj), limit_msecs);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_monitor_emit_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "file", "other_file", "event_type", NULL };
    PyGObject *file, *other_file;
    PyObject *py_event_type = NULL;
    GFileMonitorEvent event_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:gio.FileMonitor.emit_event", kwlist, &PyGFile_Type, &file, &PyGFile_Type, &other_file, &py_event_type))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_FILE_MONITOR_EVENT, py_event_type, (gpointer)&event_type))
        return NULL;
    
    g_file_monitor_emit_event(G_FILE_MONITOR(self->obj), G_FILE(file->obj), G_FILE(other_file->obj), event_type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFileMonitor_methods[] = {
    { "cancel", (PyCFunction)_wrap_g_file_monitor_cancel, METH_NOARGS,
      NULL },
    { "is_cancelled", (PyCFunction)_wrap_g_file_monitor_is_cancelled, METH_NOARGS,
      NULL },
    { "set_rate_limit", (PyCFunction)_wrap_g_file_monitor_set_rate_limit, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "emit_event", (PyCFunction)_wrap_g_file_monitor_emit_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileMonitor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileMonitor",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileMonitor_methods, /* tp_methods */
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



/* ----------- GInputStream ----------- */

#line 28 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_read(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", "cancellable", NULL };
    PyGObject *pycancellable = NULL;
    PyObject *v;
    GCancellable *cancellable;
    long count = -1;
    GError *error = NULL;
    size_t bytesread, buffersize, chunksize;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|lO:InputStream.read",
                                     kwlist, &count,
                                     &pycancellable))
        return NULL;

    buffersize = (count < 0 ? BUFSIZE : count);

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    v = PyString_FromStringAndSize((char *)NULL, buffersize);
    if (v == NULL)
        return NULL;

    bytesread = 0;
    for (;;)
        {
            pyg_begin_allow_threads;
            errno = 0;
            chunksize = g_input_stream_read(G_INPUT_STREAM(self->obj),
                                            PyString_AS_STRING((PyStringObject *)v) + bytesread,
                                            buffersize - bytesread, cancellable,
                                            &error);
            pyg_end_allow_threads;

            if (pyg_error_check(&error)) {
		Py_DECREF(v);
		return NULL;
	    }
	    if (chunksize == 0) {
		/* End of file. */
                break;
	    }

            bytesread += chunksize;
            if (bytesread < buffersize) {
		/* g_input_stream_read() decided to not read full buffer.  We
		 * then return early too, even if 'count' is less than 0.
		 */
                break;
	    }

            if (count < 0) {
		buffersize += BUFSIZE;
		if (_PyString_Resize(&v, buffersize) < 0)
		    return NULL;
	    }
            else {
                /* Got what was requested. */
                break;
	    }
        }

    if (bytesread != buffersize)
        _PyString_Resize(&v, bytesread);

    return v;
}
#line 3021 "gio.c"


#line 100 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_read_all(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", "cancellable", NULL };
    PyGObject *pycancellable = NULL;
    PyObject *v;
    GCancellable *cancellable;
    long count = -1;
    GError *error = NULL;
    size_t bytesread, buffersize, chunksize;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|lO:InputStream.read",
                                     kwlist, &count,
                                     &pycancellable))
        return NULL;

    buffersize = (count < 0 ? BUFSIZE : count);

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    v = PyString_FromStringAndSize((char *)NULL, buffersize);
    if (v == NULL)
        return NULL;

    bytesread = 0;
    for (;;)
        {
            pyg_begin_allow_threads;
            errno = 0;
            g_input_stream_read_all(G_INPUT_STREAM(self->obj),
				    PyString_AS_STRING((PyStringObject *)v) + bytesread,
				    buffersize - bytesread,
				    &chunksize,
				    cancellable, &error);
            pyg_end_allow_threads;

            if (pyg_error_check(&error)) {
		Py_DECREF(v);
		return NULL;
	    }

            bytesread += chunksize;
            if (bytesread < buffersize || chunksize == 0) {
		/* End of file. */
                break;
	    }

            if (count < 0) {
		buffersize += BUFSIZE;
		if (_PyString_Resize(&v, buffersize) < 0)
		    return NULL;
	    }
            else {
                /* Got what was requested. */
                break;
	    }
        }

    if (bytesread != buffersize)
        _PyString_Resize(&v, bytesread);

    return v;
}
#line 3090 "gio.c"


static PyObject *
_wrap_g_input_stream_skip(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    gsize count;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    gssize ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k|O:gio.InputStream.skip", kwlist, &count, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_input_stream_skip(G_INPUT_STREAM(self->obj), count, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_input_stream_close(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.InputStream.close", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_input_stream_close(G_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 167 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_read_async(PyGObject *self,
                                PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "count", "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    long count = -1;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "lO|iOO:InputStream.read_async",
                                     kwlist,
                                     &count,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    if (!pygio_notify_allocate_buffer(notify, count))
        goto error;

    pygio_notify_reference_callback(notify);
    pygio_notify_attach_to_result(notify);

    g_input_stream_read_async(G_INPUT_STREAM(self->obj),
                              notify->buffer,
                              notify->buffer_size,
                              io_priority,
                              cancellable,
                              (GAsyncReadyCallback) async_result_callback_marshal,
                              notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3203 "gio.c"


#line 221 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_read_finish(PyGObject *self,
                                 PyObject *args,
                                 PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    Py_ssize_t bytesread;
    PyGIONotify *notify;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.InputStream.read_finish",
                                     kwlist, &PyGAsyncResult_Type, &result))
        return NULL;

    bytesread = g_input_stream_read_finish(G_INPUT_STREAM(self->obj),
                                           G_ASYNC_RESULT(result->obj), &error);

    if (pyg_error_check(&error))
        return NULL;

    if (bytesread == 0)
        return PyString_FromString("");

    notify = pygio_notify_get_attached(result);
    return PyString_FromStringAndSize(notify->buffer, bytesread);
}
#line 3235 "gio.c"


#line 297 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_skip_async(PyGObject *self,
                                PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "count", "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    long count = -1;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "lO|iOO:InputStream.skip_async",
                                     kwlist,
                                     &count,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);
    

    g_input_stream_skip_async(G_INPUT_STREAM(self->obj),
                              count,
                              io_priority,
                              cancellable,
                              (GAsyncReadyCallback) async_result_callback_marshal,
                              notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3287 "gio.c"


static PyObject *
_wrap_g_input_stream_skip_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gssize ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.InputStream.skip_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_input_stream_skip_finish(G_INPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

#line 251 "ginputstream.override"
static PyObject *
_wrap_g_input_stream_close_async(PyGObject *self,
                                 PyObject *args,
                                 PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority", "cancellable",
                              "user_data", NULL };
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:InputStream.close_async",
                                     kwlist,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_input_stream_close_async(G_INPUT_STREAM(self->obj),
                               io_priority,
                               cancellable,
                               (GAsyncReadyCallback)async_result_callback_marshal,
                               notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3354 "gio.c"


static PyObject *
_wrap_g_input_stream_close_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.InputStream.close_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_input_stream_close_finish(G_INPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_input_stream_is_closed(PyGObject *self)
{
    int ret;

    
    ret = g_input_stream_is_closed(G_INPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_input_stream_has_pending(PyGObject *self)
{
    int ret;

    
    ret = g_input_stream_has_pending(G_INPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_input_stream_set_pending(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_input_stream_set_pending(G_INPUT_STREAM(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_input_stream_clear_pending(PyGObject *self)
{
    
    g_input_stream_clear_pending(G_INPUT_STREAM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGInputStream_methods[] = {
    { "read_part", (PyCFunction)_wrap_g_input_stream_read, METH_VARARGS|METH_KEYWORDS,
      (char *) "STREAM.read_part([count, [cancellable]]) -> string\n"
"\n"
"Read 'count' bytes from the stream. If 'count' is not specified or is\n"
"omitted, read until the end of the stream. This method is allowed to\n"
"stop at any time after reading at least 1 byte from the stream. E.g.\n"
"when reading over a (relatively slow) HTTP connection, it will often\n"
"stop after receiving one packet. Therefore, to reliably read requested\n"
"number of bytes, you need to use a loop. See also gio.InputStream.read\n"
"for easier to use (though less efficient) method.\n"
"\n"
"Note: this method roughly corresponds to C GIO g_input_stream_read." },
    { "read", (PyCFunction)_wrap_g_input_stream_read_all, METH_VARARGS|METH_KEYWORDS,
      (char *) "STREAM.read([count, [cancellable]]) -> string\n"
"\n"
"Read 'count' bytes from the stream. If 'count' is not specified or is\n"
"omitted, read until the end of the stream. This method will stop only\n"
"after reading requested number of bytes, reaching end of stream or\n"
"triggering an I/O error. See also gio.InputStream.read_part for more\n"
"efficient, but more cumbersome to use method.\n"
"\n"
"Note: this method roughly corresponds to C GIO g_input_stream_read_all.\n"
"It was renamed for consistency with Python standard file.read." },
    { "skip", (PyCFunction)_wrap_g_input_stream_skip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close", (PyCFunction)_wrap_g_input_stream_close, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_async", (PyCFunction)_wrap_g_input_stream_read_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_finish", (PyCFunction)_wrap_g_input_stream_read_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skip_async", (PyCFunction)_wrap_g_input_stream_skip_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skip_finish", (PyCFunction)_wrap_g_input_stream_skip_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_async", (PyCFunction)_wrap_g_input_stream_close_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_finish", (PyCFunction)_wrap_g_input_stream_close_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_closed", (PyCFunction)_wrap_g_input_stream_is_closed, METH_NOARGS,
      NULL },
    { "has_pending", (PyCFunction)_wrap_g_input_stream_has_pending, METH_NOARGS,
      NULL },
    { "set_pending", (PyCFunction)_wrap_g_input_stream_set_pending, METH_NOARGS,
      NULL },
    { "clear_pending", (PyCFunction)_wrap_g_input_stream_clear_pending, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.InputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGInputStream_methods, /* tp_methods */
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



/* ----------- GFileInputStream ----------- */

static PyObject *
_wrap_g_file_input_stream_query_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "cancellable", NULL };
    char *attributes;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.FileInputStream.query_info", kwlist, &attributes, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_input_stream_query_info(G_FILE_INPUT_STREAM(self->obj), attributes, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 25 "gfileinputstream.override"
static PyObject *
_wrap_g_file_input_stream_query_info_async(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *attributes;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "sO|iOO:gio.FileInputStream.query_info_async",
                                kwlist,
                                &attributes,
                                &notify->callback,
                                &io_priority,
                                &pycancellable,
                                &notify->data))

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_input_stream_query_info_async(G_FILE_INPUT_STREAM(self->obj),
                         attributes, io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3599 "gio.c"


static PyObject *
_wrap_g_file_input_stream_query_info_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileInputStream.query_info_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_input_stream_query_info_finish(G_FILE_INPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGFileInputStream_methods[] = {
    { "query_info", (PyCFunction)_wrap_g_file_input_stream_query_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_async", (PyCFunction)_wrap_g_file_input_stream_query_info_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_finish", (PyCFunction)_wrap_g_file_input_stream_query_info_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileInputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileInputStream_methods, /* tp_methods */
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



/* ----------- GIOStream ----------- */

static PyObject *
_wrap_g_io_stream_get_input_stream(PyGObject *self)
{
    GInputStream *ret;

    
    ret = g_io_stream_get_input_stream(G_IO_STREAM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_io_stream_get_output_stream(PyGObject *self)
{
    GOutputStream *ret;

    
    ret = g_io_stream_get_output_stream(G_IO_STREAM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_io_stream_close(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.IOStream.close", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_io_stream_close(G_IO_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 25 "giostream.override"
static PyObject *
_wrap_g_io_stream_close_async(PyGObject *self,
                              PyObject *args,
                              PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority", "cancellable",
                              "user_data", NULL };
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:IOStream.close_async",
                                     kwlist,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_io_stream_close_async(G_IO_STREAM(self->obj),
                            io_priority,
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3777 "gio.c"


static PyObject *
_wrap_g_io_stream_close_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.IOStream.close_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_io_stream_close_finish(G_IO_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_io_stream_is_closed(PyGObject *self)
{
    int ret;

    
    ret = g_io_stream_is_closed(G_IO_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_io_stream_has_pending(PyGObject *self)
{
    int ret;

    
    ret = g_io_stream_has_pending(G_IO_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_io_stream_set_pending(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_io_stream_set_pending(G_IO_STREAM(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_io_stream_clear_pending(PyGObject *self)
{
    
    g_io_stream_clear_pending(G_IO_STREAM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGIOStream_methods[] = {
    { "get_input_stream", (PyCFunction)_wrap_g_io_stream_get_input_stream, METH_NOARGS,
      NULL },
    { "get_output_stream", (PyCFunction)_wrap_g_io_stream_get_output_stream, METH_NOARGS,
      NULL },
    { "close", (PyCFunction)_wrap_g_io_stream_close, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_async", (PyCFunction)_wrap_g_io_stream_close_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_finish", (PyCFunction)_wrap_g_io_stream_close_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_closed", (PyCFunction)_wrap_g_io_stream_is_closed, METH_NOARGS,
      NULL },
    { "has_pending", (PyCFunction)_wrap_g_io_stream_has_pending, METH_NOARGS,
      NULL },
    { "set_pending", (PyCFunction)_wrap_g_io_stream_set_pending, METH_NOARGS,
      NULL },
    { "clear_pending", (PyCFunction)_wrap_g_io_stream_clear_pending, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGIOStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.IOStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGIOStream_methods, /* tp_methods */
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



/* ----------- GFileIOStream ----------- */

static PyObject *
_wrap_g_file_io_stream_query_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "cancellable", NULL };
    char *attributes;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.FileIOStream.query_info", kwlist, &attributes, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_io_stream_query_info(G_FILE_IO_STREAM(self->obj), attributes, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 25 "gfileiostream.override"
static PyObject *
_wrap_g_file_io_stream_query_info_async(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *attributes;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "sO|iOO:gio.FileIOStream.query_info_async",
                                kwlist,
                                &attributes,
                                &notify->callback,
                                &io_priority,
                                &pycancellable,
                                &notify->data))

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_io_stream_query_info_async(G_FILE_IO_STREAM(self->obj),
                         attributes, io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 3993 "gio.c"


static PyObject *
_wrap_g_file_io_stream_query_info_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileIOStream.query_info_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_io_stream_query_info_finish(G_FILE_IO_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_io_stream_get_etag(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_io_stream_get_etag(G_FILE_IO_STREAM(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFileIOStream_methods[] = {
    { "query_info", (PyCFunction)_wrap_g_file_io_stream_query_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_async", (PyCFunction)_wrap_g_file_io_stream_query_info_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_finish", (PyCFunction)_wrap_g_file_io_stream_query_info_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_etag", (PyCFunction)_wrap_g_file_io_stream_get_etag, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileIOStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileIOStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileIOStream_methods, /* tp_methods */
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



/* ----------- GFilterInputStream ----------- */

static PyObject *
_wrap_g_filter_input_stream_get_base_stream(PyGObject *self)
{
    GInputStream *ret;

    
    ret = g_filter_input_stream_get_base_stream(G_FILTER_INPUT_STREAM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_filter_input_stream_get_close_base_stream(PyGObject *self)
{
    int ret;

    
    ret = g_filter_input_stream_get_close_base_stream(G_FILTER_INPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_filter_input_stream_set_close_base_stream(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "close_base", NULL };
    int close_base;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FilterInputStream.set_close_base_stream", kwlist, &close_base))
        return NULL;
    
    g_filter_input_stream_set_close_base_stream(G_FILTER_INPUT_STREAM(self->obj), close_base);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFilterInputStream_methods[] = {
    { "get_base_stream", (PyCFunction)_wrap_g_filter_input_stream_get_base_stream, METH_NOARGS,
      NULL },
    { "get_close_base_stream", (PyCFunction)_wrap_g_filter_input_stream_get_close_base_stream, METH_NOARGS,
      NULL },
    { "set_close_base_stream", (PyCFunction)_wrap_g_filter_input_stream_set_close_base_stream, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFilterInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FilterInputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFilterInputStream_methods, /* tp_methods */
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



/* ----------- GBufferedInputStream ----------- */

static int
_wrap_g_buffered_input_stream_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"base_stream", NULL };
    char *prop_names[] = {"base_stream", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gio.BufferedInputStream.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.BufferedInputStream object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_buffered_input_stream_get_buffer_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_buffered_input_stream_get_buffer_size(G_BUFFERED_INPUT_STREAM(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_buffered_input_stream_set_buffer_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    gsize size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:gio.BufferedInputStream.set_buffer_size", kwlist, &size))
        return NULL;
    
    g_buffered_input_stream_set_buffer_size(G_BUFFERED_INPUT_STREAM(self->obj), size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_buffered_input_stream_get_available(PyGObject *self)
{
    gsize ret;

    
    ret = g_buffered_input_stream_get_available(G_BUFFERED_INPUT_STREAM(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_buffered_input_stream_fill(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "count", "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    gssize count, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"l|O:gio.BufferedInputStream.fill", kwlist, &count, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_buffered_input_stream_fill(G_BUFFERED_INPUT_STREAM(self->obj), count, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

#line 24 "gbufferedinputstream.override"
static PyObject *
_wrap_g_buffered_input_stream_fill_async(PyGObject *self,
                                         PyObject *args,
                                         PyObject *kwargs)
{
    static char *kwlist[] = { "count", "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    long count = -1;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "lO|iOO:gio.BufferedInputStream.fill_async",
                                    kwlist,
                                    &count,
                                    &notify->callback,
                                    &io_priority,
                                    &pycancellable,
                                    &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_buffered_input_stream_fill_async(G_BUFFERED_INPUT_STREAM(self->obj),
                            count,
                            io_priority,
                            cancellable,
                            (GAsyncReadyCallback) async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 4336 "gio.c"


static PyObject *
_wrap_g_buffered_input_stream_fill_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gssize ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.BufferedInputStream.fill_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_buffered_input_stream_fill_finish(G_BUFFERED_INPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_buffered_input_stream_read_byte(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.BufferedInputStream.read_byte", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_buffered_input_stream_read_byte(G_BUFFERED_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGBufferedInputStream_methods[] = {
    { "get_buffer_size", (PyCFunction)_wrap_g_buffered_input_stream_get_buffer_size, METH_NOARGS,
      NULL },
    { "set_buffer_size", (PyCFunction)_wrap_g_buffered_input_stream_set_buffer_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_available", (PyCFunction)_wrap_g_buffered_input_stream_get_available, METH_NOARGS,
      NULL },
    { "fill", (PyCFunction)_wrap_g_buffered_input_stream_fill, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fill_async", (PyCFunction)_wrap_g_buffered_input_stream_fill_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fill_finish", (PyCFunction)_wrap_g_buffered_input_stream_fill_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_byte", (PyCFunction)_wrap_g_buffered_input_stream_read_byte, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGBufferedInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.BufferedInputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGBufferedInputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_buffered_input_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GDataInputStream ----------- */

 static int
_wrap_g_data_input_stream_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"base_stream", NULL };
    char *prop_names[] = {"base_stream", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gio.DataInputStream.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.DataInputStream object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_data_input_stream_set_byte_order(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "order", NULL };
    PyObject *py_order = NULL;
    GDataStreamByteOrder order;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.DataInputStream.set_byte_order", kwlist, &py_order))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_DATA_STREAM_BYTE_ORDER, py_order, (gpointer)&order))
        return NULL;
    
    g_data_input_stream_set_byte_order(G_DATA_INPUT_STREAM(self->obj), order);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_data_input_stream_get_byte_order(PyGObject *self)
{
    gint ret;

    
    ret = g_data_input_stream_get_byte_order(G_DATA_INPUT_STREAM(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_DATA_STREAM_BYTE_ORDER, ret);
}

static PyObject *
_wrap_g_data_input_stream_set_newline_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GDataStreamNewlineType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.DataInputStream.set_newline_type", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_DATA_STREAM_NEWLINE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    g_data_input_stream_set_newline_type(G_DATA_INPUT_STREAM(self->obj), type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_data_input_stream_get_newline_type(PyGObject *self)
{
    gint ret;

    
    ret = g_data_input_stream_get_newline_type(G_DATA_INPUT_STREAM(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_DATA_STREAM_NEWLINE_TYPE, ret);
}

static PyObject *
_wrap_g_data_input_stream_read_byte(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    gchar ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_byte", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_byte(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyString_FromStringAndSize(&ret, 1);
}

static PyObject *
_wrap_g_data_input_stream_read_int16(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_int16", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_int16(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_data_input_stream_read_uint16(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_uint16", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_uint16(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_data_input_stream_read_int32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_int32", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_int32(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_data_input_stream_read_uint32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    guint32 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_uint32", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_uint32(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_g_data_input_stream_read_int64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    gint64 ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_int64", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_int64(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_g_data_input_stream_read_uint64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.DataInputStream.read_uint64", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_input_stream_read_uint64(G_DATA_INPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromUnsignedLongLong(ret);
}

#line 26 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_line(PyGObject *self,
				    PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    char *line;
    gsize length;
    PyObject *py_line;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|O:gio.DataInputStream.read_line",
                                     kwlist, &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;

    line = g_data_input_stream_read_line(G_DATA_INPUT_STREAM(self->obj),
					 &length, cancellable, &error);
    if (pyg_error_check(&error))
        return NULL;

    py_line = PyString_FromStringAndSize(line, length);
    g_free(line);
    return py_line;
}

#line 4762 "gio.c"


#line 138 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_until(PyGObject *self,
				     PyObject *args,
				     PyObject *kwargs)
{
    static char *kwlist[] = { "stop_chars", "cancellable", NULL };
    const char *stop_chars;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    char *line;
    gsize length;
    PyObject *py_line;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s|O:gio.DataInputStream.read_line",
                                     kwlist, &stop_chars, &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;

    line = g_data_input_stream_read_until(G_DATA_INPUT_STREAM(self->obj),
					  stop_chars, &length, cancellable, &error);
    if (pyg_error_check(&error))
        return NULL;

    py_line = PyString_FromStringAndSize(line, length);
    g_free(line);
    return py_line;
}

#line 4798 "gio.c"


#line 172 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_until_async(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "stop_chars", "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    const char *stop_chars;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "sO|iOO:gio.DataInputStream.read_until_async",
                                     kwlist,
                                     &stop_chars,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_data_input_stream_read_until_async(G_DATA_INPUT_STREAM(self->obj),
                                         stop_chars,
                                         io_priority,
                                         cancellable,
                                         (GAsyncReadyCallback) async_result_callback_marshal,
                                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}

#line 4850 "gio.c"


#line 222 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_until_finish(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gchar *line;
    gsize length;
    PyObject *py_line;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.DataInputStream.read_until_finish",
                                     kwlist, &PyGAsyncResult_Type, &result))
        return NULL;

    line = g_data_input_stream_read_until_finish(G_DATA_INPUT_STREAM(self->obj),
                                                 G_ASYNC_RESULT(result->obj),
                                                 &length,
                                                 &error);

    if (pyg_error_check(&error))
        return NULL;

    py_line = PyString_FromStringAndSize(line, length);
    g_free(line);
    return py_line;
}
#line 4883 "gio.c"


#line 59 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_line_async(PyGObject *self,
				          PyObject *args,
				          PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:gio.DataInputStream.read_line_async",
                                     kwlist,
                                     &notify->callback,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_data_input_stream_read_line_async(G_DATA_INPUT_STREAM(self->obj),
                                        io_priority,
                                        cancellable,
                                        (GAsyncReadyCallback) async_result_callback_marshal,
                                        notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}

#line 4932 "gio.c"


#line 106 "gdatainputstream.override"
static PyObject *
_wrap_g_data_input_stream_read_line_finish(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gchar *line;
    gsize length;
    PyObject *py_line;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.DataInputStream.read_line_finish",
                                     kwlist, &PyGAsyncResult_Type, &result))
        return NULL;

    line = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(self->obj),
                                                G_ASYNC_RESULT(result->obj),
                                                &length,
                                                &error);

    if (pyg_error_check(&error))
        return NULL;

    py_line = PyString_FromStringAndSize(line, length);
    g_free(line);
    return py_line;
}

#line 4966 "gio.c"


static const PyMethodDef _PyGDataInputStream_methods[] = {
    { "set_byte_order", (PyCFunction)_wrap_g_data_input_stream_set_byte_order, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_byte_order", (PyCFunction)_wrap_g_data_input_stream_get_byte_order, METH_NOARGS,
      NULL },
    { "set_newline_type", (PyCFunction)_wrap_g_data_input_stream_set_newline_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_newline_type", (PyCFunction)_wrap_g_data_input_stream_get_newline_type, METH_NOARGS,
      NULL },
    { "read_byte", (PyCFunction)_wrap_g_data_input_stream_read_byte, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_int16", (PyCFunction)_wrap_g_data_input_stream_read_int16, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_uint16", (PyCFunction)_wrap_g_data_input_stream_read_uint16, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_int32", (PyCFunction)_wrap_g_data_input_stream_read_int32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_uint32", (PyCFunction)_wrap_g_data_input_stream_read_uint32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_int64", (PyCFunction)_wrap_g_data_input_stream_read_int64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_uint64", (PyCFunction)_wrap_g_data_input_stream_read_uint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_line", (PyCFunction)_wrap_g_data_input_stream_read_line, METH_VARARGS|METH_KEYWORDS,
      (char *) "S.read_line([cancellable]) -> str\n"
"Read a line from the stream. Return value includes ending newline\n"
"character." },
    { "read_until", (PyCFunction)_wrap_g_data_input_stream_read_until, METH_VARARGS|METH_KEYWORDS,
      (char *) "S.read_until(stop_chars, [cancellable]) -> str\n"
"Read characters from the string, stopping at the end or upon reading\n"
"any character in stop_chars. Return value does not include the stopping\n"
"character." },
    { "read_until_async", (PyCFunction)_wrap_g_data_input_stream_read_until_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_until_finish", (PyCFunction)_wrap_g_data_input_stream_read_until_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_line_async", (PyCFunction)_wrap_g_data_input_stream_read_line_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read_line_finish", (PyCFunction)_wrap_g_data_input_stream_read_line_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGDataInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.DataInputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGDataInputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_data_input_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GMemoryInputStream ----------- */

 static int
_wrap_g_memory_input_stream_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.MemoryInputStream.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.MemoryInputStream object");
        return -1;
    }
    return 0;
}

#line 25 "gmemoryinputstream.override"
static PyObject *
_wrap_g_memory_input_stream_add_data(PyGObject *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject *data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gio.MemoryInputStream.add_data",
                                     kwlist, &data))
        return NULL;

    if (data != Py_None) {
        char *copy;
        int length;

        if (!PyString_Check(data)) {
            PyErr_SetString(PyExc_TypeError, "data must be a string or None");
            return NULL;
        }

        length = PyString_Size(data);
        copy = g_malloc(length);
        memcpy(copy, PyString_AsString(data), length);

        g_memory_input_stream_add_data(G_MEMORY_INPUT_STREAM(self->obj),
                                       copy, length, (GDestroyNotify) g_free);
    }

    Py_INCREF(Py_None);
    return Py_None;
}
#line 5114 "gio.c"


static const PyMethodDef _PyGMemoryInputStream_methods[] = {
    { "add_data", (PyCFunction)_wrap_g_memory_input_stream_add_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGMemoryInputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.MemoryInputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGMemoryInputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_memory_input_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GMountOperation ----------- */

static int
_wrap_g_mount_operation_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.MountOperation.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.MountOperation object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_mount_operation_get_username(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_mount_operation_get_username(G_MOUNT_OPERATION(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_set_username(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "username", NULL };
    char *username;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.MountOperation.set_username", kwlist, &username))
        return NULL;
    
    g_mount_operation_set_username(G_MOUNT_OPERATION(self->obj), username);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_get_password(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_mount_operation_get_password(G_MOUNT_OPERATION(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_set_password(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "password", NULL };
    char *password;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.MountOperation.set_password", kwlist, &password))
        return NULL;
    
    g_mount_operation_set_password(G_MOUNT_OPERATION(self->obj), password);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_get_anonymous(PyGObject *self)
{
    int ret;

    
    ret = g_mount_operation_get_anonymous(G_MOUNT_OPERATION(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_mount_operation_set_anonymous(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "anonymous", NULL };
    int anonymous;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.MountOperation.set_anonymous", kwlist, &anonymous))
        return NULL;
    
    g_mount_operation_set_anonymous(G_MOUNT_OPERATION(self->obj), anonymous);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_get_domain(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_mount_operation_get_domain(G_MOUNT_OPERATION(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_set_domain(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "domain", NULL };
    char *domain;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.MountOperation.set_domain", kwlist, &domain))
        return NULL;
    
    g_mount_operation_set_domain(G_MOUNT_OPERATION(self->obj), domain);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_get_password_save(PyGObject *self)
{
    gint ret;

    
    ret = g_mount_operation_get_password_save(G_MOUNT_OPERATION(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_PASSWORD_SAVE, ret);
}

static PyObject *
_wrap_g_mount_operation_set_password_save(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "save", NULL };
    PyObject *py_save = NULL;
    GPasswordSave save;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.MountOperation.set_password_save", kwlist, &py_save))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_PASSWORD_SAVE, py_save, (gpointer)&save))
        return NULL;
    
    g_mount_operation_set_password_save(G_MOUNT_OPERATION(self->obj), save);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_get_choice(PyGObject *self)
{
    int ret;

    
    ret = g_mount_operation_get_choice(G_MOUNT_OPERATION(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_mount_operation_set_choice(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "choice", NULL };
    int choice;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.MountOperation.set_choice", kwlist, &choice))
        return NULL;
    
    g_mount_operation_set_choice(G_MOUNT_OPERATION(self->obj), choice);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_operation_reply(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyObject *py_result = NULL;
    GMountOperationResult result;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.MountOperation.reply", kwlist, &py_result))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_MOUNT_OPERATION_RESULT, py_result, (gpointer)&result))
        return NULL;
    
    g_mount_operation_reply(G_MOUNT_OPERATION(self->obj), result);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGMountOperation_methods[] = {
    { "get_username", (PyCFunction)_wrap_g_mount_operation_get_username, METH_NOARGS,
      NULL },
    { "set_username", (PyCFunction)_wrap_g_mount_operation_set_username, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_password", (PyCFunction)_wrap_g_mount_operation_get_password, METH_NOARGS,
      NULL },
    { "set_password", (PyCFunction)_wrap_g_mount_operation_set_password, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_anonymous", (PyCFunction)_wrap_g_mount_operation_get_anonymous, METH_NOARGS,
      NULL },
    { "set_anonymous", (PyCFunction)_wrap_g_mount_operation_set_anonymous, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_domain", (PyCFunction)_wrap_g_mount_operation_get_domain, METH_NOARGS,
      NULL },
    { "set_domain", (PyCFunction)_wrap_g_mount_operation_set_domain, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_password_save", (PyCFunction)_wrap_g_mount_operation_get_password_save, METH_NOARGS,
      NULL },
    { "set_password_save", (PyCFunction)_wrap_g_mount_operation_set_password_save, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_choice", (PyCFunction)_wrap_g_mount_operation_get_choice, METH_NOARGS,
      NULL },
    { "set_choice", (PyCFunction)_wrap_g_mount_operation_set_choice, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "reply", (PyCFunction)_wrap_g_mount_operation_reply, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGMountOperation_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.MountOperation",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGMountOperation_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_mount_operation_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GInetAddress ----------- */

static PyObject *
_wrap_g_inet_address_to_string(PyGObject *self)
{
    gchar *ret;

    
    ret = g_inet_address_to_string(G_INET_ADDRESS(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_inet_address_get_native_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_inet_address_get_native_size(G_INET_ADDRESS(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_family(PyGObject *self)
{
    gint ret;

    
    ret = g_inet_address_get_family(G_INET_ADDRESS(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_FAMILY, ret);
}

static PyObject *
_wrap_g_inet_address_get_is_any(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_any(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_loopback(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_loopback(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_link_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_link_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_site_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_site_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_multicast(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_multicast(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_mc_global(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_mc_global(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_mc_link_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_mc_link_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_mc_node_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_mc_node_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_mc_org_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_mc_org_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_inet_address_get_is_mc_site_local(PyGObject *self)
{
    int ret;

    
    ret = g_inet_address_get_is_mc_site_local(G_INET_ADDRESS(self->obj));
    
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGInetAddress_methods[] = {
    { "to_string", (PyCFunction)_wrap_g_inet_address_to_string, METH_NOARGS,
      NULL },
    { "get_native_size", (PyCFunction)_wrap_g_inet_address_get_native_size, METH_NOARGS,
      NULL },
    { "get_family", (PyCFunction)_wrap_g_inet_address_get_family, METH_NOARGS,
      NULL },
    { "get_is_any", (PyCFunction)_wrap_g_inet_address_get_is_any, METH_NOARGS,
      NULL },
    { "get_is_loopback", (PyCFunction)_wrap_g_inet_address_get_is_loopback, METH_NOARGS,
      NULL },
    { "get_is_link_local", (PyCFunction)_wrap_g_inet_address_get_is_link_local, METH_NOARGS,
      NULL },
    { "get_is_site_local", (PyCFunction)_wrap_g_inet_address_get_is_site_local, METH_NOARGS,
      NULL },
    { "get_is_multicast", (PyCFunction)_wrap_g_inet_address_get_is_multicast, METH_NOARGS,
      NULL },
    { "get_is_mc_global", (PyCFunction)_wrap_g_inet_address_get_is_mc_global, METH_NOARGS,
      NULL },
    { "get_is_mc_link_local", (PyCFunction)_wrap_g_inet_address_get_is_mc_link_local, METH_NOARGS,
      NULL },
    { "get_is_mc_node_local", (PyCFunction)_wrap_g_inet_address_get_is_mc_node_local, METH_NOARGS,
      NULL },
    { "get_is_mc_org_local", (PyCFunction)_wrap_g_inet_address_get_is_mc_org_local, METH_NOARGS,
      NULL },
    { "get_is_mc_site_local", (PyCFunction)_wrap_g_inet_address_get_is_mc_site_local, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGInetAddress_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.InetAddress",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGInetAddress_methods, /* tp_methods */
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



/* ----------- GSocketAddress ----------- */

static PyObject *
_wrap_g_socket_address_get_family(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_address_get_family(G_SOCKET_ADDRESS(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_FAMILY, ret);
}

static PyObject *
_wrap_g_socket_address_get_native_size(PyGObject *self)
{
    gssize ret;

    
    ret = g_socket_address_get_native_size(G_SOCKET_ADDRESS(self->obj));
    
    return PyLong_FromLongLong(ret);

}

static const PyMethodDef _PyGSocketAddress_methods[] = {
    { "get_family", (PyCFunction)_wrap_g_socket_address_get_family, METH_NOARGS,
      NULL },
    { "get_native_size", (PyCFunction)_wrap_g_socket_address_get_native_size, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketAddress_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketAddress",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketAddress_methods, /* tp_methods */
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



/* ----------- GInetSocketAddress ----------- */

static int
_wrap_g_inet_socket_address_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "port", NULL };
    PyGObject *address;
    int port;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:gio.InetSocketAddress.__init__", kwlist, &PyGInetAddress_Type, &address, &port))
        return -1;
    self->obj = (GObject *)g_inet_socket_address_new(G_INET_ADDRESS(address->obj), port);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GInetSocketAddress object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_inet_socket_address_get_address(PyGObject *self)
{
    GInetAddress *ret;

    
    ret = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_inet_socket_address_get_port(PyGObject *self)
{
    int ret;

    
    ret = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(self->obj));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGInetSocketAddress_methods[] = {
    { "get_address", (PyCFunction)_wrap_g_inet_socket_address_get_address, METH_NOARGS,
      NULL },
    { "get_port", (PyCFunction)_wrap_g_inet_socket_address_get_port, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGInetSocketAddress_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.InetSocketAddress",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGInetSocketAddress_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_inet_socket_address_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GNetworkAddress ----------- */

static int
_wrap_g_network_address_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "hostname", "port", NULL };
    char *hostname;
    int port;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:gio.NetworkAddress.__init__", kwlist, &hostname, &port))
        return -1;
    self->obj = (GObject *)g_network_address_new(hostname, port);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GNetworkAddress object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_network_address_get_hostname(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_network_address_get_hostname(G_NETWORK_ADDRESS(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_network_address_get_port(PyGObject *self)
{
    int ret;

    
    ret = g_network_address_get_port(G_NETWORK_ADDRESS(self->obj));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGNetworkAddress_methods[] = {
    { "get_hostname", (PyCFunction)_wrap_g_network_address_get_hostname, METH_NOARGS,
      NULL },
    { "get_port", (PyCFunction)_wrap_g_network_address_get_port, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGNetworkAddress_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.NetworkAddress",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGNetworkAddress_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_network_address_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GNetworkService ----------- */

static int
_wrap_g_network_service_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "service", "protocol", "domain", NULL };
    char *service, *protocol, *domain;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sss:gio.NetworkService.__init__", kwlist, &service, &protocol, &domain))
        return -1;
    self->obj = (GObject *)g_network_service_new(service, protocol, domain);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GNetworkService object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_network_service_get_service(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_network_service_get_service(G_NETWORK_SERVICE(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_network_service_get_protocol(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_network_service_get_protocol(G_NETWORK_SERVICE(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_network_service_get_domain(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_network_service_get_domain(G_NETWORK_SERVICE(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGNetworkService_methods[] = {
    { "get_service", (PyCFunction)_wrap_g_network_service_get_service, METH_NOARGS,
      NULL },
    { "get_protocol", (PyCFunction)_wrap_g_network_service_get_protocol, METH_NOARGS,
      NULL },
    { "get_domain", (PyCFunction)_wrap_g_network_service_get_domain, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGNetworkService_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.NetworkService",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGNetworkService_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_network_service_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GResolver ----------- */

static PyObject *
_wrap_g_resolver_set_default(PyGObject *self)
{
    
    g_resolver_set_default(G_RESOLVER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 24 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_by_name(PyGObject *self,
                                PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "hostname", "cancellable", NULL };
    gchar *hostname;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    GList *addr;
    GError *error = NULL;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "s|O:gio.Resolver.lookup_by_name",
                                    kwlist,
                                    &hostname,
                                    &pycancellable))
        return NULL;
    
    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;
    
    addr = g_resolver_lookup_by_name(G_RESOLVER(self->obj),
                                     hostname, cancellable, &error);
    
    if (addr) {
        PYLIST_FROMGLIST(ret, addr, pygobject_new(list_item), g_resolver_free_addresses, NULL);
        return ret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 6141 "gio.c"


#line 60 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_by_name_async(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "hostname",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    gchar *hostname;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "Os|OO:gio.Resolver.lookup_by_name_async",
                                     kwlist,
                                     &notify->callback,
                                     &hostname,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_resolver_lookup_by_name_async(G_RESOLVER(self->obj),
                          hostname,
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 6189 "gio.c"


#line 106 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_by_name_finish(PyGObject *self,
                                       PyObject *args,
                                       PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GList *addr;
    PyObject *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.Resolver.lookup_by_name_finish",
                                     kwlist,
                                     &PyGAsyncResult_Type,
                                     &result))
        return NULL;
    
    addr = g_resolver_lookup_by_name_finish(G_RESOLVER(self->obj),
                                            G_ASYNC_RESULT(result->obj),
                                            &error);
    
    if (pyg_error_check(&error))
        return NULL;

    if (addr) {
        PYLIST_FROMGLIST(ret, addr, pygobject_new(list_item),
                         g_resolver_free_addresses, NULL);
        return ret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 6227 "gio.c"


static PyObject *
_wrap_g_resolver_lookup_by_address(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "cancellable", NULL };
    PyGObject *address, *py_cancellable = NULL;
    gchar *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:gio.Resolver.lookup_by_address", kwlist, &PyGInetAddress_Type, &address, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_resolver_lookup_by_address(G_RESOLVER(self->obj), G_INET_ADDRESS(address->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 142 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_by_address_async(PyGObject *self,
                                         PyObject *args,
                                         PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "address",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *address;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OO|OO:gio.Resolver.lookup_by_address_async",
                                     kwlist,
                                     &notify->callback,
                                     &address,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_resolver_lookup_by_address_async(G_RESOLVER(self->obj),
                          G_INET_ADDRESS(address->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 6308 "gio.c"


static PyObject *
_wrap_g_resolver_lookup_by_address_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    gchar *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Resolver.lookup_by_address_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_resolver_lookup_by_address_finish(G_RESOLVER(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 188 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_service(PyGObject *self,
                                PyObject *args,
                                PyObject *kwargs)
{
    static char *kwlist[] = { "service", "protocol",
                              "domain", "cancellable", NULL };
    gchar *service, *protocol, *domain;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    GList *targets;
    GError *error = NULL;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "sss|O:gio.Resolver.lookup_service",
                                    kwlist,
                                    &service,
                                    &protocol,
                                    &domain,
                                    &pycancellable))
        return NULL;
    
    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;
    
    targets = g_resolver_lookup_service(G_RESOLVER(self->obj),
                                        service, protocol, domain,
                                        cancellable, &error);
    
    if (targets) {
        PYLIST_FROMGLIST(ret, targets,
                        pyg_boxed_new(G_TYPE_SRV_TARGET, list_item, TRUE, TRUE),
                        g_resolver_free_targets, NULL);
        return ret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 6376 "gio.c"


#line 230 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_service_async(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "service", "protocol", "domain",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    gchar *service, *protocol, *domain;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "Osss|OO:gio.Resolver.lookup_service_async",
                                    kwlist,
                                    &notify->callback,
                                    &service,
                                    &protocol,
                                    &domain,
                                    &py_cancellable,
                                    &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_resolver_lookup_service_async(G_RESOLVER(self->obj),
                          service, protocol, domain,
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 6426 "gio.c"


#line 278 "gresolver.override"
static PyObject *
_wrap_g_resolver_lookup_service_finish(PyGObject *self,
                                       PyObject *args,
                                       PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GList *targets;
    PyObject *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.Resolver.lookup_service_finish",
                                     kwlist,
                                     &PyGAsyncResult_Type,
                                     &result))
        return NULL;
    
    targets = g_resolver_lookup_service_finish(G_RESOLVER(self->obj),
                                               G_ASYNC_RESULT(result->obj),
                                               &error);
    
    if (pyg_error_check(&error))
        return NULL;

    if (targets) {
        PYLIST_FROMGLIST(ret, targets,
                        pyg_boxed_new(G_TYPE_SRV_TARGET, list_item, TRUE, TRUE),
                        g_resolver_free_targets, NULL);
        return ret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 6465 "gio.c"


static const PyMethodDef _PyGResolver_methods[] = {
    { "set_default", (PyCFunction)_wrap_g_resolver_set_default, METH_NOARGS,
      NULL },
    { "lookup_by_name", (PyCFunction)_wrap_g_resolver_lookup_by_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_by_name_async", (PyCFunction)_wrap_g_resolver_lookup_by_name_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_by_name_finish", (PyCFunction)_wrap_g_resolver_lookup_by_name_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_by_address", (PyCFunction)_wrap_g_resolver_lookup_by_address, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_by_address_async", (PyCFunction)_wrap_g_resolver_lookup_by_address_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_by_address_finish", (PyCFunction)_wrap_g_resolver_lookup_by_address_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_service", (PyCFunction)_wrap_g_resolver_lookup_service, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_service_async", (PyCFunction)_wrap_g_resolver_lookup_service_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_service_finish", (PyCFunction)_wrap_g_resolver_lookup_service_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGResolver_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Resolver",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGResolver_methods, /* tp_methods */
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



/* ----------- GSocket ----------- */

static int
_wrap_g_socket_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", "type", "protocol", NULL };
    PyObject *py_family = NULL, *py_type = NULL, *py_protocol = NULL;
    GSocketFamily family;
    GSocketProtocol protocol;
    GError *error = NULL;
    GSocketType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOO:gio.Socket.__init__", kwlist, &py_family, &py_type, &py_protocol))
        return -1;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return -1;
    if (pyg_enum_get_value(G_TYPE_SOCKET_TYPE, py_type, (gpointer)&type))
        return -1;
    if (pyg_enum_get_value(G_TYPE_SOCKET_PROTOCOL, py_protocol, (gpointer)&protocol))
        return -1;
    self->obj = (GObject *)g_socket_new(family, type, protocol, &error);
    if (pyg_error_check(&error))
        return -1;

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GSocket object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_socket_connection_factory_create_connection(PyGObject *self)
{
    GSocketConnection *ret;

    
    ret = g_socket_connection_factory_create_connection(G_SOCKET(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_get_fd(PyGObject *self)
{
    int ret;

    
    ret = g_socket_get_fd(G_SOCKET(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_socket_get_family(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_get_family(G_SOCKET(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_FAMILY, ret);
}

static PyObject *
_wrap_g_socket_get_socket_type(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_get_socket_type(G_SOCKET(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_TYPE, ret);
}

static PyObject *
_wrap_g_socket_get_protocol(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_get_protocol(G_SOCKET(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_PROTOCOL, ret);
}

static PyObject *
_wrap_g_socket_get_local_address(PyGObject *self)
{
    GSocketAddress *ret;
    GError *error = NULL;

    
    ret = g_socket_get_local_address(G_SOCKET(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_get_remote_address(PyGObject *self)
{
    GSocketAddress *ret;
    GError *error = NULL;

    
    ret = g_socket_get_remote_address(G_SOCKET(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_set_blocking(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "blocking", NULL };
    int blocking;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.Socket.set_blocking", kwlist, &blocking))
        return NULL;
    
    g_socket_set_blocking(G_SOCKET(self->obj), blocking);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_get_blocking(PyGObject *self)
{
    int ret;

    
    ret = g_socket_get_blocking(G_SOCKET(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_set_keepalive(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "keepalive", NULL };
    int keepalive;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.Socket.set_keepalive", kwlist, &keepalive))
        return NULL;
    
    g_socket_set_keepalive(G_SOCKET(self->obj), keepalive);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_get_keepalive(PyGObject *self)
{
    int ret;

    
    ret = g_socket_get_keepalive(G_SOCKET(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_get_listen_backlog(PyGObject *self)
{
    int ret;

    
    ret = g_socket_get_listen_backlog(G_SOCKET(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_socket_set_listen_backlog(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "backlog", NULL };
    int backlog;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.Socket.set_listen_backlog", kwlist, &backlog))
        return NULL;
    
    g_socket_set_listen_backlog(G_SOCKET(self->obj), backlog);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_is_connected(PyGObject *self)
{
    int ret;

    
    ret = g_socket_is_connected(G_SOCKET(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_bind(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "allow_reuse", NULL };
    PyGObject *address;
    int allow_reuse, ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:gio.Socket.bind", kwlist, &PyGSocketAddress_Type, &address, &allow_reuse))
        return NULL;
    
    ret = g_socket_bind(G_SOCKET(self->obj), G_SOCKET_ADDRESS(address->obj), allow_reuse, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_connect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "cancellable", NULL };
    PyGObject *address, *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:gio.Socket.connect", kwlist, &PyGSocketAddress_Type, &address, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_connect(G_SOCKET(self->obj), G_SOCKET_ADDRESS(address->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_check_connect_result(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_socket_check_connect_result(G_SOCKET(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 25 "gsocket.override"
static PyObject *
_wrap_g_socket_condition_check(PyGObject *self,
                               PyObject *args,
                               PyObject *kwargs)
{
    static char *kwlist[] = { "condition", NULL };
    gint condition, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i:gio.Socket.condition_check",
                                     kwlist, &condition))
        return NULL;
    
    ret = g_socket_condition_check(G_SOCKET(self->obj), condition);
    
    return pyg_flags_from_gtype(G_TYPE_IO_CONDITION, ret);
}
#line 6829 "gio.c"


#line 44 "gsocket.override"
static PyObject *
_wrap_g_socket_condition_wait(PyGObject *self,
                              PyObject *args,
                              PyObject *kwargs)
{
    static char *kwlist[] = { "condition", "cancellable", NULL };
    gboolean ret;
    gint condition;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    GError *error;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i|O:gio.Socket.condition_wait",
                                     kwlist, &condition, &cancellable))
        return NULL;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        return NULL;
    
    ret = g_socket_condition_wait(G_SOCKET(self->obj), condition,
                                  cancellable, &error);
    
    return PyBool_FromLong(ret);
}
#line 6858 "gio.c"


static PyObject *
_wrap_g_socket_accept(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GSocket *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.Socket.accept", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_accept(G_SOCKET(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_listen(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_socket_listen(G_SOCKET(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_receive(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", "size", "cancellable", NULL };
    gsize size;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    gssize ret;
    char *buffer;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sk|O:gio.Socket.receive", kwlist, &buffer, &size, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_receive(G_SOCKET(self->obj), buffer, size, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_socket_send(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", "size", "cancellable", NULL };
    gsize size;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    gssize ret;
    char *buffer;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sk|O:gio.Socket.send", kwlist, &buffer, &size, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_send(G_SOCKET(self->obj), buffer, size, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_socket_send_to(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "buffer", "size", "cancellable", NULL };
    gsize size;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    gssize ret;
    char *buffer;
    PyGObject *address, *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!sk|O:gio.Socket.send_to", kwlist, &PyGSocketAddress_Type, &address, &buffer, &size, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_send_to(G_SOCKET(self->obj), G_SOCKET_ADDRESS(address->obj), buffer, size, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_socket_close(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_socket_close(G_SOCKET(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_shutdown(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "shutdown_read", "shutdown_write", NULL };
    int shutdown_read, shutdown_write, ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:gio.Socket.shutdown", kwlist, &shutdown_read, &shutdown_write))
        return NULL;
    
    ret = g_socket_shutdown(G_SOCKET(self->obj), shutdown_read, shutdown_write, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_is_closed(PyGObject *self)
{
    int ret;

    
    ret = g_socket_is_closed(G_SOCKET(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_socket_speaks_ipv4(PyGObject *self)
{
    int ret;

    
    ret = g_socket_speaks_ipv4(G_SOCKET(self->obj));
    
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGSocket_methods[] = {
    { "connection_factory_create_connection", (PyCFunction)_wrap_g_socket_connection_factory_create_connection, METH_NOARGS,
      NULL },
    { "get_fd", (PyCFunction)_wrap_g_socket_get_fd, METH_NOARGS,
      NULL },
    { "get_family", (PyCFunction)_wrap_g_socket_get_family, METH_NOARGS,
      NULL },
    { "get_socket_type", (PyCFunction)_wrap_g_socket_get_socket_type, METH_NOARGS,
      NULL },
    { "get_protocol", (PyCFunction)_wrap_g_socket_get_protocol, METH_NOARGS,
      NULL },
    { "get_local_address", (PyCFunction)_wrap_g_socket_get_local_address, METH_NOARGS,
      NULL },
    { "get_remote_address", (PyCFunction)_wrap_g_socket_get_remote_address, METH_NOARGS,
      NULL },
    { "set_blocking", (PyCFunction)_wrap_g_socket_set_blocking, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_blocking", (PyCFunction)_wrap_g_socket_get_blocking, METH_NOARGS,
      NULL },
    { "set_keepalive", (PyCFunction)_wrap_g_socket_set_keepalive, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_keepalive", (PyCFunction)_wrap_g_socket_get_keepalive, METH_NOARGS,
      NULL },
    { "get_listen_backlog", (PyCFunction)_wrap_g_socket_get_listen_backlog, METH_NOARGS,
      NULL },
    { "set_listen_backlog", (PyCFunction)_wrap_g_socket_set_listen_backlog, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_connected", (PyCFunction)_wrap_g_socket_is_connected, METH_NOARGS,
      NULL },
    { "bind", (PyCFunction)_wrap_g_socket_bind, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect", (PyCFunction)_wrap_g_socket_connect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "check_connect_result", (PyCFunction)_wrap_g_socket_check_connect_result, METH_NOARGS,
      NULL },
    { "condition_check", (PyCFunction)_wrap_g_socket_condition_check, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "condition_wait", (PyCFunction)_wrap_g_socket_condition_wait, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept", (PyCFunction)_wrap_g_socket_accept, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "listen", (PyCFunction)_wrap_g_socket_listen, METH_NOARGS,
      NULL },
    { "receive", (PyCFunction)_wrap_g_socket_receive, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send", (PyCFunction)_wrap_g_socket_send, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_to", (PyCFunction)_wrap_g_socket_send_to, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close", (PyCFunction)_wrap_g_socket_close, METH_NOARGS,
      NULL },
    { "shutdown", (PyCFunction)_wrap_g_socket_shutdown, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_closed", (PyCFunction)_wrap_g_socket_is_closed, METH_NOARGS,
      NULL },
    { "speaks_ipv4", (PyCFunction)_wrap_g_socket_speaks_ipv4, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocket_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Socket",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocket_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_socket_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GSocketAddressEnumerator ----------- */

static PyObject *
_wrap_g_socket_address_enumerator_next(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GSocketAddress *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.SocketAddressEnumerator.next", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_address_enumerator_next(G_SOCKET_ADDRESS_ENUMERATOR(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 71 "gsocket.override"
static PyObject *
_wrap_g_socket_address_enumerator_next_async(PyGObject *self,
                                             PyObject *args,
                                             PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "O|OO:gio.SocketAddressEnumerator.next_async",
                                    kwlist,
                                    &notify->callback,
                                    &py_cancellable,
                                    &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_address_enumerator_next_async(G_SOCKET_ADDRESS_ENUMERATOR(self->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 7229 "gio.c"


static PyObject *
_wrap_g_socket_address_enumerator_next_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GSocketAddress *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketAddressEnumerator.next_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_socket_address_enumerator_next_finish(G_SOCKET_ADDRESS_ENUMERATOR(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGSocketAddressEnumerator_methods[] = {
    { "next", (PyCFunction)_wrap_g_socket_address_enumerator_next, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "next_async", (PyCFunction)_wrap_g_socket_address_enumerator_next_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "next_finish", (PyCFunction)_wrap_g_socket_address_enumerator_next_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketAddressEnumerator_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketAddressEnumerator",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketAddressEnumerator_methods, /* tp_methods */
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



/* ----------- GSocketClient ----------- */

static int
_wrap_g_socket_client_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.SocketClient.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.SocketClient object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_socket_client_get_family(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_client_get_family(G_SOCKET_CLIENT(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_FAMILY, ret);
}

static PyObject *
_wrap_g_socket_client_set_family(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", NULL };
    PyObject *py_family = NULL;
    GSocketFamily family;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.SocketClient.set_family", kwlist, &py_family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    
    g_socket_client_set_family(G_SOCKET_CLIENT(self->obj), family);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_client_get_socket_type(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_client_get_socket_type(G_SOCKET_CLIENT(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_TYPE, ret);
}

static PyObject *
_wrap_g_socket_client_set_socket_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GSocketType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.SocketClient.set_socket_type", kwlist, &py_type))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    g_socket_client_set_socket_type(G_SOCKET_CLIENT(self->obj), type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_client_get_protocol(PyGObject *self)
{
    gint ret;

    
    ret = g_socket_client_get_protocol(G_SOCKET_CLIENT(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_SOCKET_PROTOCOL, ret);
}

static PyObject *
_wrap_g_socket_client_set_protocol(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    PyObject *py_protocol = NULL;
    GSocketProtocol protocol;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.SocketClient.set_protocol", kwlist, &py_protocol))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_PROTOCOL, py_protocol, (gpointer)&protocol))
        return NULL;
    
    g_socket_client_set_protocol(G_SOCKET_CLIENT(self->obj), protocol);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_client_get_local_address(PyGObject *self)
{
    GSocketAddress *ret;

    
    ret = g_socket_client_get_local_address(G_SOCKET_CLIENT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_client_set_local_address(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", NULL };
    PyGObject *address;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketClient.set_local_address", kwlist, &PyGSocketAddress_Type, &address))
        return NULL;
    
    g_socket_client_set_local_address(G_SOCKET_CLIENT(self->obj), G_SOCKET_ADDRESS(address->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_client_connect(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "connectable", "cancellable", NULL };
    PyGObject *connectable, *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GSocketConnection *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:gio.SocketClient.connect", kwlist, &PyGSocketConnectable_Type, &connectable, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_client_connect(G_SOCKET_CLIENT(self->obj), G_SOCKET_CONNECTABLE(connectable->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_client_connect_to_host(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "host_and_port", "default_port", "cancellable", NULL };
    GSocketConnection *ret;
    int default_port;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *host_and_port;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si|O:gio.SocketClient.connect_to_host", kwlist, &host_and_port, &default_port, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_client_connect_to_host(G_SOCKET_CLIENT(self->obj), host_and_port, default_port, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_client_connect_to_service(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "domain", "service", "cancellable", NULL };
    char *domain, *service;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GSocketConnection *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss|O:gio.SocketClient.connect_to_service", kwlist, &domain, &service, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_socket_client_connect_to_service(G_SOCKET_CLIENT(self->obj), domain, service, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 113 "gsocket.override"
static PyObject *
_wrap_g_socket_client_connect_async(PyGObject *self,
                                    PyObject *args,
                                    PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "connectable", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    PyGObject *py_connectable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                    "OO|OO:gio.SocketClient.connect_async",
                                    kwlist,
                                    &notify->callback,
                                    &py_connectable,
                                    &py_cancellable,
                                    &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_client_connect_async(G_SOCKET_CLIENT(self->obj),
                          G_SOCKET_CONNECTABLE(py_connectable->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 7575 "gio.c"


static PyObject *
_wrap_g_socket_client_connect_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    GSocketConnection *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketClient.connect_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_socket_client_connect_finish(G_SOCKET_CLIENT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 158 "gsocket.override"
static PyObject *
_wrap_g_socket_client_connect_to_host_async(PyGObject *self,
                                            PyObject *args,
                                            PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "host_and_port", "default_port",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    gchar *host_and_port;
    guint16 default_port;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "OsH|OO:gio.SocketClient.connect_to_host_async",
                                kwlist,
                                &notify->callback,
                                &host_and_port,
                                &default_port,
                                &py_cancellable,
                                &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_client_connect_to_host_async(G_SOCKET_CLIENT(self->obj),
                          host_and_port, default_port,
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 7644 "gio.c"


static PyObject *
_wrap_g_socket_client_connect_to_host_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    GSocketConnection *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketClient.connect_to_host_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_socket_client_connect_to_host_finish(G_SOCKET_CLIENT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 206 "gsocket.override"
static PyObject *
_wrap_g_socket_client_connect_to_service_async(PyGObject *self,
                                               PyObject *args,
                                               PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "domain", "service",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    gchar *domain, *service;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                            "Oss|OO:gio.SocketClient.connect_to_service_async",
                            kwlist,
                            &notify->callback,
                            &domain,
                            &service,
                            &py_cancellable,
                            &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_client_connect_to_service_async(G_SOCKET_CLIENT(self->obj),
                          domain, service,
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 7712 "gio.c"


static PyObject *
_wrap_g_socket_client_connect_to_service_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    GSocketConnection *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketClient.connect_to_service_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_socket_client_connect_to_service_finish(G_SOCKET_CLIENT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGSocketClient_methods[] = {
    { "get_family", (PyCFunction)_wrap_g_socket_client_get_family, METH_NOARGS,
      NULL },
    { "set_family", (PyCFunction)_wrap_g_socket_client_set_family, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_socket_type", (PyCFunction)_wrap_g_socket_client_get_socket_type, METH_NOARGS,
      NULL },
    { "set_socket_type", (PyCFunction)_wrap_g_socket_client_set_socket_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_protocol", (PyCFunction)_wrap_g_socket_client_get_protocol, METH_NOARGS,
      NULL },
    { "set_protocol", (PyCFunction)_wrap_g_socket_client_set_protocol, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_local_address", (PyCFunction)_wrap_g_socket_client_get_local_address, METH_NOARGS,
      NULL },
    { "set_local_address", (PyCFunction)_wrap_g_socket_client_set_local_address, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect", (PyCFunction)_wrap_g_socket_client_connect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_host", (PyCFunction)_wrap_g_socket_client_connect_to_host, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_service", (PyCFunction)_wrap_g_socket_client_connect_to_service, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_async", (PyCFunction)_wrap_g_socket_client_connect_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_finish", (PyCFunction)_wrap_g_socket_client_connect_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_host_async", (PyCFunction)_wrap_g_socket_client_connect_to_host_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_host_finish", (PyCFunction)_wrap_g_socket_client_connect_to_host_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_service_async", (PyCFunction)_wrap_g_socket_client_connect_to_service_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "connect_to_service_finish", (PyCFunction)_wrap_g_socket_client_connect_to_service_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketClient_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketClient",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketClient_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_socket_client_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GSocketConnection ----------- */

static PyObject *
_wrap_g_socket_connection_get_socket(PyGObject *self)
{
    GSocket *ret;

    
    ret = g_socket_connection_get_socket(G_SOCKET_CONNECTION(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_connection_get_local_address(PyGObject *self)
{
    GSocketAddress *ret;
    GError *error = NULL;

    
    ret = g_socket_connection_get_local_address(G_SOCKET_CONNECTION(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_connection_get_remote_address(PyGObject *self)
{
    GSocketAddress *ret;
    GError *error = NULL;

    
    ret = g_socket_connection_get_remote_address(G_SOCKET_CONNECTION(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGSocketConnection_methods[] = {
    { "get_socket", (PyCFunction)_wrap_g_socket_connection_get_socket, METH_NOARGS,
      NULL },
    { "get_local_address", (PyCFunction)_wrap_g_socket_connection_get_local_address, METH_NOARGS,
      NULL },
    { "get_remote_address", (PyCFunction)_wrap_g_socket_connection_get_remote_address, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketConnection_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketConnection",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketConnection_methods, /* tp_methods */
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



/* ----------- GSocketControlMessage ----------- */

static PyObject *
_wrap_g_socket_control_message_get_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_socket_control_message_get_size(G_SOCKET_CONTROL_MESSAGE(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_socket_control_message_get_level(PyGObject *self)
{
    int ret;

    
    ret = g_socket_control_message_get_level(G_SOCKET_CONTROL_MESSAGE(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_g_socket_control_message_get_msg_type(PyGObject *self)
{
    int ret;

    
    ret = g_socket_control_message_get_msg_type(G_SOCKET_CONTROL_MESSAGE(self->obj));
    
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGSocketControlMessage_methods[] = {
    { "get_size", (PyCFunction)_wrap_g_socket_control_message_get_size, METH_NOARGS,
      NULL },
    { "get_level", (PyCFunction)_wrap_g_socket_control_message_get_level, METH_NOARGS,
      NULL },
    { "get_msg_type", (PyCFunction)_wrap_g_socket_control_message_get_msg_type, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketControlMessage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketControlMessage",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketControlMessage_methods, /* tp_methods */
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



/* ----------- GSocketListener ----------- */

static int
_wrap_g_socket_listener_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.SocketListener.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.SocketListener object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_socket_listener_set_backlog(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "listen_backlog", NULL };
    int listen_backlog;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.SocketListener.set_backlog", kwlist, &listen_backlog))
        return NULL;
    
    g_socket_listener_set_backlog(G_SOCKET_LISTENER(self->obj), listen_backlog);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_listener_add_socket(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "socket", "source_object", NULL };
    PyGObject *socket, *source_object;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:gio.SocketListener.add_socket", kwlist, &PyGSocket_Type, &socket, &PyGObject_Type, &source_object))
        return NULL;
    
    ret = g_socket_listener_add_socket(G_SOCKET_LISTENER(self->obj), G_SOCKET(socket->obj), G_OBJECT(source_object->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 253 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_add_address(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "address", "type", "protocol",
                              "source_object", NULL };
    GSocketProtocol protocol;
    PyObject *py_type = NULL, *py_protocol = NULL;
    GError *error = NULL;
    gboolean ret;
    GSocketType type;
    GSocketAddress *effective_address;
    PyGObject *address, *py_source_object = NULL;
    GObject *source_object;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO|O!:gio.SocketListener.add_address",
                                     kwlist,
                                     &PyGSocketAddress_Type, &address,
                                     &py_type, &py_protocol,
                                     &PyGObject_Type, &source_object,
                                     &PyGSocketAddress_Type, &effective_address))
        return NULL;

    if (pyg_enum_get_value(G_TYPE_SOCKET_TYPE, py_type, (gpointer)&type))
        return NULL;

    if (pyg_enum_get_value(G_TYPE_SOCKET_PROTOCOL, py_protocol, (gpointer)&protocol))
        return NULL;
    
    if (py_source_object == NULL || (PyObject*)py_source_object == Py_None)
        source_object = NULL;
    else if (pygobject_check(py_source_object, &PyGObject_Type))
        source_object = G_OBJECT(py_source_object->obj);
    else {
      PyErr_SetString(PyExc_TypeError, "source_object should be a gobject.GObject or None");
      return NULL;
    }
    
    ret = g_socket_listener_add_address(G_SOCKET_LISTENER(self->obj),
                                        G_SOCKET_ADDRESS(address->obj),
                                        type, protocol,
                                        source_object,
                                        &effective_address,
                                        &error);
    
    if (pyg_error_check(&error))
        return NULL;
    
    if (ret)
        return pygobject_new((GObject *)effective_address);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 8124 "gio.c"


static PyObject *
_wrap_g_socket_listener_add_inet_port(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "port", "source_object", NULL };
    int port, ret;
    PyGObject *source_object;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO!:gio.SocketListener.add_inet_port", kwlist, &port, &PyGObject_Type, &source_object))
        return NULL;
    
    ret = g_socket_listener_add_inet_port(G_SOCKET_LISTENER(self->obj), port, G_OBJECT(source_object->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 438 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept_socket(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    GError *error = NULL;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    PyObject *py_socket, *py_source_object;
    GObject *source_object;
    GSocket *socket;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.SocketListener.accept_socket",
                                     kwlist,
                                     &py_cancellable))
        return NULL;


    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        return NULL;

    socket = g_socket_listener_accept_socket(G_SOCKET_LISTENER(self->obj),
                                             &source_object,
                                             cancellable,
                                             &error);

    if (pyg_error_check(&error))
        return NULL;

    if (socket)
        py_socket = pygobject_new((GObject *)socket);
    else {
        py_socket = Py_None;
        Py_INCREF(py_socket);
    }

    if (source_object)
        py_source_object = pygobject_new((GObject *)source_object);
    else {
        py_source_object= Py_None;
        Py_INCREF(py_source_object);
    }
    return Py_BuildValue("(NN)", py_socket, py_source_object);
}
#line 8192 "gio.c"


#line 485 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept_socket_async(PyGObject *self,
                                            PyObject *args,
                                            PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                            "O|OO:gio.SocketListener.accept_socket_async",
                            kwlist,
                            &notify->callback,
                            &py_cancellable,
                            &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_listener_accept_socket_async(G_SOCKET_LISTENER(self->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 8236 "gio.c"


#line 527 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept_socket_finish(PyGObject *self,
                                            PyObject *args,
                                            PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    GError *error = NULL;
    PyGObject *result;
    PyObject *py_socket, *py_source_object;
    GObject *source_object;
    GSocket *socket;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketListener.accept_socket_finish",
                                     kwlist,
                                     &PyGAsyncResult_Type, &result))
        return NULL;

    socket = g_socket_listener_accept_socket_finish(G_SOCKET_LISTENER(self->obj),
                                                    G_ASYNC_RESULT(result->obj),
                                                    &source_object,
                                                    &error);

    if (pyg_error_check(&error))
        return NULL;

    if (socket)
        py_socket = pygobject_new((GObject *)socket);
    else {
        py_socket= Py_None;
        Py_INCREF(py_socket);
    }

    if (source_object)
        py_source_object = pygobject_new((GObject *)source_object);
    else {
        py_source_object= Py_None;
        Py_INCREF(py_source_object);
    }
    return Py_BuildValue("(NN)", py_socket, py_source_object);
}

/* Could not write method GSocketAddress.to_native: No ArgType for gpointer */
/* Could not write method GSocket.receive_from: No ArgType for GSocketAddress** */
/* Could not write method GSocket.receive_message: No ArgType for GSocketAddress** */
/* Could not write method GSocket.send_message: No ArgType for GOutputVector* */
/* Could not write method GSocket.create_source: No ArgType for GIOCondition */
/* Could not write method GSocketControlMessage.serialize: No ArgType for gpointer */
/* Could not write function socket_address_new_from_native: No ArgType for gpointer */
/* Could not write function socket_control_message_deserialize: No ArgType for gpointer */
#line 8289 "gio.c"


#line 309 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    GError *error = NULL;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    PyObject *py_connection, *py_source_object;
    GObject *source_object;
    GSocketConnection *connection;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.SocketListener.accept",
                                     kwlist,
                                     &py_cancellable))
        return NULL;


    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        return NULL;

    connection = g_socket_listener_accept(G_SOCKET_LISTENER(self->obj),
                                          &source_object,
                                          cancellable,
                                          &error);

    if (pyg_error_check(&error))
        return NULL;

    if (connection)
        py_connection = pygobject_new((GObject *)connection);
    else {
        py_connection = Py_None;
        Py_INCREF(py_connection);
    }

    if (source_object)
        py_source_object = pygobject_new((GObject *)source_object);
    else {
        py_source_object= Py_None;
        Py_INCREF(py_source_object);
    }
    return Py_BuildValue("(NN)", py_connection, py_source_object);
}
#line 8336 "gio.c"


#line 354 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept_async(PyGObject *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                            "O|OO:gio.SocketListener.accept_async",
                            kwlist,
                            &notify->callback,
                            &py_cancellable,
                            &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_socket_listener_accept_async(G_SOCKET_LISTENER(self->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 8380 "gio.c"


#line 396 "gsocket.override"
static PyObject *
_wrap_g_socket_listener_accept_finish(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    GError *error = NULL;
    PyGObject *result;
    PyObject *py_connection, *py_source_object;
    GObject *source_object;
    GSocketConnection *connection;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.SocketListener.accept_finish",
                                     kwlist,
                                     &PyGAsyncResult_Type, &result))
        return NULL;

    connection = g_socket_listener_accept_finish(G_SOCKET_LISTENER(self->obj),
                                                 G_ASYNC_RESULT(result->obj),
                                                 &source_object,
                                                 &error);

    if (pyg_error_check(&error))
        return NULL;

    if (connection)
        py_connection = pygobject_new((GObject *)connection);
    else {
        py_connection = Py_None;
        Py_INCREF(py_connection);
    }

    if (source_object)
        py_source_object = pygobject_new((GObject *)source_object);
    else {
        py_source_object= Py_None;
        Py_INCREF(py_source_object);
    }
    return Py_BuildValue("(NN)", py_connection, py_source_object);
}
#line 8424 "gio.c"


static PyObject *
_wrap_g_socket_listener_close(PyGObject *self)
{
    
    g_socket_listener_close(G_SOCKET_LISTENER(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGSocketListener_methods[] = {
    { "set_backlog", (PyCFunction)_wrap_g_socket_listener_set_backlog, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_socket", (PyCFunction)_wrap_g_socket_listener_add_socket, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_address", (PyCFunction)_wrap_g_socket_listener_add_address, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_inet_port", (PyCFunction)_wrap_g_socket_listener_add_inet_port, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_socket", (PyCFunction)_wrap_g_socket_listener_accept_socket, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_socket_async", (PyCFunction)_wrap_g_socket_listener_accept_socket_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_socket_finish", (PyCFunction)_wrap_g_socket_listener_accept_socket_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept", (PyCFunction)_wrap_g_socket_listener_accept, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_async", (PyCFunction)_wrap_g_socket_listener_accept_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_finish", (PyCFunction)_wrap_g_socket_listener_accept_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close", (PyCFunction)_wrap_g_socket_listener_close, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketListener_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketListener",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketListener_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_socket_listener_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GSocketService ----------- */

static int
_wrap_g_socket_service_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gio.SocketService.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.SocketService object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_socket_service_start(PyGObject *self)
{
    
    g_socket_service_start(G_SOCKET_SERVICE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_service_stop(PyGObject *self)
{
    
    g_socket_service_stop(G_SOCKET_SERVICE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_service_is_active(PyGObject *self)
{
    int ret;

    
    ret = g_socket_service_is_active(G_SOCKET_SERVICE(self->obj));
    
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGSocketService_methods[] = {
    { "start", (PyCFunction)_wrap_g_socket_service_start, METH_NOARGS,
      NULL },
    { "stop", (PyCFunction)_wrap_g_socket_service_stop, METH_NOARGS,
      NULL },
    { "is_active", (PyCFunction)_wrap_g_socket_service_is_active, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketService_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketService",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketService_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_socket_service_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GTcpConnection ----------- */

PyTypeObject G_GNUC_INTERNAL PyGTcpConnection_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.TcpConnection",                   /* tp_name */
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



/* ----------- GThreadedSocketService ----------- */

static int
_wrap_g_threaded_socket_service_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "max_threads", NULL };
    int max_threads;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.ThreadedSocketService.__init__", kwlist, &max_threads))
        return -1;
    self->obj = (GObject *)g_threaded_socket_service_new(max_threads);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GThreadedSocketService object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

PyTypeObject G_GNUC_INTERNAL PyGThreadedSocketService_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.ThreadedSocketService",                   /* tp_name */
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
    (initproc)_wrap_g_threaded_socket_service_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GOutputStream ----------- */

#line 24 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_write(PyGObject *self,
			    PyObject *args,
			    PyObject *kwargs)
{
  static char *kwlist[] = { "buffer", "cancellable", NULL };
  PyGObject *pycancellable = NULL;
  gchar *buffer;
  long count = 0; 
  GCancellable *cancellable;
  GError *error = NULL;
  gssize written;
  
  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				   "s#|O!:OutputStream.write",
				   kwlist, &buffer, &count,
				   &PyGCancellable_Type, &pycancellable))
    return NULL;
  
  if (!pygio_check_cancellable(pycancellable, &cancellable))
      return NULL;

  pyg_begin_allow_threads;
  written = g_output_stream_write(G_OUTPUT_STREAM(self->obj),
				  buffer, count, cancellable, &error);
  pyg_end_allow_threads;

  if (pyg_error_check(&error))
    return NULL;
      
  return PyInt_FromLong(written);
}
#line 8772 "gio.c"


#line 58 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_write_all(PyGObject *self,
				PyObject *args,
				PyObject *kwargs)
{
  static char *kwlist[] = { "buffer", "cancellable", NULL };
  PyGObject *pycancellable = NULL;
  gchar *buffer;
  long count = 0; 
  GCancellable *cancellable;
  GError *error = NULL;
  gsize written;

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				   "s#|O!:OutputStream.write",
				   kwlist, &buffer, &count,
				   &PyGCancellable_Type, &pycancellable))
    return NULL;

  if (!pygio_check_cancellable(pycancellable, &cancellable))
      return NULL;

  pyg_begin_allow_threads;
  g_output_stream_write_all(G_OUTPUT_STREAM(self->obj),
			    buffer, count, &written, cancellable, &error);
  pyg_end_allow_threads;

  if (pyg_error_check(&error))
    return NULL;
      
  return PyInt_FromLong(written);
}
#line 8808 "gio.c"


static PyObject *
_wrap_g_output_stream_splice(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "source", "flags", "cancellable", NULL };
    GError *error = NULL;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GOutputStreamSpliceFlags flags = G_OUTPUT_STREAM_SPLICE_NONE;
    gssize ret;
    PyGObject *source, *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|OO:gio.OutputStream.splice", kwlist, &PyGInputStream_Type, &source, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_OUTPUT_STREAM_SPLICE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_output_stream_splice(G_OUTPUT_STREAM(self->obj), G_INPUT_STREAM(source->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_output_stream_flush(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.OutputStream.flush", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_output_stream_flush(G_OUTPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_output_stream_close(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.OutputStream.close", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_output_stream_close(G_OUTPUT_STREAM(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 92 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_write_async(PyGObject *self,
				  PyObject *args,
				  PyObject *kwargs)
{
  static char *kwlist[] = { "buffer", "callback", "io_priority", "cancellable",
			    "user_data", NULL };
  gchar *buffer;
  long count = -1;
  int io_priority = G_PRIORITY_DEFAULT;
  PyGObject *pycancellable = NULL;
  GCancellable *cancellable;
  PyGIONotify *notify;

  notify = pygio_notify_new();

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				   "s#O|iOO:OutputStream.write_async",
				   kwlist, &buffer,
				   &count,
				   &notify->callback,
				   &io_priority,
				   &pycancellable,
				   &notify->data))
      goto error;

  if (!pygio_notify_callback_is_valid(notify))
      goto error;
  
  if (!pygio_check_cancellable(pycancellable, &cancellable))
      goto error;

  pygio_notify_reference_callback(notify);
  pygio_notify_copy_buffer(notify, buffer, count);

  g_output_stream_write_async(G_OUTPUT_STREAM(self->obj),
			      notify->buffer,
			      notify->buffer_size,
			      io_priority,
			      cancellable,
			      (GAsyncReadyCallback)async_result_callback_marshal,
			      notify);
  
  Py_INCREF(Py_None);
  return Py_None;

 error:
  pygio_notify_free(notify);
  return NULL;
}
#line 8950 "gio.c"


static PyObject *
_wrap_g_output_stream_write_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gssize ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.OutputStream.write_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_output_stream_write_finish(G_OUTPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

#line 236 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_splice_async(PyGObject *self,
                                   PyObject *args,
                                   PyObject *kwargs)
{
    static char *kwlist[] = { "source", "callback", "flags", "io_priority",
                              "cancellable", "user_data", NULL };
    
    int io_priority = G_PRIORITY_DEFAULT;
    GOutputStreamSpliceFlags flags = G_OUTPUT_STREAM_SPLICE_NONE;
    PyObject *py_flags = NULL;
    PyGObject *source;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;
  
    notify = pygio_notify_new();
  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O|OiOO:OutputStream.splice_async",
                                     kwlist,
                                     &PyGInputStream_Type,
                                     &source,
                                     &notify->callback,
                                     &py_flags,
                                     &io_priority,
                                     &pycancellable,
                                     &notify->data))
        goto error;
  
    if (!pygio_notify_callback_is_valid(notify))
        goto error;
  
    if (py_flags && pyg_flags_get_value(G_TYPE_OUTPUT_STREAM_SPLICE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;
    
    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;
  
    pygio_notify_reference_callback(notify);
    
    g_output_stream_splice_async(G_OUTPUT_STREAM(self->obj),
                            G_INPUT_STREAM(source->obj), flags, io_priority,
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);
    
    Py_INCREF(Py_None);
        return Py_None;
  
    error:
        pygio_notify_free(notify);
        return NULL;
}

/* GOutputStream.write_all: No ArgType for const-void* */
#line 9030 "gio.c"


static PyObject *
_wrap_g_output_stream_splice_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    gssize ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.OutputStream.splice_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_output_stream_splice_finish(G_OUTPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyLong_FromLongLong(ret);

}

#line 190 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_flush_async(PyGObject *self,
				  PyObject *args,
				  PyObject *kwargs)
{
  static char *kwlist[] = { "callback", "io_priority",
			    "cancellable", "user_data", NULL };
  int io_priority = G_PRIORITY_DEFAULT;
  PyGObject *pycancellable = NULL;
  GCancellable *cancellable;
  PyGIONotify *notify;

  notify = pygio_notify_new();

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				   "O|iOO:OutputStream.flush_async",
				   kwlist,
				   &notify->callback,
				   &io_priority,
				   &pycancellable,
				   &notify->data))
      goto error;

  if (!pygio_notify_callback_is_valid(notify))
      goto error;

  if (!pygio_check_cancellable(pycancellable, &cancellable))
      goto error;

  pygio_notify_reference_callback(notify);
  
  g_output_stream_flush_async(G_OUTPUT_STREAM(self->obj),
			      io_priority,
			      cancellable,
			      (GAsyncReadyCallback)async_result_callback_marshal,
			      notify);
  
  Py_INCREF(Py_None);
  return Py_None;

 error:
  pygio_notify_free(notify);
  return NULL;
}
#line 9097 "gio.c"


static PyObject *
_wrap_g_output_stream_flush_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.OutputStream.flush_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_output_stream_flush_finish(G_OUTPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 144 "goutputstream.override"
static PyObject *
_wrap_g_output_stream_close_async(PyGObject *self,
				  PyObject *args,
				  PyObject *kwargs)
{
  static char *kwlist[] = { "callback", "io_priority",
			    "cancellable", "user_data", NULL };
  int io_priority = G_PRIORITY_DEFAULT;
  PyGObject *pycancellable = NULL;
  GCancellable *cancellable;
  PyGIONotify *notify;

  notify = pygio_notify_new();

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				   "O|iOO:OutputStream.close_async",
				   kwlist,
				   &notify->callback,
				   &io_priority,
				   &pycancellable,
				   &notify->data))
      goto error;

  if (!pygio_notify_callback_is_valid(notify))
      goto error;

  if (!pygio_check_cancellable(pycancellable, &cancellable))
      goto error;

  pygio_notify_reference_callback(notify);
  
  g_output_stream_close_async(G_OUTPUT_STREAM(self->obj),
			      io_priority,
			      cancellable,
			      (GAsyncReadyCallback)async_result_callback_marshal,
			      notify);
  
  Py_INCREF(Py_None);
  return Py_None;

 error:
  pygio_notify_free(notify);
  return NULL;
}
#line 9164 "gio.c"


static PyObject *
_wrap_g_output_stream_close_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.OutputStream.close_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_output_stream_close_finish(G_OUTPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_output_stream_is_closed(PyGObject *self)
{
    int ret;

    
    ret = g_output_stream_is_closed(G_OUTPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_output_stream_has_pending(PyGObject *self)
{
    int ret;

    
    ret = g_output_stream_has_pending(G_OUTPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_output_stream_set_pending(PyGObject *self)
{
    int ret;
    GError *error = NULL;

    
    ret = g_output_stream_set_pending(G_OUTPUT_STREAM(self->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_output_stream_clear_pending(PyGObject *self)
{
    
    g_output_stream_clear_pending(G_OUTPUT_STREAM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGOutputStream_methods[] = {
    { "write_part", (PyCFunction)_wrap_g_output_stream_write, METH_VARARGS|METH_KEYWORDS,
      (char *) "STREAM.write_part(buffer, [cancellable]) -> int\n"
"\n"
"Write the bytes in 'buffer' to the stream. Return the number of bytes\n"
"successfully written. This method is allowed to stop at any time after\n"
"writing at least 1 byte. Therefore, to reliably write the whole buffer,\n"
"you need to use a loop. See also gio.OutputStream.write for easier to\n"
"use (though less efficient) method.\n"
"\n"
"Note: this method roughly corresponds to C GIO g_output_stream_write." },
    { "write", (PyCFunction)_wrap_g_output_stream_write_all, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "splice", (PyCFunction)_wrap_g_output_stream_splice, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flush", (PyCFunction)_wrap_g_output_stream_flush, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close", (PyCFunction)_wrap_g_output_stream_close, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "write_async", (PyCFunction)_wrap_g_output_stream_write_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "S.write_async(buffer, callback [,io_priority] [,cancellable] [,user_data])\n"
"\n"
"Request an asynchronous write of count bytes from buffer into the stream.\n"
"When the operation is finished callback will be called. You can then call\n"
"gio.OutputStream.write_finish() to get the result of the operation.\n"
"On success, the number of bytes written will be passed to the callback.\n"
"It is not an error if this is not the same as the requested size, as it can\n"
"happen e.g. on a partial I/O error, but generally tries to write as many \n"
"bytes as requested.\n"
"For the synchronous, blocking version of this function, see\n"
"gio.OutputStream.write().\n" },
    { "write_finish", (PyCFunction)_wrap_g_output_stream_write_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "splice_async", (PyCFunction)_wrap_g_output_stream_splice_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "splice_finish", (PyCFunction)_wrap_g_output_stream_splice_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flush_async", (PyCFunction)_wrap_g_output_stream_flush_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flush_finish", (PyCFunction)_wrap_g_output_stream_flush_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_async", (PyCFunction)_wrap_g_output_stream_close_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "close_finish", (PyCFunction)_wrap_g_output_stream_close_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_closed", (PyCFunction)_wrap_g_output_stream_is_closed, METH_NOARGS,
      NULL },
    { "has_pending", (PyCFunction)_wrap_g_output_stream_has_pending, METH_NOARGS,
      NULL },
    { "set_pending", (PyCFunction)_wrap_g_output_stream_set_pending, METH_NOARGS,
      NULL },
    { "clear_pending", (PyCFunction)_wrap_g_output_stream_clear_pending, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.OutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGOutputStream_methods, /* tp_methods */
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



/* ----------- GMemoryOutputStream ----------- */

#line 24 "gmemoryoutputstream.override"
static int
_wrap_g_memory_output_stream_new(PyGObject *self)
{
    self->obj = (GObject *)g_memory_output_stream_new(NULL, 0, g_realloc, g_free);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create gio.MemoryOutputStream object");
        return -1;
    }

    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 9354 "gio.c"


#line 39 "gmemoryoutputstream.override"
static PyObject *
_wrap_g_memory_output_stream_get_data(PyGObject *self)
{
    GMemoryOutputStream *stream = G_MEMORY_OUTPUT_STREAM(self->obj);
    return PyString_FromStringAndSize(g_memory_output_stream_get_data(stream),
				      g_seekable_tell(G_SEEKABLE(stream)));
}
#line 9365 "gio.c"


static PyObject *
_wrap_g_memory_output_stream_get_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_memory_output_stream_get_size(G_MEMORY_OUTPUT_STREAM(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_memory_output_stream_get_data_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_memory_output_stream_get_data_size(G_MEMORY_OUTPUT_STREAM(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static const PyMethodDef _PyGMemoryOutputStream_methods[] = {
    { "get_contents", (PyCFunction)_wrap_g_memory_output_stream_get_data, METH_NOARGS,
      NULL },
    { "get_size", (PyCFunction)_wrap_g_memory_output_stream_get_size, METH_NOARGS,
      NULL },
    { "get_data_size", (PyCFunction)_wrap_g_memory_output_stream_get_data_size, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGMemoryOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.MemoryOutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGMemoryOutputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_memory_output_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GFilterOutputStream ----------- */

static PyObject *
_wrap_g_filter_output_stream_get_base_stream(PyGObject *self)
{
    GOutputStream *ret;

    
    ret = g_filter_output_stream_get_base_stream(G_FILTER_OUTPUT_STREAM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_filter_output_stream_get_close_base_stream(PyGObject *self)
{
    int ret;

    
    ret = g_filter_output_stream_get_close_base_stream(G_FILTER_OUTPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_filter_output_stream_set_close_base_stream(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "close_base", NULL };
    int close_base;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.FilterOutputStream.set_close_base_stream", kwlist, &close_base))
        return NULL;
    
    g_filter_output_stream_set_close_base_stream(G_FILTER_OUTPUT_STREAM(self->obj), close_base);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFilterOutputStream_methods[] = {
    { "get_base_stream", (PyCFunction)_wrap_g_filter_output_stream_get_base_stream, METH_NOARGS,
      NULL },
    { "get_close_base_stream", (PyCFunction)_wrap_g_filter_output_stream_get_close_base_stream, METH_NOARGS,
      NULL },
    { "set_close_base_stream", (PyCFunction)_wrap_g_filter_output_stream_set_close_base_stream, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFilterOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FilterOutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFilterOutputStream_methods, /* tp_methods */
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



/* ----------- GBufferedOutputStream ----------- */

static int
_wrap_g_buffered_output_stream_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"base_stream", NULL };
    char *prop_names[] = {"base_stream", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gio.BufferedOutputStream.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.BufferedOutputStream object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_buffered_output_stream_get_buffer_size(PyGObject *self)
{
    gsize ret;

    
    ret = g_buffered_output_stream_get_buffer_size(G_BUFFERED_OUTPUT_STREAM(self->obj));
    
    return PyLong_FromUnsignedLongLong(ret);

}

static PyObject *
_wrap_g_buffered_output_stream_set_buffer_size(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    gsize size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:gio.BufferedOutputStream.set_buffer_size", kwlist, &size))
        return NULL;
    
    g_buffered_output_stream_set_buffer_size(G_BUFFERED_OUTPUT_STREAM(self->obj), size);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_buffered_output_stream_get_auto_grow(PyGObject *self)
{
    int ret;

    
    ret = g_buffered_output_stream_get_auto_grow(G_BUFFERED_OUTPUT_STREAM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_buffered_output_stream_set_auto_grow(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "auto_grow", NULL };
    int auto_grow;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.BufferedOutputStream.set_auto_grow", kwlist, &auto_grow))
        return NULL;
    
    g_buffered_output_stream_set_auto_grow(G_BUFFERED_OUTPUT_STREAM(self->obj), auto_grow);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGBufferedOutputStream_methods[] = {
    { "get_buffer_size", (PyCFunction)_wrap_g_buffered_output_stream_get_buffer_size, METH_NOARGS,
      NULL },
    { "set_buffer_size", (PyCFunction)_wrap_g_buffered_output_stream_set_buffer_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_auto_grow", (PyCFunction)_wrap_g_buffered_output_stream_get_auto_grow, METH_NOARGS,
      NULL },
    { "set_auto_grow", (PyCFunction)_wrap_g_buffered_output_stream_set_auto_grow, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGBufferedOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.BufferedOutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGBufferedOutputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_buffered_output_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GDataOutputStream ----------- */

 static int
_wrap_g_data_output_stream_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"base_stream", NULL };
    char *prop_names[] = {"base_stream", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gio.DataOutputStream.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gio.DataOutputStream object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_g_data_output_stream_set_byte_order(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "order", NULL };
    PyObject *py_order = NULL;
    GDataStreamByteOrder order;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:gio.DataOutputStream.set_byte_order", kwlist, &py_order))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_DATA_STREAM_BYTE_ORDER, py_order, (gpointer)&order))
        return NULL;
    
    g_data_output_stream_set_byte_order(G_DATA_OUTPUT_STREAM(self->obj), order);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_data_output_stream_get_byte_order(PyGObject *self)
{
    gint ret;

    
    ret = g_data_output_stream_get_byte_order(G_DATA_OUTPUT_STREAM(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_DATA_STREAM_BYTE_ORDER, ret);
}

static PyObject *
_wrap_g_data_output_stream_put_byte(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    char data;
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"c|O:gio.DataOutputStream.put_byte", kwlist, &data, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_byte(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_int16(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    int data, ret;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|O:gio.DataOutputStream.put_int16", kwlist, &data, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_int16(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_uint16(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    int data, ret;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|O:gio.DataOutputStream.put_uint16", kwlist, &data, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_uint16(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_int32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    int data, ret;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i|O:gio.DataOutputStream.put_int32", kwlist, &data, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_int32(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_uint32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    unsigned long data;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k|O:gio.DataOutputStream.put_uint32", kwlist, &data, &py_cancellable))
        return NULL;
    if (data > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " data parameter to unsigned 32 bit integer");
        return NULL;
    }
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_uint32(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_int64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    gint64 data;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L|O:gio.DataOutputStream.put_int64", kwlist, &data, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_int64(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_uint64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "cancellable", NULL };
    PyObject *py_data = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    int ret;
    guint64 data;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:gio.DataOutputStream.put_uint64", kwlist, &PyLong_Type, &py_data, &py_cancellable))
        return NULL;
    data = PyLong_AsUnsignedLongLong(py_data);
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_uint64(G_DATA_OUTPUT_STREAM(self->obj), data, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_data_output_stream_put_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "str", "cancellable", NULL };
    char *str;
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.DataOutputStream.put_string", kwlist, &str, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_data_output_stream_put_string(G_DATA_OUTPUT_STREAM(self->obj), str, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGDataOutputStream_methods[] = {
    { "set_byte_order", (PyCFunction)_wrap_g_data_output_stream_set_byte_order, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_byte_order", (PyCFunction)_wrap_g_data_output_stream_get_byte_order, METH_NOARGS,
      NULL },
    { "put_byte", (PyCFunction)_wrap_g_data_output_stream_put_byte, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_int16", (PyCFunction)_wrap_g_data_output_stream_put_int16, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_uint16", (PyCFunction)_wrap_g_data_output_stream_put_uint16, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_int32", (PyCFunction)_wrap_g_data_output_stream_put_int32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_uint32", (PyCFunction)_wrap_g_data_output_stream_put_uint32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_int64", (PyCFunction)_wrap_g_data_output_stream_put_int64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_uint64", (PyCFunction)_wrap_g_data_output_stream_put_uint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "put_string", (PyCFunction)_wrap_g_data_output_stream_put_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGDataOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.DataOutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGDataOutputStream_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_data_output_stream_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GFileOutputStream ----------- */

static PyObject *
_wrap_g_file_output_stream_query_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "cancellable", NULL };
    char *attributes;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.FileOutputStream.query_info", kwlist, &attributes, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_output_stream_query_info(G_FILE_OUTPUT_STREAM(self->obj), attributes, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 25 "gfileoutputstream.override"
static PyObject *
_wrap_g_file_output_stream_query_info_async(PyGObject *self,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *attributes;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "sO|iOO:gio.FileOutputStream.query_info_async",
                                kwlist,
                                &attributes,
                                &notify->callback,
                                &io_priority,
                                &pycancellable,
                                &notify->data))

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_output_stream_query_info_async(G_FILE_OUTPUT_STREAM(self->obj),
                         attributes, io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 10137 "gio.c"


static PyObject *
_wrap_g_file_output_stream_query_info_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileOutputStream.query_info_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_output_stream_query_info_finish(G_FILE_OUTPUT_STREAM(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_output_stream_get_etag(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_output_stream_get_etag(G_FILE_OUTPUT_STREAM(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGFileOutputStream_methods[] = {
    { "query_info", (PyCFunction)_wrap_g_file_output_stream_query_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_async", (PyCFunction)_wrap_g_file_output_stream_query_info_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_finish", (PyCFunction)_wrap_g_file_output_stream_query_info_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_etag", (PyCFunction)_wrap_g_file_output_stream_get_etag, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGFileOutputStream_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileOutputStream",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGFileOutputStream_methods, /* tp_methods */
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



/* ----------- GSimpleAsyncResult ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

static PyObject *
_wrap_g_simple_async_result_set_op_res_gssize(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "op_res", NULL };
    gssize op_res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"l:gio.SimpleAsyncResult.set_op_res_gssize", kwlist, &op_res))
        return NULL;
    
    g_simple_async_result_set_op_res_gssize(G_SIMPLE_ASYNC_RESULT(self->obj), op_res);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_simple_async_result_get_op_res_gssize(PyGObject *self)
{
    gssize ret;

    
    ret = g_simple_async_result_get_op_res_gssize(G_SIMPLE_ASYNC_RESULT(self->obj));
    
    return PyLong_FromLongLong(ret);

}

static PyObject *
_wrap_g_simple_async_result_set_op_res_gboolean(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "op_res", NULL };
    int op_res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.SimpleAsyncResult.set_op_res_gboolean", kwlist, &op_res))
        return NULL;
    
    g_simple_async_result_set_op_res_gboolean(G_SIMPLE_ASYNC_RESULT(self->obj), op_res);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_simple_async_result_get_op_res_gboolean(PyGObject *self)
{
    int ret;

    
    ret = g_simple_async_result_get_op_res_gboolean(G_SIMPLE_ASYNC_RESULT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_simple_async_result_set_handle_cancellation(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handle_cancellation", NULL };
    int handle_cancellation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:gio.SimpleAsyncResult.set_handle_cancellation", kwlist, &handle_cancellation))
        return NULL;
    
    g_simple_async_result_set_handle_cancellation(G_SIMPLE_ASYNC_RESULT(self->obj), handle_cancellation);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_simple_async_result_complete(PyGObject *self)
{
    
    g_simple_async_result_complete(G_SIMPLE_ASYNC_RESULT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_simple_async_result_complete_in_idle(PyGObject *self)
{
    
    g_simple_async_result_complete_in_idle(G_SIMPLE_ASYNC_RESULT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_simple_async_result_propagate_error(PyGObject *self)
{
    int ret;
    GError *dest = NULL;

    
    ret = g_simple_async_result_propagate_error(G_SIMPLE_ASYNC_RESULT(self->obj), &dest);
    
    if (pyg_error_check(&dest))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGSimpleAsyncResult_methods[] = {
    { "set_op_res_gssize", (PyCFunction)_wrap_g_simple_async_result_set_op_res_gssize, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_op_res_gssize", (PyCFunction)_wrap_g_simple_async_result_get_op_res_gssize, METH_NOARGS,
      NULL },
    { "set_op_res_gboolean", (PyCFunction)_wrap_g_simple_async_result_set_op_res_gboolean, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_op_res_gboolean", (PyCFunction)_wrap_g_simple_async_result_get_op_res_gboolean, METH_NOARGS,
      NULL },
    { "set_handle_cancellation", (PyCFunction)_wrap_g_simple_async_result_set_handle_cancellation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "complete", (PyCFunction)_wrap_g_simple_async_result_complete, METH_NOARGS,
      NULL },
    { "complete_in_idle", (PyCFunction)_wrap_g_simple_async_result_complete_in_idle, METH_NOARGS,
      NULL },
    { "propagate_error", (PyCFunction)_wrap_g_simple_async_result_propagate_error, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSimpleAsyncResult_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SimpleAsyncResult",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSimpleAsyncResult_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GVfs ----------- */

static PyObject *
_wrap_g_vfs_is_active(PyGObject *self)
{
    int ret;

    
    ret = g_vfs_is_active(G_VFS(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_vfs_get_file_for_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "path", NULL };
    char *path;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.Vfs.get_file_for_path", kwlist, &path))
        return NULL;
    
    ret = g_vfs_get_file_for_path(G_VFS(self->obj), path);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_vfs_get_file_for_uri(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.Vfs.get_file_for_uri", kwlist, &uri))
        return NULL;
    
    ret = g_vfs_get_file_for_uri(G_VFS(self->obj), uri);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_vfs_parse_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parse_name", NULL };
    char *parse_name;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.Vfs.parse_name", kwlist, &parse_name))
        return NULL;
    
    ret = g_vfs_parse_name(G_VFS(self->obj), parse_name);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 369 "gio.override"
static PyObject *
_wrap_g_vfs_get_supported_uri_schemes(PyGObject *self)
{
    const char * const *names;
    PyObject *ret;

    names = g_vfs_get_supported_uri_schemes(G_VFS(self->obj));

    ret = PyList_New(0);
    while (names && *names) {
        PyObject *item = PyString_FromString(names[0]);
        PyList_Append(ret, item);
        Py_DECREF(item);

        names++;
    }

    return ret;
}
#line 10509 "gio.c"


static const PyMethodDef _PyGVfs_methods[] = {
    { "is_active", (PyCFunction)_wrap_g_vfs_is_active, METH_NOARGS,
      NULL },
    { "get_file_for_path", (PyCFunction)_wrap_g_vfs_get_file_for_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_file_for_uri", (PyCFunction)_wrap_g_vfs_get_file_for_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_name", (PyCFunction)_wrap_g_vfs_parse_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_supported_uri_schemes", (PyCFunction)_wrap_g_vfs_get_supported_uri_schemes, METH_NOARGS,
      (char *) "VFS.get_supported_uri_schemes() -> [uri, ..]\n"
"Gets a list of URI schemes supported by vfs." },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGVfs_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Vfs",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGVfs_methods, /* tp_methods */
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



/* ----------- GVolumeMonitor ----------- */

#line 31 "gvolumemonitor.override"
static PyObject *
_wrap_g_volume_monitor_get_connected_drives (PyGObject *self)
{
  GList *list, *l;
  PyObject *ret;
  
  list = g_volume_monitor_get_connected_drives (G_VOLUME_MONITOR (self->obj));

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GDrive *drive = l->data;
    PyObject *item = pygobject_new((GObject *)drive);
    PyList_Append(ret, item);
    Py_DECREF(item);
    g_object_unref(drive);
  }
  g_list_free(list);
  
  return ret;
}
#line 10597 "gio.c"


#line 53 "gvolumemonitor.override"
static PyObject *
_wrap_g_volume_monitor_get_volumes (PyGObject *self)
{
  GList *list, *l;
  PyObject *ret;
  
  list = g_volume_monitor_get_volumes (G_VOLUME_MONITOR (self->obj));

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GVolume *volume = l->data;
    PyObject *item = pygobject_new((GObject *)volume);
    PyList_Append(ret, item);
    Py_DECREF(item);
    g_object_unref(volume);
  }
  g_list_free(list);
  
  return ret;
}
#line 10621 "gio.c"


#line 75 "gvolumemonitor.override"
static PyObject *
_wrap_g_volume_monitor_get_mounts (PyGObject *self)
{
  GList *list, *l;
  PyObject *ret;
  
  list = g_volume_monitor_get_mounts (G_VOLUME_MONITOR (self->obj));

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GMount *mount = l->data;
    PyObject *item = pygobject_new((GObject *)mount);
    PyList_Append(ret, item);
    Py_DECREF(item);
    g_object_unref(mount);
  }
  g_list_free(list);
  
  return ret;
}
#line 10645 "gio.c"


static PyObject *
_wrap_g_volume_monitor_get_volume_for_uuid(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uuid", NULL };
    char *uuid;
    PyObject *py_ret;
    GVolume *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.VolumeMonitor.get_volume_for_uuid", kwlist, &uuid))
        return NULL;
    
    ret = g_volume_monitor_get_volume_for_uuid(G_VOLUME_MONITOR(self->obj), uuid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_volume_monitor_get_mount_for_uuid(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uuid", NULL };
    char *uuid;
    GMount *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.VolumeMonitor.get_mount_for_uuid", kwlist, &uuid))
        return NULL;
    
    ret = g_volume_monitor_get_mount_for_uuid(G_VOLUME_MONITOR(self->obj), uuid);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static const PyMethodDef _PyGVolumeMonitor_methods[] = {
    { "get_connected_drives", (PyCFunction)_wrap_g_volume_monitor_get_connected_drives, METH_NOARGS,
      NULL },
    { "get_volumes", (PyCFunction)_wrap_g_volume_monitor_get_volumes, METH_NOARGS,
      NULL },
    { "get_mounts", (PyCFunction)_wrap_g_volume_monitor_get_mounts, METH_NOARGS,
      NULL },
    { "get_volume_for_uuid", (PyCFunction)_wrap_g_volume_monitor_get_volume_for_uuid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_mount_for_uuid", (PyCFunction)_wrap_g_volume_monitor_get_mount_for_uuid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 24 "gvolumemonitor.override"
static PyObject *
_wrap_g_volume_monitor_tp_new(PyObject *type, PyObject *args, PyObject *kwargs)
{
    return pygobject_new(G_OBJECT(g_volume_monitor_get()));
}
#line 10706 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGVolumeMonitor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.VolumeMonitor",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGVolumeMonitor_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)_wrap_g_volume_monitor_tp_new,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GNativeVolumeMonitor ----------- */

PyTypeObject G_GNUC_INTERNAL PyGNativeVolumeMonitor_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.NativeVolumeMonitor",                   /* tp_name */
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



/* ----------- GFileIcon ----------- */

static int
_wrap_g_file_icon_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "file", NULL };
    PyGObject *file;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.FileIcon.__init__", kwlist, &PyGFile_Type, &file))
        return -1;
    self->obj = (GObject *)g_file_icon_new(G_FILE(file->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GFileIcon object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_g_file_icon_get_file(PyGObject *self)
{
    GFile *ret;

    
    ret = g_file_icon_get_file(G_FILE_ICON(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGFileIcon_methods[] = {
    { "get_file", (PyCFunction)_wrap_g_file_icon_get_file, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 171 "gicon.override"
static PyObject *
_wrap_g_file_icon_tp_repr(PyGObject *self)
{
    GFile *file = g_file_icon_get_file(G_FILE_ICON(self->obj));
    char *uri = (file ? g_file_get_uri(file) : NULL);
    gchar *representation;
    PyObject *result;

    if (uri) {
	representation = g_strdup_printf("<%s at %p: %s>", self->ob_type->tp_name, self, uri);
	g_free(uri);
    }
    else
	representation = g_strdup_printf("<%s at %p: UNKNOWN URI>", self->ob_type->tp_name, self);

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 10863 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGFileIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.FileIcon",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_file_icon_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGFileIcon_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_file_icon_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GThemedIcon ----------- */

#line 194 "gicon.override"
static int
_wrap_g_themed_icon_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "use_default_fallbacks", NULL };
    PyObject *name;
    gboolean use_default_fallbacks = FALSE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|i:gio.ThemedIcon.__init__",
				     kwlist, &name, &use_default_fallbacks))
	return -1;

    if (PyString_Check(name)) {
	pygobject_construct(self,
			    "name", PyString_AsString(name),
			    "use-default-fallbacks", use_default_fallbacks, NULL);
	return 0;
    }
    else if (PySequence_Check(name)) {
	PyObject *tuple = PySequence_Tuple(name);

	if (tuple) {
	    int k;
	    int length = PyTuple_Size(tuple);
	    char **names = g_new(char *, length + 1);

	    for (k = 0; k < length; k++) {
		PyObject *str = PyTuple_GetItem(tuple, k);
		if (str && PyString_Check(str))
		    names[k] = PyString_AsString(str);
		else {
		    Py_DECREF(tuple);
		    g_free(names);
		    goto error;
		}
	    }

	    names[length] = NULL;
	    pygobject_construct(self,
				"names", names,
				"use-default-fallbacks", use_default_fallbacks, NULL);
	    Py_DECREF(tuple);
	    g_free(names);
	    return 0;
	}
    }

 error:
    if (!PyErr_Occurred()) {
	PyErr_SetString(PyExc_TypeError,
			"argument 1 of gio.ThemedIcon.__init__ "
			"must be either a string or a sequence of strings");
    }
    return -1;
}
#line 10970 "gio.c"


static PyObject *
_wrap_g_themed_icon_prepend_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "iconname", NULL };
    char *iconname;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.ThemedIcon.prepend_name", kwlist, &iconname))
        return NULL;
    
    g_themed_icon_prepend_name(G_THEMED_ICON(self->obj), iconname);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 250 "gicon.override"
static PyObject *
_wrap_g_themed_icon_get_names(PyGObject *self)
{
    const char * const *names;
    PyObject *ret;

    names = g_themed_icon_get_names(G_THEMED_ICON(self->obj));

    ret = PyList_New(0);
    while (names && *names) {
        PyObject *item = PyString_FromString(names[0]);
        PyList_Append(ret, item);
        Py_DECREF(item);

        names++;
    }

    return ret;
}
#line 11008 "gio.c"


static PyObject *
_wrap_g_themed_icon_append_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "iconname", NULL };
    char *iconname;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.ThemedIcon.append_name", kwlist, &iconname))
        return NULL;
    
    g_themed_icon_append_name(G_THEMED_ICON(self->obj), iconname);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGThemedIcon_methods[] = {
    { "prepend_name", (PyCFunction)_wrap_g_themed_icon_prepend_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_names", (PyCFunction)_wrap_g_themed_icon_get_names, METH_NOARGS,
      NULL },
    { "append_name", (PyCFunction)_wrap_g_themed_icon_append_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 271 "gicon.override"
static PyObject *
_wrap_g_themed_icon_tp_repr(PyGObject *self)
{
    const char * const *names = g_themed_icon_get_names(G_THEMED_ICON(self->obj));
    GString *representation = g_string_new(NULL);
    PyObject *result;

    g_string_append_printf(representation, "<%s at %p: ", self->ob_type->tp_name, self);

    if (names) {
	gboolean first_name = TRUE;
	while (*names) {
	    if (!first_name)
		g_string_append(representation, ", ");
	    else
		first_name = FALSE;

	    g_string_append(representation, *names++);
	}
    }

    g_string_append(representation, ">");
    result = PyString_FromString(representation->str);
    g_string_free(representation, TRUE);
    return result;
}
#line 11063 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGThemedIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.ThemedIcon",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_themed_icon_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGThemedIcon_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_g_themed_icon_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GAppInfo ----------- */

static PyObject *
_wrap_g_app_info_dup(PyGObject *self)
{
    GAppInfo *ret;
    PyObject *py_ret;

    
    ret = g_app_info_dup(G_APP_INFO(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_app_info_equal(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "appinfo2", NULL };
    PyGObject *appinfo2;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.AppInfo.equal", kwlist, &PyGAppInfo_Type, &appinfo2))
        return NULL;
    
    ret = g_app_info_equal(G_APP_INFO(self->obj), G_APP_INFO(appinfo2->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_get_id(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_app_info_get_id(G_APP_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_app_info_get_name(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_app_info_get_name(G_APP_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_app_info_get_description(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_app_info_get_description(G_APP_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_app_info_get_executable(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_app_info_get_executable(G_APP_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_app_info_get_icon(PyGObject *self)
{
    GIcon *ret;

    
    ret = g_app_info_get_icon(G_APP_INFO(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 123 "gappinfo.override"
static PyObject *
_wrap_g_app_info_launch(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "files", "launch_context", NULL };

    GList *file_list = NULL;
    PyGObject *pycontext = NULL;
    GAppLaunchContext *ctx;
    PyObject *pyfile_list = Py_None;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|OO:gio.AppInfo.launch",
				     kwlist,
				     &pyfile_list, &pycontext))
        return NULL;

    if (!pygio_check_launch_context(pycontext, &ctx))
	return NULL;

    if (pyfile_list == Py_None)
        file_list = NULL;

    else if (PySequence_Check (pyfile_list))
        file_list = pygio_pylist_to_gfile_glist(pyfile_list);

    else {
        PyErr_SetString(PyExc_TypeError,
                        "file_list should be a list of strings or None");
        return NULL;
    }

    ret = g_app_info_launch(G_APP_INFO(self->obj),
                            file_list, ctx, &error);

    g_list_free(file_list);

    if (pyg_error_check(&error))
        return NULL;

    return PyBool_FromLong(ret);
}
#line 11258 "gio.c"


static PyObject *
_wrap_g_app_info_supports_uris(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_supports_uris(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_supports_files(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_supports_files(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 73 "gappinfo.override"
static PyObject *
_wrap_g_app_info_launch_uris(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "files", "launch_context", NULL };

    GList *file_list = NULL;
    PyGObject *pycontext = NULL;
    GAppLaunchContext *ctx;
    PyObject *pyfile_list = Py_None;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|OO:gio.AppInfo.launch_uris",
				     kwlist,
				     &pyfile_list, &pycontext))
        return NULL;

    if (!pygio_check_launch_context(pycontext, &ctx))
	return NULL;

    if (pyfile_list == Py_None)
        file_list = NULL;

    else if (PySequence_Check (pyfile_list))
        file_list = pygio_pylist_to_uri_glist(pyfile_list);

    else {
        PyErr_SetString(PyExc_TypeError,
                        "file_list should be a list of strings or None");
        return NULL;
    }

    ret = g_app_info_launch_uris(G_APP_INFO(self->obj),
                                 file_list, ctx, &error);

    /* in python 3 the C strings are not internal to the Unicode string object
     * so we now strdup when adding element to the list and must free them here
     */
    g_list_foreach (file_list,
                   (GFunc) g_free, NULL);
    g_list_free(file_list);

    if (pyg_error_check(&error))
        return NULL;

    return PyBool_FromLong(ret);
}
#line 11334 "gio.c"


static PyObject *
_wrap_g_app_info_should_show(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_should_show(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_set_as_default_for_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", NULL };
    char *content_type;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.AppInfo.set_as_default_for_type", kwlist, &content_type))
        return NULL;
    
    ret = g_app_info_set_as_default_for_type(G_APP_INFO(self->obj), content_type, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_set_as_default_for_extension(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "extension", NULL };
    char *extension;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.AppInfo.set_as_default_for_extension", kwlist, &extension))
        return NULL;
    
    ret = g_app_info_set_as_default_for_extension(G_APP_INFO(self->obj), extension, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_add_supports_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", NULL };
    char *content_type;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.AppInfo.add_supports_type", kwlist, &content_type))
        return NULL;
    
    ret = g_app_info_add_supports_type(G_APP_INFO(self->obj), content_type, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_can_remove_supports_type(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_can_remove_supports_type(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_remove_supports_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", NULL };
    char *content_type;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.AppInfo.remove_supports_type", kwlist, &content_type))
        return NULL;
    
    ret = g_app_info_remove_supports_type(G_APP_INFO(self->obj), content_type, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_can_delete(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_can_delete(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_delete(PyGObject *self)
{
    int ret;

    
    ret = g_app_info_delete(G_APP_INFO(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_app_info_get_commandline(PyGObject *self)
{
    const gchar *ret;

    
    ret = g_app_info_get_commandline(G_APP_INFO(self->obj));
    
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGAppInfo_methods[] = {
    { "dup", (PyCFunction)_wrap_g_app_info_dup, METH_NOARGS,
      NULL },
    { "equal", (PyCFunction)_wrap_g_app_info_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_id", (PyCFunction)_wrap_g_app_info_get_id, METH_NOARGS,
      NULL },
    { "get_name", (PyCFunction)_wrap_g_app_info_get_name, METH_NOARGS,
      NULL },
    { "get_description", (PyCFunction)_wrap_g_app_info_get_description, METH_NOARGS,
      NULL },
    { "get_executable", (PyCFunction)_wrap_g_app_info_get_executable, METH_NOARGS,
      NULL },
    { "get_icon", (PyCFunction)_wrap_g_app_info_get_icon, METH_NOARGS,
      NULL },
    { "launch", (PyCFunction)_wrap_g_app_info_launch, METH_VARARGS|METH_KEYWORDS,
      (char *) "launch (files=None, launch_context=None) -> gboolean\n"
"\n"
"Launches the application. Passes files to the launched application\n"
"as arguments, using the optional launch_context to get information\n"
"about the details of the launcher (like what screen it is on).\n"
"On error, error will be set accordingly.\n\n"
"Note that even if the launch is successful the application launched\n"
"can fail to start if it runs into problems during startup.\n"
"There is no way to detect this.\n\n"
"Some URIs can be changed when passed through a gio.File\n"
"(for instance unsupported uris with strange formats like mailto:),\n"
"so if you have a textual uri you want to pass in as argument,\n"
"consider using gio.AppInfo.launch_uris() instead." },
    { "supports_uris", (PyCFunction)_wrap_g_app_info_supports_uris, METH_NOARGS,
      NULL },
    { "supports_files", (PyCFunction)_wrap_g_app_info_supports_files, METH_NOARGS,
      NULL },
    { "launch_uris", (PyCFunction)_wrap_g_app_info_launch_uris, METH_VARARGS|METH_KEYWORDS,
      (char *) "launch_uris (files=None, launch_context=None) -> gboolean\n"
"\n"
"Launches the application. Passes files to the launched application\n"
"as arguments, using the optional launch_context to get information\n"
"about the details of the launcher (like what screen it is on).\n"
"On error, error will be set accordingly.\n\n"
"Note that even if the launch is successful the application launched\n"
"can fail to start if it runs into problems during startup.\n"
"There is no way to detect this.\n\n" },
    { "should_show", (PyCFunction)_wrap_g_app_info_should_show, METH_NOARGS,
      NULL },
    { "set_as_default_for_type", (PyCFunction)_wrap_g_app_info_set_as_default_for_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_as_default_for_extension", (PyCFunction)_wrap_g_app_info_set_as_default_for_extension, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_supports_type", (PyCFunction)_wrap_g_app_info_add_supports_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_remove_supports_type", (PyCFunction)_wrap_g_app_info_can_remove_supports_type, METH_NOARGS,
      NULL },
    { "remove_supports_type", (PyCFunction)_wrap_g_app_info_remove_supports_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_delete", (PyCFunction)_wrap_g_app_info_can_delete, METH_NOARGS,
      NULL },
    { "delete", (PyCFunction)_wrap_g_app_info_delete, METH_NOARGS,
      NULL },
    { "get_commandline", (PyCFunction)_wrap_g_app_info_get_commandline, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 199 "gappinfo.override"
static PyObject *
_wrap_g_app_info_tp_repr(PyGObject *self)
{
    const char *name = g_app_info_get_name(G_APP_INFO(self->obj));
    gchar *representation;
    PyObject *result;

    representation = g_strdup_printf("<%s at %p: %s>",
				     self->ob_type->tp_name, self,
				     name ? name : "UNKNOWN NAME");

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 11555 "gio.c"


#line 168 "gappinfo.override"
static PyObject *
_wrap_g_app_info_tp_richcompare(PyGObject *self, PyGObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGAppInfo_Type)
        && PyObject_TypeCheck(other, &PyGAppInfo_Type)) {
        GAppInfo *info1 = G_APP_INFO(self->obj);
        GAppInfo *info2 = G_APP_INFO(other->obj);

        switch (op) {
        case Py_EQ:
            result = (g_app_info_equal(info1, info2)
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (!g_app_info_equal(info1, info2)
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
#line 11588 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGAppInfo_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.AppInfo",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_app_info_tp_repr,             /* tp_repr */
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
    (richcmpfunc)_wrap_g_app_info_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGAppInfo_methods, /* tp_methods */
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



/* ----------- GAsyncInitable ----------- */

static PyObject *
_wrap_g_async_initable_init_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.AsyncInitable.init_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_async_initable_init_finish(G_ASYNC_INITABLE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_async_initable_new_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GObject *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.AsyncInitable.new_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_async_initable_new_finish(G_ASYNC_INITABLE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGAsyncInitable_methods[] = {
    { "init_finish", (PyCFunction)_wrap_g_async_initable_init_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "new_finish", (PyCFunction)_wrap_g_async_initable_new_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGAsyncInitable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.AsyncInitable",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGAsyncInitable_methods, /* tp_methods */
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



/* ----------- GAsyncResult ----------- */

static PyObject *
_wrap_g_async_result_get_source_object(PyGObject *self)
{
    GObject *ret;

    
    ret = g_async_result_get_source_object(G_ASYNC_RESULT(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGAsyncResult_methods[] = {
    { "get_source_object", (PyCFunction)_wrap_g_async_result_get_source_object, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGAsyncResult_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.AsyncResult",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGAsyncResult_methods, /* tp_methods */
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



/* ----------- GDrive ----------- */

static PyObject *
_wrap_g_drive_get_name(PyGObject *self)
{
    gchar *ret;

    
    ret = g_drive_get_name(G_DRIVE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_drive_get_icon(PyGObject *self)
{
    PyObject *py_ret;
    GIcon *ret;

    
    ret = g_drive_get_icon(G_DRIVE(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_drive_has_volumes(PyGObject *self)
{
    int ret;

    
    ret = g_drive_has_volumes(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 26 "gdrive.override"
static PyObject *
_wrap_g_drive_get_volumes (PyGObject *self)
{
  GList *list, *l;
  PyObject *ret;

  pyg_begin_allow_threads;

  list = g_drive_get_volumes (G_DRIVE (self->obj));

  pyg_end_allow_threads;

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GVolume *volume = l->data;
    PyObject *item = pygobject_new((GObject *)volume);
    PyList_Append(ret, item);
    Py_DECREF(item);
    g_object_unref(volume);
  }
  g_list_free(list);

  return ret;
}
#line 11871 "gio.c"


static PyObject *
_wrap_g_drive_is_media_removable(PyGObject *self)
{
    int ret;

    
    ret = g_drive_is_media_removable(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_has_media(PyGObject *self)
{
    int ret;

    
    ret = g_drive_has_media(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_is_media_check_automatic(PyGObject *self)
{
    int ret;

    
    ret = g_drive_is_media_check_automatic(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_can_poll_for_media(PyGObject *self)
{
    int ret;

    
    ret = g_drive_can_poll_for_media(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_can_eject(PyGObject *self)
{
    int ret;

    
    ret = g_drive_can_eject(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 52 "gdrive.override"
static PyObject *
_wrap_g_drive_eject(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:gio.Drive.eject",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;
    
    if (PyErr_Warn(PyExc_DeprecationWarning,
                   "gio.Drive.ejectis deprecated, \
                   use gtk.Drive.eject_with_operation instead") < 0)
        return NULL;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_drive_eject(G_DRIVE(self->obj),
		  flags,
		  cancellable,
		  (GAsyncReadyCallback) async_result_callback_marshal,
		  notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 11986 "gio.c"


static PyObject *
_wrap_g_drive_eject_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Drive.eject_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_drive_eject_finish(G_DRIVE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 105 "gdrive.override"
static PyObject *
_wrap_g_drive_poll_for_media(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OO:gio.Drive.eject",
				     kwlist,
				     &notify->callback,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    pyg_begin_allow_threads;

    g_drive_poll_for_media(G_DRIVE(self->obj),
			   cancellable,
			   (GAsyncReadyCallback) async_result_callback_marshal,
			   notify);
    
    pyg_end_allow_threads;

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 12051 "gio.c"


static PyObject *
_wrap_g_drive_poll_for_media_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Drive.poll_for_media_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_drive_poll_for_media_finish(G_DRIVE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_get_identifier(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "kind", NULL };
    char *kind;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.Drive.get_identifier", kwlist, &kind))
        return NULL;
    
    ret = g_drive_get_identifier(G_DRIVE(self->obj), kind);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 169 "gdrive.override"
static PyObject *
_wrap_g_drive_enumerate_identifiers (PyGObject *self)
{
    char **ids;
    PyObject *ret;
  
    pyg_begin_allow_threads;
  
    ids = g_drive_enumerate_identifiers(G_DRIVE (self->obj));
  
    pyg_end_allow_threads;
  
    if (ids && ids[0] != NULL) {
	ret = strv_to_pylist(ids);
	g_strfreev (ids);
    } else {
	ret = Py_None;
	Py_INCREF(ret);
    }
    return ret;
}
#line 12116 "gio.c"


static PyObject *
_wrap_g_drive_can_start(PyGObject *self)
{
    int ret;

    
    ret = g_drive_can_start(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_can_start_degraded(PyGObject *self)
{
    int ret;

    
    ret = g_drive_can_start_degraded(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_can_stop(PyGObject *self)
{
    int ret;

    
    ret = g_drive_can_stop(G_DRIVE(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 192 "gdrive.override"
static PyObject *
_wrap_g_drive_eject_with_operation(PyGObject *self,
                                   PyObject *args,
                                   PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:gio.Drive.eject_with_operation",
                                     kwlist,
                                     &notify->callback,
                                     &py_flags,
                                     &mount_operation,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_drive_eject_with_operation(G_DRIVE(self->obj),
                          flags,
                          G_MOUNT_OPERATION(mount_operation->obj),
                          cancellable,
                          (GAsyncReadyCallback) async_result_callback_marshal,
                          notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 12208 "gio.c"


static PyObject *
_wrap_g_drive_eject_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Drive.eject_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_drive_eject_with_operation_finish(G_DRIVE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_drive_get_start_stop_type(PyGObject *self)
{
    gint ret;

    
    ret = g_drive_get_start_stop_type(G_DRIVE(self->obj));
    
    return pyg_enum_from_gtype(G_TYPE_DRIVE_START_STOP_TYPE, ret);
}

#line 246 "gdrive.override"
static PyObject *
_wrap_g_drive_start(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GDriveStartFlags flags = G_DRIVE_START_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:gio.Drive.start",
                                     kwlist,
                                     &notify->callback,
                                     &py_flags,
                                     &mount_operation,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_DRIVE_START_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_drive_start(G_DRIVE(self->obj), 
                  flags,
                  G_MOUNT_OPERATION(mount_operation->obj),
                  cancellable,
                  (GAsyncReadyCallback) async_result_callback_marshal,
                  notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 12292 "gio.c"


static PyObject *
_wrap_g_drive_start_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Drive.start_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_drive_start_finish(G_DRIVE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 298 "gdrive.override"
static PyObject *
_wrap_g_drive_stop(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:gio.Drive.stop",
                                     kwlist,
                                     &notify->callback,
                                     &py_flags,
                                     &mount_operation,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_drive_stop(G_DRIVE(self->obj), 
                 flags,
                 G_MOUNT_OPERATION(mount_operation->obj),
                 cancellable,
                 (GAsyncReadyCallback) async_result_callback_marshal,
                 notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 12365 "gio.c"


static PyObject *
_wrap_g_drive_stop_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Drive.stop_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_drive_stop_finish(G_DRIVE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGDrive_methods[] = {
    { "get_name", (PyCFunction)_wrap_g_drive_get_name, METH_NOARGS,
      NULL },
    { "get_icon", (PyCFunction)_wrap_g_drive_get_icon, METH_NOARGS,
      NULL },
    { "has_volumes", (PyCFunction)_wrap_g_drive_has_volumes, METH_NOARGS,
      NULL },
    { "get_volumes", (PyCFunction)_wrap_g_drive_get_volumes, METH_NOARGS,
      NULL },
    { "is_media_removable", (PyCFunction)_wrap_g_drive_is_media_removable, METH_NOARGS,
      NULL },
    { "has_media", (PyCFunction)_wrap_g_drive_has_media, METH_NOARGS,
      NULL },
    { "is_media_check_automatic", (PyCFunction)_wrap_g_drive_is_media_check_automatic, METH_NOARGS,
      NULL },
    { "can_poll_for_media", (PyCFunction)_wrap_g_drive_can_poll_for_media, METH_NOARGS,
      NULL },
    { "can_eject", (PyCFunction)_wrap_g_drive_can_eject, METH_NOARGS,
      NULL },
    { "eject", (PyCFunction)_wrap_g_drive_eject, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_finish", (PyCFunction)_wrap_g_drive_eject_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "poll_for_media", (PyCFunction)_wrap_g_drive_poll_for_media, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "poll_for_media_finish", (PyCFunction)_wrap_g_drive_poll_for_media_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_identifier", (PyCFunction)_wrap_g_drive_get_identifier, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enumerate_identifiers", (PyCFunction)_wrap_g_drive_enumerate_identifiers, METH_NOARGS,
      NULL },
    { "can_start", (PyCFunction)_wrap_g_drive_can_start, METH_NOARGS,
      NULL },
    { "can_start_degraded", (PyCFunction)_wrap_g_drive_can_start_degraded, METH_NOARGS,
      NULL },
    { "can_stop", (PyCFunction)_wrap_g_drive_can_stop, METH_NOARGS,
      NULL },
    { "eject_with_operation", (PyCFunction)_wrap_g_drive_eject_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_with_operation_finish", (PyCFunction)_wrap_g_drive_eject_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_start_stop_type", (PyCFunction)_wrap_g_drive_get_start_stop_type, METH_NOARGS,
      NULL },
    { "start", (PyCFunction)_wrap_g_drive_start, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start_finish", (PyCFunction)_wrap_g_drive_start_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop", (PyCFunction)_wrap_g_drive_stop, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop_finish", (PyCFunction)_wrap_g_drive_stop_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 149 "gdrive.override"
static PyObject *
_wrap_g_drive_tp_repr(PyGObject *self)
{
    char *name = g_drive_get_name(G_DRIVE(self->obj));
    gchar *representation;
    PyObject *result;

    if (name) {
	representation = g_strdup_printf("<%s at %p: %s>", self->ob_type->tp_name, self, name);
	g_free(name);
    }
    else
	representation = g_strdup_printf("<%s at %p: UNKNOWN NAME>", self->ob_type->tp_name, self);

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 12460 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGDrive_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Drive",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_drive_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGDrive_methods, /* tp_methods */
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



/* ----------- GFile ----------- */

static PyObject *
_wrap_g_file_dup(PyGObject *self)
{
    PyObject *py_ret;
    GFile *ret;

    
    ret = g_file_dup(G_FILE(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_equal(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "file2", NULL };
    PyGObject *file2;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.equal", kwlist, &PyGFile_Type, &file2))
        return NULL;
    
    ret = g_file_equal(G_FILE(self->obj), G_FILE(file2->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_get_basename(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_get_basename(G_FILE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_get_path(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_get_path(G_FILE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_get_uri(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_get_uri(G_FILE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_get_parse_name(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_get_parse_name(G_FILE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_get_parent(PyGObject *self)
{
    PyObject *py_ret;
    GFile *ret;

    
    ret = g_file_get_parent(G_FILE(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_get_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.File.get_child", kwlist, &name))
        return NULL;
    
    ret = g_file_get_child(G_FILE(self->obj), name);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_get_child_for_display_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display_name", NULL };
    char *display_name;
    PyObject *py_ret;
    GFile *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.File.get_child_for_display_name", kwlist, &display_name))
        return NULL;
    
    ret = g_file_get_child_for_display_name(G_FILE(self->obj), display_name, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_has_prefix(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "descendant", NULL };
    PyGObject *descendant;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.has_prefix", kwlist, &PyGFile_Type, &descendant))
        return NULL;
    
    ret = g_file_has_prefix(G_FILE(self->obj), G_FILE(descendant->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_get_relative_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "descendant", NULL };
    PyGObject *descendant;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.get_relative_path", kwlist, &PyGFile_Type, &descendant))
        return NULL;
    
    ret = g_file_get_relative_path(G_FILE(self->obj), G_FILE(descendant->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_resolve_relative_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "relative_path", NULL };
    char *relative_path;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.File.resolve_relative_path", kwlist, &relative_path))
        return NULL;
    
    ret = g_file_resolve_relative_path(G_FILE(self->obj), relative_path);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_is_native(PyGObject *self)
{
    int ret;

    
    ret = g_file_is_native(G_FILE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_has_uri_scheme(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri_scheme", NULL };
    char *uri_scheme;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.File.has_uri_scheme", kwlist, &uri_scheme))
        return NULL;
    
    ret = g_file_has_uri_scheme(G_FILE(self->obj), uri_scheme);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_get_uri_scheme(PyGObject *self)
{
    gchar *ret;

    
    ret = g_file_get_uri_scheme(G_FILE(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_file_read(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GFileInputStream *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.read", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_read(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 126 "gfile.override"
static PyObject *
_wrap_g_file_read_async(PyGObject *self,
			PyObject *args,
			PyObject *kwargs)
{
  static char *kwlist[] = { "callback", "io_priority",
			    "cancellable", "user_data", NULL };
  int io_priority = G_PRIORITY_DEFAULT;
  PyGObject *pycancellable = NULL;
  GCancellable *cancellable;
  PyGIONotify *notify;

  notify = pygio_notify_new();

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                   "O|iOO:File.read_async",
                                   kwlist,
                                   &notify->callback,
                                   &io_priority,
                                   &pycancellable,
                                   &notify->data))
      goto error;

  if (!pygio_notify_callback_is_valid(notify))
      goto error;

  if (!pygio_check_cancellable(pycancellable, &cancellable))
      goto error;

  pygio_notify_reference_callback(notify);

  g_file_read_async(G_FILE(self->obj),
                    io_priority,
                    cancellable,
                    (GAsyncReadyCallback)async_result_callback_marshal,
                    notify);

  Py_INCREF(Py_None);
  return Py_None;

 error:
  pygio_notify_free(notify);
  return NULL;
}
#line 12844 "gio.c"


static PyObject *
_wrap_g_file_read_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFileInputStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.read_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_read_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_append_to(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyGObject *py_cancellable = NULL;
    GFileOutputStream *ret;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:gio.File.append_to", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_append_to(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_create(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyGObject *py_cancellable = NULL;
    GFileOutputStream *ret;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:gio.File.create", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_create(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_replace(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "etag", "make_backup", "flags", "cancellable", NULL };
    int make_backup;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *etag;
    PyObject *py_flags = NULL, *py_ret;
    PyGObject *py_cancellable = NULL;
    GFileOutputStream *ret;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si|OO:gio.File.replace", kwlist, &etag, &make_backup, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_replace(G_FILE(self->obj), etag, make_backup, flags, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1139 "gfile.override"
static PyObject *
_wrap_g_file_append_to_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "io_priority",
                              "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OiOO:File.append_to_async",
                                      kwlist,
                                      &notify->callback,
                                      &py_flags, &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_append_to_async(G_FILE(self->obj), flags, io_priority, cancellable,
                           (GAsyncReadyCallback)async_result_callback_marshal,
                           notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13020 "gio.c"


static PyObject *
_wrap_g_file_append_to_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFileOutputStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.append_to_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_append_to_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1187 "gfile.override"
static PyObject *
_wrap_g_file_create_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "io_priority",
                              "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OiOO:File.create_async",
                                      kwlist,
                                      &notify->callback,
                                      &py_flags, &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_create_async(G_FILE(self->obj), flags, io_priority, cancellable,
                        (GAsyncReadyCallback)async_result_callback_marshal,
                        notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13092 "gio.c"


static PyObject *
_wrap_g_file_create_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFileOutputStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.create_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_create_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1385 "gfile.override"
static PyObject *
_wrap_g_file_replace_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "etag", "make_backup", "flags",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *etag = NULL;
    gboolean make_backup = TRUE;
    PyObject *py_backup = Py_True;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|zOOiOO:File.replace_async",
                                      kwlist,
                                      &notify->callback,
                                      &etag, &py_backup,
                                      &py_flags, &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    make_backup = PyObject_IsTrue(py_backup) ? TRUE : FALSE;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_replace_async(G_FILE(self->obj), etag, make_backup, flags,
                         io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13171 "gio.c"


static PyObject *
_wrap_g_file_replace_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFileOutputStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.replace_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_replace_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_query_exists(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.query_exists", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_query_exists(G_FILE(self->obj), (GCancellable *) cancellable);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_query_file_type(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL;
    PyGObject *py_cancellable = NULL;
    GFileQueryInfoFlags flags;
    gint ret;
    GCancellable *cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O|O:gio.File.query_file_type", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_query_file_type(G_FILE(self->obj), flags, (GCancellable *) cancellable);
    
    return pyg_enum_from_gtype(G_TYPE_FILE_TYPE, ret);
}

static PyObject *
_wrap_g_file_query_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attributes;
    GFileInfo *ret;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|OO:gio.File.query_info", kwlist, &attributes, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_query_info(G_FILE(self->obj), attributes, flags, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1440 "gfile.override"
static PyObject *
_wrap_g_file_query_info_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback", "flags",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *attributes;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "sO|OiOO:File.query_info_async",
                                     kwlist,
                                     &attributes,
                                     &notify->callback,
                                     &flags, &io_priority,
                                     &pycancellable,
                                     &notify->data)) {
        /* To preserve compatibility with 2.16 we also allow swapped
         * 'attributes' and 'callback'.  FIXME: Remove for 3.0. */
        static char *old_kwlist[] = { "callback", "attributes", "flags",
                                      "io_priority", "cancellable", "user_data", NULL };
        PyObject *exc_type, *exc_value, *exc_traceback;

        PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);

        if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                         "Os|OiOO:File.query_info_async",
                                         old_kwlist,
                                         &notify->callback,
                                         &attributes,
                                         &flags, &io_priority,
                                         &pycancellable,
                                         &notify->data)
            || !pygio_notify_callback_is_valid(notify)) {
            /* Report the error with new parameters. */
            PyErr_Restore(exc_type, exc_value, exc_traceback);
            goto error;
        }

        Py_XDECREF(exc_type);
        Py_XDECREF(exc_value);
        Py_XDECREF(exc_traceback);
    }

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_query_info_async(G_FILE(self->obj), attributes, flags,
                         io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13359 "gio.c"


static PyObject *
_wrap_g_file_query_info_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.query_info_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_query_info_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_query_filesystem_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "cancellable", NULL };
    PyObject *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attributes;
    GFileInfo *ret;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.File.query_filesystem_info", kwlist, &attributes, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_query_filesystem_info(G_FILE(self->obj), attributes, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1881 "gfile.override"
static PyObject *
_wrap_g_file_query_filesystem_info_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback",
			      "io_priority", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    char *attributes;
    int io_priority = G_PRIORITY_DEFAULT;
    GCancellable *cancellable = NULL;
    PyGObject *py_cancellable = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sO|iOO:gio.File.query_filesystem_info_async",
				     kwlist,
				     &attributes,
				     &notify->callback,
				     &io_priority,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;


    if (!pygio_check_cancellable(py_cancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);

    g_file_query_filesystem_info_async(G_FILE(self->obj),
				       attributes,
				       io_priority,
				       (GCancellable *) cancellable,
				       (GAsyncReadyCallback)async_result_callback_marshal,
				       notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13463 "gio.c"


static PyObject *
_wrap_g_file_query_filesystem_info_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GFileInfo *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.query_filesystem_info_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_query_filesystem_info_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_find_enclosing_mount(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GMount *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.find_enclosing_mount", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_find_enclosing_mount(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 1835 "gfile.override"
static PyObject *
_wrap_g_file_find_enclosing_mount_async(PyGObject *self,
				   PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority",
			      "cancellable", "user_data", NULL };
    int io_priority = G_PRIORITY_DEFAULT;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;
  
    notify = pygio_notify_new();
  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O|iOO:File.enclosing_mount_async",
				     kwlist,
				     &notify->callback,
				     &io_priority,
				     &pycancellable,
				     &notify->data))
	goto error;
  
    if (!pygio_notify_callback_is_valid(notify))
	goto error;
  
    if (!pygio_check_cancellable(pycancellable, &cancellable))
	goto error;
  
    pygio_notify_reference_callback(notify);
  
    g_file_find_enclosing_mount_async(G_FILE(self->obj),
			    io_priority,
			    cancellable,
			    (GAsyncReadyCallback)async_result_callback_marshal,
			    notify);
  
    Py_INCREF(Py_None);
	return Py_None;
  
    error:
	pygio_notify_free(notify);
	return NULL;
}
#line 13561 "gio.c"


static PyObject *
_wrap_g_file_find_enclosing_mount_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GMount *ret;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.find_enclosing_mount_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_find_enclosing_mount_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_enumerate_children(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "flags", "cancellable", NULL };
    GFileEnumerator *ret;
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attributes;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|OO:gio.File.enumerate_children", kwlist, &attributes, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_enumerate_children(G_FILE(self->obj), attributes, flags, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 300 "gfile.override"
static PyObject *
_wrap_g_file_enumerate_children_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attributes", "callback", "flags",
			      "io_priority", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    char *attributes;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    GCancellable *cancellable = NULL;
    PyGObject *py_cancellable = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sO|OiOO:gio.File.enumerate_children_async",
				     kwlist,
				     &attributes,
				     &notify->callback,
				     &py_flags,
				     &io_priority,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);

    g_file_enumerate_children_async(G_FILE(self->obj),
				    attributes,
				    flags,
				    io_priority,
				    (GCancellable *) cancellable,
				    (GAsyncReadyCallback)async_result_callback_marshal,
				    notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13675 "gio.c"


static PyObject *
_wrap_g_file_enumerate_children_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GFileEnumerator *ret;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.enumerate_children_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_enumerate_children_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_set_display_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "display_name", "cancellable", NULL };
    PyObject *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *display_name;
    GFile *ret;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.File.set_display_name", kwlist, &display_name, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_set_display_name(G_FILE(self->obj), display_name, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 2021 "gfile.override"
static PyObject *
_wrap_g_file_set_display_name_async(PyGObject *self,
				    PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "display_name", "callback",
			      "io_priority", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    char *display_name;
    int io_priority = G_PRIORITY_DEFAULT;
    GCancellable *cancellable = NULL;
    PyGObject *py_cancellable = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "sO|iOO:gio.File.set_display_name_async",
				     kwlist,
				     &display_name,
				     &notify->callback,
				     &io_priority,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);

    g_file_set_display_name_async(G_FILE(self->obj),
			    display_name,
			    io_priority,
			    (GCancellable *) cancellable,
			    (GAsyncReadyCallback)async_result_callback_marshal,
			    notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13780 "gio.c"


static PyObject *
_wrap_g_file_set_display_name_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    PyObject *py_ret;
    GFile *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.set_display_name_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_set_display_name_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_delete(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.delete", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_delete(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_trash(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.trash", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_trash(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 570 "gfile.override"
static PyObject *
_wrap_g_file_copy(PyGObject *self,
		  PyObject *args,
		  PyObject *kwargs)
{
    static char *kwlist[] = { "destination", "progress_callback",
			      "flags", "cancellable", 
			      "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *destination = NULL;
    PyGObject *py_cancellable = NULL;
    GFileCopyFlags flags = G_FILE_COPY_NONE;
    GCancellable *cancellable;
    int ret;
    GError *error = NULL;
    GFileProgressCallback callback = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!|OOOO:File.copy",
				     kwlist,
				     &PyGFile_Type,
				     &destination,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (pygio_notify_using_optional_callback(notify)) {
        callback = (GFileProgressCallback)file_progress_callback_marshal;
        if (!pygio_notify_callback_is_valid(notify))
            goto error;
    }

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_COPY_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    /* No need to reference callback here, because it will be used
     * only while this function is in progress. */

    pyg_begin_allow_threads;

    ret = g_file_copy(G_FILE(self->obj),
		      G_FILE(destination->obj),
		      flags,
		      cancellable,
		      callback,
		      notify,
		      &error);

    pyg_end_allow_threads;

    if (pyg_error_check(&error))
        goto error;

    pygio_notify_free(notify);
    return PyBool_FromLong(ret);

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 13931 "gio.c"


#line 641 "gfile.override"
static PyObject *
_wrap_g_file_copy_async(PyGObject *self,
			PyObject *args,
			PyObject *kwargs)
{
  static char *kwlist[] = { "destination", "callback", "progress_callback",
                            "flags", "io_priority", "cancellable",
                            "user_data", "progress_callback_data", NULL };
  PyGIONotify *notify, *progress_notify;
  PyObject *py_flags = NULL;
  PyGObject *destination = NULL;
  PyGObject *py_cancellable = NULL;
  GFileCopyFlags flags = G_FILE_COPY_NONE;
  int io_priority = G_PRIORITY_DEFAULT;
  PyGObject *pycancellable = NULL;
  GCancellable *cancellable;
  GFileProgressCallback progress_callback = NULL;

  /* After the creation, referencing/freeing will automatically be
   * done on the master and the slave. */
  notify = pygio_notify_new();
  progress_notify = pygio_notify_new_slave(notify);

  if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                   "O!O|OOiOOO:File.copy_async",
                                   kwlist,
                                   &PyGFile_Type,
                                   &destination,
                                   &notify->callback,
                                   &progress_notify->callback,
                                   &py_flags,
                                   &io_priority,
                                   &pycancellable,
                                   &notify->data,
                                   &progress_notify->data))
      goto error;

  if (!pygio_notify_callback_is_valid(notify))
      goto error;

  if (!pygio_check_cancellable(py_cancellable, &cancellable))
      goto error;

  if (pygio_notify_using_optional_callback(progress_notify)) {
      progress_callback = (GFileProgressCallback) file_progress_callback_marshal;
      if (!pygio_notify_callback_is_valid_full(progress_notify, "progress_callback"))
          goto error;
  }

  if (!pygio_check_cancellable(pycancellable, &cancellable))
      goto error;

  pygio_notify_reference_callback(notify);

  g_file_copy_async(G_FILE(self->obj),
                    G_FILE(destination->obj),
                    flags,
                    io_priority,
                    cancellable,
                    progress_callback,
                    progress_notify,
                    (GAsyncReadyCallback)async_result_callback_marshal,
                    notify);

  Py_INCREF(Py_None);
  return Py_None;

 error:
  pygio_notify_free(notify);
  return NULL;
}
#line 14006 "gio.c"


static PyObject *
_wrap_g_file_copy_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.copy_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_copy_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 714 "gfile.override"
static PyObject *
_wrap_g_file_move(PyGObject *self,
		  PyObject *args,
		  PyObject *kwargs)
{
    static char *kwlist[] = { "destination", "progress_callback",
			      "flags", "cancellable", 
			      "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *destination = NULL;
    PyGObject *py_cancellable = NULL;
    GFileCopyFlags flags = G_FILE_COPY_NONE;
    GCancellable *cancellable;
    int ret;
    GError *error = NULL;
    GFileProgressCallback callback = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!|OOOO:File.move",
				     kwlist,
				     &PyGFile_Type,
				     &destination,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (pygio_notify_using_optional_callback(notify)) {
        callback = (GFileProgressCallback)file_progress_callback_marshal;
        if (!pygio_notify_callback_is_valid(notify))
            goto error;
    }

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_COPY_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    /* No need to reference callback here, because it will be used
     * only while this function is in progress. */

    pyg_begin_allow_threads;

    ret = g_file_move(G_FILE(self->obj),
		      G_FILE(destination->obj),
		      flags,
		      cancellable,
		      callback,
		      notify,
		      &error);
    
    pyg_end_allow_threads;

    if (pyg_error_check(&error))
        goto error;

    pygio_notify_free(notify);
    return PyBool_FromLong(ret);

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 14098 "gio.c"


static PyObject *
_wrap_g_file_make_directory(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.make_directory", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_make_directory(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_make_directory_with_parents(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.make_directory_with_parents", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_make_directory_with_parents(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_make_symbolic_link(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "symlink_value", "cancellable", NULL };
    char *symlink_value;
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s|O:gio.File.make_symbolic_link", kwlist, &symlink_value, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = g_file_make_symbolic_link(G_FILE(self->obj), symlink_value, (GCancellable *) cancellable, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 1047 "gfile.override"
static PyObject *
_wrap_g_file_query_settable_attributes(PyGObject *self,
                                       PyObject *args,
                                       PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileAttributeInfoList *ret;
    GError *error = NULL;
    gint i, n_infos;
    GFileAttributeInfo *infos;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|O:gio.File.query_settable_attributes",
                                     kwlist, &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    ret = g_file_query_settable_attributes(G_FILE(self->obj),
                                           (GCancellable *) cancellable,
                                           &error);
    if (pyg_error_check(&error))
        return NULL;

    n_infos = ret->n_infos;
    infos = ret->infos;

    if (n_infos > 0) {
        py_ret = PyList_New(n_infos);
        for (i = 0; i < n_infos; i++) {
            PyList_SetItem(py_ret, i, pyg_file_attribute_info_new(&infos[i]));
        }
        g_file_attribute_info_list_unref(ret);
        return py_ret;

    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 14231 "gio.c"


#line 1093 "gfile.override"
static PyObject *
_wrap_g_file_query_writable_namespaces(PyGObject *self,
                                       PyObject *args,
                                       PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileAttributeInfoList *ret;
    GError *error = NULL;
    gint i, n_infos;
    GFileAttributeInfo *infos;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|O:gio.File.query_writable_namespaces",
                                     kwlist, &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    ret = g_file_query_writable_namespaces(G_FILE(self->obj),
                                           (GCancellable *) cancellable,
                                           &error);
    if (pyg_error_check(&error))
        return NULL;

    n_infos = ret->n_infos;
    infos = ret->infos;

    if (n_infos > 0) {
        py_ret = PyList_New(n_infos);
        for (i = 0; i < n_infos; i++) {
            PyList_SetItem(py_ret, i, pyg_file_attribute_info_new(&infos[i]));
        }
        g_file_attribute_info_list_unref(ret);
        return py_ret;

    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 14279 "gio.c"


#line 785 "gfile.override"
static char**
pyg_strv_from_pyobject(PyObject *value, const char *exc_msg)
{
    gchar** strv;
    Py_ssize_t len, i;
    PyObject* fast_seq;

    fast_seq = PySequence_Fast(value, exc_msg);
    if (fast_seq == NULL)
	return NULL;

    len = PySequence_Length(fast_seq);
    if (len == -1)
	return NULL;

    strv = g_malloc(sizeof(char*) * (len + 1));
    if (strv == NULL) {
	PyErr_NoMemory();
	goto failure;
    }

    for (i = 0; i < len + 1; i++)
	strv[i] = NULL;

    for (i = 0; i < len; i++) {
	PyObject* item = PySequence_Fast_GET_ITEM(fast_seq, i);
	const char *s;

	if (!PyString_Check(item)) {
	    PyErr_SetString(PyExc_TypeError, exc_msg);
	    goto failure;
	}

	s = PyString_AsString(item);
	if (s == NULL)
	    goto failure;
		
	strv[i] = g_strdup(s);
	if (strv[i] == NULL) { 
	    PyErr_NoMemory();
	    goto failure;
	}
    }

    return strv;

 failure:
    g_strfreev(strv);
    Py_XDECREF(fast_seq);
    return NULL;
}

static PyObject *
_wrap_g_file_set_attribute(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "type", "value_p",
                              "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    int ret = 0;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute;
    PyObject *py_type = NULL, *py_flags = NULL, *value;
    PyGObject *pycancellable = NULL;
    GFileAttributeType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sOO|OO:gio.File.set_attribute",
                                     kwlist, &attribute, &py_type, &value,
                                     &py_flags, &pycancellable))
        return NULL;

    if (pyg_enum_get_value(G_TYPE_FILE_ATTRIBUTE_TYPE, py_type,
                            (gpointer)&type))
        return NULL;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags,
                                        (gpointer)&flags))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    switch (type) {
    case G_FILE_ATTRIBUTE_TYPE_STRING:
	{
	    char* s;
	    if (!PyString_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be a str when type is FILE_ATTRIBUTE_TYPE_STRING");
		return NULL;
	    }

	    s = PyString_AsString(value);
	    if (s == NULL)
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       s, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_BYTE_STRING:
	{
	    char* s;
	    if (!PyString_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be a bytes instance when type is FILE_ATTRIBUTE_TYPE_BYTE_STRING");
		return NULL;
	    }

	    s = PyString_AsString(value);
	    if (s == NULL)
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       s, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_STRINGV:
	{
	    gchar** strv;
	    
	    strv = pyg_strv_from_pyobject(value, "set_attribute value must be a list of strings when type is FILE_ATTRIBUTE_TYPE_STRINGV");
	    if (strv == NULL)
		break;
	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       strv, flags, (GCancellable *) cancellable,
				       &error);
	    g_strfreev(strv);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_OBJECT:
	{
	    GObject* obj;

	    if (!pygobject_check(value, &PyGObject_Type)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be a GObject instance when type is FILE_ATTRIBUTE_TYPE_OBJECT");
		return NULL;
	    }
		
	    obj = pygobject_get(value);

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       obj, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_BOOLEAN:
	{
	    gboolean boolval;

	    boolval = PyObject_IsTrue(value);
	    if (boolval == -1 && PyErr_Occurred())
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       &boolval, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_UINT32:
	{
	    guint32 intval;

	    if (!PyInt_Check(value) && !PyLong_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be an int when type is FILE_ATTRIBUTE_TYPE_UINT32");
		return NULL;
	    }
		
	    intval = PyLong_AsUnsignedLong(value);
	    if (intval == -1 && PyErr_Occurred())
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       &intval, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_INT32:
	{
	    gint32 intval;

	    if (!PyInt_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be an int when type is FILE_ATTRIBUTE_TYPE_INT32");
		return NULL;
	    }
		
	    intval = PyInt_AsLong(value);
	    if (intval == -1 && PyErr_Occurred())
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       &intval, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_UINT64:
	{
	    guint64 intval;

	    if (!PyInt_Check(value) && !PyLong_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be a long int when type is FILE_ATTRIBUTE_TYPE_UINT64");
		return NULL;
	    }
		
	    intval = PyLong_AsLongLong(value);
	    if (intval == -1 && PyErr_Occurred())
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       &intval, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_INT64:
	{
	    gint64 intval;

	    if (!PyInt_Check(value) && !PyLong_Check(value)) {
		PyErr_Format(PyExc_TypeError, 
			     "set_attribute value must be a long int when type is FILE_ATTRIBUTE_TYPE_INT64");
		return NULL;
	    }
		
	    intval = PyLong_AsUnsignedLongLong(value);
	    if (intval == -1 && PyErr_Occurred())
		return NULL;

	    ret = g_file_set_attribute(G_FILE(self->obj), attribute, type,
				       &intval, flags, (GCancellable *) cancellable,
				       &error);
	}
	break;

    case G_FILE_ATTRIBUTE_TYPE_INVALID:

    default:
        PyErr_SetString(PyExc_TypeError, 
			"Unknown type specified in set_attribute\n");
	return NULL;
    }

    if (pyg_error_check(&error))
        return NULL;

    return PyBool_FromLong(ret);
}
#line 14543 "gio.c"


static PyObject *
_wrap_g_file_set_attributes_from_info(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "info", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    int ret;
    PyGObject *info, *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|OO:gio.File.set_attributes_from_info", kwlist, &PyGFileInfo_Type, &info, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attributes_from_info(G_FILE(self->obj), G_FILE_INFO(info->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 1929 "gfile.override"
static PyObject *
_wrap_g_file_set_attributes_async(PyGObject *self,
				  PyObject *args,
				  PyObject *kwargs)
{
    static char *kwlist[] = { "info", "callback", "flags",
			      "io_priority", "cancellable", "user_data", NULL };
    PyGObject *info;
    PyGIONotify *notify;
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    int io_priority = G_PRIORITY_DEFAULT;
    GCancellable *cancellable = NULL;
    PyGObject *py_cancellable = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "O!O|OiOO:gio.File.set_attributes_async",
				     kwlist,
				     &PyGFileInfo_Type,
				     &info,
				     &notify->callback,
				     &flags,
				     &io_priority,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;


    if (!pygio_check_cancellable(py_cancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);

    g_file_set_attributes_async(G_FILE(self->obj),
			    G_FILE_INFO(info->obj),
			    flags,
			    io_priority,
			    (GCancellable *) cancellable,
			    (GAsyncReadyCallback)async_result_callback_marshal,
			    notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}

#line 14632 "gio.c"


#line 1984 "gfile.override"
static PyObject *
_wrap_g_file_set_attributes_finish(PyGObject *self,
				   PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *res;
    GFileInfo *info = NULL;
    GError *error = NULL;
    gboolean ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:File.set_attributes_finish",
                                      kwlist,
                                      &PyGAsyncResult_Type,
                                      &res))
        return NULL;

    ret = g_file_set_attributes_finish(G_FILE(self->obj),
                                       G_ASYNC_RESULT(res->obj), &info,
                                       &error);

    if (pyg_error_check(&error))
        return NULL;

    if (ret) {
        py_ret = pygobject_new((GObject *)info);
    } else {
        py_ret = Py_None;
        Py_INCREF(py_ret);
    }

    return py_ret;
}
#line 14671 "gio.c"


static PyObject *
_wrap_g_file_set_attribute_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute, *value;
    int ret;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss|OO:gio.File.set_attribute_string", kwlist, &attribute, &value, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_string(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_set_attribute_byte_string(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute, *value;
    int ret;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss|OO:gio.File.set_attribute_byte_string", kwlist, &attribute, &value, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_byte_string(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_set_attribute_uint32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute;
    int ret;
    unsigned long value;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sk|OO:gio.File.set_attribute_uint32", kwlist, &attribute, &value, &py_flags, &py_cancellable))
        return NULL;
    if (value > G_MAXUINT32) {
        PyErr_SetString(PyExc_ValueError,
                        "Value out of range in conversion of"
                        " value parameter to unsigned 32 bit integer");
        return NULL;
    }
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_uint32(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_set_attribute_int32(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    int value, ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute;
    PyObject *py_flags = NULL;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si|OO:gio.File.set_attribute_int32", kwlist, &attribute, &value, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_int32(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_set_attribute_uint64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_value = NULL, *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute;
    int ret;
    guint64 value;
    PyGObject *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!|OO:gio.File.set_attribute_uint64", kwlist, &attribute, &PyLong_Type, &py_value, &py_flags, &py_cancellable))
        return NULL;
    value = PyLong_AsUnsignedLongLong(py_value);
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_uint64(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_set_attribute_int64(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "attribute", "value", "flags", "cancellable", NULL };
    GFileQueryInfoFlags flags = G_FILE_QUERY_INFO_NONE;
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *attribute;
    int ret;
    PyGObject *py_cancellable = NULL;
    gint64 value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sL|OO:gio.File.set_attribute_int64", kwlist, &attribute, &value, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_QUERY_INFO_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_set_attribute_int64(G_FILE(self->obj), attribute, value, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 515 "gfile.override"
static PyObject *
_wrap_g_file_mount_enclosing_volume(PyGObject *self,
				    PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "mount_operation", "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountMountFlags flags = G_MOUNT_MOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O|OOO:File.mount_enclosing_volume",
				     kwlist,
				     &PyGMountOperation_Type,
				     &mount_operation,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_MOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_mount_enclosing_volume(G_FILE(self->obj),
				  flags,
				  G_MOUNT_OPERATION(mount_operation->obj),
				  cancellable,
				  (GAsyncReadyCallback)async_result_callback_marshal,
				  notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 14936 "gio.c"


static PyObject *
_wrap_g_file_mount_enclosing_volume_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.mount_enclosing_volume_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_mount_enclosing_volume_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 355 "gfile.override"
static PyObject *
_wrap_g_file_mount_mountable(PyGObject *self,
			     PyObject *args,
			     PyObject *kwargs)
{
    static char *kwlist[] = { "mount_operation", "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountMountFlags flags = G_MOUNT_MOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!O|OOO:File.mount_mountable",
				     kwlist,
				     &PyGMountOperation_Type,
				     &mount_operation,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_MOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_mount_mountable(G_FILE(self->obj),
			   flags,
			   G_MOUNT_OPERATION(mount_operation->obj),
			   cancellable,
			   (GAsyncReadyCallback)async_result_callback_marshal,
			   notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15012 "gio.c"


static PyObject *
_wrap_g_file_mount_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    PyObject *py_ret;
    GFile *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.mount_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_mount_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 410 "gfile.override"
static PyObject *
_wrap_g_file_unmount_mountable(PyGObject *self,
			     PyObject *args,
			     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:File.unmount_mountable",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_unmount_mountable(G_FILE(self->obj),
			     flags,
			     cancellable,
			     (GAsyncReadyCallback)async_result_callback_marshal,
			     notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15087 "gio.c"


static PyObject *
_wrap_g_file_unmount_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.unmount_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_unmount_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 1734 "gfile.override"
static PyObject *
_wrap_g_file_eject_mountable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags",
                              "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:File.eject_mountable",
                                      kwlist,
                                      &notify->callback,
                                      &flags,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_eject_mountable(G_FILE(self->obj), flags, cancellable,
                           (GAsyncReadyCallback)async_result_callback_marshal,
                           notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15155 "gio.c"


static PyObject *
_wrap_g_file_eject_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.eject_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_eject_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_copy_attributes(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "destination", "flags", "cancellable", NULL };
    int ret;
    GFileCopyFlags flags = G_FILE_COPY_NONE;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyObject *py_flags = NULL;
    PyGObject *destination, *py_cancellable = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|OO:gio.File.copy_attributes", kwlist, &PyGFile_Type, &destination, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_COPY_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_copy_attributes(G_FILE(self->obj), G_FILE(destination->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_monitor_directory(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GFileMonitorFlags flags = G_FILE_MONITOR_NONE;
    PyGObject *py_cancellable = NULL;
    GFileMonitor *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:gio.File.monitor_directory", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_MONITOR_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_monitor_directory(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_monitor_file(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL, *py_ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GFileMonitorFlags flags = G_FILE_MONITOR_NONE;
    PyGObject *py_cancellable = NULL;
    GFileMonitor *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:gio.File.monitor_file", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_MONITOR_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_monitor_file(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_file_monitor(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GFileMonitorFlags flags = G_FILE_MONITOR_NONE;
    PyGObject *py_cancellable = NULL;
    GFileMonitor *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|OO:gio.File.monitor", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_MONITOR_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_monitor(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_query_default_handler(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GAppInfo *ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.query_default_handler", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_query_default_handler(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

#line 172 "gfile.override"
static PyObject *
_wrap_g_file_load_contents(PyGObject *self,
                           PyObject *args,
                           PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    gchar *contents, *etag_out;
    gsize length;
    GError *error = NULL;
    gboolean ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|O:File.load_contents",
                                      kwlist,
                                      &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;

    pyg_begin_allow_threads;

    ret = g_file_load_contents(G_FILE(self->obj), cancellable,
                               &contents, &length, &etag_out, &error);

    pyg_end_allow_threads;

    if (pyg_error_check(&error))
        return NULL;

    if (ret) {
        PyObject *pyret;

        pyret = Py_BuildValue("(s#ks)", contents, length, length, etag_out);
        g_free(contents);
	g_free(etag_out);
        return pyret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 15385 "gio.c"


#line 218 "gfile.override"
static PyObject *
_wrap_g_file_load_contents_async(PyGObject *self,
                                 PyObject *args,
                                 PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OO:File.load_contents_async",
                                      kwlist,
                                      &notify->callback,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_load_contents_async(G_FILE(self->obj),
			       cancellable,
			       (GAsyncReadyCallback)async_result_callback_marshal,
			       notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15429 "gio.c"


#line 260 "gfile.override"
static PyObject *
_wrap_g_file_load_contents_finish(PyGObject *self,
                           PyObject *args,
                           PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    gchar *contents, *etag_out;
    gsize length;
    GError *error = NULL;
    gboolean ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:File.load_contents_finish",
                                      kwlist,
                                      &PyGAsyncResult_Type,
                                      &res))
        return NULL;

    ret = g_file_load_contents_finish(G_FILE(self->obj),
                                      G_ASYNC_RESULT(res->obj), &contents,
                                      &length, &etag_out, &error);

    if (pyg_error_check(&error))
        return NULL;

    if (ret) {
        PyObject *pyret;

        pyret = Py_BuildValue("(s#ks)", contents, length, length, etag_out);
        g_free(contents);
	g_free(etag_out);
        return pyret;
    } else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}
#line 15471 "gio.c"


#line 1516 "gfile.override"
static PyObject *
_wrap_g_file_replace_contents(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "contents", "etag", "make_backup",
                              "flags", "cancellable", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    gsize length;
    gboolean make_backup = FALSE;
    char *contents;
    char *etag = NULL;
    char *new_etag = NULL;
    GError *error = NULL;
    gboolean ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s#|zbOO:File.replace_contents",
                                     kwlist,
                                     &contents,
                                     &length,
                                     &etag,
                                     &make_backup,
                                     &flags,
                                     &cancellable))
    {
        return NULL;
    }

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        return NULL;

    pyg_begin_allow_threads;

    ret = g_file_replace_contents(G_FILE(self->obj), contents, length, etag,
                                  make_backup, flags, &new_etag, cancellable,
                                  &error);

    pyg_end_allow_threads;

    if (pyg_error_check(&error))
        return NULL;

    if (ret) {
        py_ret = PyString_FromString(new_etag);
    } else {
        py_ret = Py_None;
        Py_INCREF(py_ret);
    }

    g_free(new_etag);
    return py_ret;
}
#line 15534 "gio.c"


#line 1614 "gfile.override"
static PyObject *
_wrap_g_file_replace_contents_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "contents", "callback", "etag", "make_backup",
                              "flags", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    PyGIONotify *notify;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    gsize length;
    gboolean make_backup = FALSE;
    char *contents;
    char *etag = NULL;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s#O|zbOOO:File.replace_contents_async",
                                      kwlist,
                                      &contents,
                                      &length,
                                      &notify->callback,
                                      &etag,
                                      &make_backup,
                                      &py_flags,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);
    pygio_notify_copy_buffer(notify, contents, length);

    g_file_replace_contents_async(G_FILE(self->obj),
                                  notify->buffer,
                                  notify->buffer_size,
                                  etag,
                                  make_backup,
                                  flags,
                                  cancellable,
                                  (GAsyncReadyCallback)async_result_callback_marshal,
                                  notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15598 "gio.c"


#line 1577 "gfile.override"
static PyObject *
_wrap_g_file_replace_contents_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *res;
    gchar *etag_out = NULL;
    GError *error = NULL;
    gboolean ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:File.replace_contents_finish",
                                      kwlist,
                                      &PyGAsyncResult_Type,
                                      &res))
        return NULL;

    ret = g_file_replace_contents_finish(G_FILE(self->obj),
                                         G_ASYNC_RESULT(res->obj), &etag_out,
                                         &error);

    if (pyg_error_check(&error))
        return NULL;

    if (ret) {
        py_ret = PyString_FromString(etag_out);
        return py_ret;
    } else {
        py_ret = Py_None;
        Py_INCREF(py_ret);
    }

    g_free(etag_out);
    return py_ret;
}
#line 15637 "gio.c"


static PyObject *
_wrap_g_file_create_readwrite(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", "cancellable", NULL };
    PyObject *py_flags = NULL;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    GFileIOStream *ret;
    PyGObject *py_cancellable = NULL;
    GFileCreateFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O|O:gio.File.create_readwrite", kwlist, &py_flags, &py_cancellable))
        return NULL;
    if (pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_create_readwrite(G_FILE(self->obj), flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1235 "gfile.override"
static PyObject *
_wrap_g_file_create_readwrite_async(PyGObject *self,
				    PyObject *args,
				    PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "io_priority",
                              "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OiOO:File.create_readwrite_async",
                                      kwlist,
                                      &notify->callback,
                                      &py_flags, &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_create_readwrite_async(G_FILE(self->obj), flags, io_priority,
			cancellable,
                        (GAsyncReadyCallback)async_result_callback_marshal,
                        notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15722 "gio.c"


static PyObject *
_wrap_g_file_create_readwrite_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GFileIOStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.create_readwrite_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_create_readwrite_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1781 "gfile.override"
static PyObject *
_wrap_g_file_eject_mountable_with_operation(PyGObject *self,
					    PyObject *args,
					    PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O|OOOO:File.eject_mountable_with_operation",
                                kwlist,
                                &notify->callback,
                                &py_flags,
                                &mount_operation,
                                &py_cancellable,
                                &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_eject_mountable_with_operation(G_FILE(self->obj), 
			    flags,
			    G_MOUNT_OPERATION(mount_operation->obj),
			    cancellable,
			    (GAsyncReadyCallback) async_result_callback_marshal,
			    notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15797 "gio.c"


static PyObject *
_wrap_g_file_eject_mountable_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.eject_mountable_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_eject_mountable_with_operation_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_open_readwrite(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable = NULL;
    GFileIOStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.File.open_readwrite", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_open_readwrite(G_FILE(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1286 "gfile.override"
static PyObject *
_wrap_g_file_open_readwrite_async(PyGObject *self,
                                  PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "io_priority",
                              "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:File.open_readwrite_async",
                                      kwlist,
                                      &notify->callback,
                                      &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_open_readwrite_async(G_FILE(self->obj), io_priority, cancellable,
                        (GAsyncReadyCallback)async_result_callback_marshal,
                        notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15890 "gio.c"


static PyObject *
_wrap_g_file_open_readwrite_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GFileIOStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.open_readwrite_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_open_readwrite_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 2070 "gfile.override"
static PyObject *
_wrap_g_file_poll_mountable(PyGObject *self,
                                  PyObject *args,
                                  PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OO:File.poll_mountable",
                                      kwlist,
                                      &notify->callback,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_poll_mountable(G_FILE(self->obj), cancellable,
                        (GAsyncReadyCallback)async_result_callback_marshal,
                        notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 15952 "gio.c"


static PyObject *
_wrap_g_file_poll_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.poll_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_poll_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_replace_readwrite(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "etag", "make_backup", "flags", "cancellable", NULL };
    int make_backup;
    GCancellable *cancellable = NULL;
    GError *error = NULL;
    char *etag;
    PyObject *py_flags = NULL;
    GFileIOStream *ret;
    PyGObject *py_cancellable = NULL;
    GFileCreateFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"siO|O:gio.File.replace_readwrite", kwlist, &etag, &make_backup, &py_flags, &py_cancellable))
        return NULL;
    if (pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS, py_flags, (gpointer)&flags))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_file_replace_readwrite(G_FILE(self->obj), etag, make_backup, flags, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1330 "gfile.override"
static PyObject *
_wrap_g_file_replace_readwrite_async(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "etag", "make_backup", "flags",
                              "io_priority", "cancellable", "user_data", NULL };
    GCancellable *cancellable;
    PyGObject *pycancellable = NULL;
    GFileCreateFlags flags = G_FILE_CREATE_NONE;
    PyObject *py_flags = NULL;
    int io_priority = G_PRIORITY_DEFAULT;
    char *etag = NULL;
    gboolean make_backup = TRUE;
    PyObject *py_backup = Py_True;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|zOOiOO:File.replace_readwrite_async",
                                      kwlist,
                                      &notify->callback,
                                      &etag, &py_backup,
                                      &flags, &io_priority,
                                      &pycancellable,
                                      &notify->data))
        goto error;

    make_backup = PyObject_IsTrue(py_backup) ? TRUE : FALSE;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_FILE_CREATE_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_replace_readwrite_async(G_FILE(self->obj), etag, make_backup, flags,
                         io_priority, cancellable,
                         (GAsyncReadyCallback)async_result_callback_marshal,
                         notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 16062 "gio.c"


static PyObject *
_wrap_g_file_replace_readwrite_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    GFileIOStream *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.replace_readwrite_finish", kwlist, &PyGAsyncResult_Type, &res))
        return NULL;
    
    ret = g_file_replace_readwrite_finish(G_FILE(self->obj), G_ASYNC_RESULT(res->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 2111 "gfile.override"
static PyObject *
_wrap_g_file_start_mountable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "start_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GDriveStartFlags flags = G_DRIVE_START_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:File.start_mountable",
                                     kwlist,
                                     &notify->callback,
                                     &py_flags,
                                     &mount_operation,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_DRIVE_START_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_start_mountable(G_FILE(self->obj), 
                 flags,
                 G_MOUNT_OPERATION(mount_operation->obj),
                 cancellable,
                 (GAsyncReadyCallback) async_result_callback_marshal,
                 notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 16135 "gio.c"


static PyObject *
_wrap_g_file_start_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.start_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_start_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 2163 "gfile.override"
static PyObject *
_wrap_g_file_stop_mountable(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:gio.File.stop_mountable",
                                     kwlist,
                                     &notify->callback,
                                     &py_flags,
                                     &mount_operation,
                                     &py_cancellable,
                                     &notify->data))
        goto error;
      
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_stop_mountable(G_FILE(self->obj), 
                 flags,
                 G_MOUNT_OPERATION(mount_operation->obj),
                 cancellable,
                 (GAsyncReadyCallback) async_result_callback_marshal,
                 notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}

/* GFile.load_partial_contents_async: No ArgType for GFileReadMoreCallback */
/* GFile.load_partial_contents_finish: No ArgType for char** */
#line 16211 "gio.c"


static PyObject *
_wrap_g_file_stop_mountable_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.stop_mountable_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_stop_mountable_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_file_supports_thread_contexts(PyGObject *self)
{
    int ret;

    
    ret = g_file_supports_thread_contexts(G_FILE(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 461 "gfile.override"
static PyObject *
_wrap_g_file_unmount_mountable_with_operation(PyGObject *self,
                                              PyObject *args,
                                              PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O|OOOO:File.unmount_mountable_with_operation",
                                kwlist,
                                &notify->callback,
                                &py_flags,
				&mount_operation,
                                &py_cancellable,
                                &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_file_unmount_mountable_with_operation(G_FILE(self->obj),
                             flags,
			     G_MOUNT_OPERATION(mount_operation->obj),
                             cancellable,
                             (GAsyncReadyCallback)async_result_callback_marshal,
                             notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 16298 "gio.c"


static PyObject *
_wrap_g_file_unmount_mountable_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.File.unmount_mountable_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_file_unmount_mountable_with_operation_finish(G_FILE(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGFile_methods[] = {
    { "dup", (PyCFunction)_wrap_g_file_dup, METH_NOARGS,
      NULL },
    { "equal", (PyCFunction)_wrap_g_file_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_basename", (PyCFunction)_wrap_g_file_get_basename, METH_NOARGS,
      NULL },
    { "get_path", (PyCFunction)_wrap_g_file_get_path, METH_NOARGS,
      NULL },
    { "get_uri", (PyCFunction)_wrap_g_file_get_uri, METH_NOARGS,
      NULL },
    { "get_parse_name", (PyCFunction)_wrap_g_file_get_parse_name, METH_NOARGS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_g_file_get_parent, METH_NOARGS,
      NULL },
    { "get_child", (PyCFunction)_wrap_g_file_get_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_child_for_display_name", (PyCFunction)_wrap_g_file_get_child_for_display_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_prefix", (PyCFunction)_wrap_g_file_has_prefix, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_relative_path", (PyCFunction)_wrap_g_file_get_relative_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "resolve_relative_path", (PyCFunction)_wrap_g_file_resolve_relative_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_native", (PyCFunction)_wrap_g_file_is_native, METH_NOARGS,
      NULL },
    { "has_uri_scheme", (PyCFunction)_wrap_g_file_has_uri_scheme, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_uri_scheme", (PyCFunction)_wrap_g_file_get_uri_scheme, METH_NOARGS,
      NULL },
    { "read", (PyCFunction)_wrap_g_file_read, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.read([cancellable]) -> input stream\n"
"Opens a file for reading. The result is a GFileInputStream that\n"
"can be used to read the contents of the file.\n"
"\n"
"If cancellable is specified, then the operation can be cancelled\n"
"by triggering the cancellable object from another thread. If the\n"
"operation was cancelled, the error gio.IO_ERROR_CANCELLED will\n"
"be returned. If the file does not exist, the gio.IO_ERROR_NOT_FOUND\n"
"error will be returned. If the file is a directory, the\n"
"gio.IO_ERROR_IS_DIRECTORY error will be returned. Other errors\n"
"are possible too, and depend on what kind of filesystem the file is on." },
    { "read_async", (PyCFunction)_wrap_g_file_read_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.read_async(callback [,io_priority [,cancellable [,user_data]]])\n"
"-> start read\n"
"\n"
"For more details, see gio.File.read() which is the synchronous\n"
"version of this call. Asynchronously opens file for reading.\n"
"When the operation is finished, callback will be called.\n"
"You can then call g_file_read_finish() to get the result of the\n"
"operation.\n" },
    { "read_finish", (PyCFunction)_wrap_g_file_read_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "append_to", (PyCFunction)_wrap_g_file_append_to, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create", (PyCFunction)_wrap_g_file_create, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "replace", (PyCFunction)_wrap_g_file_replace, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "append_to_async", (PyCFunction)_wrap_g_file_append_to_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.append_to_async(callback [flags, [,io_priority [,cancellable\n"
"                  [,user_data]]]]) -> open for append\n"
"\n"
"Asynchronously opens file for appending.\n"
"For more details, see gio.File.append_to() which is the synchronous\n"
"version of this call. When the operation is finished, callback will\n"
"be called. You can then call F.append_to_finish() to get the result\n"
"of the operation." },
    { "append_to_finish", (PyCFunction)_wrap_g_file_append_to_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_async", (PyCFunction)_wrap_g_file_create_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.create_async(callback [flags, [,io_priority [,cancellable\n"
"               [,user_data]]]]) -> file created\n"
"\n"
"Asynchronously creates a new file and returns an output stream for\n"
"writing to it. The file must not already exist.\n"
"For more details, see F.create() which is the synchronous\n"
"version of this call.\n"
"When the operation is finished, callback will be called. You can\n"
"then call F.create_finish() to get the result of the operation." },
    { "create_finish", (PyCFunction)_wrap_g_file_create_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "replace_async", (PyCFunction)_wrap_g_file_replace_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.replace_async(callback [etag, [make_backup, [flags, [io_priority,\n"
"                [cancellable, [user_data]]]]]]) -> file replace\n"
"\n"
"Asynchronously overwrites the file, replacing the contents, possibly\n"
"creating a backup copy of the file first.\n"
"For more details, see F.replace() which is the synchronous\n"
"version of this call.\n"
"When the operation is finished, callback will be called. You can\n"
"then call F.replace_finish() to get the result of the operation." },
    { "replace_finish", (PyCFunction)_wrap_g_file_replace_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_exists", (PyCFunction)_wrap_g_file_query_exists, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_file_type", (PyCFunction)_wrap_g_file_query_file_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info", (PyCFunction)_wrap_g_file_query_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_info_async", (PyCFunction)_wrap_g_file_query_info_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.query_info_async(attributes, callback, [flags, [io_priority,\n"
"                   [cancellable, [user_data]]]]) -> query attributes\n"
"\n"
"Asynchronously gets the requested information about specified file.\n"
"The result is a GFileInfo object that contains key-value attributes\n"
"(such as type or size for the file).\n"
"For more details, see F.query_info() which is the synchronous\n"
"version of this call. \n"
"When the operation is finished, callback will be called. You can\n"
"then call F.query_info_finish() to get the result of the operation.\n" },
    { "query_info_finish", (PyCFunction)_wrap_g_file_query_info_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_filesystem_info", (PyCFunction)_wrap_g_file_query_filesystem_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_filesystem_info_async", (PyCFunction)_wrap_g_file_query_filesystem_info_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_filesystem_info_finish", (PyCFunction)_wrap_g_file_query_filesystem_info_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_enclosing_mount", (PyCFunction)_wrap_g_file_find_enclosing_mount, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_enclosing_mount_async", (PyCFunction)_wrap_g_file_find_enclosing_mount_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_enclosing_mount_finish", (PyCFunction)_wrap_g_file_find_enclosing_mount_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enumerate_children", (PyCFunction)_wrap_g_file_enumerate_children, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.enumerate_children(attributes, [flags, cancellable]) -> enumerator\n"
"Gets the requested information about the files in a directory.\n"
"The result is a gio.FileEnumerator object that will give out gio.FileInfo\n"
"objects for all the files in the directory.\n"
"The attribute value is a string that specifies the file attributes that\n"
"should be gathered. It is not an error if it's not possible to read a \n"
"particular requested attribute from a file - it just won't be set.\n"
"attribute should be a comma-separated list of attribute or attribute\n"
"wildcards. The wildcard \"*\" means all attributes, and a wildcard like\n"
"\"standard::*\" means all attributes in the standard namespace.\n"
"An example attribute query be \"standard::*,owner::user\". The standard\n"
"attributes are available as defines, like gio.FILE_ATTRIBUTE_STANDARD_NAME.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation was\n"
"cancelled, the error gio.ERROR_CANCELLED will be returned.\n"
"\n"
"If the file does not exist, the gio.ERROR_NOT_FOUND error will be returned.\n"
"If the file is not a directory, the gio.FILE_ERROR_NOTDIR error will\n"
"be returned. Other errors are possible too." },
    { "enumerate_children_async", (PyCFunction)_wrap_g_file_enumerate_children_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.enumerate_children_async(attributes, callback,\n"
"                           [flags, io_priority, cancellable, user_data])\n"
"Asynchronously gets the requested information about the files in a\n"
"directory. The result is a GFileEnumerator object that will give out\n"
"GFileInfo objects for all the files in the directory.\n"
"\n"
"For more details, see gio.File.enumerate_children() which is the synchronous\n"
"version of this call.\n"
"\n"
"When the operation is finished, callback will be called. You can then call\n"
"gio.File.enumerate_children_finish() to get the result of the operation." },
    { "enumerate_children_finish", (PyCFunction)_wrap_g_file_enumerate_children_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_display_name", (PyCFunction)_wrap_g_file_set_display_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_display_name_async", (PyCFunction)_wrap_g_file_set_display_name_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_display_name_finish", (PyCFunction)_wrap_g_file_set_display_name_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "delete", (PyCFunction)_wrap_g_file_delete, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "trash", (PyCFunction)_wrap_g_file_trash, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy", (PyCFunction)_wrap_g_file_copy, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.copy(destination, [callback, flags, cancellable, user_data])\n"
"Copies the file source to the location specified by destination.\n"
"Can not handle recursive copies of directories.\n"
"\n"
"If the flag gio.FILE_COPY_OVERWRITE is specified an already existing\n"
"destination file is overwritten.\n"
"\n"
"If the flag gio.FILE_COPY_NOFOLLOW_SYMLINKS is specified then symlink\n"
"will be copied as symlinks, otherwise the target of the source symlink\n"
"will be copied.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled b\n"
"triggering the cancellable object from another thread.\n"
"If the operation was cancelled, the error gio.ERROR_CANCELLED\n"
"will be returned.\n"
"\n"
"If progress_callback is not None, then the operation can be monitored\n"
"by setting this to a callable. if specified progress_callback_data will\n"
"be passed to this function. It is guaranteed that this callback\n"
"will be called after all data has been transferred with the total number\n"
"of bytes copied during the operation.\n"
"\n"
"If the source file does not exist then the gio.ERROR_NOT_FOUND\n"
"error is returned, independent on the status of the destination.\n"
"\n"
"If gio.FILE_COPY_OVERWRITE is not specified and the target exists\n"
"then the error gio.ERROR_EXISTS is returned.\n"
"\n"
"If trying to overwrite a file over a directory the gio.ERROR_IS_DIRECTORY\n"
"error is returned. If trying to overwrite a directory with a directory\n"
"the gio.ERROR_WOULD_MERGE error is returned.\n"
"\n"
"If the source is a directory and the target does not exist\n"
"or gio.FILE_COPY_OVERWRITE is specified and the target is a file\n"
"then the gio.ERROR_WOULD_RECURSE error is returned.\n"
"\n"
"If you are interested in copying the GFile object itself\n"
"(not the on-disk file), see gio.File.dup()." },
    { "copy_async", (PyCFunction)_wrap_g_file_copy_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.copy_async(destination, callback, [flags, io_priority, user_data, cancellable, progress_callback])\n"
"-> start copy\n"
"\n"
"For more details, see gio.File.copy() which is the synchronous\n"
"version of this call. Asynchronously copies file.\n"
"When the operation is finished, callback will be called.\n"
"You can then call g_file_copy_finish() to get the result of the\n"
"operation.\n" },
    { "copy_finish", (PyCFunction)_wrap_g_file_copy_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "move", (PyCFunction)_wrap_g_file_move, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.move(destination, [callback, flags, cancellable, user_data])\n"
"Tries to move the file or directory source to the location\n"
"specified by destination. If native move operations are\n"
"supported then this is used, otherwise a copy + delete fallback\n"
"is used. The native implementation may support moving directories\n"
"(for instance on moves inside the same filesystem), but the \n"
"fallback code does not.\n"
"\n"
"If the flag gio.FILE_COPY_OVERWRITE is specified an already existing\n"
"destination file is overwritten.\n"
"\n"
"If the flag gio.FILE_COPY_NOFOLLOW_SYMLINKS is specified then symlink\n"
"will be copied as symlinks, otherwise the target of the source symlink\n"
"will be copied.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled b\n"
"triggering the cancellable object from another thread.\n"
"If the operation was cancelled, the error gio.ERROR_CANCELLED\n"
"will be returned.\n"
"\n"
"If progress_callback is not None, then the operation can be monitored\n"
"by setting this to a callable. if specified progress_callback_data will\n"
"be passed to this function. It is guaranteed that this callback\n"
"will be called after all data has been transferred with the total number\n"
"of bytes copied during the operation.\n"
"\n"
"If the source file does not exist then the gio.ERROR_NOT_FOUND\n"
"error is returned, independent on the status of the destination.\n"
"\n"
"If gio.FILE_COPY_OVERWRITE is not specified and the target exists\n"
"then the error gio.ERROR_EXISTS is returned.\n"
"\n"
"If trying to overwrite a file over a directory the gio.ERROR_IS_DIRECTORY\n"
"error is returned. If trying to overwrite a directory with a directory\n"
"the gio.ERROR_WOULD_MERGE error is returned.\n"
"\n"
"If the source is a directory and the target does not exist\n"
"or gio.FILE_COPY_OVERWRITE is specified and the target is a file\n"
"then the gio.ERROR_WOULD_RECURSE error is returned." },
    { "make_directory", (PyCFunction)_wrap_g_file_make_directory, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "make_directory_with_parents", (PyCFunction)_wrap_g_file_make_directory_with_parents, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "make_symbolic_link", (PyCFunction)_wrap_g_file_make_symbolic_link, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_settable_attributes", (PyCFunction)_wrap_g_file_query_settable_attributes, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.query_settable_attributes([cancellable]) -> list\n\n"
"Obtain the list of settable attributes for the file.\n"
"Returns the type and full attribute name of all the attributes that\n"
"can be set on this file. This doesn't mean setting it will always\n"
"succeed though, you might get an access failure, or some specific\n"
"file may not support a specific attribute.\n\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned." },
    { "query_writable_namespaces", (PyCFunction)_wrap_g_file_query_writable_namespaces, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.query_writable_namespaces([cancellable]) -> list\n\n"
"Obtain the list of attribute namespaces where new attributes can\n"
"be created by a user. An example of this is extended attributes\n"
"(in the "
"xattr"
" namespace).\n"
"If cancellable is not None, then the operation can be cancelled\n"
"by triggering the cancellable object from another thread. If the\n"
"operation was cancelled, the error gio.IO_ERROR_CANCELLED\n"
"will be returned." },
    { "set_attribute", (PyCFunction)_wrap_g_file_set_attribute, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.set_attribute(attribute, type, value_p [,flags [,cancellable ]])->bool\n"
"\n"
"Sets an attribute in the file with attribute name attribute to value_p.\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned." },
    { "set_attributes_from_info", (PyCFunction)_wrap_g_file_set_attributes_from_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attributes_async", (PyCFunction)_wrap_g_file_set_attributes_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attributes_finish", (PyCFunction)_wrap_g_file_set_attributes_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_string", (PyCFunction)_wrap_g_file_set_attribute_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_byte_string", (PyCFunction)_wrap_g_file_set_attribute_byte_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_uint32", (PyCFunction)_wrap_g_file_set_attribute_uint32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_int32", (PyCFunction)_wrap_g_file_set_attribute_int32, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_uint64", (PyCFunction)_wrap_g_file_set_attribute_uint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_attribute_int64", (PyCFunction)_wrap_g_file_set_attribute_int64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "mount_enclosing_volume", (PyCFunction)_wrap_g_file_mount_enclosing_volume, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.mount_enclosing_volume(mount_operation, callback, [cancellable,\n"
"                         user_data])\n"
"Starts a mount_operation, mounting the volume that contains\n"
"the file location.\n"
"\n"
"When this operation has completed, callback will be called with\n"
"user_user data, and the operation can be finalized with\n"
"gio.File.mount_enclosing_volume_finish().\n"
"\n"
"If cancellable is not None, then the operation can be cancelled\n"
"by triggering the cancellable object from another thread.\n"
"If the operation was cancelled, the error gio.ERROR_CANCELLED\n"
"will be returned." },
    { "mount_enclosing_volume_finish", (PyCFunction)_wrap_g_file_mount_enclosing_volume_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "mount_mountable", (PyCFunction)_wrap_g_file_mount_mountable, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.mount_mountable(mount_operation, callback, [flags, cancellable,\n"
"                  user_data])\n"
"Mounts a file of type gio.FILE_TYPE_MOUNTABLE. Using mount_operation,\n"
"you can request callbacks when, for instance, passwords are needed\n"
"during authentication.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled by\n"
" triggering the cancellable object from another thread. If the\n"
"operation was cancelled, the error gio.ERROR_CANCELLED will be returned.\n"
"\n"
"When the operation is finished, callback will be called. You can then\n"
"call g_file_mount_mountable_finish() to get the result of the operation.\n" },
    { "mount_mountable_finish", (PyCFunction)_wrap_g_file_mount_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unmount_mountable", (PyCFunction)_wrap_g_file_unmount_mountable, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.unmount_mountable(callback, [flags, cancellable, user_data])\n"
"Unmounts a file of type gio.FILE_TYPE_MOUNTABLE.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the\n"
"operation was cancelled, the error gio.ERROR_CANCELLED will be returned.\n"
"\n"
"When the operation is finished, callback will be called. You can\n"
"then call gio.File.unmount_mountable_finish() to get the\n"
"result of the operation.\n" },
    { "unmount_mountable_finish", (PyCFunction)_wrap_g_file_unmount_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_mountable", (PyCFunction)_wrap_g_file_eject_mountable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_mountable_finish", (PyCFunction)_wrap_g_file_eject_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy_attributes", (PyCFunction)_wrap_g_file_copy_attributes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "monitor_directory", (PyCFunction)_wrap_g_file_monitor_directory, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "monitor_file", (PyCFunction)_wrap_g_file_monitor_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "monitor", (PyCFunction)_wrap_g_file_monitor, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_default_handler", (PyCFunction)_wrap_g_file_query_default_handler, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "load_contents", (PyCFunction)_wrap_g_file_load_contents, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.load_contents([cancellable]) -> contents, length, etag_out\n\n"
"Loads the content of the file into memory, returning the size of the\n"
"data. The data is always zero terminated, but this is not included\n"
"in the resultant length.\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned.\n" },
    { "load_contents_async", (PyCFunction)_wrap_g_file_load_contents_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.load_contents_async(callback, [cancellable, [user_data]])->start loading\n\n"
"Starts an asynchronous load of the file's contents.\n\n"
"For more details, see F.load_contents() which is the synchronous\n"
"version of this call.\n\n"
"When the load operation has completed, callback will be called with\n"
"user data. To finish the operation, call F.load_contents_finish() with\n"
"the parameter 'res' returned by the callback.\n\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned.\n" },
    { "load_contents_finish", (PyCFunction)_wrap_g_file_load_contents_finish, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.load_contents_finish(res) -> contents, length, etag_out\n\n"
"Finishes an asynchronous load of the file's contents. The contents are\n"
"placed in contents, and length is set to the size of the contents\n"
"string. If etag_out is present, it will be set to the new entity\n"
"tag for the file.\n" },
    { "replace_contents", (PyCFunction)_wrap_g_file_replace_contents, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.replace_contents(contents, [etag, [make_backup, [flags, [cancellable]]]])\n"
"-> etag_out\n"
"\n"
"Replaces the content of the file, returning the new etag value for the\n"
"file. If an etag is specified, any existing file must have that etag, or\n"
"the error gio.IO_ERROR_WRONG_ETAG will be returned.\n"
"If make_backup is True, this method will attempt to make a backup of the\n"
"file. If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned.\n" },
    { "replace_contents_async", (PyCFunction)_wrap_g_file_replace_contents_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.replace_contents_async(contents, callback, [etag, [make_backup, [flags,\n"
"                         [cancellable]]]]) -> etag_out\n"
"\n"
"Starts an asynchronous replacement of the file with the given contents.\n"
"For more details, see F.replace_contents() which is the synchronous\n"
"version of this call.\n\n"
"When the load operation has completed, callback will be called with\n"
"user data. To finish the operation, call F.replace_contents_finish() with\n"
"the parameter 'res' returned by the callback.\n\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the operation\n"
"was cancelled, the error gio.IO_ERROR_CANCELLED will be returned.\n" },
    { "replace_contents_finish", (PyCFunction)_wrap_g_file_replace_contents_finish, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.replace_contents_finish(res) -> etag_out\n\n"
"Finishes an asynchronous replacement of the file's contents.\n"
"The new entity tag for the file is returned.\n" },
    { "create_readwrite", (PyCFunction)_wrap_g_file_create_readwrite, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_readwrite_async", (PyCFunction)_wrap_g_file_create_readwrite_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_readwrite_finish", (PyCFunction)_wrap_g_file_create_readwrite_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_mountable_with_operation", (PyCFunction)_wrap_g_file_eject_mountable_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_mountable_with_operation_finish", (PyCFunction)_wrap_g_file_eject_mountable_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "open_readwrite", (PyCFunction)_wrap_g_file_open_readwrite, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "open_readwrite_async", (PyCFunction)_wrap_g_file_open_readwrite_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "open_readwrite_finish", (PyCFunction)_wrap_g_file_open_readwrite_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "poll_mountable", (PyCFunction)_wrap_g_file_poll_mountable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "poll_mountable_finish", (PyCFunction)_wrap_g_file_poll_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "replace_readwrite", (PyCFunction)_wrap_g_file_replace_readwrite, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "replace_readwrite_async", (PyCFunction)_wrap_g_file_replace_readwrite_async, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "replace_readwrite_finish", (PyCFunction)_wrap_g_file_replace_readwrite_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start_mountable", (PyCFunction)_wrap_g_file_start_mountable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start_mountable_finish", (PyCFunction)_wrap_g_file_start_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop_mountable", (PyCFunction)_wrap_g_file_stop_mountable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop_mountable_finish", (PyCFunction)_wrap_g_file_stop_mountable_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "supports_thread_contexts", (PyCFunction)_wrap_g_file_supports_thread_contexts, METH_NOARGS,
      NULL },
    { "unmount_mountable_with_operation", (PyCFunction)_wrap_g_file_unmount_mountable_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unmount_mountable_with_operation_finish", (PyCFunction)_wrap_g_file_unmount_mountable_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 1714 "gfile.override"
static PyObject *
_wrap_g_file_tp_repr(PyGObject *self)
{
    char *uri = g_file_get_uri(G_FILE(self->obj));
    gchar *representation;
    PyObject *result;

    if (uri) {
	representation = g_strdup_printf("<%s at %p: %s>", self->ob_type->tp_name, self, uri);
	g_free(uri);
    }
    else
	representation = g_strdup_printf("<%s at %p: UNKNOWN URI>", self->ob_type->tp_name, self);

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 16806 "gio.c"


#line 1707 "gfile.override"
static long
_wrap_g_file_tp_hash(PyGObject *self)
{
    return g_file_hash(G_FILE(self->obj));
}
#line 16815 "gio.c"


#line 1676 "gfile.override"
static PyObject *
_wrap_g_file_tp_richcompare(PyGObject *self, PyGObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGFile_Type)
        && PyObject_TypeCheck(other, &PyGFile_Type)) {
        GFile *file1 = G_FILE(self->obj);
        GFile *file2 = G_FILE(other->obj);

        switch (op) {
        case Py_EQ:
            result = (g_file_equal(file1, file2)
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (!g_file_equal(file1, file2)
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
#line 16848 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGFile_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.File",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_file_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)_wrap_g_file_tp_hash,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    (char *) "File(arg, path=None, uri=None) -> gio.File subclass\n"
"\n"
"If arg is specified; creates a GFile with the given argument from the\n"
"command line.  The value of arg can be either a URI, an absolute path\n"
"or a relative path resolved relative to the current working directory.\n"
"If path is specified, create a file from an absolute or relative path.\n"
"If uri is specified, create a file from a URI.\n\n"
"This operation never fails, but the returned object might not \n"
"support any I/O operation if arg points to a malformed path.",                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_g_file_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGFile_methods, /* tp_methods */
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



/* ----------- GIcon ----------- */

static PyObject *
_wrap_g_icon_equal(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon2", NULL };
    PyGObject *icon2;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Icon.equal", kwlist, &PyGIcon_Type, &icon2))
        return NULL;
    
    ret = g_icon_equal(G_ICON(self->obj), G_ICON(icon2->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_icon_to_string(PyGObject *self)
{
    gchar *ret;

    
    ret = g_icon_to_string(G_ICON(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGIcon_methods[] = {
    { "equal", (PyCFunction)_wrap_g_icon_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_string", (PyCFunction)_wrap_g_icon_to_string, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 60 "gicon.override"
static long
_wrap_g_icon_tp_hash(PyGObject *self)
{
    return g_icon_hash(G_ICON(self->obj));
}
#line 16955 "gio.c"


#line 29 "gicon.override"
static PyObject *
_wrap_g_icon_tp_richcompare(PyGObject *self, PyGObject *other, int op)
{
    PyObject *result;

    if (PyObject_TypeCheck(self, &PyGIcon_Type)
        && PyObject_TypeCheck(other, &PyGIcon_Type)) {
        GIcon *icon1 = G_ICON(self->obj);
        GIcon *icon2 = G_ICON(other->obj);

        switch (op) {
        case Py_EQ:
            result = (g_icon_equal(icon1, icon2)
                      ? Py_True : Py_False);
            break;
        case Py_NE:
            result = (!g_icon_equal(icon1, icon2)
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
#line 16988 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Icon",                   /* tp_name */
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
    (hashfunc)_wrap_g_icon_tp_hash,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_g_icon_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGIcon_methods, /* tp_methods */
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



/* ----------- GInitable ----------- */

static PyObject *
_wrap_g_initable_init(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|O:gio.Initable.init", kwlist, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_initable_init(G_INITABLE(self->obj), (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGInitable_methods[] = {
    { "init", (PyCFunction)_wrap_g_initable_init, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGInitable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Initable",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGInitable_methods, /* tp_methods */
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



/* ----------- GLoadableIcon ----------- */

#line 67 "gicon.override"
static PyObject *
_wrap_g_loadable_icon_load(PyGObject *self,
                           PyObject *args,
                           PyObject *kwargs)
{
    static char *kwlist[] = { "size", "cancellable", NULL };
    int size = 0;
    char *type = NULL;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    GError *error = NULL;
    GInputStream *stream;
    PyObject *result;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|iO:gio.LoadableIcon.load",
				     kwlist,
				     &size, &pycancellable))
        return NULL;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
	return NULL;

    stream = g_loadable_icon_load(G_LOADABLE_ICON(self->obj), size, &type,
				  cancellable, &error);
    if (pyg_error_check(&error))
        return NULL;

    result = Py_BuildValue("Ns", pygobject_new((GObject *) stream), type);
    g_free(type);
    return result;
}
#line 17156 "gio.c"


#line 101 "gicon.override"
static PyObject *
_wrap_g_loadable_icon_load_async(PyGObject *self,
				 PyObject *args,
				 PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "size", "cancellable", "user_data", NULL };
    int size = 0;
    PyGObject *pycancellable = NULL;
    GCancellable *cancellable;
    PyGIONotify *notify;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|iOO:gio.LoadableIcon.load_async",
				     kwlist,
				     &notify->callback, &size, &pycancellable, &notify->data))
	goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(pycancellable, &cancellable))
	goto error;

    pygio_notify_reference_callback(notify);

    g_loadable_icon_load_async(G_LOADABLE_ICON(self->obj),
			       size,
			       cancellable,
			       (GAsyncReadyCallback) async_result_callback_marshal,
			       notify);
    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17199 "gio.c"


#line 142 "gicon.override"
static PyObject *
_wrap_g_loadable_icon_load_finish(PyGObject *self,
				  PyObject *args,
				  PyObject *kwargs)
{
    static char *kwlist[] = { "res", NULL };
    PyGObject *res;
    char *type = NULL;
    GError *error = NULL;
    GInputStream *stream;
    PyObject *result;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:gio.LoadableIcon.load_finish",
				     kwlist,
				     &PyGAsyncResult_Type, &res))
        return NULL;

    stream = g_loadable_icon_load_finish(G_LOADABLE_ICON(self->obj),
					 G_ASYNC_RESULT(res->obj), &type, &error);
    if (pyg_error_check(&error))
        return NULL;

    result = Py_BuildValue("Ns", pygobject_new((GObject *) stream), type);
    g_free(type);
    return result;
}
#line 17230 "gio.c"


static const PyMethodDef _PyGLoadableIcon_methods[] = {
    { "load", (PyCFunction)_wrap_g_loadable_icon_load, METH_VARARGS|METH_KEYWORDS,
      (char *) "ICON.load([size, [cancellable]]) -> input stream, type\n"
"\n"
"Opens a stream of icon data for reading. The result is a tuple of\n"
"gio.InputStream and type (either a string or None). The stream can\n"
"be read to retrieve icon data.\n"
"\n"
"Optional size is a hint at desired icon size. Not all implementations\n"
"support it and the hint will be just ignored in such cases.\n"
"If cancellable is specified, then the operation can be cancelled\n"
"by triggering the cancellable object from another thread. See\n"
"gio.File.read for details." },
    { "load_async", (PyCFunction)_wrap_g_loadable_icon_load_async, METH_VARARGS|METH_KEYWORDS,
      (char *) "ICON.load_async(callback, [size, [cancellable, [user_data]]])\n"
"-> start loading\n"
"\n"
"For more information, see gio.LoadableIcon.load() which is the\n"
"synchronous version of this call. Asynchronously opens icon data for\n"
"reading. When the operation is finished, callback will be called.\n"
"You can then call gio.LoadableIcon.load_finish() to get the result of\n"
"the operation.\n" },
    { "load_finish", (PyCFunction)_wrap_g_loadable_icon_load_finish, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.load_finish(res) -> start loading\n"
"\n"
"Finish asynchronous icon loading operation. Must be called from callback\n"
"as specified to gio.LoadableIcon.load_async. Returns a tuple of\n"
"gio.InputStream and type, just as gio.LoadableIcon.load." },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGLoadableIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.LoadableIcon",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGLoadableIcon_methods, /* tp_methods */
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



/* ----------- GMount ----------- */

static PyObject *
_wrap_g_mount_get_root(PyGObject *self)
{
    PyObject *py_ret;
    GFile *ret;

    
    ret = g_mount_get_root(G_MOUNT(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_mount_get_name(PyGObject *self)
{
    gchar *ret;

    
    ret = g_mount_get_name(G_MOUNT(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_get_icon(PyGObject *self)
{
    PyObject *py_ret;
    GIcon *ret;

    
    ret = g_mount_get_icon(G_MOUNT(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_mount_get_uuid(PyGObject *self)
{
    gchar *ret;

    
    ret = g_mount_get_uuid(G_MOUNT(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_get_volume(PyGObject *self)
{
    PyObject *py_ret;
    GVolume *ret;

    
    ret = g_mount_get_volume(G_MOUNT(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_mount_get_drive(PyGObject *self)
{
    PyObject *py_ret;
    GDrive *ret;

    
    ret = g_mount_get_drive(G_MOUNT(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_mount_can_unmount(PyGObject *self)
{
    int ret;

    
    ret = g_mount_can_unmount(G_MOUNT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_mount_can_eject(PyGObject *self)
{
    int ret;

    
    ret = g_mount_can_eject(G_MOUNT(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 212 "gmount.override"
static PyObject *
_wrap_g_mount_unmount(PyGObject *self,
		      PyObject *args,
		      PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:gio.Mount.unmount",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    pyg_begin_allow_threads;

    g_mount_unmount(G_MOUNT(self->obj),
		    flags,
		    cancellable,
		    (GAsyncReadyCallback)async_result_callback_marshal,
		    notify);

    pyg_end_allow_threads;

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17485 "gio.c"


static PyObject *
_wrap_g_mount_unmount_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Mount.unmount_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_mount_unmount_finish(G_MOUNT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 267 "gmount.override"
static PyObject *
_wrap_g_mount_eject(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:gio.Mount.eject",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    pyg_begin_allow_threads;

    g_mount_eject(G_MOUNT(self->obj),
		  flags,
		  cancellable,
		  (GAsyncReadyCallback) async_result_callback_marshal,
		  notify);

    pyg_end_allow_threads;

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17558 "gio.c"


static PyObject *
_wrap_g_mount_eject_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Mount.eject_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_mount_eject_finish(G_MOUNT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 145 "gmount.override"
static PyObject *
_wrap_g_mount_remount(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    PyObject *py_mount_operation = Py_None;
    GMountOperation *mount_operation = NULL;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOOO:gio.Mount.remount",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_mount_operation,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_mount_operation != Py_None) {
	if (!pygobject_check(py_mount_operation, &PyGMountOperation_Type)) {
	    PyErr_SetString(PyExc_TypeError,
			    "mount_operation must be a gio.MountOperation or None");
            goto error;
	}

	mount_operation = G_MOUNT_OPERATION(pygobject_get(py_mount_operation));
    }

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer) &flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    pyg_begin_allow_threads;

    g_mount_remount(G_MOUNT(self->obj),
		    flags,
		    mount_operation,
		    cancellable,
		    (GAsyncReadyCallback) async_result_callback_marshal,
		    notify);

    pyg_end_allow_threads;

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17646 "gio.c"


static PyObject *
_wrap_g_mount_remount_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Mount.remount_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_mount_remount_finish(G_MOUNT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 24 "gmount.override"
static PyObject *
_wrap_g_mount_guess_content_type(PyGObject *self,
                                 PyObject *args,
                                 PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "force_rescan",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    gboolean force_rescan;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "Oi|OO:Mount.guess_content_type",
                                     kwlist,
                                     &notify->callback,
                                     &force_rescan,
                                     &py_cancellable,
                                     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_mount_guess_content_type(G_MOUNT(self->obj),
                            force_rescan,
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

    error:
       pygio_notify_free(notify);
       return NULL;
}
#line 17713 "gio.c"


#line 70 "gmount.override"
static PyObject *
_wrap_g_mount_guess_content_type_finish(PyGObject *self,
                                        PyObject *args,
                                        PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    GError *error = NULL;
    char **ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!:Mount.guess_content_type_finish",
                                      kwlist,
                                      &PyGAsyncResult_Type,
                                      &result))
        return NULL;

    ret = g_mount_guess_content_type_finish(G_MOUNT(self->obj),
                                         G_ASYNC_RESULT(result->obj), &error);

    if (pyg_error_check(&error))
        return NULL;

    if (ret && ret[0] != NULL) {
        py_ret = strv_to_pylist(ret);
        g_strfreev (ret);
    } else {
        py_ret = Py_None;
        Py_INCREF(py_ret);
    }
    return py_ret;
}
#line 17750 "gio.c"


#line 105 "gmount.override"
static PyObject *
_wrap_g_mount_guess_content_type_sync(PyGObject *self,
                                      PyObject *args,
                                      PyObject *kwargs)
{
    static char *kwlist[] = { "force_rescan", "cancellable", NULL };
    gboolean force_rescan;
    PyGObject *py_cancellable = NULL;
    GCancellable *cancellable;
    GError *error = NULL;
    char **ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "i|O:Mount.guess_content_type_sync",
                                      kwlist,
                                      &force_rescan,
                                      &py_cancellable))
        return NULL;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        return NULL;

    ret = g_mount_guess_content_type_sync(G_MOUNT(self->obj), force_rescan,
                                          cancellable, &error);

    if (pyg_error_check(&error))
        return NULL;

    if (ret && ret[0] != NULL) {
        py_ret = strv_to_pylist(ret);
        g_strfreev (ret);
    } else {
        py_ret = Py_None;
        Py_INCREF(py_ret);
    }
    return py_ret;
}
#line 17792 "gio.c"


static PyObject *
_wrap_g_mount_is_shadowed(PyGObject *self)
{
    int ret;

    
    ret = g_mount_is_shadowed(G_MOUNT(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_mount_shadow(PyGObject *self)
{
    
    g_mount_shadow(G_MOUNT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_mount_unshadow(PyGObject *self)
{
    
    g_mount_unshadow(G_MOUNT(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 349 "gmount.override"
static PyObject *
_wrap_g_mount_unmount_with_operation(PyGObject *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O|OOOO:gio.Mount.unmount_with_operation",
                                kwlist,
                                &notify->callback,
                                &py_flags,
				&mount_operation,
                                &py_cancellable,
                                &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_mount_unmount_with_operation(G_MOUNT(self->obj),
                             flags,
			     G_MOUNT_OPERATION(mount_operation->obj),
                             cancellable,
                             (GAsyncReadyCallback)async_result_callback_marshal,
                             notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17880 "gio.c"


static PyObject *
_wrap_g_mount_unmount_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Mount.unmount_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_mount_unmount_with_operation_finish(G_MOUNT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 403 "gmount.override"
static PyObject *
_wrap_g_mount_eject_with_operation(PyGObject *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O|OOOO:gio.Mount.eject_with_operation",
                                kwlist,
                                &notify->callback,
                                &py_flags,
                                &mount_operation,
                                &py_cancellable,
                                &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_mount_eject_with_operation(G_MOUNT(self->obj),
                            flags,
                            G_MOUNT_OPERATION(mount_operation->obj),
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 17955 "gio.c"


static PyObject *
_wrap_g_mount_eject_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Mount.eject_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_mount_eject_with_operation_finish(G_MOUNT(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGMount_methods[] = {
    { "get_root", (PyCFunction)_wrap_g_mount_get_root, METH_NOARGS,
      NULL },
    { "get_name", (PyCFunction)_wrap_g_mount_get_name, METH_NOARGS,
      NULL },
    { "get_icon", (PyCFunction)_wrap_g_mount_get_icon, METH_NOARGS,
      NULL },
    { "get_uuid", (PyCFunction)_wrap_g_mount_get_uuid, METH_NOARGS,
      NULL },
    { "get_volume", (PyCFunction)_wrap_g_mount_get_volume, METH_NOARGS,
      NULL },
    { "get_drive", (PyCFunction)_wrap_g_mount_get_drive, METH_NOARGS,
      NULL },
    { "can_unmount", (PyCFunction)_wrap_g_mount_can_unmount, METH_NOARGS,
      NULL },
    { "can_eject", (PyCFunction)_wrap_g_mount_can_eject, METH_NOARGS,
      NULL },
    { "unmount", (PyCFunction)_wrap_g_mount_unmount, METH_VARARGS|METH_KEYWORDS,
      (char *) "M.unmount(callback, [flags, cancellable, user_data])\n"
"Unmounts a mount. This is an asynchronous operation, and is finished\n"
"by calling gio.Mount.unmount_finish() with the mount and gio.AsyncResults\n"
"data returned in the callback." },
    { "unmount_finish", (PyCFunction)_wrap_g_mount_unmount_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject", (PyCFunction)_wrap_g_mount_eject, METH_VARARGS|METH_KEYWORDS,
      (char *) "F.eject(callback, [flags, cancellable, user_data])\n"
"Ejects a volume.\n"
"\n"
"If cancellable is not None, then the operation can be cancelled by\n"
"triggering the cancellable object from another thread. If the\n"
"operation was cancelled, the error gio.ERROR_CANCELLED will be returned.\n"
"\n"
"When the operation is finished, callback will be called. You can\n"
"then call gio.Volume.eject_finish() to get the result of the operation.\n" },
    { "eject_finish", (PyCFunction)_wrap_g_mount_eject_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remount", (PyCFunction)_wrap_g_mount_remount, METH_VARARGS|METH_KEYWORDS,
      (char *) "M.remount(callback, [flags, [mount_operation, [cancellable, [user_data]]]])\n"
"Remounts a mount. This is an asynchronous operation, and is finished by\n"
"calling gio.Mount.remount_finish with the mount and gio.AsyncResults data\n"
"returned in the callback." },
    { "remount_finish", (PyCFunction)_wrap_g_mount_remount_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "guess_content_type", (PyCFunction)_wrap_g_mount_guess_content_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "guess_content_type_finish", (PyCFunction)_wrap_g_mount_guess_content_type_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "guess_content_type_sync", (PyCFunction)_wrap_g_mount_guess_content_type_sync, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_shadowed", (PyCFunction)_wrap_g_mount_is_shadowed, METH_NOARGS,
      NULL },
    { "shadow", (PyCFunction)_wrap_g_mount_shadow, METH_NOARGS,
      NULL },
    { "unshadow", (PyCFunction)_wrap_g_mount_unshadow, METH_NOARGS,
      NULL },
    { "unmount_with_operation", (PyCFunction)_wrap_g_mount_unmount_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unmount_with_operation_finish", (PyCFunction)_wrap_g_mount_unmount_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_with_operation", (PyCFunction)_wrap_g_mount_eject_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_with_operation_finish", (PyCFunction)_wrap_g_mount_eject_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 319 "gmount.override"
static PyObject *
_wrap_g_mount_tp_repr(PyGObject *self)
{
    char *name = g_mount_get_name(G_MOUNT(self->obj));
    char *uuid = g_mount_get_uuid(G_MOUNT(self->obj));
    gchar *representation;
    PyObject *result;

    if (name) {
	if (uuid) {
	    representation = g_strdup_printf("<%s at %p: %s (%s)>",
					     self->ob_type->tp_name, self, name, uuid);
	}
	else {
	    representation = g_strdup_printf("<%s at %p: %s>",
					     self->ob_type->tp_name, self, name);
	}
    }
    else
	representation = g_strdup_printf("<%s at %p: UNKNOWN NAME>", self->ob_type->tp_name, self);

    g_free(name);
    g_free(uuid);

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 18072 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGMount_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Mount",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_mount_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGMount_methods, /* tp_methods */
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



/* ----------- GSeekable ----------- */

static PyObject *
_wrap_g_seekable_tell(PyGObject *self)
{
    gint64 ret;

    
    ret = g_seekable_tell(G_SEEKABLE(self->obj));
    
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_g_seekable_can_seek(PyGObject *self)
{
    int ret;

    
    ret = g_seekable_can_seek(G_SEEKABLE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_seekable_seek(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "type", "cancellable", NULL };
    int type = G_SEEK_SET, ret;
    PyGObject *py_cancellable = NULL;
    gint64 offset;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L|iO:gio.Seekable.seek", kwlist, &offset, &type, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_seekable_seek(G_SEEKABLE(self->obj), offset, type, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_seekable_can_truncate(PyGObject *self)
{
    int ret;

    
    ret = g_seekable_can_truncate(G_SEEKABLE(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_seekable_truncate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "cancellable", NULL };
    PyGObject *py_cancellable = NULL;
    int ret;
    gint64 offset;
    GCancellable *cancellable = NULL;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L|O:gio.Seekable.truncate", kwlist, &offset, &py_cancellable))
        return NULL;
    if ((PyObject *)py_cancellable == Py_None)
        cancellable = NULL;
    else if (py_cancellable && pygobject_check(py_cancellable, &PyGCancellable_Type))
        cancellable = G_CANCELLABLE(py_cancellable->obj);
    else if (py_cancellable) {
        PyErr_SetString(PyExc_TypeError, "cancellable should be a GCancellable or None");
        return NULL;
    }
    
    ret = g_seekable_truncate(G_SEEKABLE(self->obj), offset, (GCancellable *) cancellable, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGSeekable_methods[] = {
    { "tell", (PyCFunction)_wrap_g_seekable_tell, METH_NOARGS,
      NULL },
    { "can_seek", (PyCFunction)_wrap_g_seekable_can_seek, METH_NOARGS,
      NULL },
    { "seek", (PyCFunction)_wrap_g_seekable_seek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_truncate", (PyCFunction)_wrap_g_seekable_can_truncate, METH_NOARGS,
      NULL },
    { "truncate", (PyCFunction)_wrap_g_seekable_truncate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSeekable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Seekable",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSeekable_methods, /* tp_methods */
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



/* ----------- GSocketConnectable ----------- */

static PyObject *
_wrap_g_socket_connectable_enumerate(PyGObject *self)
{
    GSocketAddressEnumerator *ret;

    
    ret = g_socket_connectable_enumerate(G_SOCKET_CONNECTABLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGSocketConnectable_methods[] = {
    { "enumerate", (PyCFunction)_wrap_g_socket_connectable_enumerate, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGSocketConnectable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.SocketConnectable",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGSocketConnectable_methods, /* tp_methods */
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



/* ----------- GVolume ----------- */

static PyObject *
_wrap_g_volume_get_name(PyGObject *self)
{
    gchar *ret;

    
    ret = g_volume_get_name(G_VOLUME(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_volume_get_icon(PyGObject *self)
{
    PyObject *py_ret;
    GIcon *ret;

    
    ret = g_volume_get_icon(G_VOLUME(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_volume_get_uuid(PyGObject *self)
{
    gchar *ret;

    
    ret = g_volume_get_uuid(G_VOLUME(self->obj));
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_volume_get_drive(PyGObject *self)
{
    PyObject *py_ret;
    GDrive *ret;

    
    ret = g_volume_get_drive(G_VOLUME(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_volume_get_mount(PyGObject *self)
{
    GMount *ret;
    PyObject *py_ret;

    
    ret = g_volume_get_mount(G_VOLUME(self->obj));
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_volume_can_mount(PyGObject *self)
{
    int ret;

    
    ret = g_volume_can_mount(G_VOLUME(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_volume_can_eject(PyGObject *self)
{
    int ret;

    
    ret = g_volume_can_eject(G_VOLUME(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_volume_should_automount(PyGObject *self)
{
    int ret;

    
    ret = g_volume_should_automount(G_VOLUME(self->obj));
    
    return PyBool_FromLong(ret);

}

#line 24 "gvolume.override"
static PyObject *
_wrap_g_volume_mount(PyGObject *self,
		     PyObject *args,
		     PyObject *kwargs)
{
    static char *kwlist[] = { "mount_operation", "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *py_mount_operation = NULL;
    GMountOperation *mount_operation = NULL;
    PyGObject *py_cancellable = NULL;
    GMountMountFlags flags = G_MOUNT_MOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OO|OOO:Volume.mount",
				     kwlist,
				     &py_mount_operation,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;
    
    if ((PyObject *)py_mount_operation == Py_None)
        mount_operation = NULL;
    
    else if (py_mount_operation && pygobject_check(py_mount_operation,
						   &PyGMountOperation_Type))
        mount_operation = G_MOUNT_OPERATION(py_mount_operation->obj);
    
    else if (py_mount_operation) {
        PyErr_SetString(PyExc_TypeError,
			"mount_operation should be a GMountOperation or None");
        return NULL;
    }
    
    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_MOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_volume_mount(G_VOLUME(self->obj),
		   flags,
		   mount_operation,
		   cancellable,
		   (GAsyncReadyCallback)async_result_callback_marshal,
		   notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 18529 "gio.c"


static PyObject *
_wrap_g_volume_mount_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Volume.mount_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_volume_mount_finish(G_VOLUME(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

#line 92 "gvolume.override"
static PyObject *
_wrap_g_volume_eject(PyGObject *self,
		     PyObject *args,
		     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags",
			      "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O|OOO:Volume.eject",
				     kwlist,
				     &notify->callback,
				     &py_flags,
				     &py_cancellable,
				     &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
					py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_volume_eject(G_VOLUME(self->obj),
		   flags,
		   cancellable,
		   (GAsyncReadyCallback)async_result_callback_marshal,
		   notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 18601 "gio.c"


static PyObject *
_wrap_g_volume_eject_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Volume.eject_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    if (PyErr_Warn(PyExc_DeprecationWarning, "use gio.Drive.eject_with_operation_finish instead.") < 0)
        return NULL;
    
    ret = g_volume_eject_finish(G_VOLUME(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_volume_get_identifier(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "kind", NULL };
    char *kind;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:gio.Volume.get_identifier", kwlist, &kind))
        return NULL;
    
    ret = g_volume_get_identifier(G_VOLUME(self->obj), kind);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 163 "gvolume.override"
static PyObject *
_wrap_g_volume_enumerate_identifiers (PyGObject *self)
{
    char **ids;
    PyObject *ret;
  
    pyg_begin_allow_threads;
  
    ids = g_volume_enumerate_identifiers(G_VOLUME (self->obj));
  
    pyg_end_allow_threads;
  
    if (ids && ids[0] != NULL) {
	ret = strv_to_pylist(ids);
	g_strfreev (ids);
    } else {
	ret = Py_None;
	Py_INCREF(ret);
    }
    return ret;
}
#line 18668 "gio.c"


static PyObject *
_wrap_g_volume_get_activation_root(PyGObject *self)
{
    GFile *ret;

    
    ret = g_volume_get_activation_root(G_VOLUME(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 186 "gvolume.override"
static PyObject *
_wrap_g_volume_eject_with_operation(PyGObject *self,
                                     PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "callback", "flags", "mount_operation",
                              "cancellable", "user_data", NULL };
    PyGIONotify *notify;
    PyObject *py_flags = NULL;
    PyGObject *mount_operation;
    PyGObject *py_cancellable = NULL;
    GMountUnmountFlags flags = G_MOUNT_UNMOUNT_NONE;
    GCancellable *cancellable;

    notify = pygio_notify_new();

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O|OOOO:gio.Volume.eject_with_operation",
                                kwlist,
                                &notify->callback,
                                &py_flags,
                                &mount_operation,
                                &py_cancellable,
                                &notify->data))
        goto error;

    if (!pygio_notify_callback_is_valid(notify))
        goto error;

    if (py_flags && pyg_flags_get_value(G_TYPE_MOUNT_UNMOUNT_FLAGS,
                                        py_flags, (gpointer)&flags))
        goto error;

    if (!pygio_check_cancellable(py_cancellable, &cancellable))
        goto error;

    pygio_notify_reference_callback(notify);

    g_volume_eject_with_operation(G_VOLUME(self->obj),
                            flags,
                            G_MOUNT_OPERATION(mount_operation->obj),
                            cancellable,
                            (GAsyncReadyCallback)async_result_callback_marshal,
                            notify);

    Py_INCREF(Py_None);
    return Py_None;

 error:
    pygio_notify_free(notify);
    return NULL;
}
#line 18736 "gio.c"


static PyObject *
_wrap_g_volume_eject_with_operation_finish(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "result", NULL };
    PyGObject *result;
    int ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:gio.Volume.eject_with_operation_finish", kwlist, &PyGAsyncResult_Type, &result))
        return NULL;
    
    ret = g_volume_eject_with_operation_finish(G_VOLUME(self->obj), G_ASYNC_RESULT(result->obj), &error);
    
    if (pyg_error_check(&error))
        return NULL;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGVolume_methods[] = {
    { "get_name", (PyCFunction)_wrap_g_volume_get_name, METH_NOARGS,
      NULL },
    { "get_icon", (PyCFunction)_wrap_g_volume_get_icon, METH_NOARGS,
      NULL },
    { "get_uuid", (PyCFunction)_wrap_g_volume_get_uuid, METH_NOARGS,
      NULL },
    { "get_drive", (PyCFunction)_wrap_g_volume_get_drive, METH_NOARGS,
      NULL },
    { "get_mount", (PyCFunction)_wrap_g_volume_get_mount, METH_NOARGS,
      NULL },
    { "can_mount", (PyCFunction)_wrap_g_volume_can_mount, METH_NOARGS,
      NULL },
    { "can_eject", (PyCFunction)_wrap_g_volume_can_eject, METH_NOARGS,
      NULL },
    { "should_automount", (PyCFunction)_wrap_g_volume_should_automount, METH_NOARGS,
      NULL },
    { "mount", (PyCFunction)_wrap_g_volume_mount, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "mount_finish", (PyCFunction)_wrap_g_volume_mount_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject", (PyCFunction)_wrap_g_volume_eject, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_finish", (PyCFunction)_wrap_g_volume_eject_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_identifier", (PyCFunction)_wrap_g_volume_get_identifier, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "enumerate_identifiers", (PyCFunction)_wrap_g_volume_enumerate_identifiers, METH_NOARGS,
      NULL },
    { "get_activation_root", (PyCFunction)_wrap_g_volume_get_activation_root, METH_NOARGS,
      NULL },
    { "eject_with_operation", (PyCFunction)_wrap_g_volume_eject_with_operation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "eject_with_operation_finish", (PyCFunction)_wrap_g_volume_eject_with_operation_finish, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 143 "gvolume.override"
static PyObject *
_wrap_g_volume_tp_repr(PyGObject *self)
{
    char *name = g_volume_get_name(G_VOLUME(self->obj));
    gchar *representation;
    PyObject *result;

    if (name) {
	representation = g_strdup_printf("<%s at %p: %s>", self->ob_type->tp_name, self, name);
	g_free(name);
    }
    else
	representation = g_strdup_printf("<%s at %p: UNKNOWN NAME>", self->ob_type->tp_name, self);

    result = PyString_FromString(representation);
    g_free(representation);
    return result;
}
#line 18815 "gio.c"


PyTypeObject G_GNUC_INTERNAL PyGVolume_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gio.Volume",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_g_volume_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGVolume_methods, /* tp_methods */
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

#line 282 "gio.override"
static PyObject *
_wrap_g_app_info_get_all (PyGObject *self)
{
  GList *list, *l;
  PyObject *ret;

  list = g_app_info_get_all ();

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GObject *obj = l->data;
    PyObject *item = pygobject_new(obj);
    PyList_Append(ret, item);
    Py_DECREF(item);
  }
  g_list_free(list);

  return ret;
}
#line 18887 "gio.c"


#line 303 "gio.override"
static PyObject *
_wrap_g_app_info_get_all_for_type (PyGObject *self, PyObject *args)
{
  GList *list, *l;
  PyObject *ret;
  gchar *type;

  if (!PyArg_ParseTuple (args, "s:app_info_get_all_for_type", &type))
    return NULL;

  list = g_app_info_get_all_for_type (type);

  ret = PyList_New(0);
  for (l = list; l; l = l->next) {
    GObject *obj = l->data;
    PyObject *item = pygobject_new(obj);
    PyList_Append(ret, item);
    Py_DECREF(item);
  }
  g_list_free(list);

  return ret;
}
#line 18914 "gio.c"


static PyObject *
_wrap_g_app_info_get_default_for_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", "must_support_uris", NULL };
    char *content_type;
    int must_support_uris;
    GAppInfo *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:app_info_get_default_for_type", kwlist, &content_type, &must_support_uris))
        return NULL;
    
    ret = g_app_info_get_default_for_type(content_type, must_support_uris);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_app_info_get_default_for_uri_scheme(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri_scheme", NULL };
    char *uri_scheme;
    GAppInfo *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:app_info_get_default_for_uri_scheme", kwlist, &uri_scheme))
        return NULL;
    
    ret = g_app_info_get_default_for_uri_scheme(uri_scheme);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_app_info_reset_type_associations(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "content_type", NULL };
    char *content_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:app_info_reset_type_associations", kwlist, &content_type))
        return NULL;
    
    g_app_info_reset_type_associations(content_type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_buffered_input_stream_new_sized(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "base_stream", "size", NULL };
    PyGObject *base_stream;
    gsize size;
    GInputStream *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:buffered_input_stream_new_sized", kwlist, &PyGInputStream_Type, &base_stream, &size))
        return NULL;
    
    ret = g_buffered_input_stream_new_sized(G_INPUT_STREAM(base_stream->obj), size);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_buffered_output_stream_new_sized(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "base_stream", "size", NULL };
    PyGObject *base_stream;
    PyObject *py_size = NULL;
    guint size = 0;
    GOutputStream *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:buffered_output_stream_new_sized", kwlist, &PyGOutputStream_Type, &base_stream, &py_size))
        return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    
    ret = g_buffered_output_stream_new_sized(G_OUTPUT_STREAM(base_stream->obj), size);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_cancellable_get_current(PyObject *self)
{
    GCancellable *ret;

    
    ret = g_cancellable_get_current();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_content_type_equals(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type1", "type2", NULL };
    char *type1, *type2;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:content_type_equals", kwlist, &type1, &type2))
        return NULL;
    
    ret = g_content_type_equals(type1, type2);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_content_type_is_a(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "supertype", NULL };
    char *type, *supertype;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:content_type_is_a", kwlist, &type, &supertype))
        return NULL;
    
    ret = g_content_type_is_a(type, supertype);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_content_type_is_unknown(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    char *type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_is_unknown", kwlist, &type))
        return NULL;
    
    ret = g_content_type_is_unknown(type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_content_type_get_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    char *type;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_get_description", kwlist, &type))
        return NULL;
    
    ret = g_content_type_get_description(type);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_content_type_get_mime_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    char *type;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_get_mime_type", kwlist, &type))
        return NULL;
    
    ret = g_content_type_get_mime_type(type);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_content_type_get_icon(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    char *type;
    PyObject *py_ret;
    GIcon *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_get_icon", kwlist, &type))
        return NULL;
    
    ret = g_content_type_get_icon(type);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_content_type_can_be_executable(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    char *type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_can_be_executable", kwlist, &type))
        return NULL;
    
    ret = g_content_type_can_be_executable(type);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_g_content_type_from_mime_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mime_type", NULL };
    char *mime_type;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:content_type_from_mime_type", kwlist, &mime_type))
        return NULL;
    
    ret = g_content_type_from_mime_type(mime_type);
    
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 328 "gio.override"
static PyObject *
_wrap_g_content_type_guess(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    char *kwlist[] = {"filename", "data", "want_uncertain", NULL};
    char *filename = NULL, *data = NULL, *type;
#ifdef PY_SSIZE_T_CLEAN
    Py_ssize_t data_size = 0;
#else
    int data_size = 0;
#endif

    gboolean result_uncertain, want_uncertain = FALSE;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords (args, kwargs,
				      "|zz#i:g_content_type_guess",
				      kwlist,
				      &filename, &data, &data_size,
				      &want_uncertain))
      return NULL;

    if (!filename && !data) {
      PyErr_SetString(PyExc_TypeError, "need at least one argument");
      return NULL;
    }

    type = g_content_type_guess(filename, (guchar *) data,
				data_size, &result_uncertain);

    if (want_uncertain) {
	ret = Py_BuildValue("zN", type, PyBool_FromLong(result_uncertain));
    
    } else {
        ret = PyString_FromString(type);
    }

    g_free(type);
    return ret;
}
#line 19208 "gio.c"


#line 390 "gio.override"
static PyObject *
_wrap_g_content_types_get_registered(PyObject *self)
{
    GList *list, *l;
    PyObject *ret;

    list = g_content_types_get_registered();

    ret = PyList_New(0);
    for (l = list; l; l = l->next) {
	char *content_type = l->data;
	PyObject *string = PyString_FromString(content_type);
	PyList_Append(ret, string);
	Py_DECREF(string);
	g_free(content_type);
    }
    g_list_free(list);

    return ret;
}
#line 19232 "gio.c"


static PyObject *
_wrap_g_emblem_new_with_origin(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "icon", "origin", NULL };
    PyGObject *icon;
    PyObject *py_origin = NULL;
    GEmblemOrigin origin;
    GEmblem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:emblem_new_with_origin", kwlist, &PyGIcon_Type, &icon, &py_origin))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_EMBLEM_ORIGIN, py_origin, (gpointer)&origin))
        return NULL;
    
    ret = g_emblem_new_with_origin(G_ICON(icon->obj), origin);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_file_parse_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parse_name", NULL };
    char *parse_name;
    PyObject *py_ret;
    GFile *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:file_parse_name", kwlist, &parse_name))
        return NULL;
    
    ret = g_file_parse_name(parse_name);
    
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref(ret);
    return py_ret;
}

static PyObject *
_wrap_g_icon_new_for_string(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "str", NULL };
    char *str;
    GError *error = NULL;
    GIcon *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:icon_new_for_string", kwlist, &str))
        return NULL;
    
    ret = g_icon_new_for_string(str, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_io_error_from_errno(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "err_no", NULL };
    int err_no;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:io_error_from_errno", kwlist, &err_no))
        return NULL;
    
    ret = g_io_error_from_errno(err_no);
    
    return pyg_enum_from_gtype(G_TYPE_IO_ERROR_ENUM, ret);
}

static PyObject *
_wrap_g_inet_address_new_from_string(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    GInetAddress *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:inet_address_new_from_string", kwlist, &string))
        return NULL;
    
    ret = g_inet_address_new_from_string(string);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_inet_address_new_from_bytes(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bytes", "family", NULL };
    PyObject *py_family = NULL;
    GSocketFamily family;
    guchar *bytes;
    GInetAddress *ret;
    Py_ssize_t bytes_len;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s#O:inet_address_new_from_bytes", kwlist, &bytes, &bytes_len, &py_family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    
    ret = g_inet_address_new_from_bytes(bytes, family);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_inet_address_new_loopback(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", NULL };
    PyObject *py_family = NULL;
    GSocketFamily family;
    GInetAddress *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:inet_address_new_loopback", kwlist, &py_family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    
    ret = g_inet_address_new_loopback(family);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_inet_address_new_any(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", NULL };
    PyObject *py_family = NULL;
    GSocketFamily family;
    GInetAddress *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:inet_address_new_any", kwlist, &py_family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    
    ret = g_inet_address_new_any(family);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 59 "gmemoryinputstream.override"
static PyObject *
_wrap_g_memory_input_stream_new_from_data(PyGObject *self,
                                          PyObject *args,
                                          PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject *data;
    GInputStream *stream = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O:gio.memory_input_stream_new_from_data",
                                     kwlist, &data))
        return NULL;

    if (data != Py_None) {
        char *copy;
        int length;

        if (!PyString_Check(data)) {
            PyErr_SetString(PyExc_TypeError, "data must be a string or None");
            return NULL;
        }

        length = PyString_Size(data);
        copy = g_malloc(length);
        memcpy(copy, PyString_AsString(data), length);

        stream = g_memory_input_stream_new_from_data(copy, length,
                                                      (GDestroyNotify) g_free);
    }

    return pygobject_new((GObject *)stream);
}
#line 19417 "gio.c"


static PyObject *
_wrap_g_network_address_parse(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "host_and_port", "default_port", NULL };
    char *host_and_port;
    int default_port;
    GSocketConnectable *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:network_address_parse", kwlist, &host_and_port, &default_port))
        return NULL;
    
    ret = g_network_address_parse(host_and_port, default_port, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_resolver_get_default(PyObject *self)
{
    GResolver *ret;

    
    ret = g_resolver_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_socket_connection_factory_register_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "g_type", "family", "type", "protocol", NULL };
    PyObject *py_g_type = NULL, *py_family = NULL, *py_type = NULL;
    GSocketFamily family;
    GType g_type;
    int protocol;
    GSocketType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOOi:socket_connection_factory_register_type", kwlist, &py_g_type, &py_family, &py_type, &protocol))
        return NULL;
    if ((g_type = pyg_type_from_object(py_g_type)) == 0)
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    g_socket_connection_factory_register_type(g_type, family, type, protocol);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_g_socket_connection_factory_lookup_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "family", "type", "protocol_id", NULL };
    PyObject *py_family = NULL, *py_type = NULL;
    GSocketFamily family;
    int protocol_id;
    GType ret;
    GSocketType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOi:socket_connection_factory_lookup_type", kwlist, &py_family, &py_type, &protocol_id))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_FAMILY, py_family, (gpointer)&family))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_SOCKET_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    ret = g_socket_connection_factory_lookup_type(family, type, protocol_id);
    
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_g_socket_new_from_fd(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fd", NULL };
    int fd;
    GSocket *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:socket_new_from_fd", kwlist, &fd))
        return NULL;
    
    ret = g_socket_new_from_fd(fd, &error);
    
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_vfs_get_default(PyObject *self)
{
    GVfs *ret;

    
    ret = g_vfs_get_default();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_vfs_get_local(PyObject *self)
{
    GVfs *ret;

    
    ret = g_vfs_get_local();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_volume_monitor_get(PyObject *self)
{
    GVolumeMonitor *ret;

    
    ret = g_volume_monitor_get();
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_g_volume_monitor_adopt_orphan_mount(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mount", NULL };
    PyGObject *mount;
    GVolume *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:volume_monitor_adopt_orphan_mount", kwlist, &PyGMount_Type, &mount))
        return NULL;
    
    ret = g_volume_monitor_adopt_orphan_mount(G_MOUNT(mount->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 72 "gfile.override"
static PyObject*
_wrap__file_init(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GFile *file;
    Py_ssize_t n_args, n_kwargs;
    char *arg;
    PyObject *py_ret;

    n_args = PyTuple_Size(args);
    n_kwargs = kwargs != NULL ? PyDict_Size(kwargs) : 0;

    if (n_args == 1 && n_kwargs == 0) {
	if (!PyArg_ParseTuple(args, "s:gio.File.__init__", &arg))
	    return NULL;
	file = g_file_new_for_commandline_arg(arg);
    } else if (n_args == 0 && n_kwargs == 1) {
	if (PyDict_GetItemString(kwargs, "path")) {
	    char *kwlist[] = { "path", NULL };
	    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
					     "s:gio.File.__init__", kwlist, &arg))
		return NULL;
	    file = g_file_new_for_path(arg);
	} else if (PyDict_GetItemString(kwargs, "uri")) {
	    char *kwlist[] = { "uri", NULL };
	    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
					     "s:gio.File.__init__", kwlist, &arg))
		return NULL;
	    file = g_file_new_for_uri(arg);
	} else {
	    PyErr_Format(PyExc_TypeError,
			 "gio.File() got an unexpected keyword argument '%s'",
			 "unknown");
	    return NULL;
	}
    } else {
	PyErr_Format(PyExc_TypeError,
		     "gio.File() takes exactly 1 argument (%zd given)",
		     n_args + n_kwargs);
	return NULL;
    }

    if (!file) {
        PyErr_SetString(PyExc_RuntimeError,
			"could not create GFile object");
        return NULL;
    }

    py_ret = pygobject_new((GObject *)file);
    g_object_unref(file);

    return py_ret;
}
#line 19623 "gio.c"


#line 56 "gfile.override"
static PyObject *
_wrap__install_file_meta(PyObject *self, PyObject *args)
{
    PyObject *metaclass;

    if (!PyArg_ParseTuple(args, "O", &metaclass))
	return NULL;

    Py_INCREF(metaclass);
    PyGFile_Type.ob_type = (PyTypeObject*)metaclass;

    Py_INCREF(Py_None);
    return Py_None;
}
#line 19641 "gio.c"


#line 25 "gappinfo.override"
static PyObject *
_wrap__install_app_info_meta(PyObject *self, PyObject *args)
{
    PyObject *metaclass;

    if (!PyArg_ParseTuple(args, "O", &metaclass))
	return NULL;

    Py_INCREF(metaclass);
    PyGAppInfo_Type.ob_type = (PyTypeObject*)metaclass;

    Py_INCREF(Py_None);
    return Py_None;
}
#line 19659 "gio.c"


#line 41 "gappinfo.override"
static PyObject *
_wrap__app_info_init(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "commandline", "application_name",
			      "flags", NULL };
    char *commandline, *application_name = NULL;
    PyObject *py_flags = NULL;
    GAppInfo *ret;
    GError *error = NULL;
    GAppInfoCreateFlags flags = G_APP_INFO_CREATE_NONE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "s|zO:gio.AppInfo",
				     kwlist,
				     &commandline, &application_name,
				     &py_flags))
        return NULL;
    if (py_flags && pyg_flags_get_value(G_TYPE_APP_INFO_CREATE_FLAGS,
					py_flags, (gpointer)&flags))
        return NULL;

    ret = g_app_info_create_from_commandline(commandline,
					     application_name, flags, &error);

    if (pyg_error_check(&error))
        return NULL;

    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}
#line 19693 "gio.c"


const PyMethodDef pygio_functions[] = {
    { "app_info_get_all", (PyCFunction)_wrap_g_app_info_get_all, METH_NOARGS,
      NULL },
    { "app_info_get_all_for_type", (PyCFunction)_wrap_g_app_info_get_all_for_type, METH_VARARGS,
      NULL },
    { "app_info_get_default_for_type", (PyCFunction)_wrap_g_app_info_get_default_for_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "app_info_get_default_for_uri_scheme", (PyCFunction)_wrap_g_app_info_get_default_for_uri_scheme, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "app_info_reset_type_associations", (PyCFunction)_wrap_g_app_info_reset_type_associations, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "buffered_input_stream_new_sized", (PyCFunction)_wrap_g_buffered_input_stream_new_sized, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "buffered_output_stream_new_sized", (PyCFunction)_wrap_g_buffered_output_stream_new_sized, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "cancellable_get_current", (PyCFunction)_wrap_g_cancellable_get_current, METH_NOARGS,
      NULL },
    { "content_type_equals", (PyCFunction)_wrap_g_content_type_equals, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_is_a", (PyCFunction)_wrap_g_content_type_is_a, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_is_unknown", (PyCFunction)_wrap_g_content_type_is_unknown, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_get_description", (PyCFunction)_wrap_g_content_type_get_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_get_mime_type", (PyCFunction)_wrap_g_content_type_get_mime_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_get_icon", (PyCFunction)_wrap_g_content_type_get_icon, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_can_be_executable", (PyCFunction)_wrap_g_content_type_can_be_executable, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_from_mime_type", (PyCFunction)_wrap_g_content_type_from_mime_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "content_type_guess", (PyCFunction)_wrap_g_content_type_guess, METH_VARARGS|METH_KEYWORDS,
      (char *) "content_type_guess([filename, data, want_uncertain]) -> mime type\n"
"\n"
"Guesses the content type based on the parameters passed.\n"
"Either filename or data must be specified\n"
"Returns a string containing the mime type.\n"
"If want_uncertain is set to True, return a tuple with the mime type and \n"
"True/False if the type guess was uncertain or not." },
    { "content_types_get_registered", (PyCFunction)_wrap_g_content_types_get_registered, METH_NOARGS,
      NULL },
    { "emblem_new_with_origin", (PyCFunction)_wrap_g_emblem_new_with_origin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "file_parse_name", (PyCFunction)_wrap_g_file_parse_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "icon_new_for_string", (PyCFunction)_wrap_g_icon_new_for_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "io_error_from_errno", (PyCFunction)_wrap_g_io_error_from_errno, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "inet_address_new_from_string", (PyCFunction)_wrap_g_inet_address_new_from_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "inet_address_new_from_bytes", (PyCFunction)_wrap_g_inet_address_new_from_bytes, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "inet_address_new_loopback", (PyCFunction)_wrap_g_inet_address_new_loopback, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "inet_address_new_any", (PyCFunction)_wrap_g_inet_address_new_any, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "memory_input_stream_new_from_data", (PyCFunction)_wrap_g_memory_input_stream_new_from_data, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "network_address_parse", (PyCFunction)_wrap_g_network_address_parse, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "resolver_get_default", (PyCFunction)_wrap_g_resolver_get_default, METH_NOARGS,
      NULL },
    { "socket_connection_factory_register_type", (PyCFunction)_wrap_g_socket_connection_factory_register_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "socket_connection_factory_lookup_type", (PyCFunction)_wrap_g_socket_connection_factory_lookup_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "socket_new_from_fd", (PyCFunction)_wrap_g_socket_new_from_fd, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "vfs_get_default", (PyCFunction)_wrap_g_vfs_get_default, METH_NOARGS,
      NULL },
    { "vfs_get_local", (PyCFunction)_wrap_g_vfs_get_local, METH_NOARGS,
      NULL },
    { "volume_monitor_get", (PyCFunction)_wrap_g_volume_monitor_get, METH_NOARGS,
      NULL },
    { "volume_monitor_adopt_orphan_mount", (PyCFunction)_wrap_g_volume_monitor_adopt_orphan_mount, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "_file_init", (PyCFunction)_wrap__file_init, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "_install_file_meta", (PyCFunction)_wrap__install_file_meta, METH_VARARGS,
      NULL },
    { "_install_app_info_meta", (PyCFunction)_wrap__install_app_info_meta, METH_VARARGS,
      NULL },
    { "_app_info_init", (PyCFunction)_wrap__app_info_init, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pygio_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_flags_add(module, "AppInfoCreateFlags", strip_prefix, G_TYPE_APP_INFO_CREATE_FLAGS);
  pyg_flags_add(module, "ConverterFlags", strip_prefix, G_TYPE_CONVERTER_FLAGS);
  pyg_enum_add(module, "ConverterResult", strip_prefix, G_TYPE_CONVERTER_RESULT);
  pyg_enum_add(module, "DataStreamByteOrder", strip_prefix, G_TYPE_DATA_STREAM_BYTE_ORDER);
  pyg_enum_add(module, "DataStreamNewlineType", strip_prefix, G_TYPE_DATA_STREAM_NEWLINE_TYPE);
  pyg_enum_add(module, "FileAttributeType", strip_prefix, G_TYPE_FILE_ATTRIBUTE_TYPE);
  pyg_flags_add(module, "FileAttributeInfoFlags", strip_prefix, G_TYPE_FILE_ATTRIBUTE_INFO_FLAGS);
  pyg_enum_add(module, "FileAttributeStatus", strip_prefix, G_TYPE_FILE_ATTRIBUTE_STATUS);
  pyg_flags_add(module, "FileQueryInfoFlags", strip_prefix, G_TYPE_FILE_QUERY_INFO_FLAGS);
  pyg_flags_add(module, "FileCreateFlags", strip_prefix, G_TYPE_FILE_CREATE_FLAGS);
  pyg_enum_add(module, "MountMountFlags", strip_prefix, G_TYPE_MOUNT_MOUNT_FLAGS);
  pyg_flags_add(module, "MountUnmountFlags", strip_prefix, G_TYPE_MOUNT_UNMOUNT_FLAGS);
  pyg_enum_add(module, "DriveStartFlags", strip_prefix, G_TYPE_DRIVE_START_FLAGS);
  pyg_enum_add(module, "DriveStartStopType", strip_prefix, G_TYPE_DRIVE_START_STOP_TYPE);
  pyg_flags_add(module, "FileCopyFlags", strip_prefix, G_TYPE_FILE_COPY_FLAGS);
  pyg_flags_add(module, "FileMonitorFlags", strip_prefix, G_TYPE_FILE_MONITOR_FLAGS);
  pyg_enum_add(module, "FileType", strip_prefix, G_TYPE_FILE_TYPE);
  pyg_enum_add(module, "FilesystemPreviewType", strip_prefix, G_TYPE_FILESYSTEM_PREVIEW_TYPE);
  pyg_enum_add(module, "FileMonitorEvent", strip_prefix, G_TYPE_FILE_MONITOR_EVENT);
  pyg_enum_add(module, "ErrorEnum", strip_prefix, G_TYPE_IO_ERROR_ENUM);
  pyg_flags_add(module, "AskPasswordFlags", strip_prefix, G_TYPE_ASK_PASSWORD_FLAGS);
  pyg_enum_add(module, "PasswordSave", strip_prefix, G_TYPE_PASSWORD_SAVE);
  pyg_enum_add(module, "MountOperationResult", strip_prefix, G_TYPE_MOUNT_OPERATION_RESULT);
  pyg_flags_add(module, "OutputStreamSpliceFlags", strip_prefix, G_TYPE_OUTPUT_STREAM_SPLICE_FLAGS);
  pyg_enum_add(module, "EmblemOrigin", strip_prefix, G_TYPE_EMBLEM_ORIGIN);
  pyg_enum_add(module, "ResolverError", strip_prefix, G_TYPE_RESOLVER_ERROR);
  pyg_enum_add(module, "SocketFamily", strip_prefix, G_TYPE_SOCKET_FAMILY);
  pyg_enum_add(module, "SocketType", strip_prefix, G_TYPE_SOCKET_TYPE);
  pyg_enum_add(module, "SocketMsgFlags", strip_prefix, G_TYPE_SOCKET_MSG_FLAGS);
  pyg_enum_add(module, "SocketProtocol", strip_prefix, G_TYPE_SOCKET_PROTOCOL);
  pyg_enum_add(module, "ZlibCompressorFormat", strip_prefix, G_TYPE_ZLIB_COMPRESSOR_FORMAT);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pygio_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("glib")) != NULL) {
        _PyGPollFD_Type = (PyTypeObject *)PyObject_GetAttrString(module, "PollFD");
        if (_PyGPollFD_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name PollFD from glib");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import glib");
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


#line 147 "gfileattribute.override"
if (PyType_Ready(&PyGFileAttributeInfo_Type) < 0) {
    g_return_if_reached();
}
if (PyDict_SetItemString(d, "FileAttributeInfo",
                         (PyObject *)&PyGFileAttributeInfo_Type) < 0) {
    g_return_if_reached();
}

#line 19872 "gio.c"
    pyg_register_boxed(d, "FileAttributeMatcher", G_TYPE_FILE_ATTRIBUTE_MATCHER, &PyGFileAttributeMatcher_Type);
    pyg_register_boxed(d, "SrvTarget", G_TYPE_SRV_TARGET, &PyGSrvTarget_Type);
    pyg_register_interface(d, "AppInfo", G_TYPE_APP_INFO, &PyGAppInfo_Type);
    pyg_register_interface(d, "AsyncInitable", G_TYPE_ASYNC_INITABLE, &PyGAsyncInitable_Type);
    pyg_register_interface(d, "AsyncResult", G_TYPE_ASYNC_RESULT, &PyGAsyncResult_Type);
    pyg_register_interface(d, "Drive", G_TYPE_DRIVE, &PyGDrive_Type);
    pyg_register_interface(d, "File", G_TYPE_FILE, &PyGFile_Type);
    pyg_register_interface(d, "Icon", G_TYPE_ICON, &PyGIcon_Type);
    pyg_register_interface(d, "Initable", G_TYPE_INITABLE, &PyGInitable_Type);
    pyg_register_interface(d, "LoadableIcon", G_TYPE_LOADABLE_ICON, &PyGLoadableIcon_Type);
    pyg_register_interface(d, "Mount", G_TYPE_MOUNT, &PyGMount_Type);
    pyg_register_interface(d, "Seekable", G_TYPE_SEEKABLE, &PyGSeekable_Type);
    pyg_register_interface(d, "SocketConnectable", G_TYPE_SOCKET_CONNECTABLE, &PyGSocketConnectable_Type);
    pyg_register_interface(d, "Volume", G_TYPE_VOLUME, &PyGVolume_Type);
    pygobject_register_class(d, "GAppLaunchContext", G_TYPE_APP_LAUNCH_CONTEXT, &PyGAppLaunchContext_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_APP_LAUNCH_CONTEXT);
    pygobject_register_class(d, "GCancellable", G_TYPE_CANCELLABLE, &PyGCancellable_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_CANCELLABLE);
    pygobject_register_class(d, "GEmblem", G_TYPE_EMBLEM, &PyGEmblem_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_EMBLEM);
    pygobject_register_class(d, "GEmblemedIcon", G_TYPE_EMBLEMED_ICON, &PyGEmblemedIcon_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GFileEnumerator", G_TYPE_FILE_ENUMERATOR, &PyGFileEnumerator_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_ENUMERATOR);
    pygobject_register_class(d, "GFileInfo", G_TYPE_FILE_INFO, &PyGFileInfo_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_INFO);
    pygobject_register_class(d, "GFileMonitor", G_TYPE_FILE_MONITOR, &PyGFileMonitor_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_MONITOR);
    pygobject_register_class(d, "GInputStream", G_TYPE_INPUT_STREAM, &PyGInputStream_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_INPUT_STREAM);
    pygobject_register_class(d, "GFileInputStream", G_TYPE_FILE_INPUT_STREAM, &PyGFileInputStream_Type, Py_BuildValue("(O)", &PyGInputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_INPUT_STREAM);
    pygobject_register_class(d, "GFilterInputStream", G_TYPE_FILTER_INPUT_STREAM, &PyGFilterInputStream_Type, Py_BuildValue("(O)", &PyGInputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILTER_INPUT_STREAM);
    pygobject_register_class(d, "GBufferedInputStream", G_TYPE_BUFFERED_INPUT_STREAM, &PyGBufferedInputStream_Type, Py_BuildValue("(O)", &PyGFilterInputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_BUFFERED_INPUT_STREAM);
    pygobject_register_class(d, "GDataInputStream", G_TYPE_DATA_INPUT_STREAM, &PyGDataInputStream_Type, Py_BuildValue("(O)", &PyGFilterInputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_DATA_INPUT_STREAM);
    pygobject_register_class(d, "GMemoryInputStream", G_TYPE_MEMORY_INPUT_STREAM, &PyGMemoryInputStream_Type, Py_BuildValue("(O)", &PyGInputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_MEMORY_INPUT_STREAM);
    pygobject_register_class(d, "GMountOperation", G_TYPE_MOUNT_OPERATION, &PyGMountOperation_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_MOUNT_OPERATION);
    pygobject_register_class(d, "GInetAddress", G_TYPE_INET_ADDRESS, &PyGInetAddress_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_INET_ADDRESS);
    pygobject_register_class(d, "GNetworkAddress", G_TYPE_NETWORK_ADDRESS, &PyGNetworkAddress_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GNetworkService", G_TYPE_NETWORK_SERVICE, &PyGNetworkService_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GResolver", G_TYPE_RESOLVER, &PyGResolver_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_RESOLVER);
    pygobject_register_class(d, "GSocket", G_TYPE_SOCKET, &PyGSocket_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GSocketAddress", G_TYPE_SOCKET_ADDRESS, &PyGSocketAddress_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_ADDRESS);
    pygobject_register_class(d, "GInetSocketAddress", G_TYPE_INET_SOCKET_ADDRESS, &PyGInetSocketAddress_Type, Py_BuildValue("(O)", &PyGSocketAddress_Type));
    pygobject_register_class(d, "GSocketAddressEnumerator", G_TYPE_SOCKET_ADDRESS_ENUMERATOR, &PyGSocketAddressEnumerator_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_ADDRESS_ENUMERATOR);
    pygobject_register_class(d, "GSocketClient", G_TYPE_SOCKET_CLIENT, &PyGSocketClient_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_CLIENT);
    pygobject_register_class(d, "GSocketControlMessage", G_TYPE_SOCKET_CONTROL_MESSAGE, &PyGSocketControlMessage_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_CONTROL_MESSAGE);
    pygobject_register_class(d, "GSocketListener", G_TYPE_SOCKET_LISTENER, &PyGSocketListener_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_LISTENER);
    pygobject_register_class(d, "GSocketService", G_TYPE_SOCKET_SERVICE, &PyGSocketService_Type, Py_BuildValue("(O)", &PyGSocketListener_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_SERVICE);
    pygobject_register_class(d, "GThreadedSocketService", G_TYPE_THREADED_SOCKET_SERVICE, &PyGThreadedSocketService_Type, Py_BuildValue("(O)", &PyGSocketService_Type));
    pygobject_register_class(d, "GIOStream", G_TYPE_IO_STREAM, &PyGIOStream_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_IO_STREAM);
    pygobject_register_class(d, "GSocketConnection", G_TYPE_SOCKET_CONNECTION, &PyGSocketConnection_Type, Py_BuildValue("(O)", &PyGIOStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_SOCKET_CONNECTION);
    pygobject_register_class(d, "GTcpConnection", G_TYPE_TCP_CONNECTION, &PyGTcpConnection_Type, Py_BuildValue("(O)", &PyGSocketConnection_Type));
    pyg_set_object_has_new_constructor(G_TYPE_TCP_CONNECTION);
    pygobject_register_class(d, "GFileIOStream", G_TYPE_FILE_IO_STREAM, &PyGFileIOStream_Type, Py_BuildValue("(O)", &PyGIOStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_IO_STREAM);
    pygobject_register_class(d, "GOutputStream", G_TYPE_OUTPUT_STREAM, &PyGOutputStream_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_OUTPUT_STREAM);
    pygobject_register_class(d, "GMemoryOutputStream", G_TYPE_MEMORY_OUTPUT_STREAM, &PyGMemoryOutputStream_Type, Py_BuildValue("(O)", &PyGOutputStream_Type));
    pygobject_register_class(d, "GFilterOutputStream", G_TYPE_FILTER_OUTPUT_STREAM, &PyGFilterOutputStream_Type, Py_BuildValue("(O)", &PyGOutputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILTER_OUTPUT_STREAM);
    pygobject_register_class(d, "GBufferedOutputStream", G_TYPE_BUFFERED_OUTPUT_STREAM, &PyGBufferedOutputStream_Type, Py_BuildValue("(O)", &PyGFilterOutputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_BUFFERED_OUTPUT_STREAM);
    pygobject_register_class(d, "GDataOutputStream", G_TYPE_DATA_OUTPUT_STREAM, &PyGDataOutputStream_Type, Py_BuildValue("(O)", &PyGFilterOutputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_DATA_OUTPUT_STREAM);
    pygobject_register_class(d, "GFileOutputStream", G_TYPE_FILE_OUTPUT_STREAM, &PyGFileOutputStream_Type, Py_BuildValue("(O)", &PyGOutputStream_Type));
    pyg_set_object_has_new_constructor(G_TYPE_FILE_OUTPUT_STREAM);
    pygobject_register_class(d, "GSimpleAsyncResult", G_TYPE_SIMPLE_ASYNC_RESULT, &PyGSimpleAsyncResult_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GVfs", G_TYPE_VFS, &PyGVfs_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_VFS);
    pygobject_register_class(d, "GVolumeMonitor", G_TYPE_VOLUME_MONITOR, &PyGVolumeMonitor_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(G_TYPE_VOLUME_MONITOR);
    pygobject_register_class(d, "GNativeVolumeMonitor", G_TYPE_NATIVE_VOLUME_MONITOR, &PyGNativeVolumeMonitor_Type, Py_BuildValue("(O)", &PyGVolumeMonitor_Type));
    pyg_set_object_has_new_constructor(G_TYPE_NATIVE_VOLUME_MONITOR);
    pygobject_register_class(d, "GFileIcon", G_TYPE_FILE_ICON, &PyGFileIcon_Type, Py_BuildValue("(OOO)", &PyGObject_Type, &PyGIcon_Type, &PyGLoadableIcon_Type));
    pygobject_register_class(d, "GThemedIcon", G_TYPE_THEMED_ICON, &PyGThemedIcon_Type, Py_BuildValue("(OO)", &PyGObject_Type, &PyGIcon_Type));
    pyg_set_object_has_new_constructor(G_TYPE_THEMED_ICON);
}
