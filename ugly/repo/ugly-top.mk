# begin ugly-top.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

EXTRA_DIST +=			\
	ugly/ugly		\
	ugly/repo/ugly-top.mk	\
	ugly/repo/ugly-pre.mk	\
	ugly/repo/ugly-post.mk	\
	ugly/repo/bdist.mk	\
	ugly/repo/bdist-top.mk

UGLY_DEPS +=			\
	ugly/repo/bdist-top.mk	\
	ugly/repo/ugly-top.mk

# end ugly-top.mk
