/* gsi-indenter-simple.c */

#include "gsi-indenter-simple.h"
#include "gsi-indenter-utils.h"

#define INDENTER_SIMPLE_PRIVATE(o) \
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
	gint line = gtk_text_iter_get_line (iter) -1;
	gchar *indentation = NULL;
	GtkTextIter start = *iter;
	GtkTextIter end;

	while (line >= 0)
	{
		if (!gsi_indenter_utils_is_empty_line (buffer, line))
		{
			indentation =  gsi_indenter_utils_get_line_indentation (buffer,line);
			break;
		}
		line++;
	}
	
	gtk_text_iter_set_line_index (&start, 0);
	
	end = start;
	
	if (gsi_indenter_utils_move_to_no_space (&end, 1, FALSE))
	{
		line = gtk_text_iter_get_line (&start);
		gtk_text_buffer_delete (buffer, &start, &end);
		if (indentation)
		{
			gtk_text_buffer_get_iter_at_line (buffer, &start, line);
			gtk_text_buffer_insert (buffer, &start, indentation, -1);
		}
	}
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
gsi_indenter_indent_new_line_impl (GsiIndenter *indenter,
				   GtkTextView *view,
				   GtkTextIter *iter)
{
	gsi_indenter_indent_line_impl (indenter, view, iter);
}

static void
gsi_indenter_indent_region_impl (GsiIndenter *indenter,
				 GtkTextView *view,
				 GtkTextIter *start,
				 GtkTextIter *end)
{
	gint start_line = gtk_text_iter_get_line (start);
	gint end_line = gtk_text_iter_get_line (end);
	GtkTextIter iter = *start;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	
	/*We indent based on the first selected line*/
	start_line++;
	gtk_text_buffer_begin_user_action (buffer);
	while (start_line <= end_line)
	{
		gtk_text_buffer_get_iter_at_line (buffer, &iter, start_line);
		gsi_indenter_indent_line_real (indenter, view, &iter);
		start_line++;
		
	}
	gtk_text_buffer_end_user_action (buffer);
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

GsiIndenterSimple*
gsi_indenter_simple_new (void)
{
	return g_object_new (GSI_TYPE_INDENTER_SIMPLE, NULL);
}
