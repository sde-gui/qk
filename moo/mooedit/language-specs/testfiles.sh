#!/bin/sh

# Langs covered here:
# changelog.lang c.lang cpp.lang desktop.lang diff.lang dtd.lang
# gap.lang gtkrc.lang html.lang ini.lang latex.lang m4.lang
# makefile.lang ms.lang perl.lang po.lang python.lang sh.lang
# texinfo.lang xml.lang yacc.lang libtool.lang pkgconfig.lang

# Langs not covered:
# ada.lang boo.lang csharp.lang css.lang def.lang d.lang gtk-doc.lang
# fortran.lang haskell.lang idl.lang java.lang javascript.lang lua.lang
# msil.lang nemerle.lang octave.lang pascal.lang php.lang python-console.lang
# R.lang ruby.lang scheme.lang sql.lang tcl.lang vbnet.lang verilog.lang
# vhdl.lang dpatch.lang

dir="testdir"
mkdir -p $dir/

cat > $dir/file.cc <<EOFEOF
#include <iostream>
int main ()
{
    std::cout << "Hi there!" << std::endl;
    return 0;
}
EOFEOF

cat > $dir/file.c <<EOFEOF
#include <stdio.h>
int main (void)
{
    printf ("Hi there!\n");
    return 0;
}
EOFEOF

cat > $dir/ChangeLog <<EOFEOF
2006-12-10  Kristian Rietveld  <kris@gtk.org>

	* gtk/gtkcellrenderertext.c (gtk_cell_renderer_text_focus_out_event):
	cancel editing (ie. don't accept changes) when the entry loses
	focus. (Fixes #164494, reported by Chris Rouch).

2006-12-10  Matthias Clasen  <mclasen@redhat.com>

	* configure.in: Correct a misapplied patch.
EOFEOF

cat > $dir/file.g <<EOFEOF
for i in [1..10] do
  Print("blah blah blah\n");
od;
EOFEOF

cat > $dir/file.html <<EOFEOF
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html401/loose.dtd">
<html>
  <head>
    <title>Hi there!</title>
    <meta http-equiv="Content-Type" content="text/html; charset=us-ascii">
    <style type="text/css"><!--
      a.summary-letter {text-decoration: none}
      pre.display {font-family: serif}
      pre.format {font-family: serif}
      pre.menu-comment {font-family: serif}
      pre.menu-preformatted {font-family: serif}
      pre.smalldisplay {font-family: serif; font-size: smaller}
      ul.toc {list-style: none}
    --></style>
  </head>
  <body lang="en" bgcolor="#FFFFFF" text="#000000" link="#0000FF" vlink="#800080" alink="#FF0000">
    Hi there!
  </body>
</html>
EOFEOF

cat > $dir/file.tex <<EOFEOF
\documentclass{article}
\begin{document}
Hi there!
\end{document}
EOFEOF

cat > $dir/file.m4 <<EOFEOF
dnl an m4 file
AC_DEFINE([foo],[echo "Hi there!"])
foo()
EOFEOF

cat > $dir/file.sh <<EOFEOF
#!/bin/bash
echo "Hi there!"
EOFEOF

cat > $dir/Makefile <<EOFEOF
all: foo bar
foo: hello ; \$(MAKE) bar
bar:
	echo "Hello world!"
EOFEOF

cat > $dir/file.ms <<EOFEOF
a = 1;
for i in [1, 2, 3] do
  a *= i;
  a += 18;
od;
EOFEOF

cat > $dir/file.py <<EOFEOF
import sys
class Hello(object):
    def __init__(self):
        object.__init__(self)
    def hello(self):
        print >> sys.stderr, "Hi there!"
Hello().hello()
EOFEOF

cat > $dir/file.xml <<EOFEOF
<?xml version="1.0" encoding="UTF-8"?>
<foolala version='8.987'>
  <bar>momomomo</bar><baz a="b"/>
</foolala>
EOFEOF

cat > $dir/file.y <<EOFEOF
%{
#include <stdio.h>
#define FOO_BAR(x,y) printf ("x, y")
%}

%name-prefix="foolala"
%error-verbose
%lex-param   {FooLaLa *lala}
%parse-param {FooLaLa *lala}
/* %expect 1 */

%union {
    int ival;
    const char *str;
}

%token <str> ATOKEN
%token <str> ATOKEN2

%type <ival> program stmt
%type <str> if_stmt

%token IF THEN ELSE ELIF FI
%token WHILE DO OD FOR IN
%token CONTINUE BREAK RETURN
%token EQ NEQ LE GE
%token AND OR NOT
%token UMINUS
%token TWODOTS

%left '-' '+'
%left '*' '/'
%left '%'
%left EQ NEQ '<' '>' GE LE
%left OR
%left AND
%left NOT
%left '#'
%left UMINUS

%%

script:   program           { _ms_parser_set_top_node (parser, \$1); }
;

program:  stmt_or_error             { \$\$ = node_list_add (parser, NULL, \$1); }
        | program stmt_or_error     { \$\$ = node_list_add (parser, MS_NODE_LIST (\$1), \$2); }
;

stmt_or_error:
          error ';'         { \$\$ = NULL; }
        | stmt ';'          { \$\$ = $1; }
;

variable: IDENTIFIER                        { \$\$ = node_var (parser, \$1); }
;

%%
EOFEOF

cat > $dir/file.desktop <<EOFEOF
[Desktop Entry]
Encoding=UTF-8
_Name=medit
_Comment=Text editor
Exec=medit %F
Terminal=false
Type=Application
StartupNotify=true
MimeType=text/plain;
Icon=medit.png
Categories=Application;Utility;TextEditor;
EOFEOF

cat > $dir/file.diff <<EOFEOF
diff -r 231ed68760a0 moo/moofileview/moofileview.c
--- a/moo/moofileview/moofileview.c     Wed Dec 20 21:08:14 2006 -0600
+++ b/moo/moofileview/moofileview.c     Wed Dec 20 20:33:06 2006 -0600
@@ -1407,7 +1413,7 @@ create_toolbar (MooFileView *fileview)

     gtk_toolbar_set_tooltips (toolbar, TRUE);
     gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
-    gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_MENU);
+    gtk_toolbar_set_icon_size (toolbar, TOOLBAR_ICON_SIZE);

     _moo_file_view_setup_button_drag_dest (fileview, "MooFileView/Toolbar/GoUp", "go-up");
     _moo_file_view_setup_button_drag_dest (fileview, "MooFileView/Toolbar/GoBack", "go-back");
EOFEOF

cat > $dir/gtkrc <<EOFEOF
# -- THEME AUTO-WRITTEN DO NOT EDIT
include "/usr/share/themes/Clearlooks/gtk-2.0/gtkrc"
style "user-font"
{
  font_name="Tahoma 11"
}
widget_class "*" style "user-font"
include "/home/muntyan/.gtkrc-2.0.mine"
# -- THEME AUTO-WRITTEN DO NOT EDIT
EOFEOF

cat > $dir/file.ini <<EOFEOF
[module]
type=Python
file=simple.py
version=1.0
[plugin]
id=APlugin
_name=A Plugin
_description=A plugin
author=Some Guy
# this is a plugin version, can be anything
version=3.1415926
EOFEOF

cat > $dir/file.pl <<EOFEOF
#!/usr/bin/perl -- # -*- Perl -*-
#
# $Id: collateindex.pl,v 1.10 2004/10/24 17:05:41 petere78 Exp $

print OUT "<title>$title</title>\n\n" if $title;

$last = {};     # the last indexterm we processed
$first = 1;     # this is the first one
$group = "";    # we're not in a group yet
$lastout = "";  # we've not put anything out yet
@seealsos = (); # See also stack.

# Termcount is > 0 iff some entries were skipped.
$quiet || print STDERR "$termcount entries ignored...\n";

&end_entry();

print OUT "</indexdiv>\n" if $lettergroups;
print OUT "</$indextag>\n";

close (OUT);

$quiet || print STDERR "Done.\n";

sub same {
    my($a) = shift;
    my($b) = shift;

    my($aP) = $a->{'psortas'} || $a->{'primary'};
    my($aS) = $a->{'ssortas'} || $a->{'secondary'};
    my($aT) = $a->{'tsortas'} || $a->{'tertiary'};

    my($bP) = $b->{'psortas'} || $b->{'primary'};
    my($bS) = $b->{'ssortas'} || $b->{'secondary'};
    my($bT) = $b->{'tsortas'} || $b->{'tertiary'};

    my($same);

    $aP =~ s/^\s*//; $aP =~ s/\s*$//; $aP = uc($aP);
    $aS =~ s/^\s*//; $aS =~ s/\s*$//; $aS = uc($aS);
    $aT =~ s/^\s*//; $aT =~ s/\s*$//; $aT = uc($aT);
    $bP =~ s/^\s*//; $bP =~ s/\s*$//; $bP = uc($bP);
    $bS =~ s/^\s*//; $bS =~ s/\s*$//; $bS = uc($bS);
    $bT =~ s/^\s*//; $bT =~ s/\s*$//; $bT = uc($bT);

#    print "[$aP]=[$bP]\n";
#    print "[$aS]=[$bS]\n";
#    print "[$aT]=[$bT]\n";

    # Two index terms are the same if:
    # 1. the primary, secondary, and tertiary entries are the same
    #    (or have the same SORTAS)
    # AND
    # 2. They occur in the same titled section
    # AND
    # 3. They point to the same place
    #
    # Notes: Scope is used to suppress some entries, but can't be used
    #          for comparing duplicates.
    #        Interpretation of "the same place" depends on whether or
    #          not $linkpoints is true.

    $same = (($aP eq $bP)
	     && ($aS eq $bS)
	     && ($aT eq $bT)
	     && ($a->{'title'} eq $b->{'title'})
	     && ($a->{'href'} eq $b->{'href'}));

    # If we're linking to points, they're only the same if they link
    # to exactly the same spot.
    $same = $same && ($a->{'hrefpoint'} eq $b->{'hrefpoint'})
	if $linkpoints;

    if ($same) {
       warn "$me: duplicated index entry found: $aP $aS $aT\n";
    }

    $same;
}

sub tsame {
    # Unlike same(), tsame only compares a single term
    my($a) = shift;
    my($b) = shift;
    my($term) = shift;
    my($sterm) = substr($term, 0, 1) . "sortas";
    my($A, $B);

    $A = $a->{$sterm} || $a->{$term};
    $B = $b->{$sterm} || $b->{$term};

    $A =~ s/^\s*//; $A =~ s/\s*$//; $A = uc($A);
    $B =~ s/^\s*//; $B =~ s/\s*$//; $B = uc($B);

    return $A eq $B;
}

=head1 EXAMPLE
B<collateindex.pl> B<-o> F<index.sgml> F<HTML.index>
=head1 EXIT STATUS
=over 5
=item B<0>
Success
=item B<1>
Failure
=back
=head1 AUTHOR
Norm Walsh E<lt>ndw@nwalsh.comE<gt>
Minor updates by Adam Di Carlo E<lt>adam@onshore.comE<gt> and Peter Eisentraut E<lt>peter_e@gmx.netE<gt>
=cut
EOFEOF

cat > $dir/file.po <<EOFEOF
# SOME DESCRIPTIVE TITLE.
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER
# This file is distributed under the same license as the PACKAGE package.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
msgid ""
msgstr ""
"Project-Id-Version: PACKAGE VERSION\n"
"Report-Msgid-Bugs-To: \n"
"POT-Creation-Date: 2006-12-17 09:49-0600\n"
"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
"Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
"Language-Team: LANGUAGE <LL@li.org>\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=CHARSET\n"
"Content-Transfer-Encoding: 8bit\n"

#: ../medit/medit.desktop.in.h:1
msgid "Text editor"
msgstr ""

#: ../medit/medit.desktop.in.h:2
msgid "medit"
msgstr ""
EOFEOF

cat > $dir/file.texi <<EOFEOF
\input texinfo
@setfilename manual
@settitle manual

@titlepage
@title manual

@c The following two commands start the copyright page.
@page
@vskip 0pt plus 1filll
@insertcopying
@end titlepage

@c Output the table of contents at the beginning.
@contents

@ifnottex
@node Top
@top manual
@insertcopying
@end ifnottex

@menu
* MooScript::    MooScript - builtin scripting language.
* Index::        Index.
@end menu

@node MooScript
@chapter MooScript

@cindex chapter, first

This is the first chapter.
@cindex index entry, another

Here is a numbered list.

@enumerate
@item
This is the first item.

@item
This is the second item.
@end enumerate

@node Index
@unnumbered Index

@printindex cp

@bye
EOFEOF

cat > $dir/file.dtd <<EOFEOF
<!-- FIXME: the "name" attribute can be "_name" to be marked for translation -->
<!ENTITY % itemattrs
 "name  CDATA #REQUIRED
  style CDATA #REQUIRED">
<!ELEMENT language (escape-char?,(line-comment|block-comment|string|syntax-item|pattern-item|keyword-list)+)>
<!-- FIXME: the "name" and "section" attributes can be prefixed with
     "_" to be marked for translation -->
<!ELEMENT keyword-list (keyword+)>
<!ATTLIST keyword-list
  %itemattrs;
  case-sensitive                  (true|false) "true"
  match-empty-string-at-beginning (true|false) "false"
  match-empty-string-at-end       (true|false) "false"
  beginning-regex                 CDATA        #IMPLIED
  end-regex                       CDATA        #IMPLIED>
EOFEOF

cat > $dir/file.la <<EOFEOF
# moo.la - a libtool library file
# Generated by ltmain.sh - GNU libtool 1.5.22 Debian 1.5.22-4 (1.1220.2.365 2005/12/18 22:14:06)
#
# Please DO NOT delete this file!
# It is necessary for linking the library.

# The name that we can dlopen(3).
dlname='moo.so'

# Names of this library.
library_names='moo.so moo.so moo.so'

# The name of the static archive.
old_library=''

# Libraries that this one depends upon.
dependency_libs=' -L/usr/local/gtk/lib /home/muntyan/projects/gtk/build/moo/moo/libmoo.la -lutil /usr/local/gtk/lib/libgtk-x11-2.0.la -lXext -lXinerama -lXrandr -lXcursor -lXfixes /usr/local/gtk/lib/libgdk-x11-2.0.la -latk-1.0 /usr/local/gtk/lib/libgdk_pixbuf-2.0.la /usr/local/gtk/lib/libpangocairo-1.0.la /usr/local/gtk/lib/libpangoft2-1.0.la /usr/local/gtk/lib/libpango-1.0.la /usr/local/gtk/lib/libcairo.la -lfreetype -lz -lfontconfig -lpng12 -lXrender -lX11 -lm /usr/local/gtk/lib/libgobject-2.0.la /usr/local/gtk/lib/libgmodule-2.0.la -ldl /usr/local/gtk/lib/libgthread-2.0.la -lpthread /usr/local/gtk/lib/libglib-2.0.la -lrt -lpcre /usr/lib/gcc/i486-linux-gnu/4.1.2/../../..//libfam.la -lrpcsvc /usr/lib/gcc/i486-linux-gnu/4.1.2/../../..//libxml2.la -L/usr/lib/python2.4 -lpython2.4  '

# Version information for moo.
current=0
age=0
revision=0

# Is this an already installed library?
installed=no

# Should we warn about portability when linking against -modules?
shouldnotlink=yes

# Files to dlopen/dlpreopen
dlopen=''
dlpreopen=''

# Directory that this library needs to be installed in:
libdir='/usr/local/gtk/lib/python2.4/site-packages'
relink_command="(cd /home/muntyan/projects/gtk/build/moo/moo; /bin/sh ../libtool  --tag=CC --mode=relink gcc -g -L/usr/local/gtk/lib -o moo.la -rpath /usr/local/gtk/lib/python2.4/site-packages -no-undefined -module -avoid-version -export-symbols-regex initmoo moopython/libmoomod.la libmoo.la -lutil -L/usr/local/gtk/lib -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 -pthread -L/usr/local/gtk/lib -lgthread-2.0 -lglib-2.0 -lpcre -lfam -lxml2 -L/usr/lib/python2.4 -lpython2.4 @inst_prefix_dir@)"
EOFEOF

cat > $dir/file.pc <<EOFEOF
prefix=/usr/local/gtk
exec_prefix=${prefix}
includedir=${prefix}/include
datarootdir=${prefix}/share
datadir=${datarootdir}
libdir=${exec_prefix}/lib

langfilesdir=${datarootdir}/moo/language-specs
pluginsdir=${exec_prefix}/lib/moo/plugins
moolibdir=${exec_prefix}/lib/moo
moodatadir=${datarootdir}/moo

Name: moo
Description: A text editor and terminal emulator library
Requires: gtk+-2.0 libxml-2.0
Version:
Cflags: -I${prefix}/include/moo
Libs: -L${libdir} -lmoo -L/usr/lib/python2.4 -lpython2.4  -lpthread -ldl  -lutil
EOFEOF
