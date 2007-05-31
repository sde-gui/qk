#ifndef G_REGEX_MANGLED_H
#define G_REGEX_MANGLED_H

#define MANGLE_REGEX_PREFIX moo
#define MANGLE_REGEX(func) MANGLE_REGEX2(MANGLE_REGEX_PREFIX,func)
#define MANGLE_REGEX2(prefix,func) MANGLE_REGEX3(prefix,func)
#define MANGLE_REGEX3(prefix,func) prefix##_##func

#define GRegexError MANGLE_REGEX(GRegexError)
#define g_regex_error_quark MANGLE_REGEX(g_regex_error_quark)
#define GRegexCompileFlags MANGLE_REGEX(GRegexCompileFlags)
#define GRegexMatchFlags MANGLE_REGEX(GRegexMatchFlags)
#define GRegexEvalCallback MANGLE_REGEX(GRegexEvalCallback)
#define _GRegex MANGLE_REGEX(_GRegex)
#define _GMatchInfo MANGLE_REGEX(_GMatchInfo)
#define GRegex MANGLE_REGEX(GRegex)
#define GMatchInfo MANGLE_REGEX(GMatchInfo)
#define g_regex_new MANGLE_REGEX(g_regex_new)
#define g_regex_ref MANGLE_REGEX(g_regex_ref)
#define g_regex_unref MANGLE_REGEX(g_regex_unref)
#define g_regex_get_pattern MANGLE_REGEX(g_regex_get_pattern)
#define g_regex_get_max_backref MANGLE_REGEX(g_regex_get_max_backref)
#define g_regex_get_capture_count MANGLE_REGEX(g_regex_get_capture_count)
#define g_regex_get_string_number MANGLE_REGEX(g_regex_get_string_number)
#define g_regex_escape_string MANGLE_REGEX(g_regex_escape_string)
#define g_regex_match_simple MANGLE_REGEX(g_regex_match_simple)
#define g_regex_match MANGLE_REGEX(g_regex_match)
#define g_regex_match_full MANGLE_REGEX(g_regex_match_full)
#define g_regex_match_all MANGLE_REGEX(g_regex_match_all)
#define g_regex_match_all_full MANGLE_REGEX(g_regex_match_all_full)
#define g_regex_split_simple MANGLE_REGEX(g_regex_split_simple)
#define g_regex_split MANGLE_REGEX(g_regex_split)
#define g_regex_split_full MANGLE_REGEX(g_regex_split_full)
#define g_regex_replace MANGLE_REGEX(g_regex_replace)
#define g_regex_replace_literal MANGLE_REGEX(g_regex_replace_literal)
#define g_regex_check_replacement MANGLE_REGEX(g_regex_check_replacement)
#define g_match_info_get_regex MANGLE_REGEX(g_match_info_get_regex)
#define g_match_info_get_string MANGLE_REGEX(g_match_info_get_string)
#define g_match_info_free MANGLE_REGEX(g_match_info_free)
#define g_match_info_next MANGLE_REGEX(g_match_info_next)
#define g_match_info_matches MANGLE_REGEX(g_match_info_matches)
#define g_match_info_get_match_count MANGLE_REGEX(g_match_info_get_match_count)
#define g_match_info_is_partial_match MANGLE_REGEX(g_match_info_is_partial_match)
#define g_match_info_expand_references MANGLE_REGEX(g_match_info_expand_references)
#define g_match_info_fetch MANGLE_REGEX(g_match_info_fetch)
#define g_match_info_fetch_pos MANGLE_REGEX(g_match_info_fetch_pos)
#define g_match_info_fetch_named MANGLE_REGEX(g_match_info_fetch_named)
#define g_match_info_fetch_named_pos MANGLE_REGEX(g_match_info_fetch_named_pos)
#define g_match_info_fetch_all MANGLE_REGEX(g_match_info_fetch_all)

#include "gregex-real.h"

#endif /* G_REGEX_MANGLED_H */
