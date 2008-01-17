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
        input_files = ['../../../moo/mooutils/pixmaps/' + p[1] for p in inp_list]
        if check_mtime([outname], input_files):
            return
        args = ['gdk-pixbuf-csource', '--static', '--build-list']
        for p in inp_list:
            args.append(p[0])
            args.append('../../../moo/mooutils/pixmaps/' + p[1])
        tmp = outname + '.tmp'
        tmp_file = open(tmp, 'wb')
        tmp_file.write(subprocess.Popen(args, stdout=subprocess.PIPE).communicate()[0])
        tmp_file.close()
        shutil.move(tmp, outname)
        print >> sys.stderr, "Created %s" % (outname,)
    generate('../../../moo/mooutils/stock-moo.h', [
		['MOO_HIDE_ICON', 'hide.png'],
		['MOO_CLOSE_ICON', 'close.png'],
		['MOO_STICKY_ICON', 'sticky.png'],
		['MOO_DETACH_ICON', 'detach.png'],
		['MOO_ATTACH_ICON', 'attach.png'],
		['MOO_KEEP_ON_TOP_ICON', 'keepontop.png'],
             ])
    generate('../../../moo/mooutils/stock-medit.h', [
		['MEDIT_ICON', 'medit.png'],
             ])

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

print >> sys.stderr, "done"
