/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang-parser.h
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

#ifndef __MOO_LANG_PARSER_H__
#define __MOO_LANG_PARSER_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif

#include "mooedit/moolang-rules.h"

G_BEGIN_DECLS

typedef struct _LangXML LangXML;
typedef struct _GeneralXML GeneralXML;
typedef struct _SyntaxXML SyntaxXML;
typedef struct _KeywordXML KeywordXML;
typedef struct _ContextXML ContextXML;
typedef struct _StyleListXML StyleListXML;
typedef struct _StyleXML StyleXML;
typedef struct _RuleXML RuleXML;
typedef struct _RuleStringXML RuleStringXML;
typedef struct _RuleRegexXML RuleRegexXML;
typedef struct _RuleCharXML RuleCharXML;
typedef struct _Rule2CharXML Rule2CharXML;
typedef struct _RuleRangeXML RuleRangeXML;
typedef struct _RuleKeywordsXML RuleKeywordsXML;
typedef struct _RuleIncludeXML RuleIncludeXML;
typedef struct _CrossRef CrossRef;
typedef struct _CtxSwitchInfo CtxSwitchInfo;

struct _CrossRef {
    char *lang;     /* lang == NULL or lang name; NULL means current lang */
    char *name;     /* may not be NULL */
};

struct _CtxSwitchInfo {
    MooCtxSwitchType type;
    union {
        guint num;
        CrossRef ref;
    };
};

struct _LangXML {
    char *name;         /* mandatory, must be unique */
    char *section;
    char *version;
    char *mimetypes;
    char *extensions;
    char *author;
    guint hidden : 1;
    GeneralXML *general;
    SyntaxXML *syntax;
    StyleListXML *style_list;

    GHashTable *context_names; /* char* -> ContextXML* */
    GHashTable *style_names; /* char* -> StyleXML* */
    GHashTable *keyword_names; /* char* -> KeywordXML* */

    GSList *keyword_refs; /* char* */
    GHashTable *style_refs; /* char* -> NULL */
    GSList *internal_refs; /* CrossRef* */
    GSList *external_refs; /* CrossRef* */
};

struct _GeneralXML {
    char *brackets;
    char *single_line_start;
    char *multi_line_start;
    char *multi_line_end;
};

struct _SyntaxXML {
    GSList *keywords; /* KeywordXML* */
    guint n_keywords;
    GSList *contexts; /* ContextXML* */
    guint n_contexts;
};

struct _KeywordXML {
    char *name; /* mandatory, must be unique */
    GSList *words; /* char*; must not be empty */
    guint n_words;
};

struct _ContextXML {
    char *name; /* mandatory, must be unique */
    char *eol_context; /* optional, defaults to #stay */
    char *style; /* optional; if non-NULL, must be in styles */
    GSList *rules; /* RuleXML* */
    guint n_rules;

    CtxSwitchInfo eol_switch_info;
};

struct _StyleListXML {
    GSList *styles; /* StyleXML* */
    guint n_styles;
};

struct _StyleXML {
    char *name;
    char *default_style;
    char *bold;
    char *italic;
    char *underline;
    char *strikethrough;
    char *foreground;
    char *background;
};

#define RULE_XML_MEMBERS                                                                \
    char *style;                    /* optional */                                      \
    char *context;                  /* mandatory for IncludeRules;                      \
                                       optional for rest, defaults to #stay */          \
    guint includes_into_next : 1;   /* whether match is included into the context the   \
                                       rule switches to; optional, defaults to FALSE */ \
    guint bol_only : 1;             /* optional, defaults to FALSE */                   \
    guint first_non_blank_only : 1; /* optional, defaults to FALSE */                   \
    guint first_line_only : 1;      /* optional, defaults to FALSE */                   \
    guint caseless : 1;                                                                 \
                                                                                        \
    GSList *child_rules;            /* RuleXML* */                                      \
    CtxSwitchInfo end_switch_info;                                                      \
                                                                                        \
    MooRuleType type                                                                    \

struct _RuleXML {
    RULE_XML_MEMBERS;
};

struct _RuleStringXML {
    RULE_XML_MEMBERS;
    char *string;
};

struct _RuleRegexXML {
    RULE_XML_MEMBERS;
    char *pattern;
    guint non_empty : 1;
};

struct _RuleCharXML {
    RULE_XML_MEMBERS;
    char ch;
};

struct _Rule2CharXML {
    RULE_XML_MEMBERS;
    char chars[3];
};

struct _RuleRangeXML {
    RULE_XML_MEMBERS;
    char *range;
};

struct _RuleKeywordsXML {
    RULE_XML_MEMBERS;
    char *keywords;
};

struct _RuleIncludeXML {
    RULE_XML_MEMBERS;
    char *from_lang;
    char *from_context;
};


LangXML             *moo_lang_parse_file                (const char     *file);
LangXML             *moo_lang_parse_memory              (const char     *buffer,
                                                         int             size);
void                 moo_lang_xml_free                  (LangXML        *xml);

MooTextStyleScheme  *moo_text_style_scheme_parse_file   (const char     *file);
MooTextStyleScheme  *moo_text_style_scheme_parse_memory (const char     *buffer,
                                                         int             size);

MooRule             *moo_rule_new_from_xml              (RuleXML        *xml,
                                                         LangXML        *lang_xml,
                                                         MooLang        *lang);


G_END_DECLS

#endif /* __MOO_LANG_PARSER_H__ */
