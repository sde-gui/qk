if __name__ == '__main__':
    import sys
    import os.path
    dir = os.path.dirname(__file__)
    sys.path.insert(0, os.path.join(dir, '../..'))
    sys.path.insert(0, os.path.join(dir, '..'))

import os.path
import moo
from moo.utils import _

from mprj.utils import expand_command
from mprj.settings import *
from mprj.simple import SimpleProject, SimpleConfig
from mprj.config._xml import XMLItem, XMLGroup

class MakeOptions(Group):
    __items__ = {
        'cmd' : String(default='make'),
        'args' : String(null_ok=True),
        'vars' : Dict(str, xml_elm_name='var')
    }
    __item_name__ = _('Make options')


class ConfigureOptions(Group):
    __items__ = {
        'args' : String(name=_('Configure arguments')),
        'vars' : Dict(str, name=_('Environment variables'), xml_elm_name='var'),
    }
    __item_name__ = _('Configure options')


class Commands(Group):
    __items__ = {
        'build' : Command(default=['$(top_builddir)', '$(make)']),
        'compile' : Command(default=['$(builddir)', '$(make) $(base).o']),
        'configure' : Command(default=['$(top_builddir)', '$(configure_vars) $(top_srcdir)/configure $(configure_args)']),
        'autogen' : Command(default=['$(top_builddir)', '$(configure_vars) $(top_srcdir)/autogen.sh $(configure_args)']),
        'clean' : Command(default=['$(top_builddir)', '$(make) clean']),
        'distclean' : Command(default=['$(top_builddir)', '$(make) distclean']),
        'install' : Command(default=['$(top_builddir)', '$(make) install']),
    }
    __item_name__ = _('Build commands')


RUN_FROM_BUILD_DIR = 0
RUN_FROM_EXE_DIR = 1

class RunFrom(Setting):
    __item_name__ = _('Run from')
    __item_default__ = RUN_FROM_BUILD_DIR

    def equal(self, value):
        if value is None:
            value = 0
        return Setting.equal(self, value)

    def set_string(self, value):
        raise RuntimeError()

    def check_value(self, value):
        return value in [RUN_FROM_BUILD_DIR, RUN_FROM_EXE_DIR] or \
               (isinstance(value, str) and value != '')

    def load(self, node):
        type_s = node.get_attr('type')
        if type_s == 'build-dir':
            return self.set_value(RUN_FROM_BUILD_DIR)
        elif type_s == 'exe-dir':
            return self.set_value(RUN_FROM_EXE_DIR)
        elif type_s == 'dir':
            return self.set_value(node.get())
        else:
            raise RuntimeError()

    def save(self):
        if self.is_default():
            return []

        value = self.get_value()

        if value in [RUN_FROM_BUILD_DIR, RUN_FROM_EXE_DIR]:
            dir = None
        else:
            dir = value

        item = XMLItem(self.get_id(), dir)

        if value == RUN_FROM_BUILD_DIR:
            item.set_attr('type', 'build-dir')
        elif value == RUN_FROM_EXE_DIR:
            item.set_attr('type', 'exe-dir')
        else:
            item.set_attr('type', 'dir')

        return [item]


class RunOptions(Group):
    __items__ = {
        'run_from' : RunFrom,
        'exe' : String(name=_('Executable')),
        'args' : String(name=_('Arguments')),
        'vars' : Dict(str, xml_elm_name='var')
    }
    __item_name__ = _('Run options')


class BuildConfiguration(Group):
    __items__ = {
        'build_dir' : String(name=_('Build directory')),
        'configure' : ConfigureOptions
    }
    __item_name__ = _('Build configuration')

    def copy_from(self, other):
        self.name = other.name
        return Group.copy_from(self, other)

    def load(self, node):
        self.name = node.name
        Group.load(self, node)


class CConfig(SimpleConfig):
    __items__ = {
        'run' : RunOptions,
        'make' : MakeOptions,
        'configurations' : Dict(BuildConfiguration),
        'active' : String,
        'commands' : Commands
    }

    def load_xml(self, xml):
        SimpleConfig.load_xml(self, xml)

        if not len(self.configurations):
            raise RuntimeError("No configurations defined")

        if self.active:
            if self.active not in self.configurations.keys():
                raise RuntimeError("Invalid configuration %s" % (self.active,))
        else:
            self.active = self.configurations.keys()[0]

    def set_active_conf(self, name):
        if self.active == name:
            return
        if not self.configurations.has_key(self.active):
            raise RuntimeError("no configuration named '%s'" % (name,))
        self.active = name

    def get_active_conf(self):
        if len(self.configurations) == 0:
            raise RuntimeError("no configurations")
        if not self.active:
            self.active = 'default'
        if not self.configurations.has_key(self.active):
            return self.configurations.items()[0][1]
        else:
            return self.configurations[self.active]

    def expand_env(self, vars):
        string = ''
        if vars:
            for v in vars:
                 string += "%s='%s' " % (v, vars[v])
        return string

    def __get_make(self, suffix=None):
        mo = self.make
        env = self.expand_env(mo.vars)
        cmd = mo.cmd or 'make'
        args = (mo.args and ' ' + mo.args) or ''
        suffix = (suffix and ' ' + suffix) or ''
        return '%s%s%s%s' % (env, cmd, args, suffix)

    def get_make(self):
        return self.__get_make()

    def get_make_install(self):
        return self.__get_make("install")

    def get_exe(self, top_dir):
        ro = self.run
        env = self.expand_env(ro.vars)
        args = ro.args or ''
        builddir = self.get_build_dir(top_dir)
        exe = ro.exe

        if ro.run_from == RUN_FROM_EXE_DIR:
            working_dir = os.path.join(builddir, os.path.dirname(exe))
            exe = "./" + os.path.basename(exe)
        elif ro.run_from == RUN_FROM_BUILD_DIR:
            working_dir = builddir
            exe = os.path.join("./", exe)
        else:
            working_dir = ro.run_from
            if not os.path.isabs(exe):
                exe = os.path.join(builddir, exe)

        return "cd '%s' && %s%s %s" % (working_dir, env, exe, args)

    def get_build_dir(self, top_dir):
        conf = self.get_active_conf()
        build_dir = conf.build_dir
        if os.path.isabs(build_dir):
            return build_dir
        else:
            return os.path.join(top_dir, build_dir)

    def __get_cmd_dict(self):
        return {
            'make' : self.__get_make(),
            'configure_args' : self.get_active_conf().configure.args,
            'configure_vars' : self.expand_env(self.get_active_conf().configure.vars),
        }

    def get_command(self, cmd, filename, topdir):
        return expand_command(getattr(self.commands, cmd),
                              self.__get_cmd_dict(),
                              filename, topdir,
                              self.get_build_dir(topdir))


_sample_file = """
    <medit-project name="moo" type="C" version="2.0">
      <commands>
        <compile>
          <cmd>foolala</cmd>
          <working_dir>foolala</working_dir>
        </compile>
      </commands>
      <make>
        <flags>-j 3</flags>
      </make>
      <run>
        <exe>medit</exe>
        <args>--g-fatal-warnings</args>
      </run>
      <configurations>
        <debug>
          <configure>
            <args>--enable-debug=full --enable-all-gcc-warnings</args>
            <vars>
              <var name="CFLAGS">-O0 -g3 -pg</var>
            </vars>
          </configure>
          <build_dir>build/debug</build_dir>
        </debug>
        <optimized>
          <configure>
            <args>--enable-all-gcc-warnings</args>
            <vars>
              <var name="CFLAGS">-g -O2</var>
            </vars>
          </configure>
          <build_dir>build/optimized</build_dir>
        </optimized>
      </configurations>
      <active>debug</active>
    </medit-project>
    """

if __name__ == '__main__':
    from mprj.config import File

    s1 = _sample_file
    c = CConfig(File(s1))
    s2 = str(c.get_xml())
    print s2
    c = CConfig(File(s2))
    s3 = str(c.get_xml())
    assert s2 == s3
