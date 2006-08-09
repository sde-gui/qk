__all__ = ['Config']

from mprj.config._group import Group, _GroupMeta
from mprj.config._xml import XMLGroup


class Config(Group):
    __no_item_methods__ = True

    # override parent's __call__ method, so Config() works as intended
    class __metaclass__(_GroupMeta):
        def __call__(self, *args, **kwargs):
            kwargs['do_create_instance'] = True
            obj = _GroupMeta.__call__(self, *args, **kwargs)
            del kwargs['do_create_instance']
            return obj

    def __init__(self, file):
        Group.__init__(self, 'medit-project')
        self.file = file
        self.name = file.name
        self.type = file.project_type
        self.version = file.version
        self.load_xml()

    def load_xml(self):
        Group.load(self, self.file.root)

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
