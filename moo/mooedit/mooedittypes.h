#ifndef MOO_EDIT_TYPES_H
#define MOO_EDIT_TYPES_H

#include <gtk/gtk.h>
#include <mooutils/mooarray.h>
#include <mooutils/moolist.h>

G_BEGIN_DECLS

typedef struct _MooFileEnc MooFileEnc;

typedef struct _MooEdit MooEdit;
typedef struct _MooEditView MooEditView;
typedef struct _MooEditBuffer MooEditBuffer;
typedef struct _MooEditWindow MooEditWindow;
typedef struct _MooEditor MooEditor;

MOO_DECLARE_OBJECT_ARRAY (MooEditArray, moo_edit_array, MooEdit)
MOO_DECLARE_OBJECT_ARRAY (MooEditViewArray, moo_edit_view_array, MooEditView)
MOO_DECLARE_OBJECT_ARRAY (MooEditWindowArray, moo_edit_window_array, MooEditWindow)

MOO_DECLARE_PTR_ARRAY (MooFileEncArray, moo_file_enc_array, MooFileEnc)

#define MOO_TYPE_LINE_END (moo_type_line_end ())
GType   moo_type_line_end   (void) G_GNUC_CONST;

G_END_DECLS

#endif /* MOO_EDIT_TYPES_H */
