local _g = getfenv(0)
local medit = require("medit")
local global = medit.get_global_obj()
_g._MG = global
_g.editor = global.editor
_g.doc = global.active_document
_g.view = global.active_view
_g.window = global.active_window
_g.app = global.application
setfenv(0, _g)
