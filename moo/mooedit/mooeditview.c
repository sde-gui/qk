/**
 * class:MooEditView: (parent MooTextView)
 **/

#define MOOEDIT_COMPILATION
#include "mooedit/mooeditview-priv.h"
#include "mooedit/mooedit-impl.h"
#include "mooedit/mooeditor-impl.h"
#include "mooedit/mooeditbookmark.h"
#include "mooedit/mooeditprefs.h"
#include "mooutils/mooutils.h"
#include "mooutils/moocompat.h"
#include "mooedit/mooeditprogress-gxml.h"

MOO_DEFINE_OBJECT_ARRAY (MooEditView, moo_edit_view)

static void     moo_edit_view_finalize              (GObject            *object);
static void     moo_edit_view_dispose               (GObject            *object);

static gboolean moo_edit_view_focus_in              (GtkWidget          *widget,
                                                     GdkEventFocus      *event);
static gboolean moo_edit_view_focus_out             (GtkWidget          *widget,
                                                     GdkEventFocus      *event);
static gboolean moo_edit_view_popup_menu            (GtkWidget          *widget);
static gboolean moo_edit_view_drag_motion           (GtkWidget          *widget,
                                                     GdkDragContext     *context,
                                                     gint                x,
                                                     gint                y,
                                                     guint               time);
static gboolean moo_edit_view_drag_drop             (GtkWidget          *widget,
                                                     GdkDragContext     *context,
                                                     gint                x,
                                                     gint                y,
                                                     guint               time);
static void     moo_edit_view_apply_style_scheme    (MooTextView        *view,
                                                     MooTextStyleScheme *scheme);
static gboolean moo_edit_view_line_mark_clicked     (MooTextView        *view,
                                                     int                 line);

G_DEFINE_TYPE (MooEditView, moo_edit_view, MOO_TYPE_TEXT_VIEW)

static void
moo_edit_view_class_init (MooEditViewClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    MooTextViewClass *textview_class = MOO_TEXT_VIEW_CLASS (klass);

    gobject_class->finalize = moo_edit_view_finalize;
    gobject_class->dispose = moo_edit_view_dispose;

    widget_class->popup_menu = moo_edit_view_popup_menu;
    widget_class->drag_motion = moo_edit_view_drag_motion;
    widget_class->drag_drop = moo_edit_view_drag_drop;
    widget_class->focus_in_event = moo_edit_view_focus_in;
    widget_class->focus_out_event = moo_edit_view_focus_out;

    textview_class->line_mark_clicked = moo_edit_view_line_mark_clicked;
    textview_class->apply_style_scheme = moo_edit_view_apply_style_scheme;

    g_type_class_add_private (klass, sizeof (MooEditViewPrivate));
}


static void
moo_edit_view_init (MooEditView *view)
{
    view->priv = G_TYPE_INSTANCE_GET_PRIVATE (view, MOO_TYPE_EDIT_VIEW, MooEditViewPrivate);
}


static void
moo_edit_view_finalize (GObject *object)
{
    G_GNUC_UNUSED MooEditView *view = MOO_EDIT_VIEW (object);

    moo_assert (view->priv->state == MOO_EDIT_STATE_NORMAL);

    G_OBJECT_CLASS (moo_edit_view_parent_class)->finalize (object);
}


static void
moo_edit_view_dispose (GObject *object)
{
    MooEditView *view = MOO_EDIT_VIEW (object);

    moo_assert (!view->priv->progress);

    view->priv->doc = NULL;

    view->priv->progress = NULL;
    view->priv->progressbar = NULL;

    if (view->priv->progress_timeout)
    {
        moo_assert_not_reached ();
        g_source_remove (view->priv->progress_timeout);
        view->priv->progress_timeout = 0;
    }

    g_free (view->priv->progress_text);
    view->priv->progress_text = NULL;

    G_OBJECT_CLASS (moo_edit_view_parent_class)->dispose (object);
}


void
_moo_edit_view_set_doc (MooEditView *view,
                        MooEdit     *doc)
{
    moo_assert (MOO_IS_EDIT_VIEW (view));
    moo_assert (MOO_IS_EDIT (doc));
    moo_assert (!view->priv->doc);
    view->priv->doc = doc;
    view->priv->editor = moo_edit_get_editor (doc);
}


static gboolean
moo_edit_view_line_mark_clicked (MooTextView *view,
                                 int          line)
{
    moo_edit_toggle_bookmark (moo_edit_view_get_doc (MOO_EDIT_VIEW (view)), line);
    return TRUE;
}

static void
moo_edit_view_apply_style_scheme (MooTextView        *view,
                                  MooTextStyleScheme *scheme)
{
    MOO_TEXT_VIEW_CLASS (moo_edit_view_parent_class)->apply_style_scheme (view, scheme);
    _moo_edit_update_bookmarks_style (moo_edit_view_get_doc (MOO_EDIT_VIEW (view)));
}

static gboolean
moo_edit_view_focus_in (GtkWidget     *widget,
                        GdkEventFocus *event)
{
    gboolean retval = FALSE;
    MooEditView *view = MOO_EDIT_VIEW (widget);
    MooEdit *doc = moo_edit_view_get_doc (view);

    _moo_editor_set_focused_doc (view->priv->editor, doc);

    if (GTK_WIDGET_CLASS (moo_edit_view_parent_class)->focus_in_event)
        retval = GTK_WIDGET_CLASS (moo_edit_view_parent_class)->focus_in_event (widget, event);

    return retval;
}

static gboolean
moo_edit_view_focus_out (GtkWidget     *widget,
                         GdkEventFocus *event)
{
    gboolean retval = FALSE;
    MooEditView *view = MOO_EDIT_VIEW (widget);
    MooEdit *doc = moo_edit_view_get_doc (view);

    _moo_editor_unset_focused_doc (view->priv->editor, doc);

    if (GTK_WIDGET_CLASS (moo_edit_view_parent_class)->focus_out_event)
        retval = GTK_WIDGET_CLASS (moo_edit_view_parent_class)->focus_out_event (widget, event);

    return retval;
}


MooEdit *
moo_edit_view_get_doc (MooEditView *view)
{
    g_return_val_if_fail (MOO_IS_EDIT_VIEW (view), NULL);
    return view->priv->doc;
}

MooEditor *
moo_edit_view_get_editor (MooEditView *view)
{
    g_return_val_if_fail (MOO_IS_EDIT_VIEW (view), NULL);
    return view->priv->editor;
}

MooEditWindow *
moo_edit_view_get_window (MooEditView *view)
{
    GtkWidget *toplevel;

    g_return_val_if_fail (MOO_IS_EDIT_VIEW (view), NULL);

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (view));

    if (MOO_IS_EDIT_WINDOW (toplevel))
        return MOO_EDIT_WINDOW (toplevel);
    else
        return NULL;
}


void
_moo_edit_view_apply_config (MooEditView *view)
{
    GtkWrapMode wrap_mode;
    gboolean line_numbers;
    guint tab_width;
    char *word_chars;

    moo_return_if_fail (MOO_IS_EDIT_VIEW (view));
    moo_return_if_fail (view->priv->doc && view->priv->doc->config);

    moo_edit_config_get (view->priv->doc->config,
                         "wrap-mode", &wrap_mode,
                         "show-line-numbers", &line_numbers,
                         "tab-width", &tab_width,
                         "word-chars", &word_chars,
                         (char*) 0);

    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), wrap_mode);
    moo_text_view_set_show_line_numbers (MOO_TEXT_VIEW (view), line_numbers);
    moo_text_view_set_tab_width (MOO_TEXT_VIEW (view), tab_width);
    moo_text_view_set_word_chars (MOO_TEXT_VIEW (view), word_chars);

    g_free (word_chars);
}


static gboolean
find_uri_atom (GdkDragContext *context)
{
    GList *targets;
    GdkAtom atom;

    atom = moo_atom_uri_list ();
    targets = context->targets;

    while (targets)
    {
        if (targets->data == GUINT_TO_POINTER (atom))
            return TRUE;
        targets = targets->next;
    }

    return FALSE;
}

static gboolean
moo_edit_view_drag_motion (GtkWidget      *widget,
                           GdkDragContext *context,
                           gint            x,
                           gint            y,
                           guint           time)
{
    if (find_uri_atom (context))
        return FALSE;

    return GTK_WIDGET_CLASS (moo_edit_view_parent_class)->drag_motion (widget, context, x, y, time);
}

static gboolean
moo_edit_view_drag_drop (GtkWidget      *widget,
                         GdkDragContext *context,
                         gint            x,
                         gint            y,
                         guint           time)
{
    if (find_uri_atom (context))
        return FALSE;

    return GTK_WIDGET_CLASS (moo_edit_view_parent_class)->drag_drop (widget, context, x, y, time);
}


/*****************************************************************************/
/* popup menu
 */

/* gtktextview.c */
static void
popup_position_func (GtkMenu   *menu,
                     gint      *x,
                     gint      *y,
                     gboolean  *push_in,
                     gpointer   user_data)
{
    GtkTextView *text_view;
    GtkWidget *widget;
    GdkRectangle cursor_rect;
    GdkRectangle onscreen_rect;
    gint root_x, root_y;
    GtkTextIter iter;
    GtkRequisition req;
    GdkScreen *screen;
    gint monitor_num;
    GdkRectangle monitor;

    text_view = GTK_TEXT_VIEW (user_data);
    widget = GTK_WIDGET (text_view);

    g_return_if_fail (GTK_WIDGET_REALIZED (text_view));

    screen = gtk_widget_get_screen (widget);

    gdk_window_get_origin (widget->window, &root_x, &root_y);

    gtk_text_buffer_get_iter_at_mark (gtk_text_view_get_buffer (text_view),
                                      &iter,
                                      gtk_text_buffer_get_insert (gtk_text_view_get_buffer (text_view)));

    gtk_text_view_get_iter_location (text_view,
                                     &iter,
                                     &cursor_rect);

    gtk_text_view_get_visible_rect (text_view, &onscreen_rect);

    gtk_widget_size_request (GTK_WIDGET (menu), &req);

    /* can't use rectangle_intersect since cursor rect can have 0 width */
    if (cursor_rect.x >= onscreen_rect.x &&
        cursor_rect.x < onscreen_rect.x + onscreen_rect.width &&
        cursor_rect.y >= onscreen_rect.y &&
        cursor_rect.y < onscreen_rect.y + onscreen_rect.height)
    {
        gtk_text_view_buffer_to_window_coords (text_view,
                                               GTK_TEXT_WINDOW_WIDGET,
                                               cursor_rect.x, cursor_rect.y,
                                               &cursor_rect.x, &cursor_rect.y);

        *x = root_x + cursor_rect.x + cursor_rect.width;
        *y = root_y + cursor_rect.y + cursor_rect.height;
    }
    else
    {
        /* Just center the menu, since cursor is offscreen. */
        *x = root_x + (widget->allocation.width / 2 - req.width / 2);
        *y = root_y + (widget->allocation.height / 2 - req.height / 2);
    }

    /* Ensure sanity */
    *x = CLAMP (*x, root_x, (root_x + widget->allocation.width));
    *y = CLAMP (*y, root_y, (root_y + widget->allocation.height));

    monitor_num = gdk_screen_get_monitor_at_point (screen, *x, *y);
    gtk_menu_set_monitor (menu, monitor_num);
    gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

    *x = CLAMP (*x, monitor.x, monitor.x + MAX (0, monitor.width - req.width));
    *y = CLAMP (*y, monitor.y, monitor.y + MAX (0, monitor.height - req.height));

    *push_in = FALSE;
}

void
_moo_edit_view_do_popup (MooEditView    *view,
                         GdkEventButton *event)
{
    MooUiXml *xml;
    MooEditWindow *window;
    GtkMenu *menu;
    MooActionCollection *actions;

    window = moo_edit_view_get_window (view);
    xml = moo_editor_get_doc_ui_xml (view->priv->editor);
    g_return_if_fail (xml != NULL);

    actions = _moo_edit_get_actions (view->priv->doc);
    menu = (GtkMenu*) moo_ui_xml_create_widget (xml, MOO_UI_MENU, "Editor/Popup", actions,
                                                window ? MOO_WINDOW(window)->accel_group : NULL);
    g_return_if_fail (menu != NULL);
    g_object_ref_sink (menu);

    _moo_edit_check_actions (view->priv->doc);

    if (event)
    {
        gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
                        event->button, event->time);
    }
    else
    {
        gtk_menu_popup (menu, NULL, NULL,
                        popup_position_func, view,
                        0, gtk_get_current_event_time ());
        gtk_menu_shell_select_first (GTK_MENU_SHELL (menu), FALSE);
    }

    g_object_unref (menu);
}

static gboolean
moo_edit_view_popup_menu (GtkWidget *widget)
{
    _moo_edit_view_do_popup (MOO_EDIT_VIEW (widget), NULL);
    return TRUE;
}


/*****************************************************************************/
/* progress dialogs and stuff
 */

MooEditState
_moo_edit_view_get_state (MooEditView *view)
{
    g_return_val_if_fail (MOO_IS_EDIT_VIEW (view), MOO_EDIT_STATE_NORMAL);
    return view->priv->state;
}


static void
position_progress (MooEditView *view)
{
    GtkAllocation *allocation;
    int x, y;

    g_return_if_fail (MOO_IS_EDIT_VIEW (view));
    g_return_if_fail (GTK_IS_WIDGET (view->priv->progress));

    if (!GTK_WIDGET_REALIZED (view))
        return;

    allocation = &GTK_WIDGET(view)->allocation;

    x = allocation->width/2 - PROGRESS_WIDTH/2;
    y = allocation->height/2 - PROGRESS_HEIGHT/2;
    gtk_text_view_move_child (GTK_TEXT_VIEW (view),
                              view->priv->progress,
                              x, y);
}

static void
update_progress (MooEditView *view)
{
    g_return_if_fail (MOO_IS_EDIT_VIEW (view));
    g_return_if_fail (view->priv->progress_text != NULL);
    g_return_if_fail (view->priv->state != MOO_EDIT_STATE_NORMAL);

    if (view->priv->progressbar)
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (view->priv->progressbar),
                                   view->priv->progress_text);
}

void
_moo_edit_view_set_progress_text (MooEditView *view,
                                  const char  *text)
{
    moo_return_if_fail (MOO_IS_EDIT_VIEW (view));
    g_free (view->priv->progress_text);
    view->priv->progress_text = g_strdup (text);
    update_progress (view);
}


static gboolean
pulse_progress (MooEditView *view)
{
    g_return_val_if_fail (MOO_IS_EDIT_VIEW (view), FALSE);
    g_return_val_if_fail (GTK_IS_WIDGET (view->priv->progressbar), FALSE);
    gtk_progress_bar_pulse (GTK_PROGRESS_BAR (view->priv->progressbar));
    update_progress (view);
    return TRUE;
}

static void
progress_cancel_clicked (MooEditView *view)
{
    g_return_if_fail (MOO_IS_EDIT_VIEW (view));
    if (view->priv->state && view->priv->cancel_op)
        view->priv->cancel_op (view->priv->cancel_data);
}

static gboolean
show_progress (MooEditView *view)
{
    ProgressDialogXml *xml;

    view->priv->progress_timeout = 0;

    g_return_val_if_fail (!view->priv->progress, FALSE);

    xml = progress_dialog_xml_new ();

    view->priv->progress = GTK_WIDGET (xml->ProgressDialog);
    view->priv->progressbar = GTK_WIDGET (xml->progressbar);
    g_assert (GTK_IS_WIDGET (view->priv->progressbar));

    g_signal_connect_swapped (xml->cancel, "clicked",
                              G_CALLBACK (progress_cancel_clicked),
                              view);

    gtk_text_view_add_child_in_window (GTK_TEXT_VIEW (view),
                                       view->priv->progress,
                                       GTK_TEXT_WINDOW_WIDGET,
                                       0, 0);
    position_progress (view);
    update_progress (view);

    view->priv->progress_timeout =
            _moo_timeout_add (PROGRESS_TIMEOUT,
                              (GSourceFunc) pulse_progress,
                              view);

    return FALSE;
}

void
_moo_edit_view_set_state (MooEditView    *view,
                          MooEditState    state,
                          const char     *text,
                          GDestroyNotify  cancel,
                          gpointer        data)
{
    moo_return_if_fail (MOO_IS_EDIT_VIEW (view));
    moo_return_if_fail (state == MOO_EDIT_STATE_NORMAL ||
                        view->priv->state == MOO_EDIT_STATE_NORMAL);

    view->priv->cancel_op = cancel;
    view->priv->cancel_data = data;

    if (state == view->priv->state)
        return;

    view->priv->state = state;
    gtk_text_view_set_editable (GTK_TEXT_VIEW (view), !state);

    if (!state)
    {
        if (view->priv->progress)
        {
            GtkWidget *tmp = view->priv->progress;
            view->priv->progress = NULL;
            view->priv->progressbar = NULL;
            gtk_widget_destroy (tmp);
        }

        g_free (view->priv->progress_text);
        view->priv->progress_text = NULL;

        if (view->priv->progress_timeout)
            g_source_remove (view->priv->progress_timeout);
        view->priv->progress_timeout = 0;
    }
    else
    {
        if (!view->priv->progress_timeout)
            view->priv->progress_timeout =
                    _moo_timeout_add (PROGRESS_TIMEOUT,
                                      (GSourceFunc) show_progress,
                                      view);
        view->priv->progress_text = g_strdup (text);
    }
}


void
_moo_edit_view_ui_set_line_wrap (MooEditView *view,
                                 gboolean     enabled)
{
    GtkWrapMode mode;
    gboolean old_enabled;

    moo_return_if_fail (MOO_IS_EDIT_VIEW (view));
    moo_return_if_fail (view->priv->doc && view->priv->doc->config);

    g_object_get (view, "wrap-mode", &mode, NULL);

    enabled = enabled != 0;
    old_enabled = mode != GTK_WRAP_NONE;

    if (enabled == old_enabled)
        return;

    if (!enabled)
        mode = GTK_WRAP_NONE;
    else if (moo_prefs_get_bool (moo_edit_setting (MOO_EDIT_PREFS_WRAP_WORDS)))
        mode = GTK_WRAP_WORD;
    else
        mode = GTK_WRAP_CHAR;

    moo_edit_config_set (view->priv->doc->config,
                         MOO_EDIT_CONFIG_SOURCE_USER,
                         "wrap-mode", mode, NULL);
}

void
_moo_edit_view_ui_set_show_line_numbers (MooEditView *view,
                                         gboolean     show)
{
    gboolean old_show;

    g_return_if_fail (MOO_IS_EDIT_VIEW (view));
    moo_return_if_fail (view->priv->doc && view->priv->doc->config);

    g_object_get (view, "show-line-numbers", &old_show, NULL);

    if (!old_show == !show)
        return;

    moo_edit_config_set (view->priv->doc->config,
                         MOO_EDIT_CONFIG_SOURCE_USER,
                         "show-line-numbers", show,
                         (char*) NULL);
}
