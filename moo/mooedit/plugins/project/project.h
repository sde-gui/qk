#ifndef MOO_PROJECT_H
#define MOO_PROJECT_H

#include <mooutils/moocobject.h>
#include <mooedit/mooeditwindow.h>
#include "mpfile.h"


@interface MPProject : MooCObject
{
@private
}

+ (id) loadFile: (MPFile*) pf
          error: (GError**) error;

- (BOOL) loadFile: (MPFile*) pf
            error: (GError**) error;

- (CSTR) name;

+ (void) registerProjectType: (CSTR) name;

@end


#endif // MOO_PROJECT_H
// -*- objc -*-
