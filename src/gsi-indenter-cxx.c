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
	g_debug ("cxx new");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gint line = gtk_text_iter_get_line (iter);
	gchar *indentation = NULL;
	
	indentation = gsi_indenter_utils_get_line_indentation (buffer, line -1, TRUE);
	
	gtk_text_buffer_begin_user_action (buffer);
	gsi_indenter_utils_replace_indentation (buffer, line, indentation);
	gtk_text_buffer_end_user_action (buffer);
	g_debug ("end cxx new");
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
	g_debug ("cxx: relocating");
	if (relocator == '}')
	{
		GtkTextIter open_iter = *iter;
		if (gsi_indenter_utils_find_open_char (&open_iter,
					   '{',
					   '}',
					   TRUE))
		{
			gint open_line = gtk_text_iter_get_line (&open_iter);
			gint line = gtk_text_iter_get_line (iter);
			gchar *indentation;
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
			indentation = gsi_indenter_utils_get_line_indentation (buffer, open_line, FALSE);

			g_debug ("ind [%s][%i-%i]", indentation, open_line, line);
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
