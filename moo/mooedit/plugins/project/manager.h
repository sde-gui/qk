#ifndef MOO_PROJECT_MANAGER_H
#define MOO_PROJECT_MANAGER_H

#include <mooutils/moocobject.h>
#include <mooedit/mooeditwindow.h>

@class MPProject;

@interface MPManager : MooCObject
{
@private
    MPProject *project;
    MooEditWindow *window;
    char *filename;
}

- (void) deinit;
- (void) attachWindow: (MooEditWindow*) window;
- (void) detachWindow: (MooEditWindow*) window;

- (void) openProject: (CSTR) file;
- (void) closeProject;
- (void) projectOptions;
@end

#endif // MOO_PROJECT_MANAGER_H
// -*- objc -*-
