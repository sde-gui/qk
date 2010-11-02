#! /usr/bin/env python

import sys
import os
import tempfile

import mooscriptparser as parser

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

tmpl_gen_method_decl = """\
    Variant %(method_name)s__imp__(const ArgSet &args);
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
            'arglist': 'const ArgList &',
            'argset': 'const ArgSet &',
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
        moo_return_val_if_reached(FunctionCallInfo("<no function>"));
}

class PushFunctionCall
{
public:
    explicit PushFunctionCall(const char *name)
    {
        func_calls.append(FunctionCallInfo(name));
    }

    ~PushFunctionCall()
    {
        func_calls.pop_back();
    }
};

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

tmpl_impl_method = """\
    meta.add_method("%(method_name)s", &%(ClassName)s::%(method_name)s__imp__);
"""

tmpl_impl_signal = """\
    meta.add_signal("%(signal_name)s");
"""

def write_class_init(cls, out):
    dic = make_class_dict(cls)
    out.write(tmpl_impl_class_init_start % dic)

    for meth in cls.methods.values():
        meth_dic = make_method_dict(meth, cls)
        out.write(tmpl_impl_method % meth_dic)

    for meth in cls.signals.values():
        meth_dic = make_method_dict(meth, cls)
        out.write(tmpl_impl_signal % meth_dic)

    out.write(tmpl_impl_class_init_end % dic)

tmpl_method_impl_start = """\
Variant %(ClassName)s::%(method_name)s__imp__(const ArgSet &args)
{
    PushFunctionCall pfc__("%(ClassName)s::%(method_name)s");
"""
tmpl_method_impl_end = """\
}
"""
tmpl_check_no_args = """\
    if (!args.pos.empty() || !args.kw.empty())
        Error::raisef("in function %s, no arguments expected",
                          (const char*) current_func().name);
"""
tmpl_check_no_kwargs = """\
    if (!args.kw.empty())
        Error::raisef("in function %s, no keyword arguments expected",
                          (const char*) current_func().name);
"""

tmpl_get_obj_arg = """\
    if (args.pos.size() <= %(iarg)d)
        Error::raisef("in function %%s, argument '%(argname)s' missing",
                          (const char*) current_func().name);
    %(type)s &%(arg)s = get_object_arg<%(type)s>(args.pos[%(iarg)d], "%(argname)s");
"""

tmpl_get_obj_arg_opt = """\
    %(type)s *%(arg)s = get_object_arg_opt<%(type)s>(args.pos.size() > %(iarg)d ? args.pos[%(iarg)d] : Variant(), "%(argname)s");
"""

def write_check_arg_object(meth, p, i, out):
    dic = {'arg': 'arg%d' % (i,), 'iarg': i, 'type': p.type.name, 'argname': p.name}
    if p.optional:
        out.write(tmpl_get_obj_arg_opt % dic)
    else:
        out.write(tmpl_get_obj_arg % dic)

tmpl_get_arg = """\
    if (args.pos.size() <= %(iarg)d)
        Error::raisef("in function %%s, argument '%(argname)s' missing",
                      (const char*) current_func().name);
    %(type)s %(arg)s = %(get_arg)s(args.pos[%(iarg)d], "%(argname)s");
"""

tmpl_get_arg_opt = """\
    %(type)s %(arg)s = %(get_arg)s_opt(args.pos.size() > %(iarg)d ? args.pos[%(iarg)d] : Variant(), "%(argname)s");
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
    elif meth.kwargs:
        pass
    elif meth.varargs:
        out.write(tmpl_check_no_kwargs)
    else:
        out.write(tmpl_check_no_kwargs)
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
        if meth.kwargs:
            out.write('args')
        elif meth.varargs:
            out.write('args.pos')
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
