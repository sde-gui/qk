#ifndef MOO_EDIT_TYPES_H
#define MOO_EDIT_TYPES_H

#include <gtk/gtk.h>
#include <mooutils/mooarray.h>
#include <mooutils/moolist.h>
#include <mooutils/mootype-macros.h>

G_BEGIN_DECLS

typedef struct MooEditOpenInfo MooEditOpenInfo;
typedef struct MooEditSaveInfo MooEditSaveInfo;
typedef struct MooEditReloadInfo MooEditReloadInfo;

typedef struct MooEdit MooEdit;
typedef struct MooEditWindow MooEditWindow;
typedef struct MooEditor MooEditor;

MOO_DECLARE_OBJECT_ARRAY (MooEdit, moo_edit)
MOO_DECLARE_OBJECT_ARRAY (MooEditWindow, moo_edit_window)
MOO_DEFINE_SLIST (MooEditList, moo_edit_list, MooEdit)

MOO_DECLARE_OBJECT_ARRAY (MooEditOpenInfo, moo_edit_open_info)

#define MOO_TYPE_LINE_END (moo_type_line_end ())
GType   moo_type_line_end   (void) G_GNUC_CONST;

#define MOO_EDIT_RELOAD_ERROR (moo_edit_reload_error_quark ())
#define MOO_EDIT_SAVE_ERROR (moo_edit_save_error_quark ())

MOO_DECLARE_QUARK (moo-edit-reload-error, moo_edit_reload_error_quark)
MOO_DECLARE_QUARK (moo-edit-save-error, moo_edit_save_error_quark)

G_END_DECLS

#endif /* MOO_EDIT_TYPES_H */
