#pragma once

#include "mooedit/mooeditview-impl.h"
#include "mooedit/mooedit-impl.h"
#include "mooedit/mooedittypes.h"

struct MooEditViewPrivate
{
    EditRawPtr doc;
    MooEditor *editor;
    MooEditTab *tab;
    GtkTextMark *fake_cursor_mark;
};
