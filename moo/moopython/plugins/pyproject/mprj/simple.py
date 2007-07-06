if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../..'))
    sys.path.insert(0, os.path.join(dir, '..'))

import gobject
import os.path
import shutil

import moo
from moo.utils import _

from mprj.project import Project
from mprj.config import Config, Dict
from mprj.settings import Filename
from mprj.utils import print_error
from mprj.session import Session
import mprj.optdialog


class SimpleConfig(Config):
    __items__ = {
        'vars' : Dict(str, name=_('Environment variables')),
    }


class SimpleProject(Project):

    __config__ = SimpleConfig

    def __init__(self, *args, **kwargs):
        Project.__init__(self, *args, **kwargs)
        self.__file_selector_dir = None
        self.filesel = None
        self.filesel_merge_id = 0
        self.__filesel_cb_id = 0

    def __filesel_cb(self, filesel, *whatever):
        self.__file_selector_dir = filesel.get_property('current-directory')
    def __setup_file_selector(self):
        plugin = moo.edit.plugin_lookup('FileSelector')
        if plugin:
            try:
                self.filesel = plugin.call_method('get-widget', self.window)
                if self.filesel:
                    last_dir = self.__file_selector_dir
                    if not last_dir:
                        last_dir = self.topdir
                    self.filesel.chdir(last_dir)
                    self.__filesel_cb_id = self.filesel.connect('notify::current-directory', self.__filesel_cb)
            except:
                print_error()

    def init_ui(self):
        Project.init_ui(self)

    def deinit_ui(self):
        if self.__filesel_cb_id:
            self.filesel.disconnect(self.__filesel_cb_id)
            self.__filesel_cb_id = 0
        if self.filesel and self.filesel_merge_id:
            xml = self.filesel.get_ui_xml()
            xml.remove_ui(self.filesel_merge_id)
        self.filesel = None
        self.filesel_merge_id = 0
        Project.deinit_ui(self)

    def load(self):
        Project.load(self)
        self.load_session()

    def close(self):
        self.save_session()
        self.save_config()
        return self.window.close_all()

    def get_session_file(self):
        return os.path.join(self.topdir, '.' + \
                            os.path.basename(self.filename) + '.session')

    def load_session(self):
        try:
            file = self.get_session_file()
            if not os.path.exists(file):
                return
            session = Session(file)
            session.attach(self.window)
            self.__file_selector_dir = session.get_file_selector_dir()
            self.__setup_file_selector()
        except:
            print_error()

    def save_session(self):
        try:
            file = self.get_session_file()
            session = Session(self.window)
            session.set_file_selector_dir(self.__file_selector_dir)
            session.save(file)
        except:
            print_error()

    def save_config(self):
        try:
            tmpname = os.path.join(os.path.dirname(self.filename), '.' + os.path.basename(self.filename) + '.tmp')
            tmp = open(tmpname, "w")
            tmp.write(self.config.format())
            tmp.close()
            shutil.copymode(self.filename, tmpname)
            shutil.move(self.filename, self.filename + '.bak')
            shutil.move(tmpname, self.filename)
        except:
            print_error()

    def options_dialog(self, window):
        dialog = self.create_options_dialog()
        dialog.run(window)

    def create_options_dialog(self):
        return None


class ConfigPage(mprj.optdialog.ConfigPage):
    __label__ = _("General")
    __types__ = {}

    def __init__(self, config):
        mprj.optdialog.ConfigPage.__init__(self, "page", config,
                                           os.path.join(os.path.dirname(__file__), "simple.glade"))

    def do_init(self):
        mprj.optdialog.ConfigPage.do_init(self)

    def do_apply(self):
        mprj.optdialog.ConfigPage.do_apply(self)


gobject.type_register(ConfigPage)


if __name__ == '__main__':
    from mprj.config import File

    s1 = """
    <medit-project name="moo" type="Simple" version="2.0">
      <vars>
        <foo>bar</foo>
        <blah>bom</blah>
      </vars>
    </medit-project>
    """

    c = SimpleConfig(File(s1))
    s2 = str(c.get_xml())

    print s2

    c = SimpleConfig(File(s2))
    s3 = str(c.get_xml())

    assert s2 == s3
