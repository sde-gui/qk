require("munit")
require("medit")

editor = medit.Editor.instance()

tassert(medit.CLOSE_RESPONSE_CANCEL ~= nil)
tassert(medit.CLOSE_RESPONSE_CONTINUE ~= nil)

function test_will_close_window()
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

function test_before_close_window()
  local seen_editor_before_close_window = 0
  local seen_window_before_close = 0

  cb_id = editor.connect('before-close-window',
    function(editor, window)
      seen_editor_before_close_window = seen_editor_before_close_window + 1
      return medit.CLOSE_RESPONSE_CANCEL
    end)

  window = editor.new_window()

  tassert(not editor.close_window(window))
  tassert(seen_editor_before_close_window == 1)
  tassert(not window.close())
  tassert(seen_editor_before_close_window == 2)

  editor.disconnect(cb_id)

  cb_id = window.connect('before-close',
    function(window)
      seen_window_before_close = seen_window_before_close + 1
      return medit.CLOSE_RESPONSE_CANCEL
    end)

  tassert(not editor.close_window(window))
  tassert(seen_editor_before_close_window == 2)
  tassert(seen_window_before_close == 1)
  tassert(not window.close())
  tassert(seen_editor_before_close_window == 2)
  tassert(seen_window_before_close == 2)

  window.disconnect(cb_id)

  cb_id = editor.connect('before-close-window',
    function(editor, window)
      seen_editor_before_close_window = seen_editor_before_close_window + 1
      return medit.CLOSE_RESPONSE_CONTINUE
    end)

  window.connect('before-close',
    function(window)
      seen_window_before_close = seen_window_before_close + 1
      return medit.CLOSE_RESPONSE_CONTINUE
    end)

  tassert(window.close())
  tassert(seen_editor_before_close_window == 3)
  tassert(seen_window_before_close == 3)

  editor.disconnect(cb_id)
end

function test_bad_callback()
  window = editor.new_window()

  n_callbacks = 0

  cb_id1 = editor.connect('before-close-window', function(editor, window)
    n_callbacks = n_callbacks + 1
    error('lua error')
  end)
  cb_id2 = editor.connect('will-close-window', function(editor, window)
    n_callbacks = n_callbacks + 1
    error('lua error')
  end)
  window.connect('before-close', function(window)
    n_callbacks = n_callbacks + 1
    error('lua error')
  end)
  window.connect('will-close', function(window)
    n_callbacks = n_callbacks + 1
    error('lua error')
  end)
  window.connect('before-close', function(window)
    n_callbacks = n_callbacks + 1
    return nil
  end)
  window.connect('will-close', function(window)
    n_callbacks = n_callbacks + 1
    -- return nothing
  end)

  tassert(cb_id1 ~= nil and cb_id1 ~= 0)
  tassert(cb_id2 ~= nil and cb_id2 ~= 0)

  was_silent = medit.test_set_silent_messages(true)
  tassert(editor.close_window(window))
  medit.test_set_silent_messages(was_silent)

  tassert(n_callbacks == 6)

  editor.disconnect(cb_id1)
  editor.disconnect(cb_id2)
end

test_will_close_window()
test_before_close_window()
test_bad_callback()
