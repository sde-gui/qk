/*
 *   moocommand.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef __MOO_COMMAND_H__
#define __MOO_COMMAND_H__

#include <mooutils/moomarkup.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define MOO_TYPE_COMMAND                    (moo_command_get_type ())
#define MOO_COMMAND(object)                 (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND, MooCommand))
#define MOO_COMMAND_CLASS(klass)            (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND, MooCommandClass))
#define MOO_IS_COMMAND(object)              (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND))
#define MOO_IS_COMMAND_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND))
#define MOO_COMMAND_GET_CLASS(obj)          (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND, MooCommandClass))

#define MOO_TYPE_COMMAND_CONTEXT            (moo_command_context_get_type ())
#define MOO_COMMAND_CONTEXT(object)         (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_COMMAND_CONTEXT, MooCommandContext))
#define MOO_COMMAND_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_COMMAND_CONTEXT, MooCommandContextClass))
#define MOO_IS_COMMAND_CONTEXT(object)      (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_COMMAND_CONTEXT))
#define MOO_IS_COMMAND_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_COMMAND_CONTEXT))
#define MOO_COMMAND_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_COMMAND_CONTEXT, MooCommandContextClass))

#define MOO_TYPE_COMMAND_DATA               (moo_command_data_get_type ())
#define MOO_TYPE_COMMAND_OPTIONS            (moo_command_options_get_type ())

typedef struct _MooCommand                  MooCommand;
typedef struct _MooCommandClass             MooCommandClass;
typedef struct _MooCommandContext           MooCommandContext;
typedef struct _MooCommandContextPrivate    MooCommandContextPrivate;
typedef struct _MooCommandContextClass      MooCommandContextClass;
typedef struct _MooCommandData              MooCommandData;
typedef struct _MooCommandTypeInfo          MooCommandTypeInfo;

typedef enum {
    MOO_COMMAND_NEED_DOC        = 1 << 0,
    MOO_COMMAND_NEED_FILE       = 1 << 1,
    MOO_COMMAND_NEED_SAVE       = 1 << 2,
    MOO_COMMAND_NEED_WINDOW     = 1 << 3
} MooCommandOptions;

struct _MooCommandContext {
    GObject base;
    MooCommandContextPrivate *priv;
};

struct _MooCommandContextClass {
    GObjectClass base_class;
};

struct _MooCommand {
    GObject base;
    /* read-only */
    MooCommandOptions options;
};

struct _MooCommandClass {
    GObjectClass base_class;

    gboolean    (*check_sensitive)  (MooCommand         *cmd,
                                     gpointer            doc,
                                     gpointer            window);
    gboolean    (*check_context)    (MooCommand         *cmd,
                                     MooCommandContext  *ctx);
    void        (*run)              (MooCommand         *cmd,
                                     MooCommandContext  *ctx);
};

typedef MooCommand *(*MooCommandFactoryFunc)    (MooCommandData     *data,
                                                 const char         *options,
                                                 gpointer            user_data);
typedef GtkWidget *(*MooCommandCreateWidgetFunc)(gpointer            user_data);
typedef void       (*MooCommandLoadDataFunc)    (GtkWidget          *widget,
                                                 MooCommandData     *data,
                                                 gpointer            user_data);
typedef gboolean   (*MooCommandSaveDataFunc)    (GtkWidget          *widget,
                                                 MooCommandData     *data,
                                                 gpointer            user_data);

struct _MooCommandTypeInfo {
    const char *id;
    const char *name;
    MooCommandFactoryFunc factory;
    MooCommandCreateWidgetFunc create_widget;
    MooCommandLoadDataFunc load_data;
    MooCommandSaveDataFunc save_data;
    gpointer data;
};


GType               moo_command_get_type        (void) G_GNUC_CONST;
GType               moo_command_context_get_type(void) G_GNUC_CONST;
GType               moo_command_data_get_type   (void) G_GNUC_CONST;
GType               moo_command_options_get_type(void) G_GNUC_CONST;

MooCommand         *moo_command_create          (const char         *type,
                                                 const char         *options,
                                                 MooCommandData     *data);

void                moo_command_run             (MooCommand         *cmd,
                                                 MooCommandContext  *ctx);
gboolean            moo_command_check_context   (MooCommand         *cmd,
                                                 MooCommandContext  *ctx);
gboolean            moo_command_check_sensitive (MooCommand         *cmd,
                                                 gpointer            doc,
                                                 gpointer            window);

void                moo_command_set_options     (MooCommand         *cmd,
                                                 MooCommandOptions   options);
MooCommandOptions   moo_command_get_options     (MooCommand         *cmd);

MooCommandOptions   moo_command_options_parse   (const char         *string);

void                moo_command_type_register   (const char         *type,
                                                 const char         *name,
                                                 MooCommandFactoryFunc factory,
                                                 MooCommandCreateWidgetFunc create_widget,
                                                 MooCommandLoadDataFunc load_data,
                                                 MooCommandSaveDataFunc save_data,
                                                 gpointer            data,
                                                 GDestroyNotify      notify);
MooCommandTypeInfo *moo_command_type_lookup     (const char         *type);
/* free return value and its contents */
GSList             *moo_command_list_types      (void);


MooCommandData     *moo_command_data_new        (void);

MooCommandData     *moo_command_data_ref        (MooCommandData     *data);
void                moo_command_data_unref      (MooCommandData     *data);
void                moo_command_data_set        (MooCommandData     *data,
                                                 const char         *key,
                                                 const char         *value);
const char         *moo_command_data_get        (MooCommandData     *data,
                                                 const char         *key);
void                moo_command_data_unset      (MooCommandData     *data,
                                                 const char         *key);
void                moo_command_data_clear      (MooCommandData     *data);


MooCommandContext  *moo_command_context_new     (gpointer            doc,
                                                 gpointer            window);

void                moo_command_context_set_doc (MooCommandContext  *ctx,
                                                 gpointer            doc);
void                moo_command_context_set_window (MooCommandContext *ctx,
                                                 gpointer            window);
gpointer            moo_command_context_get_doc (MooCommandContext  *ctx);
gpointer            moo_command_context_get_window (MooCommandContext *ctx);

void                moo_command_context_set     (MooCommandContext  *ctx,
                                                 const char         *name,
                                                 const GValue       *value);
gboolean            moo_command_context_get     (MooCommandContext  *ctx,
                                                 const char         *name,
                                                 GValue             *value);
void                moo_command_context_unset   (MooCommandContext  *ctx,
                                                 const char         *name);

typedef void (*MooCommandContextForeachFunc)    (const char         *name,
                                                 const GValue       *value,
                                                 gpointer            data);
void                moo_command_context_foreach (MooCommandContext  *ctx,
                                                 MooCommandContextForeachFunc func,
                                                 gpointer            data);

void                _moo_command_init           (void);
MooCommandData     *_moo_command_parse_markup   (MooMarkupNode      *node,
                                                 char              **type,
                                                 char              **options);
void                _moo_command_format_markup  (MooMarkupNode      *parent,
                                                 MooCommandData     *data,
                                                 char               *type,
                                                 char               *options);


G_END_DECLS

#endif /* __MOO_COMMAND_H__ */
