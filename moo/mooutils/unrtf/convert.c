
/*===========================================================================
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
===========================================================================*/


/*----------------------------------------------------------------------
 * Module name:    convert
 * Author name:    Zach Smith
 * Create date:    24 Jul 01
 * Purpose:        Performs conversion from RTF to other formats.
 *----------------------------------------------------------------------
 * Changes:
 * 24 Jul 01, tuorfa@yahoo.com: moved code over from word.c
 * 24 Jul 01, tuorfa@yahoo.com: fixed color table reference numbering.
 * 30 Jul 01, tuorfa@yahoo.com: moved hex convert to util.c
 * 30 Jul 01, tuorfa@yahoo.com: moved special expr tables to special.c
 * 30 Jul 01, tuorfa@yahoo.com: moved attribute stack to attr.c
 * 31 Jul 01, tuorfa@yahoo.com: began addition of hash of rtf commands
 * 01 Aug 01, tuorfa@yahoo.com: finished bulk of rtf command hash
 * 03 Aug 01, tuorfa@yahoo.com: removed no-op hash entries to save space
 * 03 Aug 01, tuorfa@yahoo.com: code to ignore rest of groups for \*, etc
 * 03 Aug 01, tuorfa@yahoo.com: fixed para-alignnot being cleared by \pard
 * 03 Aug 01, tuorfa@yahoo.com: added support for \keywords group
 * 03 Aug 01, tuorfa@yahoo.com: added dummy funcs for header/footer
 * 03 Aug 01, tuorfa@yahoo.com: began addition of hyperlink support
 * 04 Aug 01, tuorfa@yahoo.com: fixed debug string printing
 * 05 Aug 01, tuorfa@yahoo.com: added support for hyperlink data with \field
 * 06 Aug 01, tuorfa@yahoo.com: added support for several font attributes
 * 08 Aug 01, gommer@gmx.net: bugfix for picture storing mechanism
 * 08 Sep 01, tuorfa@yahoo.com: added use of PROGRAM_NAME
 * 11 Sep 01, tuorfa@yahoo.com: added support for JPEG and PNG pictures
 * 19 Sep 01, tuorfa@yahoo.com: added output personality support
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 * 23 Sep 01, tuorfa@yahoo.com: fixed translation of \'XX expressions
 * 08 Oct 03, daved@physiol.usyd.edu.au: more special character code
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <glib.h>

#include "defs.h"
#include "parse.h"
#include "util.h"
#include "word.h"
#include "hash.h"
#include "convert.h"
#include "attr.h"


/*
#define BINARY_ATTRS
*/


/* Paragraph alignment (kludge)
 */
enum {
	ALIGN_LEFT=0,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_JUSTIFY
};


/* Most pictures must be written to files. */
enum {
	PICT_UNKNOWN=0,
	PICT_WM,
	PICT_MAC,
	PICT_PM,
	PICT_DI,
	PICT_WB,
	PICT_JPEG,
	PICT_PNG,
};


void starting_body (OutputContext *oc);
void starting_text (OutputContext *oc);


#if 1 /* daved - 0.19.0 */
char *entity(int symbol);
#endif



/*========================================================================
 * Name:	starting_body
 * Purpose:	Switches output stream for writing document contents.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void
starting_body (OutputContext *oc)
{
	if (!oc->have_printed_body) {
		if (!oc->inline_mode) {
                        oc->oh->attr (OH_ATTR_HEADER, 0);
                        oc->oh->attr (OH_ATTR_BODY, 1);
		}
		oc->within_header = FALSE;
		oc->have_printed_body = TRUE;
	}
}


/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/


static char *month_strings[12]= {
#ifdef ENGLISH
  "January","February","March","April","May","June","July","August",
  "September","October","November","December"
#endif
#ifdef FRANCAIS
  "Janvier","Fevrier","Mars","Avril","Mai","Juin","Juillet","Aout","Septembre",
  "Octobre","Novembre","Decembre"
#endif
#ifdef ITALIANO
  "Ianuario","Febbraio","Marzo","Aprile","Maggio","Iuno",
  "Luglio","Agusto","Settembre","Ottobre","Novembre","Dicembre"
#endif
#ifdef ESPANOL /* amaral - 0.19.2 */
  "Enero","Febrero","Marzo","Abril","Mayo","Junio","Julio","Agosto",
  "Septiembre","Octubre","Noviembre","Diciembre"
#endif
#ifdef DEUTCH
  "?","?","?","?","?","?","?","?",
  "?","?","?","?"
#endif
#ifdef PORTUGUES /* amaral - 0.19.2 */
  "Janeiro","Fevereiro","Marco","Abril","Maio","Junho","Julho","Agosto",
  "Setembro","Outubro","Novembro","Dezembro"
#endif
};


/*========================================================================
 * Name:	word_dump_date
 * Purpose:	Extracts date from an RTF input stream, writes it to
 *              output stream.
 * Args:	Word*, buffered RTF stream
 * Returns:	None.
 *=======================================================================*/

static void 
word_dump_date_comment (OutputContext *oc, Word *w)
{
	int year=0, month=0, day=0, hour=0, minute=0;
	g_assert (w != NULL);
	while (w) {
	 	char *s = word_string (w);
		if (*s == '\\') {
			++s;
			if (!strncmp (s, "yr", 2) && isdigit(s[2])) {
				year = atoi (&s[2]);
			}
			else if (!strncmp (s, "mo", 2) && isdigit(s[2])) {
				month= atoi (&s[2]);
			}
			else if (!strncmp (s, "dy", 2) && isdigit(s[2])) {
				day= atoi (&s[2]);
			}
			else if (!strncmp (s, "min", 3) && isdigit(s[3])) {
				minute= atoi (&s[3]);
			}
			else if (!strncmp (s, "hr", 2) && isdigit(s[2])) {
				hour= atoi (&s[2]);
			}
		}
		w=w->next;
	}
	if (year && month && day) {
		oc->oh->comment ("%d %s %d ", day, month_strings[month-1], year);
	}
	if (hour && minute) {
		oc->oh->comment ("%02d:%02d ", hour, minute);
	}
}



/*-------------------------------------------------------------------*/

typedef struct {
	int num;
	char *name;
} FontEntry;

#define MAX_FONTS (256)
static FontEntry font_table[MAX_FONTS];
static int total_fonts=0;



/*========================================================================
 * Name:	lookup_fontname
 * Purpose:	Fetches the name of a font from the already-read font table.
 * Args:	Font#.
 * Returns:	Font name.
 *=======================================================================*/

static char* 
lookup_fontname (int num) {
	int i;
	if (total_fonts) 
	for(i=0;i<total_fonts;i++) {
		if (font_table[i].num==num) 
			return font_table[i].name;
	}
	return NULL;
}


/*========================================================================
 * Name:	process_font_table
 * Purpose:	Processes the font table of an RTF file.
 * Args:	Tree of words.
 * Returns:	None.
 *=======================================================================*/

static void
process_font_table (OutputContext *oc, Word *w)
{
	Word *w2;

	g_assert (w != NULL);

	while(w) {
		int num;
		char name[255];
		char *tmp;

		if ((w2=w->child)) {
			tmp = word_string (w2);
			if (!strncmp("\\f",tmp,2)) {
				num = atoi (&tmp[2]);
				name[0]=0;

				w2=w2->next;
				while(w2) {
					tmp = word_string (w2);
					if (tmp && tmp[0] != '\\')
						strcat(name,tmp);

					w2=w2->next;
				}

				/* Chop the gall-derned semicolon. */
				if ((tmp=strchr(name,';')))
					*tmp=0;

				font_table[total_fonts].num = num;
				font_table[total_fonts].name = g_strdup (name);
				total_fonts++;
			}
		}
		w=w->next;
	}

        oc->oh->comment ("font table contains %d fonts total", total_fonts);

	if (oc->debug_mode) {
		int i;

                oc->oh->comment ("font table dump: \n");
		for (i=0; i<total_fonts; i++) {
                        oc->oh->comment (" font %d = %s\n", font_table[i].num,
				               font_table[i].name);
		}
	}
}


/*========================================================================
 * Name:	process_index_entry
 * Purpose:	Processes an index entry of an RTF file.
 * Args:	Tree of words.
 * Returns:	None.
 *=======================================================================*/

static void
process_index_entry (OutputContext *oc, Word *w)
{
	Word *w2;

	g_assert (w != NULL);

	while(w) {
		if ((w2=w->child)) {
			char *str = word_string (w2);

			if (oc->debug_mode && str) {
                                oc->oh->comment ("index entry word: %s ", str);
			}
		}
		w=w->next;
	}
}


/*========================================================================
 * Name:	process_toc_entry
 * Purpose:	Processes an index entry of an RTF file.
 * Args:	Tree of words, flag to say whether to include a page#.
 * Returns:	None.
 *=======================================================================*/

static void
process_toc_entry (OutputContext *oc, Word *w, int include_page_num)
{
	Word *w2;

	g_assert (w != NULL);

	while(w) {
		if ((w2=w->child)) {
			char *str = word_string (w2);

			if (oc->debug_mode && str) {
                                oc->oh->comment ("toc %s entry word: %s ",
					include_page_num ? "page#":"no page#",
					str);
			}
		}
		w=w->next;
	}
}


/*========================================================================
 * Name:	process_info_group
 * Purpose:	Processes the \info group of an RTF file.
 * Args:	Tree of words.
 * Returns:	None.
 *=======================================================================*/

static void
process_info_group (OutputContext *oc, Word *w)
{
	Word *child;

#if 1 /* amaral - 0.19.2 */
	/* g_assert (w != NULL); */
	if (!w) printf ("AUTHOR'S COMMENT: \\info command is null!\n"); 
#endif
	

	while(w) {
		child = w->child;
		if (child) {
			Word *w2;
			char *s;

			s = word_string(child);

			if (!oc->inline_mode) {
				if (!strcmp("\\title", s)) {
					w2=child->next;
					while (w2) {
						char *s2 = word_string(w2);
						if (s2[0] != '\\') 
							oc->oh->document_title ("%s", s2);
						w2=w2->next;
					}
				}
				else if (!strcmp("\\keywords", s)) {
					w2=child->next;
					while (w2) {
						char *s2 = word_string(w2);
						if (s2[0] != '\\') 
							oc->oh->document_keywords ("%s,", s2);
						w2=w2->next;
					}
				}
				else if (!strcmp("\\author", s)) {
					w2=child->next;
					while (w2) {
						char *s2 = word_string(w2);
						if (s2[0] != '\\') 
							printf ("%s", s2);
							oc->oh->document_author ("%s,", s2);
						w2=w2->next;
					}
				}
				else if (!strcmp("\\comment", s)) {
                                        oc->oh->comment ("comments: ");
					w2=child->next;
					while (w2) {
						char *s2 = word_string(w2);
						if (s2[0] != '\\') 
                                                        oc->oh->comment ("%s", s2);
						w2=w2->next;
					}
				}
				else if (!strncmp("\\nofpages", s, 9)) {
					oc->oh->comment ("total pages: %s",&s[9]);
				}
				else if (!strncmp("\\nofwords", s, 9)) {
					oc->oh->comment ("total words: %s",&s[9]);
				}
				else if (!strncmp("\\nofchars", s, 9) && isdigit(s[9])) {
					oc->oh->comment ("total chars: %s",&s[9]);
				}
				else if (!strcmp("\\creatim", s)) {
					oc->oh->comment ("creaton date: ");
					if (child->next) word_dump_date_comment (oc, child->next);
				}
				else if (!strcmp("\\printim", s)) {
					oc->oh->comment ("last printed: ");
					if (child->next) word_dump_date_comment (oc, child->next);
				}
				else if (!strcmp("\\buptim", s)) {
					oc->oh->comment ("last backup: ");
					if (child->next) word_dump_date_comment (oc, child->next);
				}
				else if (!strcmp("\\revtim", s)) {
					oc->oh->comment ("revision date: ");
					if (child->next) word_dump_date_comment (oc, child->next);
				}
			}

			/* Irregardless of whether we're in inline mode,
			 * we want to process the following.
			 */
			if (!strcmp("\\hlinkbase", s)) {
				char *linkstr = NULL;

				oc->oh->comment ("hyperlink base: ");
				if (child->next) {
					Word *nextword = child->next;

					if (nextword) 
						linkstr = word_string (nextword);
				}

				if (linkstr)
					oc->oh->comment ("%s", linkstr);
				else
					oc->oh->comment ("(none)");

				/* Store the pointer, it will remain good. */
				oc->hyperlink_base = linkstr; 
			}
		} 
		w = w->next;
	}
}

/*-------------------------------------------------------------------*/

/* RTF color table colors are RGB */

typedef struct {
	unsigned char r,g,b;
} Color;

#define MAX_COLORS (256)
static Color color_table[MAX_COLORS];
static int total_colors=0;


/*========================================================================
 * Name:	process_color_table
 * Purpose:	Processes the color table of an RTF file.
 * Args:	Tree of words.
 * Returns:	None.
 *=======================================================================*/

static void
process_color_table (OutputContext *oc, Word *w)
{
	int r,g,b;

	g_assert (w != NULL);

	/* Sometimes, RTF color tables begin with a semicolon,
	 * i.e. an empty color entry. This seems to indicate that color 0 
	 * will not be used, so here I set it to black.
	 */
	r=g=b=0;

	while(w) {
		char *s = word_string (w);

		if(!strncmp("\\red",s,4)) {
			r = atoi(&s[4]);
			while(r>255) r>>=8;
		}
		else if(!strncmp("\\green",s,6)) {
			g = atoi(&s[6]);
			while(g>255) g>>=8;
		}
		else if(!strncmp("\\blue",s,5)) {
			b = atoi(&s[5]);
			while(b>255) b>>=8;
		}
		else
		/* If we find the semicolon which denotes the end of
		 * a color entry then store the color, even if we don't
		 * have all of it.
		 */
		if (!strcmp (";", s)) {
			color_table[total_colors].r = r;
			color_table[total_colors].g = g;
			color_table[total_colors++].b = b;
			if (oc->debug_mode) {
				oc->oh->comment ("storing color entry %d: %02x%02x%02x",
					total_colors-1, r,g,b);
			}
			r=g=b=0;
		}

		w=w->next;
	}

	if (oc->debug_mode) {
	  	oc->oh->comment ("color table had %d entries -->\n", total_colors);
	}
}

/*========================================================================
 * Name:	cmd_cf
 * Purpose:	Executes the \cf command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_cf (OutputContext *oc, Word *w, int align, char has_param, short num) {
	char str[40];

	if (!has_param || num>=total_colors) {
		g_warning ("font color change attempted is invalid");
	}
	else
	{
		sprintf (str,"#%02x%02x%02x",
			color_table[num].r,
			color_table[num].g,
			color_table[num].b);
		attr_push (oc, ATTR_FOREGROUND, str);
	}
	return FALSE;
}



/*========================================================================
 * Name:	cmd_cb
 * Purpose:	Executes the \cb command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_cb (OutputContext *oc, Word *w, int align, char has_param, short num) {
	char str[40];

	if (!has_param || num>=total_colors) {
		g_warning ("font color change attempted is invalid");
	}
	else
	{
		sprintf (str,"#%02x%02x%02x",
			color_table[num].r,
			color_table[num].g,
			color_table[num].b);
		attr_push (oc, ATTR_BACKGROUND,str);
	}
	return FALSE;
}


/*========================================================================
 * Name:	cmd_fs
 * Purpose:	Executes the \fs command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_fs (OutputContext *oc, Word *w, int align, char has_param, short points) {
	char str[20];

	if (!has_param) return FALSE;

	/* Note, fs20 means 10pt */
	points /= 2;

	sprintf (str,"%d",points);
	attr_push (oc, ATTR_FONTSIZE,str);

	return FALSE;
}


/*========================================================================
 * Name:	cmd_field
 * Purpose:	Interprets fields looking for hyperlinks.
 * Comment:	Because hyperlinks are put in \field groups,
 *		we must interpret all \field groups, which is
 *		slow and laborious.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_field (OutputContext *oc, Word *w, int align, char has_param, short num) {
	Word *child;

	g_assert (w != NULL);

	while(w) {
		child = w->child;
		if (child) {
			Word *w2;
			char *s;

			s = word_string(child);

			if (!strcmp("\\*", s)) {
				w2=child->next;
				while (w2) {
					char *s2 = word_string(w2);
					if (s2 && !strcmp("\\fldinst", s2)) {
						Word *w3;
						w3=w2->next;
#if 1 /* daved - 0.19.0 */
						{
						char *s;
						char *s4;
						Word *w4;
						s = word_string(w3);
						if(s && !strcmp(s, "SYMBOL") )
						{
						w4=w3->next;
						while(w4 && !strcmp(word_string(w4), " "))
							w4 = w4->next;
						s4 = word_string(w4);
						if(s4)
							printf("%s", entity(atoi(s4)));
						}
						}
#endif
						while (w3 && !w3->child) {
							w3=w3->next;
						}
						if (w3) w3=w3->child;
						while (w3) {
							char *s3=word_string(w3);
							if (s3 && !strcmp("HYPERLINK",s3)) {
								Word *w4;
								char *s4;
								w4=w3->next;
								while (w4 && !strcmp(" ", word_string(w4)))
									w4=w4->next;
								if (w4) {
									s4=word_string(w4);
									oc->oh->hyperlink ("%s", s4);
									return TRUE;
								}
								
							}
							w3=w3->next;
						}
					}
					w2=w2->next;
				}
				
			}
		}
		w=w->next;
	}
	return TRUE;
}



/*========================================================================
 * Name:	cmd_f
 * Purpose:	Executes the \f command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_f (OutputContext *oc, Word *w, int align, char has_param, short num) {
	char *name;

	/* no param exit early XX */
	if (!has_param) 
		return FALSE;

	name = lookup_fontname(num);
	if(!name) {
		oc->oh->comment ("invalid font number %d",num);
	} else {
		attr_push (oc, ATTR_FONTFACE,name);
	}

	return FALSE;
}


/*========================================================================
 * Name:	cmd_highlight
 * Purpose:	Executes the \cf command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_highlight (OutputContext *oc, Word *w, int align, char has_param, short num) 
{
	char str[40];

	if (!has_param || num>=total_colors) {
		g_warning ("font background color change attempted is invalid");
	}
	else
	{
		sprintf (str,"#%02x%02x%02x",
			color_table[num].r,
			color_table[num].g,
			color_table[num].b);
		attr_push (oc, ATTR_BACKGROUND,str);
	}
	return FALSE;
}



/*========================================================================
 * Name:	cmd_tab
 * Purpose:	Executes the \tab command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_tab (OutputContext *oc, Word *w, int align, char has_param, short param) 
{
	/* Tab presents a genuine problem
	 * since some output formats don't have 
	 * an equivalent. As a kludge fix, I shall 
	 * assume the font is fixed width and that
	 * the tabstops are 8 characters apart.
	 */
	int need= 8 - (oc->total_chars_this_line % 8);
	oc->total_chars_this_line += need;
	while(need>0) {
		oc->oh->forced_space ();
		need--;
	}
	oc->oh->print ("\n");
	return FALSE;
}


/*========================================================================
 * Name:	cmd_plain
 * Purpose:	Executes the \plain command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_plain (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_pop_all (oc);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_fnil
 * Purpose:	Executes the \fnil command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_fnil (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTNIL_STR);
	return FALSE;
}



/*========================================================================
 * Name:	cmd_froman
 * Purpose:	Executes the \froman command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_froman (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTROMAN_STR);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_fswiss
 * Purpose:	Executes the \fswiss command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_fswiss (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTSWISS_STR);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_fmodern
 * Purpose:	Executes the \fmodern command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_fmodern (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTMODERN_STR);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_fscript
 * Purpose:	Executes the \fscript command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_fscript (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTSCRIPT_STR);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_fdecor
 * Purpose:	Executes the \fdecor command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_fdecor (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTDECOR_STR);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_ftech
 * Purpose:	Executes the \ftech command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_ftech (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_FONTFACE,FONTTECH_STR);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_expand
 * Purpose:	Executes the \expand command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_expand (OutputContext *oc, Word *w, int align, char has_param, short param) {
	char str[10];
	if (has_param) {
		sprintf (str, "%d", param/4);
		if (!param) 
			attr_pop (oc, ATTR_EXPAND);
		else 
			attr_push (oc, ATTR_EXPAND, str);
	}
	return FALSE;
}


/*========================================================================
 * Name:	cmd_emboss
 * Purpose:	Executes the \embo command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_emboss (OutputContext *oc, Word *w, int align, char has_param, short param) {
	char str[10];
	if (has_param && !param)
		attr_pop (oc, ATTR_EMBOSS);
	else
	{
		sprintf (str, "%d", param);
		attr_push (oc, ATTR_EMBOSS, str);
	}
	return FALSE;
}


/*========================================================================
 * Name:	cmd_engrave
 * Purpose:	Executes the \impr command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_engrave (OutputContext *oc, Word *w, int align, char has_param, short param) {
	char str[10];
	if (has_param && !param) 
		attr_pop (oc, ATTR_ENGRAVE);
	else
	{
		sprintf (str, "%d", param);
		attr_push (oc, ATTR_ENGRAVE, str);
	}
	return FALSE;
}

/*========================================================================
 * Name:	cmd_caps
 * Purpose:	Executes the \caps command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_caps (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && !param)
		attr_pop (oc, ATTR_CAPS);
	else 
		attr_push (oc, ATTR_CAPS,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_scaps
 * Purpose:	Executes the \scaps command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_scaps (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && !param)
		attr_pop (oc, ATTR_SMALLCAPS);
	else 
		attr_push (oc, ATTR_SMALLCAPS,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_bullet
 * Purpose:	Executes the \bullet command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_bullet (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_BULLET);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}

/*========================================================================
 * Name:	cmd_ldblquote
 * Purpose:	Executes the \ldblquote command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_ldblquote (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_LEFT_DBL_QUOTE);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_rdblquote
 * Purpose:	Executes the \rdblquote command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_rdblquote (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_RIGHT_DBL_QUOTE);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_lquote
 * Purpose:	Executes the \lquote command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_lquote (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_LEFT_QUOTE);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_nonbreaking_space
 * Purpose:	Executes the nonbreaking space command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_nonbreaking_space (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_NONBREAKING_SPACE);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_nonbreaking_hyphen
 * Purpose:	Executes the nonbreaking hyphen command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_nonbreaking_hyphen (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_NONBREAKING_HYPHEN);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_optional_hyphen
 * Purpose:	Executes the optional hyphen command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_optional_hyphen (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_OPTIONAL_HYPHEN);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_emdash
 * Purpose:	Executes the \emdash command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_emdash (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_EMDASH);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_endash
 * Purpose:	Executes the \endash command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_endash (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_ENDASH);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_rquote
 * Purpose:	Executes the \rquote command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_rquote (OutputContext *oc, Word *w, int align, char has_param, short param) {
        oc->oh->print_char (OH_CHAR_RIGHT_QUOTE);
	++oc->total_chars_this_line; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_par
 * Purpose:	Executes the \par command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int 
cmd_par (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->oh->line_break ();
	oc->total_chars_this_line = 0; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_line
 * Purpose:	Executes the \line command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_line (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->oh->line_break ();
	oc->total_chars_this_line = 0; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_page
 * Purpose:	Executes the \page command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_page (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->oh->page_break ();
	oc->total_chars_this_line = 0; /* \tab */
	return FALSE;
}


/*========================================================================
 * Name:	cmd_intbl
 * Purpose:	Executes the \intbl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_intbl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	++oc->coming_pars_that_are_tabular;
	return FALSE;
}


/*========================================================================
 * Name:	cmd_ulnone
 * Purpose:	Executes the \ulnone command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ulnone (OutputContext *oc, Word *w, int align, char has_param, short param) {
	int attr, more=TRUE;
#ifdef BINARY_ATTRS
	attr_remove_underlining ();
#else
	do {
		attr = attr_read ();
		if (attr==ATTR_UNDERLINE ||
		    attr==ATTR_DOT_UL ||
		    attr==ATTR_DASH_UL ||
		    attr==ATTR_DOT_DASH_UL ||
		    attr==ATTR_2DOT_DASH_UL ||
		    attr==ATTR_WORD_UL ||
		    attr==ATTR_WAVE_UL ||
		    attr==ATTR_THICK_UL ||
		    attr==ATTR_DOUBLE_UL)
		{
		  if (!attr_pop (oc, ATTR_UNDERLINE))
		    ;
		} else
		  more=FALSE;
	} while(more);
#endif
	return FALSE;
}

/*========================================================================
 * Name:	cmd_ul
 * Purpose:	Executes the \ul command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ul (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param == 0) {
		cmd_ulnone (oc, w, align, has_param, param);
	} else {
		attr_push (oc, ATTR_UNDERLINE,NULL);
	}
	return FALSE;
}

/*========================================================================
 * Name:	cmd_uld
 * Purpose:	Executes the \uld command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_uld (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_DOUBLE_UL,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_uldb
 * Purpose:	Executes the \uldb command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_uldb (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_DOT_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_uldash
 * Purpose:	Executes the \uldash command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_uldash (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_DASH_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_uldashd
 * Purpose:	Executes the \cmd_uldashd command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_uldashd (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_DOT_DASH_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_uldashdd
 * Purpose:	Executes the \uldashdd command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_uldashdd (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_2DOT_DASH_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_ulw
 * Purpose:	Executes the \ulw command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ulw (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_WORD_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_ulth
 * Purpose:	Executes the \ulth command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ulth (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_THICK_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_ulwave
 * Purpose:	Executes the \ulwave command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ulwave (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_push (oc, ATTR_WAVE_UL,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_strike
 * Purpose:	Executes the \strike command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_strike (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_STRIKE);
	else
		attr_push (oc, ATTR_STRIKE,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_strikedl
 * Purpose:	Executes the \strikedl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_strikedl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_DBL_STRIKE);
	else
		attr_push (oc, ATTR_DBL_STRIKE,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_striked
 * Purpose:	Executes the \striked command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_striked (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_DBL_STRIKE);
	else
		attr_push (oc, ATTR_DBL_STRIKE,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_rtf
 * Purpose:	Executes the \rtf command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_rtf (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return FALSE;
}


/*========================================================================
 * Name:	cmd_up
 * Purpose:	Executes the \up command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_up (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_SUPER);
	else
		attr_push (oc, ATTR_SUPER,NULL);
	return FALSE;
}


/*========================================================================
 * Name:	cmd_dn
 * Purpose:	Executes the \dn command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_dn (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_SUB);
	else
		attr_push (oc, ATTR_SUB,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_nosupersub
 * Purpose:	Executes the \nosupersub command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_nosupersub (OutputContext *oc, Word *w, int align, char has_param, short param) {
	attr_pop (oc, ATTR_SUPER);
	attr_pop (oc, ATTR_SUB);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_super
 * Purpose:	Executes the \super command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_super (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_SUPER);
	else
		attr_push (oc, ATTR_SUPER,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_sub
 * Purpose:	Executes the \sub command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_sub (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_SUB);
	else
		attr_push (oc, ATTR_SUB,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_shad
 * Purpose:	Executes the \shad command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_shad (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_SHADOW);
	else
		attr_push (oc, ATTR_SHADOW,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_b
 * Purpose:	Executes the \b command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int 
cmd_b (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0) {
		attr_pop (oc, ATTR_BOLD);
	}
	else
		attr_push (oc, ATTR_BOLD,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_i
 * Purpose:	Executes the \i command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_i (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_ITALIC);
	else
		attr_push (oc, ATTR_ITALIC,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_s
 * Purpose:	Executes the \s command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/
static int cmd_s (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return FALSE;
}

/*========================================================================
 * Name:	cmd_sect
 * Purpose:	Executes the \sect command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_sect (OutputContext *oc, Word *w, int align, char has_param, short param) {
	/* XX kludge */
	oc->oh->paragraph_begin ();
	return FALSE;
}

/*========================================================================
 * Name:	cmd_shp
 * Purpose:	Executes the \shp command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_shp (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->oh->comment ("Drawn Shape (ignored--not implemented yet)");
	return FALSE;
}

/*========================================================================
 * Name:	cmd_outl
 * Purpose:	Executes the \outl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_outl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (has_param && param==0)
		attr_pop (oc, ATTR_OUTLINE);
	else
		attr_push (oc, ATTR_OUTLINE,NULL);
	return FALSE;
}

/*========================================================================
 * Name:	cmd_ansi
 * Purpose:	Executes the \ansi command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ansi (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->charset_type = CHARSET_ANSI;
	oc->oh->comment ("document uses ANSI character set");
	return FALSE;
}

/*========================================================================
 * Name:	cmd_pc
 * Purpose:	Executes the \pc command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pc (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->charset_type = CHARSET_CP437 ;
	oc->oh->comment ("document uses PC codepage 437 character set");
	return FALSE;
}

/*========================================================================
 * Name:	cmd_pca
 * Purpose:	Executes the \pca command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pca (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->charset_type = CHARSET_CP850;
	oc->oh->comment ("document uses PC codepage 850 character set");
	return FALSE;
}

/*========================================================================
 * Name:	cmd_mac
 * Purpose:	Executes the \mac command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_mac (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->charset_type = CHARSET_MAC;
	oc->oh->comment ("document uses Macintosh character set");
	return FALSE;
}

/*========================================================================
 * Name:	cmd_colortbl
 * Purpose:	Executes the \colortbl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_colortbl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (w->next) {
		process_color_table (oc, w->next);
	}
	return TRUE;
}

/*========================================================================
 * Name:	cmd_fonttbl
 * Purpose:	Executes the \fonttbl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_fonttbl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (w->next) {
		process_font_table (oc, w->next);
	}
	return TRUE;
}

/*========================================================================
 * Name:	cmd_header
 * Purpose:	Executes the \header command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_header (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_headerl
 * Purpose:	Executes the \headerl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_headerl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_headerr
 * Purpose:	Executes the \headerr command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_headerr (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_headerf
 * Purpose:	Executes the \headerf command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_headerf (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_footer
 * Purpose:	Executes the \footer command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_footer (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_footerl
 * Purpose:	Executes the \footerl command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_footerl (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_footerr
 * Purpose:	Executes the \footerr command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_footerr (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_footerf
 * Purpose:	Executes the \footerf command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_footerf (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_ignore
 * Purpose:	Dummy function to get rid of subgroups 
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_ignore (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return TRUE;
}

/*========================================================================
 * Name:	cmd_info
 * Purpose:	Executes the \info command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_info (OutputContext *oc, Word *w, int align, char has_param, short param) {
	process_info_group (oc, w->next);
	return TRUE;
}

/*========================================================================
 * Name:	cmd_pict
 * Purpose:	Executes the \pict command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pict (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->within_picture = TRUE;
	oc->picture_width = oc->picture_height = 0;
	oc->picture_type = PICT_WB;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_bin
 * Purpose:	Executes the \bin command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_bin (OutputContext *oc, Word *w, int align, char has_param, short param) {
	return FALSE;		
}


/*========================================================================
 * Name:	cmd_macpict
 * Purpose:	Executes the \macpict command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_macpict (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->picture_type = PICT_MAC;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_jpegblip
 * Purpose:	Executes the \jpegblip command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_jpegblip (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->picture_type = PICT_JPEG;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_pngblip
 * Purpose:	Executes the \pngblip command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pngblip (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->picture_type = PICT_PNG;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_pnmetafile
 * Purpose:	Executes the \pnmetafile command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pnmetafile (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->picture_type = PICT_PM;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_wmetafile
 * Purpose:	Executes the \wmetafile command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_wmetafile (OutputContext *oc, Word *w, int align, char has_param, short param) {
	oc->picture_type = PICT_WM;
	if (oc->within_picture && has_param) {
		oc->picture_wmetafile_type = param;
		switch(param) {
		case 1: oc->picture_wmetafile_type_str="MM_TEXT"; break;
		case 2: oc->picture_wmetafile_type_str="MM_LOMETRIC"; break;
		case 3: oc->picture_wmetafile_type_str="MM_HIMETRIC"; break;
		case 4: oc->picture_wmetafile_type_str="MM_LOENGLISH"; break;
		case 5: oc->picture_wmetafile_type_str="MM_HIENGLISH"; break;
		case 6: oc->picture_wmetafile_type_str="MM_TWIPS"; break;
		case 7: oc->picture_wmetafile_type_str="MM_ISOTROPIC"; break;
		case 8: oc->picture_wmetafile_type_str="MM_ANISOTROPIC"; break;
		default: oc->picture_wmetafile_type_str="default:MM_TEXT"; break;
		}
	}
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_wbmbitspixel
 * Purpose:	Executes the \wbmbitspixel command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_wbmbitspixel (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (oc->within_picture && has_param) 
		oc->picture_bits_per_pixel = param;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_picw
 * Purpose:	Executes the \picw command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_picw (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (oc->within_picture && has_param) 
		oc->picture_width = param;
	return FALSE;		
}

/*========================================================================
 * Name:	cmd_pich
 * Purpose:	Executes the \pich command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_pich (OutputContext *oc, Word *w, int align, char has_param, short param) {
	if (oc->within_picture && has_param) 
		oc->picture_height = param;
	return FALSE;		
}


/*========================================================================
 * Name:	cmd_xe
 * Purpose:	Executes the \xe (index entry) command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_xe (OutputContext *oc, Word *w, int align, char has_param, short param) {
	process_index_entry (oc, w);
	return TRUE;		
}

/*========================================================================
 * Name:	cmd_tc
 * Purpose:	Executes the \tc (TOC entry) command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_tc (OutputContext *oc, Word *w, int align, char has_param, short param) {
	process_toc_entry (oc, w, TRUE);
	return TRUE;		
}

/*========================================================================
 * Name:	cmd_tcn
 * Purpose:	Executes the \tcn (TOC entry, no page #) command.
 * Args:	Word, paragraph align info, and numeric param if any.
 * Returns:	Flag, true only if rest of Words on line should be ignored.
 *=======================================================================*/

static int cmd_tcn (OutputContext *oc, Word *w, int align, char has_param, short param) {
	process_toc_entry (oc, w, FALSE);
	return TRUE;		
}


typedef struct {
	char *name;
	int (*func)(Word*,int,char,short);
	char *debug_print;
}
HashItem;



static HashItem hashArray_other [] = {
	{ "*", cmd_ignore, NULL },
	{ "-", cmd_optional_hyphen, "optional hyphen" },
	{ "_", cmd_nonbreaking_hyphen, "nonbreaking hyphen" },
	{ "~", cmd_nonbreaking_space, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_a [] = {
	{ "ansi", &cmd_ansi , NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_b [] = {
	{ "b", &cmd_b, NULL },
	{ "bullet", &cmd_bullet, NULL },
	{ "bin", &cmd_bin, "picture is binary" },
#if 0
	{ "bgbdiag", NULL, NULL },
	{ "bgcross", NULL, NULL },
	{ "bgdcross", NULL, NULL },
	{ "bgfdiag", NULL, NULL },
	{ "bghoriz", NULL, NULL },
	{ "bgkbdiag", NULL, NULL },
	{ "bgkcross", NULL, NULL },
	{ "bgkdcross", NULL, NULL },
	{ "bgkfdiag", NULL, NULL },
	{ "bgkhoriz", NULL, NULL },
	{ "bgkvert", NULL, NULL },
	{ "bgvert", NULL, NULL },
	{ "brdrcf", NULL, NULL },
	{ "brdrdb", NULL, NULL },
	{ "brdrdot", NULL, NULL },
	{ "brdrhair", NULL, NULL },
	{ "brdrs", NULL, NULL },
	{ "brdrsh", NULL, NULL },
	{ "brdrth", NULL, NULL },
	{ "brdrw", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_c [] = {
	{ "caps", &cmd_caps, NULL },
	{ "cb", cmd_cb, NULL },
	{ "cf", cmd_cf, NULL },
	{ "colortbl", &cmd_colortbl, "color table" },
	{ "cols", NULL, "columns (not implemented)" },
	{ "column", NULL, "column break (not implemented)" },
#if 0
	{ "cbpat", NULL, NULL },
	{ "cellx", NULL, NULL },
	{ "cfpat", NULL, NULL },
	{ "cgrid", NULL, NULL },
	{ "clbgbcross", NULL, NULL },
	{ "clbgbdiag", NULL, NULL },
	{ "clbgbkbdiag", NULL, NULL },
	{ "clbgbkcross", NULL, NULL },
	{ "clbgbkdcross", NULL, NULL },
	{ "clbgbkfdiag", NULL, NULL },
	{ "clbgbkhor", NULL, NULL },
	{ "clbgbkvert", NULL, NULL },
	{ "clbgdcross", NULL, NULL },
	{ "clbgfdiag", NULL, NULL },
	{ "clbghoriz", NULL, NULL },
	{ "clbgvert", NULL, NULL },
	{ "clbrdrb", NULL, NULL },
	{ "clbrdrl", NULL, NULL },
	{ "clbrdrr", NULL, NULL },
	{ "clbrdrt", NULL, NULL },
	{ "clcbpat", NULL, NULL },
	{ "clcfpat", NULL, NULL },
	{ "clmgf", NULL, NULL },
	{ "clmrg", NULL, NULL },
	{ "clshdng", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_d [] = {
	{ "dn", &cmd_dn, NULL },
#if 0
	{ "dibitmap", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_e [] = {
	{ "emdash", cmd_emdash, NULL },
	{ "endash", cmd_endash, NULL },
	{ "embo", &cmd_emboss, NULL },
	{ "expand", &cmd_expand, NULL },
	{ "expnd", &cmd_expand, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_f [] = {
	{ "f", cmd_f, NULL },
	{ "fdecor", cmd_fdecor, NULL },
	{ "fmodern", cmd_fmodern, NULL },
	{ "fnil", cmd_fnil, NULL },
	{ "fonttbl", cmd_fonttbl, "font table" },
	{ "froman", cmd_froman, NULL },
	{ "fs", cmd_fs, NULL },
	{ "fscript", cmd_fscript, NULL },
	{ "fswiss", cmd_fswiss, NULL },
	{ "ftech", cmd_ftech, NULL },
	{ "field", cmd_field, NULL },
	{ "footer", cmd_footer, NULL },
	{ "footerf", cmd_footerf, NULL },
	{ "footerl", cmd_footerl, NULL },
	{ "footerr", cmd_footerr, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_h [] = {
	{ "highlight", &cmd_highlight, NULL },
	{ "header", cmd_header, NULL },
	{ "headerf", cmd_headerf, NULL },
	{ "headerl", cmd_headerl, NULL },
	{ "headerr", cmd_headerr, NULL },
	{ "hl", cmd_ignore, "hyperlink within object" },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_i [] = {
	{ "i", &cmd_i, NULL },
	{ "info", &cmd_info, NULL },
	{ "intbl", &cmd_intbl, NULL },
	{ "impr", &cmd_engrave, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_j [] = {
	{ "jpegblip", &cmd_jpegblip, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_l [] = {
	{ "ldblquote", &cmd_ldblquote, NULL },
	{ "line", &cmd_line, NULL },
	{ "lquote", &cmd_lquote, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_m [] = {
	{ "mac", &cmd_mac , NULL },
	{ "macpict", &cmd_macpict, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_n [] = {
	{ "nosupersub", &cmd_nosupersub, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_o [] = {
	{ "outl", &cmd_outl, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_p [] = {
	{ "page", &cmd_page, NULL },
	{ "par", &cmd_par, NULL },
	{ "pc", &cmd_pc , NULL },
	{ "pca", &cmd_pca , NULL },
	{ "pich", &cmd_pich, NULL },
	{ "pict", &cmd_pict, "picture" },
	{ "picw", &cmd_picw, NULL },
	{ "plain", &cmd_plain, NULL },
	{ "pngblip", &cmd_pngblip, NULL },
	{ "pnmetafile", &cmd_pnmetafile, NULL },
#if 0
	{ "piccropb", NULL, NULL },
	{ "piccropl", NULL, NULL },
	{ "piccropr", NULL, NULL },
	{ "piccropt", NULL, NULL },
	{ "pichgoal", NULL, NULL },
	{ "pichgoal", NULL, NULL },
	{ "picscaled", NULL, NULL },
	{ "picscalex", NULL, NULL },
	{ "picwgoal", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_r [] = {
	{ "rdblquote", &cmd_rdblquote, NULL },
	{ "rquote", &cmd_rquote, NULL },
	{ "rtf", &cmd_rtf, NULL },
	{ NULL, NULL, NULL}
};
static HashItem hashArray_s [] = {
	{ "s", cmd_s, "style" },
	{ "sect", &cmd_sect, "section break"},
	{ "scaps", &cmd_scaps, NULL },
	{ "super", &cmd_super, NULL },
	{ "sub", &cmd_sub, NULL },
	{ "shad", &cmd_shad, NULL },
	{ "strike", &cmd_strike, NULL },
	{ "striked", &cmd_striked, NULL },
	{ "strikedl", &cmd_strikedl, NULL },
	{ "stylesheet", &cmd_ignore, "style sheet" },
	{ "shp", cmd_shp, "drawn shape" },
#if 0
	{ "shading", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_t [] = {
	{ "tab", &cmd_tab, NULL },
	{ "tc", cmd_tc, "TOC entry" },
	{ "tcn", cmd_tcn, "TOC entry" },
#if 0
	{ "tcf", NULL , NULL },
	{ "tcl", NULL , NULL },
	{ "trgaph", NULL , NULL },
	{ "trleft", NULL , NULL },
	{ "trowd", NULL , NULL },
	{ "trqc", NULL , NULL },
	{ "trql", NULL , NULL },
	{ "trqr", NULL , NULL },
	{ "trrh", NULL , NULL },
#endif
	{ NULL, NULL, NULL}
};
static HashItem hashArray_u [] = {
	{ "ul", &cmd_ul, NULL },
	{ "up", &cmd_up, NULL },
	{ "uld", &cmd_uld, NULL },
	{ "uldash", &cmd_uldash, NULL },
	{ "uldashd", &cmd_uldashd, NULL },
	{ "uldashdd", &cmd_uldashdd, NULL },
	{ "uldb", &cmd_uldb, NULL },
	{ "ulnone", &cmd_ulnone, NULL },
	{ "ulth", &cmd_ulth, NULL },
	{ "ulw", &cmd_ulw, NULL },
	{ "ulwave", &cmd_ulwave, NULL },
	{ NULL, NULL, NULL}
};

static HashItem hashArray_w [] = {
	{ "wbmbitspixel", &cmd_wbmbitspixel, NULL },
	{ "wmetafile", &cmd_wmetafile, NULL },
#if 0
	{ "wbitmap", NULL, NULL },
	{ "wbmplanes", NULL, NULL },
	{ "wbmwidthbytes", NULL, NULL },
#endif
	{ NULL, NULL, NULL}
};

static HashItem hashArray_x [] = {
	{ "xe", cmd_xe, "index entry" },
	{ NULL, NULL, NULL}
};

static HashItem *hash [26] = {
	hashArray_a,
	hashArray_b,
	hashArray_c,
	hashArray_d,
	hashArray_e,
	hashArray_f,
	NULL,
	hashArray_h,
	hashArray_i,
	hashArray_j,
	NULL,
	hashArray_l,
	hashArray_m,
	hashArray_n,
	hashArray_o,
	hashArray_p,
	NULL,
	hashArray_r,
	hashArray_s,
	hashArray_t,
	hashArray_u,
	NULL,
	hashArray_w,
	hashArray_x, 
	NULL, NULL
};


/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/



/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/


/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
print_with_special_exprs (OutputContext *oc, char *s) {
	int ch;
	int state;

enum { SMALL=0, BIG=1 };

	g_assert (s != NULL);

	if (oc->simulate_smallcaps) {
		if (*s >= 'a' && *s <= 'z') {
			state=SMALL;
                        oc->oh->attr (OH_ATTR_SMALLER, 1);
		}
		else
			state=BIG;
	}

	while ((ch=*s)) {
		char *post_trans = NULL;

		if (oc->simulate_allcaps || oc->simulate_smallcaps)
			ch = toupper (ch);

		if (ch >= 0x20 && ch < 0x80) {
			post_trans = op_translate_char (oc, oc->charset_type, ch);
			oc->oh->print ("%s", post_trans);
		}

		s++;

		if (oc->simulate_smallcaps) {
			ch = *s;
			if (ch >= 'a' && ch <= 'z') {
				if (state==BIG)
                                        oc->oh->attr (OH_ATTR_SMALLER, 1);
				state=SMALL;
			}
			else
			{
				if (state==SMALL)
                                        oc->oh->attr (OH_ATTR_SMALLER, 0);
				state=BIG;
			}
		}
	}
}



/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

static void 
begin_table (OutputContext *oc)
{
	oc->within_table=TRUE;
	oc->have_printed_row_begin = FALSE;
	oc->have_printed_cell_begin = FALSE;
	oc->have_printed_row_end = FALSE;
	oc->have_printed_cell_end = FALSE;
	attrstack_push (oc);
	starting_body (oc);
        oc->oh->table_begin ();
}



/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
end_table (OutputContext *oc)
{
	if (oc->within_table) {
		if (!oc->have_printed_cell_end) {
			attr_pop_dump (oc);
			oc->oh->table_cell_end ();
		}
		if (!oc->have_printed_row_end) {
			oc->oh->table_row_end ();
		}
		oc->oh->table_end ();
		oc->within_table=FALSE;
		oc->have_printed_row_begin = FALSE;
		oc->have_printed_cell_begin = FALSE;
		oc->have_printed_row_end = FALSE;
		oc->have_printed_cell_end = FALSE;
	}
}



/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
starting_text (OutputContext *oc)
{
	if (oc->within_table) {
		if (!oc->have_printed_row_begin) {
			oc->oh->table_row_begin ();
			oc->have_printed_row_begin=TRUE;
			oc->have_printed_row_end=FALSE;
			oc->have_printed_cell_begin=FALSE;
		}
		if (!oc->have_printed_cell_begin) {
			oc->oh->table_cell_begin ();
			attrstack_express_all (oc);
			oc->have_printed_cell_begin=TRUE;
			oc->have_printed_cell_end=FALSE;
		}
	}
}




/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

static void
starting_paragraph_align (OutputContext *oc, int align)
{
	if (oc->within_header && oc->align != ALIGN_LEFT)
		starting_body (oc);

	switch (oc->align)
	{
	case ALIGN_CENTER:
                oc->oh->attr (OH_ATTR_CENTER, 1);
		break;
	case ALIGN_LEFT:
		break;
	case ALIGN_RIGHT:
                oc->oh->attr (OH_ATTR_ALIGN_RIGHT, 1);
		break;
	case ALIGN_JUSTIFY:
                oc->oh->attr (OH_ATTR_ALIGN_RIGHT, 1);
		break;
	}
}



/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

static void
ending_paragraph_align (OutputContext *oc, int align)
{
	switch (align) {
	case ALIGN_CENTER:
                oc->oh->attr (OH_ATTR_CENTER, 0);
		break;
	case ALIGN_LEFT:
		// printf (op->align_left_end);
		break;
	case ALIGN_RIGHT:
                oc->oh->attr (OH_ATTR_ALIGN_RIGHT, 0);
		break;
	case ALIGN_JUSTIFY:
                oc->oh->attr (OH_ATTR_JUSTIFY, 0);
		break;
	}
}


/*========================================================================
 * Name:	
 * Purpose:	Recursive routine to produce the output in the target
 *		format given on a tree of words.
 * Args:	Word* (the tree).
 * Returns:	None.
 *=======================================================================*/

static void
word_print_core (OutputContext *oc, Word *w)
{
	char *s;
	FILE *f=NULL;
	int is_cell_group=FALSE;
	int paragraph_begined=FALSE;
	int paragraph_align=ALIGN_LEFT;

	g_assert (w != NULL);

	if (!coming_pars_that_are_tabular && within_table) {
		end_table (oc);
	}
	else if (coming_pars_that_are_tabular && !within_table) {
		begin_table (oc);
	}

	/* Mark our place in the stack */
	attrstack_push (oc);

	while (w) {
		s = word_string (w);

		if (s) {
			/*--Ignore whitespace in header--------------------*/
			if (*s==' ' && within_header) {
				/* no op */
			} 
			else
			/*--Handle word -----------------------------------*/
			if (s[0] != '\\') 
			{
				starting_body (oc);
				starting_text (oc);

				if (!oc->paragraph_begined) {
					starting_paragraph_align (oc, paragraph_align);
					oc->paragraph_begined=TRUE;
				}

				/*----------------------------------------*/
				if (oc->within_picture) {
					starting_body (oc);
					if (!f) {
						char *ext=NULL;
						switch (oc->picture_type) {
						case PICT_WB: ext="bmp"; break;
						case PICT_WM: ext="wmf"; break;
						case PICT_MAC: ext="pict"; break;
						case PICT_JPEG: ext="jpg"; break;
						case PICT_PNG: ext="png"; break;
						case PICT_DI: ext="dib"; break; /* Device independent bitmap=??? */
						case PICT_PM: ext="pmm"; break; /* OS/2 metafile=??? */
						}
						sprintf (picture_path, "pict%03d.%s", 
							picture_file_number++,ext);
						f=fopen(picture_path,"w");
					}

					if (s[0]!=' ') {
						char *s2;
						oc->oh->comment ("picture data found, ");
						if (picture_wmetafile_type_str) {
							oc->oh->comment ("WMF type is %s, ",
								picture_wmetafile_type_str);
						}
						oc->oh->comment ("picture dimensions are %d by %d, depth %d",
							picture_width, picture_height, picture_bits_per_pixel);
						if (picture_width && picture_height && picture_bits_per_pixel) {
							s2=s;
							while (*s2) {
								unsigned int tmp,value;
								tmp=tolower(*s2++);
								if (tmp>'9') tmp-=('a'-10); 
								else tmp-='0';
								value=16*tmp;
								tmp=tolower(*s2++);
								if (tmp>'9') tmp-=('a'-10); 
								else tmp-='0';
								value+=tmp;
								fprintf (f,"%c", value);
							}
						}
					}
				}
				/*----------------------------------------*/
				else {
					total_chars_this_line += strlen(s);

					if (op->word_begin)
						printf (op->word_begin);

					print_with_special_exprs (oc, s);

					if (op->word_end)
						printf (op->word_end);
				}

			/*---Handle RTF keywords---------------------------*/

			} else {
				int done=FALSE;

				s++;

/*----Paragraph alignment----------------------------------------------------*/
				if (!strcmp ("ql", s))
					paragraph_align = ALIGN_LEFT;
				else if (!strcmp ("qr", s))
					paragraph_align = ALIGN_RIGHT;
				else if (!strcmp ("qj", s))
					paragraph_align = ALIGN_JUSTIFY;
				else if (!strcmp ("qc", s))
					paragraph_align = ALIGN_CENTER;
				else if (!strcmp ("pard", s)) 
				{
					/* Clear out all font attributes.
					 */
					attr_pop_all (oc);
					if(coming_pars_that_are_tabular) {
						--coming_pars_that_are_tabular;
					}

					/* Clear out all paragraph attributes.
					 */
					ending_paragraph_align (oc, paragraph_align);
					paragraph_align = ALIGN_LEFT;
					paragraph_begined = FALSE;
				}
/*----Table keywords---------------------------------------------------------*/
				else
				if (!strcmp (s, "cell")) {
					is_cell_group=TRUE;
					if (!oc->have_printed_cell_begin) {
						/* Need this with empty cells */
						printf (op->table_cell_begin);
						attrstack_express_all ();
					}
					attr_pop_dump (oc);
					oc->oh->table_cell_end ();
					oc->have_printed_cell_begin = FALSE;
					oc->have_printed_cell_end=TRUE;
				}
				else if (!strcmp (s, "row")) {
					if (oc->within_table) {
						oc->oh->table_row_end ();
						oc->have_printed_row_begin = FALSE;
						oc->have_printed_row_end=TRUE;
					} else {
						if (oc->debug_mode) {
							oc->oh->comment ("end of table row");
						}
					}
				}

/*----Special chars---------------------------------------------------------*/
				else if (*s == '\'') {
					/* \'XX is a hex char code expression */
					int ch = h2toi (&s[1]);
					char *s2;

					s2 = op_translate_char (oc, oc->charset_type, ch);

					if (!s2 || !*s2) {
						oc->oh->comment ("char 0x%02x",ch);
					} else {
						if (op->word_begin)
							printf (op->word_begin);
						printf ("%s", s2);
						if (op->word_end)
							printf (op->word_end);
					}
				}
				else
/*----Search the RTF command hash-------------------------------------------*/
				{
					int ch;
					int index=0;
					int have_param=FALSE, param=0;
					HashItem *hip;
					char *p;
					int match;

					/* Look for a parameter */
					p=s;
					while(*p && (!isdigit(*p) && *p!='-')) p++;
					if (*p && (isdigit(*p) || *p=='-')) { 
						have_param=TRUE; 
						param=atoi (p);
					}

					/* Generate a hash index
					 */
					ch = tolower (*s);
					if (ch>='a' && ch<='z') 
						hip = hash [ch-'a'];
					else
						hip = hashArray_other;

					if (!hip) {
						if (oc->debug_mode) {
							oc->oh->comment ("unfamiliar rtf command: %s", s);
						}
					}
					else
					{
						while (!done) {
							match=FALSE;

							if (have_param) {
								int len=p-s;
								if (!hip[index].name[len] && !strncmp (s, hip[index].name, len))
									match=TRUE;
							}
							else
								match = !strcmp(s, hip[index].name);

							if (match) {
#if 0
								char *always;
#endif
								char *debug;
								int terminate_group;

								if (hip[index].func) {
									terminate_group = hip[index].func (w,paragraph_align, have_param, param);

									if (terminate_group)
										while(w) w=w->next;
								}

								debug=hip[index].debug_print;

#if 0
								always=hip[index].always_print;
								if (always)
									printf ("%s", always);
#endif
								if (debug && oc->debug_mode) {
									oc->oh->comment ("%s", debug);
								}

								done=TRUE;
							}
							else
							{
								index++;
								if (!hip[index].name)
									done=TRUE;
							}
						}
					}
					if (!match) {
						if (oc->debug_mode) {
							oc->oh->comment ("unfamiliar rtf command: %s", s);
						}
					}
				}
			}
/*-------------------------------------------------------------------------*/
		} else {
			Word *child;

			child = w->child;

			if (!paragraph_begined) {
				starting_paragraph_align (paragraph_align);
				paragraph_begined=TRUE;
			}

			if (child) 
			  word_print_core (child);
		}

		if (w) 
			w = w->next;
	}

	if (within_picture) {
		if(f) { 
			fclose(f);
			printf (op->imagelink_begin);
			printf ("%s", picture_path);
			printf (op->imagelink_end);
			within_picture=FALSE;
		}
	}

	/* Undo font attributes UNLESS we're doing table cells
	 * since they would appear between </td> and </tr>.
	 */
	if (!is_cell_group)
		attr_pop_all ();
	else
		attr_drop_all ();

	/* Undo paragraph alignment
	 */
	if (paragraph_begined)
		ending_paragraph_align (paragraph_align);

	attrstack_drop ();
}




/*========================================================================
 * Name:	
 * Purpose:	
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
word_print (OutputContext *oc, Word *w) 
{
	CHECK_PARAM_NOT_NULL (w);

	if (!inline_mode) {
		printf (op->document_begin);
		printf (op->header_begin);
	}

	within_header=TRUE;
	have_printed_body=FALSE;
	within_table=FALSE;
	simulate_allcaps=FALSE;
	word_print_core (w);
	end_table ();

	if (!inline_mode) {
		printf (op->body_end);
		printf (op->document_end);
	}
}


