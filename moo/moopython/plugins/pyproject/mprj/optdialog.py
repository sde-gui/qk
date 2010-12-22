#
#  mprj/optdialog.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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
import os.path
import moo
from moo import _

dir = os.path.dirname(__file__)

def _init_page(page, page_id, config, glade_file):
    cls = page.__class__

    xml = moo.GladeXML(moo.GETTEXT_PACKAGE)
    xml.map_id(page_id, cls)

    types = getattr(cls, '__types__', {})
    for id in types:
        xml.map_id(id, types[id])

    glade_file = open(glade_file)
    try:
        xml.fill_widget(page, glade_file.read(), page_id)
        assert xml.get_widget(page_id) is page
    finally:
        glade_file.close()

    label = getattr(cls, '__label__', None)
    if label is not None:
        page.set_property('label', label)

    page.xml = xml
    page.config = config
    page.widgets = [xml.get_widget(k) for k in types]

    return page


class ConfigPage(moo.PrefsPage):
    def __init__(self, page_id, config, glade_file):
        moo.PrefsPage.__init__(self)
        _init_page(self, page_id, config, glade_file)

    def do_init(self):
        pass

    def do_apply(self):
        for widget in self.widgets:
            widget.apply()


class Dialog(moo.PrefsDialog):
    def __init__(self, project, title=_('Project Options')):
        moo.PrefsDialog.__init__(self, title)
        self.project = project
        self.config_copy = project.config.copy()

    def do_apply(self):
        moo.PrefsDialog.do_apply(self)
        self.project.config.copy_from(self.config_copy)
        self.project.save_config()
#         print '============================='
#         print self.config_copy.dump_xml()
#         print '============================='
#         print self.project.config.dump_xml()
#         print '============================='


gobject.type_register(ConfigPage)
gobject.type_register(Dialog)
