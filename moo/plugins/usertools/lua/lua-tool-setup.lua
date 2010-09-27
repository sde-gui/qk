local _g = getfenv(0)

require("moo.builtin")._inject(_g)

local medit = require("medit")
_g.app = medit.get_app_obj()
_g.editor = _g.app.editor()
_g.doc = _g.app.active_document()
_g.view = _g.app.active_view()
_g.window = _g.app.active_window()
