if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../..'))
    sys.path.insert(0, os.path.join(dir, '..'))

import gobject
import gtk
import os.path

import moo
from moo.utils import _

import mprj.optdialog
from mprj.config.view import *
from cproj.config import *


class ConfigsPage(mprj.optdialog.ConfigPage):
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
        mprj.optdialog.ConfigPage.do_init(self)
        self.init_combo()
        self.combo_changed()


class RunOptionsPage(mprj.optdialog.ConfigPage):
    __label__ = _('Run options')
    __types__ = {'vars' : DictView,
                 'exe' : Entry,
                 'args' : Entry}

    def do_init(self):
        mprj.optdialog.ConfigPage.do_init(self)
        self.xml.w_vars.set_dict(self.config.run.vars)
        self.xml.w_exe.set_setting(self.config.run['exe'])
        self.xml.w_args.set_setting(self.config.run['args'])

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
        mprj.optdialog.ConfigPage.do_apply(self)

        if self.xml.w_build_dir.get_active():
            self.config.run.run_from = RUN_FROM_BUILD_DIR
        elif self.xml.w_exe_dir.get_active():
            self.config.run.run_from = RUN_FROM_EXE_DIR
        else:
            self.config.run.run_from = self.xml.w_custom_dir_entry.get_text()


class BuildCommandsPage(mprj.optdialog.ConfigPage):
    __label__ = _('Build commands')
    __types__ = {'commands' : GroupView}

    def do_init(self):
        mprj.optdialog.ConfigPage.do_init(self)
        self.xml.w_commands.set_items(self.config.commands.items())


class Dialog(mprj.optdialog.Dialog):
    def __init__(self, project):
        mprj.optdialog.Dialog.__init__(self, project)
        glade_file = os.path.join(os.path.dirname(__file__), 'options.glade')
#         self.append_page(mprj.simple.ConfigPage(self.config_copy))
        self.append_page(ConfigsPage('page_configs', self.config_copy, glade_file))
        self.append_page(RunOptionsPage('page_run', self.config_copy, glade_file))
        self.append_page(BuildCommandsPage('page_commands', self.config_copy, glade_file))


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
    config_file = File(_sample_file, '/tmp/fake/file')
    config = CConfig(config_file)
    project = CProject(None, config, config_file)
    dialog = Dialog(project)
    dialog.connect('destroy', gtk.main_quit)
    dialog.run()
    gtk.main()
