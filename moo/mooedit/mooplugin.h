/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   mooplugin.h
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

#ifndef __MOO_PLUGIN_H__
#define __MOO_PLUGIN_H__

#include <mooedit/mooeditor.h>

G_BEGIN_DECLS

#define MOO_PLUGIN_PREFS_ROOT  "Plugins"
#define MOO_PLUGIN_CURRENT_VERSION 15
#define MOO_PLUGIN_DIR_BASENAME "plugins"


#define MOO_TYPE_PLUGIN                 (moo_plugin_get_type ())
#define MOO_PLUGIN(object)              (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_PLUGIN, MooPlugin))
#define MOO_PLUGIN_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_PLUGIN, MooPluginClass))
#define MOO_IS_PLUGIN(object)           (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_PLUGIN))
#define MOO_IS_PLUGIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_PLUGIN))
#define MOO_PLUGIN_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_PLUGIN, MooPluginClass))

#define MOO_TYPE_WIN_PLUGIN             (moo_win_plugin_get_type ())
#define MOO_WIN_PLUGIN(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_WIN_PLUGIN, MooWinPlugin))
#define MOO_WIN_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_WIN_PLUGIN, MooWinPluginClass))
#define MOO_IS_WIN_PLUGIN(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_WIN_PLUGIN))
#define MOO_IS_WIN_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_WIN_PLUGIN))
#define MOO_WIN_PLUGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_WIN_PLUGIN, MooWinPluginClass))

#define MOO_TYPE_DOC_PLUGIN             (moo_doc_plugin_get_type ())
#define MOO_DOC_PLUGIN(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_DOC_PLUGIN, MooDocPlugin))
#define MOO_DOC_PLUGIN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_DOC_PLUGIN, MooDocPluginClass))
#define MOO_IS_DOC_PLUGIN(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_DOC_PLUGIN))
#define MOO_IS_DOC_PLUGIN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_DOC_PLUGIN))
#define MOO_DOC_PLUGIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_DOC_PLUGIN, MooDocPluginClass))


typedef struct _MooPlugin            MooPlugin;
typedef struct _MooPluginInfo        MooPluginInfo;
typedef struct _MooPluginParams      MooPluginParams;
typedef struct _MooPluginPrefsParams MooPluginPrefsParams;
typedef struct _MooPluginClass       MooPluginClass;
typedef struct _MooWinPlugin         MooWinPlugin;
typedef struct _MooWinPluginClass    MooWinPluginClass;
typedef struct _MooDocPlugin         MooDocPlugin;
typedef struct _MooDocPluginClass    MooDocPluginClass;


typedef gboolean    (*MooPluginModuleInitFunc)  (void);

typedef gboolean    (*MooPluginInitFunc)        (MooPlugin      *plugin);
typedef void        (*MooPluginDeinitFunc)      (MooPlugin      *plugin);
typedef void        (*MooPluginAttachWinFunc)   (MooPlugin      *plugin,
                                                 MooEditWindow  *window);
typedef void        (*MooPluginDetachWinFunc)   (MooPlugin      *plugin,
                                                 MooEditWindow  *window);
typedef void        (*MooPluginAttachDocFunc)   (MooPlugin      *plugin,
                                                 MooEdit        *doc,
                                                 MooEditWindow  *window);
typedef void        (*MooPluginDetachDocFunc)   (MooPlugin      *plugin,
                                                 MooEdit        *doc,
                                                 MooEditWindow  *window);

typedef GtkWidget  *(*MooPluginPrefsPageFunc)   (MooPlugin      *plugin);

typedef gboolean    (*MooWinPluginCreateFunc)   (MooWinPlugin   *win_plugin);
typedef void        (*MooWinPluginDestroyFunc)  (MooWinPlugin   *win_plugin);
typedef gboolean    (*MooDocPluginCreateFunc)   (MooDocPlugin   *doc_plugin);
typedef void        (*MooDocPluginDestroyFunc)  (MooDocPlugin   *doc_plugin);


struct _MooPluginParams
{
    gboolean enabled;
    gboolean visible;
};

struct _MooPluginPrefsParams
{
    /* it's needed to make sizeof() > 0 */
    guint dummy : 1;
};

struct _MooPluginInfo
{
    const char *id;

    const char *name;
    const char *description;
    const char *author;
    const char *version;

    MooPluginParams *params;
    MooPluginPrefsParams *prefs_params;
};

struct _MooPlugin
{
    GObject parent;

    gboolean initialized;
    GModule *module;

    GQuark id_quark;
    MooPluginInfo *info;
    GType win_plugin_type;
    GType doc_plugin_type;
};

struct _MooWinPlugin
{
    GObject parent;
    MooEditWindow *window;
    MooPlugin *plugin;
};

struct _MooDocPlugin
{
    GObject parent;
    MooEditWindow *window;
    MooEdit *doc;
    MooPlugin *plugin;
};

struct _MooPluginClass
{
    GObjectClass parent_class;

    guint plugin_system_version;

    MooPluginInitFunc init;
    MooPluginDeinitFunc deinit;
    MooPluginAttachWinFunc attach_win;
    MooPluginDetachWinFunc detach_win;
    MooPluginAttachDocFunc attach_doc;
    MooPluginDetachDocFunc detach_doc;
    MooPluginPrefsPageFunc create_prefs_page;
};

struct _MooWinPluginClass
{
    GObjectClass parent_class;

    MooWinPluginCreateFunc create;
    MooWinPluginDestroyFunc destroy;
};

struct _MooDocPluginClass
{
    GObjectClass parent_class;

    MooDocPluginCreateFunc create;
    MooDocPluginDestroyFunc destroy;
};


GType       moo_plugin_get_type         (void) G_GNUC_CONST;
GType       moo_win_plugin_get_type     (void) G_GNUC_CONST;
GType       moo_doc_plugin_get_type     (void) G_GNUC_CONST;

gboolean    moo_plugin_register         (GType           type);
void        moo_plugin_unregister       (GType           type);

gboolean    moo_plugin_initialized      (MooPlugin      *plugin);
gboolean    moo_plugin_enabled          (MooPlugin      *plugin);
gboolean    moo_plugin_set_enabled      (MooPlugin      *plugin,
                                         gboolean        enabled);

MooPlugin  *moo_plugin_get              (GType           type);

gpointer    moo_plugin_lookup           (const char     *plugin_id);
gpointer    moo_win_plugin_lookup       (const char     *plugin_id,
                                         MooEditWindow  *window);
gpointer    moo_doc_plugin_lookup       (const char     *plugin_id,
                                         MooEdit        *doc);

/* list of MooPlugin*; list must be freed */
GSList     *moo_list_plugins            (void);

const char *moo_plugin_id               (MooPlugin      *plugin);

const char *moo_plugin_name             (MooPlugin      *plugin);
const char *moo_plugin_description      (MooPlugin      *plugin);
const char *moo_plugin_author           (MooPlugin      *plugin);
const char *moo_plugin_version          (MooPlugin      *plugin);

void        moo_plugin_read_dirs        (char          **dirs);
void        moo_plugin_init_builtin     (void);

void        _moo_window_attach_plugins  (MooEditWindow  *window);
void        _moo_window_detach_plugins  (MooEditWindow  *window);
void        _moo_doc_attach_plugins     (MooEditWindow  *window,
                                         MooEdit        *doc);
void        _moo_doc_detach_plugins     (MooEditWindow  *window,
                                         MooEdit        *doc);

void        _moo_plugin_attach_prefs    (GtkWidget      *prefs_dialog);


G_END_DECLS

#endif /* __MOO_PLUGIN_H__ */
