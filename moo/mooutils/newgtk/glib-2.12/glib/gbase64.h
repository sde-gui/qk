#ifndef G_BASE64_MANGLED_H
#define G_BASE64_MANGLED_H

#define MANGLE_B64_PREFIX moo
#define MANGLE_B64(func) MANGLE_B642(MANGLE_B64_PREFIX,func)
#define MANGLE_B642(prefix,func) MANGLE_B643(prefix,func)
#define MANGLE_B643(prefix,func) prefix##_##func

#define g_base64_encode_step    MANGLE_B64(g_base64_encode_step)
#define g_base64_encode_close   MANGLE_B64(g_base64_encode_close)
#define g_base64_encode         MANGLE_B64(g_base64_encode)
#define g_base64_decode_step    MANGLE_B64(g_base64_decode_step)
#define g_base64_decode         MANGLE_B64(g_base64_decode)

#include "gbase64-real.h"

#endif /* G_BASE64_MANGLED_H */
