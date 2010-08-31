ACLOCAL_AMFLAGS = -I m4 $(ACLOCAL_FLAGS)

SUBDIRS = po po-gsv doc moo plugins medit

unix_scripts =				\
	plat/unix/xdg-open		\
	plat/unix/xdg-email		\
	plat/unix/moo-open-html-help
EXTRA_DIST = $(unix_scripts)
if MOO_OS_UNIX
scriptsdir = $(MOO_DATA_DIR)/scripts
scripts_DATA = $(unix_scripts)
endif MOO_OS_UNIX

EXTRA_DIST +=				\
	tools/genenums.py		\
	tools/glade2c.py		\
	tools/rules.mk			\
	tools/xml2h.py
