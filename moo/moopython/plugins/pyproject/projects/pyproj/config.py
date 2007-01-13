import os.path
import moo
from moo.utils import _

from mprj.settings import *
from mprj.simple import SimpleConfig


class RunOptions(Group):
    __items__ = {
        'exe' : String(name=_('Script file')),
        'args' : String(name=_('Arguments')),
        'vars' : Dict(str, xml_elm_name='var'),
    }
    __item_name__ = _('Run options')


class PyConfig(SimpleConfig):
    __items__ = {
        'run' : RunOptions,
    }

    def get_exe(self, top_dir):
        ro = self.run
        return ro.exe, ro.args, top_dir


_sample_file = """
    <medit-project name="foo" type="Python" version="2.0">
      <run>
        <exe>foo.py</exe>
        <args>--blah</args>
      </run>
    </medit-project>
    """

if __name__ == '__main__':
    from mprj.config import File

    s1 = _sample_file
    c = PyConfig(File(s1))
    s2 = str(c.get_xml())
    print s2
    c = PyConfig(File(s2))
    s3 = str(c.get_xml())
    assert s2 == s3
