/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4; coding: utf-8 -*-
 *
 *   moohtml.h
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

#ifndef __MOO_HTML_H__
#define __MOO_HTML_H__

#include <gtk/gtktextview.h>

G_BEGIN_DECLS


#define MOO_TYPE_HTML              (moo_html_get_type ())
#define MOO_HTML(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_HTML, MooHtml))
#define MOO_HTML_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_HTML, MooHtmlClass))
#define MOO_IS_HTML(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_HTML))
#define MOO_IS_HTML_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_HTML))
#define MOO_HTML_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_HTML, MooHtmlClass))

#define MOO_TYPE_HTML_TAG              (moo_html_tag_get_type ())
#define MOO_HTML_TAG(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOO_TYPE_HTML_TAG, MooHtmlTag))
#define MOO_HTML_TAG_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOO_TYPE_HTML_TAG, MooHtmlTagClass))
#define MOO_IS_HTML_TAG(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOO_TYPE_HTML_TAG))
#define MOO_IS_HTML_TAG_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOO_TYPE_HTML_TAG))
#define MOO_HTML_TAG_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOO_TYPE_HTML_TAG, MooHtmlTagClass))

typedef struct _MooHtml             MooHtml;
typedef struct _MooHtmlPrivate      MooHtmlPrivate;
typedef struct _MooHtmlClass        MooHtmlClass;
typedef struct _MooHtmlTag          MooHtmlTag;
typedef struct _MooHtmlTagClass     MooHtmlTagClass;
typedef struct _MooHtmlAttr         MooHtmlAttr;

struct _MooHtml
{
    GtkTextView parent;
    MooHtmlPrivate *priv;
};

struct _MooHtmlClass
{
    GtkTextViewClass parent_class;

    gboolean (*load_url)    (MooHtml    *html,
                             const char *url);
    void     (*hover_link)  (MooHtml    *html,
                             const char *link);
};

struct _MooHtmlTag
{
    GtkTextTag base;

    MooHtmlTag *parent;
    GHashTable *child_tags; /* char* -> MooHtmlTag* */

    MooHtmlAttr *attr;
    char *href;
};

struct _MooHtmlTagClass
{
    GtkTextTagClass base_class;
};


GType           moo_html_get_type       (void) G_GNUC_CONST;
GType           moo_html_tag_get_type   (void) G_GNUC_CONST;

GtkWidget      *moo_html_new            (void);

gboolean        moo_html_load_memory    (MooHtml            *html,
                                         const char         *buffer,
                                         int                 size,
                                         const char         *url,
                                         const char         *encoding);
gboolean        moo_html_load_file      (MooHtml            *html,
                                         const char         *file,
                                         const char         *encoding);

void            moo_html_set_font       (MooHtml            *html,
                                         const char         *font);


G_END_DECLS

#endif /* __MOO_HTML_H__ */
