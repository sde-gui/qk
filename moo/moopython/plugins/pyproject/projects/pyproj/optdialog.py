if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '..'))

import gobject
import os.path
import moo
from moo.utils import _

from pyproj.view import *
from pyproj.config import *

dir = os.path.dirname(__file__)

def create_page(cls, page_id, config, types={}, label=None, file='options.glade'):
    file = os.path.join(dir, file)

    xml = moo.utils.GladeXML(moo.utils.GETTEXT_PACKAGE)
    xml.map_id(page_id, cls)

    if not types and hasattr(cls, '__types__'):
        types = getattr(cls, '__types__')
    for id in types:
        xml.map_id(id, types[id])

    page = cls()
    file = open(file)
    try:
        xml.fill_widget(page, file.read(), page_id)
        assert xml.get_widget(page_id) is page
    finally:
        file.close()

    if not label and hasattr(cls, '__label__'):
        label = getattr(cls, '__label__')
    if label:
        page.set_property('label', label)

    page.xml = xml
    page.config = config

    return page


class RunOptionsPage(moo.utils.PrefsDialogPage):
    __label__ = _('Run options')
    __types__ = {'vars' : DictView,
                 'exe' : Entry,
                 'args' : Entry}

    def do_init(self):
        self.xml.w_vars.set_dict(self.config.run.vars)
        self.xml.w_exe.set_setting(self.config.run['exe'])
        self.xml.w_args.set_setting(self.config.run['args'])
        self.widgets = [self.xml.get_widget(name) for name in ['vars', 'exe', 'args']]

    def do_apply(self):
        for widget in self.widgets:
            widget.apply()


class Dialog(moo.utils.PrefsDialog):
    def __init__(self, project, title=_('Project Options')):
        moo.utils.PrefsDialog.__init__(self, title)
        self.project = project
        self.config_copy = project.config.copy()
        self.append_page(create_page(RunOptionsPage, 'page_run', self.config_copy))

    def do_apply(self):
        moo.utils.PrefsDialog.do_apply(self)
        self.project.config.copy_from(self.config_copy)
        self.project.save_config()
        print '============================='
        print self.project.config.dump_xml()
        print '============================='


gobject.type_register(RunOptionsPage)
gobject.type_register(Dialog)


if __name__ == '__main__':
    import gtk
    from pyproj.config import PyConfig, _sample_file
    from mprj.config import File
    from python import PyProject

    editor = moo.edit.create_editor_instance()
    file = File(_sample_file, '/tmp/fake/file')
    config = PyConfig(file)
    project = PyProject(None, config, file)
    dialog = Dialog(project)
    dialog.connect('destroy', gtk.main_quit)
    dialog.run()
    gtk.main()
