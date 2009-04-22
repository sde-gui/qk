# begin ugly-top.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

EXTRA_DIST +=			\
	ugly/ugly		\
	ugly/ugly-top.mk	\
	ugly/ugly-sub.mk	\
	ugly/ugly-pre.mk	\
	ugly/ugly-post.mk	\
	ugly/bdist.mk		\
	ugly/bdist-top.mk	\
	ugly/ugly-subdir-Makefile

UGLY_DEPS +=			\
	ugly/bdist-top.mk	\
	ugly/ugly-top.mk

$(top_srcdir)/m4/ugly-stamp.m4: $(UGLY_STAMP_DEPS)
	files=""; for file in $(UGLY_STAMP_DEPS); do \
	  dir=`dirname $$file`; dir=`cd $$dir && pwd`; \
	  files="$$files $$dir/`basename $$file`"; \
	done && \
	cd $(srcdir) && $(SHELL) ./ugly/ugly --stamp $$files

# end ugly-top.mk
