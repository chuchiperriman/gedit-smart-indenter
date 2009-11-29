/* gsi-indenter-simple.c */

#include "gsi-indenter-simple.h"
#include "gsi-indenter-utils.h"

#define GSI_INDENTER_SIMPLE_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_SIMPLE, GsiIndenterSimplePrivate))
  
typedef struct _GsiIndenterSimplePrivate GsiIndenterSimplePrivate;

struct _GsiIndenterSimplePrivate
{
	gboolean dummy;
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterSimple,
                         gsi_indenter_simple,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))

static void
gsi_indenter_indent_line_real (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gint line = gtk_text_iter_get_line (iter);
	gchar *indentation = NULL;
	
	indentation = gsi_indenter_utils_get_line_indentation (buffer, line -1, TRUE);
	
	gsi_indenter_utils_replace_indentation (buffer, line, indentation);
	
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

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
        iface->indent_line = gsi_indenter_indent_line_impl;
}

static void
gsi_indenter_simple_dispose (GObject *object)
{
	G_OBJECT_CLASS (gsi_indenter_simple_parent_class)->dispose (object);
}

static void
gsi_indenter_simple_class_init (GsiIndenterSimpleClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (GsiIndenterSimplePrivate));
	
	object_class->dispose = gsi_indenter_simple_dispose;
}

static void
gsi_indenter_simple_init (GsiIndenterSimple *self)
{
}

GsiIndenter*
gsi_indenter_simple_new (void)
{
	return GSI_INDENTER (g_object_new (GSI_TYPE_INDENTER_SIMPLE, NULL));
}
