docparser_files =		\
	parsedocs.py		\
	mdp/__init__.py		\
	mdp/module.py		\
	mdp/docparser.py	\
	mdp/xmlwriter.py

gendefs_files =			\
	gendefs.py		\
	mpi/__init__.py		\
	mpi/module.py		\
	mpi/defswriter.py

genlua_files =			\
	genlua.py		\
	mpi/__init__.py		\
	mpi/module.py		\
	mpi/luawriter.py

EXTRA_DIST =			\
	$(docparser_files)	\
	$(gendefs_files)	\
	$(genlua_files)		\
	sourcefiles.mak		\
	moo.xml

BUILT_SOURCES =

if MOO_DEV_MODE

include sourcefiles.mak

BUILT_SOURCES += moo.xml.stamp
moo.xml.stamp: $(docparser_files) $(source_files)
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

endif
