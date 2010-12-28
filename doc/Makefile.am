BUILT_SOURCES =

if MOO_DEV_MODE

$(srcdir)/medit-defines.ent: medit-defines.ent.in $(top_builddir)/config.status
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/medit-defines.ent
	$(AM_V_at)$(MOO_MV_TO_SRCDIR) medit-defines.ent $(srcdir)/medit-defines.ent

gendocbook_files =				\
	$(top_srcdir)/api/gendocbook.py		\
	$(top_srcdir)/api/mpi/__init__.py	\
	$(top_srcdir)/api/mpi/module.py		\
	$(top_srcdir)/api/mpi/docbookwriter.py

$(srcdir)/script-python.docbook: $(gendocbook_files) script-python.tmpl.docbook $(top_srcdir)/api/moo.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py \
		--python --template $(srcdir)/script-python.tmpl.docbook \
		$(top_srcdir)/api/moo.xml > script-python.docbook.tmp && \
		mv script-python.docbook.tmp $(srcdir)/script-python.docbook

$(srcdir)/script-lua.docbook: $(gendocbook_files) script-lua.tmpl.docbook $(top_srcdir)/api/moo.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py \
		--lua --template $(srcdir)/script-lua.tmpl.docbook \
		$(top_srcdir)/api/moo.xml > script-lua.docbook.tmp && \
		mv script-lua.docbook.tmp $(srcdir)/script-lua.docbook

$(srcdir)/script-lua-gtk.docbook: $(gendocbook_files) script-lua-gtk.tmpl.docbook $(top_srcdir)/api/gtk.xml
	$(AM_V_GEN)$(PYTHON) $(top_srcdir)/api/gendocbook.py \
		--lua --template $(srcdir)/script-lua-gtk.tmpl.docbook \
		$(top_srcdir)/api/gtk.xml > script-lua-gtk.docbook.tmp && \
		mv script-lua-gtk.docbook.tmp $(srcdir)/script-lua-gtk.docbook

$(srcdir)/help/script-python.html: script-python.docbook medit-defines.ent script.xsl
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/
	$(AM_V_GEN)xsltproc --output script-python.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-python.docbook \
		&& mv script-python.html.tmp $(srcdir)/help/script-python.html

$(srcdir)/help/script-lua.html: script-lua.docbook medit-defines.ent script.xsl
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/
	$(AM_V_GEN)xsltproc --output script-lua.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-lua.docbook \
		&& mv script-lua.html.tmp $(srcdir)/help/script-lua.html

$(srcdir)/help/script-lua-gtk.html: script-lua-gtk.docbook medit-defines.ent script.xsl
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/
	$(AM_V_GEN)xsltproc --output script-lua-gtk.html.tmp \
		$(srcdir)/script.xsl $(srcdir)/script-lua-gtk.docbook \
		&& mv script-lua-gtk.html.tmp $(srcdir)/help/script-lua-gtk.html

$(srcdir)/help/medit.css: medit.css
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/
	$(AM_V_GEN)cp $(srcdir)/medit.css $(srcdir)/help/

$(srcdir)/lgpl.no-fancy-chars: $(top_srcdir)/COPYING
	tr -d '\014' < $(top_srcdir)/COPYING > lgpl.no-fancy-chars.tmp && \
		 mv lgpl.no-fancy-chars.tmp $(srcdir)/lgpl.no-fancy-chars

docbook_files =				\
	medit.docbook			\
	prefs.docbook			\
	regex.docbook			\
	user-tools.docbook		\
	license.docbook

docbook_sources =			\
	$(docbook_files)		\
	lgpl.no-fancy-chars		\
	medit-defines.ent

$(srcdir)/help/index.html: $(docbook_sources) medit.xsl medit-common.xsl
	$(AM_V_GEN)cd $(srcdir) && xsltproc --xinclude medit.xsl medit.docbook

$(srcdir)/help/help.html: $(docbook_sources) medit-single.xsl medit-common.xsl
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/
	$(AM_V_GEN)xsltproc --xinclude --output help.html.tmp $(srcdir)/medit-single.xsl $(srcdir)/medit.docbook && \
		mv help.html.tmp $(srcdir)/help/help.html

toc.xml: $(docbook_sources)
	$(AM_V_GEN)xsltproc --output toc.xml --xinclude \
		--stringparam chunk.first.sections 1 \
		http://docbook.sourceforge.net/release/xsl/current/html/maketoc.xsl $(srcdir)/medit.docbook

moo-help-sections.h.stamp: $(docbook_files) toc.xml genhelpsectionsh.py
	$(AM_V_GEN)$(PYTHON) $(srcdir)/genhelpsectionsh.py --toc=toc.xml --srcdir=$(srcdir) $(docbook_files) > moo-help-sections.h.tmp
	$(AM_V_at)cmp -s moo-help-sections.h.tmp $(srcdir)/moo-help-sections.h || mv moo-help-sections.h.tmp $(srcdir)/moo-help-sections.h
	$(AM_V_at)rm -f moo-help-sections.h.tmp
	$(AM_V_at)echo stamp > moo-help-sections.h.stamp

png_files = prefs-file-filters.png prefs-file-selector.png
dest_png_files = $(addprefix $(srcdir)/help/img/,$(png_files))

$(srcdir)/help/img/%.png: img/%.png
	$(AM_V_at)$(MKDIR_P) $(srcdir)/help/img/
	$(AM_V_GEN)cp $(srcdir)/img/$*.png $(srcdir)/help/img/

all-am: doc
doc:						\
	$(srcdir)/help/index.html		\
	$(srcdir)/help/help.html		\
	$(srcdir)/help/script-lua.html		\
	$(srcdir)/help/script-lua-gtk.html	\
	$(srcdir)/help/script-python.html	\
	$(srcdir)/help/medit.css		\
	$(srcdir)/medit.1			\
	$(dest_png_files)			\
	moo-help-sections.h.stamp

man-medit.t2t: man-medit.t2t.in $(top_builddir)/config.status
	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=doc/man-medit.t2t
	$(AM_V_at)$(MOO_MV_TO_SRCDIR) man-medit.t2t $(srcdir)/man-medit.t2t

medit.1: man-medit.t2t
	$(AM_V_GEN)txt2tags --target=man --outfile=- $(srcdir)/man-medit.t2t | grep -v "cmdline: txt2tags" \
		> medit.1.tmp && mv medit.1.tmp $(srcdir)/medit.1

endif

EXTRA_DIST = help moo-help-sections.h medit.1

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(MOO_HELP_DIR)/img
	cd $(srcdir) && $(INSTALL_DATA) help/*.html help/*.css $(DESTDIR)$(MOO_HELP_DIR)
	cd $(srcdir) && $(INSTALL_DATA) help/img/*.png $(DESTDIR)$(MOO_HELP_DIR)/img

uninstall-local:
	rm -f $(DESTDIR)$(MOO_HELP_DIR)/*.html $(DESTDIR)$(MOO_HELP_DIR)/*.css $(DESTDIR)$(MOO_HELP_DIR)/img/*.png

if MOO_OS_UNIX
dist_man_MANS = medit.1
endif MOO_OS_UNIX
