import os.path
import moo

from mprj.utils import expand_command
from mprj.settings import *
from mprj.simple import SimpleProject, SimpleConfig


class Commands(Group):
    __items__ = {
        'latex' : [String, {'default' : 'cd $(srcdir) && latex $(basename)'}],
        'viewdvi' : [String, {'default' : 'cd $(srcdir) && kdvi $(base).dvi'}]
    }


class LatexConfig(SimpleConfig):
    __items__ = {
        'master' : String,
        'commands' : Commands
    }

    def get_command(self, cmd, filename, topdir):
        if self.master:
            filename = self.master
        if not os.path.isabs(filename):
            filename = os.path.join(topdir, filename)
        return expand_command(getattr(self.commands, cmd),
                              None, filename, topdir, None)

if __name__ == '__main__':
    from mprj.configxml import File

    s1 = """
    <medit-project name="moo" type="LaTeX" version="1.0">
      <master>norm.tex</master>
    </medit-project>
    """

    c = LatexConfig(File(s1))
    c.load()
    s2 = str(c.get_xml())

    print s2

    c = LatexConfig(File(s2))
    c.load()
    s3 = str(c.get_xml())

    assert s2 == s3
