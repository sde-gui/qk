# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

# UGLY_MK_TRIGGER = PCH_HEADER

EXTRA_DIST += $(PCH_HEADER)

ugly_base_compile_c = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
ugly_base_compile_cxx = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

COMPILE = $(ugly_base_compile_c)
CXXCOMPILE = $(ugly_base_compile_cxx)

if MOO_ENABLE_PCH

BUILT_SOURCES += $(ugly_pch_files)
CLEANFILES += $(ugly_pch_files)

ugly_pch_name = precompiled-header-name
ugly_pch_files = $(ugly_pch_name)-c.gch
ugly_pch_mk = $(top_srcdir)/ugly/pch.mk

if MOO_USE_CPP
ugly_pch_files += $(ugly_pch_name)-c++.gch
endif

$(ugly_pch_name)-c.gch: $(PCH_HEADER) $(ugly_pch_mk)
	$(ugly_base_compile_c) -x c-header -c $< -o $@
$(ugly_pch_name)-c++.gch: $(PCH_HEADER) $(ugly_pch_mk)
	$(ugly_base_compile_cxx) -x c++-header -c $< -o $@

COMPILE += -include $(ugly_pch_name)-c -Winvalid-pch
CXXCOMPILE += -include $(ugly_pch_name)-c++ -Winvalid-pch

endif
