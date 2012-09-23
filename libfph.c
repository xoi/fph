// Adobe Flash Player unfocus hack
// for Firefox 4 with plugin-container

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <assert.h>

static gboolean event_callback(GtkWidget *widget, GdkEvent *event, gpointer callback_data);
static typeof(event_callback) *orig_event_callback = NULL;
static gint key_snooper(GtkWidget *grab_widget, GdkEventKey *event, gpointer func_data);
static Window get_parent_xwindow(GtkWidget *widget);
static void translate_key_event(GdkEventKey *gdk_event_key, XKeyEvent *xev);
static void send_key_event(XKeyEvent *xev);
static gboolean focus_enabled = FALSE;

static gboolean event_callback(GtkWidget *widget, GdkEvent *event, gpointer callback_data)
{
  switch (event->type) {
    case GDK_BUTTON_PRESS:
      if (event->button.button == 2 && !focus_enabled) {
	// middle click when focus is disabled
	focus_enabled = TRUE;
	return TRUE;
      }
      break;
    default:
      break;
  }
  return orig_event_callback(widget, event, callback_data);
}

static gint key_snooper(GtkWidget *grab_widget, GdkEventKey *event, gpointer func_data)
{
  if (focus_enabled) {
    switch (event->type) {
      case GDK_KEY_PRESS:
	if (event->state == GDK_CONTROL_MASK && event->keyval == GDK_bracketleft) {
	  // press CTRL-[
	  focus_enabled = FALSE;
	  g_signal_emit_by_name(GTK_WINDOW(grab_widget), "move-focus", GTK_DIR_TAB_FORWARD);
	  return TRUE;
	}
	break;
      default:
	break;
    }
    return FALSE;
  }
  GtkWidget *window = gtk_widget_get_toplevel(grab_widget);
  assert(GTK_IS_WINDOW(window));
  XKeyEvent xev;
  translate_key_event(event, &xev);
  xev.window = get_parent_xwindow(window);
  send_key_event(&xev);
  return TRUE;
}

// wrapping g_signal_connect_data
gulong	 fph_g_signal_connect_		      (gpointer		  instance,
					       const gchar	 *detailed_signal,
					       GCallback	  c_handler,
					       gpointer		  data,
					       GClosureNotify	  destroy_data,
					       GConnectFlags	  connect_flags)
{
  if (strcmp(detailed_signal, "event") == 0) {
    assert(((GtkWidget*)instance)->parent == gtk_widget_get_toplevel(instance));	// in plugin-container
    if (orig_event_callback == NULL) {
      orig_event_callback = (typeof(event_callback)*)c_handler;
      gtk_key_snooper_install(key_snooper, NULL);
    }
    if (orig_event_callback == (typeof(event_callback)*)c_handler) {
      return g_signal_connect_data(instance, detailed_signal, G_CALLBACK(event_callback), data, destroy_data, connect_flags);
    }
    assert(FALSE);
  }
  return g_signal_connect_data(instance, detailed_signal, c_handler, data, destroy_data, connect_flags);
}

// wrapping gtk_widget_grab_focus
void	   fph_gtk_widget_grab_f	  (GtkWidget	       *widget)
{
  if (focus_enabled) {
    gtk_widget_grab_focus(widget);
  }
}

Window get_parent_xwindow(GtkWidget *widget)
{
  GdkWindow *window = widget->window;
  Display *display = GDK_WINDOW_XDISPLAY(window);
  Window w = GDK_WINDOW_XWINDOW(window);
  Window root_return;
  Window parent_return;
  Window *children_return;
  unsigned int nchildren_return;
  Status s = XQueryTree(display, w, &root_return, &parent_return, &children_return, &nchildren_return);
  assert(s != 0);
  if (children_return) {
    XFree(children_return);
  }
  return parent_return;
}

// translate X key event
void translate_key_event(GdkEventKey *gdk_event_key, XKeyEvent *xev)
{
  GdkWindow *window = gdk_event_key->window;
  GdkScreen *screen = gdk_drawable_get_screen(window);

  xev->type = (gdk_event_key->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
  xev->display = GDK_WINDOW_XDISPLAY(window);
  xev->window = GDK_WINDOW_XWINDOW(window);
  xev->root = GDK_WINDOW_XWINDOW(gdk_screen_get_root_window(screen));
  xev->subwindow = None;
  xev->time = gdk_event_key->time;
  xev->x = 0;
  xev->y = 0;
  xev->x_root = 0;
  xev->y_root = 0;
  xev->state = gdk_event_key->state;
  xev->keycode = gdk_event_key->hardware_keycode;
  xev->same_screen = True;
}

// send X key event
void send_key_event(XKeyEvent *xev)
{
  XSendEvent(xev->display, xev->window, False, NoEventMask, (XEvent*)xev);
}
