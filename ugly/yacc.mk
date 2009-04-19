# -%- lang: makefile; indent-width: 8; use-tabs: true -%-
# UGLY_MK_TRIGGER = YACC_FILES|YACC_PP_FILES

# $(YACC_FILES) should be a list of .y files
# $(YACC_PP_FILES) should be a list of .y files

# bison_verbose = -v

$(srcdir)/%-yacc.h: $(srcdir)/%.y
	touch $@
$(srcdir)/%-yacc.c: $(srcdir)/%.y
	bison $(bison_verbose) -o $@ -d $<
$(srcdir)/%-yacc.cpp: $(srcdir)/%.y
	bison $(bison_verbose) -o $@ -d $< && \
	mv $(srcdir)/$*-yacc.hpp $(srcdir)/$*-yacc.h

ugly_yacc_sources =			\
    $(YACC_FILES:.y=-yacc.c)		\
    $(YACC_FILES:.y=-yacc.h)		\
    $(YACC_PP_FILES:.y=-yacc.cpp)	\
    $(YACC_PP_FILES:.y=-yacc.h)

BUILT_SOURCES += $(ugly_yacc_sources)
EXTRA_DIST += $(YACC_FILES) $(YACC_PP_FILES)
@MODULE@_SOURCES += $(ugly_yacc_sources)
