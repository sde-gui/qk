#include "moopython.h"
#include "mooscript/python/moopython-init.h"
#include "mooscript/python/moopython-init-script.h"
#include "mooscript/mooscript-api.h"
#include "mooutils/mooutils-misc.h"

using namespace mom;

typedef void (*Fn_Py_InitializeEx) (int initsigs);
typedef void (*Fn_Py_Finalize) (void);
typedef int  (*Fn_PyRun_SimpleString) (const char *command);

typedef struct {
    GModule *module;
    gboolean initialized;
    int ref_count;
    gboolean py_initialized;
    Fn_Py_InitializeEx pfn_Py_InitializeEx;
    Fn_Py_Finalize pfn_Py_Finalize;
    Fn_PyRun_SimpleString pfn_PyRun_SimpleString;

    guint last_retval_id;
    moo::Dict<guint, Variant> retvals;
} MooPythonModule;

static MooPythonModule moo_python_module;

static GModule *
find_python_dll (gboolean py3)
{
    const char *python2_libs[] = {
#ifdef __WIN32__
        "python27", "python26",
#else
        "python2.7", "python2.6",
#endif
        NULL
    };

    const char *python3_libs[] = {
#ifdef __WIN32__
        "python35", "python34", "python33", "python32", "python31",
#else
        "python3.5", "python3.4", "python3.3", "python3.2", "python3.1",
#endif
        NULL
    };

    const char **libs, **p;
    GModule *module = NULL;

    if (py3)
        libs = python3_libs;
    else
        libs = python2_libs;

    for (p = libs; p && *p; ++p)
    {
        char *path =
#ifdef __WIN32__
            g_strdup_printf ("%s.dll", *p);
#else
            g_module_build_path (NULL, *p);
#endif
        if (path)
            module = g_module_open (path, G_MODULE_BIND_LAZY);
        g_free (path);
        if (module)
            break;
    }

    return module;
}

static void encode_list (const VariantArray &list, GString *str);
static void encode_variant (const Variant &val, GString *str);
static void encode_string (const char *s, GString *str);

char *encode_error (const char *message)
{
    GString *str = g_string_new ("RuntimeError(");
    encode_string (message, str);
    g_string_append (str, ")");
    return g_string_free (str, FALSE);
}

static void
encode_string (const char *s, GString *str)
{
    g_string_append_c (str, '"');
    for ( ; *s; ++s)
    {
        if (g_ascii_isprint (*s))
            g_string_append_c (str, *s);
        else
            g_string_append_printf (str, "\\x%x", (guint) (guchar) *s);
    }
    g_string_append_c (str, '"');
}

static void
encode_dict (const VariantDict &dict, GString *str)
{
    g_string_append (str, "{");

    for (moo::Dict<String, Variant>::const_iterator iter = dict.begin(); iter != dict.end(); ++iter)
    {
        if (iter != dict.begin())
            g_string_append (str, ", ");

        encode_string (iter.key(), str);
        g_string_append (str, ": ");
        encode_variant (iter.value(), str);
    }

    g_string_append (str, "}");
}

static void
encode_object (const HObject &h, GString *str)
{
    g_string_append_printf (str, "_medit_raw.Object(%u)", h.id());
}

static void
encode_variant (const Variant &val, GString *str)
{
    switch (val.vt())
    {
        case VtVoid:
            g_string_append (str, "None");
            break;
        case VtBool:
            g_string_append (str, val.value<VtBool>() ? "True" : "False");
            break;
        case VtIndex:
            g_string_append_printf (str, "%d", val.value<VtIndex>().get());
            break;
        case VtInt:
            g_string_append_printf (str, "%" G_GINT64_FORMAT, val.value<VtInt>());
            break;
        case VtDouble:
            g_string_append_printf (str, "%f", val.value<VtDouble>());
            break;
        case VtString:
            encode_string (val.value<VtString>(), str);
            break;
        case VtArray:
            encode_list (val.value<VtArray>(), str);
            break;
//         case VtArgList:
//             encode_list (val.value<VtArgList>(), str);
//             break;
        case VtDict:
            encode_dict (val.value<VtDict>(), str);
            break;
        case VtObject:
            encode_object (val.value<VtObject>(), str);
            break;
        default:
            moo_critical ("oops");
            g_string_append (str, "None");
            break;
    }
}

static void
encode_list_elms (const VariantArray &list, GString *str)
{
    for (int i = 0, c = list.size(); i < c; ++i)
    {
        if (i > 0)
            g_string_append (str, ", ");
        encode_variant (list[i], str);
    }
}

static void
encode_list (const VariantArray &list, GString *str)
{
    g_string_append (str, "[");
    encode_list_elms (list, str);
    g_string_append (str, "]");
}

static void
encode_args (const ArgList &args, GString *str)
{
    encode_list_elms (args, str);
}

static char *
encode_variant (const Variant &val)
{
    GString *str = g_string_new (NULL);
    encode_variant (val, str);
    return g_string_free (str, FALSE);
}

#define CFUNC_PROTO_variant_new "ctypes.c_void_p"
static Variant *
cfunc_variant_new ()
{
    return new Variant;
}

#define CFUNC_PROTO_variant_free "None, ctypes.c_void_p"
static void
cfunc_variant_free (Variant *var)
{
    delete var;
}

#define CFUNC_PROTO_variant_set_bool "None, ctypes.c_void_p, ctypes.c_int"
static void
cfunc_variant_set_bool (Variant *var, int val)
{
    var->setValue(bool(val));
}

#define CFUNC_PROTO_variant_set_int "None, ctypes.c_void_p, ctypes.c_longlong"
static void
cfunc_variant_set_int (Variant *var, gint64 val)
{
    var->setValue(val);
}

#define CFUNC_PROTO_variant_set_double "None, ctypes.c_void_p, ctypes.c_double"
static void
cfunc_variant_set_double (Variant *var, double val)
{
    var->setValue(val);
}

#define CFUNC_PROTO_variant_set_string "None, ctypes.c_void_p, ctypes.c_char_p"
static void
cfunc_variant_set_string (Variant *var, const char *val)
{
    var->setValue(String(val));
}

#define CFUNC_PROTO_variant_set_array "None, ctypes.c_void_p, ctypes.c_void_p"
static void
cfunc_variant_set_array (Variant *var, const VariantArray *val)
{
    var->setValue(*val);
}

#define CFUNC_PROTO_variant_set_dict "None, ctypes.c_void_p, ctypes.c_void_p"
static void
cfunc_variant_set_dict (Variant *var, const VariantDict *val)
{
    var->setValue(*val);
}

#define CFUNC_PROTO_variant_set_object "None, ctypes.c_void_p, ctypes.c_uint"
static void
cfunc_variant_set_object (Variant *var, guint val)
{
    var->setValue(HObject(val));
}

#define CFUNC_PROTO_variant_array_new "ctypes.c_void_p"
static VariantArray *
cfunc_variant_array_new ()
{
    return new VariantArray;
}

#define CFUNC_PROTO_variant_array_free "None, ctypes.c_void_p"
static void
cfunc_variant_array_free (VariantArray *ar)
{
    delete ar;
}

#define CFUNC_PROTO_variant_array_append "None, ctypes.c_void_p, ctypes.c_void_p"
static void
cfunc_variant_array_append (VariantArray *ar, const Variant *val)
{
    ar->append(*val);
}

#define CFUNC_PROTO_variant_dict_new "ctypes.c_void_p"
static VariantDict *
cfunc_variant_dict_new ()
{
    return new VariantDict;
}

#define CFUNC_PROTO_variant_dict_free "None, ctypes.c_void_p"
static void
cfunc_variant_dict_free (VariantDict *dict)
{
    delete dict;
}

#define CFUNC_PROTO_variant_dict_set "None, ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p"
static void
cfunc_variant_dict_set (VariantDict *dict, const char *key, const Variant *val)
{
    (*dict)[key] = *val;
}

#define CFUNC_PROTO_free "None, ctypes.c_char_p"
static void
cfunc_free (char *p)
{
    g_free (p);
}

#define CFUNC_PROTO_get_app_obj "ctypes.c_uint"
static guint
cfunc_get_app_obj (void)
{
    return Script::get_app_obj().id();
}

#define CFUNC_PROTO_push_retval "None, ctypes.c_uint, ctypes.c_void_p"
static void
cfunc_push_retval (guint id, const Variant *value)
{
    moo_return_if_fail (value != 0);

    if (moo_python_module.retvals.contains(id))
    {
        moo_critical ("oops");
        return;
    }

    moo_python_module.retvals[id] = *value;
}

#define CFUNC_PROTO_call_method "ctypes.c_char_p, ctypes.c_uint, ctypes.c_char_p, ctypes.c_void_p, ctypes.c_void_p"
static char *
cfunc_call_method (guint obj_id, const char *method, const VariantArray *args_pos, const VariantDict *args_kw)
{
    ArgSet args;
    if (args_pos)
        args.pos = *args_pos;
    if (args_kw)
        args.kw = *args_kw;

    Variant ret;

    moo_python_ref ();
    Result r = Script::call_method (HObject (obj_id), method, args, ret);
    moo_python_unref ();

    if (r.succeeded ())
        return encode_variant (ret);
    else
        return encode_error (r.message ());
}

struct MooPythonCallback : public Callback
{
    gulong id;

    Variant run(const ArgList &args)
    {
        guint retval_id = ++moo_python_module.last_retval_id;

        GString *str = g_string_new (NULL);
        g_string_append_printf (str, "import _medit_raw\n_medit_raw.Object._invoke_callback(%lu, %u, ", id, retval_id);
        encode_args (args, str);
        g_string_append (str, ")\n");

//         g_print ("%s", str->str);

        int result = moo_python_module.pfn_PyRun_SimpleString (str->str);
        g_string_free (str, TRUE);

        if (result != 0)
        {
//             moo_message ("error in PyRun_SimpleString");
            return Variant();
        }
        else if (!moo_python_module.retvals.contains(retval_id))
        {
            moo_critical ("oops");
            return Variant();
        }
        else
        {
            Variant v = moo_python_module.retvals[retval_id];
            moo_python_module.retvals.erase(retval_id);
            return v;
        }
    }

    void on_connect()
    {
    }

    void on_disconnect()
    {
    }
};

#define CFUNC_PROTO_connect_callback "ctypes.c_char_p, ctypes.c_uint, ctypes.c_char_p"
static char *
cfunc_connect_callback (guint obj_id, const char *sig)
{
    moo::SharedPtr<MooPythonCallback> cb (new MooPythonCallback);
    gulong id;
    Result r = Script::connect_callback (HObject (obj_id), sig, cb, id);
    if (!r.succeeded ())
        return encode_error (r.message ());
    cb->id = id;
    return g_strdup_printf ("%lu", id);
}

#define CFUNC_PROTO_disconnect_callback "ctypes.c_char_p, ctypes.c_uint, ctypes.c_uint"
static char *
cfunc_disconnect_callback (guint obj_id, guint callback_id)
{
    Result r = Script::disconnect_callback (HObject (obj_id), callback_id);
    if (!r.succeeded ())
        return encode_error (r.message ());
    else
        return g_strdup ("None");
}

static gboolean
moo_python_init_impl (void)
{
    GString *init_script = NULL;
    char **dirs = NULL;
    gpointer p;

    if (moo_python_module.initialized)
    {
        if (moo_python_module.module != NULL)
        {
            moo_python_module.ref_count++;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    moo_python_module.initialized = TRUE;

    moo_python_module.module = find_python_dll (FALSE);
    if (!moo_python_module.module)
    {
        moo_message ("python module not found");
        goto error;
    }

    if (g_module_symbol (moo_python_module.module, "Py_InitializeEx", &p))
        moo_python_module.pfn_Py_InitializeEx = (Fn_Py_InitializeEx) p;
    if (g_module_symbol (moo_python_module.module, "Py_Finalize", &p))
        moo_python_module.pfn_Py_Finalize = (Fn_Py_Finalize) p;
    if (g_module_symbol (moo_python_module.module, "PyRun_SimpleString", &p))
        moo_python_module.pfn_PyRun_SimpleString = (Fn_PyRun_SimpleString) p;

    if (!moo_python_module.pfn_Py_InitializeEx)
    {
        moo_message ("Py_InitializeEx not found");
        goto error;
    }

    if (!moo_python_module.pfn_PyRun_SimpleString)
    {
        moo_message ("PyRun_SimpleString not found");
        goto error;
    }

    if (!moo_python_module.pfn_Py_Finalize)
    {
        moo_message ("Py_Finalize not found");
        goto error;
    }

    moo_python_module.pfn_Py_InitializeEx (0);
    moo_python_module.py_initialized = TRUE;

#define FUNC_ENTRY(func) "        " #func "=ctypes.CFUNCTYPE(" CFUNC_PROTO_##func ")(%" G_GUINT64_FORMAT "),\n"

    init_script = g_string_new (
        "def __medit_init():\n"
        "    import ctypes\n"
        "    import sys\n"
        "    import imp\n"
    );

    g_string_append_printf (init_script,
        "    funcs = dict(\n"
            FUNC_ENTRY(free)
            FUNC_ENTRY(get_app_obj)
            FUNC_ENTRY(call_method)
            FUNC_ENTRY(connect_callback)
            FUNC_ENTRY(disconnect_callback)
            FUNC_ENTRY(push_retval)

            FUNC_ENTRY(variant_new)
            FUNC_ENTRY(variant_free)
            FUNC_ENTRY(variant_set_bool)
            FUNC_ENTRY(variant_set_int)
            FUNC_ENTRY(variant_set_double)
            FUNC_ENTRY(variant_set_string)
            FUNC_ENTRY(variant_set_array)
            FUNC_ENTRY(variant_set_dict)
            FUNC_ENTRY(variant_set_object)

            FUNC_ENTRY(variant_array_new)
            FUNC_ENTRY(variant_array_free)
            FUNC_ENTRY(variant_array_append)

            FUNC_ENTRY(variant_dict_new)
            FUNC_ENTRY(variant_dict_free)
            FUNC_ENTRY(variant_dict_set)
        "    )\n",
        (guint64) cfunc_free,
        (guint64) cfunc_get_app_obj,
        (guint64) cfunc_call_method,
        (guint64) cfunc_connect_callback,
        (guint64) cfunc_disconnect_callback,
        (guint64) cfunc_push_retval,
        (guint64) cfunc_variant_new,
        (guint64) cfunc_variant_free,
        (guint64) cfunc_variant_set_bool,
        (guint64) cfunc_variant_set_int,
        (guint64) cfunc_variant_set_double,
        (guint64) cfunc_variant_set_string,
        (guint64) cfunc_variant_set_array,
        (guint64) cfunc_variant_set_dict,
        (guint64) cfunc_variant_set_object,
        (guint64) cfunc_variant_array_new,
        (guint64) cfunc_variant_array_free,
        (guint64) cfunc_variant_array_append,
        (guint64) cfunc_variant_dict_new,
        (guint64) cfunc_variant_dict_free,
        (guint64) cfunc_variant_dict_set
        );

    g_string_append (init_script, "    path_entries = [");
    dirs = moo_get_data_subdirs ("python");
    for (char **p = dirs; p && *p; ++p)
        g_string_append_printf (init_script, "%" G_GUINT64_FORMAT ", ", (guint64) *p);
    g_string_append (init_script, "]\n");

    g_string_append (init_script, MOO_PYTHON_INIT);

//     g_print ("%s\n", init_script->str);

    if (moo_python_module.pfn_PyRun_SimpleString (init_script->str) != 0)
    {
        moo_critical ("error in PyRun_SimpleString");
        goto error;
    }

    if (dirs)
        g_strfreev (dirs);
    if (init_script)
        g_string_free (init_script, TRUE);

    moo_python_module.ref_count = 1;
    return TRUE;

error:
    if (moo_python_module.py_initialized)
    {
        moo_python_module.pfn_Py_Finalize ();
        moo_python_module.py_initialized = FALSE;
    }
    if (moo_python_module.module)
    {
        g_module_close (moo_python_module.module);
        moo_python_module.module = NULL;
    }
    if (dirs)
        g_strfreev (dirs);
    if (init_script)
        g_string_free (init_script, TRUE);
    return FALSE;
}

static void
moo_python_deinit_impl (void)
{
    if (!moo_python_module.initialized)
        return;

    g_assert (moo_python_module.ref_count == 0);

    if (moo_python_module.py_initialized)
    {
        moo_python_module.pfn_Py_Finalize ();
        moo_python_module.py_initialized = FALSE;
    }
    if (moo_python_module.module)
    {
        g_module_close (moo_python_module.module);
        moo_python_module.module = NULL;
    }
}

extern "C" gboolean
moo_python_init (void)
{
    return moo_python_init_impl ();
}

extern "C" void
moo_python_deinit (void)
{
    moo_python_unref ();
}

extern "C" gboolean
moo_python_ref (void)
{
    return moo_python_init_impl ();
}

extern "C" void
moo_python_unref (void)
{
    moo_return_if_fail (moo_python_module.ref_count > 0);
    if (!--moo_python_module.ref_count)
        moo_python_deinit_impl ();
}

extern "C" gboolean
moo_python_run_string_full (const char *prefix,
                            const char *string)
{
    GString *script;
    gboolean ret = TRUE;

    moo_return_val_if_fail (string != NULL, FALSE);

    if (!moo_python_ref ())
        return FALSE;

    script = g_string_new (MOO_PYTHON_INIT_SCRIPT);

    if (prefix && *prefix)
        g_string_append (script, prefix);
    if (string && *string)
        g_string_append (script, string);

    if (moo_python_module.pfn_PyRun_SimpleString (script->str) != 0)
    {
//         moo_message ("error in PyRun_SimpleString");
        ret = FALSE;
    }

    moo_python_unref ();
    g_string_free (script, TRUE);
    return ret;
}

extern "C" gboolean
moo_python_run_string (const char *string)
{
    moo_return_val_if_fail (string != NULL, FALSE);
    return moo_python_run_string_full (NULL, string);
}

extern "C" gboolean
moo_python_run_file (const char *filename)
{
    char *content = NULL;
    GError *error = NULL;
    gboolean ret;

    moo_return_val_if_fail (filename != NULL, FALSE);

    if (!g_file_get_contents (filename, &content, NULL, &error))
    {
        moo_warning ("could not read file '%s': %s", filename, error->message);
        g_error_free (error);
        return FALSE;
    }

    ret = moo_python_run_string (content);
    g_free (content);
    return ret;
}
