local test_ret = {}

tassert = function(cond, msg, ...)
  if msg == nil then msg = ''; end
  table.insert(test_ret, string.format(msg, ...))
  table.insert(test_ret, not not cond)
end

munit_report = function()
  return unpack(test_ret)
end
