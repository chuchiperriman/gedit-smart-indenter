/* gsi-indenter-python.c */

#include <string.h>
#include "gsi-indenter-python.h"
#include "gsi-indenter-utils.h"

#define GSI_INDENTER_PYTHON_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_PYTHON, GsiIndenterPythonPrivate))
  
typedef struct _GsiIndenterPythonPrivate GsiIndenterPythonPrivate;

struct _GsiIndenterPythonPrivate
{
	gboolean dummy;
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterPython,
                         gsi_indenter_python,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))


static gboolean
is_sentence (GtkTextIter *iter)
{
	GtkTextIter copy = *iter;
	
	//TODO Move to last non blank char
	if (!gtk_text_iter_forward_to_line_end (&copy) ||
	    !gtk_text_iter_backward_char(&copy))
	{
		return FALSE;
	}
	
	if (gtk_text_iter_get_char(&copy) == ':')
		return TRUE;
	
	return FALSE;
}

static gboolean
gsi_indenter_indent_line_real (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gchar *indent = NULL;
	GtkTextIter copy = *iter;
	gint level;
	
	if (!gsi_indenter_utils_iter_backward_line_not_empty (&copy))
	{
		return FALSE;
	}
	
	level = gsi_indenter_utils_get_amount_indents (view, &copy);
	
	if (level == -1)
		level = 0;
	
	if (is_sentence(&copy))
		level += gsi_indenter_utils_view_get_real_indent_width(GTK_SOURCE_VIEW (view));

	indent = gsi_indenter_utils_get_indent_string_from_indent_level	(GTK_SOURCE_VIEW (view),
									 level);
	
	if (!indent)
		indent = g_strdup ("");
	
	gsi_indenter_utils_replace_indentation (buffer, 
						gtk_text_iter_get_line (iter), 
						indent);
	
	g_free(indent);
	
	return TRUE;
}

static void
gsi_indenter_indent_line_impl (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gtk_text_buffer_begin_user_action (buffer);
	gsi_indenter_indent_line_real (indenter, view, iter);
	gtk_text_buffer_end_user_action (buffer);
}

static const gchar*
gsi_indenter_get_relocators_impl (GsiIndenter	*self,
			     	  GtkTextView	*view)
{
	return NULL;
}

static void
gsi_indenter_indent_region_imp (GsiIndenter	*self,
				GtkTextView	*view,
				GtkTextIter	*start,
				GtkTextIter	*end)
{
	/*
	  Do not support region indentation because python has
	  not regions like {} to check how indent a region
	*/
}


static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
	iface->indent_line = gsi_indenter_indent_line_impl;
	iface->get_relocators = gsi_indenter_get_relocators_impl;
	iface->indent_region = gsi_indenter_indent_region_imp;
}

static void
gsi_indenter_python_dispose (GObject *object)
{
	G_OBJECT_CLASS (gsi_indenter_python_parent_class)->dispose (object);
}

static void
gsi_indenter_python_class_init (GsiIndenterPythonClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (GsiIndenterPythonPrivate));
	
	object_class->dispose = gsi_indenter_python_dispose;
}

static void
gsi_indenter_python_init (GsiIndenterPython *self)
{
}

GsiIndenter*
gsi_indenter_python_new (void)
{
	return g_object_new (GSI_TYPE_INDENTER_PYTHON, NULL);
}
