local test_ret = {}

local function getline()
  return debug.getinfo(4, "l").currentline
end

local function _tassert(cond, msg, ...)
  table.insert(test_ret, string.format(msg or '', ...))
  table.insert(test_ret, not not cond)
  table.insert(test_ret, getline())
end

function tassert(cond, msg, ...)
  _tassert(cond, msg, ...)
end

local function cmp(a,b)
  if type(a) == 'table' and type(b) == 'table' then
    local function contains(a, b)
      for k,v in pairs(b) do
        if a[k] ~= v then
          return false
        end
      end
      return true
    end
    return contains(a,b) and contains(b,a)
  else
    return a == b
  end
end

local function pr(obj)
  if type(obj) == 'table' then
    s = '{'
    for k, v in pairs(obj) do
      if #s ~= 1 then
        s = s .. ', '
      end
      s = s .. string.format("%s: %s", pr(k), pr(v))
    end
    s = s .. '}'
    return s
  elseif type(obj) == 'string' then
    return string.format("%q", obj)
  else
    return tostring(obj)
  end
end

function tassert_eq(actual, exp, msg)
  if msg == nil then
    msg = ''
  else
    msg = msg .. ':'
  end
  _tassert(cmp(actual, exp), "%sexpected %s, got %s",
           msg, pr(exp), pr(actual))
end

munit_report = function()
  return unpack(test_ret)
end
