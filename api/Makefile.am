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
	moo.defs		\
	moo-lua.cpp		\
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

BUILT_SOURCES += moo.defs.stamp
moo.defs.stamp: $(gendefs_files) moo.xml Makefile
	$(PYTHON) $(srcdir)/gendefs.py $(srcdir)/moo.xml > moo.defs.tmp && \
	(cmp -s moo.defs.tmp $(srcdir)/moo.defs || mv moo.defs.tmp $(srcdir)/moo.defs)
	rm -f moo.defs.tmp
	echo stamp > moo.defs.stamp

BUILT_SOURCES += moo-lua.cpp.stamp
moo-lua.cpp.stamp: $(genlua_files) moo.xml Makefile
	$(PYTHON) $(srcdir)/genlua.py $(srcdir)/moo.xml > moo-lua.cpp.tmp && \
	(cmp -s moo-lua.cpp.tmp $(srcdir)/moo-lua.cpp || mv moo-lua.cpp.tmp $(srcdir)/moo-lua.cpp)
	rm -f moo-lua.cpp.tmp
	echo stamp > moo-lua.cpp.stamp

endif
