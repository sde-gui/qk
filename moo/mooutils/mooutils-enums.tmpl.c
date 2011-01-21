/*** BEGIN file-header ***/
#include "mooutils/mooutils-enums.h"

/*** END file-header ***/

/*** BEGIN file-production ***/
#include "@filename@"

/*** END file-production ***/

/*** BEGIN enumeration-production ***/
/* @EnumName@ */
/*** END enumeration-production ***/

/*** BEGIN value-header ***/
GType
@enum_name@_get_type (void)
{
    static GType etype;

    if (G_UNLIKELY (!etype))
    {
        static const G@Type@Value values[] = {
/*** END value-header ***/

/*** BEGIN value-production ***/
            { @VALUENAME@, (char*) "@VALUENAME@", (char*) "@valuenick@" },
/*** END value-production ***/

/*** BEGIN value-tail ***/
            { 0, NULL, NULL }
        };

        etype = g_@type@_register_static ("@EnumName@", values);
    }

    return etype;
}

/*** END value-tail ***/
