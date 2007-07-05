upstream_sources =			\
	gtksourcecontextengine.c	\
	gtksourceengine.c		\
	gtksourceiter.c			\
	gtksourcelanguage.c		\
	gtksourcelanguage-parser-1.c	\
	gtksourcelanguage-parser-2.c	\
	gtksourcelanguagemanager.c	\
	gtksourcestyle.c		\
	gtksourcestylemanager.c		\
	gtksourcestylescheme.c		\
	gtktextregion.c			\
	gtksourcecontextengine.h	\
	gtksourceengine.h		\
	gtksourceiter.h			\
	gtksourcelanguage.h		\
	gtksourcelanguage-private.h	\
	gtksourcelanguagemanager.h	\
	gtksourcestyle.h		\
	gtksourcestyle-private.h	\
	gtksourcestylemanager.h		\
	gtksourcestylescheme.h		\
	gtktextregion.h			\
	gtksourceview-utils.h		\
	gtksourceview-utils.c		\
	gtksourcelanguage-noxml.c	\
	gtksourcestylescheme-noxml.c

EXTRA_DIST = $(upstream_sources) mangle.sh

%.lo:
	cd .. && make upstream/$@
%.o:
	cd .. && make upstream/$*.lo
