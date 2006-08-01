import cgi
import xml.dom
from xml.dom.minidom import parseString


class BadFile(Exception):
    pass

class File(object):
    def __init__(self, string, path=None):
        self.xml = XML(string)
        self.root = self.xml.root

        if self.root.name != 'medit-project':
            raise BadFile('Invalid root element "%s"' % (self.root.name,))

        self.path = path
        self.version = self.root.get_attr('version')
        self.name = self.root.get_attr('name')
        self.project_type = self.root.get_attr('type')

        if not self.version:
            raise BadFile('Version missing')
        if not self.name:
            raise BadFile('Name missing')
        if not self.project_type:
            raise BadFile('Project type missing')


_INDENT_STRING = " "

class Node(object):
    def __init__(self, name):
        if not name:
            raise ValueError()
        self.name = name
        self.parent = None
        self.__attrs = {}
    def get_attr(self, attr):
        if self.__attrs.has_key(attr):
            return self.__attrs[attr]
        else:
            return None
    def set_attr(self, attr, val):
        if val is None:
            if self.__attrs.has_key(attr):
                del self.__attrs[attr]
        else:
            self.__attrs[attr] = val
    def load_xml(self, elm):
        for i in range(elm.attributes.length):
            item = elm.attributes.item(i)
            self.set_attr(item.name, item.value)
    def format_start(self):
        s = '<' + self.name
        for k in self.__attrs:
            s += ' %s="%s"' % (k, cgi.escape(str(self.__attrs[k]), True))
        return s

    def get_string(self):
        raise NotImplementedError()

    def __repr__(self):
        return self.get_string()

class Dir(Node):
    def __init__(self, name, children=[]):
        Node.__init__(self, name)
        self.__children = []
        self.__children_names = {}

        if children:
            for c in children:
                self.add_child(c)

    def children(self):
        return self.__children

    def get_child(self, name):
        if self.__children_names.has_key(name):
            return self.__children_names[name]
        else:
            return None

    def add_child(self, child, index=-1):
        if not isinstance(child, Node):
            raise TypeError
        if index < 0 or index >= len(self.__children):
            self.__children.append(child)
        else:
            self.__children.insert(index, child)
        self.__children_names[child.name] = child
        child.parent = self

    def remove_child(self, child):
        if isinstance(child, str):
            child = self.get_child(child)
            if not child is None:
                self.remove_child(child)
            return
        if self.__children_names[child.name] is child:
            del self.__children_names[child.name]
        self.__children.remove(child)
        child.parent = None

    def get_xml_elm_type(self, elm):
        if len(elm.childNodes) > 1:
            return Dir
        if not elm.childNodes:
            return Text
        c = elm.childNodes[0]
        if c.nodeType == xml.dom.Node.ELEMENT_NODE:
            return Dir
        elif c.nodeType == xml.dom.Node.TEXT_NODE:
            return Text

    def load_xml(self, elm):
        Node.load_xml(self, elm)
        for c in elm.childNodes:
            if c.nodeType == xml.dom.Node.ELEMENT_NODE:
                t = self.get_xml_elm_type(c)
                if t is Dir:
                    child = Dir(c.tagName)
                    self.add_child(child)
                    child.load_xml(c)
                elif t is Text:
                    child = Text(c.tagName)
                    self.add_child(child)
                    child.load_xml(c)

    def get_string(self, indent=0):
        s = _INDENT_STRING * indent + self.format_start() + ">\n"
        for child in self.children():
            s += child.get_string(indent+1)
        s += _INDENT_STRING * indent + "</%s>\n" % (self.name,)
        return s

class Text(Node):
    def __init__(self, name, content=None):
        Node.__init__(self, name)
        self.set(content)

    def get(self):
        return self.__content

    def set(self, content):
        self.__content = content

    def get_string(self, indent=0):
        s = _INDENT_STRING * indent + self.format_start()
        if self.get() is None:
            s += "/>\n"
        else:
            s += ">%s</%s>\n" % (cgi.escape(str(self.get())), self.name)
        return s

    def load_xml(self, elm):
        if elm.childNodes:
            self.set(elm.childNodes[0].data)
        Node.load_xml(self, elm)


class XML(object):
    def __init__(self, string):
        object.__init__(self)

        dom = parseString(string)
        self.root = Dir(dom.documentElement.tagName)
        self.root.load_xml(dom.documentElement)
        dom.unlink()

    def get_string(self):
        self.sync_xml()
        return self.root.get_string()

    def sync_xml(self):
        pass


if __name__ == "__main__":
    c = XML("""<xml version="1.0">
                 <dir>
                   <text>blah</text>
                 </dir>
               </xml>""")
    print c.get_string(), '================'
    print c.root.get_child('dir')
