SET(MOOPYGTK_SOURCES
  pygtk/moo-pygtk.c
  pygtk/moo-pygtk.h
  pygtk/mooapp-mod.h
  pygtk/mooedit-mod.h
  pygtk/mooutils-mod.h
  pygtk/moo-mod.h
)

SET(mooedit_defs_files
  pygtk/mooeditor.defs
  pygtk/mooplugin.defs
  pygtk/moocommand.defs
)

SET(mooutils_defs_files
  pygtk/moofileview.defs
  pygtk/moopaned.defs
)
SET(mooutils_override_files
  pygtk/moopaned.override
)

IF(WIN32)
  SET(codegen_platform --platform win32)
ELSE(WIN32)
  SET(codegen_platform)
ENDIF(WIN32)

SET(codegen_files
  codegen/codegen.py
  codegen/argtypes.py
  codegen/argtypes_m.py
  codegen/reversewrapper.py
)

GET_FILENAME_COMPONENT(_moo_abs_srcdir ${CMAKE_CURRENT_SOURCE_DIR} ABSOLUTE)

SET(codegen_script ${_moo_abs_srcdir}/codegen/codegen.py)
SET(codegen ${PYTHON_EXECUTABLE} ${codegen_script} ${codegen_platform} --pygtk-version=${PYGTK_MINOR_VERSION})

pygtk/mooutils-pygtk.c: pygtk/mooutils-pygtk.defs pygtk/mooutils-pygtk.override $(mooutils_override_files) $(mooutils_defs_files) $(codegen_files)
	mkdir -p pygtk
	$(codegen) --prefix _moo_utils
		--load-types $(srcdir)/codegen/argtypes_m.py
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs
		--override $(srcdir)/pygtk/mooutils-pygtk.override
		--outfilename pygtk/mooutils-pygtk.c
		$(srcdir)/pygtk/mooutils-pygtk.defs > $@.tmp &&
		mv $@.tmp $@

pygtk/mooapp-pygtk.c: pygtk/mooapp-pygtk.defs pygtk/mooapp-pygtk.override $(codegen_files)
	mkdir -p pygtk
	$(codegen) --prefix _moo_app
		--load-types $(srcdir)/codegen/argtypes_m.py
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs
		--register $(srcdir)/pygtk/mooutils-pygtk.defs
		--override $(srcdir)/pygtk/mooapp-pygtk.override
		--outfilename pygtk/mooapp-pygtk.c
		$(srcdir)/pygtk/mooapp-pygtk.defs > $@.tmp &&
		mv $@.tmp $@

pygtk/mooedit-pygtk.c: pygtk/mooedit-pygtk.defs pygtk/mooedit-pygtk.override $(mooedit_defs_files) $(codegen_files)
	mkdir -p pygtk
	$(codegen) --prefix _moo_edit
		--load-types $(srcdir)/codegen/argtypes_m.py
		--register $(PYGTK_DEFS_DIR)/gtk-types.defs
		--register $(PYGTK_DEFS_DIR)/gdk-types.defs
		--register $(srcdir)/pygtk/mooutils-pygtk.defs
		--override $(srcdir)/pygtk/mooedit-pygtk.override
		--outfilename pygtk/mooedit-pygtk.c
		$(srcdir)/pygtk/mooedit-pygtk.defs > $@.tmp &&
		mv $@.tmp $@

PY2H = $(srcdir)/../mooutils/py2h.sh
pygtk/%-mod.h: $(srcdir)/pygtk/%-mod.py $(PY2H)
	mkdir -p pygtk
	$(SHELL) $(PY2H) `echo $* | tr '[a-z]' '[A-Z]'`_PY $(srcdir)/pygtk/$*-mod.py > $@.tmp &&
	mv $@.tmp $@

FOREACH(_moo_comp utils edit app)
  STRING(TOUPPER ${_moo_comp} _moo_COMP)
  MOO_ADD_GENERATED_FILE(
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-pygtk.stamp
    ${CMAKE_CURRENT_BINARY_DIR}/pygtk/moo${_moo_comp}-pygtk.c
    ${MOO_${_moo_COMP}_PYGTK_COMMAND}
    DEPENDS pygtk/moo${_moo_comp}-pygtk.defs pygtk/moo${_moo_comp}-pygtk.override ${codegen_files})
ENDFOREACH(_moo_comp)

# noinst_LTLIBRARIES += libmoopygtk.la
#
# libmoopygtk_la_SOURCES = $(moopygtk_sources)
# nodist_libmoopygtk_la_SOURCES = $(nodist_moopygtk_sources)
# libmoopygtk_la_LIBADD =
#
# libmoopygtk_la_CFLAGS =
# 	-Ipygtk
# 	$(MOO_CFLAGS)
# 	$(MOO_W_NO_WRITE_STRINGS)
# 	$(MOO_W_NO_UNUSED)
# 	$(PYTHON_INCLUDES)
# 	$(PYGTK_CFLAGS)
#
# libmoopygtk_la_CXXFLAGS =
# 	-Ipygtk
# 	$(MOO_CXXFLAGS)
# 	$(MOO_W_NO_WRITE_STRINGS)
# 	$(MOO_W_NO_UNUSED)
# 	$(PYTHON_INCLUDES)
# 	$(PYGTK_CFLAGS)

# -%- strip: true -%-
