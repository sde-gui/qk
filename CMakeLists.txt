cmake_minimum_required(VERSION 2.8)

list(APPEND EXTRA_DIST
    tools/genenums.py
    tools/glade2c.py
    tools/xml2h.py
    plat/win32/installer.iss.in
    plat/win32/istrans.sh
    plat/win32/mingw-configure
)

# zzz
# if MOO_OS_WIN32
# plat/win32/installer.iss: $(top_srcdir)/plat/win32/installer.iss.in $(top_builddir)/config.status
# 	$(AM_V_at)$(MKDIR_P) plat/win32
# 	$(AM_V_GEN)cd $(top_builddir) && ./config.status --silent --file=plat/win32/installer.iss
# installer: plat/win32/installer.iss install
# 	$(MEDIT_INNO_COMPILER) plat/win32/installer.iss
# copy:
# 	cp moo/medit.exe "/win/Program Files (x86)/medit/bin/"
# bd_tmpdir = medit-win-dist-tmp
# bd_distdir = $(PACKAGE)-$(VERSION)
# bd_pzip = $(PACKAGE)-portable-$(VERSION)
# bd_zip = $(PACKAGE)-$(VERSION)
# portable: install
# 	rm -fr $(bd_tmpdir)
# 	mkdir $(bd_tmpdir)
# 	cp -lR ${prefix} $(bd_tmpdir)/$(bd_distdir)
# 	cd $(bd_tmpdir) && zip -r9 $(bd_zip).zip $(bd_distdir)
# 	echo "This file enables portable mode for medit" > $(bd_tmpdir)/$(bd_distdir)/bin/$(MEDIT_PORTABLE_MAGIC_FILE_NAME)
# 	cd $(bd_tmpdir) && zip -r9 $(bd_pzip).zip $(bd_distdir)
# 	mv $(bd_tmpdir)/$(bd_pzip).zip $(bd_tmpdir)/$(bd_zip).zip .
# 	rm -fr $(bd_tmpdir)
# bdist: installer portable
# endif

# zzz @MOO_PO_SUBDIRS_RULE@

project(medit)

if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset the configurations to what we need" FORCE)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(GTK2 2.6 REQUIRED gtk)

# zzz SUBDIRS = po po-gsv api doc moo
add_subdirectory(moo)
