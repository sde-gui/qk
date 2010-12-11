docparser_files =		\
	parsedocs.py		\
	mdp/__init__.py		\
	mdp/module.py		\
	mdp/docparser.py	\
	mdp/xmlwriter.py

mpi_files =			\
	gendefs.py		\
	mpi/__init__.py		\
	mpi/module.py		\
	mpi/defswriter.py

EXTRA_DIST =			\
	$(docparser_files)	\
	$(xmlparser_files)	\
	moo.xml

BUILT_SOURCES =

if MOO_DEV_MODE

include sourcefiles.mak

BUILT_SOURCES += moo.xml.stamp
moo.xml.stamp: $(docparser_files) $(source_files) Makefile
	$(PYTHON) $(srcdir)/parsedocs.py \
		--source-dir $(top_srcdir)/moo/mooapp \
		--source-dir $(top_srcdir)/moo/mooedit \
		--source-dir $(top_srcdir)/moo/moofileview \
		--source-dir $(top_srcdir)/moo/mooutils \
		--source-dir $(top_srcdir)/moo/plugins/usertools \
		--source-dir $(top_srcdir)/moo/plugins/support \
                --skip 'moofontsel.*' \
		--output moo.xml.tmp && \
	(cmp -s moo.xml.tmp $(srcdir)/moo.xml || mv moo.xml.tmp $(srcdir)/moo.xml)
	rm -f moo.xml.tmp
	echo stamp > moo.xml.stamp

BUILT_SOURCES += moo.defs.stamp
moo.defs.stamp: $(mpi_files) moo.xml Makefile
	$(PYTHON) $(srcdir)/gendefs.py $(srcdir)/moo.xml > moo.defs.tmp && \
	(cmp -s moo.defs.tmp $(srcdir)/moo.defs || mv moo.defs.tmp $(srcdir)/moo.defs)
	rm -f moo.defs.tmp
	echo stamp > moo.defs.stamp

endif
