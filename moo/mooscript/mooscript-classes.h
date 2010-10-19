#ifndef MOO_SCRIPT_CLASSES_GENERATED_H
#define MOO_SCRIPT_CLASSES_GENERATED_H

#include "mooscript-classes-base.h"

namespace mom {

class DocumentView;
class Application;
class Document;
class Editor;
class DocumentWindow;

class DocumentView : public _GObjectWrapper<DocumentView, MooEdit>
{
    MOM_GOBJECT_DECL(DocumentView, MooEdit)

public:

    /* methods */
    void set_show_line_numbers(bool show);
    void set_line_wrap_mode(bool enabled);
    bool overwrite_mode();
    void set_overwrite_mode(bool enabled);
    DocumentWindow *window();
    bool line_wrap_mode();
    bool show_line_numbers();
    Document *document();

    /* methods */
    Variant _set_show_line_numbers(const ArgArray &args);
    Variant _set_line_wrap_mode(const ArgArray &args);
    Variant _overwrite_mode(const ArgArray &args);
    Variant _set_overwrite_mode(const ArgArray &args);
    Variant _window(const ArgArray &args);
    Variant _line_wrap_mode(const ArgArray &args);
    Variant _show_line_numbers(const ArgArray &args);
    Variant _document(const ArgArray &args);
};

class Application : public _Singleton<Application>
{
    MOM_SINGLETON_DECL(Application)

public:

    /* methods */
    Document *active_document();
    void quit();
    DocumentWindow *active_window();
    void set_active_window(DocumentWindow &window);
    VariantArray windows();
    Editor *editor();
    DocumentView *active_view();

    /* methods */
    Variant _active_document(const ArgArray &args);
    Variant _quit(const ArgArray &args);
    Variant _active_window(const ArgArray &args);
    Variant _set_active_window(const ArgArray &args);
    Variant _windows(const ArgArray &args);
    Variant _editor(const ArgArray &args);
    Variant _active_view(const ArgArray &args);
};

class Document : public _GObjectWrapper<Document, MooEdit>
{
    MOM_GOBJECT_DECL(Document, MooEdit)

public:

    /* methods */
    String line_endings();
    void set_encoding(const String &encoding);
    String encoding();
    void set_line_endings(const String &value);
    String text(const ArgArray &args);
    String basename();
    void end_not_undoable_action();
    void set_selection(const ArgArray &args);
    VariantArray selection();
    void replace_selected_lines(const Variant &repl);
    void redo();
    bool save_as(const String &filename);
    void cut();
    void select_lines_at_pos(const ArgArray &args);
    void select_all();
    void select_text(const ArgArray &args);
    gint64 line_count();
    bool can_redo();
    gint64 line_at_pos(gint64 pos);
    String filename();
    void append_text(const String &text);
    void replace_selected_text(const String &text);
    gint64 start_pos();
    gint64 end_pos();
    gint64 selection_bound();
    gint64 pos_at_line_end(gint64 line);
    void paste();
    void replace_text(gint64 start, gint64 end, const String &text);
    VariantArray views();
    bool has_selection();
    VariantArray selected_lines();
    void undo();
    void select_lines(const ArgArray &args);
    void delete_selected_lines();
    gint64 pos_at_line(gint64 line);
    DocumentView *active_view();
    gint64 cursor_pos();
    void copy();
    String char_at_pos(gint64 pos);
    String selected_text();
    void begin_not_undoable_action();
    bool save();
    void insert_text(const ArgArray &args);
    gint64 char_count();
    String uri();
    void reload();
    void delete_text(gint64 start, gint64 end);
    bool can_undo();
    void set_cursor_pos(gint64 pos);
    void delete_selected_text();
    void clear();

    /* methods */
    Variant _line_endings(const ArgArray &args);
    Variant _set_encoding(const ArgArray &args);
    Variant _encoding(const ArgArray &args);
    Variant _set_line_endings(const ArgArray &args);
    Variant _text(const ArgArray &args);
    Variant _basename(const ArgArray &args);
    Variant _end_not_undoable_action(const ArgArray &args);
    Variant _set_selection(const ArgArray &args);
    Variant _selection(const ArgArray &args);
    Variant _replace_selected_lines(const ArgArray &args);
    Variant _redo(const ArgArray &args);
    Variant _save_as(const ArgArray &args);
    Variant _cut(const ArgArray &args);
    Variant _select_lines_at_pos(const ArgArray &args);
    Variant _select_all(const ArgArray &args);
    Variant _select_text(const ArgArray &args);
    Variant _line_count(const ArgArray &args);
    Variant _can_redo(const ArgArray &args);
    Variant _line_at_pos(const ArgArray &args);
    Variant _filename(const ArgArray &args);
    Variant _append_text(const ArgArray &args);
    Variant _replace_selected_text(const ArgArray &args);
    Variant _start_pos(const ArgArray &args);
    Variant _end_pos(const ArgArray &args);
    Variant _selection_bound(const ArgArray &args);
    Variant _pos_at_line_end(const ArgArray &args);
    Variant _paste(const ArgArray &args);
    Variant _replace_text(const ArgArray &args);
    Variant _views(const ArgArray &args);
    Variant _has_selection(const ArgArray &args);
    Variant _selected_lines(const ArgArray &args);
    Variant _undo(const ArgArray &args);
    Variant _select_lines(const ArgArray &args);
    Variant _delete_selected_lines(const ArgArray &args);
    Variant _pos_at_line(const ArgArray &args);
    Variant _active_view(const ArgArray &args);
    Variant _cursor_pos(const ArgArray &args);
    Variant _copy(const ArgArray &args);
    Variant _char_at_pos(const ArgArray &args);
    Variant _selected_text(const ArgArray &args);
    Variant _begin_not_undoable_action(const ArgArray &args);
    Variant _save(const ArgArray &args);
    Variant _insert_text(const ArgArray &args);
    Variant _char_count(const ArgArray &args);
    Variant _uri(const ArgArray &args);
    Variant _reload(const ArgArray &args);
    Variant _delete_text(const ArgArray &args);
    Variant _can_undo(const ArgArray &args);
    Variant _set_cursor_pos(const ArgArray &args);
    Variant _delete_selected_text(const ArgArray &args);
    Variant _clear(const ArgArray &args);
};

class Editor : public _Singleton<Editor>
{
    MOM_SINGLETON_DECL(Editor)

public:

    /* methods */
    Document *get_document_by_uri(const String &uri);
    Document *active_document();
    Document *get_document_by_path(const String &path);
    VariantArray documents();
    VariantArray windows();
    VariantArray views();
    void set_active_document(Document &doc);
    Document *new_file(const String &file, const String &encoding, DocumentWindow *window);
    void set_active_view(DocumentView &view);
    void set_active_window(DocumentWindow &window);
    DocumentWindow *active_window();
    DocumentView *active_view();
    void reload(Document &doc);
    void open_uris(const VariantArray &uris, DocumentWindow *window);
    Document *open_file(const String &file, const String &encoding, DocumentWindow *window);
    bool close(Document &doc);
    void open_files(const VariantArray &files, DocumentWindow *window);
    bool save(Document &doc);
    Document *open_uri(const String &file, const String &encoding, DocumentWindow *window);
    bool save_as(Document &doc, const String &filename);

    /* methods */
    Variant _get_document_by_uri(const ArgArray &args);
    Variant _active_document(const ArgArray &args);
    Variant _get_document_by_path(const ArgArray &args);
    Variant _documents(const ArgArray &args);
    Variant _windows(const ArgArray &args);
    Variant _views(const ArgArray &args);
    Variant _set_active_document(const ArgArray &args);
    Variant _new_file(const ArgArray &args);
    Variant _set_active_view(const ArgArray &args);
    Variant _set_active_window(const ArgArray &args);
    Variant _active_window(const ArgArray &args);
    Variant _active_view(const ArgArray &args);
    Variant _reload(const ArgArray &args);
    Variant _open_uris(const ArgArray &args);
    Variant _open_file(const ArgArray &args);
    Variant _close(const ArgArray &args);
    Variant _open_files(const ArgArray &args);
    Variant _save(const ArgArray &args);
    Variant _open_uri(const ArgArray &args);
    Variant _save_as(const ArgArray &args);
};

class DocumentWindow : public _GObjectWrapper<DocumentWindow, MooEditWindow>
{
    MOM_GOBJECT_DECL(DocumentWindow, MooEditWindow)

public:

    /* methods */
    Document *active_document();
    VariantArray documents();
    VariantArray views();
    void set_active_document(Document &doc);
    bool is_active();
    void set_active();
    void set_active_view(DocumentView &view);
    Editor *editor();
    DocumentView *active_view();

    /* methods */
    Variant _active_document(const ArgArray &args);
    Variant _documents(const ArgArray &args);
    Variant _views(const ArgArray &args);
    Variant _set_active_document(const ArgArray &args);
    Variant _is_active(const ArgArray &args);
    Variant _set_active(const ArgArray &args);
    Variant _set_active_view(const ArgArray &args);
    Variant _editor(const ArgArray &args);
    Variant _active_view(const ArgArray &args);
};


} // namespace mom

#endif /* MOO_SCRIPT_CLASSES_GENERATED_H */
