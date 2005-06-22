
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
 * Module name:    attr
 * Author name:    Zach Smith
 * Create date:    01 Aug 01
 * Purpose:        Character attribute stack.
 *----------------------------------------------------------------------
 * Changes:
 * 01 Aug 01, tuorfa@yahoo.com: moved code over from convert.c
 * 06 Aug 01, tuorfa@yahoo.com: added several font attributes.
 * 18 Sep 01, tuorfa@yahoo.com: added AttrStack (stack of stacks) paradigm
 * 22 Sep 01, tuorfa@yahoo.com: added comment blocks
 *--------------------------------------------------------------------*/


#include <glib.h>
#include <stdlib.h>

#include "attr.h"
#include "defs.h"
#include "error.h"


void starting_body (void);
void starting_text (void);


#define MAX_ATTRS (1000)



/* For each RTF text block (the text within braces) we must keep
 * an AttrStack which is a stack of attributes and their optional
 * parameter. Since RTF text blocks are nested, these make up a
 * stack of stacks. And, since RTF text blocks inherit attributes
 * from parent blocks, all new AttrStacks do the same from
 * their parent AttrStack.
 */
typedef struct _stack {
	unsigned char attr_stack [MAX_ATTRS];
	char *attr_stack_params [MAX_ATTRS]; 
	int tos;
	struct _stack *next;
}
AttrStack;

static AttrStack *stack_of_stacks = NULL;
static AttrStack *stack_of_stacks_top = NULL;




/*========================================================================
 * Name:	attr_express_begin
 * Purpose:	Print the HTML for beginning an attribute.
 * Args:	Attribute number, optional string parameter.
 * Returns:	None.
 *=======================================================================*/

static void 
attr_express_begin (OutputContext *oc, int attr, char* param) {
	switch(attr) 
	{
	case ATTR_BOLD:
		oc->oh->attr (OH_ATTR_BOLD, 1);
		break;
	case ATTR_ITALIC: 
		oc->oh->attr (OH_ATTR_ITALIC, 1);
		break;

	/* Various underlines, they all resolve to HTML's <u> */
	case ATTR_THICK_UL:
	case ATTR_WAVE_UL:
	case ATTR_DASH_UL:
	case ATTR_DOT_UL: 
	case ATTR_DOT_DASH_UL:
	case ATTR_2DOT_DASH_UL:
	case ATTR_WORD_UL: 
	case ATTR_UNDERLINE: 
		oc->oh->attr (OH_ATTR_UNDERLINE, 1);
		break;

	case ATTR_DOUBLE_UL: 
		oc->oh->attr (OH_ATTR_DBL_UNDERLINE, 1);
		break;

	case ATTR_FONTSIZE: 
		oc->oh->std_fontsize (atoi (param), 1);
		break;

	case ATTR_FONTFACE: 
		oc->oh->font (param, 1);
		break;

	case ATTR_FOREGROUND: 
		oc->oh->foreground (param, 1);
		break;

	case ATTR_BACKGROUND: 
		if (!oc->simple_mode)
			oc->oh->background (param, 1);  /* TODO: why foreground? */
		break;

	case ATTR_SUPER: 
		oc->oh->attr (OH_ATTR_SUPERSCRIPT, 1);
		break;
	case ATTR_SUB: 
		oc->oh->attr (OH_ATTR_SUBSCRIPT, 1);
		break;

	case ATTR_STRIKE: 
		oc->oh->attr (OH_ATTR_STRIKETHROUGH, 1);
		break;

	case ATTR_DBL_STRIKE: 
		oc->oh->attr (OH_ATTR_DBL_STRIKETHROUGH, 1);
		break;

	case ATTR_EXPAND: 
		oc->oh->expand (param, 1);
		break;

	case ATTR_OUTLINE: 
		oc->oh->attr (OH_ATTR_OUTLINE, 1);
		break;
	case ATTR_SHADOW: 
		oc->oh->attr (OH_ATTR_SHADOW, 1);
		break;
	case ATTR_EMBOSS: 
		oc->oh->attr (OH_ATTR_EMBOSS, 1);
		break;
	case ATTR_ENGRAVE: 
		oc->oh->attr (OH_ATTR_ENGRAVE, 1);
		break;

	case ATTR_CAPS:
		if (oc->oh->simulate_all_caps)
			oc->simulate_allcaps = TRUE;
		break;

	case ATTR_SMALLCAPS: 
		if (oc->oh->simulate_small_caps)
			oc->simulate_smallcaps = TRUE;
		else {
			oc->oh->attr (OH_ATTR_SMALL_CAPS, 1);
		}
		break;
	}
}


/*========================================================================
 * Name:	attr_express_end
 * Purpose:	Print HTML to complete an attribute.
 * Args:	Attribute number.
 * Returns:	None.
 *=======================================================================*/

static void 
attr_express_end (OutputContext *oc, int attr, G_GNUC_UNUSED char *param)
{
	switch(attr) 
	{
	case ATTR_BOLD: 
		oc->oh->attr (OH_ATTR_BOLD, 0);
		break;
	case ATTR_ITALIC: 
		oc->oh->attr (OH_ATTR_ITALIC, 0);
		break;

	/* Various underlines, they all resolve to HTML's </u> */
	case ATTR_THICK_UL:
	case ATTR_WAVE_UL:
	case ATTR_DASH_UL:
	case ATTR_DOT_UL: 
	case ATTR_DOT_DASH_UL:
	case ATTR_2DOT_DASH_UL: 
	case ATTR_WORD_UL: 
	case ATTR_UNDERLINE: 
		oc->oh->attr (OH_ATTR_UNDERLINE, 0);
		break;

	case ATTR_DOUBLE_UL: 
		oc->oh->attr (OH_ATTR_DBL_UNDERLINE, 0);
		break;

	case ATTR_FONTSIZE: 
		oc->oh->std_fontsize (atoi (param), 0);
		break;

	case ATTR_FONTFACE:
		oc->oh->font (NULL, 0);
		break;

	case ATTR_FOREGROUND: 
		oc->oh->foreground (NULL, 0);
		break;
	case ATTR_BACKGROUND: 
		if (!oc->simple_mode)
			oc->oh->background (NULL, 0);
		break;

	case ATTR_SUPER: 
		oc->oh->attr (OH_ATTR_SUPERSCRIPT, 0);
		break;
	case ATTR_SUB: 
		oc->oh->attr (OH_ATTR_SUBSCRIPT, 0);
		break;

	case ATTR_STRIKE: 
		oc->oh->attr (OH_ATTR_STRIKETHROUGH, 0);
		break;

	case ATTR_DBL_STRIKE: 
		oc->oh->attr (OH_ATTR_DBL_STRIKETHROUGH, 0);
		break;

	case ATTR_OUTLINE: 
		oc->oh->attr (OH_ATTR_OUTLINE, 0);
		break;
	case ATTR_SHADOW: 
		oc->oh->attr (OH_ATTR_SHADOW, 0);
		break;
	case ATTR_EMBOSS: 
		oc->oh->attr (OH_ATTR_EMBOSS, 0);
		break;
	case ATTR_ENGRAVE: 
		oc->oh->attr (OH_ATTR_ENGRAVE, 0);
		break;

	case ATTR_EXPAND: 
		oc->oh->expand (NULL, 0);
		break;

	case ATTR_CAPS:
		if (oc->oh->simulate_all_caps)
			oc->simulate_allcaps = FALSE;
		break;

	case ATTR_SMALLCAPS: 
		if (oc->oh->simulate_small_caps)
			oc->simulate_smallcaps = FALSE;
		else {
			oc->oh->attr (OH_ATTR_SMALL_CAPS, 0);
		}
		break;
	}
}



/*========================================================================
 * Name:	attr_push
 * Purpose:	Pushes an attribute onto the current attribute stack.
 * Args:	Attribute number, optional string parameter.
 * Returns:	None.
 *=======================================================================*/

void 
attr_push(OutputContext *oc, int attr, char* param)
{
	AttrStack *stack = stack_of_stacks_top;
	if (!stack) {
		g_warning ("no stack to push attribute onto");
		return;
	}

	if (stack->tos>=MAX_ATTRS) {
		g_critical ("Too many attributes!\n"); 
		return;
	}

	/* Make sure it's understood we're in the <body> section. */
	/* KLUDGE */
	starting_body ();
	starting_text ();

	++stack->tos;
	stack->attr_stack [stack->tos]=attr;
	if (param) 
		stack->attr_stack_params [stack->tos] = g_strdup (param);
	else
		stack->attr_stack_params [stack->tos]=NULL;

	attr_express_begin (oc, attr, param);
}


/*========================================================================
 * Name:	attrstack_copy_all
 * Purpose:	Routine to copy all attributes from one stack to another.
 * Args:	Two stacks.
 * Returns:	None.
 *=======================================================================*/

static void 
attrstack_copy_all (AttrStack *src, AttrStack *dest)
{
	int i;
	int total;

	g_assert (src != NULL);
	g_assert (dest != NULL);

	total = src->tos + 1;

	for (i=0; i<total; i++)
	{
		int attr=src->attr_stack [i];
		char *param=src->attr_stack_params [i];

		dest->attr_stack[i] = attr;
		if (param)
			dest->attr_stack_params[i] = g_strdup (param);
		else
			dest->attr_stack_params[i] = NULL;
	}

	dest->tos = src->tos;
}

/*========================================================================
 * Name:	attrstack_unexpress_all
 * Purpose:	Routine to un-express all attributes heretofore applied,
 * 		without removing any from the stack.
 * Args:	Stack whost contents should be unexpressed.
 * Returns:	None.
 * Notes:	This is needed by attrstack_push, but also for \cell, which
 * 		often occurs within a brace group, yet HTML uses <td></td> 
 *		which clear attribute info within that block.
 *=======================================================================*/

static void 
attrstack_unexpress_all (OutputContext *oc, AttrStack *stack)
{
	int i;

	g_assert (stack != NULL);

	i=stack->tos;
	while (i>=0)
	{
		int attr=stack->attr_stack [i];
		char *param=stack->attr_stack_params [i];

		attr_express_end (oc, attr, param);
		i--;
	}
}


/*========================================================================
 * Name:	attrstack_push
 * Purpose:	Creates a new attribute stack, pushes it onto the stack
 *		of stacks, performs inheritance from previous stack.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/
void
attrstack_push (OutputContext *oc)
{
	AttrStack *new_stack;
	AttrStack *prev_stack;

	new_stack = g_new0 (AttrStack, 1);

	prev_stack = stack_of_stacks_top;

	if (!stack_of_stacks) {
		stack_of_stacks = new_stack;
	} else {
		stack_of_stacks_top->next = new_stack;
	}
	stack_of_stacks_top = new_stack;
	new_stack->tos = -1;

	if (prev_stack) {
		attrstack_unexpress_all (oc, prev_stack);
		attrstack_copy_all (prev_stack, new_stack);
		attrstack_express_all (oc);
	}
}



/*========================================================================
 * Name:	attr_pop 
 * Purpose:	Removes and undoes the effect of the top attribute of
 *		the current AttrStack.
 * Args:	The top attribute's number, for verification.
 * Returns:	Success/fail flag.
 *=======================================================================*/

int 
attr_pop (OutputContext *oc, int attr)
{
	AttrStack *stack = stack_of_stacks_top;

	if (!stack) {
		g_warning ("no stack to pop attribute from");
		return FALSE;
	}

	if(stack->tos>=0 && stack->attr_stack[stack->tos]==attr)
	{
		char *param = stack->attr_stack_params [stack->tos];

		attr_express_end (oc, attr, param);

		if (param) g_free (param);

		stack->tos--;

		return TRUE;
	}
	else
		return FALSE;
}



/*========================================================================
 * Name:	attr_read
 * Purpose:	Reads but leaves in place the top attribute of the top
 * 		attribute stack.
 * Args:	None.
 * Returns:	Attribute number.
 *=======================================================================*/

int
attr_read (void)
{
	AttrStack *stack = stack_of_stacks_top;
	if (!stack) {
		g_warning ("no stack to read attribute from");
		return FALSE;
	}

	if(stack->tos>=0)
	{
		int attr = stack->attr_stack [stack->tos];
		return attr;
	}
	else
		return ATTR_NONE;
}


/*========================================================================
 * Name:	attr_drop_all 
 * Purpose:	Undoes all attributes that an AttrStack contains.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
attr_drop_all (void) 
{
	AttrStack *stack = stack_of_stacks_top;
	if (!stack) {
		g_warning ("no stack to drop all attributes from");
		return;
	}

	while (stack->tos>=0) 
	{
		char *param=stack->attr_stack_params [stack->tos];
		if (param) g_free (param);
		stack->tos--;
	}
}


/*========================================================================
 * Name:	attrstack_drop
 * Purpose:	Removes the top AttrStack from the stack of stacks, undoing
 *		all attributes that it had in it.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
attrstack_drop (OutputContext *oc)
{
	AttrStack *stack = stack_of_stacks_top;
	AttrStack *prev_stack;
	if (!stack) {
		g_warning ("no attr-stack to drop");
		return;
	}

	attr_pop_all (oc);

	prev_stack = stack_of_stacks;
	while(prev_stack && prev_stack->next && prev_stack->next != stack)
		prev_stack = prev_stack->next;

	if (prev_stack) {
		stack_of_stacks_top = prev_stack;
		prev_stack->next = NULL;
	} else {
		stack_of_stacks_top = NULL;
		stack_of_stacks = NULL;
	}
	g_free ((void*) stack);

	attrstack_express_all (oc);
}

/*========================================================================
 * Name:	attr_pop_all
 * Purpose:	Routine to undo all attributes heretofore applied, 
 *		also reversing the order in which they were applied.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void 
attr_pop_all (OutputContext *oc)
{
	AttrStack *stack = stack_of_stacks_top;
	if (!stack) {
		g_warning ("no stack to pop from");
		return;
	}

	while (stack->tos>=0) {
		int attr=stack->attr_stack [stack->tos];
		char *param=stack->attr_stack_params [stack->tos];
		attr_express_end (oc, attr, param);
		if (param) g_free (param);
		stack->tos--;
	}
}


/*========================================================================
 * Name:	attrstack_express_all
 * Purpose:	Routine to re-express all attributes heretofore applied.
 * Args:	None.
 * Returns:	None.
 * Notes:	This is needed by attrstack_push, but also for \cell, which
 * 		often occurs within a brace group, yet HTML uses <td></td> 
 *		which clear attribute info within that block.
 *=======================================================================*/

void 
attrstack_express_all (OutputContext *oc)
{
	AttrStack *stack = stack_of_stacks_top;
	int i;

	if (!stack) {
		g_warning ("no stack to pop from");
		return;
	}

	i=0;
	while (i<=stack->tos) 
	{
		int attr=stack->attr_stack [i];
		char *param=stack->attr_stack_params [i];
		attr_express_begin (oc, attr, param);
		i++;
	}
}


/*========================================================================
 * Name:	attr_pop_dump
 * Purpose:	Routine to un-express all attributes heretofore applied.
 * Args:	None.
 * Returns:	None.
 * Notes:	This is needed for \cell, which often occurs within a 
 *		brace group, yet HTML uses <td></td> which clear attribute 
 *		info within that block.
 *=======================================================================*/

void 
attr_pop_dump (OutputContext *oc)
{
	AttrStack *stack = stack_of_stacks_top;
	int i;

	if (!stack) return;

	i=stack->tos;
	while (i>=0) 
	{
		int attr=stack->attr_stack [i];
		attr_pop (oc, attr);
		i--;
	}
}

