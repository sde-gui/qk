if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '..'))

import gobject
import os.path
import moo
from moo.utils import _

from cproj.view import *
from cproj.config import *

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


class ConfigsPage(moo.utils.PrefsDialogPage):
    __label__ = _('Configurations')
    __types__ = {'build_dir' : Entry,
                 'args' : Entry,
                 'vars' : DictView}

    def load_config(self, config):
        self.xml.w_build_dir.set_setting(config['build_dir'])
        self.xml.w_args.set_setting(config.configure['args'])
        self.xml.w_vars.set_dict(config.configure['vars'])

    def combo_changed(self, *whatever):
        self.do_apply()
        combo = self.xml.w_configuration
        iter = combo.get_active_iter()
        store = combo.get_model()
        config = store.get_value(iter, 1)
        self.load_config(config)

    def init_combo(self):
        store = gtk.ListStore(str, object)
        combo = self.xml.w_configuration
        combo.set_model(store)
        cell = gtk.CellRendererText()
        combo.pack_start(cell)
        combo.set_attributes(cell, text=0)

        active = (0,)
        configurations = self.config.configurations
        for name in configurations:
            c = configurations[name]
            iter = store.append([c.name, c])
            if c is self.config.get_active_conf():
                active = store.get_path(iter)
        combo.set_active_iter(store.get_iter(active))

        combo.connect('changed', self.combo_changed)

    def do_init(self):
        self.widgets = [self.xml.w_build_dir, self.xml.w_args]
        self.init_combo()
        self.combo_changed()

    def do_apply(self):
        for widget in self.widgets:
            widget.apply()


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

        run_from = self.config.run.run_from
        if run_from == RUN_FROM_BUILD_DIR:
            self.xml.w_build_dir.set_active(True)
        elif run_from == RUN_FROM_EXE_DIR:
            self.xml.w_exe_dir.set_active(True)
        else:
            self.xml.w_custom_dir.set_active(True)
            self.xml.w_custom_dir_entry.set_text(run_from)
        moo.utils.bind_sensitive(self.xml.w_custom_dir,
                                 self.xml.w_custom_dir_entry)

    def do_apply(self):
        for widget in self.widgets:
            widget.apply()

        if self.xml.w_build_dir.get_active():
            self.config.run.run_from = RUN_FROM_BUILD_DIR
        elif self.xml.w_exe_dir.get_active():
            self.config.run.run_from = RUN_FROM_EXE_DIR
        else:
            self.config.run.run_from = self.xml.w_custom_dir_entry.get_text()


class BuildCommandsPage(moo.utils.PrefsDialogPage):
    __label__ = _('Build commands')
    __types__ = {'commands' : GroupView}

    def do_init(self):
        self.xml.w_commands.set_items(self.config.commands.items())
        self.widgets = [self.xml.get_widget(name) for name in ['commands']]

    def do_apply(self):
        for widget in self.widgets:
            widget.apply()


class Dialog(moo.utils.PrefsDialog):
    def __init__(self, project, title=_('Project Options')):
        moo.utils.PrefsDialog.__init__(self, title)
        self.project = project
        self.config_copy = project.config.copy()
        self.append_page(create_page(ConfigsPage, 'page_configs', self.config_copy))
        self.append_page(create_page(RunOptionsPage, 'page_run', self.config_copy))
        self.append_page(create_page(BuildCommandsPage, 'page_commands', self.config_copy))

    def do_apply(self):
        moo.utils.PrefsDialog.do_apply(self)
        self.project.config.copy_from(self.config_copy)
        self.project.save_config()
#         print '============================='
#         print self.project.config.dump_xml()
#         print '============================='


gobject.type_register(ConfigsPage)
gobject.type_register(RunOptionsPage)
gobject.type_register(BuildCommandsPage)
gobject.type_register(Dialog)


if __name__ == '__main__':
    import gtk
    from cproj.config import CConfig, _sample_file
    from mprj.config import File
    from c import CProject

    editor = moo.edit.create_editor_instance()
    config = CConfig(File(_sample_file, '/tmp/fake/file'))
    project = CProject(None, config)
    dialog = Dialog(project)
    dialog.connect('destroy', gtk.main_quit)
    dialog.run()
    gtk.main()
