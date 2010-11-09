#! /bin/sh

srcdir=`dirname $0`

for f in gspawn.h gspawn-win32.c; do
  sed -r -e 's/G_DISABLE_SINGLE_INCLUDES/NOT_DEFINED/g' -e 's/__G_SPAWN_H__/__M_SPAWN_H__/g' \
         -e 's/G_SPAWN/M_SPAWN/g' -e 's/GSpawn/MSpawn/g' -e 's/g_spawn/m_spawn/g' \
         -e 's@#include <glib/gerror.h>@#include <glib.h>@g' \
         -e 's@#include "glib.h"@#include "mooutils/gspawn/gspawn-support.h"@g' \
         -e 's@#include "glibintl.h"@#include "mooutils/mooi18n.h"@g' \
         -e 's@#include "g\w+.h"@#include <glib.h>@g' \
         -e 's/_g_sprintf/sprintf/g' \
         -e 's/gspawn-win64-helper/mspawn-win64-helper/g' -e 's/gspawn-win32-helper/mspawn-win32-helper/g' \
         -e 's@extern gchar \*_glib_get_dll_directory@// extern gchar *_glib_get_dll_directory@g' \
    < $srcdir/$f > $srcdir/$f.tmp || exit 1
  cmp -s $srcdir/$f $srcdir/$f.tmp || mv $srcdir/$f.tmp $srcdir/$f || exit 1
  rm -f $srcdir/$f.tmp || exit 1
done
