/*
 *   mooutils-dialog-win32.h
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
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

#include <windows.h>

class MooWinDialogHelper
{
public:
    MooWinDialogHelper();
    ~MooWinDialogHelper();

    MooWinDialogHelper(const MooWinDialogHelper&) = delete;
    MooWinDialogHelper& operator=(const MooWinDialogHelper&) = delete;

    void hookup(HWND hwnd_dialog);

private:
    static LRESULT CALLBACK hook_proc(int code, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hwnd = nullptr;
    bool m_set = false;
    HHOOK m_hook = nullptr;
    bool m_in_hook_proc = false;
    DWORD m_thread_id;
    UINT m_got_gdk_events_message;
    static MooWinDialogHelper* m_instance;
};
