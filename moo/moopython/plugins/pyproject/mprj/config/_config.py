__all__ = ['Config']

from mprj.config._group import Group, _GroupMeta
from mprj.config._xml import XMLGroup, File


class Config(Group):
    __no_item_methods__ = True

    # override parent's __call__ method, so Config() works as intended
    class __metaclass__(_GroupMeta):
        def __call__(self, *args, **kwargs):
            kwargs['_do_create_instance'] = True
            obj = _GroupMeta.__call__(self, *args, **kwargs)
            del kwargs['_do_create_instance']
            return obj

    def __init__(self, file):
        Group.__init__(self, 'medit-project')
        if file is not None:
            self.load_xml(file.root)
            self.name = file.name
            self.type = file.project_type
            self.version = file.version

    def copy(self):
        copy = type(self)(None)
        copy.copy_from(self)
        return copy

    def copy_from(self, other):
        self.name = other.name
        self.type = other.type
        self.version = other.version
        return Group.copy_from(self, other)

    def load_xml(self, xml):
        Group.load(self, xml)

    def format(self):
        return '<?xml version="1.0" encoding="UTF-8"?>\n' + \
                self.get_xml().get_string()

    def dump_xml(self):
        return self.get_xml().get_string()

    def get_xml(self):
        xml = Group.save(self)
        if xml:
            xml = xml[0]
        else:
            xml = XMLGroup('medit-project')
        xml.set_attr('name', self.name)
        xml.set_attr('type', self.type)
        xml.set_attr('version', self.version)
        return xml
