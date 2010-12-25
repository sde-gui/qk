BUILT_SOURCES =

if MOO_DEV_MODE

# docbook_files =			\
# 	medit.docbook		\
# 	prefs.docbook
#
# medit.docbook: medit.docbook.in $(top_builddir)/config.status
# 	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/medit.docbook
# 	$(AM_V_at)mv medit.docbook $(srcdir)/medit.docbook
#
# medit.html: $(docbook_files)
# 	$(AM_V_GEN)xsltproc /usr/share/xml/docbook/stylesheet/docbook-xsl/html/docbook.xsl $(srcdir)/medit.docbook > medit.html.tmp && \
# 		mv medit.html.tmp $(srcdir)/medit.html

src_texinfo_files =		\
	medit.texi		\
	prefs.texi		\
	prefs-file.texi		\
	user-tools.texi		\
	regex.texi		\
	license.texi		\
	script.texi

medit.texi: medit.texi.in $(top_builddir)/config.status
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/medit.texi
	$(AM_V_at)mv medit.texi $(srcdir)/medit.texi

gendocbook_files =				\
	$(top_srcdir)/api/gendocbook.py		\
	$(top_srcdir)/api/mpi/__init__.py	\
	$(top_srcdir)/api/mpi/module.py		\
	$(top_srcdir)/api/mpi/docbookwriter.py

script-python.docbook: $(gendocbook_files) $(top_srcdir)/api/moo.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py --python $(top_srcdir)/api/moo.xml \
		> script-python.docbook.tmp && mv script-python.docbook.tmp $(srcdir)/script-python.docbook

script-lua.docbook: $(gendocbook_files) $(top_srcdir)/api/moo.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py --lua $(top_srcdir)/api/moo.xml \
		> script-lua.docbook.tmp && mv script-lua.docbook.tmp $(srcdir)/script-lua.docbook

script-lua-gtk.docbook: $(gendocbook_files) $(top_srcdir)/moo/moolua/gtk.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py --lua $(top_srcdir)/moo/moolua/gtk.xml \
		> script-lua-gtk.docbook.tmp && mv script-lua-gtk.docbook.tmp $(srcdir)/script-lua-gtk.docbook

help/script-python.html: script-python.docbook script.xsl help/index.html
	$(AM_V_GEN)xsltproc --output script-python.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-python.docbook \
		&& mv script-python.html.tmp $(srcdir)/help/script-python.html

help/script-lua.html: script-lua.docbook script.xsl help/index.html
	$(AM_V_GEN)xsltproc --output script-lua.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-lua.docbook \
		&& mv script-lua.html.tmp $(srcdir)/help/script-lua.html

help/script-lua-gtk.html: script-lua-gtk.docbook script.xsl help/index.html
	$(AM_V_GEN)xsltproc --output script-lua-gtk.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-lua-gtk.docbook \
		&& mv script-lua-gtk.html.tmp $(srcdir)/help/script-lua-gtk.html

help/script.css: script.css
	$(AM_V_GEN)cp $(srcdir)/script.css $(srcdir)/help/

help/index.html: $(src_texinfo_files)
	$(AM_V_at)rm -f $(srcdir)/help/*.html
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help
	$(AM_V_GEN)cd $(srcdir) && makeinfo --html medit.texi

help/help.html: help/index.html
	$(AM_V_GEN)cd $(srcdir) && makeinfo --html --no-split medit.texi && mv help.html help/

help-sections.h.stamp: help/index.html $(srcdir)/gensections.py
	$(AM_V_GEN)$(PYTHON) $(srcdir)/gensections.py $(srcdir)/help/*.html > help-sections.h.tmp && \
		(cmp -s help-sections.h.tmp $(srcdir)/help-sections.h || mv help-sections.h.tmp $(srcdir)/help-sections.h)
	$(AM_V_at)rm -f help-sections.h.tmp
	$(AM_V_at)echo stamp > help-sections.h.stamp

all-am: doc
doc:					\
	help/index.html			\
	help/help.html			\
	help/script-lua.html		\
	help/script-lua-gtk.html	\
	help/script-python.html		\
	help/script.css			\
	help-sections.h.stamp		\
	medit.1

man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/man-medit.t2t
	$(AM_V_at)mv man-medit.t2t $(srcdir)/man-medit.t2t

medit.1: man-medit.t2t
	$(AM_V_GEN)txt2tags --target=man --outfile=- $(srcdir)/man-medit.t2t | grep -v "cmdline: txt2tags" \
		> medit.1.tmp && mv medit.1.tmp $(srcdir)/medit.1

endif

EXTRA_DIST = help help-sections.h medit.1

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(MOO_HELP_DIR)
	cd $(srcdir) && $(INSTALL_DATA) help/*.html $(DESTDIR)$(MOO_HELP_DIR)

uninstall-local:
	rm -f $(DESTDIR)$(MOO_HELP_DIR)/*.html

if MOO_OS_UNIX
dist_man_MANS = medit.1
endif MOO_OS_UNIX
