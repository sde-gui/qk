# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

moo_sublibs =

if MOO_PYTHON_BUILTIN
moo_sublibs += $(top_builddir)/moo/moopython/libmoopython.la
endif

if MOO_BUILD_APP
moo_sublibs += $(top_builddir)/moo/mooapp/libmooapp.la
endif

if MOO_BUILD_EDIT
moo_sublibs += $(top_builddir)/moo/mooedit/libmooedit.la
moo_sublibs += $(top_builddir)/moo/moofileview/libmoofileview.la
endif

if MOO_BUILD_LUA
moo_sublibs += $(top_builddir)/moo/moolua/libmoolua.la
endif

if MOO_BUILD_UTILS
moo_sublibs += $(top_builddir)/moo/mooutils/libmooutils.la
endif
