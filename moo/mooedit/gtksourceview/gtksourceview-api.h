#ifndef __GTK_SOURCE_VIEW_API_H__
#define __GTK_SOURCE_VIEW_API_H__


#include "mooedit/gtksourceview/gtksourcelanguage-private-mangled.h"
#include "mooedit/gtksourceview/gtksourcelanguagemanager-mangled.h"
#include "mooedit/gtksourceview/gtksourcestylemanager-mangled.h"
#include "mooedit/gtksourceview/gtksourceiter-mangled.h"
#include "mooedit/gtksourceview/gtksourcestyle-private-mangled.h"

#define GtkSourceLanguage		MooGtkSourceLanguage
#define GtkSourceLanguageClass		MooGtkSourceLanguageClass
#define GtkSourceEngine			MooGtkSourceEngine
#define GtkSourceLanguageManager	MooGtkSourceLanguageManager
#define GtkSourceLanguageManagerClass	MooGtkSourceLanguageManagerClass
#define GtkSourceStyle			MooGtkSourceStyle
#define GtkSourceStyleScheme		MooGtkSourceStyleScheme
#define GtkSourceStyleManager		MooGtkSourceStyleManager
#define GtkSourceStyleManagerClass	MooGtkSourceStyleManagerClass
#define GtkSourceSearchFlags		MooGtkSourceSearchFlags

#define _gtk_source_language_create_engine			_moo_gtk_source_language_create_engine
#define _gtk_source_language_get_language_manager		_moo_gtk_source_language_get_language_manager
#define gtk_source_language_get_metadata			_moo_gtk_source_language_get_metadata
#define gtk_source_style_manager_new				_moo_gtk_source_style_manager_new
#define gtk_source_style_manager_set_search_path		_moo_gtk_source_style_manager_set_search_path
#define gtk_source_style_manager_get_scheme_ids			_moo_gtk_source_style_manager_get_scheme_ids
#define gtk_source_style_manager_get_scheme			_moo_gtk_source_style_manager_get_scheme
#define gtk_source_language_manager_list_languages		_moo_gtk_source_language_manager_list_languages
#define _gtk_source_engine_attach_buffer			_moo_gtk_source_engine_attach_buffer
#define _gtk_source_engine_text_inserted			_moo_gtk_source_engine_text_inserted
#define _gtk_source_engine_text_deleted				_moo_gtk_source_engine_text_deleted
#define _gtk_source_engine_attach_buffer			_moo_gtk_source_engine_attach_buffer
#define _gtk_source_engine_attach_buffer			_moo_gtk_source_engine_attach_buffer
#define _gtk_source_engine_set_style_scheme			_moo_gtk_source_engine_set_style_scheme
#define _gtk_source_engine_update_highlight			_moo_gtk_source_engine_update_highlight
#define _gtk_source_engine_set_style_scheme			_moo_gtk_source_engine_set_style_scheme
#define gtk_source_iter_forward_search				_moo_gtk_source_iter_forward_search
#define gtk_source_iter_backward_search				_moo_gtk_source_iter_backward_search
#define gtk_source_style_new					_moo_gtk_source_style_new
#define gtk_source_style_copy					_moo_gtk_source_style_copy
#define gtk_source_style_free					_moo_gtk_source_style_free
#define gtk_source_style_scheme_get_id				_moo_gtk_source_style_scheme_get_id
#define gtk_source_style_scheme_get_name			_moo_gtk_source_style_scheme_get_name
#define gtk_source_style_scheme_get_style			_moo_gtk_source_style_scheme_get_style
#define gtk_source_style_scheme_get_current_line_color		_moo_gtk_source_style_scheme_get_current_line_color
#define _gtk_source_style_scheme_apply				_moo_gtk_source_style_scheme_apply
#define _gtk_source_style_apply					_moo_gtk_source_style_apply


#endif /* __GTK_SOURCE_VIEW_API_H__ */
