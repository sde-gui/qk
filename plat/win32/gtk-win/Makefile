top:
	@echo "Do you want 'build' target?"
	@exit 1

check-make:
	@if [ ! -e $(mgwdir)/mjhbuild.sh ]; then echo "use make.sh"; exit 1; fi

build-gtk: check-make
	$(mgwdir)/mjhbuild.sh build

build-data: check-make
	mkdir -p $(mgwtargetdir)
	cp -flR $(mgwdir)/extra/medit-data/* $(mgwtargetdir)/
# 	cp -flR $(mgwdir)/extra/medit-data/etc/* $(mgwtargetdir)/etc/

build-python: check-make
	mkdir -p $(mgwtargetdir)/bin $(mgwtargetdir)/lib $(mgwtargetdir)/include/python
# 	cp -fl $(mgwpythoninstdir)/*.exe $(mgwtargetdir)/bin/
	cp -fl $(mgwpythonsystem32dir)/python2*.dll $(mgwtargetdir)/bin/
# 	cp -flR $(mgwpythoninstdir)/DLLs $(mgwpythoninstdir)/Lib $(mgwtargetdir)/bin/
	cp -fl $(mgwpythoninstdir)/libs/python2*.lib $(mgwtargetdir)/lib/
	cp -fl $(mgwpythoninstdir)/libs/libpython2*.a $(mgwtargetdir)/lib/
	cp -fl $(mgwpythoninstdir)/include/* $(mgwtargetdir)/include/python/

build: build-python build-gtk build-data
rebuild: clean build

bdist-en:
	rm -fr $(mgwdestdir)-en
	$(mgwdir)/copyuser.sh --en $(mgwtargetdir) $(mgwdestdir)-en

bdist:
	rm -fr $(mgwdestdir)
	$(mgwdir)/copyuser.sh $(mgwtargetdir) $(mgwdestdir)

bdist-all: bdist-en bdist

clean:
	rm -rf $(mgwbuilddir) $(mgwdestdir) $(mgwdestdir)-en
