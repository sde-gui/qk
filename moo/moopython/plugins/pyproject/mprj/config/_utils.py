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
