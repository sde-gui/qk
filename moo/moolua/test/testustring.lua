require("ustring")

local _test_ret = {}
local tassert = function(cond, msg, ...)
  table.insert(_test_ret, string.format(msg, ...))
  table.insert(_test_ret, not not cond)
end


local strings = {
  { "", "", "", 0, 0 },
  { "a", "A", "a", 1, 1 },
  { "ABC", "ABC", "abc", 3, 3 },
  { "AbC", "ABC", "abc", 3, 3 },
}

local non_ascii_strings = {
  { "\208\176\208\177\209\134", "\208\144\208\145\208\166", "\208\176\208\177\209\134", 6, 3 },
  { "\049\050\051\208\176\208\177\209\134", "\049\050\051\208\144\208\145\208\166", "\049\050\051\208\176\208\177\209\134", 9, 6 },
  { "\208\156\208\176\208\188\208\176", "\208\156\208\144\208\156\208\144", "\208\188\208\176\208\188\208\176", 8, 4 },
  { "\208\159\208\176\208\191\208\176", "\208\159\208\144\208\159\208\144", "\208\191\208\176\208\191\208\176", 8, 4 },
  { "Мама", "МАМА", "мама", 8, 4 },
}

local test_ustring_one = function(case, ascii)
  s = case[1]
  upper = case[2]
  lower = case[3]
  byte_len = case[4]
  char_len = case[5]

  us = ustring.new(s)

  tassert(us ~= s, 'ustring.new(...) ~= "..."')
  tassert(tostring(us) == s, 'tostring(ustring.new(%q)) == %q', s, s)

  tassert(us:upper() == ustring.new(upper), 'ustring.new(%q):upper() == ustring.new(%q)', s, upper)
  tassert(tostring(us:upper()) == upper, 'tostring(ustring.new(%q):upper()) == %q', s, upper)
  tassert(ustring.upper(s) == ustring.upper(us), 'ustring.upper(%q) == ustring.upper(ustring.new(%q))', s, s)

  tassert(us:lower() == ustring.new(lower), 'ustring.new(%q):lower() == ustring.new(%q)', s, lower)
  tassert(tostring(us:lower()) == lower, 'tostring(ustring.new(%q):lower()) == %q', s, lower)
  tassert(ustring.lower(s) == ustring.lower(us), 'ustring.lower(%q) == ustring.lower(ustring.new(%q))', s, s)

  tassert(us:len() == char_len, 'ustring.new(%q):len() == %d', s, char_len)
  tassert(us:bytelen() == byte_len, 'ustring.new(%q):bytelen() == %d', s, byte_len)
  tassert(ustring.len(s) == us:len(), 'ustring.len(%q) == ustring.new(%q):len()', s, s)
  tassert(#s == us:bytelen(), '#%q == ustring.new(%q):bytelen()', s, s)

  if ascii then
    tassert(tostring(ustring.upper(s)) == s:upper(), 'tostring(ustring.upper(%q)) == s:upper(%q)', s, s)
    tassert(tostring(ustring.lower(s)) == s:lower(), 'tostring(ustring.lower(%q)) == s:lower(%q)', s, s)

    tassert(#s == ustring.len(s), '#%q == ustring.len(%q)', s, s)
    tassert(#s == us:len(), '#%q == ustring.new(%q):len()', s, s)
  end
end

for index, case in pairs(strings) do
  test_ustring_one(case, true)
end

for index, case in pairs(non_ascii_strings) do
  test_ustring_one(case, false)
end


return unpack(_test_ret)
