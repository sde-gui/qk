# begin ugly-top.mk
# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

EXTRA_DIST +=			\
	ugly/ugly		\
	ugly/repo/ugly-top.mk	\
	ugly/repo/ugly-sub.mk	\
	ugly/repo/ugly-pre.mk	\
	ugly/repo/ugly-post.mk	\
	ugly/repo/bdist.mk	\
	ugly/repo/bdist-top.mk	\
	ugly/repo/ugly-subdir-Makefile

UGLY_DEPS +=			\
	ugly/repo/bdist-top.mk	\
	ugly/repo/ugly-top.mk

$(top_srcdir)/m4/ugly-stamp.m4: $(UGLY_STAMP_DEPS)
	files=""; for file in $(UGLY_STAMP_DEPS); do \
	  dir=`dirname $$file`; dir=`cd $$dir && pwd`; \
	  files="$$files $$dir/`basename $$file`"; \
	done && \
	cd $(srcdir) && $(SHELL) $(UGLY) --stamp $$files

# end ugly-top.mk
