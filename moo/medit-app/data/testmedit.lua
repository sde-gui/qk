-- -%- indent-width:2 -%- --

require("munit")
require("medit")
require("_moo_utils")

app = medit.get_app_obj()
editor = app.editor()

local __docs_to_cleanup = {}
local function add_doc_to_cleanup(doc)
  table.insert(__docs_to_cleanup, doc)
end
local function cleanup()
  for i, doc in pairs(__docs_to_cleanup) do
    doc.set_modified(false)
  end
end

local function test_active_window()
  w1 = editor.active_window()
  tassert(w1 ~= nil, 'editor.active_window() ~= nil')
  tassert(w1 == w1, 'w1 == w1')
  tassert(#editor.windows() == 1, 'one window')
  w2 = editor.new_window()
  tassert(w2 ~= nil, 'editor.new_window() ~= nil')
  tassert(w1 ~= w2, 'old window != new window')
  tassert(#editor.windows() == 2, 'two window')
  editor.set_active_window(w2)
  _moo_utils.spin_main_loop(0.1)
  tassert(w2 == editor.active_window(), 'w2 == editor.active_window()')
  editor.set_active_window(w1)
  _moo_utils.spin_main_loop(0.1)
  tassert(w1 == editor.active_window(), 'w1 == editor.active_window()')
  editor.close_window(w1)
  tassert(#editor.windows() == 1, 'two window')
  tassert(w2 == editor.active_window(), 'w2 == editor.active_window()')
end

local function test_selection()
  doc = editor.new_document()
  add_doc_to_cleanup(doc)
  tassert(doc.text() == '')
  tassert(doc.start_pos() == 1)
  tassert(doc.end_pos() == 1)
  tassert(doc.text(1, 1) == '')
  doc.insert_text('a')
  doc.insert_text(1, 'b')
  tassert(doc.text() == 'ba')
  tassert(doc.start_pos() == 1)
  tassert(doc.end_pos() == 3)
  tassert(doc.text(1, 2) == 'b')
  text = [[abcdefg
abcdefghij
1234567890
]]
  doc.replace_text(doc.start_pos(), doc.end_pos(), text)
  tassert(doc.start_pos() == 1)
  tassert(doc.end_pos() == #text + 1)
  doc.select_text(2, 3)
  tassert(doc.selected_text() == 'b')
  doc.select_text({3, 4})
  tassert(doc.selected_text() == 'c')
  doc.select_lines(1)
  tassert(doc.selected_text() == 'abcdefg\n')
end

-- test_active_window()
test_selection()
cleanup()
