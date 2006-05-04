/*
 *   moolang-private.h
 *
 *   Copyright (C) 2004-2006 by Yevgen Muntyan <muntyan@math.tamu.edu>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   See COPYING file that comes with this distribution.
 */

#ifndef MOOEDIT_COMPILATION
#error "This file may not be used"
#endif

#ifndef __MOO_LANG_PRIVATE_H__
#define __MOO_LANG_PRIVATE_H__

#include "mooedit/moolangmgr.h"
#include "mooedit/mooeditconfig.h"

G_BEGIN_DECLS


typedef struct _MooContext MooContext;
typedef struct _MooContextArray MooContextArray;
typedef struct _MooCtxSwitch MooCtxSwitch;
typedef struct _MooRule MooRule;
typedef struct _MooRuleArray MooRuleArray;
typedef struct _MooRuleMatchData MooRuleMatchData;
typedef struct _MooRuleMatchResult MooRuleMatchResult;

typedef struct _MooLangMgrClass MooLangMgrClass;

struct _MooLang {
    struct _MooLangMgr *mgr;

    GHashTable *context_names; /* char* -> MooContext* */
    MooContextArray *contexts;

    GHashTable *style_names; /* char* -> MooStyle* */
    MooTextStyleArray *styles;
    GHashTable *style_cache;

    char *id;           /* not NULL */
    char *display_name; /* not NULL */
    char *section;      /* not NULL; "Others" by default */
    char *version;      /* not NULL; "" by default */
    char *author;       /* not NULL; "" by default */
    GSList *mime_types; /* list of mime types */
    GSList *extensions; /* list of globs */

    char *brackets;
    char *line_comment;
    char *block_comment_start;
    char *block_comment_end;

    char *sample;

    guint hidden : 1;
};


struct _MooLangMgr {
    GObject base;

    GSList *lang_dirs;
    GSList *langs;
    GHashTable *lang_names;
    GHashTable *schemes;
    MooTextStyleScheme *active_scheme;
    GHashTable *extensions;
    GHashTable *mime_types;

    GHashTable *config;

    guint dirs_read : 1;
};

struct _MooLangMgrClass
{
    GObjectClass base_class;
};


typedef enum {
    MOO_CONTEXT_STAY = 0,
    MOO_CONTEXT_POP,
    MOO_CONTEXT_SWITCH /*,
    MOO_CONTEXT_JUMP */
} MooCtxSwitchType;

struct _MooCtxSwitch {
    MooCtxSwitchType type;

    union {
        guint num;
        MooContext *ctx;
    };
};


struct _MooRuleArray {
    MooRule **data;
    guint len;
};

struct _MooContextArray {
    MooContext **data;
    guint len;
};

struct _MooContext {
    MooLang *lang;
    char *name;
    char *style;

    MooRuleArray *rules;

    MooCtxSwitch line_end;
};


typedef enum {
    MOO_RULE_MATCH_FIRST_CHAR            = 1 << 0,
    MOO_RULE_MATCH_FIRST_NON_EMPTY_CHAR  = 1 << 1,
    MOO_RULE_MATCH_FIRST_LINE            = 1 << 2,
    MOO_RULE_MATCH_CASELESS              = 1 << 3,
    MOO_RULE_INCLUDE_INTO_NEXT           = 1 << 4
} MooRuleFlags;

typedef enum {
    MOO_RULE_MATCH_START_ONLY = 1 << 0
} MooRuleMatchFlags;


typedef struct {
    char *string;
    guint length;
    guint caseless : 1;
} MooRuleAsciiString;

typedef struct {
    gpointer regex; /* EggRegex* */
    guint non_empty : 1;
    guint left_word_bndry : 1;
    guint right_word_bndry : 1;
} MooRuleRegex;

typedef struct {
    char ch;
    guint caseless : 1;
} MooRuleAsciiChar;

typedef struct {
    char str[3];
} MooRuleAscii2Char;

typedef struct {
    char *chars;
    guint n_chars;
} MooRuleAsciiAnyChar;

typedef struct {
    MooContext *ctx;
} MooRuleInclude;


struct _MooRule
{
    MooRule* (*match)   (MooRule                *self,
                         const MooRuleMatchData *data,
                         MooRuleMatchResult     *result,
                         MooRuleMatchFlags       flags);
    void     (*destroy) (MooRule                *self);

    char *description;
    char *style;

    MooContext *context;
    MooCtxSwitch exit;

    gboolean include_eol;

    MooRuleFlags flags;

    MooRuleArray *child_rules;

    union {
        MooRuleAsciiString str;
        MooRuleRegex regex;
        MooRuleAsciiChar _char;
        MooRuleAscii2Char _2char;
        MooRuleAsciiAnyChar anychar;
        MooRuleInclude incl;
    };
};


/*****************************************************************************/
/* MooContext
 */

void        _moo_context_add_rule               (MooContext     *ctx,
                                                 MooRule        *rule);
void        _moo_context_set_eol_stay           (MooContext     *ctx);
void        _moo_context_set_eol_pop            (MooContext     *ctx,
                                                 guint           num);
void        _moo_context_set_eol_switch         (MooContext     *ctx,
                                                 MooContext     *target);
#if 0
void        moo_context_set_eol_jump            (MooContext     *ctx,
                                                 MooContext     *target);
#endif


/*****************************************************************************/
/* MooLang
 */

MooLang    *_moo_lang_new                       (struct _MooLangMgr *mgr,
                                                 const char         *name,
                                                 const char         *section,
                                                 const char         *version,
                                                 const char         *author);

MooContext *_moo_lang_add_context               (MooLang            *lang,
                                                 const char         *name,
                                                 const char         *style);
MooContext *_moo_lang_get_context               (MooLang            *lang,
                                                 const char         *ctx_name);
MooContext *_moo_lang_get_default_context       (MooLang            *lang);
void        _moo_lang_add_style                 (MooLang            *lang,
                                                 const char         *name,
                                                 const MooTextStyle *style);


/*****************************************************************************/
/* Auxiliary private methods
 */

void        _moo_lang_free                      (MooLang            *lang);

void        _moo_lang_scheme_changed            (MooLang            *lang);
void        _moo_lang_set_tag_style             (MooLang            *lang,
                                                 GtkTextTag         *tag,
                                                 MooContext         *ctx,
                                                 MooRule            *rule,
                                                 MooTextStyleScheme *scheme);
void        _moo_style_set_tag_style            (const MooTextStyle *style,
                                                 GtkTextTag         *tag);
void        _moo_lang_erase_tag_style           (GtkTextTag         *tag);

/* implemented in moohighlighter.c */
GtkTextTag *_moo_text_iter_get_syntax_tag       (const GtkTextIter  *iter);

/* implemented in moolangmgr.c */
void        _moo_lang_set_mime_types            (MooLang            *lang,
                                                 const char         *string);
void        _moo_lang_set_extensions            (MooLang            *lang,
                                                 const char         *string);


/*****************************************************************************/
/* MooLangMgr
 */

MooContext *_moo_lang_mgr_get_context           (MooLangMgr         *mgr,
                                                 const char         *lang_name,
                                                 const char         *ctx_name);
MooTextStyle *_moo_lang_mgr_get_style           (MooLangMgr         *mgr,
                                                 const char         *lang_name, /* default style if NULL */
                                                 const char         *style_name,
                                                 MooTextStyleScheme *scheme);
void        _moo_lang_mgr_set_active_scheme     (MooLangMgr         *mgr,
                                                 const char         *scheme_name);
void        _moo_lang_mgr_add_lang              (MooLangMgr         *mgr,
                                                 MooLang            *lang);
const char *_moo_lang_mgr_get_config            (MooLangMgr         *mgr,
                                                 const char         *lang_id);
void        _moo_lang_mgr_set_config            (MooLangMgr         *mgr,
                                                 const char         *lang_id,
                                                 const char         *config);
void        _moo_lang_mgr_update_config         (MooLangMgr         *mgr,
                                                 MooEditConfig      *config,
                                                 const char         *lang_id);
void        _moo_lang_mgr_load_config           (MooLangMgr         *mgr);
void        _moo_lang_mgr_save_config           (MooLangMgr         *mgr);


G_END_DECLS

#endif /* __MOO_LANG_H__ */
