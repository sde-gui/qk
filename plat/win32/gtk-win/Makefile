top:
	@echo "What target do you want?"
	@exit 1

check-make:
	@if [ ! -e $(mgwdir)/mjhbuild.sh ]; then echo "use make.sh"; exit 1; fi

build-gtk: check-make
	$(mgwdir)/mjhbuild.sh build

build-data: check-make
	mkdir -p $(mgwtargetdir)
	cp -R $(mgwdir)/extra/medit-data/* $(mgwtargetdir)/

build-python: check-make
	mkdir -p $(mgwtargetdir)/bin $(mgwtargetdir)/lib $(mgwtargetdir)/include/python
	cp -f -l $(mgwpythoninstdir)/*.exe $(mgwtargetdir)/bin/
	cp -f -l $(mgwpythonsystem32dir)/python2*.dll $(mgwtargetdir)/bin/
	cp -f -l -R $(mgwpythoninstdir)/DLLs $(mgwpythoninstdir)/Lib $(mgwtargetdir)/bin/
	cp -f -l $(mgwpythoninstdir)/libs/python2*.lib $(mgwtargetdir)/lib/
	cp -f -l $(mgwpythoninstdir)/libs/libpython2*.a $(mgwtargetdir)/lib/
	cp -f -l $(mgwpythoninstdir)/include/* $(mgwtargetdir)/include/python/

build: build-gtk build-data

bdist-en:
	rm -fr $(mgwdestdir)-en
	$(mgwdir)/copyuser.sh --en $(mgwtargetdir) $(mgwdestdir)-en

bdist:
	rm -fr $(mgwdestdir)
	$(mgwdir)/copyuser.sh $(mgwtargetdir) $(mgwdestdir)

bdist-all: bdist-en bdist

clean:
	rm -rf $(mgwbuilddir) $(mgwdestdir) $(mgwdestdir)-en
