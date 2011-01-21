/*** BEGIN file-header ***/
#ifndef MOO_UTILS_ENUMS_H
#define MOO_UTILS_ENUMS_H

#include <glib-object.h>

G_BEGIN_DECLS

/*** END file-header ***/

/*** BEGIN enumeration-production ***/
/* @EnumName@ */
GType @enum_name@_get_type (void) G_GNUC_CONST;
#define MOO_TYPE_@ENUMSHORT@ (@enum_name@_get_type ())

/*** END enumeration-production ***/

/*** BEGIN file-tail ***/

G_END_DECLS

#endif /* MOO_UTILS_ENUMS_H */
/*** END file-tail ***/
