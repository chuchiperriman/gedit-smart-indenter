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

#include <glib/gi18n-lib.h>
#include <gedit/gedit-debug.h>

#define WINDOW_DATA_KEY	"GeditsmartindenterPluginWindowData"

#define GEDITSMARTINDENTER_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPluginPrivate))

#define get_window_data(window) ((WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY)))


struct _GeditsmartindenterPluginPrivate
{
	gpointer dummy;
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
}

static void
geditsmartindenter_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditsmartindenterPlugin finalizing");

	G_OBJECT_CLASS (geditsmartindenter_plugin_parent_class)->finalize (object);
}

static void
is_plus_indent (const gchar *text)
{
	gint i;
	gchar *indent;
	if (g_regex_match_simple ("\\.*\\{\\.*[^\\}]*\\.*$",
				  text,
				  0,
				  0))
	{
		printf ("%s\n", text);
		/*indent = get_indent (text);
		if (indent)
		{
			printf ("%s    print();\n", indent);
			g_free (indent);
		}
		*/
	}
	else
	{
		printf ("%s (NOT)\n", text);
	}
}

static void
window_data_free (WindowData *data)
{
        g_return_if_fail (data != NULL);

        g_slice_free (WindowData, data);
}

static void
insert_cb (GtkTextBuffer	*buffer,
	   GtkTextIter		*location,
	   gchar		*text,
	   gint			 len,
	   GeditsmartindenterPlugin *self)
{
	gint line;
	
	if (g_utf8_collate (text, "\n") == 0)
	{
		gchar *line_text;
		GtkTextIter start_line = *location;
		GtkTextIter end_line = *location;
		
		gtk_text_iter_backward_line (&end_line);
		gtk_text_iter_backward_line (&start_line);
		
		gtk_text_iter_set_line_offset (&start_line, 0);
		gtk_text_iter_forward_to_line_end (&end_line);
		
		line_text = gtk_text_buffer_get_text (buffer,
						      &start_line,
						      &end_line,
						      FALSE);
		g_debug ("Text [%s]", line_text);
		
		is_plus_indent (line_text);
		
		g_free (line_text);
	}
}

static void
document_enable (GeditsmartindenterPlugin *self, GeditDocument *doc)
{
	g_signal_connect_after (doc,
				"insert-text",
				G_CALLBACK (insert_cb),
				self);
}

static void
tab_changed_cb (GeditWindow *geditwindow,
                GeditTab    *tab,
                GeditsmartindenterPlugin   *self)
{
        GeditDocument *doc = gedit_tab_get_document (tab);

        document_enable (self, doc);
}


static void
impl_activate (GeditPlugin *plugin,
	       GeditWindow *window)
{
	GeditsmartindenterPlugin *self = GEDITSMARTINDENTER_PLUGIN (plugin);
	WindowData *wdata;
	GList *docs, *l;
	GeditDocument *doc;
	
	gedit_debug (DEBUG_PLUGINS);
	
	wdata = g_slice_new (WindowData);
	wdata->plugin = self;
	wdata->window = window;

	g_object_set_data_full (G_OBJECT (window),
                                WINDOW_DATA_KEY,
                                wdata,
                                (GDestroyNotify) window_data_free);

	g_signal_connect (window, "active-tab-changed",
                          G_CALLBACK (tab_changed_cb),
                          self);

	/*
        g_signal_connect (window, "active-tab-state-changed",
                          G_CALLBACK (tab_state_changed_cb),
                          self);
	*/

	docs = gedit_window_get_documents (window);
	for (l = docs; l != NULL; l = g_list_next (l))
	{
		doc = GEDIT_DOCUMENT (l->data);
		document_enable (self, doc);
        }

}

static void
impl_deactivate (GeditPlugin *plugin,
		 GeditWindow *window)
{
	GeditsmartindenterPlugin *self = GEDITSMARTINDENTER_PLUGIN (plugin);
	WindowData *wdata;
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
