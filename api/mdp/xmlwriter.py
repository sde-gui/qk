import xml.etree.ElementTree as etree

import mdp.module as module

class Writer(object):
    def __init__(self, out):
        object.__init__(self)
        self.out = out
        self.xml = etree.TreeBuilder()
        self.__tag_opened = False
        self.__depth = 0

    def __start_tag(self, tag, attrs={}):
        if self.__tag_opened:
            self.xml.data('\n')
        if self.__depth > 0:
            self.xml.data('  ' * self.__depth)
        elm = self.xml.start(tag, attrs)
        self.__tag_opened = True
        self.__depth += 1
        return elm

    def __end_tag(self, tag):
        if not self.__tag_opened and self.__depth > 1:
            self.xml.data('  ' * (self.__depth - 1))
        elm = self.xml.end(tag)
        self.xml.data('\n')
        self.__tag_opened = False
        self.__depth -= 1
        return elm

    def __write_docs(self, docs):
        if not docs:
            return
        self.__start_tag('doc')
        docs = ' '.join(docs)
        self.xml.data(docs)
        self.__end_tag('doc')

    def __write_param_or_retval_annotations(self, param, elm):
        for k in param.attributes:
            elm.set(k, self.attributes[v])
        if param.transfer_mode is not None:
            elm.set('transfer_mode', param.transfer_mode)
        if param.element_type is not None:
            elm.set('element_type', param.element_type)
        if param.array:
            elm.set('array', '1')
        if param.array_fixed_len is not None:
            elm.set('array_fixed_len', str(param.array_fixed_len))
        if param.array_len_param is not None:
            elm.set('array_len_param', param.array_len_param)
        if param.array_zero_terminated:
            elm.set('array_zero_terminated', '1')

    def __write_param_annotations(self, param, elm):
        self.__write_param_or_retval_annotations(param, elm)
        if param.inout:
            elm.set('inout', '1')
        elif param.out:
            elm.set('out', '1')
        if param.caller_allocates:
            elm.set('caller_allocates', '1')
        elif param.callee_allocates:
            elm.set('callee_allocates', '1')
        if param.allow_none:
            elm.set('allow_none', '1')
        if param.default_value is not None:
            elm.set('default_value', param.default_value)
        if param.scope != 'call':
            elm.set('scope', param.scope)

    def __write_retval_annotations(self, retval, elm):
        self.__write_param_or_retval_annotations(retval, elm)

    def __write_param(self, param):
        dic = dict(name=param.name, type=param.type)
        elm = self.__start_tag('param', dic)
        self.__write_param_annotations(param, elm)
        self.__write_docs(param.docs)
        self.__end_tag('param')

    def __write_retval(self, retval):
        dic = dict(type=retval.type)
        elm = self.__start_tag('retval', dic)
        self.__write_retval_annotations(retval, elm)
        self.__write_docs(retval.docs)
        self.__end_tag('retval')

    def __write_class(self, cls):
        if not cls.parent:
            raise RuntimeError('parent missing in class %s' % (cls.name,))
        dic = dict(name=cls.name, short_name=cls.short_name, parent=cls.parent, gtype_id=cls.gtype_id)
        if cls.constructable:
            dic['constructable'] = '1'
        self.__start_tag('class', dic)
        if cls.constructor is not None:
            self.__write_function(cls.constructor, 'constructor')
        for meth in sorted(cls.vmethods, lambda x, y: cmp(x.name, y.name)):
            self.__write_function(meth, 'virtual')
        for meth in sorted(cls.methods, lambda x, y: cmp(x.name, y.name)):
            self.__write_function(meth, 'method')
        self.__write_docs(cls.docs)
        self.__end_tag('class')

    def __write_boxed(self, cls):
        dic = dict(name=cls.name, short_name=cls.short_name, gtype_id=cls.gtype_id)
        tag = 'boxed' if isinstance(cls, module.Boxed) else 'pointer'
        self.__start_tag(tag, dic)
        if cls.constructor is not None:
            self.__write_function(cls.constructor, 'constructor')
        for meth in cls.methods:
            self.__write_function(meth, 'method')
        self.__write_docs(cls.docs)
        self.__end_tag(tag)

    def __write_enum(self, enum):
        if isinstance(enum, module.Enum):
            tag = 'enum'
        else:
            tag = 'flags'
        dic = dict(name=enum.name, short_name=enum.short_name, gtype_id=enum.gtype_id)
        self.__start_tag(tag, dic)
        self.__write_docs(enum.docs)
        self.__end_tag(tag)

    def __write_function(self, func, tag):
        dic = dict(name=func.name)
        if tag != 'virtual':
            dic['c_name'] = func.c_name
        for k in func.annotations:
            dic[k] = func.annotations[k]
        self.__start_tag(tag, dic)
        for p in func.params:
            self.__write_param(p)
        if func.retval:
            self.__write_retval(func.retval)
        self.__write_docs(func.docs)
        self.__end_tag(tag)

    def write(self, module):
        self.__start_tag('module', dict(name=module.name))
        for cls in sorted(module.classes, lambda x, y: cmp(x.name, y.name)):
            self.__write_class(cls)
        for cls in sorted(module.boxed, lambda x, y: cmp(x.name, y.name)):
            self.__write_boxed(cls)
        for enum in sorted(module.enums, lambda x, y: cmp(x.name, y.name)):
            self.__write_enum(enum)
        for func in sorted(module.functions, lambda x, y: cmp(x.name, y.name)):
            self.__write_function(func, 'function')
        self.__end_tag('module')
        elm = self.xml.close()
        etree.ElementTree(elm).write(self.out)

def write_xml(module, out):
    Writer(out).write(module)
