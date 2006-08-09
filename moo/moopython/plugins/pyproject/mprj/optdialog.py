import moo
import gobject
import gtk
from moo.utils import _

class Page(moo.utils.PrefsDialogPage):
    pass

class Dialog(moo.utils.PrefsDialog):
    def __init__(self, project, title=_('Project Options')):
        moo.utils.PrefsDialog.__init__(self, title)
        self.project = project

def page_new_from_file(file, label, cls=Page, page_id='page'):
    xml = moo.utils.GladeXML(moo.utils.GETTEXT_PACKAGE)
    xml.map_id(page_id, cls)
    xml.parse_file(file, page_id)

    page = xml.get_widget(page_id)
    page.xml = xml
    page.set_property('label', label)

    return page

gobject.type_register(Page)
gobject.type_register(Dialog)
