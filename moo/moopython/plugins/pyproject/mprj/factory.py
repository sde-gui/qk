#
#  mprj/factory.py
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

import os
import gtk
import gobject
import moo
from moo import _, N_

import mprj.utils

class Factory(object):
    def __init__(self, name, template = None, project_type = None):
        object.__init__(self)
        self.name = name
        self.template = template
        self.project_type = project_type

    def create(self, proj_info):
        if not self.template:
            raise NotImplementedError("Missing template")
        content = self.template % proj_info.get_subst_dict()
        file = open(os.path.join(proj_info.dir, proj_info.file), 'w')
        file.write(content)
        file.close()

class ProjectInfo(object):
    def __init__(self, factory, dir, file, name):
        object.__init__(self)
        self.factory = factory
        self.dir = dir
        self.file = file
        self.name = name

    def get_subst_dict(self):
        return { 'project_name': self.name,
               }

_COLUMN_TEXT = 0
_COLUMN_FACTORY = 1

class _Wizard(object):
    def __setup_type_list(self, factories, xml):
        store = gtk.ListStore(str, object)
        for f in factories:
            store.append([f.name, f])
        treeview = xml.w_type
        treeview.set_model(store)
        cell = gtk.CellRendererText()
        column = gtk.TreeViewColumn(None, cell, text=_COLUMN_TEXT)
        treeview.append_column(column)
        treeview.get_selection().select_path('0')

    def __check_sensitivity(self):
        sensitive = self.xml.w_name.get_text() != '' and \
                    self.xml.w_location.get_text() != ''
        self.xml.w_dialog.set_response_sensitive(gtk.RESPONSE_OK, sensitive)
    def __update_file_label(self):
        if self.xml.w_name.get_text() != '' and self.xml.w_location.get_text() != '':
            d, f = self.__get_project_file()
            self.xml.w_file.set_text(os.path.join(d, f))
        else:
            self.xml.w_file.set_text('')
    def __name_changed(self, *whatever):
        self.__check_sensitivity()
        self.__update_file_label()
    def __location_changed(self, *whatever):
        self.__check_sensitivity()
        self.__update_file_label()
    def __location_button_clicked(self, *whatever):
        path = moo.file_dialogp(self.xml.w_dialog,
                                moo.FILE_DIALOG_OPEN_DIR,
                                self.xml.w_location.get_text(),
                                _("Choose Folder"),
                                mprj.utils.prefs_key('new_project_dir'))
        if path:
            self.xml.w_location.set_text(path)

    def __get_project_file(self):
        dir = self.xml.w_location.get_text()
        file = ''
        for c in self.xml.w_name.get_text():
            if c in "/":
                c = '_'
            file += c
        file += '.mprj'
        return [dir, file]

    def ask(self, factories, window):
        glade_file = os.path.join(os.path.dirname(__file__), "factory.glade")
        xml = moo.glade_xml_new_from_file(glade_file, domain=moo.GETTEXT_PACKAGE)
        self.xml = xml

        self.__setup_type_list(factories, xml)

        xml.w_location.set_text(os.path.expanduser('~'))
        xml.w_location_button.connect('clicked', self.__location_button_clicked)
        xml.w_name.connect('changed', self.__name_changed)
        xml.w_location.connect('changed', self.__location_changed)

        dialog = xml.w_dialog
        dialog.set_response_sensitive(gtk.RESPONSE_OK, False)
        dialog.set_transient_for(window)

        try:
            while 1:
                if dialog.run() != gtk.RESPONSE_OK:
                    return None

                d, f = self.__get_project_file()

                if os.path.lexists(d) and not os.path.isdir(d):
                    moo.error_dialog(dialog, '%s is not a directory' % (d,))
                    continue

                if os.path.exists(os.path.join(d, f)):
                    if not moo.overwrite_file_dialog(dialog, f, d):
                        continue

                if os.path.exists(d) and not os.access(d, os.W_OK):
                    moo.error_dialog(dialog, '%s is not writable' % (d,))
                    continue
                if not os.path.exists(d):
                    try:
                        os.mkdir(d)
                    except Exception, e:
                        moo.error_dialog(dialog, 'Could not create directory %s' % (d,), str(e))
                        continue

                model, iter = xml.w_type.get_selection().get_selected()
                factory = model.get_value(iter, _COLUMN_FACTORY)

                pi = ProjectInfo(factory, d, f, xml.w_name.get_text())

                return pi
        finally:
            dialog.destroy()

    def run(self, factories, window):
        pi = self.ask(factories, window)
        if pi is None:
            return None
        try:
            pi.factory.create(pi)
        except Exception, e:
            moo.error_dialog(window, 'Could not create project', str(e))
            return None
        return os.path.join(pi.dir, pi.file)

def new_project(project_types, window):
    factories = []
    for pt in project_types:
        factory = None
        ft = getattr(pt, '__factory__', None)
        if ft is not None:
            factory = ft()
        else:
            ft = getattr(pt, '__factory_template__', None)
            if ft:
                factory = Factory(pt.__factory_name__, ft, pt)
        if factory is not None:
            factories.append(factory)

    if not factories:
        return None

    w = _Wizard()
    return w.run(factories, window)
