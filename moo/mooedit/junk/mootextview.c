// static gboolean has_boxes                   (MooTextView        *view);
// static void     update_box_tag              (MooTextView        *view);
// static gboolean has_box_at_iter             (MooTextView        *view,
//                                              GtkTextIter        *iter);

static void
paste_moo_text_view_content (GtkTextView *target,
                             MooTextView *source)
{
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    MooTextViewClipboard *contents;

    g_return_if_fail (MOO_IS_TEXT_VIEW (source));

    buffer = gtk_text_view_get_buffer (target);

    contents = source->priv->clipboard;
    g_return_if_fail (contents != NULL);

    if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
        gtk_text_buffer_delete (buffer, &start, &end);

    if (strstr (contents->text, MOO_TEXT_UNKNOWN_CHAR_S))
    {
        char **pieces, **p;

        pieces = g_strsplit (contents->text, MOO_TEXT_UNKNOWN_CHAR_S, 0);

        for (p = pieces; *p; ++p)
        {
            if (p != pieces)
                moo_text_view_insert_placeholder (MOO_TEXT_VIEW (target), &end, NULL);

            gtk_text_buffer_insert (buffer, &end, *p, -1);
        }

        g_strfreev (pieces);
    }
    else
    {
        gtk_text_buffer_insert (buffer, &end, contents->text, -1);
    }
}


// static void
// draw_box (GtkTextView       *text_view,
//           GdkEventExpose    *event,
//           const GtkTextIter *iter)
// {
//     GtkTextBuffer *buffer;
//     GtkTextIter sel_start, sel_end;
//     gboolean selected = FALSE;
//     GdkGC *gc;
//     GdkRectangle rect;
//
//     buffer = gtk_text_view_get_buffer (text_view);
//
//     if (gtk_text_buffer_get_selection_bounds (buffer, &sel_start, &sel_end) &&
//         gtk_text_iter_compare (&sel_start, iter) <= 0 &&
//         gtk_text_iter_compare (iter, &sel_end) < 0)
//             selected = TRUE;
//
//     gtk_text_view_get_iter_location (text_view, iter, &rect);
//     gtk_text_view_buffer_to_window_coords (text_view, GTK_TEXT_WINDOW_TEXT,
//                                            rect.x, rect.y, &rect.x, &rect.y);
//
//     rect.x += 2;
//     rect.y += 2;
//     rect.width -= 4;
//     rect.height -= 4;
//
//     if (selected)
//         gc = GTK_WIDGET(text_view)->style->base_gc[GTK_STATE_NORMAL];
//     else
//         gc = GTK_WIDGET(text_view)->style->text_gc[GTK_STATE_NORMAL];
//
//     gdk_draw_rectangle (event->window, gc, FALSE,
//                         rect.x, rect.y, rect.width, rect.height);
//
//     rect.x += 1;
//     rect.y += 1;
//     rect.width -= 2;
//     rect.height -= 2;
//
//     gdk_draw_rectangle (event->window, gc, FALSE,
//                         rect.x, rect.y, rect.width, rect.height);
// }
//
//
// static void
// moo_text_view_draw_boxes (GtkTextView       *text_view,
//                           GdkEventExpose    *event,
//                           const GtkTextIter *start,
//                           const GtkTextIter *end)
// {
//     GtkTextIter iter = *start;
//
//     while (gtk_text_iter_compare (&iter, end) < 0)
//     {
//         if (has_box_at_iter (MOO_TEXT_VIEW (text_view), &iter))
//             draw_box (text_view, event, &iter);
//
//         if (!gtk_text_iter_forward_char (&iter))
//             break;
//     }
// }


// /*****************************************************************************/
// /* Placeholders
//  */
//
// static void
// update_box_tag (MooTextView *view)
// {
//     GtkTextTag *tag = moo_text_view_lookup_tag (view, "moo-text-box");
//
//     if (tag && GTK_WIDGET_REALIZED (view))
//     {
//         PangoContext *ctx;
//         PangoLayout *layout;
//         PangoLayoutLine *line;
//         PangoRectangle rect;
//         int rise;
//
//         ctx = gtk_widget_get_pango_context (GTK_WIDGET (view));
//         g_return_if_fail (ctx != NULL);
//
//         layout = pango_layout_new (ctx);
//         pango_layout_set_text (layout, "AA", -1);
//         line = pango_layout_get_line (layout, 0);
//
//         pango_layout_line_get_extents (line, NULL, &rect);
//
//         rise = rect.y + rect.height;
//
//         if (tag)
//             g_object_set (tag, "rise", -rise, NULL);
//
//         g_object_unref (layout);
//     }
// }
//
//
// static GtkTextTag *
// create_box_tag (MooTextView *view)
// {
//     GtkTextTag *tag;
//
//     tag = moo_text_view_lookup_tag (view, "moo-text-box");
//
//     if (!tag)
//     {
//         GtkTextBuffer *buffer = get_buffer (view);
//         tag = gtk_text_buffer_create_tag (buffer, "moo-text-box", NULL);
//         update_box_tag (view);
//     }
//
//     return tag;
// }
//
//
// static GtkTextTag *
// get_placeholder_tag (MooTextView *view)
// {
//     return moo_text_view_lookup_tag (view, MOO_PLACEHOLDER_TAG);
// }
//
//
// static GtkTextTag *
// create_placeholder_tag (MooTextView *view)
// {
//     GtkTextTag *tag;
//
//     tag = moo_text_view_lookup_tag (view, MOO_PLACEHOLDER_TAG);
//
//     if (!tag)
//     {
//         GtkTextBuffer *buffer = get_buffer (view);
//         tag = gtk_text_buffer_create_tag (buffer, MOO_PLACEHOLDER_TAG, NULL);
//         g_object_set (tag, "background", "yellow", NULL);
//     }
//
//     return tag;
// }
//
//
// static void
// moo_text_view_insert_box (MooTextView *view,
//                           GtkTextIter *iter)
// {
//     GtkTextBuffer *buffer;
//     GtkTextChildAnchor *anchor;
//     GtkWidget *box;
//     GtkTextTag *tag;
//     GtkTextIter start;
//
//     g_return_if_fail (MOO_IS_TEXT_VIEW (view));
//     g_return_if_fail (iter != NULL);
//
//     anchor = GTK_TEXT_CHILD_ANCHOR (g_object_new (MOO_TYPE_TEXT_ANCHOR, (const char*) NULL));
//     box = GTK_WIDGET (g_object_new (MOO_TYPE_TEXT_BOX, (const char*) NULL));
//     MOO_TEXT_ANCHOR (anchor)->widget = box;
//
//     buffer = get_buffer (view);
//     gtk_text_buffer_insert_child_anchor (buffer, iter, anchor);
//
//     tag = create_box_tag (view);
//     start = *iter;
//     gtk_text_iter_backward_char (&start);
//     gtk_text_buffer_apply_tag (buffer, tag, &start, iter);
//
//     gtk_widget_show (box);
//     gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (view), box, anchor);
//     view->priv->boxes = g_slist_prepend (view->priv->boxes, box);
//
//     g_object_unref (anchor);
// }
//
//
// void
// moo_text_view_insert_placeholder (MooTextView  *view,
//                                   GtkTextIter  *iter,
//                                   const char   *text)
// {
//     MooTextBuffer *buffer;
//     GtkTextTag *tag;
//
//     g_return_if_fail (MOO_IS_TEXT_VIEW (view));
//     g_return_if_fail (iter != NULL);
//
//     if (!text || !text[0])
//     {
//         moo_text_view_insert_box (view, iter);
//         return;
//     }
//
//     tag = create_placeholder_tag (view);
//     buffer = get_moo_buffer (view);
//     gtk_text_buffer_insert_with_tags (GTK_TEXT_BUFFER (buffer),
//                                       iter, text, -1, tag, NULL);
// }
//
//
// static gboolean
// has_boxes (MooTextView *view)
// {
//     return view->priv->boxes != NULL;
// }
//
//
// static gboolean
// has_box_at_iter (MooTextView *view,
//                  GtkTextIter *iter)
// {
//     GtkTextChildAnchor *anchor;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//     g_return_val_if_fail (iter != NULL, FALSE);
//
//     if (gtk_text_iter_get_char (iter) != MOO_TEXT_UNKNOWN_CHAR)
//         return FALSE;
//
//     anchor = gtk_text_iter_get_child_anchor (iter);
//     return MOO_IS_TEXT_ANCHOR (anchor) &&
//             MOO_IS_TEXT_BOX (MOO_TEXT_ANCHOR(anchor)->widget);
// }
//
//
// static gboolean
// moo_text_view_find_box_forward (MooTextView *view,
//                                 GtkTextIter *match_start,
//                                 GtkTextIter *match_end)
// {
//     GtkTextIter start;
//     GtkTextBuffer *buffer;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//
//     buffer = get_buffer (view);
//     gtk_text_buffer_get_selection_bounds (buffer, NULL, &start);
//
//     while (gtk_text_iter_forward_search (&start, MOO_TEXT_UNKNOWN_CHAR_S,
//                                          0, match_start, match_end, NULL))
//     {
//         if (has_box_at_iter (view, match_start))
//             return TRUE;
//         else
//             start = *match_end;
//     }
//
//     return FALSE;
// }
//
//
// static gboolean
// moo_text_view_find_placeholder_forward (MooTextView *view,
//                                         GtkTextIter *match_start,
//                                         GtkTextIter *match_end)
// {
//     GtkTextBuffer *buffer;
//     GtkTextTag *tag;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//
//     if (!(tag = get_placeholder_tag (view)))
//         return FALSE;
//
//     buffer = get_buffer (view);
//     gtk_text_buffer_get_selection_bounds (buffer, NULL, match_start);
//
//     if (gtk_text_iter_has_tag (match_start, tag))
//     {
//         if (gtk_text_iter_begins_tag (match_start, tag))
//         {
//             *match_end = *match_start;
//             gtk_text_iter_forward_to_tag_toggle (match_end, tag);
//             return TRUE;
//         }
//
//         if (!gtk_text_iter_forward_to_tag_toggle (match_start, tag))
//             return FALSE;
//     }
//
//     if (!gtk_text_iter_forward_to_tag_toggle (match_start, tag))
//         return FALSE;
//
//     g_assert (gtk_text_iter_begins_tag (match_start, tag));
//
//     *match_end = *match_start;
//     gtk_text_iter_forward_to_tag_toggle (match_end, tag);
//
//     return TRUE;
// }
//
//
// static gboolean
// moo_text_view_find_placeholder_backward (MooTextView *view,
//                                          GtkTextIter *match_start,
//                                          GtkTextIter *match_end)
// {
//     GtkTextBuffer *buffer;
//     GtkTextTag *tag;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//
//     if (!(tag = get_placeholder_tag (view)))
//         return FALSE;
//
//     buffer = get_buffer (view);
//     gtk_text_buffer_get_selection_bounds (buffer, match_start, NULL);
//
//     if (gtk_text_iter_has_tag (match_start, tag))
//     {
//         if (!gtk_text_iter_begins_tag (match_start, tag))
//             gtk_text_iter_backward_to_tag_toggle (match_start, tag);
//     }
//     else if (gtk_text_iter_ends_tag (match_start, tag))
//     {
//         *match_end = *match_start;
//         gtk_text_iter_backward_to_tag_toggle (match_end, tag);
//         return TRUE;
//     }
//
//     if (!gtk_text_iter_backward_to_tag_toggle (match_start, tag))
//         return FALSE;
//
//     g_assert (gtk_text_iter_ends_tag (match_start, tag));
//
//     *match_end = *match_start;
//     gtk_text_iter_backward_to_tag_toggle (match_end, tag);
//
//     return TRUE;
// }
//
//
// static gboolean
// moo_text_view_find_box_backward (MooTextView *view,
//                                  GtkTextIter *match_start,
//                                  GtkTextIter *match_end)
// {
//     GtkTextIter start;
//     GtkTextBuffer *buffer;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//
//     buffer = get_buffer (view);
//     gtk_text_buffer_get_selection_bounds (buffer, &start, NULL);
//
//     while (gtk_text_iter_backward_search (&start, MOO_TEXT_UNKNOWN_CHAR_S,
//                                           0, match_start, match_end, NULL))
//     {
//         if (has_box_at_iter (view, match_start))
//         {
//             start = *match_start;
//             *match_start = *match_end;
//             *match_end = start;
//             return TRUE;
//         }
//         else
//         {
//             start = *match_start;
//         }
//     }
//
//     return FALSE;
// }
//
//
// static gboolean
// moo_text_view_find_placeholder (MooTextView *view,
//                                 gboolean     forward)
// {
//     GtkTextIter box_start, box_end, ph_start, ph_end;
//     GtkTextIter *start, *end;
//     GtkTextBuffer *buffer;
//     gboolean found_box, found_ph;
//
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//
//     buffer = get_buffer (view);
//
//     if (forward)
//     {
//         found_box = moo_text_view_find_box_forward (view, &box_start, &box_end);
//         found_ph = moo_text_view_find_placeholder_forward (view, &ph_start, &ph_end);
//     }
//     else
//     {
//         found_box = moo_text_view_find_box_backward (view, &box_start, &box_end);
//         found_ph = moo_text_view_find_placeholder_backward (view, &ph_start, &ph_end);
//     }
//
//     if (!found_box && !found_ph)
//     {
//         moo_text_view_message (view, "No placeholder found");
//         return FALSE;
//     }
//
//     if (found_box && found_ph)
//     {
//         if (forward)
//             found_box = gtk_text_iter_compare (&box_start, &ph_start) < 0;
//         else
//             found_box = gtk_text_iter_compare (&box_start, &ph_start) > 0;
//     }
//
//     if (found_box)
//     {
//         start = &box_start;
//         end = &box_end;
//     }
//     else
//     {
//         start = &ph_start;
//         end = &ph_end;
//     }
//
//     if (forward)
//         gtk_text_buffer_select_range (buffer, start, end);
//     else
//         gtk_text_buffer_select_range (buffer, end, start);
//
//     scroll_selection_onscreen (GTK_TEXT_VIEW (view));
//     return TRUE;
// }
//
//
// gboolean
// moo_text_view_prev_placeholder (MooTextView *view)
// {
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//     return moo_text_view_find_placeholder (view, FALSE);
// }
//
//
// gboolean
// moo_text_view_next_placeholder (MooTextView *view)
// {
//     g_return_val_if_fail (MOO_IS_TEXT_VIEW (view), FALSE);
//     return moo_text_view_find_placeholder (view, TRUE);
// }
