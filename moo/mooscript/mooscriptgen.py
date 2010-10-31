#! /usr/bin/env python

import sys
import os
import tempfile

import mooscriptparser as parser

marshals = set()

tmpl_decl_file_start = """\
#ifndef MOO_SCRIPT_CLASSES_GENERATED_H
#define MOO_SCRIPT_CLASSES_GENERATED_H

#include "mooscript-classes-base.h"

namespace mom {

"""

tmpl_decl_file_end = """\

} // namespace mom

#endif /* MOO_SCRIPT_CLASSES_GENERATED_H */
"""

# tmpl_decl_get_type_func = """\
# GType mom_%(class_name)s_get_type (void) G_GNUC_CONST;
# """

tmpl_decl_forward_cls = """\
class %(ClassName)s;
"""

tmpl_decl_cls_struct_start = """\
class %(ClassName)s : public %(BaseClass)s
{
    %(CLASS_DECL)s

public:
"""

tmpl_decl_cls_struct_end = """\
};
"""

def split_camel_case_name(name):
    comps = []
    cur = ''
    for c in name:
        if c.islower() or not cur:
            cur += c
        else:
            comps.append(cur)
            cur = c
    if cur:
        comps.append(cur)
    return comps

def make_class_dict(cls):
    comps = split_camel_case_name(cls.name)
    dic= dict(ClassName=cls.name,
              CLASS_NAME='_'.join([s.upper() for s in comps]),
              class_name='_'.join([s.lower() for s in comps]),
              BaseClass='Object',
              CLASS_DECL='MOM_OBJECT_DECL(%s)' % (cls.name,),
              CLASS_DEFN='MOM_OBJECT_DEFN(%s)' % (cls.name,))
    if cls.singleton:
        dic['BaseClass'] = '_Singleton<%s>' % (cls.name,)
        dic['CLASS_DECL'] = 'MOM_SINGLETON_DECL(%s)' % (cls.name,)
        dic['CLASS_DEFN'] = 'MOM_SINGLETON_DEFN(%s)' % (cls.name,)
    elif cls.gobject:
        dic['BaseClass'] = '_GObjectWrapper<%s, %s>' % (cls.name, cls.gobject)
        dic['CLASS_DECL'] = 'MOM_GOBJECT_DECL(%s, %s)' % (cls.name, cls.gobject)
        dic['CLASS_DEFN'] = 'MOM_GOBJECT_DEFN(%s, %s)' % (cls.name, cls.gobject)
    return dic

def make_method_dict(meth, cls):
    dic = make_class_dict(cls)
    comps = meth.name.replace('-', '_').split('_')
    dic['method_name'] = '_'.join(comps)
    dic['signal_name'] = '-'.join(comps)
    return dic

# tmpl_method_impl_decl = """\
# %(retval)smom_%(class_name)s_%(method_name)s (Mom%(ClassName)s *self%(args)s);
# """
tmpl_gen_method_decl = """\
    Variant _%(method_name)s(const ArgArray &args);
"""
tmpl_method_decl = """\
    %(retval)s%(method_name)s(%(args)s);
"""

def format_param_decl(p):
    if isinstance(p.type, parser.Class):
        if p.optional:
            return '%s *%s' % (p.type.name, p.name)
        else:
            return '%s &%s' % (p.type.name, p.name)
    else:
        basic_names = {
            'bool': 'bool ',
            'string': 'const String &',
            'variant': 'const Variant &',
            'list': 'const VariantArray &',
            'int': 'gint64 ',
            'index': 'gint64 ',
            'args': 'const ArgArray &',
        }
        return basic_names[p.type.name] + p.name

def format_retval(retval):
    if retval is None:
        return 'void '
    elif isinstance(retval.type, parser.Class):
        return '%s *' % (retval.type.name,)
    else:
        basic_names = {
            'bool': 'bool ',
            'string': 'String ',
            'variant': 'Variant ',
            'list': 'VariantArray ',
            'int': 'gint64 ',
            'index': 'gint64 ',
        }
        return basic_names[retval.type.name]

def _write_method_decl(cls, meth, tmpl, out):
    args = ''
    if meth.params:
        for p in meth.params:
            if args:
                args += ', '
            args += format_param_decl(p)
    dic = make_method_dict(meth, cls)
    dic['retval'] = format_retval(meth.retval)
    dic['args'] = args
    out.write(tmpl % dic)

def write_gen_method_decl(cls, meth, out):
    _write_method_decl(cls, meth, tmpl_gen_method_decl, out)

def write_method_decl(cls, meth, out):
    _write_method_decl(cls, meth, tmpl_method_decl, out)

def write_method_impl_decl(cls, meth, out):
    _write_method_decl(cls, meth, tmpl_method_impl_decl, out)

def write_module_decl(out, mod):
    out.write(tmpl_decl_file_start)

    classes = mod.classes.values()

    for cls in classes:
        dic = make_class_dict(cls)
        out.write(tmpl_decl_forward_cls % dic)

    out.write('\n')

    for cls in classes:
        dic = make_class_dict(cls)
        out.write(tmpl_decl_cls_struct_start % dic)
        if cls.methods:
            out.write('\n')
            out.write('    /* methods */\n')
            for meth in cls.methods.values():
                write_method_decl(cls, meth, out)
            out.write('\n')
            out.write('    /* methods */\n')
            for meth in cls.methods.values():
                    write_gen_method_decl(cls, meth, out)
#         if cls.signals:
#             out.write('\n')
#             out.write('    /* signals */\n')
#             for sig in cls.signals.values():
#                 write_method_decl(cls, sig, out)
        out.write(tmpl_decl_cls_struct_end % dic)
        out.write('\n')

#     for cls in classes:
#         dic = make_class_dict(cls)
#         out.write(tmpl_decl_get_type_func % dic)

#     for cls in classes:
#         if cls.methods:
#             out.write('\n')
#             for meth in cls.methods.values():
#                 write_method_impl_decl(cls, meth, out)

    out.write(tmpl_decl_file_end)

tmpl_impl_file_start = """\
#include "mooscript-classes.h"
#include "mooscript-classes-util.h"

namespace mom {

static moo::Vector<FunctionCallInfo> func_calls;

FunctionCallInfo current_func()
{
    if (!func_calls.empty())
        return func_calls[func_calls.size() - 1];
    else
        return FunctionCallInfo("<no function>");
}

static void push_function_call(const char *name)
{
    func_calls.append(FunctionCallInfo(name));
}

static void pop_function_call()
{
    func_calls.pop_back();
}

"""

tmpl_impl_file_end = """\
} // namespace mom
"""

tmpl_impl_object_defn = """\
%(CLASS_DEFN)s
"""

tmpl_impl_class_init_start = """\
void %(ClassName)s::InitMetaObject(MetaObject &meta)
{
"""

tmpl_impl_class_init_end = """\
}
"""

# tmpl_impl_assign_meth = """\
#     klass->%(method_name)s = mom_%(class_name)s_%(method_name)s;
# """

tmpl_impl_method = """\
    meta.add_method("%(method_name)s", &%(ClassName)s::_%(method_name)s);
"""

tmpl_impl_signal = """\
    meta.add_signal("%(signal_name)s");
"""

def format_mom_type(retval):
    if retval is None:
        return 'MOM_TYPE_NONE'
    elif isinstance(retval.type, parser.Class):
        comps = split_camel_case_name(retval.type.name)
        return '_'.join(['MOM', 'TYPE'] + [s.upper() for s in comps])
    else:
        basic_types = {
            'bool': 'MOM_TYPE_BOOL',
            'string': 'MOM_TYPE_STRING',
            'variant': 'MOM_TYPE_VARIANT',
            'list': 'MOM_TYPE_LIST',
            'args': 'MOM_TYPE_ARGS',
            'int': 'MOM_TYPE_INT',
            'index': 'MOM_TYPE_INDEX',
            'base1': 'MOM_TYPE_BASE1',
        }
        return basic_types[retval.type.name]

def format_marshal_type(p):
    if p is None:
        return 'VOID'
    elif isinstance(p.type, parser.Class):
        return 'OBJECT'
    else:
        basic_types = {
            'bool': 'BOOL',
            'string': 'STRING',
            'variant': 'BOXED',
            'list': 'BOXED',
            'int': 'INT64',
            'index': 'UINT64',
            'args': 'BOXED',
        }
        return basic_types[p.type.name]

def format_marshal(meth):
    if meth.params:
        arg_types = [format_marshal_type(p) for p in meth.params]
    else:
        arg_types = ['VOID']
    m = 'mom_marshal_%s__%s' % (format_marshal_type(meth.retval), '_'.join(arg_types))
    global marshals
    marshals.add(m)
    return m

def write_class_init(cls, out):
    dic = make_class_dict(cls)
    out.write(tmpl_impl_class_init_start % dic)

#     for meth in cls.methods.values():
#         meth_dic = make_method_dict(meth, cls)
#         out.write(tmpl_impl_assign_meth % meth_dic)

    for meth in cls.methods.values():
        meth_dic = make_method_dict(meth, cls)
        meth_dic['accumulator'] = 'NULL'
        meth_dic['marshal'] = format_marshal(meth)
        meth_dic['TYPE_RET'] = format_mom_type(meth.retval)
        meth_dic['n_params'] = len(meth.params)
        out.write(tmpl_impl_method % meth_dic)

    for meth in cls.signals.values():
        meth_dic = make_method_dict(meth, cls)
        meth_dic['accumulator'] = 'NULL'
        meth_dic['marshal'] = format_marshal(meth)
        meth_dic['TYPE_RET'] = format_mom_type(meth.retval)
        meth_dic['n_params'] = len(meth.params)
        out.write(tmpl_impl_signal % meth_dic)
#         if meth.params:
#             for p in meth.params:
#                 mtype = format_mom_type(p)
#                 out.write(tmpl_impl_method_param % (mtype,))
#         out.write(');\n')

    out.write(tmpl_impl_class_init_end % dic)

# def write_object_init(cls, out):
#     dic = make_class_dict(cls)
#     out.write(tmpl_impl_object_init % dic)

tmpl_method_impl_start = """\
Variant %(ClassName)s::_%(method_name)s(const ArgArray &args)
{
    push_function_call("%(ClassName)s::%(method_name)s");
    try {
"""
tmpl_method_impl_end = """\
    } catch(...) {
        pop_function_call();
        throw;
    }
}
"""
tmpl_check_no_args = """\
    if (args.size() != 0)
        Error::raisef("in function %s, no arguments expected",
                      (const char*) current_func().name);
"""

tmpl_get_obj_arg = """\
    if (args.size() <= %(iarg)d)
        Error::raisef("in function %%s, argument '%(argname)s' missing",
                      (const char*) current_func().name);
    %(type)s &%(arg)s = get_object_arg<%(type)s>(args[%(iarg)d], "%(argname)s");
"""

tmpl_get_obj_arg_opt = """\
    %(type)s *%(arg)s = get_object_arg_opt<%(type)s>(args.size() > %(iarg)d ? args[%(iarg)d] : Variant(), "%(argname)s");
"""

def write_check_arg_object(meth, p, i, out):
    dic = {'arg': 'arg%d' % (i,), 'iarg': i, 'type': p.type.name, 'argname': p.name}
    if p.optional:
        out.write(tmpl_get_obj_arg_opt % dic)
    else:
        out.write(tmpl_get_obj_arg % dic)

tmpl_get_arg = """\
    if (args.size() <= %(iarg)d)
        Error::raisef("in function %%s, argument '%(argname)s' missing",
                      (const char*) current_func().name);
    %(type)s %(arg)s = %(get_arg)s(args[%(iarg)d], "%(argname)s");
"""

tmpl_get_arg_opt = """\
    %(type)s %(arg)s = %(get_arg)s_opt(args.size() > %(iarg)d ? args[%(iarg)d] : Variant(), "%(argname)s");
"""

def write_check_arg(meth, p, i, out):
    if isinstance(p.type, parser.Class):
        write_check_arg_object(meth, p, i, out)
    else:
        typenames = {
            'bool': 'bool',
            'string': 'String',
            'variant': 'Variant',
            'list': 'VariantArray',
            'int': 'gint64',
            'index': 'gint64',
        }
        get_arg = {
            'bool': 'get_arg_bool',
            'string': 'get_arg_string',
            'variant': 'get_arg_variant',
            'list': 'get_arg_array',
            'int': 'get_arg_int',
            'index': 'get_arg_index',
        }
        dic = {'arg': 'arg%d' % (i,), 'iarg': i, 'type': typenames[p.type.name], 'get_arg': get_arg[p.type.name], 'argname': p.name}
        if p.optional:
            out.write(tmpl_get_arg_opt % dic)
        else:
            out.write(tmpl_get_arg % dic)
def write_method_impl_check_args(meth, out):
    if not meth.params:
        out.write(tmpl_check_no_args)
    elif len(meth.params) == 1 and meth.params[0].type.name == 'args':
        pass
    else:
        i = 0
        for p in meth.params:
            write_check_arg(meth, p, i, out)
            i += 1
def write_wrap_retval(meth, out):
    if meth.retval is None:
        return
    out.write('return ')
    wrap_func = None
    if isinstance(meth.retval.type, parser.Class):
        wrap_func = 'wrap_object'
    else:
        funcs = {
            'bool': 'wrap_bool',
            'string': 'wrap_string',
            'variant': 'wrap_variant',
            'list': 'wrap_array',
            'int': 'wrap_int',
            'index': 'wrap_index',
        }
        wrap_func = funcs[meth.retval.type.name]
    out.write(wrap_func)
    out.write('(')
def write_method_impl_call_func(meth, out):
    out.write('    ')
    write_wrap_retval(meth, out)
    out.write(meth.name + '(')
    if meth.params:
        if len(meth.params) == 1 and meth.params[0].type.name == 'args':
            out.write('args')
        else:
            out.write(', '.join(['arg%d' % (i,) for i in range(len(meth.params))]))
    out.write(')')
    if meth.retval is not None:
        out.write(')')
    out.write(';\n')
    if meth.retval is None:
        out.write('    return Variant();\n')
def write_method_impl(cls, meth, out):
    meth_dic = make_method_dict(meth, cls)
    out.write(tmpl_method_impl_start % meth_dic)
    write_method_impl_check_args(meth, out)
    write_method_impl_call_func(meth, out)
    out.write(tmpl_method_impl_end % meth_dic)
    out.write('\n')

def write_module_impl(out, mod):
    out.write(tmpl_impl_file_start)

    classes = mod.classes.values()

    for cls in classes:
        dic = make_class_dict(cls)
        out.write(tmpl_impl_object_defn % dic)

    out.write('\n')

    for cls in classes:
        write_class_init(cls, out)
        out.write('\n')

    for cls in classes:
        for meth in cls.methods.values():
            write_method_impl(cls, meth, out)

    out.write(tmpl_impl_file_end)

# def write_marshals(out):
#     global marshals
#     print >> out, "namespace mom {"
#     for m in sorted(marshals):
#         print >> out, 'static Variant %s(Object *self, const ArgArray &args);' % (m,)
#     print >> out, "} // namespace mom"

def generate_file(filename, gen_func, *args):
    tmp = tempfile.NamedTemporaryFile(dir=os.path.dirname(filename), delete=False)
    try:
        gen_func(tmp, *args)
        tmp.close()
        os.rename(tmp.name, filename)
    finally:
        if os.path.exists(tmp.name):
            os.unlink(tmp.name)

def do_generate(input_file, decl_file, impl_file):
    p = parser.Parser()
    mod = p.parse(input_file)
    generate_file(decl_file, write_module_decl, mod)
    generate_file(impl_file, write_module_impl, mod)
#     generate_file(marshals_file, write_marshals)

if __name__ == '__main__':
    from optparse import OptionParser
    op = OptionParser()
    op.add_option("--input", dest="input", help="read classes description from FILE", metavar="FILE")
    op.add_option("--decl", dest="decl", help="write declarations to DECL", metavar="DECL")
    op.add_option("--impl", dest="impl", help="write implementation to IMPL", metavar="IMPL")
    opts, args = op.parse_args()
    if args:
        op.error("too many arguments")
    do_generate(opts.input, opts.decl, opts.impl)
