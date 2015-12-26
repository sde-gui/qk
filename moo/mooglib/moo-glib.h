#pragma once

#include <glib.h>
#include <glib/gstdio.h>

#ifndef MOO_DO_NOT_MANGLE_GLIB_FUNCTIONS

#undef g_stat
#undef g_lstat

#define g_stat DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD
#define g_lstat DO_NOT_USE_THIS_DIRECTLY_USE_MGW_WRAPPERS_INSTEAD

#endif // MOO_DO_NOT_MANGLE_GLIB_FUNCTIONS

//#include <mooglib/moo-glib-wrappers.h>
