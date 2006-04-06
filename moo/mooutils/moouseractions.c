/*
 *   moouseractions.c
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

#include "mooutils/moouseractions.h"
#include "mooscript/mooscript-parser.h"
#include "mooutils/moocompat.h"
#include <string.h>

typedef MooUserActionCtxFunc CtxFunc;

typedef struct {
    const char *filename;
    GSList *actions;

    char *action;
    char *label;
    char *accel;
    GString *code;
} Parser;

typedef struct {
    char *name;
    char *label;
    char *accel;
    MSNode *script;
    CtxFunc ctx_func;
} Action;

typedef struct {
    MooAction base;
    MooWindow *window;
    MSNode *script;
    CtxFunc ctx_func;
} MooUserAction;

typedef struct {
    MooActionClass base_class;
} MooUserActionClass;


GType       _moo_user_action_get_type   (void) G_GNUC_CONST;
static void parser_create_actions       (Parser *parser);

G_DEFINE_TYPE (MooUserAction, _moo_user_action, MOO_TYPE_ACTION)


static void
action_free (Action *action)
{
    if (action)
    {
        g_free (action->name);
        g_free (action->label);
        g_free (action->accel);
        ms_node_unref (action->script);
        g_free (action);
    }
}


static char *
parse_accel (const char *string)
{
    if (!string || !string[0])
        return NULL;

    return g_strdup (string);
}


static MSNode *
parse_code (const char *code)
{
    g_return_val_if_fail (code != NULL, NULL);
    return ms_script_parse (code);
}

static Action *
action_new (const char *name,
            const char *label,
            const char *accel,
            const char *code,
            CtxFunc     ctx_func)
{
    Action *action;
    MSNode *script;

    g_return_val_if_fail (name != NULL, NULL);
    g_return_val_if_fail (code != NULL, NULL);

    script = parse_code (code);

    if (!script)
    {
        g_warning ("could not parse script\n%s\n", code);
        return NULL;
    }

    action = g_new0 (Action, 1);
    action->name = g_strdup (name);
    action->label = label ? g_strdup (label) : g_strdup (name);
    action->accel = parse_accel (accel);
    action->script = script;
    action->ctx_func = ctx_func;

    return action;
}


static void
parser_init (Parser     *parser,
             const char *filename)
{
    memset (parser, 0, sizeof (Parser));
    parser->filename = filename;
    parser->code = g_string_new (NULL);
}


static gboolean
is_blank_or_comment (const char *line)
{
    char c;

    while ((c = *line))
    {
        if (!c || c == '\r' || c == '#')
            return TRUE;

        if (c != ' ' && c != '\t')
            return FALSE;

        line++;
    }

    return TRUE;
}


static void
parser_cleanup (Parser *parser)
{
//     g_slist_foreach (parser->actions, (GFunc) action_free, NULL);
    g_slist_free (parser->actions);
    g_string_free (parser->code, TRUE);
}


static gboolean
starts_action (const char *line)
{
    return !g_ascii_strncasecmp (line, "action:", strlen ("action:"));
}


static gboolean
is_header_line (const char *line)
{
    return !g_ascii_strncasecmp (line, "action:", strlen ("action:")) ||
            !g_ascii_strncasecmp (line, "label:", strlen ("label:")) ||
            !g_ascii_strncasecmp (line, "accel:", strlen ("accel:"));
}


static gboolean
parse_header_line (Parser *parser,
                   char   *line,
                   guint   line_no)
{
    char *colon, *value, *comment;

    colon = strchr (line, ':');
    g_assert (colon != NULL);
    *colon = 0;

    value = colon + 1;
    comment = strchr (value, '#');
    if (comment)
        *comment = 0;
    g_strstrip (value);

    if (!value[0])
    {
        g_warning ("file %s, line %d: empty key value",
                   parser->filename, line_no);
        return FALSE;
    }

    if (!g_ascii_strcasecmp (line, "action"))
    {
        if (parser->action)
        {
            g_warning ("file %s, line %d: duplicated action field",
                       parser->filename, line_no);
            return FALSE;
        }

        parser->action = value;
//         g_print ("action %s\n", value);
    }
    else if (!g_ascii_strcasecmp (line, "label"))
    {
        if (!parser->action)
        {
            g_warning ("file %s, line %d: missing action field",
                       parser->filename, line_no);
            return FALSE;
        }
        else if (parser->label)
        {
            g_warning ("file %s, line %d: duplicated label field",
                       parser->filename, line_no);
            return FALSE;
        }

        parser->label = value;
//         g_print ("in action %s: label %s\n", parser->action, value);
    }
    else if (!g_ascii_strcasecmp (line, "accel"))
    {
        if (!parser->action)
        {
            g_warning ("file %s, line %d: missing action field",
                       parser->filename, line_no);
            return FALSE;
        }
        else if (parser->accel)
        {
            g_warning ("file %s, line %d: duplicated accel field",
                       parser->filename, line_no);
            return FALSE;
        }

        parser->accel = value;
//         g_print ("in action %s: accel %s\n", parser->action, value);
    }
    else
    {
        g_return_val_if_reached (FALSE);
    }

    return TRUE;
}


static void
parser_add_code (Parser *parser,
                 char   *line)
{
    if (parser->code->len)
        g_string_append_c (parser->code, '\n');
    g_string_append (parser->code, line);
}


static void
parser_end_code (Parser *parser,
                 CtxFunc func)
{
    Action *action;

    if (!parser->code->len)
    {
        g_warning ("file %s: missing code for action '%s'",
                   parser->filename, parser->action);
        goto out;
    }

//     g_print ("in action %s\n-----------------\n%s\n----------------\n",
//              parser->action, parser->code->str);

    action = action_new (parser->action, parser->label,
                         parser->accel, parser->code->str,
                         func);

    if (!action)
        goto out;

    parser->actions = g_slist_prepend (parser->actions, action);

out:
    parser->action = NULL;
    parser->label = NULL;
    parser->accel = NULL;
    g_string_truncate (parser->code, 0);
}


inline static const char *
find_line_term (const char *string,
                guint       len,
                guint      *term_len)
{
    while (len)
    {
        switch (string[0])
        {
            case '\r':
                if (len > 1 && string[1] == '\n')
                    *term_len = 2;
                else
                    *term_len = 1;
                return string;

            case '\n':
                *term_len = 1;
                return string;

            default:
                len--;
                string++;
        }
    }

    return NULL;
}

static char **
splitlines (const char *string,
            guint       len,
            guint      *n_lines_p)
{
    GSList *list = NULL;
    char **lines;
    guint n_lines = 0, i;

    if (!len)
        return NULL;

    while (len)
    {
        guint term_len = 0;
        const char *term = find_line_term (string, len, &term_len);

        n_lines++;

        if (term)
        {
            list = g_slist_prepend (list, g_strndup (string, term - string));
            len -= (term - string + term_len);
            string = term + term_len;
        }
        else
        {
            list = g_slist_prepend (list, g_strndup (string, len));
            break;
        }
    }

    list = g_slist_reverse (list);
    lines = g_new (char*, n_lines + 1);
    lines[n_lines] = NULL;
    i = 0;

    while (list)
    {
        lines[i++] = list->data;
        list = g_slist_delete_link (list, list);
    }

    *n_lines_p = n_lines;
    return lines;
}


void
moo_parse_user_actions (const char          *filename,
                        MooUserActionCtxFunc ctx_func)
{
    GMappedFile *file;
    GError *error = NULL;
    Parser parser;
    char **lines;
    guint n_lines = 0, i;

    g_return_if_fail (filename != NULL);
    g_return_if_fail (ctx_func != NULL);

    file = g_mapped_file_new (filename, FALSE, &error);

    if (!file)
    {
        g_warning ("%s: could not open file %s", G_STRLOC, filename);
        g_warning ("%s: %s", G_STRLOC, error->message);
        g_error_free (error);
        return;
    }

    if (!g_mapped_file_get_length (file))
    {
        g_mapped_file_free (file);
        return;
    }

    lines = splitlines (g_mapped_file_get_contents (file),
                        g_mapped_file_get_length (file),
                        &n_lines);
    g_mapped_file_free (file);

    parser_init (&parser, filename);
    i = 0;

    while (i < n_lines)
    {
        while (i < n_lines && is_blank_or_comment (lines[i]))
            i++;

        if (i == n_lines)
            break;

        if (!starts_action (lines[i]))
        {
            g_warning ("file %s, line %d: invalid text '%s'",
                       filename, i, lines[i]);
            goto out;
        }

        while (TRUE)
        {
            if (is_header_line (lines[i]))
            {
                if (!parse_header_line (&parser, lines[i], i))
                    goto out;

            }
            else if (!is_blank_or_comment (lines[i]))
            {
                break;
            }

            if (++i == n_lines)
            {
                g_warning ("file %s: missing code in action '%s'",
                           filename, parser.action);
                goto out;
            }
        }

        while (i < n_lines)
        {
            if ((!lines[i][0] || lines[i][0] == '\r') &&
                  (i == n_lines - 1 || starts_action (lines[i+1])))
            {
                parser_end_code (&parser, ctx_func);
                break;
            }

            parser_add_code (&parser, lines[i++]);
        }

        if (i == n_lines)
            parser_end_code (&parser, ctx_func);
    }

    parser.actions = g_slist_reverse (parser.actions);
    parser_create_actions (&parser);

out:
    parser_cleanup (&parser);
    g_strfreev (lines);
}


static void
_moo_user_action_init (G_GNUC_UNUSED MooUserAction *action)
{
}


static void
moo_user_action_finalize (GObject *object)
{
    MooUserAction *action = (MooUserAction*) object;
    ms_node_unref (action->script);
    G_OBJECT_CLASS(_moo_user_action_parent_class)->finalize (object);
}


static void
moo_user_action_activate (MooAction *_action)
{
    MSContext *ctx;
    MSValue *value;
    MooUserAction *action = (MooUserAction*) _action;

    ctx = action->ctx_func (action->window);
    value = ms_top_node_eval (action->script, ctx);

    if (!value)
        g_warning ("%s: %s", G_STRLOC, ms_context_get_error_msg (ctx));
    else
        ms_value_unref (value);

    g_object_unref (ctx);
}


static void
_moo_user_action_class_init (MooUserActionClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = moo_user_action_finalize;
    MOO_ACTION_CLASS(klass)->activate = moo_user_action_activate;
}


static MooAction *
create_action (MooWindow *window,
               Action    *data)
{
    MooUserAction *action;

    g_return_val_if_fail (data != NULL, NULL);

    action = g_object_new (_moo_user_action_get_type (),
                           "name", data->name,
                           "accel", data->accel,
                           "label", data->label,
                           NULL);

    action->window = window;
    action->script = ms_node_ref (data->script);
    action->ctx_func = data->ctx_func;

    return MOO_ACTION (action);
}


static void
parser_create_actions (Parser *parser)
{
    GSList *l;
    MooWindowClass *klass;

    klass = g_type_class_ref (MOO_TYPE_WINDOW);

    for (l = parser->actions; l != NULL; l = l->next)
    {
        Action *action = l->data;
        moo_window_class_new_action_custom (klass, action->name,
                                            (MooWindowActionFunc) create_action,
                                            action, (GDestroyNotify) action_free);
    }

    g_type_class_unref (klass);
}
