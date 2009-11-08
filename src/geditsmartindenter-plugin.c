/*
 * geditsmartindenter-plugin.c - Type here a short description of your plugin
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

#include "geditsmartindenter-plugin.h"
#include "gsi-indenters-manager.h"

#include <glib/gi18n-lib.h>
#include <gedit/gedit-debug.h>
#include <gdk/gdkkeysyms.h>
#include <gtksourceview/gtksourceview.h>


#define WINDOW_DATA_KEY	"GeditsmartindenterPluginWindowData"
#define VIEW_KEY	"GeditsmartindenterPluginView"
#define ENABLED_KEY     "GeditsmartindenterPluginEnabled"

#define GEDITSMARTINDENTER_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPluginPrivate))

#define get_window_data(window) ((WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY)))
#define get_view(buffer) ((GtkTextView *) (g_object_get_data (G_OBJECT (buffer), VIEW_KEY)))
#define is_enabled(view) ((g_object_get_data (G_OBJECT (view), ENABLED_KEY)) != NULL)

struct _GeditsmartindenterPluginPrivate
{
	GsiIndentersManager *manager;
};

typedef struct{
	GeditWindow			*window;
	GeditsmartindenterPlugin	*plugin;
} WindowData;

GEDIT_PLUGIN_REGISTER_TYPE (GeditsmartindenterPlugin, geditsmartindenter_plugin)

static void
geditsmartindenter_plugin_init (GeditsmartindenterPlugin *plugin)
{
	plugin->priv = GEDITSMARTINDENTER_PLUGIN_GET_PRIVATE (plugin);

	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditsmartindenterPlugin initializing");

	plugin->priv->manager = gsi_indenters_manager_new ();
}

static void
geditsmartindenter_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditsmartindenterPlugin finalizing");

	GeditsmartindenterPlugin *self = GEDITSMARTINDENTER_PLUGIN (object);
	
	g_object_unref (self->priv->manager);
	
	G_OBJECT_CLASS (geditsmartindenter_plugin_parent_class)->finalize (object);
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
		GeditsmartindenterPlugin *self)
{
	/*TODO prevent paste by the moment*/
	if (len >2)
		return;
		
	gchar c = text[len-1];
	if (c == '\n')
	{
		g_debug ("insert");
		GtkTextView *view = get_view (buffer);
		GsiIndenter *indenter;
		GtkSourceLanguage *language = gtk_source_buffer_get_language (GTK_SOURCE_BUFFER (buffer));
		const gchar *lang_id = NULL;
		if (language)
		{
			lang_id = gtk_source_language_get_id (language);
		}
		indenter = gsi_indenters_manager_get_indenter (self->priv->manager, lang_id);
		g_assert (indenter != NULL);
	
		gsi_indenter_indent_new_line (indenter, view, location);
		g_debug ("end insert");
	}
}

static gboolean
key_press_event_cb (GtkTextView *view,
		    GdkEventKey *event,
		    GeditsmartindenterPlugin *self)
{
	if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_j)
	{
		g_debug ("cj");
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
			gsi_indenter_indent_region (indenter, view, &start, &end);
		}
		else
		{
			gtk_text_buffer_get_iter_at_mark (buffer,
							  &start,
							  gtk_text_buffer_get_insert (buffer));
			gsi_indenter_indent_line (indenter, view, &start);
		}
		
		g_signal_handlers_unblock_by_func (buffer, insert_text_cb, self);
		g_debug ("end cj");
	}
	return FALSE;
}

static void
document_enable (GeditsmartindenterPlugin *self, GeditView *view)
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
	g_object_set_data (G_OBJECT (view), ENABLED_KEY, buffer);
}

static void
tab_changed_cb (GeditWindow *geditwindow,
                GeditTab    *tab,
                GeditsmartindenterPlugin   *self)
{
        GeditView *view = gedit_tab_get_view (tab);
	
	if (!is_enabled (view))
	  document_enable (self, view);
}


static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GeditsmartindenterPlugin *self = GEDITSMARTINDENTER_PLUGIN (plugin);
	WindowData *wdata;
	GList *views, *l;
	GeditView *view;
	
	gedit_debug (DEBUG_PLUGINS);
	
	wdata = g_slice_new (WindowData);
	wdata->plugin = self;
	wdata->window = window;

	g_object_set_data_full (G_OBJECT (window),
                                WINDOW_DATA_KEY,
                                wdata,
                                (GDestroyNotify) window_data_free);

	g_signal_connect_after (window, "active-tab-changed",
				G_CALLBACK (tab_changed_cb),
				self);

	/* TODO
        g_signal_connect (window, "active-tab-state-changed",
                          G_CALLBACK (tab_state_changed_cb),
                          self);
	*/

	views = gedit_window_get_views (window);
	for (l = views; l != NULL; l = g_list_next (l))
	{
		view = GEDIT_VIEW (l->data);
		document_enable (self, view);
        }

}

static void
impl_deactivate (GeditPlugin *plugin,
		 GeditWindow *window)
{
	/*GeditsmartindenterPlugin *self = GEDITSMARTINDENTER_PLUGIN (plugin);*/
	gedit_debug (DEBUG_PLUGINS);

	g_object_set_data (G_OBJECT (window), WINDOW_DATA_KEY, NULL);
}

static void
impl_update_ui (GeditPlugin *plugin,
		GeditWindow *window)
{
	gedit_debug (DEBUG_PLUGINS);
}


static void
geditsmartindenter_plugin_class_init (GeditsmartindenterPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GeditPluginClass *plugin_class = GEDIT_PLUGIN_CLASS (klass);

	object_class->finalize = geditsmartindenter_plugin_finalize;

	plugin_class->activate = impl_activate;
	plugin_class->deactivate = impl_deactivate;
	plugin_class->update_ui = impl_update_ui;

	g_type_class_add_private (object_class, 
				  sizeof (GeditsmartindenterPluginPrivate));
}
