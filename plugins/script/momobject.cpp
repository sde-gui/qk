#include "momobject.h"

namespace mom {
namespace impl {

Result MomObject::call_method(const char *method, const MomValue *args, guint n_args, MomValue *ret)
{
    g_return_val_if_fail(MOM_UNEXPECTED, method != NULL);
    g_return_val_if_fail(MOM_UNEXPECTED, n_args == 0 || args != NULL);

    MethodInfo *meth = get_class()->lookup_method(method);
    if (!meth)
        return MOM_UNKNOWN_METHOD;

    if (n_args != meth->n_args)
        return MOM_BAD_ARGS;

    for (guint i = 0; i < n_args; ++i)
    {
        ArgInfo &a = meth->args[i];
        if (MOM_VALUE_IS_NONE (&args[i]))
            return MOM_BAD_ARGS;
    }

    return meth->pclass->call_method(this, meth, args, n_args, ret);
}

}
}