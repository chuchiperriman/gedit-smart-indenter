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
#include <gdk/gdkkeysyms.h>


#define WINDOW_DATA_KEY	"GeditsmartindenterPluginWindowData"

#define GEDITSMARTINDENTER_PLUGIN_GET_PRIVATE(object)	(G_TYPE_INSTANCE_GET_PRIVATE ((object), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPluginPrivate))

#define get_window_data(window) ((WindowData *) (g_object_get_data (G_OBJECT (window), WINDOW_DATA_KEY)))

typedef struct{
	gchar *match;
	gchar *indent;
	gchar *append;
	gboolean search_pair;
	gchar *start_pair;
	gchar *end_pair;
} Indenter;

struct _GeditsmartindenterPluginPrivate
{
	GList *indenters;
};

typedef struct{
	GeditWindow			*window;
	GeditsmartindenterPlugin	*plugin;
} WindowData;

GEDIT_PLUGIN_REGISTER_TYPE (GeditsmartindenterPlugin, geditsmartindenter_plugin)

static Indenter*
indenter_new (const gchar *match,
	      const gchar *indent,
	      const gchar *append)
{
	Indenter *indenter;
	indenter = g_slice_new (Indenter);
	indenter->match = g_strdup (match);
	indenter->indent = g_strdup (indent);
	indenter->append = g_strdup (append);
	indenter->search_pair = FALSE;
	
	return indenter;
}

static Indenter*
indenter_new_pair (const gchar *match,
		   const gchar *indent,
		   const gchar *append,
		   const gchar *start_pair,
		   const gchar *end_pair)
{
	Indenter *indenter = indenter_new (match, indent, append);
	indenter->search_pair = TRUE;
	indenter->start_pair = g_strdup (start_pair);
	indenter->end_pair = g_strdup (end_pair);
	
	return indenter;
}

static gchar *
get_indent_string (guint tabs, guint spaces)
{
	gchar *str;

	str = g_malloc (tabs + spaces + 1);
	if (tabs > 0)
		memset (str, '\t', tabs);
	if (spaces > 0)
		memset (str + tabs, ' ', spaces);
	str[tabs + spaces] = '\0';

	return str;
}

static guint
get_real_indent_width (GtkSourceView *view)
{
	return gtk_source_view_get_indent_width (view) < 0 ?
	       gtk_source_view_get_tab_width (view) :
	       (guint) gtk_source_view_get_indent_width (view);
}

static gchar*
get_indent_pair (GtkSourceView *view)
{
	GtkTextBuffer *buf;
	guint spaces = 0, tabs = 0;
	gchar *tab_buffer = NULL;
	
	buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	
	if (gtk_source_view_get_insert_spaces_instead_of_tabs (view))
	{
		spaces = get_real_indent_width (view);

		tab_buffer = g_strnfill (spaces, ' ');
	}
	else if (gtk_source_view_get_indent_width (view) > 0)
	{
		guint indent_width;

		indent_width = get_real_indent_width (view);
		spaces = indent_width % gtk_source_view_get_tab_width (view);
		tabs = indent_width / gtk_source_view_get_tab_width (view);

		tab_buffer = get_indent_string (tabs, spaces);
	}
	else
	{
		tabs = 1;
		tab_buffer = g_strdup ("\t");
	}
	
	return tab_buffer;
}

static void
geditsmartindenter_plugin_init (GeditsmartindenterPlugin *plugin)
{
	plugin->priv = GEDITSMARTINDENTER_PLUGIN_GET_PRIVATE (plugin);

	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditsmartindenterPlugin initializing");

	plugin->priv->indenters = NULL;
	
	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
						 indenter_new (".*\\/\\*(?!.*\\*/)", "+1", " * "));
	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
						 indenter_new (".*\\{[^\\}]*", "+1", "	"));
	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
						 indenter_new_pair ("\\([^\\)]*$", "+1", NULL, "(", ")"));
	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
						 indenter_new ("(if|while|for)\\s*\\(.*\\)\\s*$", "+1", "	"));
	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
						 indenter_new ("^\\s*\\*[^/].*$", "+1", "* "));
}

static void
geditsmartindenter_plugin_finalize (GObject *object)
{
	gedit_debug_message (DEBUG_PLUGINS,
			     "GeditsmartindenterPlugin finalizing");

	G_OBJECT_CLASS (geditsmartindenter_plugin_parent_class)->finalize (object);
}

static gboolean
move_to_pair_char (GtkTextIter *iter, 
		   const gchar *start_pair,
		   const gchar *end_pair)
{
	gint count = 0;
	gboolean found = FALSE;
	gchar *c;
	GtkTextIter prev_iter = *iter;
	
	g_debug ("searching pair %s", start_pair);
	
	while (gtk_text_iter_backward_char (iter))
	{
		c = gtk_text_iter_get_slice (iter, &prev_iter);
		
		prev_iter = *iter;
		
		if (g_utf8_collate (end_pair, c) == 0)
		{
			count++;
		}
		else if (g_utf8_collate (start_pair, c) == 0)
		{
			if (count == 0)
			{
				g_debug ("Found pair: %i-%i",
					 gtk_text_iter_get_line (iter),
					 gtk_text_iter_get_line_offset (iter));
				found = TRUE;
				break;
			}
			count--;
		}
	}
	return found;
}

static gchar*
get_indent (Indenter *indenter, const gchar *text)
{
	GRegex *regex;
	GMatchInfo *match_info;
	gchar *ind = NULL;
	gchar *end = NULL;

	/*TODO Store the regex in cache*/
	regex = g_regex_new ("^\\s*", 0, 0, NULL);
	g_regex_match (regex, text, 0, &match_info);
	if (g_match_info_matches (match_info))
	{
		ind = g_match_info_fetch (match_info, 0);
	}
	
	g_match_info_free (match_info);
	g_regex_unref (regex);
	
	if (indenter && indenter->append)
	{
		end = g_strconcat (ind, indenter->append, NULL);
		g_free (ind);
	}
	else
	{
		end = ind;
	}
	
	return end;
}

static gchar*
indenter_process (Indenter *indenter, GtkTextIter *iter, const gchar *text)
{
	gchar *indent = NULL;
	/*TODO Store the regexp in cache*/
	if (g_regex_match_simple (indenter->match,
				  text,
				  0,
				  0))
	{
		if (indenter->search_pair)
		{
			indent = get_indent (indenter, text);
			
			if (move_to_pair_char (iter, indenter->start_pair, indenter->end_pair))
			{
				gchar *spaces = g_strnfill (gtk_text_iter_get_line_offset (iter) + 1, ' ');
				g_free (indent);
				indent = spaces;
			}
		}
		else
		{
			indent = get_indent (indenter, text);
		}
	}
	
	return indent;
}

static void
window_data_free (WindowData *data)
{
        g_return_if_fail (data != NULL);

        g_slice_free (WindowData, data);
}

static gboolean
key_press_cb (GtkTextView		*view,
		GdkEventKey 		*event,
		GeditsmartindenterPlugin *self)
{
	GList *l;
	Indenter *indenter;
	GtkTextIter location, start_line, end_line;
	GtkTextIter temp_iter;
	GtkTextBuffer *buffer;
	gchar *line_text;
	gchar *indent = NULL;

	if (event->keyval != GDK_Return)
		return FALSE;
	
	buffer = gtk_text_view_get_buffer (view);
	gtk_text_buffer_get_iter_at_mark (buffer, 
	                            &location,
	                            gtk_text_buffer_get_insert (buffer));
		
	if (gtk_text_iter_get_line_offset (&location) != 0)
	{
		start_line = location;
		gtk_text_iter_set_line_offset (&start_line, 0);
		end_line = start_line;
		gtk_text_iter_forward_to_line_end (&end_line);

		line_text = gtk_text_buffer_get_text (buffer,
						      &start_line,
						      &end_line,
						      FALSE);
		for (l = self->priv->indenters; l != NULL ; l = g_list_next (l))
		{
			temp_iter = location;
			indenter = (Indenter*)l->data;
			indent = indenter_process (indenter, &temp_iter, line_text);

			if (indent) break;
		}
		
		if (!indent)
		{
			indent = get_indent (NULL, line_text);
		}
		g_debug ("start insert");
		gtk_text_buffer_begin_user_action (buffer);
		gtk_text_buffer_insert_at_cursor (buffer,
						  "\n",
						  -1);
		if (indent)
		{
			gtk_text_buffer_insert_at_cursor (buffer,
							  indent,
							  g_utf8_strlen (indent, -1));
			g_free (indent);
		}
		gtk_text_buffer_end_user_action (buffer);
		g_debug ("end insert");
		
		g_free (line_text);
		
		return TRUE;
	}
	//g_debug ("location end %i: ", gtk_text_iter_get_line_offset (&location));
	/*
	g_signal_handlers_unblock_by_func (buffer,
					   insert_cb,
					   self);
	*/
	return FALSE;
}

static void
document_enable (GeditsmartindenterPlugin *self, GeditView *view)
{
	g_signal_connect (view,
			  "key-press-event",
			  G_CALLBACK (key_press_cb),
			  self);
}

static void
tab_changed_cb (GeditWindow *geditwindow,
                GeditTab    *tab,
                GeditsmartindenterPlugin   *self)
{
        GeditView *view = gedit_tab_get_view (tab);

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

	g_signal_connect (window, "active-tab-changed",
                          G_CALLBACK (tab_changed_cb),
                          self);

	/*
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
