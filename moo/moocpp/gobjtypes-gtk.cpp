/*
 *   moocpp/gobjtypes-gtk.cpp
 *
 *   Copyright (C) 2004-2016 by Yevgen Muntyan <emuntyan@users.sourceforge.net>
 *
 *   This file is part of medit.  medit is free software; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU Lesser General Public License as published by the
 *   Free Software Foundation; either version 2.1 of the License,
 *   or (at your option) any later version.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with medit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "moocpp/moocpp.h"

using namespace moo;
using namespace moo::gtk;

namespace moo {

namespace _test {

void test()
{
    ListStorePtr m1 = wrap_new (gtk_list_store_new (1, G_TYPE_STRING));
    TreeStorePtr m2 = wrap_new (gtk_tree_store_new (1, G_TYPE_STRING));
    TreeModelPtr m3 = wrap_new<GtkTreeModel> (GTK_TREE_MODEL (gtk_tree_store_new (1, G_TYPE_STRING)));
    ListStorePtr m4 = ListStore::create ({ G_TYPE_STRING });

    if (true)
        return;
    
    //gobj_ptr<GtkTreeModel> m4 = m1;

    //create_cpp_gobj<A>();
    //init_cpp_gobj(a);

    //GtkObject* xyz = nullptr;
    //init_cpp_gobj(xyz);

    //create_cpp_gobj<GtkToolbar>(GTK_TYPE_TOOLBAR);

    {
        gobj_ptr<GtkObject> p;
        gobj_ref<GtkObject> r = *p;
        GtkObject* o1 = r.gobj();
        GtkObject* o2 = p->gobj();
        g_assert(o1 == o2);
        GObject* o = p.gobj<GObject>();
        g_assert(o == nullptr);
        GtkObject* x = p.gobj<GtkObject>();
        GObject* y = p.gobj<GObject>();
        g_assert((void*) x == (void*) y);
        const GObject* c1 = p;
        const GtkObject* c2 = p;
        g_assert((void*) c1 == (void*) c2);
    }

    {
        gobj_ptr<GtkWidget> p = wrap_new(gtk_widget_new(0, "blah", nullptr, nullptr));
        gobj_ref<GtkWidget> r = *p;
        GtkWidget* o1 = r.gobj();
        GtkWidget* o2 = p->gobj();
        g_assert(o1 == o2);
        GtkWidget* x = p.gobj<GtkWidget>();
        GtkWidget* y = p.gobj();
        GtkObject* z = p.gobj<GtkObject>();
        GObject* t = p.gobj<GObject>();
        g_assert((void*) x == (void*) y);
        g_assert((void*) z == (void*) t);
        const GObject* c1 = p;
        const GtkObject* c2 = p;
        const GtkWidget* c3 = p;
        g_assert((void*) c1 == (void*) c2);
        g_assert((void*) c1 == (void*) c3);

        gobj_ref<GtkWidget> or(*p.gobj());
        or.freeze_notify();
        p->freeze_notify();

        gobj_raw_ptr<GtkWidget> rp = p.gobj();
    }
}

} // namespace _test
} // namespace moo


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkObject
//

void
Object::destroy ()
{
    gtk_object_destroy (gobj ());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkTreeModel
//

GtkTreeModelFlags
TreeModel::get_flags ()
{
    return gtk_tree_model_get_flags (gobj ());
}

int
TreeModel::get_n_columns ()
{
    return gtk_tree_model_get_n_columns (gobj ());
}

GType
TreeModel::get_column_type (int index_)
{
    return gtk_tree_model_get_column_type (gobj (), index_);
}

bool
TreeModel::get_iter (GtkTreeIter* iter, GtkTreePath* path)
{
    return gtk_tree_model_get_iter (gobj (), iter, path);
}

bool
TreeModel::get_iter_from_string (GtkTreeIter* iter, const char* path_string)
{
    return gtk_tree_model_get_iter_from_string (gobj (), iter, path_string);
}

gstr
TreeModel::get_string_from_iter (GtkTreeIter* iter)
{
    return gtk_tree_model_get_string_from_iter (gobj (), iter);
}

bool
TreeModel::get_iter_first (GtkTreeIter* iter)
{
    return gtk_tree_model_get_iter_first (gobj (), iter);
}

GtkTreePath*
TreeModel::get_path (GtkTreeIter* iter)
{
    return gtk_tree_model_get_path (gobj (), iter);
}

void
TreeModel::get_value (GtkTreeIter* iter, int column, GValue* value)
{
    gtk_tree_model_get_value (gobj (), iter, column, value);
}

bool
TreeModel::iter_next (GtkTreeIter* iter)
{
    return gtk_tree_model_iter_next (gobj (), iter);
}

bool
TreeModel::iter_children (GtkTreeIter* iter, GtkTreeIter* parent)
{
    return gtk_tree_model_iter_children (gobj (), iter, parent);
}

bool
TreeModel::iter_has_child (GtkTreeIter* iter)
{
    return gtk_tree_model_iter_has_child (gobj (), iter);
}

int
TreeModel::iter_n_children (GtkTreeIter* iter)
{
    return gtk_tree_model_iter_n_children (gobj (), iter);
}

bool
TreeModel::iter_nth_child (GtkTreeIter* iter, GtkTreeIter* parent, int n)
{
    return gtk_tree_model_iter_nth_child (gobj (), iter, parent, n);
}

bool
TreeModel::iter_parent (GtkTreeIter* iter, GtkTreeIter* child)
{
    return gtk_tree_model_iter_parent (gobj (), iter, child);
}

void
TreeModel::ref_node (GtkTreeIter* iter)
{
    gtk_tree_model_ref_node (gobj (), iter);
}

void
TreeModel::unref_node (GtkTreeIter* iter)
{
    gtk_tree_model_unref_node (gobj (), iter);
}

void
TreeModel::get (GtkTreeIter* iter, ...)
{
    va_list args;
    va_start (args, iter);
    gtk_tree_model_get_valist (gobj (), iter, args);
    va_end (args);
}

void
TreeModel::get_valist (GtkTreeIter* iter, va_list var_args)
{
    gtk_tree_model_get_valist (gobj (), iter, var_args);
}

void
TreeModel::row_changed (GtkTreePath* path, GtkTreeIter* iter)
{
    gtk_tree_model_row_changed (gobj (), path, iter);
}

void
TreeModel::row_inserted (GtkTreePath* path, GtkTreeIter* iter)
{
    gtk_tree_model_row_inserted (gobj (), path, iter);
}

void
TreeModel::row_has_child_toggled (GtkTreePath* path, GtkTreeIter* iter)
{
    gtk_tree_model_row_has_child_toggled (gobj (), path, iter);
}

void
TreeModel::row_deleted (GtkTreePath* path)
{
    gtk_tree_model_row_deleted (gobj (), path);
}

void
TreeModel::rows_reordered (GtkTreePath* path, GtkTreeIter* iter, gint* new_order)
{
    gtk_tree_model_rows_reordered (gobj (), path, iter, new_order);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkListStore
//

ListStorePtr
ListStore::create (size_t n_types, const GType* types)
{
    g_return_val_if_fail (types != nullptr, nullptr);
    g_return_val_if_fail (n_types != 0, nullptr);
    return wrap_new (gtk_list_store_newv (n_types, const_cast<GType*>(types)));
}

void
ListStore::set_value (GtkTreeIter* iter,
                      int column,
                      GValue* value)
{
    gtk_list_store_set_value (gobj (), iter, column, value);
}

void
ListStore::set (GtkTreeIter* iter, ...)
{
    va_list args;
    va_start (args, iter);
    gtk_list_store_set_valist (gobj (), iter, args);
    va_end (args);
}

void
ListStore::set_valuesv (GtkTreeIter* iter,
                        int* columns,
                        GValue* values,
                        int n_values)
{
    gtk_list_store_set_valuesv (gobj (), iter, columns, values, n_values);
}

void
ListStore::set_valist (GtkTreeIter* iter,
                       va_list var_args)
{
    gtk_list_store_set_valist (gobj (), iter, var_args);
}

bool
ListStore::remove (GtkTreeIter* iter)
{
    return gtk_list_store_remove (gobj (), iter);
}

void
ListStore::insert (GtkTreeIter* iter,
                   int position)
{
    gtk_list_store_insert (gobj (), iter, position);
}

void
ListStore::insert_before (GtkTreeIter* iter,
                          GtkTreeIter* sibling)
{
    gtk_list_store_insert_before (gobj (), iter, sibling);
}

void
ListStore::insert_after (GtkTreeIter* iter,
                         GtkTreeIter* sibling)
{
    gtk_list_store_insert_after (gobj (), iter, sibling);
}

//void
//ListStore::insert_with_values (GtkTreeIter* iter,
//                               int position,
//                               ...)
//{
//    gtk_list_store_insert_with_values (gobj (), iter, position, ...);
//}
//
//void
//ListStore::insert_with_values (GtkTreeIter* iter,
//                               int position,
//                               int* columns,
//                               GValue* values,
//                               int n_values)
//{
//    gtk_list_store_insert_with_values (gobj (), iter, position, columns, values, n_values);
//}

void
ListStore::prepend (GtkTreeIter* iter)
{
    gtk_list_store_prepend (gobj (), iter);
}

void
ListStore::append (GtkTreeIter* iter)
{
    gtk_list_store_append (gobj (), iter);
}

void
ListStore::clear ()
{
    gtk_list_store_clear (gobj ());
}

bool
ListStore::iter_is_valid (GtkTreeIter* iter)
{
    return gtk_list_store_iter_is_valid (gobj (), iter);
}

void
ListStore::reorder (int* new_order)
{
    gtk_list_store_reorder (gobj (), new_order);
}

void
ListStore::swap (GtkTreeIter* a,
                 GtkTreeIter* b)
{
    gtk_list_store_swap (gobj (), a, b);
}

void
ListStore::move_after (GtkTreeIter* iter,
                       GtkTreeIter* position)
{
    gtk_list_store_move_after (gobj (), iter, position);
}

void
ListStore::move_before (GtkTreeIter* iter,
                        GtkTreeIter* position)
{
    gtk_list_store_move_before (gobj (), iter, position);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkTreeView
//

TreeViewPtr
TreeView::create (TreeModelPtr model)
{
    GtkWidget* w = model ? gtk_tree_view_new_with_model (model->gobj ()) : gtk_tree_view_new ();
    return w ? wrap_new (GTK_TREE_VIEW (w)) : nullptr;
}

TreeModelPtr
TreeView::get_model ()
{
    return wrap (gtk_tree_view_get_model (gobj()));
}

void
TreeView::set_model (TreeModelPtr model)
{
    gtk_tree_view_set_model (gobj (), model ? model->gobj () : nullptr);
}

GtkTreeSelection*
TreeView::get_selection ()
{
    return gtk_tree_view_get_selection (gobj ());
}

GtkAdjustment*
TreeView::get_hadjustment ()
{
    return gtk_tree_view_get_hadjustment (gobj ());
}

void
TreeView::set_hadjustment (GtkAdjustment* adjustment)
{
    gtk_tree_view_set_hadjustment (gobj (), adjustment);
}

GtkAdjustment*
TreeView::get_vadjustment ()
{
    return gtk_tree_view_get_vadjustment (gobj ());
}

void
TreeView::set_vadjustment (GtkAdjustment* adjustment)
{
    gtk_tree_view_set_vadjustment (gobj (), adjustment);
}

bool
TreeView::get_headers_visible ()
{
    return gtk_tree_view_get_headers_visible (gobj ());
}

void
TreeView::set_headers_visible (bool headers_visible)
{
    gtk_tree_view_set_headers_visible (gobj (), headers_visible);
}

void
TreeView::columns_autosize ()
{
    gtk_tree_view_columns_autosize (gobj ());
}

bool
TreeView::get_headers_clickable ()
{
    return gtk_tree_view_get_headers_clickable (gobj ());
}

void
TreeView::set_headers_clickable (bool setting)
{
    gtk_tree_view_set_headers_clickable (gobj (), setting);
}

void
TreeView::set_rules_hint (bool setting)
{
    gtk_tree_view_set_rules_hint (gobj (), setting);
}

bool
TreeView::get_rules_hint ()
{
    return gtk_tree_view_get_rules_hint (gobj ());
}

int
TreeView::append_column (TreeViewColumn& column)
{
    return gtk_tree_view_append_column (gobj (), column.gobj ());
}

int
TreeView::remove_column (TreeViewColumn& column)
{
    return gtk_tree_view_remove_column (gobj (), column.gobj ());
}

int
TreeView::insert_column (TreeViewColumn& column,
                         int position)
{
    return gtk_tree_view_insert_column (gobj (), column.gobj (), position);
}

//int
//TreeView::insert_column_with_attributes (int position,
//                                         const char* title,
//                                         CellRenderer& cell,
//                                         ...)
//{
//    gtk_tree_view_insert_column_with_attributes (gobj (), position,
//                                   const char* title,
//                                   CellRenderer& cell,
//                                   ...);
//}

TreeViewColumnPtr
TreeView::get_column (int n)
{
    return wrap_new (gtk_tree_view_get_column (gobj (), n));
}

std::vector<TreeViewColumnPtr>
TreeView::get_columns ()
{
    GList* list = gtk_tree_view_get_columns (gobj ());
    auto ret = object_list_to_vector<GtkTreeViewColumn> (list);
    g_list_free (list);
    return ret;
}

void
TreeView::move_column_after (TreeViewColumn& column,
                             TreeViewColumn& base_column)
{
    return gtk_tree_view_move_column_after (gobj (), column.gobj (), base_column.gobj ());
}

void
TreeView::set_expander_column (TreeViewColumn& column)
{
    return gtk_tree_view_set_expander_column (gobj (), column.gobj ());
}

TreeViewColumnPtr
TreeView::get_expander_column ()
{
    return wrap (gtk_tree_view_get_expander_column (gobj ()));
}

void
TreeView::set_column_drag_function (GtkTreeViewColumnDropFunc func,
                                    gpointer user_data,
                                    GDestroyNotify destroy)
{
    gtk_tree_view_set_column_drag_function (gobj (), func, user_data, destroy);
}

void
TreeView::scroll_to_point (int tree_x,
                           int tree_y)
{
    gtk_tree_view_scroll_to_point (gobj (), tree_x, tree_y);
}

void
TreeView::scroll_to_cell (GtkTreePath* path,
                          TreeViewColumn& column,
                          bool use_align,
                          float row_align,
                          float col_align)
{
    gtk_tree_view_scroll_to_cell (gobj (), path, column.gobj (), use_align, row_align, col_align);
}

void
TreeView::row_activated (GtkTreePath* path,
                         TreeViewColumn& column)
{
    gtk_tree_view_row_activated (gobj (), path, column.gobj ());
}

void
TreeView::expand_all ()
{
    gtk_tree_view_expand_all (gobj ());
}

void
TreeView::collapse_all ()
{
    gtk_tree_view_collapse_all (gobj ());
}

void
TreeView::expand_to_path (GtkTreePath* path)
{
    gtk_tree_view_expand_to_path (gobj (), path);
}

bool
TreeView::expand_row (GtkTreePath* path,
                      bool   open_all)
{
    return gtk_tree_view_expand_row (gobj (), path, open_all);
}

bool
TreeView::collapse_row (GtkTreePath* path)
{
    return gtk_tree_view_collapse_row (gobj (), path);
}

void
TreeView::map_expanded_rows (GtkTreeViewMappingFunc func,
                             gpointer data)
{
    gtk_tree_view_map_expanded_rows (gobj (), func, data);
}

bool
TreeView::row_expanded (GtkTreePath* path)
{
    return gtk_tree_view_row_expanded (gobj (), path);
}

void
TreeView::set_reorderable (bool reorderable)
{
    gtk_tree_view_set_reorderable (gobj (), reorderable);
}

bool
TreeView::get_reorderable ()
{
    return gtk_tree_view_get_reorderable (gobj ());
}

void
TreeView::set_cursor (GtkTreePath* path,
                      TreeViewColumn& focus_column,
                      bool start_editing)
{
    gtk_tree_view_set_cursor (gobj (), path, focus_column.gobj (), start_editing);
}

void
TreeView::set_cursor_on_cell (GtkTreePath* path,
                              TreeViewColumn& focus_column,
                              CellRenderer& focus_cell,
                              bool start_editing)
{
    gtk_tree_view_set_cursor_on_cell (gobj (), path, focus_column.gobj (), focus_cell.gobj (), start_editing);
}

void
TreeView::get_cursor (GtkTreePath** path,
                      TreeViewColumnPtr& focus_column)
{
    GtkTreeViewColumn* col = nullptr;
    gtk_tree_view_get_cursor (gobj (), path, &col);
    focus_column.set (col);
}

GdkWindow*
TreeView::get_bin_window ()
{
    return gtk_tree_view_get_bin_window (gobj ());
}

bool
TreeView::get_path_at_pos (int x,
                           int y,
                           GtkTreePath** path,
                           TreeViewColumnPtr& column,
                           int* cell_x,
                           int* cell_y)
{
    GtkTreeViewColumn* col = nullptr;
    bool ret = gtk_tree_view_get_path_at_pos (gobj (), x, y, path, &col, cell_x, cell_y);
    column.set (col);
    return ret;
}

void
TreeView::get_cell_area (GtkTreePath* path,
                         TreeViewColumn& column,
                         GdkRectangle& rect)
{
    gtk_tree_view_get_cell_area (gobj (), path, column.gobj(), &rect);
}

void
TreeView::get_background_area (GtkTreePath* path,
                               TreeViewColumn& column,
                               GdkRectangle& rect)
{
    gtk_tree_view_get_background_area (gobj (), path, column.gobj (), &rect);
}

void
TreeView::get_visible_rect (GdkRectangle& visible_rect)
{
    gtk_tree_view_get_visible_rect (gobj (), &visible_rect);
}

bool
TreeView::get_visible_range (GtkTreePath** start_path,
                             GtkTreePath** end_path)
{
    return gtk_tree_view_get_visible_range (gobj (), start_path, end_path);
}

void
TreeView::enable_model_drag_source (GdkModifierType start_button_mask,
                                    const GtkTargetEntry* targets,
                                    int n_targets,
                                    GdkDragAction actions)
{
    gtk_tree_view_enable_model_drag_source (gobj (), start_button_mask,
                                            targets, n_targets, actions);
}

void
TreeView::enable_model_drag_dest (const GtkTargetEntry* targets,
                                  int n_targets,
                                  GdkDragAction actions)
{
    gtk_tree_view_enable_model_drag_dest (gobj (), targets, n_targets, actions);
}

void
TreeView::unset_rows_drag_source ()
{
    gtk_tree_view_unset_rows_drag_source (gobj ());
}

void
TreeView::unset_rows_drag_dest ()
{
    gtk_tree_view_unset_rows_drag_dest (gobj ());
}

void
TreeView::set_drag_dest_row (GtkTreePath* path,
                             GtkTreeViewDropPosition pos)
{
    gtk_tree_view_set_drag_dest_row (gobj (), path, pos);
}

void
TreeView::get_drag_dest_row (GtkTreePath** path,
                             GtkTreeViewDropPosition* pos)
{
    gtk_tree_view_get_drag_dest_row (gobj (), path, pos);
}

bool
TreeView::get_dest_row_at_pos (int drag_x,
                               int drag_y,
                               GtkTreePath** path,
                               GtkTreeViewDropPosition* pos)
{
    return gtk_tree_view_get_dest_row_at_pos (gobj (), drag_x, drag_y, path, pos);
}

GdkPixmap*
TreeView::create_row_drag_icon (GtkTreePath* path)
{
    return gtk_tree_view_create_row_drag_icon (gobj (), path);
}

void
TreeView::set_enable_search (bool enable_search)
{
    gtk_tree_view_set_enable_search (gobj (), enable_search);
}

bool
TreeView::get_enable_search ()
{
    return gtk_tree_view_get_enable_search (gobj ());
}

int
TreeView::get_search_column ()
{
    return gtk_tree_view_get_search_column (gobj ());
}

void
TreeView::set_search_column (int column)
{
    gtk_tree_view_set_search_column (gobj (), column);
}

GtkTreeViewSearchEqualFunc
TreeView::get_search_equal_func ()
{
    return gtk_tree_view_get_search_equal_func (gobj ());
}

void
TreeView::set_search_equal_func (GtkTreeViewSearchEqualFunc search_equal_func,
                                 gpointer search_user_data,
                                 GDestroyNotify search_destroy)
{
    gtk_tree_view_set_search_equal_func (gobj (), search_equal_func, search_user_data, search_destroy);
}

GtkEntry*
TreeView::get_search_entry ()
{
    return gtk_tree_view_get_search_entry (gobj ());
}

void
TreeView::set_search_entry (GtkEntry* entry)
{
    gtk_tree_view_set_search_entry (gobj (), entry);
}

GtkTreeViewSearchPositionFunc
TreeView::get_search_position_func ()
{
    return gtk_tree_view_get_search_position_func (gobj ());
}

void
TreeView::set_search_position_func (GtkTreeViewSearchPositionFunc func,
                                    gpointer data,
                                    GDestroyNotify destroy)
{
    gtk_tree_view_set_search_position_func (gobj (), func, data, destroy);
}

void
TreeView::convert_widget_to_tree_coords (int wx,
                                         int wy,
                                         int* tx,
                                         int* ty)
{
    gtk_tree_view_convert_widget_to_tree_coords (gobj (), wx, wy, tx, ty);
}

void
TreeView::convert_tree_to_widget_coords (int tx,
                                         int ty,
                                         int* wx,
                                         int* wy)
{
    gtk_tree_view_convert_tree_to_widget_coords (gobj (), tx, ty, wx, wy);
}

void
TreeView::convert_widget_to_bin_window_coords (int wx,
                                               int wy,
                                               int* bx,
                                               int* by)
{
    gtk_tree_view_convert_widget_to_bin_window_coords (gobj (), wx, wy, bx, by);
}

void
TreeView::convert_bin_window_to_widget_coords (int bx,
                                               int by,
                                               int* wx,
                                               int* wy)
{
    gtk_tree_view_convert_bin_window_to_widget_coords (gobj (), bx, by, wx, wy);
}

void
TreeView::convert_tree_to_bin_window_coords (int tx,
                                             int ty,
                                             int* bx,
                                             int* by)
{
    gtk_tree_view_convert_tree_to_bin_window_coords (gobj (), tx, ty, bx, by);
}

void
TreeView::convert_bin_window_to_tree_coords (int bx,
                                             int by,
                                             int* tx,
                                             int* ty)
{
    gtk_tree_view_convert_bin_window_to_tree_coords (gobj (), bx, by, tx, ty);
}

void
TreeView::set_fixed_height_mode (bool enable)
{
    gtk_tree_view_set_fixed_height_mode (gobj (), enable);
}

bool
TreeView::get_fixed_height_mode ()
{
    return gtk_tree_view_get_fixed_height_mode (gobj ());
}

void
TreeView::set_hover_selection (bool hover)
{
    gtk_tree_view_set_hover_selection (gobj (), hover);
}

bool
TreeView::get_hover_selection ()
{
    return gtk_tree_view_get_hover_selection (gobj ());
}

void
TreeView::set_hover_expand (bool expand)
{
    gtk_tree_view_set_hover_expand (gobj (), expand);
}

bool
TreeView::get_hover_expand ()
{
    return gtk_tree_view_get_hover_expand (gobj ());
}

void
TreeView::set_rubber_banding (bool enable)
{
    gtk_tree_view_set_rubber_banding (gobj (), enable);
}

bool
TreeView::get_rubber_banding ()
{
    return gtk_tree_view_get_rubber_banding (gobj ());
}

bool
TreeView::is_rubber_banding_active ()
{
    return gtk_tree_view_is_rubber_banding_active (gobj ());
}

GtkTreeViewRowSeparatorFunc
TreeView::get_row_separator_func ()
{
    return gtk_tree_view_get_row_separator_func (gobj ());
}

void
TreeView::set_row_separator_func (GtkTreeViewRowSeparatorFunc func,
                                  gpointer data,
                                  GDestroyNotify destroy)
{
    gtk_tree_view_set_row_separator_func (gobj (), func, data, destroy);
}

GtkTreeViewGridLines
TreeView::get_grid_lines ()
{
    return gtk_tree_view_get_grid_lines (gobj ());
}

void
TreeView::set_grid_lines (GtkTreeViewGridLines grid_lines)
{
    gtk_tree_view_set_grid_lines (gobj (), grid_lines);
}

bool
TreeView::get_enable_tree_lines ()
{
    return gtk_tree_view_get_enable_tree_lines (gobj ());
}

void
TreeView::set_enable_tree_lines (bool enabled)
{
    gtk_tree_view_set_enable_tree_lines (gobj (), enabled);
}

void
TreeView::set_show_expanders (bool enabled)
{
    gtk_tree_view_set_show_expanders (gobj (), enabled);
}

bool
TreeView::get_show_expanders ()
{
    return gtk_tree_view_get_show_expanders (gobj ());
}

void
TreeView::set_level_indentation (int indentation)
{
    gtk_tree_view_set_level_indentation (gobj (), indentation);
}

int
TreeView::get_level_indentation ()
{
    return gtk_tree_view_get_level_indentation (gobj ());
}

void
TreeView::set_tooltip_row (GtkTooltip* tooltip,
                           GtkTreePath* path)
{
    gtk_tree_view_set_tooltip_row (gobj (), tooltip, path);
}

void
TreeView::set_tooltip_cell (GtkTooltip* tooltip,
                            GtkTreePath* path,
                            TreeViewColumn& column,
                            CellRenderer& cell)
{
    gtk_tree_view_set_tooltip_cell (gobj (), tooltip, path, column.gobj (), cell.gobj ());
}

bool
TreeView::get_tooltip_context (int* x,
                               int* y,
                               bool keyboard_tip,
                               GtkTreeModel** model,
                               GtkTreePath** path,
                               GtkTreeIter* iter)
{
    return gtk_tree_view_get_tooltip_context (gobj (), x, y, keyboard_tip, model, path, iter);
}

void
TreeView::set_tooltip_column (int column)
{
    gtk_tree_view_set_tooltip_column (gobj (), column);
}

int
TreeView::get_tooltip_column ()
{
    return gtk_tree_view_get_tooltip_column (gobj ());
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkTreeViewColumn
//

TreeViewColumnPtr
TreeViewColumn::create ()
{
    return wrap_new (gtk_tree_view_column_new ());
}

//TreeViewColumnPtr
//TreeViewColumn::create (const char* title,
//                                      CellRendererPtr cell,
//                                      ...)
//{
//}

void
TreeViewColumn::pack_start (CellRenderer& cell, bool expand)
{
    gtk_tree_view_column_pack_start (gobj (), cell.gobj(), expand);
}

void
TreeViewColumn::pack_end (CellRenderer& cell, bool expand)
{
    gtk_tree_view_column_pack_end (gobj (), cell.gobj (), expand);
}

void
TreeViewColumn::clear ()
{
    gtk_tree_view_column_clear (gobj ());
}

std::vector<CellRendererPtr>
TreeViewColumn::get_cell_renderers ()
{
    GList* list = gtk_tree_view_column_get_cell_renderers (gobj ());
    auto ret = object_list_to_vector<GtkCellRenderer> (list);
    g_list_free (list);
    return ret;
}

void
TreeViewColumn::add_attribute (CellRenderer& cell,
                               const char* attribute,
                               int column)
{
    gtk_tree_view_column_add_attribute (gobj (), cell.gobj (), attribute, column);
}

//void
//TreeViewColumn::set_attributes (CellRenderer& cell_renderer, ...)
//{
//    gtk_tree_view_column_set_attributes (gobj (), cell_renderer.gobj (), ...);
//}

//void
//TreeViewColumn::set_cell_data_func (CellRenderer& cell_renderer,
//                                    GtkTreeCellDataFunc func,
//                                    gpointer func_data,
//                                    GDestroyNotify destroy)
//{
//    gtk_tree_view_column_set_cell_data_func (gobj (), cell_renderer.gobj (), func, func_data, destroy);
//}

void
TreeViewColumn::clear_attributes (CellRenderer& cell_renderer)
{
    gtk_tree_view_column_clear_attributes (gobj (), cell_renderer.gobj ());
}

void
TreeViewColumn::set_spacing (int spacing)
{
    gtk_tree_view_column_set_spacing (gobj (), spacing);
}

int
TreeViewColumn::get_spacing ()
{
    return gtk_tree_view_column_get_spacing (gobj ());
}

void
TreeViewColumn::set_visible (bool visible)
{
    gtk_tree_view_column_set_visible (gobj (), visible);
}

bool
TreeViewColumn::get_visible ()
{
    return gtk_tree_view_column_get_visible (gobj ());
}

void
TreeViewColumn::set_resizable (bool resizable)
{
    gtk_tree_view_column_set_resizable (gobj (), resizable);
}

bool
TreeViewColumn::get_resizable ()
{
    return gtk_tree_view_column_get_resizable (gobj ());
}

void
TreeViewColumn::set_sizing (GtkTreeViewColumnSizing type)
{
    gtk_tree_view_column_set_sizing (gobj (), type);
}

GtkTreeViewColumnSizing
TreeViewColumn::get_sizing ()
{
    return gtk_tree_view_column_get_sizing (gobj ());
}

int
TreeViewColumn::get_width ()
{
    return gtk_tree_view_column_get_width (gobj ());
}

int
TreeViewColumn::get_fixed_width ()
{
    return gtk_tree_view_column_get_fixed_width (gobj ());
}

void
TreeViewColumn::set_fixed_width (int fixed_width)
{
    gtk_tree_view_column_set_fixed_width (gobj (), fixed_width);
}

void
TreeViewColumn::set_min_width (int min_width)
{
    gtk_tree_view_column_set_min_width (gobj (), min_width);
}

int
TreeViewColumn::get_min_width ()
{
    return gtk_tree_view_column_get_min_width (gobj ());
}

void
TreeViewColumn::set_max_width (int max_width)
{
    gtk_tree_view_column_set_max_width (gobj (), max_width);
}

int
TreeViewColumn::get_max_width ()
{
    return gtk_tree_view_column_get_max_width (gobj ());
}

void
TreeViewColumn::clicked ()
{
    gtk_tree_view_column_clicked (gobj ());
}

void
TreeViewColumn::set_title (const char* title)
{
    gtk_tree_view_column_set_title (gobj (), title);
}

gstr
TreeViewColumn::get_title ()
{
    return wrap (gtk_tree_view_column_get_title (gobj ()));
}

void
TreeViewColumn::set_expand (bool expand)
{
    gtk_tree_view_column_set_expand (gobj (), expand);
}

bool
TreeViewColumn::get_expand ()
{
    return gtk_tree_view_column_get_expand (gobj ());
}

void
TreeViewColumn::set_clickable (bool clickable)
{
    gtk_tree_view_column_set_clickable (gobj (), clickable);
}

bool
TreeViewColumn::get_clickable ()
{
    return gtk_tree_view_column_get_clickable (gobj ());
}

void
TreeViewColumn::set_widget (WidgetPtr widget)
{
    gtk_tree_view_column_set_widget (gobj (), widget.gobj ());
}

WidgetPtr
TreeViewColumn::get_widget ()
{
    return wrap (gtk_tree_view_column_get_widget (gobj ()));
}

void
TreeViewColumn::set_alignment (float xalign)
{
    gtk_tree_view_column_set_alignment (gobj (), xalign);
}

float
TreeViewColumn::get_alignment ()
{
    return gtk_tree_view_column_get_alignment (gobj ());
}

void
TreeViewColumn::set_reorderable (bool reorderable)
{
    gtk_tree_view_column_set_reorderable (gobj (), reorderable);
}

bool
TreeViewColumn::get_reorderable ()
{
    return gtk_tree_view_column_get_reorderable (gobj ());
}

void
TreeViewColumn::set_sort_column_id (int sort_column_id)
{
    gtk_tree_view_column_set_sort_column_id (gobj (), sort_column_id);
}

int
TreeViewColumn::get_sort_column_id ()
{
    return gtk_tree_view_column_get_sort_column_id (gobj ());
}

void
TreeViewColumn::set_sort_indicator (bool setting)
{
    gtk_tree_view_column_set_sort_indicator (gobj (), setting);
}

bool
TreeViewColumn::get_sort_indicator ()
{
    return gtk_tree_view_column_get_sort_indicator (gobj ());
}

void
TreeViewColumn::set_sort_order (GtkSortType order)
{
    gtk_tree_view_column_set_sort_order (gobj (), order);
}

GtkSortType
TreeViewColumn::get_sort_order ()
{
    return gtk_tree_view_column_get_sort_order (gobj ());
}

void
TreeViewColumn::cell_set_cell_data (TreeModel& tree_model,
                                    GtkTreeIter* iter,
                                    bool is_expander,
                                    bool is_expanded)
{
    gtk_tree_view_column_cell_set_cell_data (gobj (), tree_model.gobj (), iter, is_expander, is_expanded);
}

void
TreeViewColumn::cell_get_size (const GdkRectangle* cell_area,
                               int* x_offset,
                               int* y_offset,
                               int* width,
                               int* height)
{
    gtk_tree_view_column_cell_get_size (gobj (), cell_area, x_offset, y_offset, width, height);
}

bool
TreeViewColumn::cell_is_visible ()
{
    return gtk_tree_view_column_cell_is_visible (gobj ());
}

void
TreeViewColumn::focus_cell (CellRenderer& cell)
{
    gtk_tree_view_column_focus_cell (gobj (), cell.gobj ());
}

bool
TreeViewColumn::cell_get_position (CellRenderer& cell_renderer,
                                   int* start_pos,
                                   int* width)
{
    return gtk_tree_view_column_cell_get_position (gobj (), cell_renderer.gobj (), start_pos, width);
}

void
TreeViewColumn::queue_resize ()
{
    gtk_tree_view_column_queue_resize (gobj ());
}

TreeViewPtr
TreeViewColumn::get_tree_view ()
{
    GtkWidget* w = gtk_tree_view_column_get_tree_view (gobj ());
    return w ? wrap (GTK_TREE_VIEW (w)) : nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkCellRenderer
//


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkCellRendererText
//

gtk::CellRendererTextPtr
CellRendererText::create ()
{
    return wrap_new (up_cast<GtkCellRendererText> (gtk_cell_renderer_text_new ()));
}

void CellRendererText::set_fixed_height_from_font (int number_of_rows)
{
    gtk_cell_renderer_text_set_fixed_height_from_font (gobj (), number_of_rows);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GtkCellRendererToggle
//

gtk::CellRendererTogglePtr
CellRendererToggle::create ()
{
    return wrap_new (up_cast<GtkCellRendererToggle> (gtk_cell_renderer_toggle_new ()));
}

bool
CellRendererToggle::get_radio ()
{
    return gtk_cell_renderer_toggle_get_radio (gobj ());
}

void
CellRendererToggle::set_radio (bool radio)
{
    gtk_cell_renderer_toggle_set_radio (gobj (), radio);
}

bool
CellRendererToggle::get_active ()
{
    return gtk_cell_renderer_toggle_get_active (gobj ());
}

void
CellRendererToggle::set_active (bool active)
{
    gtk_cell_renderer_toggle_set_active (gobj(), active);
}
