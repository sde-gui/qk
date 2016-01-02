LIST(APPEND moo_edit_enum_headers mooedit/mooedit-enums.h)

SET(mooedit_sources
    mooedit/mooedit.cmake
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
    mooedit/mooeditaction-factory.cpp
    mooedit/mooeditaction-factory.h
    mooedit/mooeditbookmark.cpp
    mooedit/mooedit.cpp
    mooedit/mooeditconfig.c	
    mooedit/mooeditdialogs.cpp
    mooedit/mooedit-enum-types.c
    mooedit/mooedit-enum-types.h
    mooedit/mooedit-enums.h	
    mooedit/mooedit-fileops.cpp
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
    mooedit/mooeditfileinfo.cpp
    mooedit/moofold.c	
    mooedit/moofold.h	
    mooedit/mooindenter.c	
    mooedit/mooindenter.h	
    mooedit/moolang.c	
    mooedit/moolang.h	
    mooedit/moolangmgr.cpp	
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
    mooedit/mooeditor.cpp
    mooedit/mooeditor-impl.h
    mooedit/mooeditor-private.h
    mooedit/mooeditor-tests.c
    mooedit/mooeditor-tests.h
    mooedit/mooeditor-tests.h
)

# SET(built_mooedit_sources mooedit/mooedit-enum-types.h.stamp mooedit/mooedit-enum-types.c.stamp)

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

install(FILES
    mooedit/langs/actionscript.lang
    mooedit/langs/ada.lang
    mooedit/langs/asp.lang
    mooedit/langs/automake.lang
    mooedit/langs/awk.lang
    mooedit/langs/bennugd.lang
    mooedit/langs/bibtex.lang
    mooedit/langs/bluespec.lang
    mooedit/langs/boo.lang
    mooedit/langs/cg.lang
    mooedit/langs/changelog.lang
    mooedit/langs/chdr.lang
    mooedit/langs/c.lang
    mooedit/langs/cmake.lang
    mooedit/langs/cobol.lang
    mooedit/langs/cpp.lang
    mooedit/langs/csharp.lang
    mooedit/langs/css.lang
    mooedit/langs/cuda.lang
    mooedit/langs/def.lang
    mooedit/langs/desktop.lang
    mooedit/langs/diff.lang
    mooedit/langs/d.lang
    mooedit/langs/docbook.lang
    mooedit/langs/dosbatch.lang
    mooedit/langs/dot.lang
    mooedit/langs/dpatch.lang
    mooedit/langs/dtd.lang
    mooedit/langs/eiffel.lang
    mooedit/langs/erlang.lang
    mooedit/langs/fcl.lang
    mooedit/langs/forth.lang
    mooedit/langs/fortran.lang
    mooedit/langs/fsharp.lang
    mooedit/langs/gap.lang
    mooedit/langs/gdb-log.lang
    mooedit/langs/glsl.lang
    mooedit/langs/go.lang
    mooedit/langs/gtk-doc.lang
    mooedit/langs/gtkrc.lang
    mooedit/langs/haddock.lang
    mooedit/langs/haskell.lang
    mooedit/langs/haskell-literate.lang
    mooedit/langs/html.lang
    mooedit/langs/idl-exelis.lang
    mooedit/langs/idl.lang
    mooedit/langs/imagej.lang
    mooedit/langs/ini.lang
    mooedit/langs/java.lang
    mooedit/langs/javascript.lang
    mooedit/langs/j.lang
    mooedit/langs/json.lang
    mooedit/langs/julia.lang
    mooedit/langs/latex.lang
    mooedit/langs/libtool.lang
    mooedit/langs/lua.lang
    mooedit/langs/m4.lang
    mooedit/langs/makefile.lang
    mooedit/langs/mallard.lang
    mooedit/langs/markdown.lang
    mooedit/langs/matlab.lang
    mooedit/langs/mediawiki.lang
    mooedit/langs/modelica.lang
    mooedit/langs/msil.lang
    mooedit/langs/mxml.lang
    mooedit/langs/nemerle.lang
    mooedit/langs/netrexx.lang
    mooedit/langs/nsis.lang
    mooedit/langs/objc.lang
    mooedit/langs/objj.lang
    mooedit/langs/ocaml.lang
    mooedit/langs/ocl.lang
    mooedit/langs/octave.lang
    mooedit/langs/ooc.lang
    mooedit/langs/opal.lang
    mooedit/langs/opencl.lang
    mooedit/langs/pascal.lang
    mooedit/langs/perl.lang
    mooedit/langs/php.lang
    mooedit/langs/pkgconfig.lang
    mooedit/langs/po.lang
    mooedit/langs/prolog.lang
    mooedit/langs/protobuf.lang
    mooedit/langs/puppet.lang
    mooedit/langs/python3.lang
    mooedit/langs/python-console.lang
    mooedit/langs/python.lang
    mooedit/langs/R.lang
    mooedit/langs/rpmspec.lang
    mooedit/langs/ruby.lang
    mooedit/langs/scala.lang
    mooedit/langs/scheme.lang
    mooedit/langs/scilab.lang
    mooedit/langs/sh.lang
    mooedit/langs/sml.lang
    mooedit/langs/sparql.lang
    mooedit/langs/sql.lang
    mooedit/langs/systemverilog.lang
    mooedit/langs/t2t.lang
    mooedit/langs/tcl.lang
    mooedit/langs/texinfo.lang
    mooedit/langs/vala.lang
    mooedit/langs/vbnet.lang
    mooedit/langs/verilog.lang
    mooedit/langs/vhdl.lang
    mooedit/langs/xml.lang
    mooedit/langs/xslt.lang
    mooedit/langs/yacc.lang
    mooedit/langs/kate.xml
    mooedit/langs/classic.xml
    mooedit/langs/cobalt.xml
    mooedit/langs/oblivion.xml
    mooedit/langs/tango.xml
    mooedit/langs/medit.xml
    mooedit/langs/language2.rng
    mooedit/langs/check.sh
DESTINATION ${MOO_TEXT_LANG_FILES_DIR})
