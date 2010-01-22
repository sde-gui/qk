SET(LANGS2
  mooedit/langs/ada.lang
  mooedit/langs/asp.lang
  mooedit/langs/awk.lang
  mooedit/langs/boo.lang
  mooedit/langs/changelog.lang
  mooedit/langs/chdr.lang
  mooedit/langs/c.lang
  mooedit/langs/cmake.lang
  mooedit/langs/cpp.lang
  mooedit/langs/csharp.lang
  mooedit/langs/css.lang
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
  mooedit/langs/forth.lang
  mooedit/langs/fortran.lang
  mooedit/langs/gap.lang
  mooedit/langs/gtk-doc.lang
  mooedit/langs/gtkrc.lang
  mooedit/langs/haddock.lang
  mooedit/langs/haskell.lang
  mooedit/langs/haskell-literate.lang
  mooedit/langs/html.lang
  mooedit/langs/idl.lang
  mooedit/langs/ini.lang
  mooedit/langs/java.lang
  mooedit/langs/javascript.lang
  mooedit/langs/latex.lang
  mooedit/langs/libtool.lang
  mooedit/langs/lua.lang
  mooedit/langs/m4.lang
  mooedit/langs/makefile.lang
  mooedit/langs/nemerle.lang
  mooedit/langs/nsis.lang
  mooedit/langs/objc.lang
  mooedit/langs/ocaml.lang
  mooedit/langs/ocl.lang
  mooedit/langs/octave.lang
  mooedit/langs/pascal.lang
  mooedit/langs/perl.lang
  mooedit/langs/php.lang
  mooedit/langs/pkgconfig.lang
  mooedit/langs/po.lang
  mooedit/langs/prolog.lang
  mooedit/langs/python-console.lang
  mooedit/langs/python.lang
  mooedit/langs/R.lang
  mooedit/langs/rpmspec.lang
  mooedit/langs/ruby.lang
  mooedit/langs/scheme.lang
  mooedit/langs/sh.lang
  mooedit/langs/sql.lang
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
)

SET(LANGS1
  mooedit/langs/msil.lang
)

SET(STYLES
  mooedit/langs/kate.xml
  mooedit/langs/classic.xml
  mooedit/langs/cobalt.xml
  mooedit/langs/oblivion.xml
  mooedit/langs/tango.xml
)

INSTALL(FILES mooedit/langs/language2.rng mooedit/langs/check.sh ${STYLES} ${LANGS1} ${LANGS2} DESTINATION ${MOO_TEXT_LANG_FILES_DIR})

# EXTRA_DIST +=
# 	$(languagespecs_DATA)
# 	langs/styles.rng

# -%- strip:true -%-
