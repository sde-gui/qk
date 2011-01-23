require("munit")
require("medit")
require("gtk")
require("moo.os")

editor = medit.Editor.instance()

tassert(medit.LE_NATIVE ~= nil)
tassert(medit.LE_UNIX ~= nil)
tassert(medit.LE_WIN32 ~= nil)
tassert(moo.os.name == 'nt' or medit.LE_NATIVE ~= medit.LE_WIN32)
tassert(moo.os.name ~= 'nt' or medit.LE_NATIVE == medit.LE_WIN32)

text_unix = 'line1\nline2\nline3\n'
text_win32 = 'line1\r\nline2\r\nline3\r\n'
text_mix = 'line1\nline2\r\nline3\r\n'
if moo.os.name == 'nt' then
  text_native = text_win32
else
  text_native = text_unix
end

function read_file(filename)
  local f = assert(io.open(filename, 'rb'))
  local t = f:read("*all")
  f:close()
  return t
end

function save_file(filename, content)
  local f = assert(io.open(filename, 'wb'))
  f:write(content)
  f:close()
end

function test_default()
  doc = editor.new_doc()
  filename = medit.tempnam()
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  doc.set_text(text_unix)
  tassert(doc.save_as(medit.SaveInfo.new_path(filename)))
  tassert(read_file(filename) == text_native)
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  doc.set_text(text_win32)
  tassert(doc.save_as(medit.SaveInfo.new_path(filename)))
  tassert(read_file(filename) == text_native)
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  doc.set_modified(false)
  editor.close_doc(doc)
end

function test_set()
  doc = editor.new_doc()
  filename = medit.tempnam()

  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  doc.set_text(text_unix)
  tassert(doc.save_as(medit.SaveInfo.new_path(filename)))
  tassert(read_file(filename) == text_native)
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)

  doc.set_line_end_type(medit.LE_UNIX)
  tassert(doc.get_line_end_type() == medit.LE_UNIX)
  tassert(doc.save_as(medit.SaveInfo.new_path(filename)))
  tassert(read_file(filename) == text_unix)
  tassert(doc.get_line_end_type() == medit.LE_UNIX)

  doc.set_line_end_type(medit.LE_WIN32)
  tassert(doc.get_line_end_type() == medit.LE_WIN32)
  tassert(doc.save_as(medit.SaveInfo.new_path(filename)))
  tassert(read_file(filename) == text_win32)
  tassert(doc.get_line_end_type() == medit.LE_WIN32)

  doc.set_modified(false)
  editor.close_doc(doc)
end

function test_load()
  filename = medit.tempnam()

  save_file(filename, text_unix)
  doc = editor.open_file(medit.OpenInfo.new_path(filename))
  tassert(doc.get_filename() == filename)
  tassert(doc.get_line_end_type() == medit.LE_UNIX)
  doc.close()

  save_file(filename, text_win32)
  doc = editor.open_file(medit.OpenInfo.new_path(filename))
  tassert(doc.get_filename() == filename)
  tassert(doc.get_line_end_type() == medit.LE_WIN32)
  doc.close()
end

function test_mix()
  filename = medit.tempnam()

  save_file(filename, text_mix)
  doc = editor.open_file(medit.OpenInfo.new_path(filename))
  tassert(doc.get_filename() == filename)
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)

  tassert(doc.save())
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  tassert(read_file(filename) == text_unix)

  save_file(filename, text_mix)
  tassert(doc.reload())
  tassert(doc.get_line_end_type() == medit.LE_NATIVE)
  doc.set_line_end_type(medit.LE_WIN32)
  tassert(doc.save())
  tassert(doc.get_line_end_type() == medit.LE_WIN32)
  tassert(read_file(filename) == text_win32)

  doc.close()
end

-- test_default()
-- test_set()
-- test_load()
test_mix()
