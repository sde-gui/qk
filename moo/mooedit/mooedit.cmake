LIST(APPEND moo_edit_enum_headers mooedit/mooedit-enums.h)

LIST(APPEND moo_sources
    ${moo_edit_enum_headers}
    mooedit/mooeditor.h
    mooedit/mootextview.h
    mooedit/mootextprint.h
    mooedit/mooeditview.h
    mooedit/mooeditaction.h
    mooedit/mooeditbookmark.h
    mooedit/mooeditconfig.h
    mooedit/mooeditdialogs.h
    mooedit/mooeditfiltersettings.h
    mooedit/mooedit.h
    mooedit/mooedithistoryitem.h
    mooedit/mooeditprefs.h
    mooedit/mooedittypes.h
    mooedit/mooeditwindow.h
    mooedit/mooeditfileinfo.h
    mooedit/mooplugin.h
    mooedit/mootextbuffer.h
    mooedit/mootextfind.h
    mooedit/mootextiter.h
    mooedit/mootextsearch.h
    mooedit/mootextstylescheme.h
    mooedit/mooeditview.c	
    mooedit/mooeditview-priv.h
    mooedit/mooeditview-impl.h
    mooedit/mooeditview-script.c
    mooedit/mooeditview-script.h
    mooedit/mooedit-accels.h
    mooedit/mooeditaction.c	
    mooedit/mooeditaction-factory.c
    mooedit/mooeditaction-factory.h
    mooedit/mooeditbookmark.c
    mooedit/mooedit.c	
    mooedit/mooeditconfig.c	
    mooedit/mooeditdialogs.c
    mooedit/mooedit-enum-types.c
    mooedit/mooedit-enum-types.h
    mooedit/mooedit-enums.h	
    mooedit/mooedit-fileops.c
    mooedit/mooedit-fileops.h
    mooedit/mooeditfiltersettings.c
    mooedit/mooedithistoryitem.c
    mooedit/mooedit-impl.h	
    mooedit/mooedit-script.c
    mooedit/mooedit-script.h
    mooedit/mooeditprefs.c	
    mooedit/mooeditprefspage.c
    mooedit/mooedit-private.h
    mooedit/mooeditprogress.h
    mooedit/mooeditprogress.c
    mooedit/mooedittab.c	
    mooedit/mooedittab.h	
    mooedit/mooedittab-impl.h
    mooedit/mooeditwindow.c	
    mooedit/mooeditwindow-impl.h
    mooedit/mooeditfileinfo-impl.h
    mooedit/mooeditfileinfo.c
    mooedit/moofold.c	
    mooedit/moofold.h	
    mooedit/mooindenter.c	
    mooedit/mooindenter.h	
    mooedit/moolang.c	
    mooedit/moolang.h	
    mooedit/moolangmgr.c	
    mooedit/moolangmgr.h	
    mooedit/moolangmgr-private.h
    mooedit/moolang-private.h
    mooedit/moolinebuffer.c	
    mooedit/moolinebuffer.h	
    mooedit/moolinemark.c	
    mooedit/moolinemark.h	
    mooedit/mooplugin.c	
    mooedit/mooplugin-loader.c
    mooedit/mooplugin-loader.h
    mooedit/mooplugin-macro.h
    mooedit/mootextbtree.c	
    mooedit/mootextbtree.h	
    mooedit/mootextbuffer.c	
    mooedit/mootextfind.c	
    mooedit/mootextprint.c	
    mooedit/mootextprint-private.h
    mooedit/mootext-private.h
    mooedit/mootextsearch.c	
    mooedit/mootextsearch-private.h
    mooedit/mootextstylescheme.c
    mooedit/mootextview.c	
    mooedit/mootextview-input.c
    mooedit/mootextview-private.h
    mooedit/mooeditor.c	
    mooedit/mooeditor-impl.h
    mooedit/mooeditor-private.h
    mooedit/mooeditor-tests.c
    mooedit/mooeditor-tests.h
    mooedit/mooeditor-tests.h
)

# LIST(APPEND built_moo_sources mooedit/mooedit-enum-types.h.stamp mooedit/mooedit-enum-types.c.stamp)

# add_custom_command(OUTPUT mooedit/mooedit-enum-types.h.stamp
#     COMMAND ${MOO_PYTHON} ${CMAKE_SOURCE_DIR}/tools/xml2h.py ${CMAKE_CURRENT_SOURCE_DIR}/${input} zzz_ui_xml -o ${_ui_output}
#     COMMAND 
#     $(AM_V_GEN)( cd $(srcdir) && \
#     	$(GLIB_MKENUMS) --template mooedit/mooedit-enum-types.h.tmpl $(moo_edit_enum_headers) ) > mooedit/mooedit-enum-types.h.tmp
#     $(AM_V_at)cmp -s mooedit/mooedit-enum-types.h.tmp $(srcdir)/mooedit/mooedit-enum-types.h || \
#     	mv mooedit/mooedit-enum-types.h.tmp $(srcdir)/mooedit/mooedit-enum-types.h
#     $(AM_V_at)rm -f mooedit/mooedit-enum-types.h.tmp
#     $(AM_V_at)echo stamp > mooedit/mooedit-enum-types.h.stamp
#     MAIN_DEPENDENCY mooedit/mooedit-enum-types.h.tmpl
#     DEPENDS ${moo_edit_enum_headers}
#     COMMENT "Generating ${_ui_output} from ${input}")

# :  Makefile 
# mooedit/mooedit-enum-types.c.stamp: $(moo_edit_enum_headers) Makefile mooedit/mooedit-enum-types.c.tmpl
# 	$(AM_V_at)$(MKDIR_P) mooedit
# 	$(AM_V_GEN)( cd $(srcdir) && \
# 		$(GLIB_MKENUMS) --template mooedit/mooedit-enum-types.c.tmpl $(moo_edit_enum_headers) ) > mooedit/mooedit-enum-types.c.tmp
# 	$(AM_V_at)cmp -s mooedit/mooedit-enum-types.c.tmp $(srcdir)/mooedit/mooedit-enum-types.c || \
# 		mv mooedit/mooedit-enum-types.c.tmp $(srcdir)/mooedit/mooedit-enum-types.c
# 	$(AM_V_at)rm -f mooedit/mooedit-enum-types.c.tmp
# 	$(AM_V_at)echo stamp > mooedit/mooedit-enum-types.c.stamp

# zzz
# include mooedit/langs/Makefile.incl

foreach(input_file
    mooedit/glade/moopluginprefs.glade		
    mooedit/glade/mooeditprefs-view.glade	
    mooedit/glade/mooeditprefs-file.glade	
    mooedit/glade/mooeditprefs-filters.glade	
    mooedit/glade/mooeditprefs-general.glade	
    mooedit/glade/mooeditprefs-langs.glade	
    mooedit/glade/mooeditprogress.glade		
    mooedit/glade/mooeditsavemult.glade		
    mooedit/glade/mootryencoding.glade		
    mooedit/glade/mooprint.glade			
    mooedit/glade/mootextfind.glade		
    mooedit/glade/mootextfind-prompt.glade	
    mooedit/glade/mootextgotoline.glade		
    mooedit/glade/mooquicksearch.glade		
    mooedit/glade/moostatusbar.glade
)
    ADD_GXML(${input_file})
endforeach(input_file)

ADD_UI(mooedit/medit.xml)
ADD_UI(mooedit/mooedit.xml)
