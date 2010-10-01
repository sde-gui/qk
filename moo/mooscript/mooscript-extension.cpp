#include "mooscript-extension.h"
#include "mooscript.h"
#include "mooscript-classes.h"

using namespace mom;

void mom::emit_signal(Object &obj, const char *name, const VariantArray &args, Accumulator &acc) throw()
{
    try
    {
        moo::Vector<moo::SharedPtr<Callback> > callbacks = obj.list_callbacks(name);

        for (int i = 0, c = callbacks.size(); i < c; ++i)
        {
            Variant ret = callbacks[i]->run(args);
            if (!acc.add_value(ret))
                return;
        }
    }
    catch (...)
    {
        moo_return_if_reached ();
    }
}

extern "C" gboolean
mom_event_editor_save_before (MooEdit *doc, GFile *file, const char *encoding)
{
    char *path = g_file_get_path(file);
    if (!path)
        path = g_file_get_uri(file);
    moo_return_val_if_fail(path != NULL, false);

    try
    {
        Editor &editor = Editor::get_instance();
        if (!editor.has_callbacks("document-save-before"))
            return false;

        VariantArray args;
        args.append(HObject(editor));
        args.append(HObject(Document::wrap(doc)));
        args.append(String(path));
        args.append(String(encoding));
        AccumulatorBool acc(true);
        emit_signal(editor, "document-save-before", args, acc);
        return acc.get_return_value().to_bool();
    }
    catch (...)
    {
        moo_return_val_if_reached(false);
    }
}

extern "C" void
mom_event_editor_save_after (MooEdit *doc)
{
    try
    {
        Editor &editor = Editor::get_instance();
        if (!editor.has_callbacks("document-save-after"))
            return;

        VariantArray args;
        args.append(HObject(Document::wrap(doc)));
        AccumulatorVoid acc;
        emit_signal(editor, "document-save-after", args, acc);
    }
    catch (...)
    {
        moo_return_if_reached();
    }
}
