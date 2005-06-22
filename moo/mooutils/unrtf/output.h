
/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001 Zachary Thayer Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The author is reachable by electronic mail at tuorfa@yahoo.com.
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    output
 * Author name:    Zach Smith
 * Create date:    18 Sep 01
 * Purpose:        Definitions for the generalized output module
 *----------------------------------------------------------------------
 * Changes:
 *--------------------------------------------------------------------*/


#ifndef _OUTPUT


typedef enum {
	OH_ATTR_DOCUMENT,
	OH_ATTR_HEADER,
	OH_ATTR_DOCUMENT_KEYWORDS,
	OH_ATTR_DOCUMENT_AUTHOR,
	OH_ATTR_DOCUMENT_CHANGEDATE,
	OH_ATTR_BODY,
	OH_ATTR_WORD,
	OH_ATTR_PARAGRAPH,
	OH_ATTR_CENTER,
	OH_ATTR_ALIGN_LEFT,
	OH_ATTR_ALIGN_RIGHT,
	OH_ATTR_JUSTIFY,
	
	OH_ATTR_LINE_BREAK,    /* no start/end */
	OH_ATTR_PAGE_BREAK,    /* no start/end */

	OH_ATTR_HYPERLINK,
	OH_ATTR_IMAGELINK,
	OH_ATTR_TABLE,
	OH_ATTR_TABLE_ROW,
	OH_ATTR_TABLE_CELL,

	/* standard font sizes are optional */
	OH_ATTR_FONTSIZE8,
	OH_ATTR_FONTSIZE10,
	OH_ATTR_FONTSIZE12,
	OH_ATTR_FONTSIZE14,
	OH_ATTR_FONTSIZE18,
	OH_ATTR_FONTSIZE24,
	OH_ATTR_FONTSIZE36,
	OH_ATTR_FONTSIZE48,

	OH_ATTR_SMALLER,
	OH_ATTR_BIGGER,
	OH_ATTR_FOREGROUND,
	OH_ATTR_BACKGROUND,
	OH_ATTR_BOLD,
	OH_ATTR_ITALIC,
	OH_ATTR_UNDERLINE,
	OH_ATTR_DBL_UNDERLINE,
	OH_ATTR_SUPERSCRIPT,
	OH_ATTR_SUBSCRIPT,
	OH_ATTR_STRIKETHROUGH,
	OH_ATTR_DBL_STRIKETHROUGH,
	OH_ATTR_EMBOSS,
	OH_ATTR_ENGRAVE,
	OH_ATTR_SHADOW,
	OH_ATTR_OUTLINE,
	OH_ATTR_SMALL_CAPS,
	OH_ATTR_POINTLIST,
	OH_ATTR_POINTLIST_ITEM,
	OH_ATTR_NUMERICLIST,
	OH_ATTR_NUMERICLIST_ITEM,
	OH_ATTR_TOC_ENTRY,
	OH_ATTR_INDEX_ENTRY
} OHAttrType;

typedef enum {
	OH_CHAR_BULLET,
	OH_CHAR_LEFT_QUOTE,
	OH_CHAR_RIGHT_QUOTE,
	OH_CHAR_LEFT_DBL_QUOTE,
	OH_CHAR_RIGHT_DBL_QUOTE,
	OH_CHAR_NONBREAKING_SPACE,
	OH_CHAR_EMDASH,
	OH_CHAR_ENDASH,
	OH_CHAR_LESSTHAN,
	OH_CHAR_GREATERTHAN,
	OH_CHAR_AMP,
	OH_CHAR_COPYRIGHT,
	OH_CHAR_TRADEMARK,
	OH_CHAR_NONBREAKING_HYPHEN,
	OH_CHAR_OPTIONAL_HYPHEN
} OHSpecialCharType;

#define OH_ATTR_START   1
#define OH_ATTR_END     0

typedef struct _OutputHandler OutputHandler;
typedef struct _OutputContext OutputContext;

struct _OutputContext {
        OutputHandler *oh;

        int charset_type; //=CHARSET_ANSI;
        
        /* Previously in word_print_core function
        */
        int total_chars_this_line; //=0; /* for simulating \tab */
        
        /* Nested tables aren't supported.
        */
        gboolean coming_pars_that_are_tabular; // = 0;
        gboolean within_table; // = FALSE;
        gboolean have_printed_row_begin; //=FALSE;
        gboolean have_printed_cell_begin; // =FALSE;
        gboolean have_printed_row_end; //=FALSE;
        gboolean have_printed_cell_end; //=FALSE;
        gboolean have_printed_body; //=FALSE;
        gboolean within_header; //=TRUE;
        gboolean inline_mode;

        /* This value is set by attr_push and attr_pop 
        */
        int simulate_smallcaps;
        int simulate_allcaps;

        gboolean within_picture; //=FALSE;
        int picture_file_number; //=1;
        char picture_path[255];
        int picture_width;
        int picture_height;
        int picture_bits_per_pixel; //=1;
        int picture_type; //=PICT_UNKNOWN;
        int picture_wmetafile_type;
        char *picture_wmetafile_type_str;

        char *hyperlink_base;// = NULL;

	gboolean simple_mode;   /* TODO: wtf is this? */
        gboolean debug_mode;
};

struct _OutputHandler {
	void (*attr) (OHAttrType attr, int start);
	void (*print) (const char *format, ...);
	void (*forced_space) (void);
	void (*comment) (const char *format, ...);
	void (*document_title) (const char *format, ...);
	void (*document_author) (const char *format, ...);
	void (*document_keywords) (const char *format, ...);
	void (*hyperlink) (const char *format, ...);
        void (*line_break) (void);
        void (*page_break) (void);
        void (*paragraph_begin) (void);
	void (*std_fontsize) (int size, int start);
	void (*font) (const char *font, int start);
	void (*fontsize) (int size_pt, int start);
	void (*foreground) (const char *color, int start);
	void (*background) (const char *color, int start);

	void (*expand) (const char *param, int start);

	void (*print_char) (OHSpecialCharType ch);
	void (*write_set_foreground) (int,int,int);

	char **ascii_translation_table;

	int simulate_small_caps : 1;
	int simulate_all_caps : 1;
	int simulate_word_underline : 1;

	char **ansi_translation_table;
	short ansi_first_char;
	short ansi_last_char;
	char **cp437_translation_table;
	short cp437_first_char;
	short cp437_last_char;
	char **cp850_translation_table;
	short cp850_first_char;
	short cp850_last_char;
	char **mac_translation_table;
	short mac_first_char;
	short mac_last_char;
};


typedef struct _OutputPersonality OutputPersonality;

OutputPersonality* op_create(void);
void op_free (OutputPersonality*);
char* op_translate_char (OutputPersonality*,int,int);

void op_begin_std_fontsize (OutputPersonality*, int);
void op_end_std_fontsize (OutputPersonality*, int);


#define _OUTPUT
#endif

