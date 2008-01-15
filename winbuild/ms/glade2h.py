import sys
import os
import shutil

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
]

ui_files = [
    '../../../moo/mooedit/medit-ui.xml',
    '../../../moo/mooedit/mooedit-ui.xml',
    '../../../moo/moofileview/moofileview-ui.xml',
]

indir = sys.argv[1]
outdir = sys.argv[2]

print >> sys.stderr, "command line arguments: %s" % (sys.argv,)

for source in glade_files + ui_files:
    if not source.endswith(".glade") and not source.endswith("-ui.xml"):
        raise RuntimeError("input must be *.glade or *-ui.xml")

    if source.endswith(".glade"):
        base = os.path.basename(source).replace(".glade", "")
        output = os.path.join(outdir, base + "-glade.h")
        varname = "%s_glade_xml" % (base.replace('-', '_'),)
    else:
        base = os.path.basename(source).replace("-ui.xml", "")
        output = os.path.join(outdir, base + "-ui.h")
        varname = "%s_ui_xml" % (base.replace('-', '_'),)

    source = os.path.join(indir, source)
    tmp = output + '.tmp'

    in_file = open(source)
    out_file = open(tmp, "w+")

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
