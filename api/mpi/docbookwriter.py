import StringIO

from mpi.util import *
from mpi.module import *

lua_constants = {
    'NULL': 'nil',
    'TRUE': 'true',
    'FALSE': 'false',
}

python_constants = {
    'NULL': 'None',
    'TRUE': 'True',
    'FALSE': 'False',
}

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

def name_all_caps(cls):
    return '_'.join([s.upper() for s in split_camel_case_name(cls.name)])

class Writer(object):
    def __init__(self, mode, template, out):
        super(Writer, self).__init__()
        self.file = out
        self.out = StringIO.StringIO()
        self.mode = mode
        self.template = template
        if mode == 'python':
            self.constants = python_constants
        elif mode == 'lua':
            self.constants = lua_constants
        else:
            oops('unknown mode %s' % mode)

        self.section_suffix = ' (%s)' % self.mode.capitalize()

    def __format_symbol_ref(self, symbol):
        return symbol

    def __string_to_bool(self, s):
        if s == '0':
            return False
        elif s == '1':
            return True
        else:
            oops()

    def __check_bind_ann(self, obj):
        bind = self.__string_to_bool(obj.annotations.get('moo.' + self.mode, '1'))
        if bind:
            bind = not self.__string_to_bool(obj.annotations.get('moo.private', '0'))
        return bind

    def __format_constant(self, value):
        if value in self.constants:
            return self.constants[value]
        try:
            i = int(value)
            return value
        except ValueError:
            pass
        warning("unknown constant '%s'" % value)
        return value

    def __format_doc(self, doc):
        text = doc.text
        text = re.sub(r'@([\w\d_]+)(?!\{)', r'<parameter>\1</parameter>', text)
        text = re.sub(r'%NULL\b', '<constant>%s</constant>' % self.constants['NULL'], text)
        text = re.sub(r'%TRUE\b', '<constant>%s</constant>' % self.constants['TRUE'], text)
        text = re.sub(r'%FALSE\b', '<constant>%s</constant>' % self.constants['FALSE'], text)

        def repl_func(m):
            return '<function><link linkend="%(mode)s.%(func_id)s" endterm="%(mode)s.%(func_id)s.title"></link></function>' % \
                dict(func_id=m.group(1), mode=self.mode)
        text = re.sub(r'([\w\d_.]+)\(\)', repl_func, text)

        def repl_func(m):
            return self.__format_symbol_ref(m.group(1))
        text = re.sub(r'#([\w\d_]+)', repl_func, text)

        assert not re.search(r'NULL|TRUE|FALSE', text)
        return text

    def __make_class_name(self, cls):
        if self.mode == 'python':
            return 'moo.%s' % cls.short_name
        elif self.mode == 'lua':
            return 'medit.%s' % cls.short_name
        else:
            oops()

    def __get_obj_name(self, cls):
        name = cls.annotations.get('moo.doc-object-name')
        if not name:
            name = '_'.join([c.lower() for c in split_camel_case_name(cls.name)[1:]])
        return name

    def __write_function(self, func, cls):
        if not self.__check_bind_ann(func):
            return

        func_params = list(func.params)
        if func.has_gerror_return:
            func_params = func_params[:-1]
        params = []
        for p in func_params:
            if p.default_value is not None:
                params.append('%s=%s' % (p.name, self.__format_constant(p.default_value)))
            else:
                params.append(p.name)

        if isinstance(func, Constructor):
            if self.mode == 'python':
                func_title = cls.short_name + '()'
                func_name = cls.short_name
            elif self.mode == 'lua':
                func_title = 'new' + '()'
                func_name = '%s.new' % cls.short_name
            else:
                oops()
        elif isinstance(func, Signal):
            if self.mode == 'python':
                func_title = 'signal ' + func.name
                func_name = func.name
            elif self.mode == 'lua':
                func_title = 'signal ' + func.name
                func_name = func.name
            else:
                oops()
        elif cls is not None:
            if self.mode in ('python', 'lua'):
                func_title = func.name + '()'
                func_name = '%s.%s' % (self.__get_obj_name(cls), func.name) \
                                if not isinstance(func, StaticMethod) \
                                else '%s.%s' % (cls.short_name, func.name)
            else:
                oops()
        else:
            if self.mode == 'python':
                func_title = func.name + '()'
                func_name = 'moo.%s' % func.name
            elif self.mode == 'lua':
                func_title = func.name + '()'
                func_name = 'medit.%s' % func.name
            else:
                oops()

        params_string = ', '.join(params)

        if func.summary:
            oops(func_id)

        func_id = func.symbol_id()
        mode = self.mode

        self.out.write("""\
<sect2 id="%(mode)s.%(func_id)s">
<title id="%(mode)s.%(func_id)s.title">%(func_title)s</title>
<programlisting>%(func_name)s(%(params_string)s)</programlisting>
""" % locals())

        if func.doc:
            self.out.write('<para>%s</para>\n' % self.__format_doc(func.doc))

        has_param_docs = False
        for p in func_params:
            if p.doc:
                has_param_docs = True
                break

        if has_param_docs:
            self.out.write("""\
<variablelist>
<?dbhtml list-presentation="table"?>
<?dbhtml term-separator=" : "?>
""")

            for p in func_params:
                param_dic = dict(param=p.name, doc=self.__format_doc(p.doc))
                self.out.write("""\
<varlistentry>
 <term><parameter>%(param)s</parameter></term>
 <listitem><para>%(doc)s</para></listitem>
</varlistentry>
""" % param_dic)

            self.out.write('</variablelist>\n')


        self.out.write('</sect2>\n')

    def __write_gobject_constructor(self, cls):
        func = Constructor()
        self.__write_function(func, cls)

    def __write_class(self, cls):
        if not self.__check_bind_ann(cls):
            return

        do_write = False
        if cls.constructor is not None and self.__check_bind_ann(cls.constructor):
            do_write = True
        elif self.mode != 'lua' and hasattr(cls, 'constructable') and cls.constructable:
            do_write = True
        else:
            for meth in cls.methods:
                if self.__check_bind_ann(meth):
                    do_write = True
                    break
            if not do_write:
                for meth in cls.static_methods:
                    if self.__check_bind_ann(meth):
                        do_write = True
                        break
            if not do_write and hasattr(cls, 'signals'):
                for meth in cls.signals:
                    if self.__check_bind_ann(meth):
                        do_write = True
                        break

        if not do_write:
            return

        title = self.__make_class_name(cls)
        if cls.summary:
            title += ' - ' + cls.summary.text
        dic = dict(Class=self.__make_class_name(cls),
                   HELPSECTION='SCRIPT_%s_%s' % (self.mode.upper(), name_all_caps(cls)),
                   section_suffix=self.section_suffix,
                   title=title,
                   summary=cls.summary.text + '.' if cls.summary else '',
                   subsection='class %s' % self.__make_class_name(cls))
        self.out.write("""\
<sect1 id="%(Class)s">
<title>%(title)s</title>
""" % dic)

        if cls.doc:
            self.out.write('<para>%s</para>\n' % self.__format_doc(cls.doc))

        if hasattr(cls, 'signals') and cls.signals:
            for signal in cls.signals:
                self.__write_function(signal, cls)
        if cls.constructor is not None:
            self.__write_function(cls.constructor, cls)
        if hasattr(cls, 'constructable') and cls.constructable:
            self.__write_gobject_constructor(cls)
        for meth in cls.static_methods:
            self.__write_function(meth, cls)
        if isinstance(cls, Class):
            if cls.vmethods:
                implement_me('virtual methods of %s' % cls.name)
        for meth in cls.methods:
            self.__write_function(meth, cls)

        self.out.write("""\
</sect1>
""" % dic)

    def write(self, module):
        self.module = module

        for cls in module.get_classes() + module.get_boxed() + module.get_pointers():
            self.__write_class(cls)

        self.out.write("""\
<sect1 id="functions">
<title>Functions</title>
""")

        for func in module.get_functions():
            self.__write_function(func, None)

        self.out.write('</sect1>\n')

        content = self.out.getvalue()
        template = open(self.template).read()
        self.file.write(template.replace('###GENERATED###', content))

        del self.out
        del self.module
