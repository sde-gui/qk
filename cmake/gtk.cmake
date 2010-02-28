FIND_PACKAGE(PkgConfig)

MACRO(_MOO_GET_PKG_CONFIG_VARIABLE cmakevar pkgconfigvar pkgname)
  MOO_EXEC_OR_DIE("pkg-config --variable=${pkgconfigvar} ${pkgname}"
    COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${pkgconfigvar} ${pkgname}
    OUTPUT_VARIABLE ${cmakevar})
  STRING(STRIP ${${cmakevar}} ${cmakevar})
ENDMACRO(_MOO_GET_PKG_CONFIG_VARIABLE)

PKG_CHECK_MODULES(GTK REQUIRED gtk+-2.0>=2.14.0 glib-2.0 gio-2.0 gmodule-2.0 gthread-2.0 gobject-2.0)
LIST(APPEND MOO_DEP_LIBS ${GTK_LIBRARIES})

MOO_ADD_COMPILE_DEFINITIONS(RELEASE -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT)
MOO_ADD_COMPILER_FLAGS(GCC WIN32 -mms-bitfields)

# Do not use pkg-config because the official win32 binary
# is distributed without pc files
FIND_PACKAGE(MooLibXml2)
IF(NOT LIBXML2_FOUND)
  MOO_ERROR("Libxml2 not found")
ENDIF(NOT LIBXML2_FOUND)
SET(MOO_USE_XML TRUE)
MOO_DEFINE_H(MOO_USE_XML)
LIST(APPEND MOO_DEP_LIBS ${LIBXML2_LIBRARIES})

LINK_DIRECTORIES(${GTK_LIBRARY_DIRS} ${XML_LIBRARY_DIRS})

_MOO_GET_PKG_CONFIG_VARIABLE(_moo_gdk_target target gdk-2.0)
SET(GDK_X11 FALSE)
SET(GDK_WIN32 FALSE)
SET(GDK_QUARTZ FALSE)
IF("${_moo_gdk_target}" STREQUAL "x11")
  SET(GDK_X11 TRUE)
ELSEIF("${_moo_gdk_target}" STREQUAL "quartz")
  SET(GDK_QUARTZ TRUE)
ELSEIF("${_moo_gdk_target}" STREQUAL "win32")
  SET(GDK_WIN32 TRUE)
ENDIF("${_moo_gdk_target}" STREQUAL "x11")

MACRO(_MOO_FIND_PROGRAM_OR_DIE varname progname pkgvar pkgname)
  _MOO_GET_PKG_CONFIG_VARIABLE(_moo_pkg_exec_prefix exec_prefix ${pkgname})
  IF(pkgvar)
    _MOO_GET_PKG_CONFIG_VARIABLE(_moo_pkg_prog ${pkgvar} ${pkgname})
    SET(_moo_prg_names "${_moo_pkg_prog}" ${progname})
  ELSE(pkgvar)
    SET(_moo_prg_names ${progname})
  ENDIF(pkgvar)
  FIND_PROGRAM(${varname} 
    NAMES ${_moo_prg_names}
    PATHS "${_moo_pkg_exec_prefix}/bin" 
    DOC "Path to ${progname} executable")
  MARK_AS_ADVANCED(${varname})
  IF(NOT ${varname})
    MOO_ERROR("Could not find ${progname} executable")
  ENDIF(NOT ${varname})
ENDMACRO(_MOO_FIND_PROGRAM_OR_DIE)

_MOO_FIND_PROGRAM_OR_DIE(GLIB_GENMARSHAL_EXECUTABLE glib-genmarshal glib_genmarshal glib-2.0)
_MOO_FIND_PROGRAM_OR_DIE(GDK_PIXBUF_CSOURCE_EXECUTABLE gdk-pixbuf-csource "" gtk+-2.0)
