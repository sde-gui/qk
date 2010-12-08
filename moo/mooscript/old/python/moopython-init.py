    # continuation of __medit_init function

    sys.path = [ctypes.c_char_p(addr).value for addr in reversed(path_entries)] + sys.path

    _medit_raw = imp.new_module('_medit_raw')
    sys.modules['_medit_raw'] = _medit_raw

    for f in funcs:
        setattr(_medit_raw, f, funcs[f])

    import moo._medit

__medit_init()
del __medit_init
