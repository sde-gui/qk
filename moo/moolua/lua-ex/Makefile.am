noinst_LTLIBRARIES = liblua-ex-posix.la liblua-ex-win32.la liblua-ex.la

# There are different source files with the same names, so
# we have to use two libraries with separate CPPFLAGS, to have
# object files renamed
liblua_ex_la_SOURCES = lua-ex.h
liblua_ex_la_LIBADD = liblua-ex-posix.la liblua-ex-win32.la
liblua_ex_posix_la_SOURCES = dummy.c
liblua_ex_win32_la_SOURCES = dummy.c

win32_sources =			\
	w32api/dirent.c		\
	w32api/dirent.h		\
	w32api/spawn.c		\
	w32api/spawn.h		\
	w32api/pusherror.c	\
	w32api/pusherror.h	\
	w32api/ex.c

posix_spawn_sources =		\
	posix/posix_spawn.c	\
	posix/posix_spawn.h

posix_sources =			\
	posix/spawn.c		\
	posix/spawn.h		\
	posix/ex.c

EXTRA_DIST =			\
	$(win32_sources)	\
	$(posix_spawn_sources)	\
	$(posix_sources)

if MOO_OS_MINGW
liblua_ex_win32_la_SOURCES += $(win32_sources)
win32_cflags = -DWIN32_LEAN_AND_MEAN -DNOGDI
else !MOO_OS_MINGW
liblua_ex_posix_la_SOURCES += $(posix_sources)
posix_cflags =			\
	-D_XOPEN_SOURCE=600	\
	-DENVIRON_DECL="extern char **environ;"
if !HAVE_POSIX_SPAWN
posix_cflags += -DMISSING_POSIX_SPAWN
liblua_ex_posix_la_SOURCES += $(posix_spawn_sources)
endif !HAVE_POSIX_SPAWN
endif !MOO_OS_MINGW

common_cflags = -I$(srcdir)/.. -I$(srcdir)/../..
liblua_ex_posix_la_CPPFLAGS = $(posix_cflags) $(common_cflags)
liblua_ex_win32_la_CPPFLAGS = $(win32_cflags) $(common_cflags)
