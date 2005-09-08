/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *   cproject-project.c
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

#include "cproject-project.h"
#include <string.h>


static Project  *project_new        (const char *name,
                                     const char *project_path);

static gboolean  project_check      (Project    *project);


static Configuration    *configuration_new      (const char         *name);
static void              configuration_check    (Configuration      *config);
static void              configuration_free     (Configuration      *config);

static RunOptions       *run_options_new        (const char         *executable);
static RunOptions       *get_run_options        (MooMarkupNode      *parent);
static void              save_run_options       (RunOptions         *options,
                                                 MooMarkupNode      *parent);
static void              run_options_free       (RunOptions         *options);

static MakeOptions      *make_options_new       (void);
static MakeOptions      *get_make_options       (MooMarkupNode      *parent);
static void              save_make_options      (MakeOptions        *options,
                                                 MooMarkupNode      *parent);
static void              make_options_free      (MakeOptions        *options);

static ConfigureOptions *configure_options_new  (void);
static ConfigureOptions *get_configure_options  (MooMarkupNode      *parent);
static void              save_configure_options (ConfigureOptions   *options,
                                                 MooMarkupNode      *parent);
static void              configure_options_free (ConfigureOptions   *options);

static void              save_file_list         (GSList             *file_list,
                                                 MooMarkupNode      *parent);
static GSList           *get_file_list          (MooMarkupNode      *parent);



void
project_set_file_list (Project    *project,
                       GSList     *files)
{
    GSList *l;

    g_return_if_fail (project != NULL);

    g_slist_foreach (project->file_list, (GFunc) g_free, NULL);
    g_slist_free (project->file_list);
    project->file_list = NULL;

    for (l = files; l != NULL; l = l->next)
    {
        const char *file = l->data;
        g_return_if_fail (file != NULL);
        project->file_list = g_slist_prepend (project->file_list,
                                              g_strdup (file));
    }

    project->file_list = g_slist_reverse (project->file_list);
}


#define CPROJECT_PROJECT_VERSION "0.1"
#define ELEMENT_PROJECT "cproject"
#define ATTR_VERSION "version"
#define ELEMENT_CONFIGURATIONS "configurations"
#define ELEMENT_CONFIGURATION "configuration"
#define ELEMENT_ACTIVE_CONFIGURATION "active_configuration"
#define ELEMENT_NAME "name"
#define ELEMENT_PATH "path"
#define ELEMENT_BUILD_DIR "build_dir"
#define ELEMENT_EXEC "exec"
#define ELEMENT_FILE_LIST "file_list"
#define ELEMENT_FILE "file"


Project    *project_load            (const char *file)
{
    MooMarkupDoc *xml = NULL;
    MooMarkupElement *root, *name_elm, *project_path_elm, *config_root,
                     *active_configuration_elm;
    MooMarkupNode *child;
    Project *project = NULL;
    const char *version, *project_path;
    GError *error = NULL;

    g_return_val_if_fail (file != NULL, NULL);

    xml = moo_markup_parse_file (file, &error);

    if (!xml)
    {
        g_warning ("%s: could not parse file '%s'", G_STRLOC, file);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        goto out;
    }

    root = moo_markup_get_root_element (xml, ELEMENT_PROJECT);

    if (!root)
    {
        g_warning ("%s: no '%s' element in '%s'", G_STRLOC, ELEMENT_PROJECT, file);
        goto out;
    }

    version = moo_markup_get_prop (root, ATTR_VERSION);

    if (!version)
    {
        g_warning ("%s: no project version attribute in '%s'", G_STRLOC, file);
        goto out;
    }

    if (strcmp (version, CPROJECT_PROJECT_VERSION))
    {
        g_warning ("%s: version '%s' in '%s' is incompatible with current '%s'",
                   G_STRLOC, version, file, CPROJECT_PROJECT_VERSION);
        goto out;
    }

    name_elm = moo_markup_get_element (MOO_MARKUP_NODE (root), ELEMENT_NAME);

    if (!name_elm)
    {
        g_warning ("%s: no project name element in '%s'", G_STRLOC, file);
        goto out;
    }

    if (!name_elm->content || !name_elm->content[0])
    {
        g_warning ("%s: empty project name in '%s'", G_STRLOC, file);
        goto out;
    }

    project_path_elm = moo_markup_get_element (MOO_MARKUP_NODE (root), ELEMENT_PATH);

    if (project_path_elm)
        project_path = project_path_elm->content;
    else
        project_path = ".";

    if (!project_path || !project_path[0])
        project_path = ".";

    config_root = moo_markup_get_element (MOO_MARKUP_NODE (root), ELEMENT_CONFIGURATIONS);

    if (!config_root)
    {
        g_warning ("%s: no configurations specified in '%s'", G_STRLOC, file);
        goto out;
    }

    project = project_new (name_elm->content, project_path);
    project->file = g_strdup (file);

    for (child = config_root->children; child != NULL; child = child->next)
    {
        MooMarkupElement *elm, *build_dir_elm;
        Configuration *configuration;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        elm = MOO_MARKUP_ELEMENT (child);

        configuration = configuration_new (elm->name);

        build_dir_elm = moo_markup_get_element (MOO_MARKUP_NODE (elm), ELEMENT_BUILD_DIR);

        if (!build_dir_elm || !build_dir_elm->content || !build_dir_elm->content[0])
            configuration->build_dir = g_strdup (".");
        else
            configuration->build_dir = g_strdup (build_dir_elm->content);

        configuration->run_options = get_run_options (MOO_MARKUP_NODE (elm));
        configuration->make_options = get_make_options (MOO_MARKUP_NODE (elm));
        configuration->configure_options = get_configure_options (MOO_MARKUP_NODE (elm));

        project->configurations = g_slist_append (project->configurations, configuration);
    }

    active_configuration_elm = moo_markup_get_element (MOO_MARKUP_NODE (root),
                                                       ELEMENT_ACTIVE_CONFIGURATION);

    if (!active_configuration_elm || !active_configuration_elm->content ||
         !active_configuration_elm->content[0])
    {
        if (project->configurations)
            project->active = project->configurations->data;
    }
    else
    {
        project->active = project_get_configuration (project,
                                                     active_configuration_elm->content);
        if (!project->active && project->configurations)
            project->active = project->configurations->data;
    }

    project->file_list = get_file_list (MOO_MARKUP_NODE (root));

    if (!project_check (project))
    {
        g_warning ("%s: oops", G_STRLOC);
        project_free (project);
        project = NULL;
    }

out:
    moo_markup_doc_unref (xml);
    return project;
}


gboolean
project_save (Project *project)
{
    MooMarkupDoc *xml = NULL;
    MooMarkupElement *root, *config_root;
    GError *error = NULL;
    GSList *l;

    g_return_val_if_fail (project != NULL, FALSE);
    g_return_val_if_fail (project_check (project), FALSE);

    xml = moo_markup_doc_new (project->name);

    root = moo_markup_create_root_element (xml, ELEMENT_PROJECT);
    moo_markup_set_prop (root, ATTR_VERSION, CPROJECT_PROJECT_VERSION);

    moo_markup_create_text_element (MOO_MARKUP_NODE (root),
                                    ELEMENT_NAME,
                                    project->name);
    moo_markup_create_text_element (MOO_MARKUP_NODE (root),
                                    ELEMENT_PATH,
                                    project->project_path);

    if (project->run_options)
        save_run_options (project->run_options,
                          MOO_MARKUP_NODE (root));
    if (project->make_options)
        save_make_options (project->make_options,
                           MOO_MARKUP_NODE (root));

    moo_markup_create_text_element (MOO_MARKUP_NODE (root),
                                    ELEMENT_ACTIVE_CONFIGURATION,
                                    project->active->name);

    config_root = moo_markup_create_element (MOO_MARKUP_NODE (root), ELEMENT_CONFIGURATIONS);

    for (l = project->configurations; l != NULL; l = l->next)
    {
        MooMarkupElement *elm;
        Configuration *configuration = l->data;

        elm = moo_markup_create_element (MOO_MARKUP_NODE (config_root),
                                         configuration->name);

        moo_markup_create_text_element (MOO_MARKUP_NODE (elm),
                                        ELEMENT_BUILD_DIR,
                                        configuration->build_dir);

        if (configuration->run_options)
            save_run_options (configuration->run_options,
                              MOO_MARKUP_NODE (elm));
        if (configuration->make_options)
            save_make_options (configuration->make_options,
                               MOO_MARKUP_NODE (elm));
        if (configuration->configure_options)
            save_configure_options (configuration->configure_options,
                                    MOO_MARKUP_NODE (elm));
    }

    save_file_list (project->file_list, MOO_MARKUP_NODE (root));

    if (!moo_markup_save_pretty (xml, project->file, 1, &error))
    {
        g_warning ("%s: could not save project file", G_STRLOC);

        if (error)
        {
            g_warning ("%s: %s", G_STRLOC, error->message);
            g_error_free (error);
        }

        moo_markup_doc_unref (xml);
        return FALSE;
    }

    moo_markup_doc_unref (xml);
    return TRUE;
}


static RunOptions*
get_run_options (MooMarkupNode *parent)
{
    MooMarkupElement *exec;

    exec = moo_markup_get_element (parent, ELEMENT_EXEC);

    if (!exec)
        return NULL;
    else
        return run_options_new (exec->content);
}


static void
save_run_options (RunOptions         *options,
                  MooMarkupNode      *parent)
{
    if (options && options->executable)
        moo_markup_create_text_element (parent, ELEMENT_EXEC,
                                        options->executable);
}


static RunOptions*
run_options_new (const char *executable)
{
    RunOptions *options;

    options = g_new0 (RunOptions, 1);
    options->executable = g_strdup (executable);

    return options;
}


static void
run_options_free (RunOptions *options)
{
    if (options)
    {
        g_free (options->executable);
        g_free (options);
    }
}


static MakeOptions*
get_make_options (G_GNUC_UNUSED MooMarkupNode *parent)
{
    return make_options_new ();
}


static void
save_make_options (G_GNUC_UNUSED MakeOptions        *options,
                   G_GNUC_UNUSED MooMarkupNode      *parent)
{
}


static MakeOptions*
make_options_new (void)
{
    return g_new0 (MakeOptions, 1);
}


static void
make_options_free (MakeOptions *options)
{
    g_free (options);
}


static ConfigureOptions*
get_configure_options (G_GNUC_UNUSED MooMarkupNode *parent)
{
    return configure_options_new ();
}


static void
save_configure_options (G_GNUC_UNUSED ConfigureOptions   *options,
                        G_GNUC_UNUSED MooMarkupNode      *parent)
{
}


static ConfigureOptions*
configure_options_new (void)
{
    return g_new0 (ConfigureOptions, 1);
}


static void
configure_options_free (ConfigureOptions *options)
{
    g_free (options);
}


static Project*
project_new (const char *name,
             const char *project_path)
{
    Project *project = g_new0 (Project, 1);
    project->name = g_strdup (name);
    project->project_path = g_strdup (project_path);
    return project;
}


static Configuration*
configuration_new (const char *name)
{
    Configuration *config = g_new0 (Configuration, 1);
    config->name = g_strdup (name);
    return config;
}


static void
configuration_free (Configuration *config)
{
    if (config)
    {
        g_free (config->name);
        g_free (config->build_dir);
        run_options_free (config->run_options);
        make_options_free (config->make_options);
        configure_options_free (config->configure_options);
        g_free (config);
    }
}


void
project_free (Project *project)
{
    if (project)
    {
        g_free (project->name);
        g_free (project->file);
        g_free (project->project_path);
        g_free (project->project_dir);
        run_options_free (project->run_options);
        make_options_free (project->make_options);

        g_slist_foreach (project->configurations, (GFunc) configuration_free, NULL);
        g_slist_free (project->configurations);

        g_slist_foreach (project->file_list, (GFunc) g_free, NULL);
        g_slist_free (project->file_list);

        g_free (project);
    }
}


static gboolean
project_check (Project *project)
{
    if (!project->name || !project->name[0])
        return FALSE;

    if (!project->file || !project->file[0])
        return FALSE;

    if (!project->project_path || !project->project_path[0])
    {
        g_free (project->project_path);
        project->project_path = g_strdup (".");
    }

    g_free (project->project_dir);
    project->project_dir = g_path_get_dirname (project->file);

    if (!project->configurations)
    {
        project->configurations =
                g_slist_append (NULL, configuration_new ("Default"));
    }

    g_slist_foreach (project->configurations, (GFunc) configuration_check, NULL);

    if (!project->active)
        project->active = project->configurations->data;

    return TRUE;
}


static void
configuration_check (Configuration *config)
{
    g_return_if_fail (config != NULL);

    if (!config->name || !config->name[0])
    {
        g_free (config->name);
        config->name = g_strdup ("Default");
    }

    if (!config->build_dir || !config->build_dir[0])
    {
        g_free (config->build_dir);
        config->build_dir = g_strdup (".");
    }

    if (!config->configure_options)
        config->configure_options = configure_options_new ();
}


Configuration*
project_get_configuration (Project    *project,
                           const char *name)
{
    GSList *l;

    g_return_val_if_fail (project != NULL, NULL);
    g_return_val_if_fail (name != NULL, NULL);

    for (l = project->configurations; l != NULL; l = l->next)
    {
        Configuration *config = l->data;
        g_return_val_if_fail (config != NULL, NULL);
        g_return_val_if_fail (config->name != NULL, NULL);
        if (!strcmp (config->name, name))
            return config;
    }

    return NULL;
}


static void
save_file_list (GSList             *file_list,
                MooMarkupNode      *parent)
{
    MooMarkupElement *list_elm = NULL;
    GSList *l;

    for (l = file_list; l != NULL; l = l->next)
    {
        const char *file = l->data;

        g_return_if_fail (file != NULL);

        if (!list_elm)
            list_elm = moo_markup_create_element (parent, ELEMENT_FILE_LIST);

        moo_markup_create_file_element (MOO_MARKUP_NODE (list_elm),
                                        ELEMENT_FILE, file);
    }
}


static GSList*
get_file_list (MooMarkupNode *parent)
{
    MooMarkupElement *list_elm, *elm;
    MooMarkupNode *child;
    GSList *files = NULL;

    list_elm = moo_markup_get_element (parent, ELEMENT_FILE_LIST);

    if (!list_elm)
        return NULL;

    for (child = list_elm->children; child != NULL; child = child->next)
    {
        char *file;

        if (!MOO_MARKUP_IS_ELEMENT (child))
            continue;

        elm = MOO_MARKUP_ELEMENT (child);
        g_return_val_if_fail (!strcmp (elm->name, ELEMENT_FILE), files);
        g_return_val_if_fail (elm->content && elm->content[0], files);

        file = moo_markup_get_file_content (elm);

        if (!file)
        {
            g_warning ("%s: could not convert '%s' to filename encoding",
                       G_STRLOC, elm->content);
            continue;
        }

        files = g_slist_append (files, file);
    }

    return files;
}


void
command_free (Command *command)
{
    if (command)
    {
        g_free (command->working_dir);
        g_strfreev (command->argv);
        g_strfreev (command->envp);
        g_free (command);
    }
}


Command*
project_get_command (Project    *project,
                     CommandType command_type)
{
    Command *command;

    g_return_val_if_fail (project != NULL, NULL);

    command = g_new0 (Command, 1);

    switch (command_type)
    {
        case COMMAND_BUILD_PROJECT:
            command->argv = g_new0 (char*, 2);
            command->argv[0] = g_strdup ("make");

            if (g_path_is_absolute (project->active->build_dir))
                command->working_dir = g_strdup (project->active->build_dir);
            else
                command->working_dir = g_build_filename (project->project_dir,
                                                         project->active->build_dir,
                                                         NULL);
            break;

        default:
            g_free (command);
            g_return_val_if_reached (NULL);
    }

    return command;
}
