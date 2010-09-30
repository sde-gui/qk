#ifndef MOO_SCRIPT_EXTENSION_H
#define MOO_SCRIPT_EXTENSION_H

#include <mooedit/mooeditor.h>

#ifdef __cplusplus

#include "mooscript-api.h"

namespace mom {

class Accumulator
{
public:
    virtual ~Accumulator() {}
    virtual bool add_value(const Variant &val) = 0;
    virtual Variant get_return_value() = 0;
};

class AccumulatorVoid : public Accumulator
{
public:
    bool add_value(const Variant &)
    {
        return true;
    }

    Variant get_return_value()
    {
        return Variant();
    }
};

class AccumulatorBool : public Accumulator
{
public:
    AccumulatorBool(bool stop_value = false)
        : m_retval(!stop_value)
        , m_stop(stop_value)
    {
    }

    bool add_value(const Variant &val)
    {
        if (m_stop == val.to_bool())
        {
            m_retval = m_stop;
            return false;
        }

        return true;
    }

    Variant get_return_value()
    {
        return m_retval;
    }

private:
    bool m_retval;
    bool m_stop;
};

void emit_signal(Object &obj, const char *name, const VariantArray &args, Accumulator &acc) throw();

} // namespace mom

#endif // __cplusplus

G_BEGIN_DECLS

gboolean mom_event_editor_save_before (MooEdit *doc, GFile *file, const char *encoding);
void mom_event_editor_save_after (MooEdit *doc);

G_END_DECLS

#endif // MOO_SCRIPT_EXTENSION_H
