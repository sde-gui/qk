# SRCDIR=
# PYTHON_EXECUTABLE=
# PYGTK_MINOR_VERSION=
# PYGTK_DEFS_DIR=
# OUTPUT=

IF(WIN32)
  SET(codegen_platform --platform win32)
ELSE(WIN32)
  SET(codegen_platform)
ENDIF(WIN32)

SET(codegen_script ${SRCDIR}/codegen/codegen.py)
SET(codegen ${PYTHON_EXECUTABLE} ${codegen_script} ${codegen_platform} --pygtk-version=${PYGTK_MINOR_VERSION})

IF(OUTPUT STREQUAL mooutils-pygtk.c)
  MOO_EXEC_OR_DIE(
    COMMAND ${codegen} --prefix _moo_utils --load-types $(srcdir)/codegen/argtypes_m.py
            --register ${PYGTK_DEFS_DIR}/gtk-types.defs
            --register ${PYGTK_DEFS_DIR}/gdk-types.defs
            --override ${SRCDIR}/pygtk/mooutils-pygtk.override
            --outfilename pygtk/mooutils-pygtk.c
            ${srcdir}/pygtk/mooutils-pygtk.defs
    OUTPUT_FILE pygtk/mooutils-pygtk.c.tmp
  )
  FILE(RENAME pygtk/mooutils-pygtk.c.tmp pygtk/mooutils-pygtk.c)
  FILE(WRITE pygtk/mooutils-pygtk.stamp "stamp")
ELSEIF(OUTPUT STREQUAL mooapp-pygtk.c)
  MOO_EXEC_OR_DIE(
    COMMAND ${codegen} --prefix _moo_app --load-types $(srcdir)/codegen/argtypes_m.py
            --register ${PYGTK_DEFS_DIR}/gtk-types.defs
            --register ${PYGTK_DEFS_DIR}/gdk-types.defs
            --register ${SRCDIR}/pygtk/mooutils-pygtk.defs
            --override ${SRCDIR}/pygtk/mooapp-pygtk.override
            --outfilename pygtk/mooapp-pygtk.c
            ${srcdir}/pygtk/mooapp-pygtk.defs
    OUTPUT_FILE pygtk/mooapp-pygtk.c.tmp
  )
  FILE(RENAME pygtk/mooapp-pygtk.c.tmp pygtk/mooapp-pygtk.c)
  FILE(WRITE pygtk/mooapp-pygtk.stamp "stamp")
ELSEIF(OUTPUT STREQUAL mooedit-pygtk.c)
  MOO_EXEC_OR_DIE(
    COMMAND ${codegen} --prefix _moo_edit --load-types $(srcdir)/codegen/argtypes_m.py
            --register ${PYGTK_DEFS_DIR}/gtk-types.defs
            --register ${PYGTK_DEFS_DIR}/gdk-types.defs
            --register ${SRCDIR}/pygtk/mooutils-pygtk.defs
            --override ${SRCDIR}/pygtk/mooedit-pygtk.override
            --outfilename pygtk/mooedit-pygtk.c
            ${srcdir}/pygtk/mooedit-pygtk.defs
    OUTPUT_FILE pygtk/mooedit-pygtk.c.tmp
  )
  FILE(RENAME pygtk/mooedit-pygtk.c.tmp pygtk/mooedit-pygtk.c)
  FILE(WRITE pygtk/mooedit-pygtk.stamp "stamp")
ELSEIF(OUTPUT STREQUAL mooutils-mod.h OR OUTPUT STREQUAL mooedit-mod.h OR OUTPUT STREQUAL mooapp-mod.h)
ENDIF(OUTPUT STREQUAL mooutils-pygtk.c)

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
