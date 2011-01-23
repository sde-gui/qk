require("munit")

while #editor.get_windows() > 1 do
  tassert(editor.close_window(editor.get_active_window()))
end

window = editor.get_active_window()
tassert_eq(editor.get_windows(), {window})

while window.get_n_tabs() > 1 do
  tassert(window.get_active_doc().close())
end

if window.get_n_tabs() == 0 then
  tassert(editor.new_doc(window))
end

doc = window.get_active_doc()

tassert_eq(editor.get_windows(), {window})
tassert_eq(editor.get_docs(), {doc})

window2 = editor.new_window()
doc2 = window2.get_active_doc()

tassert_eq(editor.get_windows(), {window, window2})
tassert_eq(editor.get_docs(), {doc, doc2})
tassert_eq(window.get_docs(), {doc})
tassert_eq(window2.get_docs(), {doc2})
