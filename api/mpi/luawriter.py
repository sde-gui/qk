import sys

from mpi.module import *

tmpl_file_start = """\
#include "moo-lua-api-util.h"

"""

tmpl_cfunc_method_start = """\
static int
%(cfunc)s (gpointer pself, G_GNUC_UNUSED lua_State *L, G_GNUC_UNUSED int first_arg)
{
    MooLuaCurrentFunc cur_func("%(current_function)s");
    %(Class)s *self = (%(Class)s*) pself;
"""

tmpl_cfunc_func_start = """\
static int
%(cfunc)s (G_GNUC_UNUSED lua_State *L)
{
    MooLuaCurrentFunc cur_func("%(current_function)s");
"""

tmpl_register_module_start = """\
static void
%(module)s_lua_api_register (void)
{
    static gboolean been_here = FALSE;

    if (been_here)
        return;

    been_here = TRUE;

"""

tmpl_register_one_type_start = """\
    MooLuaMethodEntry methods_%(Class)s[] = {
"""
tmpl_register_one_type_end = """\
        { NULL, NULL }
    };
    moo_lua_register_methods (%(gtype_id)s, methods_%(Class)s);

"""

class ArgHelper(object):
    def format_arg(self, allow_none, default_value, arg_name, arg_idx, param_name):
        return ''

class SimpleArgHelper(ArgHelper):
    def __init__(self, name, suffix):
        super(SimpleArgHelper, self).__init__()
        self.name = name
        self.suffix = suffix

    def format_arg(self, allow_none, default_value, arg_name, arg_idx, param_name):
        dic = dict(type=self.name, arg_name=arg_name, default_value=default_value,
                   arg_idx=arg_idx, param_name=param_name, suffix=self.suffix)
        if default_value is not None:
            return '%(type)s %(arg_name)s = moo_lua_get_arg_%(suffix)s_opt (L, %(arg_idx)s, "%(param_name)s", %(default_value)s);' % dic
        else:
            return '%(type)s %(arg_name)s = moo_lua_get_arg_%(suffix)s (L, %(arg_idx)s, "%(param_name)s");' % dic

_arg_helpers = {}
_arg_helpers['int'] = SimpleArgHelper('int', 'int')
_arg_helpers['uint'] = SimpleArgHelper('guint', 'int')
_arg_helpers['gint'] = SimpleArgHelper('int', 'int')
_arg_helpers['guint'] = SimpleArgHelper('guint', 'int')
_arg_helpers['gboolean'] = SimpleArgHelper('gboolean', 'bool')
_arg_helpers['index'] = SimpleArgHelper('int', 'index')
_arg_helpers['double'] = SimpleArgHelper('double', 'double')
_arg_helpers['const-char*'] = SimpleArgHelper('const char*', 'string')
_arg_helpers['char*'] = SimpleArgHelper('char*', 'string')
_arg_helpers['strv'] = SimpleArgHelper('char**', 'strv')
def find_arg_helper(param):
    return _arg_helpers[param.type.name]

_pod_ret_helpers = {}
_pod_ret_helpers['int'] = ('int', 'int')
_pod_ret_helpers['uint'] = ('guint', 'int')
_pod_ret_helpers['gint'] = ('int', 'int')
_pod_ret_helpers['guint'] = ('guint', 'int')
_pod_ret_helpers['gboolean'] = ('gboolean', 'bool')
_pod_ret_helpers['index'] = ('int', 'index')
def find_pod_ret_helper(name):
    return _pod_ret_helpers[name]

class Writer(object):
    def __init__(self, out):
        super(Writer, self).__init__()
        self.out = out

    def __write_function_param(self, func_body, param, i, meth, cls):
        dic = dict(narg=i, gtype_id=param.type.gtype_id, param_name=param.name,
                   allow_none=('TRUE' if param.allow_none else 'FALSE'),
                   default_value=param.default_value,
                   arg_idx=('first_arg + %d' % (i,)) if cls else ('1 + %d' % (i,)),
                   TypeName=param.type.name,
                   )
        if param.type.name == 'GtkTextIter':
            assert param.default_value is None or param.default_value == 'NULL'
            if param.default_value is not None:
                dic['get_arg'] = 'moo_lua_get_arg_iter_opt'
            else:
                dic['get_arg'] = 'moo_lua_get_arg_iter'
            if cls.name == 'MooEdit':
                dic['buffer'] = 'moo_edit_get_buffer (self)'
            else:
                dic['buffer'] = 'NULL'
            if param.default_value is not None:
                func_body.start.append('GtkTextIter arg%(narg)d_iter;' % dic)
                func_body.start.append(('GtkTextIter *arg%(narg)d = %(get_arg)s (L, %(arg_idx)s, ' + \
                                        '"%(param_name)s", %(buffer)s, &arg%(narg)d_iter) ? &arg%(narg)d_iter : NULL;') % dic)
            else:
                func_body.start.append('GtkTextIter arg%(narg)d_iter;' % dic)
                func_body.start.append('GtkTextIter *arg%(narg)d = &arg%(narg)d_iter;' % dic)
                func_body.start.append('%(get_arg)s (L, %(arg_idx)s, "%(param_name)s", %(buffer)s, &arg%(narg)d_iter);' % dic)
        elif isinstance(param.type, Class) or isinstance(param.type, Boxed) or isinstance(param.type, Pointer):
            if param.default_value is not None:
                func_body.start.append(('%(TypeName)s *arg%(narg)d = (%(TypeName)s*) ' + \
                                        'moo_lua_get_arg_instance_opt (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s);') % dic)
            else:
                func_body.start.append(('%(TypeName)s *arg%(narg)d = (%(TypeName)s*) ' + \
                                        'moo_lua_get_arg_instance (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s);') % dic)
        elif isinstance(param.type, Enum) or isinstance(param.type, Flags):
            if param.default_value is not None:
                func_body.start.append(('%(TypeName)s arg%(narg)d = (%(TypeName)s) ' + \
                                        'moo_lua_get_arg_enum_opt (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s, %(default_value)s);') % dic)
            else:
                func_body.start.append(('%(TypeName)s arg%(narg)d = (%(TypeName)s) ' + \
                                        'moo_lua_get_arg_enum (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s);') % dic)
        elif isinstance(param.type, ArrayType):
            assert isinstance(param.type.elm_type, Class)
            dic['gtype_id'] = param.type.elm_type.gtype_id
            if param.default_value is not None:
                func_body.start.append(('%(TypeName)s arg%(narg)d = (%(TypeName)s) ' + \
                                        'moo_lua_get_arg_object_array_opt (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s);') % dic)
            else:
                func_body.start.append(('%(TypeName)s arg%(narg)d = (%(TypeName)s) ' + \
                                        'moo_lua_get_arg_object_array (L, %(arg_idx)s, "%(param_name)s", ' + \
                                        '%(gtype_id)s);') % dic)
            func_body.end.append('moo_object_array_free ((MooObjectArray*) arg%(narg)d);' % dic)
        elif param.type.name == 'strv':
            assert param.default_value is None or param.default_value == 'NULL'
            if param.default_value is not None:
                func_body.start.append(('char **arg%(narg)d = moo_lua_get_arg_strv_opt (L, %(arg_idx)s, "%(param_name)s");') % dic)
            else:
                func_body.start.append(('char **arg%(narg)d = moo_lua_get_arg_strv (L, %(arg_idx)s, "%(param_name)s");') % dic)
            func_body.end.append('g_strfreev (arg%(narg)d);' % dic)
        else:
            arg_helper = find_arg_helper(param)
            func_body.start.append(arg_helper.format_arg(param.allow_none, param.default_value,
                                            'arg%(narg)d' % dic, '%(arg_idx)s' % dic, param.name))

    def __write_function(self, meth, cls, method_cfuncs):
        assert not isinstance(meth, Constructor) and not isinstance(meth, VMethod)

        bind = meth.annotations.get('moo.lua', '1')
        if bind == '0':
            return
        elif bind != '1':
            raise RuntimeError('invalid value %s for moo.lua annotation' % (bind,))

        has_gerror_return = False
        params = []
        for i in range(len(meth.params)):
            p = meth.params[i]

#             if isinstance(p.type, GErrorReturnType):
#                 print >> sys.stderr, "Skipping function %s because of 'GError**' parameter" % meth.c_name
#                 return

            if not p.type.name in _arg_helpers and not isinstance(p.type, ArrayType) and \
               not isinstance(p.type, GTypedType) and not isinstance(p.type, GErrorReturnType):
                print >> sys.stderr, "Skipping function %s because of '%s' parameter" % (meth.c_name, p.type.name)
                return

            if isinstance(p.type, GErrorReturnType):
                assert i == len(meth.params) - 1
                has_gerror_return = True
            else:
                params.append(p)

        dic = dict(name=meth.name, c_name=meth.c_name)
        if cls:
            dic['cfunc'] = 'cfunc_%s_%s' % (cls.name, meth.name)
            dic['Class'] = cls.name
            dic['current_function'] = '%s.%s' % (cls.name, meth.name)
            self.out.write(tmpl_cfunc_method_start % dic)
        else:
            dic['cfunc'] = 'cfunc_%s' % meth.name
            dic['current_function'] = meth.name
            self.out.write(tmpl_cfunc_func_start % dic)

        method_cfuncs.append([meth.name, dic['cfunc']])

        class FuncBody:
            def __init__(self):
                self.start = []
                self.end = []

        func_body = FuncBody()
        func_call = ''

        i = 0
        for p in params:
            self.__write_function_param(func_body, p, i, meth, cls)
            i += 1

        if has_gerror_return:
            func_body.start.append('GError *error = NULL;')

        if meth.retval:
            dic = {'gtype_id': meth.retval.type.gtype_id,
                   'make_copy': ('FALSE' if meth.retval.transfer_mode == 'full' else 'TRUE'),
                   }
            if isinstance(meth.retval.type, Class) or isinstance(meth.retval.type, Boxed) or isinstance(meth.retval.type, Pointer):
                func_call = 'gpointer ret = '
                push_ret = 'moo_lua_push_instance (L, ret, %(gtype_id)s, %(make_copy)s);' % dic
            elif isinstance(meth.retval.type, Enum) or isinstance(meth.retval.type, Flags):
                func_call = '%s ret = ' % meth.retval.type.name
                push_ret = 'moo_lua_push_int (L, ret);' % dic
            elif isinstance(meth.retval.type, ArrayType):
                assert isinstance(meth.retval.type.elm_type, Class)
                dic['gtype_id'] = meth.retval.type.elm_type.gtype_id
                func_call = 'MooObjectArray *ret = (MooObjectArray*) '
                push_ret = 'moo_lua_push_object_array (L, ret, %(make_copy)s);' % dic
            elif meth.retval.type.name == 'strv':
                assert meth.retval.transfer_mode == 'full'
                func_call = 'char **ret = '
                push_ret = 'moo_lua_push_strv (L, ret);'
            elif meth.retval.type.name == 'char*':
                assert meth.retval.transfer_mode == 'full'
                func_call = 'char *ret = '
                push_ret = 'moo_lua_push_string (L, ret);'
            elif meth.retval.type.name == 'const-char*':
                assert meth.retval.transfer_mode != 'full'
                func_call = 'const char *ret = '
                push_ret = 'moo_lua_push_string_copy (L, ret);'
            elif meth.retval.type.name == 'gunichar':
                func_call = 'gunichar ret = '
                push_ret = 'moo_lua_push_gunichar (L, ret);'
            else:
                typ, suffix = find_pod_ret_helper(meth.retval.type.name)
                dic['suffix'] = suffix
                func_call = '%s ret = ' % typ
                push_ret = 'moo_lua_push_%(suffix)s (L, ret);' % dic
        else:
            push_ret = '0;'

        if not has_gerror_return:
            func_body.end.append('return %s' % push_ret)
        else:
            func_body.end.append('int ret_lua = %s' % push_ret)
            func_body.end.append('ret_lua += moo_lua_push_error (L, error);')
            func_body.end.append('return ret_lua;')

        func_call += '%s (' % meth.c_name
        first_arg = True
        if cls:
            first_arg = False
            func_call += 'self'
        for i in range(len(params)):
            if not first_arg:
                func_call += ', '
            first_arg = False
            func_call += 'arg%d' % i
        if has_gerror_return:
            func_call += ', &error'
        func_call += ');'

        for line in func_body.start:
            print >>self.out, '    ' + line
        print >>self.out, '    ' + func_call
        for line in func_body.end:
            print >>self.out, '    ' + line

        self.out.write('}\n\n')

#         if not cls:
#             self.out.write(function_start_template % dic)
#         elif isinstance(meth, Constructor):
#             dic['class'] = cls.name
#             self.out.write(function_start_template % dic)
#             self.out.write('  (is-constructor-of %s)\n' % cls.name)
#         elif isinstance(meth, VMethod):
#             dic['class'] = cls.name
#             self.out.write(vmethod_start_template % dic)
#         else:
#             dic['class'] = cls.name
#             self.out.write(method_start_template % dic)
#         if meth.retval:
#             if meth.retval.transfer_mode == 'full':
#                 self.out.write('  (caller-owns-return #t)\n')
#             elif meth.retval.transfer_mode is not None:
#                 raise RuntimeError('do not know how to handle transfer mode %s' % (meth.retval.transfer_mode,))
#         if meth.params:
#             self.out.write('  (parameters\n')
#             for p in meth.params:
#                 self.out.write('    \'("%s" "%s"' % (p.type, p.name))
#                 if p.allow_none:
#                     self.out.write(' (null-ok)')
#                 if p.default_value is not None:
#                     self.out.write(' (default "%s")' % (p.default_value,))
#                 self.out.write(')\n')
#             self.out.write('  )\n')
#         self.out.write(')\n\n')

    def __write_class(self, cls):
        bind = cls.annotations.get('moo.lua', '1')
        if bind == '0':
            return []
        self.out.write('// methods of %s\n\n' % cls.name)
        method_cfuncs = []
        for meth in cls.methods:
            if not isinstance(meth, VMethod):
                self.__write_function(meth, cls, method_cfuncs)
        return method_cfuncs

    def __write_register_module(self, module, all_method_cfuncs):
        self.out.write(tmpl_register_module_start % dict(module=module.name.lower()))
        for cls in module.get_classes() + module.get_boxed() + module.get_pointers():
            method_cfuncs = all_method_cfuncs[cls.name]
            if method_cfuncs:
                dic = dict(Class=cls.name, gtype_id=cls.gtype_id)
                self.out.write(tmpl_register_one_type_start % dic)
                for name, cfunc in method_cfuncs:
                    self.out.write('        { "%s", %s },\n' % (name, cfunc))
                self.out.write(tmpl_register_one_type_end % dic)
        self.out.write('}\n\n')

    def write(self, module, include_headers):
        self.module = module

        self.out.write(tmpl_file_start)

        if include_headers:
            for h in include_headers:
                self.out.write('#include "%s"\n' % h)
            self.out.write('\n')

        all_method_cfuncs = {}

        for cls in module.get_classes() + module.get_boxed() + module.get_pointers():
            method_cfuncs = self.__write_class(cls)
            all_method_cfuncs[cls.name] = method_cfuncs

#         for enum in module.get_enums():
#             self.__write_enum_decl(enum)

        dic = dict(module=module.name.lower())

        all_func_cfuncs = []
        for func in module.get_functions():
            self.__write_function(func, None, all_func_cfuncs)
        self.out.write('const luaL_Reg %(module)s_lua_functions[] = {\n' % dic)
        for name, cfunc in all_func_cfuncs:
            self.out.write('    { "%s", %s },\n' % (name, cfunc))
        self.out.write('    { NULL, NULL }\n')
        self.out.write('};\n\n')

        self.__write_register_module(module, all_method_cfuncs)

        self.out.write("""\
void %(module)s_lua_api_add_to_lua (lua_State *L, const char *package_name)
{
    %(module)s_lua_api_register ();

    luaL_register (L, package_name, %(module)s_lua_functions);
}
""" % dic)

        del self.module
