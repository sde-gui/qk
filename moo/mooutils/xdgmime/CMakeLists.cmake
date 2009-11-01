SET(XDGMIME_SOURCES
  xdgmimealias.c
  xdgmimealias.h
  xdgmime.c
  xdgmimecache.c
  xdgmimecache.h
  xdgmimeglob.c
  xdgmimeglob.h
  xdgmime.h
  xdgmimeint.h
  xdgmimeicon.c
  xdgmimeicon.h
  xdgmimemagic.c
  xdgmimemagic.h
  xdgmimeparent.c
  xdgmimeparent.h
)

MOO_ADD_MOO_CODE_MODULE(xdgmime SUBDIR mooutils/xdgmime)
IF(MSVC)
  MOO_ADD_COMPILE_FLAGS(xdgmime "/wd4018")
ENDIF(MSVC)

# -%- strip:true -%-
