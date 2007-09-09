TESTS = check.sh

languagespecsdir = ${MOO_TEXT_LANG_FILES_DIR}

langs2 =			\
	ada.lang		\
	awk.lang		\
	boo.lang		\
	changelog.lang		\
	chdr.lang		\
	c.lang			\
	cpp.lang		\
	csharp.lang		\
	css.lang		\
	def.lang		\
	desktop.lang		\
	diff.lang		\
	d.lang			\
	docbook.lang		\
	dpatch.lang		\
	dtd.lang		\
	erlang.lang		\
	fortran.lang		\
	gap.lang		\
	gtk-doc.lang		\
	gtkrc.lang		\
	haddock.lang		\
	haskell.lang		\
	haskell-literate.lang	\
	html.lang		\
	idl.lang		\
	ini.lang		\
	java.lang		\
	javascript.lang		\
	latex.lang		\
	libtool.lang		\
	lua.lang		\
	m4.lang			\
	makefile.lang		\
	nemerle.lang		\
	objc.lang		\
	ocaml.lang		\
	ocl.lang		\
	octave.lang		\
	pascal.lang		\
	perl.lang		\
	php.lang		\
	pkgconfig.lang		\
	po.lang			\
	python-console.lang	\
	python.lang		\
	R.lang			\
	rpmspec.lang		\
	ruby.lang		\
	scheme.lang		\
	sh.lang			\
	sql.lang		\
	tcl.lang		\
	texinfo.lang		\
	vala.lang		\
	vbnet.lang		\
	verilog.lang		\
	vhdl.lang		\
	xml.lang		\
	yacc.lang

langs1 =			\
	msil.lang

styles =			\
	kate.xml		\
	classic.xml		\
	oblivion.xml		\
	tango.xml

languagespecs_DATA =		\
	language2.rng		\
	check.sh		\
	$(styles)		\
	$(langs1)		\
	$(langs2)

packagesdir = $(mimedir)/packages
packages_DATA = moo.xml
update_mime = update-mime-database $(DESTDIR)${mimedir}
@INTLTOOL_XML_RULE@

if !MOO_OS_MINGW
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
else MOO_OS_MINGW
install-data-hook:
	cp /usr/share/mime/packages/freedesktop.org.xml $(DESTDIR)${mimedir}/packages/ && \
	$(update_mime)
endif

EXTRA_DIST =			\
	$(languagespecs_DATA)	\
	moo.xml.in		\
	styles.rng
CLEANFILES =			\
	moo.xml
