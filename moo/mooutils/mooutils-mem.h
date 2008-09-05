/*
 *   mooutils-mem.h
 *
 *   Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOO_UTILS_MEM_H
#define MOO_UTILS_MEM_H

#include <glib.h>
#include <string.h>
#include <stdlib.h>


#define _MOO_COPYELMS(func_,dest_,src_,n_)      \
G_STMT_START {                                  \
    size_t n__ = n_;                            \
    if ((dest_) == (src_))                      \
        (void) 0;                               \
    func_ ((dest_), (src_),                     \
           n__ * sizeof *(dest_));              \
} G_STMT_END

#define MOO_ELMCPY(dest_,src_,n_) _MOO_COPYELMS (memcpy, dest_, src_, n_)
#define MOO_ELMMOVE(dest_,src_,n_) _MOO_COPYELMS (memmove, dest_, src_, n_)

#define MOO_ARRAY_GROW(Type_,mem_,new_size_)                \
G_STMT_START {                                              \
    size_t ns__ = new_size_;                                \
    (mem_) = g_renew (Type_, (mem_), ns__);                 \
} G_STMT_END


#define MOO_IP_ARRAY_ELMS(ElmType,name_)                            \
    ElmType *name_;                                                 \
    size_t n_##name_;                                               \
    size_t n_##name_##_allocd__

#define MOO_IP_ARRAY_INIT(c_,name_,len_)                            \
G_STMT_START {                                                      \
    (c_)->name_ = g_malloc0 (len_ * sizeof *(c_)->name_);           \
    (c_)->n_##name_ = len_;                                         \
    (c_)->n_##name_##_allocd__ = len_;                              \
} G_STMT_END

#define MOO_IP_ARRAY_DESTROY(c_,name_)                              \
G_STMT_START {                                                      \
    g_free ((c_)->name_);                                           \
    (c_)->name_ = NULL;                                             \
} G_STMT_END

#define MOO_IP_ARRAY_GROW(c_,name_,howmuch_)                        \
G_STMT_START {                                                      \
    if ((c_)->n_##name_ + howmuch_ > (c_)->n_##name_##_allocd__)    \
    {                                                               \
        gsize old_size__ = (c_)->n_##name_##_allocd__;              \
        gsize new_size__ = MAX(old_size__ * 1.2,                    \
                               (c_)->n_##name_ + howmuch_);         \
        (c_)->name_ = g_realloc ((c_)->name_,                       \
                                 new_size__ * sizeof *(c_)->name_); \
        (c_)->n_##name_##_allocd__ = new_size__;                    \
    }                                                               \
                                                                    \
    memset ((c_)->name_ + (c_)->n_##name_, 0,                       \
            howmuch_ * sizeof *(c_)->name_);                        \
    (c_)->n_##name_ += howmuch_;                                    \
} G_STMT_END


#endif /* MOO_UTILS_MEM_H */
