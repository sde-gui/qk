if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '..'))

import gtk
import moo
import os.path
import gobject

import mprj.utils
from mprj.simple import SimpleProject
from mprj.utils import print_error
from moo.utils import _, N_

import cproj.config
from cproj.config import CConfig
from cproj.parser import parse_make_error
from cproj.optdialog import Dialog as OptionsDialog


_STOCK_BUILD = moo.utils.STOCK_BUILD
_STOCK_COMPILE = moo.utils.STOCK_COMPILE
_STOCK_EXECUTE = moo.utils.STOCK_EXECUTE
_STOCK_PROJECT_OPTIONS = moo.utils.STOCK_PROJECT_OPTIONS

_BUILD_PANE_ID = "CProjectBuild"
_OUTPUT_PANE_ID = "CProjectOutput"

_CMD_BUILD       = 'build'
_CMD_COMPILE     = 'compile'
_CMD_CONFIGURE   = 'configure'
_CMD_AUTOGEN     = 'autogen'
_CMD_CLEAN       = 'clean'
_CMD_DISTCLEAN   = 'distclean'
_CMD_EXECUTE     = 'execute'
_CMD_INSTALL     = 'install'


class CProject(SimpleProject):
    __config__ = CConfig
    __factory_name__ = "C"
    __factory_template__ = cproj.config.factory_template

    class DoCmd(object):
        def __init__(self, obj, *args):
            object.__init__(self)
            self.obj = obj
            self.args = args
        def __call__(self, window):
            return self.obj.do_command(window, *self.args)

    def init_ui(self):
        SimpleProject.init_ui(self)

        self.panes.extend([_BUILD_PANE_ID, _OUTPUT_PANE_ID])

        commands = [
            ["Build", _("Build Project"), _STOCK_BUILD, "F8", _CMD_BUILD],
            ["Compile", _("Compile File"), _STOCK_COMPILE, "F9", _CMD_COMPILE],
            ["RunConfigure", _("Run Configure"), None, None, _CMD_CONFIGURE],
            ["RunAutogen", _("Run autogen.sh"), None, None, _CMD_AUTOGEN],
            ["Clean", _("Clean Project"), None, None, _CMD_CLEAN],
            ["Distclean", _("Distclean"), None, None, _CMD_DISTCLEAN],
            ["Execute", _("Execute Program"), _STOCK_EXECUTE, "<shift>F9", _CMD_EXECUTE],
            ["Install", _("Install"), None, "<shift><ctrl>I", _CMD_INSTALL],
        ]

        for c in commands:
            self.add_action("CProject" + c[0],
                            display_name=c[1], label=c[1],
                            stock_id=c[2], default_accel=c[3],
                            callback=CProject.DoCmd(self, c[4]))
        self.add_action("CProjectBuildConfiguration",
                        factory=_BuildConfigurationActionFactory(self))

        editor = moo.edit.editor_instance()
        xml = editor.get_ui_xml()
        xml.insert_markup_after(self.merge_id, "Editor/Menubar",
                                "Project", """
                                <item name="Build" _label="%s">
                                  <item action="CProjectBuild"/>
                                  <item action="CProjectCompile"/>
                                  <item action="CProjectRunConfigure"/>
                                  <item action="CProjectRunAutogen"/>
                                  <separator/>
                                  <item action="CProjectInstall"/>
                                  <separator/>
                                  <item action="CProjectClean"/>
                                  <item action="CProjectDistclean"/>
                                  <separator/>
                                  <item action="CProjectExecute"/>
                                </item>
                                """ % (N_("_Build"),))
        xml.insert_markup(self.merge_id, "Editor/Toolbar/BuildToolbar",
                          0, """
                          <item action="CProjectBuild"/>
                          <item action="CProjectExecute"/>
                          <separator/>
                          """)
        xml.insert_markup_before(self.merge_id,
            "Editor/Menubar/Project", "ProjectOptions",
            """
            <separator/>
            <item action="CProjectBuildConfiguration"/>
            """)

    def get_build_pane(self, window):
        pane = window.get_pane(_BUILD_PANE_ID)
        if not pane:
            label = moo.utils.PaneLabel(icon_name=_STOCK_BUILD,
                                        label_text=_("Build Messages"))
            output = moo.edit.CmdView()
            window.add_stop_client(output)

            if 1:
                output.set_property("highlight-current-line", True)
                output.set_wrap_mode(gtk.WRAP_NONE)
            else:
                output.set_property("highlight-current-line", False)
                output.set_wrap_mode(gtk.WRAP_CHAR)

            output.set_filter(moo.edit.command_filter_create('make'))

            pane = gtk.ScrolledWindow()
            pane.set_shadow_type(gtk.SHADOW_ETCHED_IN)
            pane.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
            pane.add(output)
            pane.show_all()

            pane.output = output
            window.add_pane(_BUILD_PANE_ID, pane, label, moo.utils.PANE_POS_BOTTOM)
        return pane

    def get_output_pane(self, window):
        pane = window.get_pane(_OUTPUT_PANE_ID)
        if not pane:
            label = moo.utils.PaneLabel(icon_name=_STOCK_EXECUTE,
                                        label_text=_("Output"))
            output = moo.edit.CmdView()
            window.add_stop_client(output)
            output.set_property("highlight-current-line", False)
            output.set_wrap_mode(gtk.WRAP_CHAR)

            pane = gtk.ScrolledWindow()
            pane.set_shadow_type(gtk.SHADOW_ETCHED_IN)
            pane.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
            pane.add(output)
            pane.show_all()

            pane.output = output
            window.add_pane(_OUTPUT_PANE_ID, pane, label, moo.utils.PANE_POS_BOTTOM)
        return pane

    def get_file_path(self, file):
        if os.path.exists(file):
            return file
        bd = self.config.get_build_dir(self.topdir)
        f = os.path.join(bd, file)
        if os.path.exists(f):
            return f
        f = os.path.join(self.topdir, file)
        if os.path.exists(f):
            return f
        return None

    def save_all(self, window):
        docs = window.list_docs()
        for d in docs:
            if d.get_filename() and d.get_status() & moo.edit.EDIT_MODIFIED:
                d.save()

    def do_command(self, window, cmd):
        try:
            self.before_command(window, cmd) and \
            self.exec_command(window, cmd)   and \
            self.after_command(window, cmd)
        except Exception, e:
            mprj.utils.oops(window, e)


    def before_command(self, window, cmd):
        self.save_all(window)
        return True

    def after_command(self, window, cmd):
        return True

    def __cmd_execute(self, window):
        exe = self.config.get_exe(self.topdir)
        assert exe
        pane = self.get_output_pane(window)
        pane.output.clear()
        window.paned.present_pane(pane)
        pane.output.run_command(exe)
        return True

    def __cmd_simple(self, cmd, filename, window):
        try:
            working_dir, command = self.config.get_command(cmd, filename, self.topdir)
        except Exception, e:
            print_error(e)
            return False
        pane = self.get_build_pane(window)
        pane.output.clear()
        window.paned.present_pane(pane)
        pane.output.run_command(command, working_dir)
        return True

    def exec_command(self, window, cmd):
        if cmd == _CMD_EXECUTE:
            return self.__cmd_execute(window)

        if cmd in [_CMD_BUILD, _CMD_INSTALL, _CMD_CONFIGURE,
                   _CMD_AUTOGEN, _CMD_CLEAN, _CMD_DISTCLEAN]:
            return self.__cmd_simple(cmd, None, window)

        if cmd in [_CMD_COMPILE]:
            doc = self.window.get_active_doc()
            filename = doc and doc.get_filename()
            if not filename:
                return False
            return self.__cmd_simple(cmd, filename, window)

        mprj.utils.implement_me(window, "Command " + cmd)
        return False

    def create_options_dialog(self):
        return OptionsDialog(self)

    def create_configurations_menu(self):
        menu = gtk.Menu()
        group = None
        for name in sorted(self.config.configurations.keys()):
            item = gtk.RadioMenuItem(group, name)
            group = item
            item.set_data('conf_name', name)
            item.show()
            if name == self.config.active:
                item.set_active(True)
            item.connect('toggled', self.conf_toggled, name)
            menu.add(item)
        return menu
    def conf_toggled(self, item, name):
        if not item.get_active() or not self.config.active \
           or item.get_data('conf_name') == self.config.active:
            return
        self.config.set_active_conf(item.get_data('conf_name'))
        self.save_config()

    def apply_config(self):
        action = self.window.get_action("CProjectBuildConfiguration")
        for p in action.get_proxies():
            if isinstance(p, gtk.MenuItem):
                p.set_submenu(self.create_configurations_menu())

class _BuildConfigurationActionFactory(object):
    def __init__(self, project):
        object.__init__(self)
        self.project = project
    def __call__(self, window):
        action = gobject.new(moo.utils.Action,
                             name="CProjectBuildConfiguration",
                             label="Build Configuration",
                             no_accel="True")
        action.project = self.project
        action.connect('connect-proxy', self.connect_proxy)
        return action
    def connect_proxy(self, action, menuitem):
        menuitem.set_submenu(self.project.create_configurations_menu())

__project__ = CProject
__project_type__ = "C"
__project_version__ = "2.0"
