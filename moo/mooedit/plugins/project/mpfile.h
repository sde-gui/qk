#ifndef MOO_PROJECT_FILE_H
#define MOO_PROJECT_FILE_H

#import <mooutils/moocobject.h>
#import <mooutils/moomarkup.h>

#define MP_FILE_ERROR (_mp_file_error_quark ())

@interface MPFile : MooCObject
{
@private
    char *filename;
    char *project_name;
    char *project_type;
    MooMarkupDoc *xml;
}

+ (id) open: (CSTR) filename
      error: (GError**) error;

- (CSTR) type;
- (CSTR) name;

@end

GQuark _mp_file_error_quark (void) G_GNUC_CONST;

#endif /* MOO_PROJECT_FILE_H */
// -*- objc -*-
