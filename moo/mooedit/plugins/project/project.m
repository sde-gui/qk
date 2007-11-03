#include "project.h"
#include <mooutils/mooutils-debug.h>


static GHashTable *registered_types;

static void
init_types (void)
{
    if (!registered_types)
        registered_types = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

@implementation MPProject

+ (void) registerProjectType: (CSTR) name
{
    Class old_class;

    init_types ();

    old_class = g_hash_table_lookup (registered_types, name);
    if (old_class != Nil && old_class != self)
        _moo_message ("%s: re-registering project type %s", G_STRLOC, name);

    g_hash_table_insert (registered_types, g_strdup (name), self);
}

+ (id) loadFile: (MPFile*) pf
          error: (GError**) error
{
    const char *project_type;
    Class project_class;
    MPProject *project;

    init_types ();

    project_type = [pf type];
    project_class = g_hash_table_lookup (registered_types, project_type);
    if (!project_class)
    {
        g_set_error (error, MP_FILE_ERROR, 0,
                     "unknown project type '%s'",
                     project_type);
        return nil;
    }

    project = [[project_class alloc] init];
    if (project && [project loadFile:pf error:error])
        return project;

    [project release];
    return nil;
}

- (BOOL) loadFile: (MPFile*) pf
            error: (GError**) error
{
    MOO_UNUSED_VAR (pf);
    MOO_UNUSED_VAR (error);
    g_return_val_if_reached (NO);
}

@end


// -*- objc -*-
