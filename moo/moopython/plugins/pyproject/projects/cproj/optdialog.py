#
#  cproj/optdialog.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

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
from moo import _

import mprj.optdialog
import mprj.simple
from mprj.config.view import *
from cproj.config import *


class ConfigsPage(mprj.optdialog.ConfigPage):
    __label__ = _('Configurations')
    __types__ = {'config_build_dir' : Entry,
                 'config_args' : Entry,
                 'config_vars' : DictView}

    def load_config(self, config):
        if config is not None:
            self.xml.w_config_build_dir.set_setting(config['build_dir'])
            self.xml.w_config_args.set_setting(config.configure['args'])
            self.xml.w_config_vars.set_dict(config.configure['vars'])
        else:
            self.xml.w_config_build_dir.set_setting(None)
            self.xml.w_config_args.set_setting(None)
            self.xml.w_config_vars.set_dict(None)

    def combo_changed(self, *whatever):
        self.do_apply()
        combo = self.xml.w_configuration
        iter = combo.get_active_iter()
        config = None
        if iter is not None:
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
        for name in sorted(configurations.keys()):
            c = configurations[name]
            iter = store.append([c.name, c])
            if c is self.config.get_active_conf():
                active = store.get_path(iter)
        combo.set_active_iter(store.get_iter(active))

        combo.connect('changed', self.combo_changed)

    def update_buttons(self):
        self.xml.w_delete_configuration.set_sensitive(len(self.config.configurations) > 1)

    def rename_config_cb(self, *whatever):
        combo = self.xml.w_configuration
        iter = combo.get_active_iter()
        store = combo.get_model()
        config = store.get_value(iter, 1)

        name = config.name
        new_name = mprj.utils.entry_dialog(parent=self,
                                           label=_("Enter new name"),
                                           entry_content=name,
                                           title=_("Enter new name"))

        if not new_name or new_name == name:
            return

        if self.config.configurations.has_key(new_name):
            mprj.utils.error_dialog(parent=self, text=_("Configuration '%s' already exists") % (new_name))
            return

        was_active = self.config.active == name
        self.config.rename_conf(name, new_name)
        store.set_value(iter, 0, new_name)
        if was_active:
            self.config.active = new_name

    def add_config_cb(self, *whatever):
        name = mprj.utils.entry_dialog(parent=self,
                                       label=_("Enter configuration name"),
                                       title=_("Enter configuration name"))

        if not name:
            return

        if self.config.configurations.has_key(name):
            mprj.utils.error_dialog(parent=self, text=_("Configuration '%s' already exists") % (name))
            return

        new_conf = self.config.add_conf(name)

        combo = self.xml.w_configuration
        store = combo.get_model()
        iter = store.append([name, new_conf])
        combo.set_active_iter(iter)

    def delete_config_cb(self, *whatever):
        combo = self.xml.w_configuration
        iter = combo.get_active_iter()
        store = combo.get_model()
        config = store.get_value(iter, 1)

        do_delete = mprj.utils.question_dialog(parent=self,
                                               text=_("Delete configuration %s?" % (config.name,)),
                                               default_ok=False)

        if do_delete:
            self.config.delete_conf(config.name)
            store.remove(iter)
            combo.set_active(0)
            self.update_buttons()

    def do_init(self):
        mprj.optdialog.ConfigPage.do_init(self)
        self.init_combo()
        self.combo_changed()
        self.xml.w_add_configuration.connect('clicked', self.add_config_cb)
        self.xml.w_rename_configuration.connect('clicked', self.rename_config_cb)
        self.xml.w_delete_configuration.connect('clicked', self.delete_config_cb)
        self.update_buttons()


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
        moo.bind_sensitive(self.xml.w_custom_dir,
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
        self.xml.w_commands.set_group(self.config.commands)

    def do_apply(self):
        mprj.optdialog.ConfigPage.do_apply(self)

class Dialog(mprj.optdialog.Dialog):
    def __init__(self, project):
        mprj.optdialog.Dialog.__init__(self, project)
        glade_file = os.path.join(os.path.dirname(__file__), 'options.glade')
#         self.append_page(mprj.simple.ConfigPage(self.config_copy))
        self.append_page(ConfigsPage('page_configs', self.config_copy, glade_file))
        self.append_page(RunOptionsPage('page_run', self.config_copy, glade_file))
        self.append_page(BuildCommandsPage('page_commands', self.config_copy, glade_file))

    def do_apply(self):
#         print '--------------------------------'
#         print self.config_copy.commands.compile
#         print '--------------------------------'
        mprj.optdialog.Dialog.do_apply(self)
        self.project.apply_config()
#         print '--------------------------------'
#         print self.config_copy.commands.compile
#         print '--------------------------------'
#         print self.project.config.commands.compile
#         print '--------------------------------'


gobject.type_register(ConfigsPage)
gobject.type_register(RunOptionsPage)
gobject.type_register(BuildCommandsPage)
gobject.type_register(Dialog)


if __name__ == '__main__':
    import gtk
    from cproj.config import CConfig, _sample_file
    from mprj.config import File
    from c import CProject

    editor = moo.create_editor_instance()
    config_file = File(_sample_file, '/tmp/test-file.mprj')
    config = CConfig(config_file)
    project = CProject(None, config, config_file)
    dialog = Dialog(project)
    dialog.connect('destroy', gtk.main_quit)
    dialog.run()
    gtk.main()
