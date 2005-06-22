/*
 *   mooutils/moowin.c
 *
 *   Copyright (C) 2004-2005 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
*/

#include "mooutils/moowin.h"


#ifdef GDK_WINDOWING_X11

#include <X11/Xatom.h>
#include <gdk/gdkx.h>


/* TODO TODO is it 64-bits safe? */
/* TODO TODO rewrite all of this */

static void add_xid (GtkWindow  *window,
                     GArray     *array)
{
    XID xid;
    if (window && GTK_WIDGET(window)->window) {
        xid = GDK_WINDOW_XID (GTK_WIDGET(window)->window);
        g_array_append_val (array, xid);
    }
}


static gboolean contains (GArray *xids, XID w)
{
    guint i;
    XID *wins = (XID*) xids->data;
    for (i = 0; i < xids->len; ++i)
        if (wins[i] == w)
            return TRUE;
    return FALSE;
}


static gboolean is_minimized (Display *display, XID w)
{
    Atom actual_type_return;
    int actual_format_return;
    gulong nitems_return;
    gulong bytes_after_return;
    Atom *data;
    int ret;
    gulong i;

    static Atom wm_state = None;
    static Atom wm_state_hidden = None;
    if (wm_state == None) {
        wm_state = XInternAtom (display, "_NET_WM_STATE", FALSE);
        wm_state_hidden = XInternAtom (display, "_NET_WM_STATE_HIDDEN", FALSE);
    }

    gdk_error_trap_push ();
    ret = XGetWindowProperty (display, w,
                              wm_state, 0,
                              G_MAXLONG, FALSE, XA_ATOM,
                              &actual_type_return,
                              &actual_format_return,
                              &nitems_return,
                              &bytes_after_return,
                              (guchar**) &data);
    g_return_val_if_fail (!gdk_error_trap_pop () && ret == Success, FALSE);

    if (!nitems_return) {
        XFree (data);
        return FALSE;
    }

    if (actual_type_return != XA_ATOM) {
        g_critical ("%s: actual_type_return != XA_WINDOW", G_STRLOC);
        XFree (data);
        return FALSE;
    }

    for (i = 0; i < nitems_return; ++i)
        if (data[i] == wm_state_hidden) {
            XFree (data);
            return TRUE;
        }

    XFree (data);
    return FALSE;
}


static GtkWindow *find_by_xid (GSList *windows, XID w)
{
    GSList *l;
    for (l = windows; l != NULL; l = l->next)
        if (GDK_WINDOW_XID (GTK_WIDGET(l->data)->window) == w)
            return l->data;
    g_return_val_if_fail (l != NULL, NULL);
    return NULL;
}


GtkWindow *moo_get_top_window (GSList *windows)
{
    GArray *xids;
    Display *display;
    Atom actual_type_return;
    int actual_format_return;
    gulong nitems_return;
    gulong bytes_after_return;
    XID *data;
    int ret;
    long i;

    static Atom list_stacking_atom = None;

    g_return_val_if_fail (windows != NULL, NULL);

    xids = g_array_new (FALSE, FALSE, sizeof (XID));
    g_slist_foreach (windows, (GFunc) add_xid, xids);

    if (!xids->len) {
        g_critical ("%s: zero length array of x ids", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    display = GDK_WINDOW_XDISPLAY (GTK_WIDGET(windows->data)->window);
    if (!display) {
        g_critical ("%s: !display", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (list_stacking_atom == None)
        list_stacking_atom = XInternAtom (display, "_NET_CLIENT_LIST_STACKING", FALSE);

    gdk_error_trap_push ();
    ret = XGetWindowProperty (display, GDK_ROOT_WINDOW(),
                              list_stacking_atom, 0,
                              G_MAXLONG, FALSE, XA_WINDOW,
                              &actual_type_return,
                              &actual_format_return,
                              &nitems_return,
                              &bytes_after_return,
                              (guchar**) &data);
    if (gdk_error_trap_pop () || ret != Success) {
        g_critical ("%s: error in XGetWindowProperty", G_STRLOC);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (!nitems_return) {
        g_critical ("%s: !nitems_return", G_STRLOC);
        XFree (data);
        g_array_free (xids, TRUE);
        return NULL;
    }

    if (actual_type_return != XA_WINDOW) {
        g_critical ("%s: actual_type_return != XA_WINDOW", G_STRLOC);
        XFree (data);
        g_array_free (xids, TRUE);
        return NULL;
    }

    for (i = nitems_return - 1; i >= 0; --i)
        if (contains (xids, data[i]) && !is_minimized (display, data[i]))
        {
            XID id = data[i];
            XFree (data);
            g_array_free (xids, TRUE);
            return find_by_xid (windows, id);
        }

    XFree (data);
    g_array_free (xids, TRUE);
    g_warning ("%s: all minimized?", G_STRLOC);
    return GTK_WINDOW (windows->data);
}


gboolean     moo_window_is_hidden   (GtkWindow  *window)
{
    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);
    return is_minimized (GDK_WINDOW_XDISPLAY (GTK_WIDGET(window)->window),
                         GDK_WINDOW_XID (GTK_WIDGET(window)->window));
}


#elif defined(__WIN32__)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gdk/gdkwin32.h>


#define get_handle(w) \
    gdk_win32_drawable_get_handle (GTK_WIDGET(w)->window)

gboolean     moo_window_is_hidden   (GtkWindow  *window)
{
    HANDLE h;
    WINDOWPLACEMENT info = {0};

    g_return_val_if_fail (GTK_IS_WINDOW (window), FALSE);

    h = get_handle (window);
    g_return_val_if_fail (h != NULL, FALSE);

    info.length = sizeof (WINDOWPLACEMENT);
    if (!GetWindowPlacement (h, &info)) {
        DWORD err = GetLastError ();
        char *msg = g_win32_error_message (err);
        g_return_val_if_fail (msg != NULL, FALSE);
        g_warning ("%s: %s", G_STRLOC, msg);
        g_free (msg);
        return FALSE;
    }

    return info.showCmd == SW_MINIMIZE ||
            info.showCmd == SW_HIDE ||
            info.showCmd == SW_SHOWMINIMIZED;
}


GtkWindow *moo_get_top_window (GSList *windows)
{
    GSList *l;
    HWND top = NULL;
    HWND current = NULL;

    g_return_val_if_fail (windows != NULL, NULL);

    for (l = windows; l != NULL; l = l->next)
        if (!moo_window_is_hidden (GTK_WINDOW (l->data)))
            break;
    if (!l)
        return GTK_WINDOW (windows->data);

    top = get_handle (windows->data);
    current = top;
    while (TRUE) {
        current = GetNextWindow (current, GW_HWNDPREV);
        if (!current)
            break;

        for (l = windows; l != NULL; l = l->next)
            if (current == get_handle (l->data))
                break;
        if (l != NULL)
            top = get_handle (l->data);
    }

    for (l = windows; l != NULL; l = l->next)
        if (top == get_handle (l->data))
            break;
    g_return_val_if_fail (l != NULL, GTK_WINDOW (windows->data));
    return GTK_WINDOW (l->data);
}

#else /* neither X nor WIN32 */

GtkWindow *moo_get_top_window (GSList *windows)
{
    g_return_val_if_fail (windows != NULL, NULL);
    g_critical ("%s: don't know how to do it", G_STRLOC);
    return GTK_WINDOW (windows->data);
}

#endif



gboolean    moo_window_set_icon_from_stock  (GtkWindow      *window,
                                             const char     *stock_id)
{
#ifndef __WIN32__
    GdkPixbuf *icon;

    g_return_val_if_fail (GTK_IS_WINDOW (window) && stock_id != NULL, FALSE);
    icon = gtk_widget_render_icon (GTK_WIDGET (window), stock_id,
                                   GTK_ICON_SIZE_BUTTON, 0);

    if (icon) {
        gtk_window_set_icon (GTK_WINDOW (window), icon);
        gdk_pixbuf_unref (icon);
        return TRUE;
    }
    else
        return FALSE;
#else /* __WIN32__ */
    return TRUE;
#endif /* __WIN32__ */
}
