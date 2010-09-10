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

build: build-gtk build-data
