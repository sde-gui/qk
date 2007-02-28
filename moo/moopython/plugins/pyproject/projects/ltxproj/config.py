import os.path
import moo

from mprj.utils import expand_command
from mprj.settings import *
from mprj.simple import SimpleProject, SimpleConfig


class Commands(Group):
    __items__ = {
        'latex' : Command(default=['$(srcdir)', 'latex $(basename)']),
        'viewdvi' : Command(default=['$(srcdir)', 'kdvi $(base).dvi)']),
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
    from mprj.config import File

    s1 = """
    <medit-project name="moo" type="LaTeX" version="1.0">
      <master>norm.tex</master>
    </medit-project>
    """

    c = LatexConfig(File(s1))
    s2 = str(c.get_xml())

    print s2

    c = LatexConfig(File(s2))
    s3 = str(c.get_xml())

    assert s2 == s3
