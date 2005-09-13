/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject-project.h
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

#ifndef __C_PROJECT_PROJECT_H__
#define __C_PROJECT_PROJECT_H__

#include "mooutils/moomarkup.h"

G_BEGIN_DECLS


typedef struct _Project Project;
typedef struct _Configuration Configuration;
typedef struct _RunOptions RunOptions;
typedef struct _MakeOptions MakeOptions;
typedef struct _ConfigureOptions ConfigureOptions;


struct _Project
{
    char *name;
    char *file;

    char *project_path;
    char *project_dir;
    RunOptions *run_options;
    MakeOptions *make_options;

    GSList *configurations; /* Configuration */
    Configuration *active;

    GSList *file_list;
};

struct _Configuration
{
    char *name;
    char *build_dir;
    RunOptions *run_options;
    MakeOptions *make_options;
    ConfigureOptions *configure_options;
};

struct _RunOptions
{
    char *executable;
};

struct _MakeOptions
{
};

struct _ConfigureOptions
{
};

typedef enum {
    COMMAND_BUILD_PROJECT
} CommandType;


Project        *project_load                (const char *file);
gboolean        project_save                (Project    *project);

void            project_free                (Project    *project);

void            project_set_file_list       (Project    *project,
                                             GSList     *files);
Configuration  *project_get_configuration   (Project    *project,
                                             const char *name);

/* must be freed */
char           *project_get_command         (Project    *project,
                                             CommandType command_type);


G_END_DECLS

#endif /* __C_PROJECT_PROJECT_H__ */
