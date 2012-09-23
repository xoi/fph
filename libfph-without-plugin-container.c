// Adobe Flash Player unfocus hack
// for Firefox 4 without plugin-container

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <assert.h>

static gboolean event_callback(GtkWidget *widget, GdkEvent *event, gpointer callback_data);
static typeof(event_callback) *orig_event_callback = NULL;
static gboolean focus_enabled = FALSE;

static gboolean event_callback(GtkWidget *widget, GdkEvent *event, gpointer callback_data)
{
  switch (event->type) {
    case GDK_KEY_PRESS:
      if (event->key.state == GDK_CONTROL_MASK && event->key.keyval == GDK_bracketleft) {
	// press CTRL-[
	focus_enabled = FALSE;
	gtk_widget_grab_focus(widget->parent->parent->parent);	// not work in plugin-container/nspluginwrapper
	return TRUE;
      }
      break;
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

// wrapping g_signal_connect_data
gulong	 fph_g_signal_connect_		      (gpointer		  instance,
					       const gchar	 *detailed_signal,
					       GCallback	  c_handler,
					       gpointer		  data,
					       GClosureNotify	  destroy_data,
					       GConnectFlags	  connect_flags)
{
  if (strcmp(detailed_signal, "event") == 0) {
    if (orig_event_callback == NULL) {
      orig_event_callback = (typeof(event_callback)*)c_handler;
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
