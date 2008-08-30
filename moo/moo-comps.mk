# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

moo_app_ldadd =

if MOO_BUILD_SHARED_LIB

moo_app_ldadd += $(top_builddir)/moo/libmoo.la

else

if MOO_PYTHON_BUILTIN
moo_app_ldadd += $(top_builddir)/moo/moopython/libmoopython.la
endif

if MOO_BUILD_APP
moo_app_ldadd += $(top_builddir)/moo/mooapp/libmooapp.a
endif

if MOO_BUILD_EDIT
moo_app_ldadd += $(top_builddir)/moo/mooedit/libmooedit.a
moo_app_ldadd += $(top_builddir)/moo/moofileview/libmoofileview.a
endif

if MOO_BUILD_LUA
moo_app_ldadd += $(top_builddir)/moo/moolua/libmoolua.a
endif

if MOO_BUILD_UTILS
moo_app_ldadd += $(top_builddir)/moo/mooutils/libmooutils.a
endif

moo_app_ldadd += $(MOO_LIBS)

endif
