from mprj.settings import *
from mprj.config import File, Config

f = File("""<medit-project version="1.0" name="Foo" type="Simple">
              <variables>
                <foo>bar</foo>
              </variables>
              <project_dir>.</project_dir>
              <stuff>
                <kff>ddd</kff>
              </stuff>
            </medit-project>""")

class D(Group):
    __items__ = {'kff' : String, 'blah' : String}
D('ddd')

List(D)

class C(Config):
    __items__ = {'variables' : Dict,
                 'project_dir' : String,
                 'stuff' : D,
                 'somestuff' : D}

c = C(f)
c.load()
# print c.variables
# print c.project_dir
# print c.stuff
# print f
# print c
print c.get_xml()
