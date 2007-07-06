from mprj.config._xml import XML, XMLGroup, XMLItem
import moo

class Session(object):
    def __init__(self, data=None):
        object.__init__(self)

        self.__file_selector_dir = None
        self.__docs = []
        self.__active = None

        if data:
            if isinstance(data, moo.edit.EditWindow):
                self.__load_from_window(data)
            else:
                file = open(data)
                contents = file.read()
                file.close()
                self.__parse(contents)

    def attach(self, window):
        editor = moo.edit.editor_instance()
        saved_silent = editor.get_property("silent")
        editor.set_property("silent", True)
        for doc in self.__docs:
            editor.open_file(window, None, doc)
        editor.set_property("silent", saved_silent)
        if self.__active is not None:
            doc = window.get_nth_doc(self.__active)
            if doc:
                window.set_active_doc(doc)

    def save(self, filename):
        string = self.__format()
        f = open(filename, "w+")
        f.write(string)
        f.close()

    def get_file_selector_dir(self):
        return self.__file_selector_dir
    def set_file_selector_dir(self, path):
        self.__file_selector_dir = path

    def __format(self):
        root = XMLGroup('medit-session')
        for d in self.__docs:
            root.add_child(XMLItem('doc', d))
        if self.__active is not None:
            root.add_child(XMLItem('active', str(self.__active)))
        if self.__file_selector_dir is not None:
            root.add_child(XMLItem('file-selector', self.__file_selector_dir))
        return '<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n' + \
                root.get_string()

    def __parse(self, string):
        xml = XML(string)

        if xml.root.name != 'medit-session':
            raise RuntimeError("Invalid root element name '%s'" % (xml.root.name,))

        for node in xml.root.children():
            if node.name == 'doc':
                self.__parse_doc(node)
            elif node.name == 'active':
                self.__active = int(node.get())
            elif node.name == 'file-selector':
                self.__file_selector_dir = node.get()
            else:
                raise RuntimeError("Uknown element '%s'" % (node.name,))

    def __parse_doc(self, node):
        self.__docs.append(node.get())

    def __load_from_window(self, window):
        docs = window.list_docs()
        active = window.get_active_doc()
        for d in docs:
            if d.get_filename():
                self.__docs.append(d.get_filename())
            if d is active:
                self.__active = len(self.__docs) - 1
