/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moolang-strings.h
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

#ifndef __MOO_LANG_STRINGS_H__
#define __MOO_LANG_STRINGS_H__

#ifndef MOOEDIT_COMPILATION
#error "This file may not be included"
#endif


#define DEF_STYLE_NORMAL        "Normal"
#define DEF_STYLE_KEYWORD       "Keyword"
#define DEF_STYLE_FUNCTION      "Function"
#define DEF_STYLE_DATA_TYPE     "DataType"
#define DEF_STYLE_DECIMAL       "Decimal"
#define DEF_STYLE_BASE_N        "BaseN"     /* octal and hex */
#define DEF_STYLE_FLOAT         "Float"
#define DEF_STYLE_CHAR          "Char"
#define DEF_STYLE_STRING        "String"
#define DEF_STYLE_COMMENT       "Comment"
#define DEF_STYLE_ALERT         "Alert"
#define DEF_STYLE_ERROR         "Error"
#define DEF_STYLE_OTHERS        "Others"
#define DEF_STYLE_PREPROCESSOR  "Preprocessor"

#define SCHEME_DEFAULT          "Default"

#define SCHEME_ELM                  "scheme"
#define DEFAULT_STYLE_ELM           "default-style"
#define SCHEME_NAME_PROP            "name"
#define DEFAULT_SCHEME_PROP         "default-scheme"
#define SCHEME_BASE_SCHEME_PROP     "base"
#define SCHEME_FOREGROUND_PROP      "foreground"
#define SCHEME_BACKGROUND_PROP      "background"
#define SCHEME_SEL_FOREGROUND_PROP  "selected-foreground"
#define SCHEME_SEL_BACKGROUND_PROP  "selected-background"
#define SCHEME_CURRENT_LINE_PROP    "current-line"
#define BRACKET_MATCH_ELM           "bracket-match"
#define BRACKET_MISMATCH_ELM        "bracket-mismatch"

#define LANGUAGE_ELM            "language"
#define GENERAL_ELM             "general"
#define KEYWORD_LIST_ELM        "keyword-list"
#define CONTEXT_ELM             "context"
#define SYNTAX_ELM              "syntax"
#define STYLE_ELM               "style"
#define STYLE_LIST_ELM          "styles"
#define KEYWORD_ELM             "keyword"
#define BRACKETS_ELM            "brackets"
#define COMMENTS_ELM            "comments"
#define SINGLE_LINE_ELM         "single-line"
#define MULTI_LINE_ELM          "multi-line"
#define SAMPLE_CODE_ELM         "sample-code"

#define LANG_NAME_PROP          "name"
#define LANG_VERSION_PROP       "version"
#define LANG_SECTION_PROP       "section"
#define LANG_MIME_TYPES_PROP    "mimetypes"
#define LANG_EXTENSIONS_PROP    "extensions"
#define LANG_AUTHOR_PROP        "author"
#define LANG_HIDDEN_PROP        "hidden"

#define KEYWORD_NAME_PROP       "name"
#define KEYWORD_WBNDRY_PROP     "word-boundary"
#define KEYWORD_PREFIX_PROP     "prefix"
#define KEYWORD_SUFFIX_PROP     "suffix"

#define CONTEXT_NAME_PROP       "name"
#define CONTEXT_STYLE_PROP      "style"
#define CONTEXT_EOL_CTX_PROP    "eol-context"

#define CONTEXT_STAY            "#stay"
#define CONTEXT_POP             "#pop"

#define STYLE_NAME_PROP             "name"
#define STYLE_DEF_STYLE_PROP        "default-style"
#define STYLE_BOLD_PROP             "bold"
#define STYLE_ITALIC_PROP           "italic"
#define STYLE_UNDERLINE_PROP        "underline"
#define STYLE_STRIKETHROUGH_PROP    "strikethrough"
#define STYLE_FOREGROUND_PROP       "foreground"
#define STYLE_BACKGROUND_PROP       "background"

#define COMMENT_START_PROP          "start"
#define COMMENT_END_PROP            "end"

#define RULE_STYLE_PROP             "style"
#define RULE_CTX_PROP               "context"
#define RULE_INCLUDE_EOL_PROP       "include-eol"
#define RULE_INCLUDE_PROP           "include-into-next"
#define RULE_BOL_PROP               "bol-only"
#define RULE_FIRST_NON_BLANK_PROP   "first-non-blank-only"
#define RULE_FIRST_LINE_PROP        "first-line-only"
#define RULE_CASELESS_PROP          "caseless"

#define RULE_INCLUDE_RULES_ELM      "IncludeRules"
#define RULE_ASCII_STRING_ELM       "String"
#define RULE_REGEX_ELM              "Regex"
#define RULE_ASCII_CHAR_ELM         "Char"
#define RULE_ASCII_2CHAR_ELM        "TwoChars"
#define RULE_ASCII_ANY_CHAR_ELM     "AnyChar"
#define RULE_KEYWORDS_ELM           "Keyword"
#define RULE_INT_ELM                "Int"
#define RULE_FLOAT_ELM              "Float"
#define RULE_HEX_ELM                "Hex"
#define RULE_OCTAL_ELM              "Octal"
#define RULE_C_CHAR_ELM             "CChar"
#define RULE_ESCAPED_CHAR_ELM       "EscapedChar"
#define RULE_WHITESPACE_ELM         "Whitespace"
#define RULE_IDENTIFIER_ELM         "Identifier"
#define RULE_LINE_CONTINUE_ELM      "LineContinue"

#define RULE_STRING_STRING_PROP     "string"
#define RULE_REGEX_PATTERN_PROP     "pattern"
#define RULE_REGEX_NON_EMPTY_PROP   "non-empty"
#define RULE_CHAR_CHAR_PROP         "char"
#define RULE_2CHAR_CHAR1_PROP       "char1"
#define RULE_2CHAR_CHAR2_PROP       "char2"
#define RULE_ANY_CHAR_CHARS_PROP    "chars"
#define RULE_KEYWORDS_KEYWORD_PROP  "keyword"
#define RULE_INCLUDE_FROM_PROP      "from"


#endif /* __MOO_LANG_STRINGS_H__ */
