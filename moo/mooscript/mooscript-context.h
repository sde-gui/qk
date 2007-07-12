/*
 *   mooscript-context.h
 *
 *   Copyright (C) 2004-2007 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOO_SCRIPT_CONTEXT_H
#define MOO_SCRIPT_CONTEXT_H

#include <mooutils/moocobject.h>
#include <mooscript/mooscript-func.h>

G_BEGIN_DECLS


typedef struct _MSContextPrivate MSContextPrivate;

@interface MSVariable : MooCObject {
@private
    MSValue *value;
    MSFunc *func; /* called with no arguments */
};

+ (MSVariable*) new:(MSValue*) value;

- (void) setValue:(MSValue*) value;
- (MSValue*) value;
- (void) setFunc:(MSFunc*) func;
- (MSFunc*) func;
@end

typedef enum {
    MS_ERROR_NONE = 0,
    MS_ERROR_TYPE,
    MS_ERROR_VALUE,
    MS_ERROR_NAME,
    MS_ERROR_RUNTIME,
    MS_ERROR_LAST
} MSError;

@interface MSContext : MooCObject {
@private
    MSContextPrivate *priv;
}

+ (MSContext*) new;
+ (MSContext*) new:(gpointer) window;

- (void) setWindow:(gpointer) window;
- (gpointer) window;

- (MSValue*) evalVariable:(CSTR) name;
- (BOOL) assignVariable:(CSTR) name
                       :(MSValue*) value;
- (BOOL) assignPositional:(guint) n
                         :(MSValue*) value;
- (BOOL) assignString:(CSTR) name
                     :(CSTR) value;

- (MSValue*) getEnvVariable:(CSTR) name;

- (BOOL) setVar:(CSTR) name
               :(MSVariable*) var;

- (BOOL) setFunc:(CSTR) name
                :(MSFunc*) func;

- (MSValue*) setError:(MSError) error;
- (MSValue*) setError:(MSError) error
                     :(CSTR) message;
- (MSValue*) formatError:(MSError) error
                        :(CSTR) format,
                        ...;

- (CSTR) getErrorMsg;
- (void) clearError;
@end


G_END_DECLS

#endif /* MOO_SCRIPT_CONTEXT_H */
