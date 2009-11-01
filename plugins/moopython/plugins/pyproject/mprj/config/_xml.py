#
#  mprj/config/_xml.py
#
#  Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

__all__ = ['BadFileError', 'File']

import cgi
import xml.dom
from xml.dom.minidom import parseString


class BadFileError(Exception):
    pass

class File(object):
    def __init__(self, string, path=None):
        self.xml = XML(string)
        self.root = self.xml.root

        if self.root.name != 'medit-project':
            raise BadFileError('Invalid root element "%s"' % (self.root.name,))

        self.path = path
        self.version = self.root.get_attr('version')
        self.name = self.root.get_attr('name')
        self.project_type = self.root.get_attr('type')

        if not self.version:
            raise BadFileError('Version missing')
        if not self.name:
            raise BadFileError('Name missing')
        if not self.project_type:
            raise BadFileError('Project type missing')


_INDENT_STRING = " "

class XMLNode(object):
    def __init__(self, name):
        if not name:
            raise ValueError()
        self.name = name
        self.parent = None
        self.__attrs = {}
    def has_attr(self, attr):
        return self.__attrs.has_key(attr)
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

    def _attributes_equal(self, other):
        return self.name == other.name and \
               self.__attrs == other.__attrs

class XMLGroup(XMLNode):
    def __init__(self, name, children=[], sort=True):
        XMLNode.__init__(self, name)
        self.__children = []
        self.__children_names = {}

        if children:
            if sort:
                def cmp_nodes(n1, n2):
                    return cmp(n1.name, n2.name)
                children = [c for c in children]
                children.sort(cmp_nodes)
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
        if not isinstance(child, XMLNode):
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
            return XMLGroup
        if not elm.childNodes:
            return XMLItem
        c = elm.childNodes[0]
        if c.nodeType == xml.dom.Node.ELEMENT_NODE:
            return XMLGroup
        elif c.nodeType == xml.dom.Node.TEXT_NODE:
            return XMLItem

    def load_xml(self, elm):
        XMLNode.load_xml(self, elm)
        for c in elm.childNodes:
            if c.nodeType == xml.dom.Node.ELEMENT_NODE:
                t = self.get_xml_elm_type(c)
                if t is XMLGroup:
                    child = XMLGroup(c.tagName)
                    self.add_child(child)
                    child.load_xml(c)
                elif t is XMLItem:
                    child = XMLItem(c.tagName)
                    self.add_child(child)
                    child.load_xml(c)

    def get_string(self, indent=0):
        s = _INDENT_STRING * indent + self.format_start() + ">\n"
        for child in self.children():
            s += child.get_string(indent+1)
        s += _INDENT_STRING * indent + "</%s>\n" % (self.name,)
        return s

    def __eq__(self, other):
        if type(other) != XMLGroup:
            return False
        if not self._attributes_equal(other):
            return False
        return self.children() == other.children()
    def __ne__(self, other):
        return not self.__eq__(other)

class XMLItem(XMLNode):
    def __init__(self, name, content=None):
        XMLNode.__init__(self, name)
        self.set(content)

    def get(self):
        return self.content

    def set(self, content):
        self.content = content

    def get_string(self, indent=0):
        s = _INDENT_STRING * indent + self.format_start()
        if not self.get():
            s += "/>\n"
        else:
            s += ">%s</%s>\n" % (cgi.escape(str(self.get())), self.name)
        return s

    def load_xml(self, elm):
        if elm.childNodes:
            self.set(elm.childNodes[0].data)
        else:
            self.set('')
        XMLNode.load_xml(self, elm)

    def __eq__(self, other):
        if type(other) != XMLItem:
            return False
        return self._attributes_equal(other) and \
               self.content == other.content
    def __ne__(self, other):
        return not self.__eq__(other)


class XML(object):
    def __init__(self, string):
        object.__init__(self)

        dom = parseString(string)
        self.root = XMLGroup(dom.documentElement.tagName)
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
