#
#  mprj/config/_utils.py
#
#  Copyright (C) 2004-2010 by Yevgen Muntyan <emuntyan@sourceforge.net>
#
#  This file is part of medit.  medit is free software; you can
#  redistribute it and/or modify it under the terms of the
#  GNU Lesser General Public License as published by the
#  Free Software Foundation; either version 2.1 of the License,
#  or (at your option) any later version.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with medit.  If not, see <http://www.gnu.org/licenses/>.
#

def dict_diff(dic1, dic2):
    first, common, second = {}, {}, {}
    for k in dic1:
        if dic2.has_key(k):
            common[k] = k
        else:
            first[k] = k
    for k in dic2:
        if not common.has_key(k):
            second[k] = k
    return first, common, second
