__all__ = ['Setting']

from mprj.config._item import Item, create_instance
from mprj.config._xml import XMLItem


class Setting(Item):
    def __init__(self, id, value=None, default=None, editable=None, data_type=None, **kwargs):
        Item.__init__(self, id, **kwargs)

        self.__default = default
        self.__value = default
        self.__editable = editable
        self.__data_type = data_type

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

    def set_value(self, value):
        if not self.check_value(value):
            raise ValueError("%s is not a valid value for %s" % (value, self))
        if not self.equal(value):
            self._assign(value)
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

    def _assign(self, value):
        if value is not None:
            data_type = self.get_data_type()
            if data_type is not None:
                if not isinstance(value, data_type):
                    value = data_type(value)
        self.__value = value

    def get_value(self): return self.__value
    def get_default(self): return self.__default
    def get_editable(self): return self.__editable
    def get_data_type(self): return self.__data_type

    def is_default(self):
        return self.equal(self.get_default())

    def copy_from(self, other):
        self.set_value(other.get_value())

    def copy(self):
        return create_instance(type(self), self.get_id(),
                               name=self.get_name(),
                               visible=self.get_visible(),
                               value=self.get_value(),
                               default=self.get_default(),
                               editable=self.get_editable(),
                               data_type=self.get_data_type())
        return copy

    def equal(self, value):
        return self.get_value() == value

    def check_value(self, value):
        if value is None:
            return True
        data_type = self.get_data_type()
        if data_type is None:
            return True
        return isinstance(value, data_type)

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
