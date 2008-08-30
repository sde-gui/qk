#! /usr/bin/env python

import sys
import getopt
import xml.dom
import xml.dom.minidom as minidom
import StringIO

def name_is_nice(name):
    return name[-1:] not in "0123456789"

class Widget(object):
    def __init__(self, node, name, class_name):
        object.__init__(self)

        self.node = node
        self.name = name
        self.class_name = class_name
        self.real_name = node.getAttribute('id')

        if self.name in ['new', 'delete']:
            self.name += '_'

class GladeXml(object):
    def __init__(self, root, buffer, params):
        object.__init__(self)

        assert root is not None
        self.root = root
        self.buffer = buffer

        self.rootName = root.getAttribute("id")
        if ':' in self.rootName:
            self.rootName, real_class_name = self.rootName.split(':')

        self.widgets = []
        def check_node(node):
            if node.tagName != "widget":
                return None
            name = node.getAttribute("id")
            class_name = node.getAttribute("class")
            if ':' in name:
                real_name, real_class_name = name.split(':')
                name = real_name
                class_name = real_class_name
            if name_is_nice(name):
                self.widgets.append(Widget(node, name, class_name))
            return None
        walk_node(root, False, check_node)

    def format_buffer(self):
        out = StringIO.StringIO()
        for l in self.buffer.splitlines():
            out.write('"')
            out.write(l.replace('\\', '\\\\').replace('"', '\\"'))
            out.write('"\n')
        ret = out.getvalue()
        out.close()
        return ret

class ConvertParams(object):
    def __init__(self):
        object.__init__(self)

        self.root = None
        self.id_map = {}
        self.struct_name = None
        self.StructName = None

    def map_id(self, id, ClassName, CLASS_TYPE = None):
        self.id_map[id] = (ClassName, CLASS_TYPE)

def walk_node(node, walk_siblings, func, *args):
    while node is not None:
        if node.nodeType == xml.dom.Node.ELEMENT_NODE:
            ret = func(node, *args)
            if ret is not None:
                return ret
            ret = walk_node(node.firstChild, True, func, *args)
            if ret is not None:
                return ret
        if not walk_siblings:
            break
        node = node.nextSibling
    return None

def find_root(dom, root_name):
    def check_node(node, name):
        if node.tagName != "widget":
            return None
        if name is None and name_is_nice(node.getAttribute("id")):
            return node
        if node.getAttribute("id") == name:
            return node
    return walk_node(dom.documentElement.firstChild, True,
                     check_node, root_name)

def write_file(gxml, params, out):
    tmpl = """\
#include <mooutils/mooglade.h>
#include <mooutils/mooi18n.h>
#include <gtk/gtk.h>

const char %(glade_xml)s[] =
%(glade_xml_content)s
;

typedef struct %(XmlStruct)s %(XmlStruct)s;

struct %(XmlStruct)s {
    MooGladeXML *xml;
%(glade_xml_widgets_decl)s
};

static void
_%(xml_struct)s_free (%(XmlStruct)s *xml)
{
    if (xml)
    {
        g_object_unref (xml->xml);
        g_free (xml);
    }
}

G_GNUC_UNUSED static %(XmlStruct)s *
%(xml_struct)s_get (gpointer widget)
{
    return g_object_get_data (widget, "moo-generated-glade-xml");
}

static void
_%(xml_struct)s_finish_build (%(XmlStruct)s *xml)
{
%(glade_xml_widgets_defs)s
    g_object_set_data_full (G_OBJECT (xml->%(root)s), "moo-generated-glade-xml",
                            xml, (GDestroyNotify) _%(xml_struct)s_free);
}

static void
%(xml_struct)s_build (%(XmlStruct)s *xml)
{
    moo_glade_xml_parse_memory (xml->xml, %(glade_xml)s, -1, "%(root)s", NULL);
    _%(xml_struct)s_finish_build (xml);
}

static void
%(xml_struct)s_fill (%(XmlStruct)s *xml, GtkWidget *root)
{
    moo_glade_xml_fill_widget (xml->xml, root, %(glade_xml)s, -1, "%(root)s", NULL);
    _%(xml_struct)s_finish_build (xml);
}

static %(XmlStruct)s *
%(xml_struct)s_new_empty (void)
{
    %(XmlStruct)s *xml = g_new0 (%(XmlStruct)s, 1);
    xml->xml = moo_glade_xml_new_empty (GETTEXT_PACKAGE);
%(glade_xml_widgets_map)s
    return xml;
}

G_GNUC_UNUSED static %(XmlStruct)s *
%(xml_struct)s_new (void)
{
    %(XmlStruct)s *xml = %(xml_struct)s_new_empty ();
    %(xml_struct)s_build (xml);
    return xml;
}

G_GNUC_UNUSED static %(XmlStruct)s *
%(xml_struct)s_new_with_root (GtkWidget *root)
{
    %(XmlStruct)s *xml = %(xml_struct)s_new_empty ();
    %(xml_struct)s_fill (xml, root);
    return xml;
}
"""

    buf = StringIO.StringIO()
    if gxml.widgets:
        print >> buf, ''
        for w in gxml.widgets:
            name = w.name
            ct = params.id_map.get(name)
            if ct is None:
                class_name = w.class_name
            else:
                class_name = ct[0]
            print >> buf, '    %s *%s;' % (class_name, name)
    glade_xml_widgets_decl = buf.getvalue()
    buf.close()

    buf = StringIO.StringIO()
    for w in gxml.widgets:
        print >> buf, '    xml->%s = moo_glade_xml_get_widget (xml->xml, "%s");' % (w.name, w.real_name)
    glade_xml_widgets_defs = buf.getvalue()
    buf.close()

    buf = StringIO.StringIO()
    for id in params.id_map:
        ct = params.id_map.get(id)
        if ct[1]:
            type_name = ct[1]
        else:
            type_name = 'g_type_from_name ("%s")' % (ct[0],)
        print >> buf, '    moo_glade_xml_map_id (xml->xml, "%s", %s);' % (id, type_name)
    glade_xml_widgets_map = buf.getvalue()
    buf.close()

    dic = {
        'glade_xml': '_' + params.struct_name + '_glade_xml',
        'glade_xml_content': gxml.format_buffer(),
        'root': gxml.rootName,
        'XmlStruct': params.StructName,
        'xml_struct': params.struct_name,
        'glade_xml_widgets_decl': glade_xml_widgets_decl,
        'glade_xml_widgets_defs': glade_xml_widgets_defs,
        'glade_xml_widgets_map': glade_xml_widgets_map,
    }

    out.write(tmpl % dic)

def convert_buffer(buf, params, output):
    dom = minidom.parseString(buf)
    root = find_root(dom, params.root)
    assert root is not None
    params.root = root.getAttribute('id')
    gxml = GladeXml(root, buf, params)

    if not params.StructName:
        params.StructName = gxml.rootName + 'Xml'
    if not params.struct_name:
        def S2s(name):
            ret = ''
            for i in range(len(name)):
                c = name[i]
                if i == 0:
                    ret += c.lower()
                elif c.istitle() and not name[i-1].istitle():
                    ret += '_' + c.lower()
                else:
                    ret += c
            return ret
        params.struct_name = S2s(params.StructName)

    close_output = False
    if output is None:
        output = sys.stdout
    elif isinstance(output, str) or isinstance(output, unicode):
        output = open(output, 'w')
        close_output = True
    write_file(gxml, params, output)
    if close_output:
        output.close()

def convert_file(filename, params, output):
    f = open(filename)
    ret = convert_buffer(f.read(), params, output)
    f.close()
    return ret

def usage():
    print "Usage: %s [--map=id,ClassName,CLASS_TYPE...] [--output=outfile] FILE" % (sys.argv[0],)

def main(args):
    params = ConvertParams()

    try:
        opts, files = getopt.getopt(args[1:], "hm:o:s:S:r:",
                                    ["help", "map=", "output=", "struct-name=", "StructName=", "root="])
    except getopt.GetoptError, err:
        print str(err)
        usage()
        return 2

    output = None

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            return 0
        elif o in ("-m", "--map"):
            t = a.split(',')
            if len(t) == 3:
                params.map_id(t[0], t[1], t[2])
            elif len(t) == 2:
                params.map_id(t[0], t[1])
            else:
                usage()
                return 2
        elif o in ("-o", "--output"):
            output = a
        elif o in ("-s", "--struct-name"):
            params.struct_name = a
        elif o in ("-S", "--StructName"):
            params.StructName = a
        elif o in ("-r", "--root"):
            params.root = a

    if len(files) != 1:
        usage()
        return 2

    convert_file(files[0], params, output)
    return 0

if __name__ == '__main__':
#     args = ['glade2c', '-o', '/tmp/test.h', '-r', 'LuaPage',
#             '-m', 'textview,MooTextView',
#             '/tmp/test.glade']
#     sys.exit(main(args))
    sys.exit(main(sys.argv))
