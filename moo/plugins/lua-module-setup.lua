local medit = require("medit")

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
