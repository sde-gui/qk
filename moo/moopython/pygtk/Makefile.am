moopygtk_sources =		\
	moo-pygtk.c		\
	moo-pygtk.h		\
	mooapp-mod.h		\
	mooedit-mod.h		\
	mooutils-mod.h		\
	moo-mod.h

BUILT_SOURCES =			\
	mooapp-mod.h		\
	mooedit-mod.h		\
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

EXTRA_DIST =				\
	moo-mod.py			\
	moo-pygtk.c			\
	mooapp-mod.py			\
	mooapp-pygtk.defs		\
	mooapp-pygtk.override		\
	mooapp-mod.c			\
	$(mooedit_defs_files)		\
	mooedit-mod.py			\
	mooedit-pygtk.defs		\
	mooedit-pygtk.override		\
	mooedit-mod.c			\
	mooutils-mod.py			\
	mooutils-pygtk.defs		\
	mooutils-pygtk.override		\
	mooutils-mod.c			\
	$(mooutils_defs_files)		\
	$(mooutils_override_files)

if MOO_BUILD_UTILS
moopygtk_sources += mooutils-mod.c
nodist_moopygtk_sources += mooutils-pygtk.c
BUILT_SOURCES += mooutils-pygtk.c
CLEANFILES += mooutils-pygtk.c
endif MOO_BUILD_UTILS

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

codegen_files =					\
	$(srcdir)/../codegen/codegen.py		\
	$(srcdir)/../codegen/argtypes.py	\
	$(srcdir)/../codegen/argtypes_m.py	\
	$(srcdir)/../codegen/reversewrapper.py
codegen_script = $(srcdir)/../codegen/codegen.py
codegen = $(PYTHON) $(codegen_script) $(codegen_platform) --pygtk-version=$(PYGTK_MINOR_VERSION)

mooutils-pygtk.c: mooutils-pygtk.defs mooutils-pygtk.override $(mooutils_override_files) $(mooutils_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_utils \
		--load-types $(srcdir)/../codegen/argtypes_m.py \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--override $(srcdir)/mooutils-pygtk.override \
		--outfilename mooutils-pygtk.c \
		$(srcdir)/mooutils-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

mooapp-pygtk.c: mooapp-pygtk.defs mooapp-pygtk.override $(codegen_files)
	$(codegen) --prefix _moo_app \
		--load-types $(srcdir)/../codegen/argtypes_m.py \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooapp-pygtk.override \
		--outfilename mooapp-pygtk.c \
		$(srcdir)/mooapp-pygtk.defs > $@.tmp && \
		mv $@.tmp $@

mooedit-pygtk.c: mooedit-pygtk.defs mooedit-pygtk.override $(mooedit_defs_files) $(codegen_files)
	$(codegen) --prefix _moo_edit \
		--load-types $(srcdir)/../codegen/argtypes_m.py \
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs \
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs \
		--register $(srcdir)/mooutils-pygtk.defs \
		--override $(srcdir)/mooedit-pygtk.override \
		--outfilename mooedit-pygtk.c \
		$(srcdir)/mooedit-pygtk.defs > $@.tmp && \
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
mooutils-mod.h: mooutils-mod.py $(PY2H)
	$(SHELL) $(PY2H) MOO_UTILS_PY $(srcdir)/mooutils-mod.py > $@.tmp && \
	mv $@.tmp $@


noinst_LTLIBRARIES = libmoopygtk.la

libmoopygtk_la_SOURCES = $(moopygtk_sources)
nodist_libmoopygtk_la_SOURCES = $(nodist_moopygtk_sources)
libmoopygtk_la_LIBADD =

AM_CFLAGS =			\
	$(MOO_CFLAGS)		\
	$(MOO_W_NO_WRITE_STRINGS)\
	$(MOO_W_NO_UNUSED)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)

AM_CXXFLAGS =			\
	$(MOO_CXXFLAGS)		\
	$(MOO_W_NO_WRITE_STRINGS)\
	$(MOO_W_NO_UNUSED)	\
	$(PYTHON_INCLUDES)	\
	$(PYGTK_CFLAGS)
