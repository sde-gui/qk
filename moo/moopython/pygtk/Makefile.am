moopygtk_sources =		\
	moo-pygtk.c		\
	moo-pygtk.h		\
	canvas-mod.h		\
	mooapp-mod.h		\
	mooedit-mod.h		\
	mooterm-mod.h		\
	mooutils-mod.h		\
	moo-mod.h

BUILT_SOURCES =			\
	canvas-mod.h		\
	mooapp-mod.h		\
	mooedit-mod.h		\
	mooterm-mod.h		\
	mooutils-mod.h		\
	moo-mod.h

nodist_moopygtk_sources =
CLEANFILES =

mooedit_defs_files =		\
	mooeditor.defs		\
	mooplugin.defs		\
	moocommand.defs
moocommand.o: mooedit-pygtk.o
	echo stamp > moocommand.o
moocommand.lo: mooedit-pygtk.lo
	echo stamp > moocommand.lo
mooplugin.o: mooedit-pygtk.o
	echo stamp > mooplugin.o
mooeditor.o: mooedit-pygtk.o
	echo stamp > mooeditor.o
moopaned.o: mooutils-pygtk.o
	echo stamp > moopaned.o
CLEANFILES += moocommand.o mooplugin.o mooeditor.o moopaned.o \
		moocommand.lo mooplugin.lo mooeditor.lo moopaned.lo

mooutils_defs_files =		\
	moofileview.defs	\
	moopaned.defs
mooutils_override_files =	\
	moopaned.override

moofileview.o: mooutils-pygtk.o
	echo stamp > moofileview.o
CLEANFILES += moofileview.o

EXTRA_DIST =			\
	moo-mod.py		\
	moo-pygtk.c		\
	mooapp-mod.py		\
	mooapp-pygtk.defs	\
	mooapp-pygtk.override	\
	mooapp-mod.c		\
	canvas-mod.py		\
	canvas-pygtk.defs	\
	canvas-pygtk.override	\
	canvas-mod.c		\
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

if MOO_BUILD_CANVAS
moopygtk_sources += canvas-mod.c
nodist_moopygtk_sources += canvas-pygtk.c
BUILT_SOURCES += canvas-pygtk.c
CLEANFILES += canvas-pygtk.c
endif MOO_BUILD_CANVAS

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
	$(srcdir)/../codegen/argtypes_m.py	\
	$(srcdir)/../codegen/reversewrapper.py

codegen_script = $(srcdir)/../codegen/codegen.py
codegen = $(PYTHON) $(codegen_script) $(codegen_platform) --pygtk-version=$(PYGTK_MINOR_VERSION)
else
codegen = $(PYTHON) $(PYGTK_CODEGEN_DIR)/codegen.py $(PYGTK_CODEGEN_ARGS)
endif

mooutils-pygtk.c: mooutils-pygtk.defs mooutils-pygtk.override $(mooutils_override_files) $(mooutils_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_utils \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--override $(srcdir)/mooutils-pygtk.override \
		--outfilename mooutils-pygtk.c \
		$(srcdir)/mooutils-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

mooterm-pygtk.c: mooterm-pygtk.defs mooterm-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_term \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooterm-pygtk.override \
		--outfilename mooterm-pygtk.c \
		$(srcdir)/mooterm-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

mooedit-pygtk.c: mooedit-pygtk.defs mooedit-pygtk.override $(mooedit_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_edit \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooedit-pygtk.override \
		--outfilename mooedit-pygtk.c \
		$(srcdir)/mooedit-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

canvas-pygtk.c: canvas-pygtk.defs canvas-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_canvas \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--override $(srcdir)/canvas-pygtk.override \
		--outfilename canvas-pygtk.c \
		$(srcdir)/canvas-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

mooapp-pygtk.c: mooapp-pygtk.defs mooapp-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_app \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooedit-pygtk.defs \
		--register $(srcdir)/mooterm-pygtk.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooapp-pygtk.override \
		--outfilename mooapp-pygtk.c \
		$(srcdir)/mooapp-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

PY2H = $(srcdir)/../../mooutils/py2h.sh
moo-mod.h: $(srcdir)/moo-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_PY $(srcdir)/moo-mod.py > $@.tmp && \
	mv $@.tmp $@
mooedit-mod.h: $(srcdir)/mooedit-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_EDIT_PY $(srcdir)/mooedit-mod.py > $@.tmp && \
	mv $@.tmp $@
mooapp-mod.h: mooapp-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_APP_PY $(srcdir)/mooapp-mod.py > $@.tmp && \
	mv $@.tmp $@
canvas-mod.h: canvas-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_CANVAS_PY $(srcdir)/canvas-mod.py > $@.tmp && \
	mv $@.tmp $@
mooutils-mod.h: mooutils-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_UTILS_PY $(srcdir)/mooutils-mod.py > $@.tmp && \
	mv $@.tmp $@
mooterm-mod.h: mooterm-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_TERM_PY $(srcdir)/mooterm-mod.py > $@.tmp && \
	mv $@.tmp $@


if MOO_OS_UNIX
noinst_LTLIBRARIES = libmoopygtk.la
else
noinst_LTLIBRARIES = libmoopygtk25.la libmoopygtk24.la
endif

libmoopygtk_la_SOURCES = $(moopygtk_sources)
libmoopygtk25_la_SOURCES = $(moopygtk_sources)
libmoopygtk24_la_SOURCES = $(moopygtk_sources)
nodist_libmoopygtk_la_SOURCES = $(nodist_moopygtk_sources)
nodist_libmoopygtk25_la_SOURCES = $(nodist_moopygtk_sources)
nodist_libmoopygtk24_la_SOURCES = $(nodist_moopygtk_sources)

AM_CFLAGS =			\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)		\
	$(MOO_W_NO_WRITE_STRINGS)\
	$(MOO_W_NO_MISSING_FIELD_INITIALIZERS)\
	$(MOO_W_NO_UNUSED)

libmoopygtk25_la_CFLAGS =	\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON25_INCLUDES)	\
	$(PYGTK25_CFLAGS)	\
	$(MOO_W_NO_WRITE_STRINGS)\
	$(MOO_W_NO_MISSING_FIELD_INITIALIZERS)\
	$(MOO_W_NO_UNUSED)

libmoopygtk24_la_CFLAGS =	\
	-I../..			\
	-I$(top_builddir)	\
	$(MOO_CFLAGS)		\
	$(MOO_DEBUG_CFLAGS)	\
	$(PYTHON24_INCLUDES)	\
	$(PYGTK24_CFLAGS)	\
	$(MOO_W_NO_WRITE_STRINGS)\
	$(MOO_W_NO_MISSING_FIELD_INITIALIZERS)\
	$(MOO_W_NO_UNUSED)
