#ifndef MOM_SCRIPT_CLASSES_H
#define MOM_SCRIPT_CLASSES_H

#include "mooscript.h"
#include <mooedit/mooeditwindow.h>

namespace mom {

#define SINGLETON_CLASS(Name)  class Name : public _Singleton<Name>
#define GOBJECT_CLASS(Name, GName) class Name : public _GObjectWrapper<Name, GName>

#define PROPERTY(name,kind)                 \
    Variant get_##name();                   \
    void set_##name(const Variant &value)

#define METHOD(name)                        \
    Variant name(const VariantArray &args)

template<typename T>
class _Singleton : public Object
{
public:
    static T &get_instance()
    {
        if (!s_instance)
            s_instance = new T;
        return *s_instance;
    }

    static void cleanup() NOTHROW
    {
        if (s_instance)
            s_instance->remove();
        moo_assert(s_instance == 0);
    }

protected:
    _Singleton(MetaObject &meta)
        : Object(meta)
    {
        mooThrowIfFalse(s_instance == 0);
        s_instance = static_cast<T*>(this);
    }

    ~_Singleton() NOTHROW
    {
        mooThrowIfFalse(s_instance == 0 || s_instance == this);
        s_instance = 0;
    }

private:
    static T *s_instance;
};

#define MOM_SINGLETON_DECL(Class)       \
private:                                \
    MOM_OBJECT_DECL(Class)              \
    friend class _Singleton<Class>;     \
    Class()                             \
        : _Singleton<Class>(s_meta)     \
    {                                   \
    }

#define MOM_SINGLETON_DEFN(Class)       \
    MOM_OBJECT_DEFN(Class)              \
    template<> Class *_Singleton<Class>::s_instance = 0;

///
/// @node Application object
/// @section Application object
/// @helpsection{SCRIPT_APPLICATION}
/// @table @method
///
SINGLETON_CLASS(Application)
{
public:
    /// @item Application.editor()
    /// returns Editor object.
    METHOD(editor);
    /// @item Application.active_view()
    /// returns current active document view or @null{} if no documents are open.
    METHOD(active_view);
    /// @item Application.active_document()
    /// returns current active document or @null{} if no documents are open.
    METHOD(active_document);

    /// @item Application.active_window()
    /// returns current active window.
    METHOD(active_window);
    /// @item Application.set_active_window(window)
    /// activates @param{window}.
    METHOD(set_active_window);
    /// @item Application.windows()
    /// returns a list of all editor windows.
    METHOD(windows);

    /// @item Application.quit()
    /// quit @medit{}.
    METHOD(quit);

private:
    MOM_SINGLETON_DECL(Application)
};
///
/// @end table
///

///
/// @node Editor object
/// @section Editor object
/// @helpsection{SCRIPT_EDITOR}
/// @table @method
///
SINGLETON_CLASS(Editor)
{
public:
    /// @item Editor.active_document()
    /// returns current active document or @null{} if there are no open documents
    METHOD(active_document);
    /// @item Editor.set_active_document(doc)
    /// makes @param{doc} active
    METHOD(set_active_document);
    /// @item Editor.active_window()
    /// returns current active window
    METHOD(active_window);
    /// @item Editor.set_active_window(window)
    /// makes @param{window} active
    METHOD(set_active_window);
    /// @item Editor.active_view()
    /// returns current active document view
    METHOD(active_view);
    /// @item Editor.set_active_view(view)
    /// makes @param{view} active
    METHOD(set_active_view);

    /// @item Editor.documents()
    /// returns list of all open documents
    METHOD(documents);
    /// @item Editor.documents()
    /// returns list of all open document views
    METHOD(views);
    /// @item Editor.documents()
    /// returns list of all document windows
    METHOD(windows);

    /// @item Editor.get_document_by_path(path)
    /// returns document with path @param{path} or @null{}.
    METHOD(get_document_by_path);
    /// @item Editor.get_document_by_uri(path)
    /// returns document with uri @param{uri} or @null{}.
    METHOD(get_document_by_uri);

    /// @item Editor.new_file(file, encoding=null, window=null)
    /// open file if it exists on disk or create a new one. If @param{encoding} is
    /// @null{} or "auto" then pick character encoding automatically, otherwise use
    /// @param{encoding}. If @param{window} is given then open file in that window,
    /// otherwise in an existing window.
    METHOD(new_file);
    /// @item Editor.open_files(files, window=null)
    /// open files. If @param{window} is given then open files in that window,
    /// otherwise in an existing window.
    METHOD(open_files);
    /// @item Editor.open_files(uris, window=null)
    /// open files. If @param{window} is given then open files in that window,
    /// otherwise in an existing window.
    METHOD(open_uris);
    /// @item Editor.open_file(file, encoding=null, window=null)
    /// open file. If @param{encoding} is @null{} or "auto" then pick character
    /// encoding automatically, otherwise use @param{encoding}. If @param{window}
    /// is given then open files in that window, otherwise in an existing window.
    METHOD(open_file);
    /// @item Editor.open_uri(uri, encoding=null, window=null)
    /// open file. If @param{encoding} is @null{} or "auto" then pick character
    /// encoding automatically, otherwise use @param{encoding}. If @param{window}
    /// is given then open files in that window, otherwise in an existing window.
    METHOD(open_uri);
    /// @item Editor.reload(doc)
    /// reload document.
    METHOD(reload);
    /// @item Editor.save(doc)
    /// save document.
    METHOD(save);
    /// @item Editor.save_as(doc, filename=null)
    /// save document as @param{filename}. If @param{filename} is not given then
    /// first ask user for new filename.
    METHOD(save_as);
    /// @item Editor.close(doc)
    /// close document.
    METHOD(close);

private:
    MOM_SINGLETON_DECL(Editor)
};
///
/// @end table
///

template<typename T, typename GT>
class _GObjectWrapper : public Object
{
public:
    GT *gobj() const throw() { return m_gobj; }

    static T &wrap(GT *gobj)
    {
        mooThrowIfFalse(G_IS_OBJECT(gobj));
        void *pwrapper = g_object_get_qdata(G_OBJECT(gobj), wrapper_quark());
        if (pwrapper)
            return *static_cast<T*>(pwrapper);
        else
            return *new T(gobj);
    }

protected:
    static GQuark wrapper_quark() G_GNUC_CONST
    {
        static GQuark q;
        if (G_UNLIKELY(!q))
            q = g_quark_from_string(T::class_name() + "-mom-script-wrapper");
        return q;
    }

    _GObjectWrapper(GT *gobj, MetaObject &meta)
        : Object(meta)
        , m_gobj(gobj)
    {
        mooThrowIfFalse(G_IS_OBJECT(gobj));
        g_object_set_qdata(G_OBJECT(gobj), wrapper_quark(), this);
        g_object_weak_ref(G_OBJECT(gobj), (GWeakNotify) gobject_weak_notify, this);
    }

    ~_GObjectWrapper()
    {
        if (m_gobj)
        {
            g_object_weak_unref(G_OBJECT(m_gobj), (GWeakNotify) gobject_weak_notify, this);
            g_object_set_qdata(G_OBJECT(m_gobj), wrapper_quark(), 0);
        }
    }

private:
    static void gobject_weak_notify(_GObjectWrapper *wrapper)
    {
        wrapper->m_gobj = 0;
    }

private:
    MOO_DISABLE_COPY_AND_ASSIGN(_GObjectWrapper)

private:
    GT *m_gobj;
};

#define MOM_GOBJECT_DECL(Class, GClass)                 \
protected:                                              \
    friend class _GObjectWrapper<Class, GClass>;        \
    Class(GClass *gobj) :                               \
        _GObjectWrapper<Class, GClass>(gobj, s_meta)    \
    {                                                   \
    }                                                   \
    MOM_OBJECT_DECL(Class)

#define MOM_GOBJECT_DEFN(Class)                         \
    MOM_OBJECT_DEFN(Class)

///
/// @node DocumentWindow object
/// @section DocumentWindow object
/// @helpsection{SCRIPT_DOCUMENT_WINDOW}
/// @table @method
///
GOBJECT_CLASS(DocumentWindow, MooEditWindow)
{
public:
    /// @item DocumentWindow.editor()
    /// returns Editor object.
    METHOD(editor);
    /// @item DocumentWindow.active_view()
    /// returns current active document view in this window.
    METHOD(active_view);
    /// @item DocumentWindow.set_active_view(view)
    /// makes @param{view} active, i.e. switches to its tab.
    METHOD(set_active_view);
    /// @item DocumentWindow.active_document()
    /// returns current active document in this window, that is the document
    /// whose view is the active one.
    METHOD(active_document);
    /// @item DocumentWindow.set_active_document(doc)
    /// makes active a view of document @param{doc}. It picks arbitrary view
    /// of @param{doc} if there are more than one in this window.
    METHOD(set_active_document);
    /// @item DocumentWindow.views()
    /// returns list of all document views in this window.
    METHOD(views);
    /// @item DocumentWindow.documents()
    /// returns list of all documents in this window.
    METHOD(documents);

    /// @item DocumentWindow.is_active()
    /// returns whether this window is the active one.
    METHOD(is_active);
    /// @item DocumentWindow.set_active()
    /// makes this window active.
    METHOD(set_active);

private:
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)
};
///
/// @end table
///

///
/// @node DocumentView object
/// @section DocumentView object
/// @helpsection{DOCUMENT_VIEW}
/// @table @method
///
GOBJECT_CLASS(DocumentView, MooEdit)
{
public:
    /// @item DocumentView.document()
    /// returns document to which this view belongs.
    METHOD(document);
    /// @item DocumentView.window()
    /// returns window which contains this view.
    METHOD(window);

    /// @item DocumentView.line_wrap_mode()
    /// returns whether line wrapping is enabled.
    METHOD(line_wrap_mode);
    /// @item DocumentView.set_line_wrap_mode(enabled)
    /// enables or disables line wrapping.
    METHOD(set_line_wrap_mode);
    /// @item DocumentView.overwrite_mode()
    /// returns whether overwrite mode is on.
    METHOD(overwrite_mode);
    /// @item DocumentView.set_overwrite_mode(enabled)
    /// enables or disables overwrite mode.
    METHOD(set_overwrite_mode);
    /// @item DocumentView.show_line_numbers()
    /// returns whether line numbers are displayed.
    METHOD(show_line_numbers);
    /// @item DocumentView.set_show_line_numbers(show)
    /// shows or hides line numbers.
    METHOD(set_show_line_numbers);

private:
    MOM_GOBJECT_DECL(DocumentView, MooEdit)
};
///
/// @end table
///

///
/// @node Document object
/// @section Document object
/// @helpsection{DOCUMENT}
///
GOBJECT_CLASS(Document, MooEdit)
{
public:
    /// @table @method

    /// @item Document.views()
    /// returns list of views which display this document.
    METHOD(views);
    /// @item Document.active_view()
    /// returns active view of this document. If the document has a single
    /// view, then that is returned; otherwise if the current active view
    /// belongs to this document, then that view is returned; otherwise
    /// a random view is picked.
    METHOD(active_view);

    /// @item Document.filename()
    /// returns full file path of the document or @null{} if the document
    /// has not been saved yet or if it can't be represented with a local
    /// path (e.g. if it is in a remote location like a web site).
    /// @itemize @minus
    /// @item Untitled => @null{}
    /// @item @file{/home/user/example.txt} => @code{"/home/user/example.txt"}
    /// @item @file{http://example.com/index.html} => @null{}
    /// @end itemize
    METHOD(filename);
    /// @item Document.uri()
    /// returns URI of the document or @null{} if the document has not been
    /// saved yet.
    METHOD(uri);
    /// @item Document.basename()
    /// returns basename of the document, that is the full path minus directory
    /// part. If the document has not been saved yet, then it returns the name
    /// shown in the titlebar, e.g. "Untitled".
    METHOD(basename);

    /// @item Document.encoding()
    /// returns character encoding of the document.
    METHOD(encoding);
    /// @item Document.set_encoding(encoding)
    /// set character encoding of the document, it will be used when the document
    /// is saved.
    METHOD(set_encoding);

    METHOD(line_endings);
    METHOD(set_line_endings);

    /// @item Document.reload()
    /// reload the document.
    METHOD(reload);
    /// @item Document.save()
    /// save the document.
    METHOD(save);
    /// @item Document.save_as(filename=null)
    /// save the document as @param{filename}. If @param{filename} is @null{} then
    /// @uilabel{Save As} will be shown to choose new filename.
    METHOD(save_as);

    /// @item Document.can_undo()
    /// returns whether undo action is available.
    METHOD(can_undo);
    /// @item Document.can_redo()
    /// returns whether redo action is available.
    METHOD(can_redo);
    /// @item Document.undo()
    /// undo.
    METHOD(undo);
    /// @item Document.redo()
    /// redo.
    METHOD(redo);
    /// @item Document.begin_not_undoable_action()
    /// mark the beginning of a non-undoable operation. Undo stack will be erased
    /// and undo will not be recorded until @method{end_not_undoable_action()} call.
    METHOD(begin_not_undoable_action);
    /// @item Document.end_not_undoable_action()
    /// end the non-undoable operation started with @method{begin_not_undoable_action()}.
    METHOD(end_not_undoable_action);

    /// @end table

    /// @table @method

    /// @item Document.start_pos()
    /// position at the beginning of the document (0 in Python, 1 in Lua, etc.)
    METHOD(start_pos);
    /// @item Document.end_pos()
    /// position at the end of the document. This is the position past the last
    /// character: it points to no character, but it is a valid position for
    /// text insertion, cursor may be put there, etc.
    METHOD(end_pos);
    /// @item Document.cursor_pos()
    /// position at the cursor.
    METHOD(cursor_pos);
    /// @item Document.set_cursor_pos(pos)
    /// move cursor to position @param{pos}.
    METHOD(set_cursor_pos);
    /// @item Document.selection()
    /// returns selection bounds as a list of two items, start and end. Returned
    /// list is always sorted, use @method{cursor()} and @method{selection_bound()}
    /// if you need to distinguish beginning and end of selection. If no text is
    /// is selected, then it returns pair @code{[cursor, cursor]}.
    METHOD(selection);
    /// @item Document.set_selection(bounds_as_list)
    /// @item Document.set_selection(start, end)
    /// select text.
    METHOD(set_selection);
    /// @item Document.selection_bound()
    /// returns the selection bound other than cursor position. Selection is
    /// either [cursor, selection_bound) or [selection_bound, cursor), depending
    /// on direction user dragged the mouse (or on @method{set_selection}
    /// arguments).
    METHOD(selection_bound);
    /// @item Document.has_selection()
    /// whether any text is selected.
    METHOD(has_selection);

    /// @item Document.char_count()
    /// character count.
    METHOD(char_count);
    /// @item Document.line_count()
    /// line count.
    METHOD(line_count);

    /// @item Document.line_at_pos(pos)
    /// returns index of the line which contains position @param{pos}.
    METHOD(line_at_pos);
    /// @item Document.pos_at_line(line)
    /// returns position at the beginning of line @param{line}.
    METHOD(pos_at_line);
    /// @item Document.pos_at_line(line)
    /// returns position at the end of line @param{line}.
    METHOD(pos_at_line_end);
    /// @item Document.char_at_pos(pos)
    /// returns character at position @param{pos} as string.
    METHOD(char_at_pos);
    /// @item Document.text()
    /// returns whole document contents.
    /// @item Document.text(start, end)
    /// returns text in the range [@param{start}, @param{end}), @param{end} not
    /// included. Example: @code{doc.text(doc.start_pos(), doc.end_pos())} is
    /// equivalent @code{to doc.text()}.
    METHOD(text);

    /// @item Document.insert_text(text)
    /// @item Document.insert_text(pos, text)
    /// insert text into the document. If @param{pos} is not given, insert at
    /// cursor position.
    METHOD(insert_text);
    /// @item Document.replace_text(start, end, text)
    /// replace text in the region [@param{start}, @param{end}). Equivalent to
    /// @code{delete_text(start, end), insert_text(start, text)}.
    METHOD(replace_text);
    /// @item Document.delete_text(start, end)
    /// delete text in the region [@param{start}, @param{end}). Example:
    /// @code{doc.delete_text(doc.start(), doc.end())} will delete all text in
    /// @code{doc}.
    METHOD(delete_text);
    /// @item Document.append_text(text)
    /// append text. Equivalent to @code{doc.insert_text(doc.end(), text)}.
    METHOD(append_text);
    /// @item Document.clear()
    /// delete all text in the document.
    METHOD(clear);

    /// @item Document.copy()
    /// copy selected text to clipboard. If no text is selected then nothing
    /// will happen, same as Ctrl-C key combination.
    METHOD(copy);
    /// @item Document.cut()
    /// cut selected text to clipboard. If no text is selected then nothing
    /// will happen, same as Ctrl-X key combination.
    METHOD(cut);
    /// @item Document.paste()
    /// paste text from clipboard. It has the same effect as Ctrl-V key combination:
    /// nothing happens if clipboard is empty, and selected text is replaced with
    /// clipboard contents otherwise.
    METHOD(paste);

    /// @item Document.select_text(bounds_as_list)
    /// @item Document.select_text(start, end)
    /// select text, same as @method{set_selection()}.
    METHOD(select_text);
    /// @item Document.select_lines(line)
    /// select a line.
    /// @item Document.select_lines(first, last)
    /// select lines from @param{first} to @param{last}, @emph{including}
    /// @param{last}.
    METHOD(select_lines);
    /// @item Document.select_lines_at_pos(bounds_as_list)
    /// @item Document.select_lines_at_pos(start, end)
    /// select lines: similar to @method{select_text}, but select whole lines.
    METHOD(select_lines_at_pos);
    /// @item Document.select_all()
    /// select all.
    METHOD(select_all);

    /// @item Document.selected_text()
    /// returns selected text.
    METHOD(selected_text);
    /// @item Document.selected_lines()
    /// returns selected lines as a list of strings, one string for each line,
    /// line terminator characters not included. If nothing is selected, then
    /// line at cursor is returned.
    METHOD(selected_lines);
    /// @item Document.delete_selected_text()
    /// delete selected text, equivalent to @code{doc.delete_text(doc.cursor(),
    /// doc.selection_bound())}.
    METHOD(delete_selected_text);
    /// @item Document.delete_selected_lines()
    /// delete selected lines. Similar to @method{delete_selected_text()} but
    /// selection is extended to include whole lines. If nothing is selected,
    /// then line at cursor is deleted.
    METHOD(delete_selected_lines);
    /// @item Document.replace_selected_text(text)
    /// replace selected text with @param{text}. If nothing is selected,
    /// @param{text} is inserted at cursor.
    METHOD(replace_selected_text);
    /// @item Document.replace_selected_lines(text)
    /// replace selected lines with @param{text}. Similar to
    /// @method{replace_selected_text()}, but selection is extended to include
    /// whole lines. If nothing is selected, then line at cursor is replaced.
    METHOD(replace_selected_lines);

    /// @end table

public:
    GtkTextBuffer *buffer() { return gtk_text_view_get_buffer(GTK_TEXT_VIEW(gobj())); }

private:
    MOM_GOBJECT_DECL(Document, MooEdit)
};
///
///

#undef PROPERTY
#undef METHOD

} // namespace mom

#endif // MOM_SCRIPT_CLASSES_H
