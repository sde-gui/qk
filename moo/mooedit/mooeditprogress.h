#pragma once

#include "mooedit.h"
#include "mooeditwindow.h"

typedef struct MooEditProgress MooEditProgress;

#ifdef __cplusplus

#include <moocpp/gobjptrtypes.h>

using MooEditProgressPtr = moo::gobjptr<MooEditProgress>;
using MooEditProgressRef = moo::gobjref<MooEditProgress>;

MooEditProgressPtr  _moo_edit_progress_new             (void);
void                _moo_edit_progress_start           (MooEditProgress& progress,
                                                        const char*      text,
                                                        GDestroyNotify   cancel_func,
                                                        gpointer         cancel_func_data);
void                _moo_edit_progress_set_cancel_func (MooEditProgress& progress,
                                                        GDestroyNotify   cancel_func,
                                                        gpointer         cancel_func_data);
void                _moo_edit_progress_set_text        (MooEditProgress& progress,
                                                        const char*      text);

#endif __cplusplus
