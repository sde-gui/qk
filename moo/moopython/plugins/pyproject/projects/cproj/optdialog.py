import os.path
from moo.utils import _
import mprj.optdialog
from mprj.optdialog import page_new_from_file

dir = os.path.dirname(__file__)

class Page(mprj.optdialog.Page):
    pass

class Dialog(mprj.optdialog.Dialog):
    def __init__(self, project):
        mprj.optdialog.Dialog.__init__(self, project)

        page = page_new_from_file(os.path.join(dir, 'options.glade'), _('Options'))
        dialog.append_page(page)

        return dialog
