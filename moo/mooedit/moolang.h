/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang.h
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

#ifndef __MOO_LANG_H__
#define __MOO_LANG_H__

#include <gtk/gtktexttag.h>
#include <mooedit/mootextstylescheme.h>

G_BEGIN_DECLS


#define MOO_LANG_DIR_BASENAME   "syntax"
#define MOO_STYLES_PREFS_PREFIX MOO_EDIT_PREFS_PREFIX "/styles"

#define MOO_TYPE_LANG       (moo_lang_get_type ())


typedef struct _MooLang MooLang;
typedef struct _MooContext MooContext;
typedef struct _MooContextArray MooContextArray;
typedef struct _MooCtxSwitch MooCtxSwitch;
typedef struct _MooRule MooRule;
typedef struct _MooRuleArray MooRuleArray;

struct _MooLang {
    struct _MooLangMgr *mgr;

    GHashTable *context_names; /* char* -> MooContext* */
    MooContextArray *contexts;

    GHashTable *style_names; /* char* -> MooStyle* */
    MooTextStyleArray *styles;
    GHashTable *style_cache;

    char *name;         /* not NULL */
    char *section;      /* not NULL; "Others" by default */
    char *version;      /* not NULL; "" by default */
    char *author;       /* not NULL; "" by default */
    GSList *mime_types; /* list of mime types */
    GSList *extensions; /* list of globs */

    char *brackets;
    char *single_line_comment;
    char *multi_line_comment_start;
    char *multi_line_comment_end;

    char *sample;

    guint hidden : 1;
};


typedef enum {
    MOO_CONTEXT_STAY = 0,
    MOO_CONTEXT_POP,
    MOO_CONTEXT_SWITCH
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
    MOO_RULE_ASCII_STRING,
    MOO_RULE_REGEX,
    MOO_RULE_ASCII_CHAR,
    MOO_RULE_ASCII_2CHAR,
    MOO_RULE_ASCII_ANY_CHAR,
    MOO_RULE_INT,
    MOO_RULE_KEYWORDS,
    MOO_RULE_INCLUDE
} MooRuleType;

typedef enum {
    MOO_RULE_MATCH_FIRST_CHAR            = 1 << 0,
    MOO_RULE_MATCH_FIRST_NON_EMPTY_CHAR  = 1 << 1,
    MOO_RULE_MATCH_FIRST_LINE            = 1 << 2,
    MOO_RULE_MATCH_CASELESS              = 1 << 3,
    MOO_RULE_INCLUDE_INTO_NEXT           = 1 << 4
} MooRuleFlags;


typedef struct {
    char *string;
    guint length;
    guint caseless : 1;
} MooRuleAsciiString;

typedef struct {
    gpointer regex; /* EggRegex* */
    guint non_empty : 1;
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
    char *style;

    MooContext *context;
    MooCtxSwitch exit;

    MooRuleType type;
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


GType       moo_lang_get_type       (void) G_GNUC_CONST;


/*****************************************************************************/
/* MooContext
 */

void        moo_context_add_rule                (MooContext     *ctx,
                                                 MooRule        *rule);
void        moo_context_set_line_end_stay       (MooContext     *ctx);
void        moo_context_set_line_end_pop        (MooContext     *ctx,
                                                 guint           num);
void        moo_context_set_line_end_switch     (MooContext     *ctx,
                                                 MooContext     *target);


/*****************************************************************************/
/* MooLang
 */

MooLang    *moo_lang_new                        (struct _MooLangMgr *mgr,
                                                 const char         *name,
                                                 const char         *section,
                                                 const char         *version,
                                                 const char         *author);

MooLang    *moo_lang_ref                        (MooLang            *lang);
void        moo_lang_unref                      (MooLang            *lang);

MooContext *moo_lang_add_context                (MooLang            *lang,
                                                 const char         *name,
                                                 const char         *style);
MooContext *moo_lang_get_context                (MooLang            *lang,
                                                 const char         *ctx_name);
MooContext *moo_lang_get_default_context        (MooLang            *lang);
void        moo_lang_add_style                  (MooLang            *lang,
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
MooContext *_moo_text_iter_get_context          (const GtkTextIter  *iter);


G_END_DECLS

#endif /* __MOO_LANG_H__ */
