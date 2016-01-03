#pragma once

#include <gtk/gtk.h>
#include <mooutils/mooarray.h>
#include <mooutils/moolist.h>
#include <mooutils/mootype-macros.h>
#include <mooedit/mooedit-enums.h>

G_BEGIN_DECLS

typedef struct MooOpenInfo MooOpenInfo;
typedef struct MooSaveInfo MooSaveInfo;
typedef struct MooReloadInfo MooReloadInfo;

typedef struct MooEdit MooEdit;
typedef struct MooEditView MooEditView;
typedef struct MooEditWindow MooEditWindow;
typedef struct MooEditor MooEditor;
typedef struct MooEditTab MooEditTab;

MOO_DECLARE_OBJECT_ARRAY (MooEdit, moo_edit)
MOO_DECLARE_OBJECT_ARRAY (MooEditView, moo_edit_view)
MOO_DECLARE_OBJECT_ARRAY (MooEditTab, moo_edit_tab)
MOO_DECLARE_OBJECT_ARRAY (MooEditWindow, moo_edit_window)
MOO_DEFINE_SLIST (MooEditList, moo_edit_list, MooEdit)

MOO_DECLARE_OBJECT_ARRAY (MooOpenInfo, moo_open_info)

#define MOO_TYPE_LINE_END (moo_type_line_end ())
GType   moo_type_line_end   (void) G_GNUC_CONST;

#define MOO_EDIT_RELOAD_ERROR (moo_edit_reload_error_quark ())
#define MOO_EDIT_SAVE_ERROR (moo_edit_save_error_quark ())

MOO_DECLARE_QUARK (moo-edit-reload-error, moo_edit_reload_error_quark)
MOO_DECLARE_QUARK (moo-edit-save-error, moo_edit_save_error_quark)

#define MOO_TYPE_EDIT_VIEW  (moo_edit_view_get_type ())
GType moo_edit_view_get_type(void) G_GNUC_CONST;

G_END_DECLS

#ifdef __cplusplus

#include <moocpp/gobjptrtypes.h>

using MooEditPtr =      moo::gobjptr<MooEdit>;
using MooEditViewPtr =  moo::gobjptr<MooEditView>;
using MooEditTabPtr =   moo::gobjptr<MooEditTab>;
using MooGFilePtr =     moo::gobjptr<GFile>;

namespace moo 
{
MOO_DEFINE_GOBJ_TYPE(MooEditView, GtkTextView, MOO_TYPE_EDIT_VIEW)
}

#endif // __cplusplus
