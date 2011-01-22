require("munit")
require("medit")

editor = medit.Editor.instance()

function test_close_window()
  local seen_editor_will_close_window = 0
  local seen_window_will_close = 0

  cb_id = editor.connect('will-close-window',
    function(editor, window)
      seen_editor_will_close_window = seen_editor_will_close_window + 1
    end)

  window = editor.new_window()

  window.connect('will-close',
    function(window)
      seen_window_will_close = seen_window_will_close + 1
    end)

  tassert(editor.close_window(window))

  window = editor.new_window()
  tassert(window.close())

  tassert(seen_editor_will_close_window == 2)
  tassert(seen_window_will_close == 1)

  editor.disconnect(cb_id)

  window = editor.new_window()
  tassert(editor.close_window(window))
  window = editor.new_window()
  tassert(window.close())

  tassert(seen_editor_will_close_window == 2)
  tassert(seen_window_will_close == 1)
end

test_close_window()
