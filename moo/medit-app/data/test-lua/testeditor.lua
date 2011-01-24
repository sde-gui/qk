require("munit")

window = nil

function prepare()
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
end

function test1()
  doc = window.get_active_doc()

  tassert_eq(editor.get_windows(), {window})
  tassert_eq(editor.get_docs(), {doc})

  window2 = editor.new_window()
  doc2 = window2.get_active_doc()

  tassert_eq(editor.get_windows(), {window, window2})
  tassert_eq(editor.get_docs(), {doc, doc2})
  tassert_eq(window.get_docs(), {doc})
  tassert_eq(window2.get_docs(), {doc2})
end

function save_file(filename, content)
  local f = assert(io.open(filename, 'wb'))
  f:write(content)
  f:close()
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

function test2()
  window = editor.new_window()
  filename1 = moo.tempnam()
  filename2 = moo.tempnam()
  filename3 = moo.tempnam()
  save_file(filename1, "file1")
  save_file(filename2, "file2")
  save_file(filename3, "file3")

  t = {moo.OpenInfo.new(filename1), moo.OpenInfo.new(filename2), moo.OpenInfo.new(filename3)}
  editor.open_files(t, window)

  tassert(window.get_n_tabs() == 3)
  tassert(#window.get_docs() == 3)
  docs = window.get_docs()
  tassert(docs[1].get_filename() == filename1)
  tassert(docs[2].get_filename() == filename2)
  tassert(docs[3].get_filename() == filename3)
  tassert(window.close())
end

prepare()
test1()
test2()
