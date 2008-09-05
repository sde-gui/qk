#
#  mprj/config/_setting.py
#
#  Copyright (C) 2004-2008 by Yevgen Muntyan <muntyan@tamu.edu>
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

__all__ = ['Setting']

from mprj.config._item import Item, create_instance
from mprj.config._xml import XMLItem


class Setting(Item):
    __class_attributes__ = {
        '__item_default__' : 'default',
        '__item_editable__' : 'editable',
        '__item_data_type__' : 'data_type',
        '__item_null_ok__' : 'null_ok',
    }

    def __init__(self, id, value=None, **kwargs):
        Item.__init__(self, id, **kwargs)

        default = None
        editable = True
        data_type = None
        null_ok = False

        attrs = getattr(type(self), '__item_attributes__')

        if attrs.has_key('default'):
            default = attrs['default']

        if attrs.has_key('editable'):
            editable = attrs['editable']

        if attrs.has_key('data_type'):
            data_type = attrs['data_type']

        if attrs.has_key('null_ok'):
            null_ok = attrs['null_ok']
        elif default is None:
            null_ok = True

        self.__default = default
        self.__value = default
        self.__editable = editable
        self.__data_type = data_type
        self.__null_ok = null_ok

        if value is None:
            self.reset()
        else:
            self.set_value(value)

    def reset(self):
        self.set_value(self.__default)

    def set_string(self, value):
        if value is not None:
            data_type = self.get_data_type()
            if data_type is None:
                raise NotImplementedError()
            else:
                value = data_type(value)
        return self.set_value(value)

    def check_value(self, value):
        try:
            self.transform_value(value)
        except Exception:
            return False

    def transform_value(self, value):
        if value is None and self.__null_ok:
            return None
        data_type = self.get_data_type()
        if data_type is not None:
            if not isinstance(value, data_type):
                raise TypeError('value %s is invalid for %s' % (value, self))
            else:
                return value
        else:
            return value

    def set_value(self, value):
        value = self.transform_value(value)
        if not self.equal(value):
            self.__value = value
            return True
        else:
            return False

    def __eq__(self, other):
        if isinstance(other, Setting):
            return self.equal(other.get_value())
        else:
            return False
    def __ne__(self, other):
        return not self.__eq__(other)

    def get_value(self): return self.__value
    def get_default(self): return self.__default
    def get_editable(self): return self.__editable
    def get_data_type(self): return self.__data_type

    def is_default(self):
        return self.equal(self.get_default())

    def copy_from(self, other):
        changed = Item.copy_from(self, other)
        return self.set_value(other.get_value()) or changed

    def equal(self, value):
        return self.get_value() == value

    def load(self, node):
        self.set_string(node.get())

    def save(self):
        if not self.is_default():
            value = self.get_value()
            if value is not None:
                value = str(value)
            return [XMLItem(self.get_id(), value)]
        else:
            return []
