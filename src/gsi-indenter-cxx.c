/* gsi-indenter-cxx.c */

#include "gsi-indenter-cxx.h"
#include "gsi-indenter-utils.h"

#define INDENTER_CXX_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_CXX, GsiIndenterCxxPrivate))
  
typedef struct _GsiIndenterCxxPrivate GsiIndenterCxxPrivate;

struct _GsiIndenterCxxPrivate
{
	gboolean use_spaces;
	guint tab_width;
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterCxx,
                         gsi_indenter_cxx,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))


//	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
//-
/*
-	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
-						 indenter_new (".*\\{[^\\}]*", "+1", "	"));
-	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
-						 indenter_new_pair ("\\([^\\)]*$", "+1", NULL, "(", ")"));
-	plugin->priv->indenters = g_list_append (plugin->priv->indenters,
-						 indenter_new ("(if|while|for)\\s*\\(.*\\)\\s*$", "+1", "	"));
*/

static gboolean
gsi_indenter_cxx_indent_regexp (GsiIndenter *indenter,
				GtkTextBuffer *buffer,
				GtkTextIter *iter,
				const gchar *regexp,
				guint indentation,
				const gchar *append)
{
	GtkTextIter prev_iter = *iter;
	gchar *line_text;
	
	gtk_text_iter_backward_line (&prev_iter);
	
	line_text = gsi_indenter_utils_get_line_text (buffer, &prev_iter);
	if (g_regex_match_simple (regexp,
				  line_text,
				  0,
				  0))
	{
		gint line = gtk_text_iter_get_line (iter);
		gchar *indent = gsi_indenter_utils_get_line_indentation (buffer, line -1, TRUE);
		gchar *final_indent;

		
		if (append)
		{
			if (indent)
				final_indent = g_strjoin (NULL, indent, append, NULL);
			else
				final_indent = g_strdup (append);
		}
		else
		{
			final_indent = g_strdup (indent);
		}
		gtk_text_buffer_begin_user_action (buffer);
		gsi_indenter_utils_replace_indentation (buffer, line, final_indent);
		gtk_text_buffer_end_user_action (buffer);
		
		g_free (final_indent);
		return TRUE;
	}
	
	return FALSE;

}

static void
gsi_indenter_indent_line_impl (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	
}

static void
gsi_indenter_indent_new_line_impl (GsiIndenter *indenter,
				   GtkTextView *view,
				   GtkTextIter *iter)
{
	/*TODO Previous indentation by the moment*/
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gint line = gtk_text_iter_get_line (iter);
	gchar *indentation = NULL;
	gboolean found = FALSE;
	
	found = gsi_indenter_cxx_indent_regexp (indenter,
						buffer,
						iter,
						".*\\/\\*(?!.*\\*/)",
						0,
						" * ");

	if (found)
		return;
		
	found = gsi_indenter_cxx_indent_regexp (indenter,
						buffer,
						iter,
						"^\\s*\\*[^/].*$",
						0,
						"* ");
	if (found)
		return;
		
	indentation = gsi_indenter_utils_get_line_indentation (buffer, line - 1, TRUE);
	
	gtk_text_buffer_begin_user_action (buffer);
	gsi_indenter_utils_replace_indentation (buffer, line, indentation);
	gtk_text_buffer_end_user_action (buffer);
}

static void
gsi_indenter_indent_region_impl (GsiIndenter *indenter,
				 GtkTextView *view,
				 GtkTextIter *start,
				 GtkTextIter *end)
{

}

static gboolean
gsi_indenter_has_relocators_impl (GsiIndenter	*self)
{
	return TRUE;
}

static const gchar*
gsi_indenter_get_relocators_impl (GsiIndenter	*self,
				  GtkTextView	*view)
{
	return "{}";
}

static gboolean
gsi_indenter_relocate_impl (GsiIndenter	*self,
			    GtkTextView	*view,
			    GtkTextIter	*iter,
			    gchar	 relocator)
{
	if (relocator == '}')
	{
		GtkTextIter open_iter = *iter;
		if (gsi_indenter_utils_find_open_char (&open_iter, '{', '}', TRUE))
		{
			gint open_line = gtk_text_iter_get_line (&open_iter);
			gint line = gtk_text_iter_get_line (iter);
			gchar *indentation;
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
			indentation = gsi_indenter_utils_get_line_indentation (buffer, open_line, FALSE);

			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_utils_replace_indentation (buffer, line, indentation);
			gtk_text_buffer_end_user_action (buffer);
			
			return TRUE;
		}
	}
	return FALSE;
}

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
        iface->indent_new_line = gsi_indenter_indent_new_line_impl;
        iface->indent_line = gsi_indenter_indent_line_impl;
        iface->indent_region = gsi_indenter_indent_region_impl;
	iface->has_relocators = gsi_indenter_has_relocators_impl;
	iface->get_relocators = gsi_indenter_get_relocators_impl;
	iface->relocate = gsi_indenter_relocate_impl;
}

static void
gsi_indenter_cxx_dispose (GObject *object)
{
	G_OBJECT_CLASS (gsi_indenter_cxx_parent_class)->dispose (object);
}

static void
gsi_indenter_cxx_class_init (GsiIndenterCxxClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (GsiIndenterCxxPrivate));
	
	object_class->dispose = gsi_indenter_cxx_dispose;
}

static void
gsi_indenter_cxx_init (GsiIndenterCxx *self)
{
}

GsiIndenter*
gsi_indenter_cxx_new (void)
{
	return g_object_new (GSI_TYPE_INDENTER_CXX, NULL);
}
