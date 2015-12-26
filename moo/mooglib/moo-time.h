#pragma once

#include <mooglib/moo-glib.h>

G_BEGIN_DECLS

typedef struct mgw_time_t mgw_time_t;

struct mgw_time_t
{
    gint64 value;
};

const struct tm *mgw_localtime(const mgw_time_t *timep);

G_END_DECLS
