import os.path
import moo

from mproj.utils import expand_command
from mproj.config import Dict, StringDict, List
from mproj.simple import SimpleProject, SimpleConfig


class MakeOptions(Dict):
    __attributes__ = {'flags' : str,
                      'cmd' : str,
                      'n_jobs' : int,
                      'vars' : StringDict}

class ConfigureOptions(Dict):
    __attributes__ = {'args' : str,
                      'cppflags' : str,
                      'ldflags' : str,
                      'cflags' : str,
                      'cxxflags' : str,
                      'cc' : str,
                      'cxx' : str,
                      'vars' : StringDict}

class Commands(Dict):
    __attributes__ = {'build' : [str, 'cd $(top_builddir) && $(make)'],
                      'compile' : [str, 'cd $(builddir) && $(make) $(base).o'],
                      'configure' : [str, 'cd $(top_builddir) && $(top_srcdir)/configure $(configure_args)'],
                      'autogen' : [str, 'cd $(top_builddir) && $(top_srcdir)/autogen.sh $(configure_args)'],
                      'clean' : [str, 'cd $(top_builddir) && $(make) clean'],
                      'distclean' : [str, 'cd $(top_builddir) && $(make) distclean'],
                      'install' : [str, 'cd $(top_builddir) && $(make) install']}

class RunOptions(Dict):
    __attributes__ = {'run_from' : str,
                      'run_from_dir' : str,
                      'exe' : str,
                      'args' : str,
                      'vars' : StringDict}

    def load(self, node):
        Dict.load(self, node)

        if not self.run_from:
            self.run_from = 'build_dir'
        if self.run_from and self.run_from not in ['exe_dir', 'build_dir', 'custom_dir']:
                raise TypeError
        if self.run_from == 'custom_dir':
            if not self.run_from_dir:
                raise TypeError

class BuildConfiguration(Dict):

    __attributes__ = {'build_dir' : str,
                      'make' : MakeOptions,
                      'run' : RunOptions,
                      'configure' : ConfigureOptions}

    def load(self, node):
        self.name = node.name
        Dict.load(self, node)


class CConfig(SimpleConfig):
    __attributes__ = {'run' : RunOptions,
                      'make' : MakeOptions,
                      'configurations' : List(BuildConfiguration),
                      'active' : str,
                      'commands' : [Commands, Commands()]}

    def load(self):
        SimpleConfig.load(self)

        if not self.configurations:
            raise RuntimeError("No configurations defined")

        if self.active:
            confs = [c.name for c in self.configurations]
            if self.active not in confs:
                raise RuntimeError("Invalid configuration %s" % (self.active,))
        else:
            self.active = self.configurations[0].name

        self.confs = {}
        for c in self.configurations:
            self.confs[c.name] = c

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
    from mproj.configxml import File

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
    c.load()
    s2 = str(c.get_xml())

    print s2

    c = CConfig(File(s2))
    c.load()
    s3 = str(c.get_xml())

    assert s2 == s3
