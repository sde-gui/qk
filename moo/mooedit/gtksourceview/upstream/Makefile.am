upstream_sources =			\
	gtksourcecontextengine.c	\
	gtksourceengine.c		\
	gtksourceiter.c			\
	gtksourcelanguage.c		\
	gtksourcelanguage-parser-1.c	\
	gtksourcelanguage-parser-2.c	\
	gtksourcelanguagesmanager.c	\
	gtksourcestyle.c		\
	gtksourcestylemanager.c		\
	gtksourcestylescheme.c		\
	gtktextregion.c			\
	gtksourcecontextengine.h	\
	gtksourceengine.h		\
	gtksourceiter.h			\
	gtksourcelanguage.h		\
	gtksourcelanguage-private.h	\
	gtksourcelanguagesmanager.h	\
	gtksourcestyle.h		\
	gtksourcestylemanager.h		\
	gtksourcestylescheme.h		\
	gtktextregion.h

EXTRA_DIST = $(upstream_sources) mangle.sh

%.lo:
	cd .. && make upstream/$*.o
%.o:
	cd .. && make upstream/$@
