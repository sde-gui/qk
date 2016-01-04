/*
 *   mooeditor-private.h
 *
 *   Copyright (C) 2004-2015 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include "mooedit/mooeditor-impl.h"
#include "mooedit/moolangmgr.h"
#include "moocpp/gobjptrtypes.h"
#include "moocpp/utils.h"

enum MooEditorOptions {
    MOO_EDITOR_OPTIONS_NONE = 0,
    OPEN_SINGLE         = 1 << 0,
    ALLOW_EMPTY_WINDOW  = 1 << 1,
    SINGLE_WINDOW       = 1 << 2,
    SAVE_BACKUPS        = 1 << 3,
    STRIP_WHITESPACE    = 1 << 4,
};

MOO_DEFINE_FLAGS(MooEditorOptions);

struct MooEditorPrivate {
    std::vector<MooEditWindow*>  windows;
    moo::gobj_ptr<MooUiXml>      doc_ui_xml;
    moo::gobj_ptr<MooUiXml>      ui_xml;
    moo::gobj_ptr<MooHistoryMgr> history;
    moo::grefptr<MooFileWatch>   file_watch;
    MooEditorOptions             opts;

    GType                        window_type;
    GType                        doc_type;

    moo::gobj_ptr<MooLangMgr>    lang_mgr;

    MooEditorPrivate();
    ~MooEditorPrivate();
};
