local _g = getfenv(0)
_g.doc = _g.app.active_document()
_g.view = _g.app.active_view()
_g.window = _g.app.active_window()
