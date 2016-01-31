/*
 *   moocpp/gobjtypes-gtk.h
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

#pragma once

#include <gtk/gtk.h>
#include <mooglib/moo-glib.h>

#ifdef __cplusplus

#include "moocpp/gobjtypes-glib.h"
#include <initializer_list>
#include <functional>

#define MOO_DEFINE_GTK_TYPE(Object, Parent, obj_g_type)         \
    MOO_DEFINE_GOBJ_TYPE(Gtk##Object, Parent, obj_g_type)       \
    namespace moo {                                             \
    namespace gtk {                                             \
    MOO_GOBJ_TYPEDEFS(Object, Gtk##Object)                      \
    }                                                           \
    }

#define MOO_DEFINE_GTK_IFACE(Iface, iface_g_type)               \
    MOO_DEFINE_GIFACE_TYPE(Gtk##Iface, iface_g_type)            \
    namespace moo {                                             \
    namespace gtk {                                             \
    MOO_GIFACE_TYPEDEFS(Iface, Gtk##Iface)                      \
    }                                                           \
    }


MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkObject)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkWidget)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkTextView)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkTreeModel)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkTreeStore)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkListStore)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkTreeView)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkTreeViewColumn)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkCellRenderer)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkCellRendererText)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkCellRendererToggle)
MOO_DECLARE_CUSTOM_GOBJ_TYPE (GtkCellRendererPixbuf)


MOO_DEFINE_GTK_TYPE (Object, GObject, GTK_TYPE_OBJECT)
MOO_DEFINE_GTK_TYPE (Widget, GtkObject, GTK_TYPE_WIDGET)

template<>
class ::moo::gobj_ref<GtkObject> : public virtual ::moo::gobj_ref_parent<GtkObject>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkObject);

    void destroy ();
};

template<>
class ::moo::gobj_ref<GtkWidget> : public virtual ::moo::gobj_ref_parent<GtkWidget>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkWidget);
};


MOO_DEFINE_GTK_TYPE(Entry, GtkWidget, GTK_TYPE_ENTRY)
MOO_DEFINE_GTK_TYPE(Action, GObject, GTK_TYPE_ACTION)
MOO_DEFINE_GTK_TYPE(TextBuffer, GObject, GTK_TYPE_TEXT_BUFFER)
MOO_DEFINE_GTK_TYPE(TextMark, GObject, GTK_TYPE_TEXT_MARK)
MOO_DEFINE_GTK_TYPE(MenuShell, GtkWidget, GTK_TYPE_MENU_SHELL)
MOO_DEFINE_GTK_TYPE(Menu, GtkMenuShell, GTK_TYPE_MENU)


MOO_DEFINE_GTK_TYPE (TextView, GtkWidget, GTK_TYPE_TEXT_VIEW)

template<>
class ::moo::gobj_ref<GtkTextView> : public virtual ::moo::gobj_ref_parent<GtkTextView>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GtkTextView);

    gtk::TextBuffer get_buffer  ();
};


MOO_DEFINE_GTK_IFACE (TreeModel, GTK_TYPE_TREE_MODEL)

template<>
class ::moo::gobj_ref<GtkTreeModel> : public virtual ::moo::gobj_ref_parent<GtkTreeModel>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkTreeModel);

    GtkTreeModelFlags get_flags ();
    int get_n_columns ();
    GType get_column_type (int index_);

    bool get_iter (GtkTreeIter* iter, GtkTreePath* path);
    bool get_iter_from_string (GtkTreeIter* iter, const char* path_string);
    gstr get_string_from_iter (GtkTreeIter* iter);
    bool get_iter_first (GtkTreeIter* iter);
    GtkTreePath* get_path (GtkTreeIter* iter);
    void get_value (GtkTreeIter* iter, int column, GValue* value);
    bool iter_next (GtkTreeIter* iter);
    bool iter_children (GtkTreeIter* iter, GtkTreeIter* parent);
    bool iter_has_child (GtkTreeIter* iter);
    int iter_n_children (GtkTreeIter* iter);
    bool iter_nth_child (GtkTreeIter* iter, GtkTreeIter* parent, int n);
    bool iter_parent (GtkTreeIter* iter, GtkTreeIter* child);
    void ref_node (GtkTreeIter* iter);
    void unref_node (GtkTreeIter* iter);

    template<typename T>
    void get (GtkTreeIter* iter, int column, T&& dest)
    {
        gtk_tree_model_get (gobj (), iter, column, std::forward<T> (dest), -1);
    }

    template<typename T, typename... Args>
    void get (GtkTreeIter* iter, int column, T&& dest, Args&&... args)
    {
        get (iter, column, std::forward<T> (dest));
        get (iter, std::forward<Args> (args)...);
    }

    // bool TFunc (GtkTreePath*, GtkTreeIter*)
    template<typename TFunc>
    void foreach (const TFunc& func)
    {
        const void* p = &func;
        gtk_tree_model_foreach (gobj (), foreach_func<TFunc>, const_cast<void*>(p));
    }

    void row_changed (GtkTreePath* path, GtkTreeIter* iter);
    void row_inserted (GtkTreePath* path, GtkTreeIter* iter);
    void row_has_child_toggled (GtkTreePath* path, GtkTreeIter* iter);
    void row_deleted (GtkTreePath* path);
    void rows_reordered (GtkTreePath* path, GtkTreeIter* iter, gint* new_order);

private:
    template<typename TFunc>
    static gboolean foreach_func (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
    {
        const TFunc& func = *reinterpret_cast<const TFunc*>(data);
        return func (path, iter);
    }
};

MOO_DEFINE_GTK_TYPE (ListStore, GObject, GTK_TYPE_LIST_STORE)
MOO_GOBJ_IMPLEMENTS_IFACE (GtkListStore, GtkTreeModel)

template<>
class ::moo::gobj_ref<GtkListStore>
    : public virtual ::moo::gobj_ref_parent<GtkListStore>
    , public virtual ::moo::gobj_ref<GtkTreeModel>
{
public:
    MOO_DEFINE_GOBJREF_METHODS(GtkListStore);

    static gtk::ListStorePtr create (std::initializer_list<GType> types)
    {
        return create (types.size (), types.begin ());
    }

    static gtk::ListStorePtr create (size_t n_columns, std::initializer_list<GType> types)
    {
        g_return_val_if_fail (n_columns == types.size (), nullptr);
        return create (n_columns, types.begin ());
    }

    static gtk::ListStorePtr create (size_t n_columns, const GType* types);

    template<typename T>
    void set (GtkTreeIter* iter, int column, const T& value)
    {
        gtk_list_store_set (gobj (), iter, column, cpp_vararg_value_fixer<T>::apply(value), -1);
    }

    template<typename T, typename... Args>
    void set (GtkTreeIter* iter, int column, const T& value, Args&&... args)
    {
        set (iter, column, value);
        set (iter, std::forward<Args> (args)...);
    }

    void    set_value (GtkTreeIter* iter,
                       int column,
                       GValue* value);
    void    set_valuesv (GtkTreeIter* iter,
                         int* columns,
                         GValue* values,
                         int n_values);
    bool    remove (GtkTreeIter* iter);
    void    insert (GtkTreeIter* iter,
                    int position);
    void    insert_before (GtkTreeIter* iter,
                           GtkTreeIter* sibling);
    void    insert_after (GtkTreeIter* iter,
                          GtkTreeIter* sibling);
    //void    insert_with_values (GtkTreeIter* iter,
    //                            int position,
    //                            ...);
    //void    insert_with_values (GtkTreeIter* iter,
    //                            int position,
    //                            int* columns,
    //                            GValue* values,
    //                            int n_values);
    void    prepend (GtkTreeIter* iter);
    void    append (GtkTreeIter* iter);
    void    clear ();
    bool    iter_is_valid (GtkTreeIter* iter);
    void    reorder (int* new_order);
    void    swap (GtkTreeIter* a,
                  GtkTreeIter* b);
    void    move_after (GtkTreeIter* iter,
                        GtkTreeIter* position);
    void    move_before (GtkTreeIter* iter,
                         GtkTreeIter* position);
};

MOO_DEFINE_GTK_TYPE (TreeStore, GObject, GTK_TYPE_TREE_STORE)
MOO_GOBJ_IMPLEMENTS_IFACE (GtkTreeStore, GtkTreeModel)

template<>
class ::moo::gobj_ref<GtkTreeStore>
    : public virtual ::moo::gobj_ref_parent<GtkTreeStore>
    , public virtual ::moo::gobj_ref<GtkTreeModel>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkTreeStore);
};

MOO_DEFINE_GTK_TYPE (TreeView, GtkContainer, GTK_TYPE_TREE_VIEW)
MOO_DEFINE_GTK_TYPE (TreeViewColumn, GtkObject, GTK_TYPE_TREE_VIEW_COLUMN)
MOO_DEFINE_GTK_TYPE (CellRenderer, GtkObject, GTK_TYPE_CELL_RENDERER)
MOO_DEFINE_GTK_TYPE (CellRendererText, GtkCellRenderer, GTK_TYPE_CELL_RENDERER_TEXT)
MOO_DEFINE_GTK_TYPE (CellRendererPixbuf, GtkCellRenderer, GTK_TYPE_CELL_RENDERER_PIXBUF)
MOO_DEFINE_GTK_TYPE (CellRendererToggle, GtkCellRenderer, GTK_TYPE_CELL_RENDERER_TOGGLE)


template<typename TFunc>
class TreeCellDataFunc
{
public:
    TreeCellDataFunc (TFunc func) : m_func (std::move (func)) {}
    ~TreeCellDataFunc () {}

    MOO_DISABLE_COPY_OPS (TreeCellDataFunc);

    static void destroy (gpointer d) { delete reinterpret_cast<TreeCellDataFunc*>(d); }

    static void cell_data_func (GtkTreeViewColumn *tree_column,
                                GtkCellRenderer   *cell,
                                GtkTreeModel      *tree_model,
                                GtkTreeIter       *iter,
                                gpointer           p);

private:
    TFunc m_func;
};


template<>
class ::moo::gobj_ref<GtkTreeView> : public virtual ::moo::gobj_ref_parent<GtkTreeView>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkTreeView);

    static gtk::TreeViewPtr create (gtk::TreeModelPtr model);

    gtk::TreeModelPtr get_model ();
    void    set_model (gtk::TreeModelPtr model);
    GtkTreeSelection* get_selection ();
    GtkAdjustment* get_hadjustment ();
    void    set_hadjustment (GtkAdjustment* adjustment);
    GtkAdjustment* get_vadjustment ();
    void    set_vadjustment (GtkAdjustment* adjustment);
    bool    get_headers_visible ();
    void    set_headers_visible (bool headers_visible);
    void    columns_autosize ();
    bool    get_headers_clickable ();
    void    set_headers_clickable (bool setting);
    void    set_rules_hint (bool setting);
    bool    get_rules_hint ();

    /* Column funtions */
    int     append_column (gtk::TreeViewColumn& column);
    int     remove_column (gtk::TreeViewColumn& column);
    int     insert_column (gtk::TreeViewColumn& column,
                           int position);
    int     insert_column_with_attributes (int position,
                                           const char* title,
                                           gtk::CellRenderer& cell,
                                           ...) G_GNUC_NULL_TERMINATED;

    template<typename TFunc>
    int     insert_column_with_data_func (int position,
                                          const char* title,
                                          gtk::CellRenderer& cell,
                                          TFunc func);

    gtk::TreeViewColumnPtr get_column (int n);
    std::vector<gtk::TreeViewColumnPtr> get_columns ();
    void    move_column_after (gtk::TreeViewColumn& column,
                               gtk::TreeViewColumn& base_column);
    void    set_expander_column (gtk::TreeViewColumn& column);
    gtk::TreeViewColumnPtr get_expander_column ();
    void    set_column_drag_function (GtkTreeViewColumnDropFunc func,
                                      gpointer user_data,
                                      GDestroyNotify destroy);

    /* Actions */
    void    scroll_to_point (int tree_x,
                             int tree_y);
    void    scroll_to_cell (GtkTreePath* path,
                            gtk::TreeViewColumn& column,
                            bool use_align,
                            float row_align,
                            float col_align);
    void    row_activated (GtkTreePath* path,
                           gtk::TreeViewColumn& column);
    void    expand_all ();
    void    collapse_all ();
    void    expand_to_path (GtkTreePath* path);
    bool    expand_row (GtkTreePath* path,
                        bool   open_all);
    bool    collapse_row (GtkTreePath* path);
    void    map_expanded_rows (GtkTreeViewMappingFunc func,
                               gpointer data);
    bool    row_expanded (GtkTreePath* path);
    void    set_reorderable (bool reorderable);
    bool    get_reorderable ();
    void    set_cursor (GtkTreePath* path,
                        gtk::TreeViewColumn& focus_column,
                        bool start_editing);
    void    set_cursor_on_cell (GtkTreePath* path,
                                gtk::TreeViewColumn& focus_column,
                                gtk::CellRenderer& focus_cell,
                                bool start_editing);
    void    get_cursor (GtkTreePath** path,
                        gtk::TreeViewColumnPtr& focus_column);

    /* Layout information */
    GdkWindow* get_bin_window ();
    bool    get_path_at_pos (int x,
                             int y,
                             GtkTreePath** path,
                             gtk::TreeViewColumnPtr& column,
                             int* cell_x,
                             int* cell_y);
    void    get_cell_area (GtkTreePath* path,
                           gtk::TreeViewColumn& column,
                           GdkRectangle& rect);
    void    get_background_area (GtkTreePath* path,
                                 gtk::TreeViewColumn& column,
                                 GdkRectangle& rect);
    void    get_visible_rect (GdkRectangle& visible_rect);

    bool    get_visible_range (GtkTreePath** start_path,
                               GtkTreePath** end_path);

    /* Drag-and-Drop support */
    void    enable_model_drag_source (GdkModifierType start_button_mask,
                                      const GtkTargetEntry* targets,
                                      int n_targets,
                                      GdkDragAction actions);
    void    enable_model_drag_dest (const GtkTargetEntry* targets,
                                    int n_targets,
                                    GdkDragAction actions);
    void    unset_rows_drag_source ();
    void    unset_rows_drag_dest ();

    /* These are useful to implement your own custom stuff. */
    void    set_drag_dest_row (GtkTreePath* path,
                               GtkTreeViewDropPosition pos);
    void    get_drag_dest_row (GtkTreePath** path,
                               GtkTreeViewDropPosition* pos);
    bool    get_dest_row_at_pos (int drag_x,
                                 int drag_y,
                                 GtkTreePath** path,
                                 GtkTreeViewDropPosition* pos);
    GdkPixmap* create_row_drag_icon (GtkTreePath* path);

    /* Interactive search */
    void    set_enable_search (bool enable_search);
    bool    get_enable_search ();
    int     get_search_column ();
    void    set_search_column (int column);
    GtkTreeViewSearchEqualFunc get_search_equal_func ();
    void    set_search_equal_func (GtkTreeViewSearchEqualFunc search_equal_func,
                                   gpointer search_user_data,
                                   GDestroyNotify search_destroy);

    GtkEntry* get_search_entry ();
    void    set_search_entry (GtkEntry* entry);
    GtkTreeViewSearchPositionFunc get_search_position_func ();
    void    set_search_position_func (GtkTreeViewSearchPositionFunc func,
                                      gpointer data,
                                      GDestroyNotify destroy);

    /* Convert between the different coordinate systems */
    void    convert_widget_to_tree_coords (int wx,
                                           int wy,
                                           int* tx,
                                           int* ty);
    void    convert_tree_to_widget_coords (int tx,
                                           int ty,
                                           int* wx,
                                           int* wy);
    void    convert_widget_to_bin_window_coords (int wx,
                                                 int wy,
                                                 int* bx,
                                                 int* by);
    void    convert_bin_window_to_widget_coords (int bx,
                                                 int by,
                                                 int* wx,
                                                 int* wy);
    void    convert_tree_to_bin_window_coords (int tx,
                                               int ty,
                                               int* bx,
                                               int* by);
    void    convert_bin_window_to_tree_coords (int bx,
                                               int by,
                                               int* tx,
                                               int* ty);

    void    set_fixed_height_mode (bool enable);
    bool    get_fixed_height_mode ();
    void    set_hover_selection (bool hover);
    bool    get_hover_selection ();
    void    set_hover_expand (bool expand);
    bool    get_hover_expand ();
    void    set_rubber_banding (bool enable);
    bool    get_rubber_banding ();

    bool    is_rubber_banding_active ();

    GtkTreeViewRowSeparatorFunc get_row_separator_func ();
    void    set_row_separator_func (GtkTreeViewRowSeparatorFunc func,
                                    gpointer data,
                                    GDestroyNotify destroy);

    GtkTreeViewGridLines get_grid_lines ();
    void    set_grid_lines (GtkTreeViewGridLines grid_lines);
    bool    get_enable_tree_lines ();
    void    set_enable_tree_lines (bool enabled);
    void    set_show_expanders (bool enabled);
    bool    get_show_expanders ();
    void    set_level_indentation (int indentation);
    int     get_level_indentation ();

    /* Convenience functions for setting tooltips */
    void    set_tooltip_row (GtkTooltip* tooltip,
                             GtkTreePath* path);
    void    set_tooltip_cell (GtkTooltip* tooltip,
                              GtkTreePath* path,
                              gtk::TreeViewColumn& column,
                              gtk::CellRenderer& cell);
    bool    get_tooltip_context (int* x,
                                 int* y,
                                 bool keyboard_tip,
                                 GtkTreeModel** model,
                                 GtkTreePath** path,
                                 GtkTreeIter* iter);
    void    set_tooltip_column (int column);
    int     get_tooltip_column ();
};

template<>
class ::moo::gobj_ref<GtkTreeViewColumn> : public virtual ::moo::gobj_ref_parent<GtkTreeViewColumn>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkTreeViewColumn);

    static gtk::TreeViewColumnPtr create ();
    static gtk::TreeViewColumnPtr create (const char* title,
                                          gtk::CellRendererPtr cell,
                                          ...) G_GNUC_NULL_TERMINATED;

    void    pack_start (gtk::CellRenderer& cell,
                        bool expand);
    void    pack_end (gtk::CellRenderer& cell,
                      bool expand);
    void    clear ();
    std::vector<gtk::CellRendererPtr> get_cell_renderers ();
    void    add_attribute (gtk::CellRenderer& cell,
                           const char* attribute,
                           int column);

    void set_attributes (gtk::CellRenderer& cell_renderer, const char* prop, int column);

    template<typename... Args>
    void set_attributes (gtk::CellRenderer& cell_renderer, const char* prop, int column, Args&&... args)
    {
        set_attributes (cell_renderer, prop, column);
        set_attributes (cell_renderer, std::forward<Args> (args)...);
    }

    // void TFunc(gtk::TreeViewColumn, gtk::CellRenderer, gtk::TreeModel, GtkTreeIter*)
    template<typename TFunc>
    void set_cell_data_func (gtk::CellRenderer& cell_renderer, TFunc func)
    {
        TreeCellDataFunc<TFunc> *data = new TreeCellDataFunc<TFunc>{ std::move (func) };
        gtk_tree_view_column_set_cell_data_func (gobj (), cell_renderer.gobj (),
                                                 TreeCellDataFunc<TFunc>::cell_data_func, data,
                                                 TreeCellDataFunc<TFunc>::destroy);
    }

    void    clear_attributes (gtk::CellRenderer& cell_renderer);
    void    set_spacing (int spacing);
    int     get_spacing ();
    void    set_visible (bool visible);
    bool    get_visible ();
    void    set_resizable (bool resizable);
    bool    get_resizable ();
    void    set_sizing (GtkTreeViewColumnSizing type);
    GtkTreeViewColumnSizing get_sizing ();
    int     get_width ();
    int     get_fixed_width ();
    void    set_fixed_width (int fixed_width);
    void    set_min_width (int min_width);
    int     get_min_width ();
    void    set_max_width (int max_width);
    int     get_max_width ();
    void    clicked ();

    /* Options for manipulating the column headers
     */
    void    set_title (const char* title);
    gstr    get_title ();
    void    set_expand (bool expand);
    bool    get_expand ();
    void    set_clickable (bool clickable);
    bool    get_clickable ();
    void    set_widget (gtk::WidgetPtr widget);
    gtk::WidgetPtr get_widget ();
    void    set_alignment (float xalign);
    float   get_alignment ();
    void    set_reorderable (bool reorderable);
    bool    get_reorderable ();

    /* You probably only want to use gtk_tree_view_column_set_sort_column_id.  The
     * other sorting functions exist primarily to let others do their own custom sorting.
     */
    void    set_sort_column_id (int sort_column_id);
    int     get_sort_column_id ();
    void    set_sort_indicator (bool setting);
    bool    get_sort_indicator ();
    void    set_sort_order (GtkSortType order);
    GtkSortType get_sort_order ();

    /* These functions are meant primarily for interaction between the GtkTreeView and the column.
     */
    void    cell_set_cell_data (gtk::TreeModel& tree_model,
                                GtkTreeIter* iter,
                                bool is_expander,
                                bool is_expanded);
    void    cell_get_size (const GdkRectangle* cell_area,
                           int* x_offset,
                           int* y_offset,
                           int* width,
                           int* height);
    bool    cell_is_visible ();
    void    focus_cell (gtk::CellRenderer& cell);
    bool    cell_get_position (gtk::CellRenderer& cell_renderer,
                               int* start_pos,
                               int* width);
    void    queue_resize ();
    gtk::TreeViewPtr get_tree_view ();
};

template<>
class ::moo::gobj_ref<GtkCellRenderer> : public virtual ::moo::gobj_ref_parent<GtkCellRenderer>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkCellRenderer);
};

template<>
class ::moo::gobj_ref<GtkCellRendererText> : public virtual ::moo::gobj_ref_parent<GtkCellRendererText>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkCellRendererText);

    static gtk::CellRendererTextPtr create  ();

    void set_fixed_height_from_font (int number_of_rows);
};

template<>
class ::moo::gobj_ref<GtkCellRendererToggle> : public virtual ::moo::gobj_ref_parent<GtkCellRendererToggle>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkCellRendererToggle);

    static gtk::CellRendererTogglePtr create ();

    bool get_radio ();
    void set_radio (bool radio);
    bool get_active ();
    void set_active (bool active);
};

template<>
class ::moo::gobj_ref<GtkCellRendererPixbuf> : public virtual ::moo::gobj_ref_parent<GtkCellRendererPixbuf>
{
public:
    MOO_DEFINE_GOBJREF_METHODS (GtkCellRendererPixbuf);

    static gtk::CellRendererPixbufPtr create ();
};


template<typename TFunc>
inline void
TreeCellDataFunc<TFunc>::cell_data_func (GtkTreeViewColumn *tree_column,
                                         GtkCellRenderer   *cell,
                                         GtkTreeModel      *tree_model,
                                         GtkTreeIter       *iter,
                                         gpointer           p)
{
    g_return_if_fail (GTK_IS_TREE_VIEW_COLUMN (tree_column));
    g_return_if_fail (GTK_IS_CELL_RENDERER (cell));
    g_return_if_fail (GTK_IS_TREE_MODEL (tree_model));
    g_return_if_fail (iter != nullptr);
    g_return_if_fail (p != nullptr);
    TreeCellDataFunc& data = *reinterpret_cast<TreeCellDataFunc*>(p);
    data.m_func (gtk::TreeViewColumn (*tree_column), gtk::CellRenderer (*cell), gtk::TreeModel (*tree_model), iter);
}


template<typename TFunc>
inline int
::moo::gobj_ref<GtkTreeView>::insert_column_with_data_func (int position,
                                                            const char* title,
                                                            gtk::CellRenderer& cell,
                                                            TFunc func)
{
    TreeCellDataFunc<TFunc> *data = new TreeCellDataFunc<TFunc>{ std::move (func) };
    gtk_tree_view_insert_column_with_data_func (gobj (), position, title, cell.gobj (),
                                                TreeCellDataFunc<TFunc>::cell_data_func, data,
                                                TreeCellDataFunc<TFunc>::destroy);
}


MOO_DEFINE_FLAGS(GdkEventMask);
MOO_DEFINE_FLAGS(GdkModifierType);
MOO_DEFINE_FLAGS(GtkCellRendererState);
MOO_DEFINE_FLAGS(GtkAttachOptions);
MOO_DEFINE_FLAGS(GdkDragAction);

#endif // __cplusplus
