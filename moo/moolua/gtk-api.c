/**
 * class:GObject
 **/

/**
 * class:GFile: (parent GObject) (moo.python 0)
 **/

/**
 * class:GdkPixbuf: (parent GObject) (moo.python 0)
 **/

/**
 * class:GtkObject: (parent GObject) (moo.python 0)
 **/

/**
 * class:GtkAccelGroup: (parent GObject) (moo.python 0)
 **/

/**
 * class:GtkWidget: (parent GtkObject) (moo.python 0)
 **/

/**
 * class:GtkContainer: (parent GtkWidget) (moo.python 0)
 **/

/**
 * class:GtkTextView: (parent GtkContainer) (moo.python 0)
 **/

/**
 * class:GtkTextBuffer: (parent GObject) (moo.python 0)
 **/

/**
 * boxed:GtkTextIter: (moo.python 0)
 **/

/**
 * boxed:GdkRectangle: (moo.python 0)
 **/

/**
 * enum:GtkResponseType: (moo.python 0)
 **/


/****************************************************************************
 *
 * GFile
 *
 */

/**
 * g_file_new_for_path: (static-method-of GFile)
 *
 * @path: (type const-filename)
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_new_for_uri: (static-method-of GFile)
 *
 * @uri: (type const-utf8)
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_parse_name: (static-method-of GFile)
 *
 * @parse_name: (type const-utf8)
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
 *
 * Returns: (type filename)
 **/

/**
 * g_file_get_path:
 *
 * Returns: (type filename)
 **/

/**
 * g_file_get_uri:
 *
 * Returns: (type utf8)
 **/

/**
 * g_file_get_parse_name:
 *
 * Returns: (type utf8)
 **/

/**
 * g_file_get_parent:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_has_parent:
 *
 * @file:
 * @parent: (allow-none) (default NULL)
 **/

/**
 * g_file_get_child:
 *
 * @file:
 * @name: (type const-filename)
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_get_child_for_display_name:
 *
 * @file:
 * @display_name: (type const-utf8)
 * @error:
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_resolve_relative_path:
 *
 * @file:
 * @relative_path: (type const-filename)
 *
 * Returns: (transfer full)
 **/

/**
 * g_file_has_prefix:
 **/

/**
 * g_file_get_relative_path:
 *
 * Returns: (type filename)
 **/

/**
 * g_file_is_native:
 **/

/**
 * g_file_has_uri_scheme:
 *
 * @file:
 * @uri_scheme: (type const-utf8)
 **/

/**
 * g_file_get_uri_scheme:
 *
 * Returns: (type utf8)
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
 *
 * Returns: (type utf8)
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
