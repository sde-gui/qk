# zzz AM_CPPFLAGS = $(MOO_CPPFLAGS) -I$(top_srcdir)/doc/built -Imooutils/glade

# zzz
# EXTRA_DIST += marshals.list
# built_moo_sources += marshals.h
# marshals.h: marshals.list
# 	$(AM_V_GEN)$(GLIB_GENMARSHAL) --prefix=_moo_marshal --header $(srcdir)/marshals.list > marshals.h.tmp \
# 		&& mv marshals.h.tmp marshals.h

# include mooedit/Makefile.incl
# include xdgmime/Makefile.incl
# include mooutils/Makefile.incl
# include moofileview/Makefile.incl
# include gtksourceview/Makefile.incl
# include eggsmclient/Makefile.incl
# include mooapp/Makefile.incl
# include moolua/Makefile.incl
# include moopython/Makefile.incl
# include plugins/Makefile.incl

SET(MEDIT_SOURCES
    medit-app/mem-debug.h
    medit-app/run-tests.h
    medit-app/parse.h
    medit-app/main.cpp
    ${MOO_SOURCES}
    ${PLUGINS_SOURCES}
)

add_executable(medit WIN32
    ${MEDIT_SOURCES}
)

include_directories(${GTK2_INCLUDE_DIRS})

target_link_libraries(medit
    ${GTK2_LIBRARIES}
    ${MEDIT_LIBS}
)

#zzz
# if MOO_OS_WIN32
# CLEANFILES += medit-app/medit.rc medit-app/medit.res
# medit-app/medit.rc: medit-app/medit.rc.in $(top_builddir)/config.status
# 	$(AM_V_at)$(MKDIR_P) medit-app
# 	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=moo/medit-app/medit.rc
# medit-app/medit.res: medit-app/medit.rc
# 	$(AM_V_GEN)$(WINDRES) -i medit-app/medit.rc --input-format=rc -o medit-app/medit.res -O coff
# medit_LDADD += medit-app/medit.res
# endif

# zzz
# if MOO_OS_UNIX
# desktopdir = $(datadir)/applications
# desktop_DATA = medit-app/medit.desktop
# medit-app/medit.desktop: medit-app/medit.desktop.in @MOO_INTLTOOL_INI_DEPS@
# 	$(AM_V_at)$(MKDIR_P) medit-app
# 	@MOO_INTLTOOL_INI_CMD@
# CLEANFILES += medit-app/medit.desktop
# endif MOO_OS_UNIX

# zzz
# iconthemedir = $(datadir)/icons/hicolor/48x48/apps
# icontheme_DATA = mooutils/pixmaps/medit.png
# if MOO_ENABLE_INSTALL_HOOKS
# update_icon_cache = gtk-update-icon-cache -f -t $(DESTDIR)$(datadir)/icons/hicolor
# install-data-hook: install-data-hook-gtk-icon-cache
# install-data-hook-gtk-icon-cache:
# 	@if echo "Updating icon cache" && $(update_icon_cache); then		\
# 		echo "Done.";							\
# 	else									\
# 		echo "*** GTK icon cache not updated. After install, run this:";\
# 		echo $(update_icon_cache);					\
# 	fi
# uninstall-hook: uninstall-data-hook-gtk-icon-cache
# uninstall-data-hook-gtk-icon-cache:
# 	@if echo "Updating icon cache" && $(update_icon_cache); then echo "Done."; else echo "Failed."; fi
# endif

# zzz include medit-module/Makefile.incl

# zzz
# mooutils/%-gxml.h: mooutils/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# moofileview/%-gxml.h: moofileview/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# mooedit/%-gxml.h: mooedit/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# mooapp/%-gxml.h: mooapp/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# plugins/%-gxml.h: plugins/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# plugins/usertools/%-gxml.h: plugins/usertools/glade/%.glade $(top_srcdir)/tools/glade2c.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/glade2c.py $< > $@.tmp && mv $@.tmp $@
#
# moofileview/%-ui.h: moofileview/%.xml $(top_srcdir)/tools/xml2h.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@
#
# mooedit/%-ui.h: mooedit/%.xml $(top_srcdir)/tools/xml2h.py
# 	$(AM_V_at) $(MKDIR_P) `dirname $@`
# 	$(AM_V_GEN)$(MOO_PYTHON) $(top_srcdir)/tools/xml2h.py $< $@.tmp $*_ui_xml && mv $@.tmp $@
