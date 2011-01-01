#include <config.h>
#ifdef MOO_ENABLE_PYTHON
#include <Python.h>
#endif
#include "medit-python.h"
#include "mooutils/mooutils.h"
#include "moopython/medit-python-init.h"

#ifdef MOO_ENABLE_PYTHON

struct MooPythonState
{
    PyObject *locals;
};

static PyObject *
create_script_dict (const char *name)
{
    PyObject *dict, *builtins;

    builtins = PyImport_ImportModule ("__builtin__");
    g_return_val_if_fail (builtins != NULL, NULL);

    dict = PyDict_New ();
    PyDict_SetItemString (dict, "__builtins__", builtins);

    if (name)
    {
        PyObject *py_name = PyString_FromString (name);
        PyDict_SetItemString (dict, "__name__", py_name);
        Py_XDECREF (py_name);
    }

    Py_XDECREF (builtins);
    return dict;
}

static PyObject *
run_string (const char *str,
            PyObject   *globals,
            PyObject   *locals)
{
    PyObject *ret;

    g_return_val_if_fail (str != NULL, NULL);

    if (!locals)
        locals = create_script_dict ("__script__");
    else
        Py_INCREF ((PyObject*) locals);

    g_return_val_if_fail (locals != NULL, NULL);

    if (!globals)
        globals = locals;

    ret = PyRun_String (str, Py_file_input, globals, locals);

    Py_DECREF (locals);
    return ret;
}

// static PyObject *
// run_code (const char *str,
//           PyObject   *globals,
//           PyObject   *locals)
// {
//     PyObject *ret;
//
//     g_return_val_if_fail (str != NULL, NULL);
//
//     if (!locals)
//         locals = create_script_dict ("__script__");
//     else
//         Py_INCREF (locals);
//
//     g_return_val_if_fail (locals != NULL, NULL);
//
//     if (!globals)
//         globals = locals;
//
//     ret = PyRun_String (str, Py_file_input, globals, locals);
//
//     if (ret)
//     {
//         Py_DECREF (ret);
//
//         if (PyMapping_HasKeyString (locals, (char*) "__retval__"))
//             ret = PyMapping_GetItemString (locals, (char*) "__retval__");
//         else
//             ret = NULL;
//     }
//
//     Py_DECREF (locals);
//     return ret;
// }

MooPythonState *
moo_python_state_new (gboolean default_init)
{
    MooPythonState *state;

    state = g_slice_new0 (MooPythonState);
    state->locals = create_script_dict ("__script__");

    if (!state->locals)
    {
        moo_python_state_free (state);
        moo_return_val_if_reached (NULL);
    }

    if (default_init && !moo_python_run_string (state, MEDIT_PYTHON_INIT_PY))
    {
        moo_python_state_free (state);
        return NULL;
    }

    return state;
}

void
moo_python_state_free (MooPythonState *state)
{
    if (state)
    {
        Py_XDECREF (state->locals);
        g_slice_free (MooPythonState, state);
    }
}

gboolean
moo_python_run_string (MooPythonState *state,
                       const char     *string)
{
    PyObject *pyret;
    gboolean ret = TRUE;

    moo_return_val_if_fail (state && state->locals, FALSE);
    moo_return_val_if_fail (string != NULL, FALSE);

    pyret = run_string (string, NULL, state->locals);

    if (!pyret)
    {
        moo_message ("error running python script '%s'", string);
        if (PyErr_Occurred ())
            PyErr_Print ();
        else
            moo_critical ("oops");
        ret = FALSE;
    }

    Py_XDECREF (pyret);
    return ret;
}

gboolean
moo_python_run_file (MooPythonState *state,
                     const char     *filename)
{
    char *contents;
    GError *error = NULL;
    gboolean ret;

    moo_return_val_if_fail (state && state->locals, FALSE);
    moo_return_val_if_fail (filename != NULL, FALSE);

    if (!g_file_get_contents (filename, &contents, NULL, &error))
    {
        moo_warning ("could not read file '%s': %s",
                     filename, moo_error_message (error));
        return FALSE;
    }

    ret = moo_python_run_string (state, contents);

    g_free (contents);
    return ret;
}

gboolean
medit_python_run_string (const char *string,
                         gboolean    default_init)
{
    MooPythonState *state;
    gboolean ret;

    moo_return_val_if_fail (string != NULL, FALSE);

    state = moo_python_state_new (default_init);

    if (!state)
        return FALSE;

    ret = moo_python_run_string (state, string);

    moo_python_state_free (state);
    return ret;
}

gboolean
medit_python_run_file (const char *filename,
                       gboolean    default_init)
{
    char *contents;
    GError *error = NULL;
    gboolean ret;

    moo_return_val_if_fail  (filename != NULL, FALSE);

    if (!g_file_get_contents (filename, &contents, NULL, &error))
    {
        moo_warning ("could not read file '%s': %s",
                     filename, moo_error_message (error));
        return FALSE;
    }

    ret = medit_python_run_string (contents, default_init);

    g_free (contents);
    return ret;
}

#else /* !MOO_ENABLE_PYTHON */
#endif /* !MOO_ENABLE_PYTHON */
