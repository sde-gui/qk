import sys
import xml.etree.ElementTree as etree

def walk(elm):
    if elm.tag in ('module', 'class', 'boxed', 'pointer', 'enum', 'flags'):
        if elm.get('moo.private') != '1':
            for child in elm.getchildren():
                walk(child)
    elif elm.tag in ('method', 'function', 'constructor'):
        if elm.get('moo.private') != '1':
            print elm.get('c_name')

for f in sys.argv[1:]:
    xml = etree.parse(f)
    walk(xml.getroot())
