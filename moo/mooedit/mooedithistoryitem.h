#ifndef MOO_EDIT_HISTORY_ITEM_H
#define MOO_EDIT_HISTORY_ITEM_H

#include <mooutils/mdhistorymgr.h>

G_BEGIN_DECLS

void         _moo_edit_history_item_set_encoding    (MdHistoryItem  *item,
                                                     const char     *encoding);
void         _moo_edit_history_item_set_line        (MdHistoryItem  *item,
                                                     int             line);
const char  *_moo_edit_history_item_get_encoding    (MdHistoryItem  *item);
int          _moo_edit_history_item_get_line        (MdHistoryItem  *item);

G_END_DECLS

#endif /* MOO_EDIT_HISTORY_ITEM_H */
