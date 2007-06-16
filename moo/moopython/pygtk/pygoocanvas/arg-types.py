from argtypes import ArgType, matcher
import reversewrapper

class CairoMatrixArg(ArgType):

    before = ('    %(name)s = &((PycairoMatrix*)(py_%(name)s))->matrix;\n')

    def write_param(self, ptype, pname, pdflt, pnull, info):
        info.varlist.add('PyObject', '*py_' + pname)
        info.varlist.add('cairo_matrix_t', '*'+pname)
        info.add_parselist('O', ['&py_'+pname], [pname])
        info.arglist.append(pname)
        info.codebefore.append (self.before % { 'name' : pname, 'namecopy' : 'NULL' })


    def write_return(self, ptype, ownsreturn, info):
        info.varlist.add('cairo_matrix_t', '*ret')
        info.codeafter.append('    if (ret)\n'
                              '        return PycairoMatrix_FromMatrix(ret);\n'
                              '    else {\n'
                              '        Py_INCREF(Py_None);\n'
                              '        return Py_None;\n'
                              '    }');

matcher.register('cairo_matrix_t*', CairoMatrixArg())


class CairoParam(reversewrapper.Parameter):
    def get_c_type(self):
        return self.props.get('c_type').replace('const-', 'const ')
    def convert_c2py(self):
        self.wrapper.add_declaration("PyObject *py_%s;" % self.name)
        self.wrapper.write_code(
            code=('py_%s = PycairoContext_FromContext(cairo_reference(%s), NULL, NULL);' %
                  (self.name, self.name)),
            cleanup=("Py_DECREF(py_%s);" % self.name))
        self.wrapper.add_pyargv_item("py_%s" % self.name)

matcher.register_reverse("cairo_t*", CairoParam)

class BoundsPtrArg(ArgType):

    def write_param(self, ptype, pname, pdflt, pnull, info):
        if pdflt:
            info.varlist.add('PyObject', '*py_' + pname + " = " + pdflt)
        else:
            info.varlist.add('PyObject', '*py_' + pname)
        if pnull:
            info.add_parselist('O', ['&py_'+pname], [pname])
            info.codebefore.append(
                '    if (!(py_%(name)s == NULL || py_%(name)s == Py_None || \n'
                '        PyObject_IsInstance(py_%(name)s, (PyObject *) &PyGooCanvasBounds_Type))) {\n'
                '        PyErr_SetString(PyExc_TypeError, "parameter %(name)s must be goocanvas.Bounds or None");\n'
                '        return NULL;\n'
                '    }\n' % dict(name=pname))
            info.arglist.append("(py_%s == NULL || py_%s == Py_None)? NULL :"
                                " &((PyGooCanvasBounds *) py_%s)->bounds" % (pname, pname, pname))
        else:
            info.add_parselist('O!', ['&PyGooCanvasBounds_Type', '&py_'+pname], [pname])
            info.arglist.append("(py_%s == NULL)? NULL :"
                                " &((PyGooCanvasBounds *) py_%s)->bounds" % (pname, pname))

    def write_return(self, ptype, ownsreturn, info):
        info.varlist.add('GooCanvasBounds', '*ret')
        info.codeafter.append('   return pygoo_canvas_bounds_new(ret);\n');

matcher.register('GooCanvasBounds*', BoundsPtrArg())
matcher.register('const-GooCanvasBounds*', BoundsPtrArg())

class GooCanvasBoundPtrReturn(reversewrapper.ReturnType):
    def get_c_type(self):
        return self.props.get('c_type')
    def write_decl(self):
        self.wrapper.add_declaration("%s retval;" % self.get_c_type())
        self.wrapper.add_declaration("PyObject *py_bounds;")
    def write_error_return(self):
        self.wrapper.write_code("return NULL;")
    def write_conversion(self):
        self.wrapper.add_pyret_parse_item("O!", "&PyGooCanvasBounds_Type, &py_bounds", prepend=True)
        self.wrapper.write_code((
            " /* FIXME: this leaks memory */\n"
            "retval = g_new(GooCanvasBounds, 1);\n"
            "*retval = ((PyGooCanvasBounds*) py_bounds)->bounds;"),
                                code_sink=self.wrapper.post_return_code)

matcher.register_reverse_ret("GooCanvasBounds*", GooCanvasBoundPtrReturn)

class GooCanvasBoundPtrParam(reversewrapper.Parameter):
    def get_c_type(self):
        return self.props.get('c_type').replace('const-', 'const ')
    def convert_c2py(self):
        self.wrapper.add_declaration("PyObject *py_%s;" % self.name)
        self.wrapper.write_code(
            code=('py_%s = pygoo_canvas_bounds_new(%s);' %
                  (self.name, self.name)),
            cleanup=("Py_DECREF(py_%s);" % self.name))
        self.wrapper.add_pyargv_item("py_%s" % self.name)

matcher.register_reverse("GooCanvasBounds*", GooCanvasBoundPtrParam)
matcher.register_reverse("const-GooCanvasBounds*", GooCanvasBoundPtrParam)

class GObjectReturn(reversewrapper.GObjectReturn):
    def write_conversion(self):
        self.wrapper.write_code(
            code=None,
            failure_expression="py_retval == Py_None")
        reversewrapper.GObjectReturn.write_conversion(self)

matcher.register_reverse_ret('GObject*', GObjectReturn)
