# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

BDIST_SUBDIRS = $(SUBDIRS)

if MOO_OS_MINGW
BDIST_FILES += $(BWIN32_FILES)
BDIST_EXTRA += $(BWIN32_EXTRA)
else
if MOO_OS_DARWIN
BDIST_FILES += $(BMAC_FILES)
BDIST_EXTRA += $(BMAC_EXTRA)
else
BDIST_FILES += $(BFDO_FILES)
BDIST_EXTRA += $(BFDO_EXTRA)
endif
endif

bdistsubdir: $(BDIST_FILES) $(BDIST_EXTRA)
	@srcdirstrip=`echo "$(srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	topsrcdirstrip=`echo "$(top_srcdir)" | sed 's/[].[^$$\\*]/\\\\&/g'`; \
	list='$(BDIST_FILES)'; \
	  dist_files=`for file in $$list; do echo $$file; done | \
	  sed -e "s|^$$srcdirstrip/||;t" \
	      -e "s|^$$topsrcdirstrip/|$(top_builddir)/|;t"`; \
	case $$dist_files in \
	  */*) $(MKDIR_P) `echo "$$dist_files" | \
			   sed '/\//!d;s|^|$(bdistdir)/|;s,/[^/]*$$,,' | \
			   sort -u` ;; \
	esac; \
	for file in $$dist_files; do \
	  if test -f $$file || test -d $$file; then d=.; else d=$(srcdir); fi; \
	  if test -d $$d/$$file; then \
	    dir=`echo "/$$file" | sed -e 's,/[^/]*$$,,'`; \
	    if test -d $(srcdir)/$$file && test $$d != $(srcdir); then \
	      cp -pR $(srcdir)/$$file $(bdistdir)$$dir || exit 1; \
	    fi; \
	    cp -pR $$d/$$file $(bdistdir)$$dir || exit 1; \
	  else \
	    test -f $(bdistdir)/$$file \
	    || cp -p $$d/$$file $(bdistdir)/$$file \
	    || exit 1; \
	  fi; \
	done
	bdistdir=`$(am__cd) $(bdistdir) && pwd`; \
	list='$(BDIST_SUBDIRS)'; for subdir in $$list; do \
	  if test "$$subdir" = .; then :; else \
	    (cd $$subdir && \
	      $(MAKE) $(AM_MAKEFLAGS) \
	        bdistdir="$$bdistdir" \
	        bdistsubdir) \
	      || exit 1; \
	  fi; \
	done
	bdistdir=`$(am__cd) $(bdistdir) && pwd`; \
	list='$(BDIST_EXTRA)'; for target in $$list; do \
	  $(MAKE) $(AM_MAKEFLAGS) bdistdir="$$bdistdir" $$target || exit 1; \
	done
