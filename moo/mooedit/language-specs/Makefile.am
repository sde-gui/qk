languagespecsdir = ${MOO_TEXT_LANG_FILES_DIR}

langs2 =			\
	ada.lang		\
	changelog.lang		\
	c.lang			\
	cpp.lang		\
	csharp.lang		\
	css.lang		\
	def.lang		\
	desktop.lang		\
	diff.lang		\
	dpatch.lang		\
	dtd.lang		\
	fortran.lang		\
	gap.lang		\
	gtk-doc.lang		\
	gtkrc.lang		\
	haskell.lang		\
	html.lang		\
	idl.lang		\
	ini.lang		\
	java.lang		\
	javascript.lang		\
	latex.lang		\
	libtool.lang		\
	m4.lang			\
	makefile.lang		\
	ms.lang			\
	octave.lang		\
	pascal.lang		\
	perl.lang		\
	php.lang		\
	po.lang			\
	python-console.lang	\
	python.lang		\
	ruby.lang		\
	scheme.lang		\
	sh.lang			\
	sql.lang		\
	tcl.lang		\
	texinfo.lang		\
	xml.lang		\
	yacc.lang

langs1 =			\
	boo.lang		\
	check.sh		\
	d.lang			\
	lua.lang		\
	msil.lang		\
	nemerle.lang		\
	R.lang			\
	vbnet.lang		\
	verilog.lang		\
	vhdl.lang

styles =			\
	kate.styles		\
	gvim.styles

languagespecs_DATA =		\
	language.dtd		\
	language2.rng		\
	check.sh		\
	$(styles)		\
	$(langs1)		\
	$(langs2)

packagesdir = $(mimedir)/packages
packages_DATA = moo.xml
update_mime = update-mime-database $(DESTDIR)${mimedir}
@INTLTOOL_XML_RULE@

install-data-hook:
	if echo "Updating mime database... " && $(update_mime); then		\
		echo "Done.";							\
	else									\
		echo "*** ";							\
		echo "*** Mime database not updated. After install, run this:";	\
		echo $(update_mime);						\
		echo "*** ";							\
	fi
uninstall-hook:
	echo "Updating mime database" && $(update_mime) && echo "Done."

EXTRA_DIST =			\
	$(languagespecs_DATA)	\
	moo.xml.in
CLEANFILES =			\
	moo.xml
