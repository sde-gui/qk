-- -%- indent-width:2 -%- --

local _G = _G
local table = table
local pairs, ipairs, select, unpack, require =
      pairs, ipairs, select, unpack, require

module('moo._string')

function startswith(s, prfx)
  return s:sub(1, #prfx) == prfx
end

function endswith(s, sfx)
  return s:sub(-#sfx) == sfx
end
