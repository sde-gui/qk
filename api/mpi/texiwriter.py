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
    def __init__(self, mode, out):
        super(Writer, self).__init__()
        self.out = out
        self.mode = mode
        self.part_body = StringIO.StringIO()
        self.part_menu = StringIO.StringIO()
        if mode == 'python':
            self.constants = python_constants
        elif mode == 'lua':
            self.constants = lua_constants
        else:
            oops('unknown mode %s' % mode)

        self.section_suffix = ' (%s)' % self.mode.capitalize()

    def __check_bind_ann(self, obj):
        bind = obj.annotations.get('moo.' + self.mode, '1')
        if bind == '0':
            return False
        elif bind == '1':
            return True
        else:
            oops()

    def __format_constant(self, value):
        if value in self.constants:
            return self.constants[value]
        try:
            i = int(value)
            return value
        except ValueError:
            pass
        oops("unknown constant '%s'" % value)

    def __format_doc(self, text):
        text = re.sub(r'@([\w\d_]+)(?!\{)', r'@param{\1}', text)
        text = re.sub(r'%method{', r'@method{', text)
        text = re.sub(r'%NULL\b', '@code{%s}' % self.constants['NULL'], text)
        text = re.sub(r'%TRUE\b', '@code{%s}' % self.constants['TRUE'], text)
        text = re.sub(r'%FALSE\b', '@code{%s}' % self.constants['FALSE'], text)
        assert not re.search(r'NULL|TRUE|FALSE', text)
        return text

    def __make_class_name(self, cls):
        if self.mode == 'python':
            return 'moo.%s' % cls.short_name
        elif self.mode == 'lua':
            return 'medit.%s' % cls.short_name
        else:
            oops()

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

        dic = dict(func=func.name,
                   params=', '.join(params))
        self.part_body.write("""\
@item %(func)s(%(params)s)
""" % dic)

        if func.doc:
            self.part_body.write(self.__format_doc(func.doc.text))
            self.part_body.write('\n')

        self.part_body.write('\n')

    def __write_class(self, cls):
        if not self.__check_bind_ann(cls):
            return

        do_write = False
        if cls.constructor is not None and self.__check_bind_ann(cls.constructor):
            do_write = True
        else:
            for meth in cls.methods:
                if self.__check_bind_ann(meth):
                    do_write = True
                    break
        if not do_write:
            return

        dic = dict(Class=self.__make_class_name(cls),
                   HELPSECTION='SCRIPT_%s_%s' % (self.mode.upper(), name_all_caps(cls)),
                   section_suffix=self.section_suffix,
                   summary=cls.summary.text + '.' if cls.summary else '',
                   subsection='class %s' % self.__make_class_name(cls))
        self.part_menu.write("""\
* %(Class)s%(section_suffix)s:: %(summary)s
""" % dic)
        self.part_body.write("""\
@node %(Class)s%(section_suffix)s
@subsection %(subsection)s
@helpsection{%(HELPSECTION)s}

""" % dic)

        if cls.doc:
            self.part_body.write(self.__format_doc(cls.doc.text))
            self.part_body.write('\n')

        self.part_body.write("""\
@table @method

""" % dic)

        if cls.constructor is not None:
            self.__write_function(cls.constructor, cls)
        if hasattr(cls, 'constructable') and cls.constructable:
            implement_me('GObject constructor of %s' % cls.name)
        if isinstance(cls, Class):
            if cls.vmethods:
                implement_me('virtual methods of %s' % cls.name)
        for meth in cls.methods:
            self.__write_function(meth, cls)

        self.part_body.write("""\
@end table

""" % dic)

    def write(self, module):
        self.module = module

        self.part_menu.write("""\
@menu
""")

        for cls in module.get_classes() + module.get_boxed() + module.get_pointers():
            self.__write_class(cls)

        dic = dict(HELPSECTION='SCRIPT_%s_FUNCTIONS' % self.mode.upper(),
                   section_suffix=self.section_suffix)
        self.part_menu.write("""\
* Functions%(section_suffix)s:: Functions.
""" % dic)
        self.part_body.write("""\
@node Functions%(section_suffix)s
@subsection Functions
@helpsection{%(HELPSECTION)s}
@table @method

""" % dic)

        for func in module.get_functions():
            self.__write_function(func, None)

        self.part_body.write("""\
@end table

""" % dic)

        self.part_menu.write("""\
@end menu

""")

        self.out.write(self.part_menu.getvalue())
        self.out.write(self.part_body.getvalue())

        del self.module
