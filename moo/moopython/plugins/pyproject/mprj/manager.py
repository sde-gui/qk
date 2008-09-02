import moo
import sys
import os, os.path
import gobject
from mprj.config import File
from mprj.simple import SimpleProject
import mprj.utils
import mprj.factory
from mprj.utils import print_error, format_error
from moo.utils import _, N_


PROJECT_VERSION = "1.0"


moo.utils.prefs_new_key('Plugins/Project/last', str, None, moo.utils.PREFS_STATE)
moo.utils.prefs_new_key('Plugins/Project/last_dir', str, None, moo.utils.PREFS_STATE)
# moo.utils.prefs_new_key_string('Plugins/Project/last', None)
# moo.utils.prefs_new_key_string('Plugins/Project/last_dir', None)


class _ProjectStore(object):
    def __init__(self):
        object.__init__(self)
        self.projects = {}
    def add(self, name, project):
        if self.projects.has_key(name):
            raise RuntimeError("Project '%s' already registered" % (name,))
        self.projects[name] = project
    def remove(self, name):
        if not self.projects.has_key(name):
            raise RuntimeError("Project '%s' not registered" % (name,))
        del self.projects[name]
    def get(self, name):
        if not self.projects.has_key(name):
            raise RuntimeError("Project '%s' not registered" % (name,))
        return self.projects[name]


class _OpenRecent(object):
    def __init__(self, mgr):
        object.__init__(self)
        self.mgr = mgr

    def __call__(self, window):
        action = moo.utils.MenuAction("OpenRecentProject", _("Open Recent Project"))
        action.set_property('display-name', _("Open Recent Project"))
        action.set_mgr(self.mgr.recent_list.get_menu_mgr())
        moo.utils.bind_bool_property(action, "sensitive",
                                     self.mgr.recent_list, "empty",
                                     True)
        return action


class Manager(object):
    def __init__(self, project):
        object.__init__(self)
        self.init(project)

    def init(self, project):
        editor = moo.edit.editor_instance()
        editor.set_property("allow-empty-window", True)
        editor.set_property("single-window", True)

        moo.utils.window_class_add_action(moo.edit.EditWindow, "NewProject",
                                          display_name=_("New Project"),
                                          label=_("New Project..."),
                                          stock_id=moo.utils.STOCK_NEW_PROJECT,
                                          callback=self.new_project_cb)
        moo.utils.window_class_add_action(moo.edit.EditWindow, "OpenProject",
                                          display_name=_("Open Project"),
                                          label=_("Open Project"),
                                          stock_id=moo.utils.STOCK_OPEN_PROJECT,
                                          callback=self.open_project_cb)
        moo.utils.window_class_add_action(moo.edit.EditWindow, "ProjectOptions",
                                          display_name=_("Project _Options"),
                                          label=_("Project Options"),
                                          stock_id=moo.utils.STOCK_PROJECT_OPTIONS,
                                          callback=self.project_options_cb)
        moo.utils.window_class_add_action(moo.edit.EditWindow, "CloseProject",
                                          display_name=_("Close Project"),
                                          label=_("Close Project"),
                                          stock_id=moo.utils.STOCK_CLOSE_PROJECT,
                                          callback=self.close_project_cb)

        self.__init_project_types()

        self.project_to_open = project
        self.project = None
        self.window = None
        self.recent_list = moo.utils.HistoryList("ProjectManager")
        self.recent_list.connect("activate_item", self.recent_item_activated)
        moo.utils.window_class_add_action(moo.edit.EditWindow,
                                          "OpenRecentProject",
                                          factory=_OpenRecent(self))

        xml = editor.get_ui_xml()
        self.merge_id = xml.new_merge_id()
        xml.insert_markup_after(self.merge_id, "Editor/Menubar",
                                "View", """
                                <item name="Project" _label="%s">
                                  <item action="NewProject"/>
                                  <item action="OpenProject"/>
                                  <item action="OpenRecentProject"/>
                                  <separator/>
                                  <item action="ProjectOptions"/>
                                  <separator/>
                                  <item action="CloseProject"/>
                                  <separator/>
                                </item>
                                """ % (N_("_Project"),))


    def deinit(self):
        self.close_project(True)
        for a in ["NewProject", "OpenProject", "CloseProject",
                  "ProjectOptions", "OpenRecentProject"]:
            moo.utils.window_class_remove_action(moo.edit.EditWindow, a)
        editor = moo.edit.editor_instance()
        editor.get_ui_xml().remove_ui(self.merge_id)
        self.merge_id = 0
        del self.project_types

    def recent_item_activated(self, recent_list, item, whatever):
        try:
            self.open_project(self.window, item.data)
        except Exception, e:
            self.bad_project(self.window, item.data, e)
            self.recent_list.remove(item.data)

    def attach_win(self, window):
        if self.project:
            self.__set_title_prefix(self.project.name)
        else:
            self.__set_title_prefix(None)
        self.window = window
        self.window.connect('close', self.close_window)
        action = self.window.get_action("CloseProject")
        if action:
            action.set_property("sensitive", False)
        action = self.window.get_action("ProjectOptions")
        if action:
            action.set_property("visible", self.project is not None)

        if not self.project:
            project = self.project_to_open
            self.project_to_open = None
            if not project:
                project = moo.utils.prefs_get_string("Plugins/Project/last")
            if not project:
                project = os.path.join(moo.utils.get_user_data_dir(), "default.mprj")
            if project and os.path.exists(project):
                try:
                    self.open_project(self.window, project)
                except Exception, e:
                    print_error(e)

    def __set_title_prefix(self, prefix):
        editor = moo.edit.editor_instance()
        editor.set_app_name(prefix or "medit")

    def detach_win(self, window):
        self.close_project(True)
        self.__set_title_prefix(None)
        self.window = None

    def new_project_cb(self, window):
        try:
            self.new_project(window)
        except Exception, e:
            mprj.utils.oops(window, e)

    def open_project_cb(self, window):
        filename = moo.utils.file_dialogp(parent=window, title=_("Open Project"),
                                          prefs_key="Plugins/Project/last_dir")
        if not filename:
            return

        try:
            self.open_project(window, filename)
        except Exception, e:
            self.bad_project(window, filename, e)

    def bad_project(self, parent, filename, error):
        moo.utils.error_dialog(parent, _("Could not open project '%s'") % (filename,), str(error))

    def fixme(self, parent, msg):
        moo.utils.warning_dialog(parent, "FIXME", str(msg))

    def close_window(self, window):
        return not self.close_project(False)

    def project_options_cb(self, window):
        if self.project:
            self.project.options_dialog(window)
        else:
            self.fixme(window, "disable Close Project command")

    def close_project_cb(self, window):
        if self.project:
            if self.close_project(False):
                moo.utils.prefs_set_string("Plugins/Project/last", None)
        else:
            self.fixme(window, "disable Close Project command")


    """ new_project """
    def new_project(self, window):
        if not self.close_project(False):
            return
        filename = mprj.factory.new_project(self.project_types.projects.values(), window)
        if filename:
            self.open_project(window, filename)
            if self.project:
                self.project.options_dialog(window)


    """ open_project """
    def open_project(self, window, filename):
        if not self.close_project(False):
            return

        f = open(filename)
        file = File(f.read())
        f.close()

        file.path = filename
        project_type = self.project_types.get(file.project_type)
        config_type = getattr(project_type, '__config__')
        config = config_type(file)

        self.project = project_type(window, config, file)
        self.project.load()
        self.__set_title_prefix(self.project.name)
        self.recent_list.add_filename(filename)
        moo.utils.prefs_set_string("Plugins/Project/last", filename)

        close = self.window.get_action("CloseProject")
        options = self.window.get_action("ProjectOptions")
        if close:
            close.set_property("sensitive", True)
        if options:
            options.set_property("visible", True)


    """ close_project """
    def close_project(self, force=False):
        if not self.project:
            return True

        if not self.project.close() and not force:
            return False

        self.project.unload()
        self.project = None

        if self.window:
            close = self.window.get_action("CloseProject")
            options = self.window.get_action("ProjectOptions")
            if close:
                close.set_property("sensitive", False)
            if close:
                options.set_property("visible", False)
            self.__set_title_prefix(None)

        return True


    def __read_project_file(self, path):
        try:
            dic = {'__name__' : '__project__', '__builtins__' : __builtins__}
            execfile(path, dic)
            if not dic.has_key('__project__'):
                print "File %s doesn't define __project__ attribute" % (path,)
            elif not dic.has_key('__project_type__'):
                print "File %s doesn't define __project_type__ attribute" % (path,)
            elif not dic.has_key('__project_version__'):
                print "File %s doesn't define __project_version__ attribute" % (path,)
            elif dic['__project_version__'] != PROJECT_VERSION:
                print "In file %s: version %s does not match current version %s" % (
                        path, dic['__project_version__'], PROJECT_VERSION)
            else:
                self.project_types.add(dic['__project_type__'], dic['__project__'])
        except Exception, e:
            print_error(e)

    def __init_project_types(self):
        self.project_types = _ProjectStore()
        self.project_types.add("Simple", SimpleProject)

        dirs = list(moo.utils.get_data_subdirs("projects", moo.utils.DATA_LIB))
        dirs = filter(lambda d: os.path.isdir(d), dirs)
        dirs.reverse()

        if not dirs:
            return

        saved_path = sys.path
        sys.path = list(dirs) + list(saved_path)

        for d in dirs:
            try:
                if not os.path.isdir(d):
                    continue
                files = os.listdir(d)
                for f in files:
                    path = os.path.join(d, f)
                    if os.path.isfile(path):
                        self.__read_project_file(path)
            except:
                print_error()

        sys.path = saved_path
