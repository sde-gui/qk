INCLUDE(${MOO_SOURCE_DIR}/cmake/modules/FindMooCmakeUtils.cmake)

MOO_EXEC_OR_DIE(glib-genmarshal
  COMMAND ${GLIB_GENMARSHAL_EXECUTABLE} --prefix=_moo_marshal --body ${MOO_SOURCE_DIR}/moo/marshals.list
  OUTPUT_FILE marshals.c.tmp
)
MOO_COPY_IF_CHANGED(marshals.c.tmp marshals.c)

MOO_EXEC_OR_DIE(glib-genmarshal
  COMMAND ${GLIB_GENMARSHAL_EXECUTABLE} --prefix=_moo_marshal --header ${MOO_SOURCE_DIR}/moo/marshals.list
  OUTPUT_FILE marshals.h.tmp
)
MOO_COPY_IF_CHANGED(marshals.h.tmp marshals.h)

FILE(WRITE marshals.stamp "stamp")

FILE(REMOVE marshals.h.tmp)
FILE(REMOVE marshals.c.tmp)
