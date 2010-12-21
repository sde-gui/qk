/**
 * class:GObject
 **/

/**
 * class:GFile: (parent GObject)
 **/

/**
 * class:GdkPixbuf: (parent GObject)
 **/

/**
 * class:GtkObject: (parent GObject)
 **/

/**
 * class:GtkAccelGroup: (parent GObject)
 **/

/**
 * class:GtkWidget: (parent GtkObject)
 **/

/**
 * class:GtkContainer: (parent GtkWidget)
 **/

/**
 * class:GtkTextView: (parent GtkContainer)
 **/

/**
 * class:GtkTextBuffer: (parent GObject)
 **/

/**
 * boxed:GtkTextIter
 **/


/****************************************************************************
 *
 * GFile
 *
 */

/**
 * g_file_new_for_path:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_new_for_uri:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_parse_name:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_dup:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_hash:
 **/

/**
 * g_file_equal:
 **/

/**
 * g_file_get_basename:
 **/

/**
 * g_file_get_path:
 **/

/**
 * g_file_get_uri:
 **/

/**
 * g_file_get_parse_name:
 **/

/**
 * g_file_get_parent:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_has_parent:
 **/

/**
 * g_file_get_child:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_get_child_for_display_name:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_resolve_relative_path:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_has_prefix:
 **/

/**
 * g_file_get_relative_path:
 **/

/**
 * g_file_is_native:
 **/

/**
 * g_file_has_uri_scheme:
 **/

/**
 * g_file_get_uri_scheme:
 **/


/****************************************************************************
 *
 * GtkTextIter
 *
 */

/**
 * gtk_text_iter_get_buffer:
 **/

/**
 * gtk_text_iter_copy:
 *
 * Returns: (transfer full)
 **/

/**
 * gtk_text_iter_get_offset:
 *
 * Returns: (type index)
 **/

/**
 * gtk_text_iter_get_line:
 *
 * Returns: (type index)
 **/

/**
 * gtk_text_iter_get_line_offset:
 *
 * Returns: (type index)
 **/

/**
 * gtk_text_iter_get_line_index:
 *
 * Returns: (type index)
 **/

/**
 * gtk_text_iter_get_char:
 **/

/**
 * gtk_text_iter_get_text:
 **/

/**
 * gtk_text_iter_starts_line:
 **/

/**
 * gtk_text_iter_ends_line:
 **/

/**
 * gtk_text_iter_is_cursor_position:
 **/

/**
 * gtk_text_iter_get_chars_in_line:
 **/

/**
 * gtk_text_iter_get_bytes_in_line:
 **/

/**
 * gtk_text_iter_is_end:
 **/

/**
 * gtk_text_iter_is_start:
 **/

/**
 * gtk_text_iter_forward_char:
 **/

/**
 * gtk_text_iter_backward_char:
 **/

/**
 * gtk_text_iter_forward_chars:
 **/

/**
 * gtk_text_iter_backward_chars:
 **/

/**
 * gtk_text_iter_forward_line:
 **/

/**
 * gtk_text_iter_backward_line:
 **/

/**
 * gtk_text_iter_forward_lines:
 **/

/**
 * gtk_text_iter_backward_lines:
 **/

/**
 * gtk_text_iter_forward_cursor_position:
 **/

/**
 * gtk_text_iter_backward_cursor_position:
 **/

/**
 * gtk_text_iter_forward_cursor_positions:
 **/

/**
 * gtk_text_iter_backward_cursor_positions:
 **/

/**
 * gtk_text_iter_set_offset:
 *
 * @iter:
 * @char_offset: (type index)
 **/

/**
 * gtk_text_iter_set_line:
 *
 * @iter:
 * @line_number: (type index)
 **/

/**
 * gtk_text_iter_set_line_offset:
 *
 * @iter:
 * @char_on_line: (type index)
 **/

/**
 * gtk_text_iter_set_line_index:
 *
 * @iter:
 * @byte_on_line: (type index)
 **/

/**
 * gtk_text_iter_forward_to_end:
 **/

/**
 * gtk_text_iter_forward_to_line_end:
 **/

/**
 * gtk_text_iter_equal:
 **/

/**
 * gtk_text_iter_compare:
 **/

/**
 * gtk_text_iter_in_range:
 **/

/**
 * gtk_text_iter_order:
 **/
