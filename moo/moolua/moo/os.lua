-- -%- indent-width:2 -%- --

local _G = _G
local math, table, package = math, table, package
local pairs, ipairs, select, unpack, require =
      pairs, ipairs, select, unpack, require

module('moo.os')

if package.config:sub(1,1) == '/' then
  name = 'posix'
else
  name = 'nt'
end
