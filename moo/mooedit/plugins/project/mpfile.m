#import "mpfile.h"

#define MP_FILE_VERSION "2.0"

@implementation MPFile

- (id) init
{
    if ((self = [super init]))
    {
        project_type = NULL;
        project_name = NULL;
        filename = NULL;
    }

    return self;
}

- (void) dealloc
{
    g_free (filename);
    g_free (project_type);
    g_free (project_name);
    if (xml)
        moo_markup_doc_unref (xml);
    [super dealloc];
}

- (BOOL) _open: (CSTR) filename_
         error: (GError**) error
{
    MooMarkupNode *root;
    const char *version, *type, *name;

    g_return_val_if_fail (filename_ != NULL, NO);
    g_return_val_if_fail (filename == NULL, NO);

    if (!(xml = moo_markup_parse_file (filename_, error)))
        return NO;

    if (!(root = moo_markup_get_root_element (xml, "medit-project")))
    {
        g_set_error (error, MP_FILE_ERROR, 0,
                     "malformed project file");
        return NO;
    }

    version = moo_markup_get_prop (root, "version");
    if (!version || strcmp (version, MP_FILE_VERSION) != 0)
    {
        g_set_error (error, MP_FILE_ERROR, 0,
                     "invalid project version %s",
                     version ? version : "<null>");
        return NO;
    }

    type = moo_markup_get_prop (root, "type");
    if (!type || !type[0])
    {
        g_set_error (error, MP_FILE_ERROR, 0,
                     "project type missing");
        return NO;
    }

    name = moo_markup_get_prop (root, "name");
    if (!name || !name[0])
    {
        g_set_error (error, MP_FILE_ERROR, 0,
                     "project name missing");
        return NO;
    }

    filename = g_strdup (filename_);
    project_name = g_strdup (name);
    project_type = g_strdup (type);

    return YES;
}

+ (id) open: (CSTR) filename
      error: (GError**) error
{
    MPFile *file = [[self alloc] init];

    if (file && [file _open:filename error:error])
        return file;

    if (file)
        [file release];

    return file;
}

- (CSTR) type
{
    return project_type;
}

- (CSTR) name
{
    return project_name;
}

@end

GQuark
_mp_file_error_quark (void)
{
    return g_quark_from_static_string ("moo-project-file-error");
}

// -*- objc -*-
