#include "mooedithistoryitem.h"
#include <stdlib.h>

#define KEY_ENCODING "encoding"
#define KEY_LINE "line"

void
_moo_edit_history_item_set_encoding (MdHistoryItem *item,
                                     const char    *encoding)
{
    g_return_if_fail (item != NULL);
    md_history_item_set (item, KEY_ENCODING, encoding);
}

void
_moo_edit_history_item_set_line (MdHistoryItem *item,
                                 int            line)
{
    char *value = NULL;

    g_return_if_fail (item != NULL);

    if (line >= 0)
        value = g_strdup_printf ("%d", line + 1);

    md_history_item_set (item, KEY_LINE, value);
    g_free (value);
}

const char *
_moo_edit_history_item_get_encoding (MdHistoryItem *item)
{
    g_return_val_if_fail (item != NULL, NULL);
    return md_history_item_get (item, KEY_ENCODING);
}

int
_moo_edit_history_item_get_line (MdHistoryItem *item)
{
    const char *strval;

    g_return_val_if_fail (item != NULL, -1);

    strval = md_history_item_get (item, KEY_LINE);

    if (strval && strval[0])
        return strtol (strval, NULL, 10) - 1;
    else
        return -1;
}
