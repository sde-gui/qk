# -*- makefile -*-

# $(yacc_files) should be a list of .y files
# $(yacc_sources) is defined to be a list of generated *-yacc.[ch] files

# uncomment to get .output file
# bison_verbose = -v

%-yacc.h: %-yacc.y
	touch $(srcdir)/$@
%-yacc.c: %-yacc.y
	bison $(bison_verbose) -o $(srcdir)/$@ -d $<

yacc_sources = $(patsubst %-yacc.y,%-yacc.h,$(yacc_files)) $(patsubst %-yacc.y,%-yacc.c,$(yacc_files))
BUILT_SOURCES += $(yacc_sources)
EXTRA_DIST += $(yacc_files)
