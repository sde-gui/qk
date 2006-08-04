from mproj.project import Project
from mproj.config import Config, StringDict
from mproj.utils import print_error
from mproj.session import Session
import moo
import os.path


class SimpleConfig(Config):
    __attributes__ = { 'vars' : StringDict }


class SimpleProject(Project):

    __config__ = SimpleConfig

    def __init__(self, window, config):
        Project.__init__(self, window, config)

    def init_ui(self):
        Project.init_ui(self)

        self.filesel = None
        self.filesel_merge_id = 0

        plugin = moo.edit.plugin_lookup('FileSelector')
        if plugin:
            try:
                self.filesel = plugin.call_method('get-widget', self.window)
                if self.filesel:
                    self.filesel.chdir(self.topdir)
            except Exception:
                print_error()

    def deinit_ui(self):
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
        return self.window.close_all()

    def get_session_file(self):
        return os.path.join(self.topdir, '.' + \
                            os.path.basename(self.filename) + '.session')

    def load_session(self):
        try:
            file = self.get_session_file()
            if os.path.exists(file):
                Session(file).attach(self.window)
        except:
            print_error()

    def save_session(self):
        try:
            file = self.get_session_file()
            Session(self.window).save(file)
        except:
            print_error()
