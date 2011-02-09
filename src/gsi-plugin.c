/*
 * gsi-plugin.c - Type here a short description of your plugin
 *
 * Copyright (C) 2009 - Jesús Barbero Rodríguez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gsi-plugin.h"
#include "gsi-indenters-manager.h"
#include "gsi-indenter-c.h"
#include "gsi-indenter-python.h"


#include <glib/gi18n-lib.h>
#include <gedit/gedit-debug.h>
#include <gedit/gedit-window.h>
#include <gedit/gedit-window-activatable.h>
#include <gedit/gedit-document.h>
#include <gdk/gdkkeysyms.h>
#include <gtksourceview/gtksourceview.h>


#define WINDOW_DATA_KEY	"GsiPluginWindowData"
#define VIEW_KEY	"GsiPluginView"
#define ENABLED_KEY     "GsiPluginEnabled"

#define GSI_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_GSI_PLUGIN, GsiPluginPrivate))

#define get_window_data(window) ((WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY)))
#define get_view(buffer) ((GtkTextView *) (g_object_get_data (G_OBJECT (buffer), VIEW_KEY)))
#define is_enabled(view) ((g_object_get_data (G_OBJECT (view), ENABLED_KEY)) != NULL)

struct _GsiPluginPrivate
{
	GeditWindow *window;
	GsiIndentersManager *manager;
	GdkCursor *busy_cursor;
};

enum
{
	PROP_0,
	PROP_WINDOW
};

typedef struct{
	GeditWindow	*window;
	GsiPlugin	*plugin;
} WindowData;

static void gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (GsiPlugin,
				gsi_plugin,
				PEAS_TYPE_EXTENSION_BASE,
				0,
				G_IMPLEMENT_INTERFACE_DYNAMIC (GEDIT_TYPE_WINDOW_ACTIVATABLE,
							       gedit_window_activatable_iface_init))

static void
gsi_plugin_init (GsiPlugin *plugin)
{
	GsiIndenter *indenter;
	
	plugin->priv = GSI_PLUGIN_GET_PRIVATE (plugin);

	gedit_debug_message (DEBUG_PLUGINS,
			     "GsiPlugin initializing");

	plugin->priv->manager = gsi_indenters_manager_new ();
	plugin->priv->busy_cursor = gdk_cursor_new (GDK_WATCH);
	
	indenter = gsi_indenter_c_new ();
	gsi_indenters_manager_register (plugin->priv->manager,
					"c",
					indenter);
	g_object_unref (indenter);

	indenter = gsi_indenter_python_new ();
	gsi_indenters_manager_register (plugin->priv->manager,
					"python",
					indenter);
	g_object_unref (indenter);
}

static void
gsi_plugin_dispose (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS,
			     "GsiPlugin finalizing");

	GsiPlugin *self = GSI_PLUGIN (object);
	
	g_object_unref (self->priv->manager);
	
	gdk_cursor_unref (self->priv->busy_cursor);
	
	if (self->priv->window != NULL)
	{
		g_object_unref (self->priv->window);
		self->priv->window = NULL;
	}
	
	G_OBJECT_CLASS (gsi_plugin_parent_class)->dispose (object);
}

static void
gsi_plugin_class_finalize (GsiPluginClass *klass)
{
}

static void
gsi_plugin_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
	GsiPlugin *plugin = GSI_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			plugin->priv->window = GEDIT_WINDOW (g_value_dup_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gsi_plugin_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
	GsiPlugin *plugin = GSI_PLUGIN (object);

	switch (prop_id)
	{
		case PROP_WINDOW:
			g_value_set_object (value, plugin->priv->window);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
window_data_free (WindowData *data)
{
        g_return_if_fail (data != NULL);

        g_slice_free (WindowData, data);
}

static void
insert_text_cb (GtkTextBuffer *buffer,
		GtkTextIter   *location,
		gchar         *text,
		gint           len,
		GsiPlugin *self)
{
	GsiIndenter *indenter;
	GtkTextView *view;
	GtkSourceLanguage *language;
	const gchar *lang_id = NULL;
	const gchar *relocators = NULL;
	gchar c;
	gboolean found = FALSE;
	GtkTextMark * mark;
	gint insert_iterator_pos;

	/*Prevent insertion when gedit insert the text loading the document*/
	if (len > 2)
		return;
	
	view = get_view (buffer);
		
	language = gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (buffer));
	
	if (language)
	{
		lang_id = gtk_source_language_get_id (language);
	}
	indenter = gsi_indenters_manager_get_indenter (self->priv->manager, lang_id);
	g_assert (indenter != NULL);
	
	/* remember where the insert pos was so that we can revalidate
	 * the iterator that was passed in 
	 */
		
	insert_iterator_pos = gtk_text_iter_get_offset (location);
	
	c = text[len-1];
	if (c == '\n')
	{
		gtk_text_buffer_begin_user_action (buffer);
		gsi_indenter_indent_line (indenter, view, location);
		gtk_text_buffer_end_user_action (buffer);
	}
	else
	{
		relocators = gsi_indenter_get_relocators (indenter, view);
	
		if (!relocators)
			return;
	
		while (*relocators != '\0')
		{
			if (c == *relocators)
			{
				found = TRUE;
				break;
			}
			relocators++;
		}
		
		if (!found)
			return;
		
		gtk_text_buffer_begin_user_action (buffer);
		if (gsi_indenter_relocate (indenter, view, location, c))
			g_debug ("relocated");
		gtk_text_buffer_end_user_action (buffer);
	}
	
	mark = gtk_text_buffer_get_mark (buffer, "insert");
	gtk_text_buffer_get_iter_at_mark (buffer, location, mark);
	gtk_text_buffer_get_iter_at_offset (buffer,
					    location,
					    insert_iterator_pos);
}

static gboolean
key_press_event_cb (GtkTextView *view,
		    GdkEventKey *event,
		    GsiPlugin *self)
{
	if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_j)
	{
		GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
		GtkTextIter start, end;

		g_signal_handlers_block_by_func (buffer, insert_text_cb, self);
				
		/*TODO Create a function to get the indenter of a view*/
		GsiIndenter *indenter;
		GtkSourceLanguage *language = gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (buffer));
		const gchar *lang_id = NULL;
		if (language)
		{
			lang_id = gtk_source_language_get_id (language);
		}
		indenter = gsi_indenters_manager_get_indenter (self->priv->manager, lang_id);
		g_assert (indenter != NULL);
		
		if (gtk_text_buffer_get_selection_bounds (buffer, &start, &end))
		{
			gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (view)),
					       self->priv->busy_cursor);
			
			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_indent_region (indenter, view, &start, &end);
			gtk_text_buffer_end_user_action (buffer);
			
			gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (view)),
					       NULL);
		}
		else
		{
			gtk_text_buffer_get_iter_at_mark (buffer,
							  &start,
							  gtk_text_buffer_get_insert (buffer));
			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_indent_line (indenter, view, &start);
			gtk_text_buffer_end_user_action (buffer);
		}
		
		g_signal_handlers_unblock_by_func (buffer, insert_text_cb, self);
	}
	return FALSE;
}

static void
paste_clipboard_cb (GtkTextView *view,
		    GsiPlugin *self)
{
	g_signal_handlers_block_by_func (gtk_text_view_get_buffer (view),
					 insert_text_cb,
					 self);
}

static void
paste_done_cb (GtkTextBuffer *buffer,
	       GtkClipboard *clip,
	       GsiPlugin *self)
{
	g_signal_handlers_unblock_by_func (buffer,
					   insert_text_cb,
					   self);
}

static void
undo_cb (GtkTextView *view,
	 GsiPlugin *self)
{
	g_signal_handlers_block_by_func (gtk_text_view_get_buffer (view),
					 insert_text_cb,
					 self);
}

static void
undo_after_cb (GtkTextView *view,
	       GsiPlugin *self)
{
	g_signal_handlers_unblock_by_func (gtk_text_view_get_buffer (view),
					   insert_text_cb,
					   self);
}

static void
document_enable (GsiPlugin *self, GeditView *view)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	g_object_set_data (G_OBJECT (buffer), VIEW_KEY, view);
	
	g_signal_connect_after (buffer,
				"insert-text",
				G_CALLBACK (insert_text_cb),
				self);
	g_signal_connect_after (view,
				"key-press-event",
				G_CALLBACK (key_press_event_cb),
				self);
	/*Block the insert-text signal when the user paste text*/
	g_signal_connect (view,
			  "paste-clipboard",
			  G_CALLBACK (paste_clipboard_cb),
			  self);
	/*Unblock the insert-text signal when the paste ends*/
	g_signal_connect (buffer,
			  "paste-done",
			  G_CALLBACK (paste_done_cb),
			  self);
	
	/*Block/unblock the insert-text signal on undo*/
	g_signal_connect (view,
			  "undo",
			  G_CALLBACK (undo_cb),
			  self);
	g_signal_connect_after (view,
				"undo",
			  	G_CALLBACK (undo_after_cb),
			  	self);
	/*Block/unblock the insert-text signal on redo*/
	g_signal_connect (view,
			  "redo",
			  G_CALLBACK (undo_cb),
			  self);
	g_signal_connect_after (view,
				"redo",
			  	G_CALLBACK (undo_after_cb),
			  	self);
	
	g_object_set_data (G_OBJECT (view), ENABLED_KEY, buffer);
}

static void
tab_changed_cb (GeditWindow *geditwindow,
                GeditTab    *tab,
                GsiPlugin   *self)
{
        GeditView *view = gedit_tab_get_view (tab);
	
	if (!is_enabled (view))
	  document_enable (self, view);
}


static void
impl_activate (GeditWindowActivatable *activatable)
{
	GsiPlugin *self = GSI_PLUGIN (activatable);
	WindowData *wdata;
	GList *views, *l;
	GeditView *view;
	
	gedit_debug (DEBUG_PLUGINS);
	
	wdata = g_slice_new (WindowData);
	wdata->plugin = self;
	wdata->window = self->priv->window;

	g_object_set_data_full (G_OBJECT (self->priv->window),
                                WINDOW_DATA_KEY,
                                wdata,
                                (GDestroyNotify) window_data_free);

	g_signal_connect_after (self->priv->window, "active-tab-changed",
				G_CALLBACK (tab_changed_cb),
				self);

	/* TODO
        g_signal_connect (window, "active-tab-state-changed",
                          G_CALLBACK (tab_state_changed_cb),
                          self);
	*/

	views = gedit_window_get_views (self->priv->window);
	for (l = views; l != NULL; l = g_list_next (l))
	{
		view = GEDIT_VIEW (l->data);
		document_enable (self, view);
        }

}

static void
impl_deactivate (GeditWindowActivatable *activatable)
{
	GsiPlugin *self = GSI_PLUGIN (activatable);
	gedit_debug (DEBUG_PLUGINS);

	g_object_set_data (G_OBJECT (self->priv->window), WINDOW_DATA_KEY, NULL);
}

static void
gsi_plugin_class_init (GsiPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gsi_plugin_dispose;
	object_class->set_property = gsi_plugin_set_property;
	object_class->get_property = gsi_plugin_get_property;
	
	g_object_class_override_property (object_class, PROP_WINDOW, "window");

	g_type_class_add_private (object_class, 
				  sizeof (GsiPluginPrivate));
}

static void
gedit_window_activatable_iface_init (GeditWindowActivatableInterface *iface)
{
	iface->activate = impl_activate;
	iface->deactivate = impl_deactivate;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	gsi_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    GEDIT_TYPE_WINDOW_ACTIVATABLE,
						    TYPE_GSI_PLUGIN);
}

