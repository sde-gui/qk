# -%- indent-width:4; use-tabs:no; strip: yes -%-

import _medit_raw

__all__ = ['Object', 'get_app_obj']

class Variant(object):
    def __init__(self, value=None):
        object.__init__(self)
        self.p = _medit_raw.variant_new()
        if value is None:
            pass
        elif isinstance(value, bool):
            _medit_raw.variant_set_bool(self.p, value)
        elif isinstance(value, int):
            _medit_raw.variant_set_int(self.p, value)
        elif isinstance(value, float):
            _medit_raw.variant_set_double(self.p, value)
        elif isinstance(value, str):
            _medit_raw.variant_set_string(self.p, value)
        elif isinstance(value, Object):
            _medit_raw.variant_set_object(self.p, value._id())
        elif isinstance(value, list):
            va = VariantArray(value)
            _medit_raw.variant_set_array(self.p, va.p)
        elif isinstance(value, dict):
            va = VariantDict(value)
            _medit_raw.variant_set_dict(self.p, va.p)
        else:
            raise RuntimeError("don't know how to wrap value '%r'" % (value,))

    def __del__(self):
        if self.p:
            _medit_raw.variant_free(self.p)
        self.p = None

class VariantArray(object):
    def __init__(self, items):
        object.__init__(self)
        self.p = None
        v_items = [Variant(i) for i in items]
        self.p = _medit_raw.variant_array_new()
        for i in v_items:
            _medit_raw.variant_array_append(self.p, i.p)

    def __del__(self):
        if self.p:
            _medit_raw.variant_array_free(self.p)
        self.p = None

class VariantDict(object):
    def __init__(self, dic):
        object.__init__(self)
        self.p = None
        v_dic = {}
        for k in dic:
            if not isinstance(k, str):
                raise ValueError('dict key must be a string')
            v_dic[k] = Variant(dic[k])
        self.p = _medit_raw.variant_dict_new()
        for k in v_dic:
            _medit_raw.variant_dict_set(self.p, k, v_dic[k].p)

    def __del__(self):
        if self.p:
            _medit_raw.variant_dict_free(self.p)
        self.p = None

class Method(object):
    def __init__(self, obj, meth):
        object.__init__(self)
        self.__obj = obj
        self.__meth = meth

    def __call__(self, *args, **kwargs):
        args_v = VariantArray(args)
        kwargs_v = VariantDict(kwargs)
        ret = eval(_medit_raw.call_method(self.__obj._id(), self.__meth, args_v.p, kwargs_v.p))
        if isinstance(ret, Exception):
            raise ret
        else:
            return ret

class Object(object):
    callbacks = {}

    @classmethod
    def _invoke_callback(cls, callback_id, retval_id, *args):
        cb = cls.callbacks[callback_id]
        ret = cb(*args)
        ret_v = Variant(ret)
        _medit_raw.push_retval(retval_id, ret_v.p)

    def __init__(self, id):
        object.__init__(self)
        self.__id = id

    def _id(self):
        return self.__id

    def connect_callback(self, sig, callback):
        ret = eval(_medit_raw.connect_callback(self._id(), sig))
        if isinstance(ret, Exception):
            raise ret
        Object.callbacks[ret] = callback
        return ret

    def disconnect_callback(self, callback_id):
        ret = eval(_medit_raw.connect_callback(self._id(), sig))
        if isinstance(ret, Exception):
            raise ret
        del Object.callbacks[callback_id]

    def __str__(self):
        return '<Object id=%d>' % (self._id(),)

    def __repr__(self):
        return '<Object id=%d>' % (self._id(),)

    def __getattr__(self, attr):
        return Method(self, attr)

_medit_raw.Object = Object

def get_app_obj():
    return Object(_medit_raw.get_app_obj())

# print VariantArray([None, False, True, 1, 14, 3.34345, 'fgoo', get_app_obj(), [12,3,4], {'a':1}])
# print VariantDict({'a': [1,2,3]})
