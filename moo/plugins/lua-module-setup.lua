local _g = getfenv(0)

require("moo.builtin")._inject(_g)

local medit = require("medit")
_g.app = medit.get_app_obj()
_g.editor = _g.app.editor()

local __medit_callbacks = {}

local __medit_connect = medit.connect
function medit.connect(obj, sig, callback)
  local id = __medit_connect(obj, sig)
  __medit_callbacks[id] = callback
end

function __medit_invoke_callback(id, ...)
  local callback = __medit_callbacks[id]
  return callback(...)
end
