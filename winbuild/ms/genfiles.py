import sys
import os
import shutil
import subprocess

def do_xml_file(name):
    if not name.endswith(".glade") and not source.endswith("-ui.xml"):
        raise RuntimeError("input must be *.glade or *-ui.xml")

    if name.endswith(".glade"):
        base = os.path.basename(name).replace(".glade", "")
        output = os.path.join(outdir, base + "-glade.h")
        varname = "%s_glade_xml" % (base.replace('-', '_'),)
    else:
        base = os.path.basename(name).replace("-ui.xml", "")
        output = os.path.join(outdir, base + "-ui.h")
        varname = "%s_ui_xml" % (base.replace('-', '_'),)

    name = os.path.join(indir, name)
    tmp = output + '.tmp'

    if check_mtime([output], [name]):
        return

    in_file = open(name)
    out_file = open(tmp, "wb")

    out_file.write("/* -*- C -*- */\n")
    out_file.write("static const char %s [] = \"\"\n" % (varname,))

    for line in in_file:
        if line.endswith("\n"):
            line = line[:-1]
        out_file.write('"' + line.replace('"', '\\"') + '"\n')

    out_file.write(";\n")
    out_file.close()

    shutil.move(tmp, output)
    print >> sys.stderr, "Created %s" % (output,)

def do_marshals():
    marshals_list = os.path.join(indir, '../../../moo/mooutils/moomarshals.list')
    marshals_h = os.path.join(indir, '../../../moo/mooutils/moomarshals.h')
    marshals_c = os.path.join(indir, '../../../moo/mooutils/moomarshals.c')

    if check_mtime([marshals_h, marshals_c], [marshals_list]):
        return

    def do_marshals_c():
        tmp = marshals_c + '.tmp'
        tmp_file = open(tmp, "wb")
	tmp_file.write('#include "moomarshals.h"\n')
        tmp_file.write(subprocess.Popen(['glib-genmarshal.exe', '--prefix=_moo_marshal', '--body', marshals_list], 
                                        stdout=subprocess.PIPE).communicate()[0])
        tmp_file.close()
	shutil.move(tmp, marshals_c)
        print >> sys.stderr, "Created %s" % (marshals_c,)

    def do_marshals_h():
        tmp = marshals_h + '.tmp'
        tmp_file = open(tmp, 'wb')
        tmp_file.write(subprocess.Popen(['glib-genmarshal.exe', '--prefix=_moo_marshal', '--header', marshals_list], 
                                        stdout=subprocess.PIPE).communicate()[0])
        tmp_file.close()
	shutil.move(tmp, marshals_h)
        print >> sys.stderr, "Created %s" % (marshals_h,)

    do_marshals_h()
    do_marshals_c()

def do_stock():
    def generate(outname, inp_list):
        input_files = ['../../../moo/' + p[1] for p in inp_list]
        if check_mtime([outname], input_files):
            return
        args = ['gdk-pixbuf-csource', '--static', '--build-list']
        for p in inp_list:
            args.append(p[0])
            args.append('../../../moo/' + p[1])
        tmp = outname + '.tmp'
        tmp_file = open(tmp, 'wb')
        tmp_file.write(subprocess.Popen(args, stdout=subprocess.PIPE).communicate()[0])
        tmp_file.close()
        shutil.move(tmp, outname)
        print >> sys.stderr, "Created %s" % (outname,)
    generate('../../../moo/mooutils/stock-moo.h', [
		['MOO_HIDE_ICON', 'mooutils/pixmaps/hide.png'],
		['MOO_CLOSE_ICON', 'mooutils/pixmaps/close.png'],
		['MOO_STICKY_ICON', 'mooutils/pixmaps/sticky.png'],
		['MOO_DETACH_ICON', 'mooutils/pixmaps/detach.png'],
		['MOO_ATTACH_ICON', 'mooutils/pixmaps/attach.png'],
		['MOO_KEEP_ON_TOP_ICON', 'mooutils/pixmaps/keepontop.png'],
             ])
    generate('../../../moo/mooutils/stock-medit.h', [
		['MEDIT_ICON', 'mooutils/pixmaps/medit.png'],
             ])
    generate('../../../moo/moofileview/symlink.h', [
		['SYMLINK_ARROW', 'moofileview/symlink.png'],
		['SYMLINK_ARROW_SMALL', 'moofileview/symlink-small.png'],
             ])

def do_gtksourceview():
    files = [
        'gtksourcecontextengine.c', 'gtksourceengine.c', 'gtksourceiter.c', 'gtksourcelanguage.c', 
        'gtksourcelanguage-parser-1.c', 'gtksourcelanguage-parser-2.c', 'gtksourcelanguagemanager.c', 
        'gtksourcestyle.c', 'gtksourcestyleschememanager.c', 'gtksourcestylescheme.c', 'gtktextregion.c', 
        'gtksourcecontextengine.h', 'gtksourceengine.h', 'gtksourceiter.h', 'gtksourcelanguage.h', 
        'gtksourcelanguage-private.h', 'gtksourcelanguagemanager.h', 'gtksourcestyle-private.h', 
        'gtksourcestyle.h', 'gtksourcestyleschememanager.h', 'gtksourcestylescheme.h', 'gtktextregion.h', 
        'gtksourceview-utils.h', 'gtksourceview-utils.c',
    ]

    def mangle(name):
        in_name = os.path.join(indir, '../../../moo/mooedit/gtksourceview/upstream', name)
        if in_name.endswith('.h'):
            out_name = os.path.join(indir, '../../../moo/mooedit/gtksourceview', name.replace('.h', '') + '-mangled.h')
        else:
            out_name = os.path.join(indir, '../../../moo/mooedit/gtksourceview', name.replace('.c', '') + '-mangled.c')
        if check_mtime([out_name], [in_name]):
            return
        in_file = open(in_name, 'rb')
        text = in_file.read()
        in_file.close()

        for f in ['gtksourcecontextengine', 'gtksourceengine', 'gtksourceiter', 'gtksourcelanguage',
                  'gtksourcelanguage-private', 'gtksourcelanguagemanager', 'gtksourcestyle',
                  'gtksourcestylescheme', 'gtksourcestyleschememanager', 'gtksourcestyle-private',
                  'gtktextregion', 'gtksourceview-utils']:
            text = text.replace('\"%s.h\"' % (f,), '\"%s-mangled.h\"' % (f,))
            text = text.replace('<gtksourceview/%s.h>' % (f,), '<gtksourceview/%s-mangled.h>' % (f,))
        for s, r in [
            ['#include \"gtksourcebuffer.h\"', ''],
            ['#include \"gtksourceview.h\"', ''],
            ['GtkSource', 'MooGtkSource'],
            ['GtkTextRegion', 'MooGtkTextRegion'],
            ['_gtk_source', 'gtk_source'],
            ['gtk_source', '_moo_gtk_source'],
            ['_gtk_text_region', 'gtk_text_region'],
            ['gtk_text_region', '_moo_gtk_text_region'],
            ['g_slice_new', '_moo_new'],
            ['g_slice_free', '_moo_free'],
        ]:
            text = text.replace(s, r)

        tmp = out_name + '.tmp'
        tmp_file = open(tmp, 'wb')
        tmp_file.write("/* This file is autogenerated from %s */\n" % (in_name,))
        if out_name.endswith('.c'):
            tmp_file.write("#include <mooutils/mooutils-misc.h>\n")
        tmp_file.write("#line 1 \"mooedit/upstream/%s\"\n" % (name,))
        tmp_file.write(text)
        tmp_file.close()
        shutil.move(tmp, out_name)
        print >> sys.stderr, "Created %s" % (out_name,)

    for name in files:
        mangle(name)

def check_mtime(newer, older):
    for n in newer:
        if not os.path.exists(n):
            return False
    for n in newer:
        for o in older:
            if os.path.getmtime(n) <= os.path.getmtime(o):
                return False
    return True

glade_files = [
    '../../../moo/mooutils/glade/mooaccelbutton.glade',
    '../../../moo/mooutils/glade/mooaccelprefs.glade',
    '../../../moo/mooutils/glade/moologwindow.glade',
    '../../../moo/mooapp/glade/mooappabout.glade',
    '../../../moo/mooedit/glade/mooeditprefs-file.glade',
    '../../../moo/mooedit/glade/mooeditprefs-general.glade',
    '../../../moo/mooedit/glade/mooeditprefs-langs.glade',
    '../../../moo/mooedit/glade/mooeditprefs-view.glade',
    '../../../moo/mooedit/glade/mooeditprogress.glade',
    '../../../moo/mooedit/glade/mooeditsavemult.glade',
    '../../../moo/mooedit/glade/mooedittools.glade',
    '../../../moo/mooedit/glade/moopluginprefs.glade',
    '../../../moo/mooedit/glade/mooprint.glade',
    '../../../moo/mooedit/glade/mooprintpreview.glade',
    '../../../moo/mooedit/glade/mootextfind.glade',
    '../../../moo/mooedit/glade/mootextgotoline.glade',
    '../../../moo/mooedit/glade/mooquicksearch.glade',
    '../../../moo/mooedit/glade/moostatusbar.glade',
    '../../../moo/mooedit/plugins/moofileselector.glade',
    '../../../moo/mooedit/plugins/moofileselector-prefs.glade',
    '../../../moo/mooedit/plugins/moofind.glade',
    '../../../moo/moofileview/glade/moobookmark-editor.glade',
    '../../../moo/moofileview/glade/moocreatefolder.glade',
    '../../../moo/moofileview/glade/moofileprops.glade',
    '../../../moo/moofileview/glade/moofileview-drop.glade',
    '../../../moo/mooterm/glade/mootermprefs.glade',
]

ui_files = [
    '../../../moo/mooedit/medit-ui.xml',
    '../../../moo/mooedit/mooedit-ui.xml',
    '../../../moo/moofileview/moofileview-ui.xml',
]

indir = sys.argv[1]
outdir = sys.argv[2]

print >> sys.stderr, ' '.join(sys.argv)

for source in glade_files + ui_files:
    do_xml_file(source)

do_marshals()
do_stock()
do_gtksourceview()

print >> sys.stderr, "done"
