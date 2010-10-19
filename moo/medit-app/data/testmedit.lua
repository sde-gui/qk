-- -%- indent-width:2 -%- --

require("munit")
require("medit")
require("_moo_utils")

app = medit.get_app_obj()
editor = app.editor()

local function test_active_window()
  w1 = editor.active_window()
  tassert(w1 ~= nil, 'editor.active_window() ~= nil')
  tassert(w1 == w1, 'w1 == w1')
  tassert(#app.windows() == 1, 'one window')
  w2 = editor.new_window()
  tassert(w2 ~= nil, 'editor.new_window() ~= nil')
  tassert(w1 ~= w2, 'old window != new window')
  tassert(#app.windows() == 2, 'two window')
  app.set_active_window(w2)
  _moo_utils.spin_main_loop(0.1)
  tassert(w2 == editor.active_window(), 'w2 == editor.active_window()')
  app.set_active_window(w1)
  _moo_utils.spin_main_loop(0.1)
  tassert(w1 == editor.active_window(), 'w1 == editor.active_window()')
  editor.close_window(w1)
  tassert(#app.windows() == 1, 'two window')
  tassert(w2 == editor.active_window(), 'w2 == editor.active_window()')
end

test_active_window()

-- print(#editor.views())
