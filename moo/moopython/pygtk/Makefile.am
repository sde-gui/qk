moopygtk_sources =		\
	moo-pygtk.c		\
	moo-pygtk.h		\
	mooapp-mod.h		\
	mooedit-mod.h		\
	mooterm-mod.h		\
	mooutils-mod.h		\
	moo-mod.h

BUILT_SOURCES =			\
	mooapp-mod.h		\
	mooedit-mod.h		\
	mooterm-mod.h		\
	mooutils-mod.h		\
	moo-mod.h

nodist_moopygtk_sources =
CLEANFILES =

mooedit_defs_files =		\
	mooeditor.defs		\
	mooplugin.defs

mooutils_defs_files =		\
	moofileview.defs

EXTRA_DIST =			\
	moo-mod.py		\
	moo-pygtk.c		\
	mooapp-mod.py		\
	mooapp-pygtk.defs	\
	mooapp-pygtk.override	\
	mooapp-mod.c		\
	$(mooedit_defs_files)	\
	mooedit-mod.py		\
	mooedit-pygtk.defs	\
	mooedit-pygtk.override	\
	mooedit-mod.c		\
	mooterm-mod.py		\
	mooterm-pygtk.defs	\
	mooterm-pygtk.override	\
	mooterm-mod.c		\
	mooutils-mod.py		\
	mooutils-pygtk.defs	\
	mooutils-pygtk.override	\
	mooutils-mod.c		\
	$(mooutils_defs_files)	\
	$(mooutils_override_files)


if MOO_BUILD_UTILS
moopygtk_sources += mooutils-mod.c
nodist_moopygtk_sources += mooutils-pygtk.c
BUILT_SOURCES += mooutils-pygtk.c
CLEANFILES += mooutils-pygtk.c
endif MOO_BUILD_UTILS

if MOO_BUILD_TERM
moopygtk_sources += mooterm-mod.c
nodist_moopygtk_sources += mooterm-pygtk.c
BUILT_SOURCES += mooterm-pygtk.c
CLEANFILES += mooterm-pygtk.c
endif MOO_BUILD_TERM

if MOO_BUILD_EDIT
moopygtk_sources += mooedit-mod.c
nodist_moopygtk_sources += mooedit-pygtk.c
BUILT_SOURCES += mooedit-pygtk.c
CLEANFILES += mooedit-pygtk.c
endif MOO_BUILD_EDIT

if MOO_BUILD_APP
moopygtk_sources += mooapp-mod.c
nodist_moopygtk_sources += mooapp-pygtk.c
BUILT_SOURCES += mooapp-pygtk.c
CLEANFILES += mooapp-pygtk.c
endif MOO_BUILD_APP

if MOO_OS_MINGW
codegen_platform = --platform win32
endif

if MOO_USE_CUSTOM_CODEGEN
codegen_files =					\
	$(srcdir)/../codegen/codegen.py		\
	$(srcdir)/../codegen/argtypes.py	\
	$(srcdir)/../codegen/reversewrapper.py

codegen_script = $(srcdir)/../codegen/codegen.py
codegen = $(PYTHON) $(codegen_script) $(codegen_platform) --pygtk-version=$(PYGTK_MINOR_VERSION)
else
codegen = $(PYTHON) $(PYGTK_CODEGEN_DIR)/codegen.py $(PYGTK_CODEGEN_ARGS)
endif

mooutils-pygtk.c: mooutils-pygtk.defs mooutils-pygtk.override $(mooutils_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_utils \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--override $(srcdir)/mooutils-pygtk.override \
		--outfilename mooutils-pygtk.c \
		$(srcdir)/mooutils-pygtk.defs > mooutils-pygtk.c

mooterm-pygtk.c: mooterm-pygtk.defs mooterm-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_term \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooterm-pygtk.override \
		--outfilename mooterm-pygtk.c \
		$(srcdir)/mooterm-pygtk.defs > mooterm-pygtk.c

mooedit-pygtk.c: mooedit-pygtk.defs mooedit-pygtk.override $(mooedit_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_edit \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooedit-pygtk.override \
		--outfilename mooedit-pygtk.c \
		$(srcdir)/mooedit-pygtk.defs > mooedit-pygtk.c

mooapp-pygtk.c: mooapp-pygtk.defs mooapp-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_app \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooedit-pygtk.defs \
		--register $(srcdir)/mooterm-pygtk.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooapp-pygtk.override \
		--outfilename mooapp-pygtk.c \
		$(srcdir)/mooapp-pygtk.defs > mooapp-pygtk.c

PY2H = $(srcdir)/../../mooutils/py2h.sh
moo-mod.h: $(srcdir)/moo-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_PY $(srcdir)/moo-mod.py > moo-mod.h
mooedit-mod.h: $(srcdir)/mooedit-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_EDIT_PY $(srcdir)/mooedit-mod.py > mooedit-mod.h
mooapp-mod.h: mooapp-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_APP_PY $(srcdir)/mooapp-mod.py > mooapp-mod.h
mooutils-mod.h: mooutils-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_UTILS_PY $(srcdir)/mooutils-mod.py > mooutils-mod.h
mooterm-mod.h: mooterm-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_TERM_PY $(srcdir)/mooterm-mod.py > mooterm-mod.h


if MOO_OS_UNIX
noinst_LTLIBRARIES = libmoopygtk.la
else
noinst_LTLIBRARIES = libmoopygtk23.la libmoopygtk24.la
endif

libmoopygtk_la_SOURCES = $(moopygtk_sources)
libmoopygtk23_la_SOURCES = $(moopygtk_sources)
libmoopygtk24_la_SOURCES = $(moopygtk_sources)
nodist_libmoopygtk_la_SOURCES = $(nodist_moopygtk_sources)
nodist_libmoopygtk23_la_SOURCES = $(nodist_moopygtk_sources)
nodist_libmoopygtk24_la_SOURCES = $(nodist_moopygtk_sources)

AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_CFLAGS)	\
	$(PYGTK_CFLAGS)

libmoopygtk23_la_CFLAGS =	\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON23_CFLAGS)	\
	$(PYGTK23_CFLAGS)

libmoopygtk24_la_CFLAGS =	\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON24_CFLAGS)	\
	$(PYGTK24_CFLAGS)
