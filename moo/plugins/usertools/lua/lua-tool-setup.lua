local _g = getfenv(0)
_g.doc = _g.editor.active_document()
_g.view = _g.editor.active_view()
_g.window = _g.editor.active_window()
