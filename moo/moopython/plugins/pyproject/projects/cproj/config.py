import os.path
import moo
from moo.utils import _

from mprj.utils import expand_command
from mprj.settings import *
from mprj.simple import SimpleProject, SimpleConfig


class MakeOptions(Group):
    __items__ = {
        'flags' : String,
        'cmd' : String,
        'n_jobs' : Int(default=1),
        'vars' : Dict(str)
    }
    __item_name__ = _('Make options')
    __item_descrption__ = _('Make options')

class ConfigureOptions(Group):
    __items__ = {
        'args' : String(name=_('Configure arguments')),
        'cppflags' : String,
        'ldflags' : String,
        'cflags' : String,
        'cxxflags' : String,
        'cc' : String,
        'cxx' : String,
        'vars' : Dict(str, name=_('Environment variables'))
    }
    __item_name__ = _('Configure options')
    __item_descrption__ = _('Configure options')

class Commands(Group):
    __items__ = {
        'build' : String(default='cd $(top_builddir) && $(make)'),
        'compile' : String(default='cd $(builddir) && $(make) $(base).o'),
        'configure' : String(default='cd $(top_builddir) && $(top_srcdir)/configure $(configure_args)'),
        'autogen' : String(default='cd $(top_builddir) && $(top_srcdir)/autogen.sh $(configure_args)'),
        'clean' : String(default='cd $(top_builddir) && $(make) clean'),
        'distclean' : String(default='cd $(top_builddir) && $(make) distclean'),
        'install' : String(default='cd $(top_builddir) && $(make) install')
    }
    __item_name__ = _('Build commands')
    __item_descrption__ = _('Build commands')

class RunOptions(Group):
    __items__ = {
        'run_from' : String,
        'run_from_dir' : String,
        'exe' : String,
        'args' : String,
        'vars' : Dict(str)
    }
    __item_name__ = _('Run options')
    __item_descrption__ = _('Run options')

    def load(self, node):
        Group.load(self, node)

        if not self.run_from:
            self.run_from = 'build_dir'
        if self.run_from not in [None, '', 'exe_dir', 'build_dir', 'custom_dir']:
            raise TypeError()
        if self.run_from == 'custom_dir':
            if not self.run_from_dir:
                raise TypeError()

class BuildConfiguration(Group):
    __items__ = {
        'build_dir' : String(name=_('Build directory')),
        'make' : MakeOptions,
        'run' : RunOptions,
        'configure' : ConfigureOptions
    }
    __item_name__ = _('Build configuration')
    __item_descrption__ = _('Build configuration')

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

    def load_xml(self):
        SimpleConfig.load_xml(self)

        if not len(self.configurations):
            raise RuntimeError("No configurations defined")

        if self.active:
            if self.active not in self.configurations.keys():
                raise RuntimeError("Invalid configuration %s" % (self.active,))
        else:
            self.active = self.configurations.keys()[0]

        self.confs = {}
        for name in self.configurations:
            self.confs[name] = self.configurations[name]

    def get_active_conf(self):
        if not self.confs:
            raise RuntimeError("no configurations")
        if not self.active:
            self.active = 'default'
        if not self.confs.has_key(self.active):
            return self.confs.items()[0][1]
        else:
            return self.confs[self.active]

    def get_make_options(self):
        ac = self.get_active_conf()
        if ac.make:
            return ac.make
        else:
            return self.make

    def __get_make(self, suffix=None):
        mo = self.get_make_options()
        vars = ""
        if mo.vars:
            for v in mo.vars:
                 vars += "%s='%s' " % (v[0], v[1])
        cmd = self.get_make_command()
        flags = ""
        if mo.n_jobs:
            flags += " -j " + str(mo.n_jobs)
        if mo.flags:
            flags += str(mo.flags)
        if suffix:
            return "%s%s%s %s" % (vars, cmd, flags, suffix)
        else:
            return "%s%s%s" % (vars, cmd, flags)

    def get_make(self):
        return self.__get_make()

    def get_make_install(self):
        return self.__get_make("install")

    def get_exe(self, top_dir):
        ac = self.get_active_conf()
        ro = ac.run
        if not ro: ro = self.run

        vars = ""
        if ro.vars:
            for v in ro.vars:
                 vars += "%s='%s' " % (v[0], v[1])

        builddir = self.get_build_dir(top_dir)
        exe = ro.exe

        if ro.run_from == 'exe_dir':
            working_dir = os.path.join(builddir, os.path.dirname(exe))
            exe = "./" + os.path.basename(exe)
        elif ro.run_from == 'build_dir':
            working_dir = builddir
            exe = "./" + exe
        else:
            working_dir = ro.run_from_dir
            if not os.path.isabs(exe):
                exe = os.path.join(top_dir, builddir, exe)

        args = ro.args
        if not args: args = ''

        return "cd '%s' && %s%s %s" % (working_dir, vars, exe, args)

    def get_make_command(self):
        mo = self.get_make_options()
        if mo.cmd:
            return mo.cmd
        else:
            return "make"

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
            'configure_args' : self.get_active_conf().configure.args
        }

    def get_command(self, cmd, filename, topdir):
        return expand_command(getattr(self.commands, cmd),
                              self.__get_cmd_dict(),
                              filename, topdir,
                              self.get_build_dir(topdir))

if __name__ == '__main__':
    from mprj.config import File

    s1 = """
    <medit-project name="moo" type="C" version="2.0">
      <make>
        <n_jobs>3</n_jobs>
      </make>
      <configurations>
        <debug>
          <run>
            <exe>medit</exe>
            <args>--g-fatal-warnings</args>
          </run>
          <configure>
            <args>--enable-debug=full --enable-all-gcc-warnings</args>
            <cflags>-O0 -g3 -pg</cflags>
          </configure>
          <build_dir>build/debug</build_dir>
        </debug>
        <optimized>
          <run>
            <exe>medit</exe>
          </run>
          <configure>
            <args>--enable-all-gcc-warnings</args>
          </configure>
          <build_dir>build/optimized</build_dir>
        </optimized>
      </configurations>
      <active>debug</active>
    </medit-project>
    """

    c = CConfig(File(s1))
    s2 = str(c.get_xml())

    print s2

    c = CConfig(File(s2))
    s3 = str(c.get_xml())

    assert s2 == s3
