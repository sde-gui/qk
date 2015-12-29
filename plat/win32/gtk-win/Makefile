top:
	@echo "Do you want 'build' target?"
	@exit 1

check-make:
	@if [ ! -e $(mgwdir)/mjhbuild.sh ]; then echo "use make.sh"; exit 1; fi

build-gtk: check-make
	$(mgwdir)/mjhbuild.sh build

build-data: check-make
	mkdir -p $(mgwtargetdir)/bin
	cp -f $(libgccdll) $(mgwtargetdir)/bin/
	cp -f $(libstdcppdll) $(mgwtargetdir)/bin/

build-medit-data: check-make
	mkdir -p $(mgwtargetdir)
	cp -flR $(mgwdir)/extra/medit-data/* $(mgwtargetdir)/
	mkdir -p $(mgwtargetdir)/share/icons/hicolor/48x48/apps/
	cp -fl $(moosrcdir)/mooutils/pixmaps/medit.png $(mgwtargetdir)/share/icons/hicolor/48x48/apps/
	gtk-update-icon-cache --force $(mgwtargetdir)/share/icons/hicolor

build-python: check-make
	mkdir -p $(mgwtargetdir)/bin $(mgwtargetdir)/lib $(mgwtargetdir)/include/python
# 	cp -fl $(mgwpythoninstdir)/*.exe $(mgwtargetdir)/bin/
	cp -fl $(mgwpythonsystem32dir)/python2*.dll $(mgwtargetdir)/bin/
# 	cp -flR $(mgwpythoninstdir)/DLLs $(mgwpythoninstdir)/Lib $(mgwtargetdir)/bin/
	cp -fl $(mgwpythoninstdir)/libs/python2*.lib $(mgwtargetdir)/lib/
	cp -fl $(mgwpythoninstdir)/libs/libpython2*.a $(mgwtargetdir)/lib/
	cp -fl $(mgwpythoninstdir)/include/* $(mgwtargetdir)/include/python/

build-defs: check-make
	cd $(mgwtargetdir)/lib && gendef ../bin/*.dll

pre-build: build-python
post-build: build-data build-medit-data build-defs

build: pre-build build-gtk post-build
rebuild: clean build

RSYNC = rsync --recursive --verbose --copy-links --update
tmpdir = $(mgwbuildroot)/tmp-$(mgwconfig)$(mgwplatsuffix)
tmpbdist = $(tmpdir)/bdist
copy-win: check-make
	mkdir -p ~/projects/win/$(mgwconfig)$(mgwplatsuffix)
	$(RSYNC) $(mgwtargetdir) ~/projects/win/$(mgwconfig)$(mgwplatsuffix)/
	rm -fr $(tmpbdist)
	mkdir -p $(tmpdir)
	cp -flR $(mgwdestdir) $(tmpbdist)
	$(RSYNC) $(tmpbdist) ~/projects/win/$(mgwconfig)$(mgwplatsuffix)/
	rm -fr $(tmpdir)

bdist-en:
	rm -fr $(mgwdestdir)-en
	$(mgwdir)/copyuser.sh --en $(mgwtargetdir) $(mgwdestdir)-en

bdist:
	rm -fr $(mgwdestdir)
	$(mgwdir)/copyuser.sh $(mgwtargetdir) $(mgwdestdir)

bdist-all: bdist-en bdist

dist-src-dbg:
	$(mgwdir)/copysrcdbg.sh

dist-src:
	$(mgwdir)/copysrc.sh

dist-bin:
	$(mgwdir)/copybin.sh

dist: dist-src dist-bin bdist dist-src-dbg

clean-dist:
	rm -rf $(mgwbuildroot)/dist-$(mgwconfig)$(mgwplatsuffix)

clean: clean-dist
	rm -rf $(mgwbuilddir) $(mgwdestdir) $(mgwdestdir)-en $(mgwdistdir)
