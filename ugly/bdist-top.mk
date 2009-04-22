# -%- lang: makefile; indent-width: 8; use-tabs: true -%-

bdistdir = $(PACKAGE)-$(VERSION)

ug_remove_bdistdir = \
  { test ! -d $(bdistdir) \
    || { find $(bdistdir) -type d ! -perm -200 -exec chmod u+w {} ';' \
         && rm -fr $(bdistdir); }; }

bdist-zip: bdistdir
	rm -f $(bdistdir).zip
	zip -9 -r $(bdistdir).zip $(bdistdir)
	$(ug_remove_bdistdir)

bdistdir:
	$(ug_remove_bdistdir)
	test -d $(bdistdir) || mkdir $(bdistdir)
	bdistdir=`$(am__cd) $(bdistdir) && pwd`; \
	$(MAKE) $(AM_MAKEFLAGS) bdistdir="$$bdistdir" bdistsubdir
	-find $(bdistdir) -type d ! -perm -777 -exec chmod a+rwx {} \; -o \
	  ! -type d ! -perm -444 -links 1 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -400 -exec chmod a+r {} \; -o \
	  ! -type d ! -perm -444 -exec $(install_sh) -c -m a+r {} {} \; \
	|| chmod -R a+r $(bdistdir)
