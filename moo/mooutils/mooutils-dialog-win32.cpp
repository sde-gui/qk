/*
 *   mooutils-dialog-win32.cpp
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

#include "config.h"

#include "mooutils/mooutils-dialog-win32.h"
#include "moocpp/moocpp.h"

#include <gdk/gdkwin32.h>
#include <gtk/gtk.h>

#include <mooglib/moo-glib.h>

#include <windows.h>
#include <windowsx.h>
#include <shobjidl.h> 
#include <shlwapi.h>

using namespace moo;

MooWinDialogHelper::MooWinDialogHelper()
    : m_got_gdk_events_message(RegisterWindowMessage(L"GDK_WIN32_GOT_EVENTS"))
    , m_thread_id(GetCurrentThreadId())
{
    g_return_if_fail(!m_instance);
    m_instance = this;
}

MooWinDialogHelper::~MooWinDialogHelper()
{
    if (m_instance == this)
    {
        if (m_hook)
            UnhookWindowsHookEx(m_hook);

        gdk_win32_set_modal_dialog_libgtk_only(nullptr);

        m_instance = nullptr;
    }
}

void MooWinDialogHelper::hookup(HWND hwnd)
{
    g_return_if_fail(this == m_instance);
    g_return_if_fail(hwnd != nullptr);

    gdk_win32_set_modal_dialog_libgtk_only(hwnd);

    if (!m_hook)
    {
        m_hook = SetWindowsHookEx(WH_CALLWNDPROC, hook_proc,
                                    GetModuleHandle(nullptr),
                                    m_thread_id);
        g_return_if_fail(m_hook != nullptr);

        while (gtk_events_pending())
            gtk_main_iteration();
    }
}

LRESULT CALLBACK MooWinDialogHelper::hook_proc(int code, WPARAM wParam, LPARAM lParam)
{
    g_return_val_if_fail(m_instance != nullptr, 0);

    if (GetCurrentThreadId() != m_instance->m_thread_id)
        return CallNextHookEx(m_instance->m_hook, code, wParam, lParam);

    if (m_instance->m_in_hook_proc)
        return CallNextHookEx(m_instance->m_hook, code, wParam, lParam);

    raii on_exit([] ()
    {
        m_instance->m_in_hook_proc = false;
    });

    m_instance->m_in_hook_proc = true;

    while (gtk_events_pending())
        gtk_main_iteration();

    return CallNextHookEx(m_instance->m_hook, code, wParam, lParam);
}

MooWinDialogHelper* MooWinDialogHelper::m_instance = nullptr;
