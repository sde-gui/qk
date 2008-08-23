# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

moo_sublibs =

if MOO_PYTHON_BUILTIN
moo_sublibs += $(top_builddir)/moo/moopython/libmoopython.la
endif

if MOO_BUILD_APP
if MOO_INSTALL_LIB
moo_sublibs += $(top_builddir)/moo/mooapp/libmooapp.la
else
moo_sublibs += $(top_builddir)/moo/mooapp/libmooapp.a
endif
endif

if MOO_BUILD_EDIT
if MOO_INSTALL_LIB
moo_sublibs += $(top_builddir)/moo/mooedit/libmooedit.la
moo_sublibs += $(top_builddir)/moo/moofileview/libmoofileview.la
else
moo_sublibs += $(top_builddir)/moo/mooedit/libmooedit.a
moo_sublibs += $(top_builddir)/moo/moofileview/libmoofileview.a
endif
endif

if MOO_BUILD_LUA
if MOO_INSTALL_LIB
moo_sublibs += $(top_builddir)/moo/moolua/libmoolua.la
else
moo_sublibs += $(top_builddir)/moo/moolua/libmoolua.a
endif
endif

if MOO_BUILD_UTILS
if MOO_INSTALL_LIB
moo_sublibs += $(top_builddir)/moo/mooutils/libmooutils.la
else
moo_sublibs += $(top_builddir)/moo/mooutils/libmooutils.a
endif
endif
