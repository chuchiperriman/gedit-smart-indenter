/* gsi-indenter.c */

#include "gsi-indenter.h"

static void
gsi_indenter_indent_line_default (GsiIndenter	*self,
				  GtkTextView	*view,
				  GtkTextIter	*iter)
{
}

static void
gsi_indenter_indent_region_default (GsiIndenter	*self,
				    GtkTextView	*view,
				    GtkTextIter	*start,
				    GtkTextIter	*end)
{
}

static const gchar*
gsi_indenter_get_relocators_default (GsiIndenter	*self,
				     GtkTextView	*view)
{
	return NULL;
}

static gboolean
gsi_indenter_relocate_default (GsiIndenter	*self,
			       GtkTextView	*view,
			       GtkTextIter	*iter,
			       gchar		 relocator)
{
	return FALSE;
}

static void
gsi_indenter_init (GsiIndenterInterface *iface)
{
	static gboolean is_initialized = FALSE;
	
	iface->indent_line = gsi_indenter_indent_line_default;
	iface->indent_region = gsi_indenter_indent_region_default;
	iface->get_relocators = gsi_indenter_get_relocators_default;
	iface->relocate = gsi_indenter_relocate_default;

	if (!is_initialized)
	{
	/* add properties and signals to the interface here */
	is_initialized = TRUE;
    }
}

GType
gsi_indenter_get_type (void)
{
	static GType iface_type = 0;
	if (iface_type == 0)
	{
		static const GTypeInfo info = {
			sizeof (GsiIndenterInterface),
			(GBaseInitFunc)gsi_indenter_init,   /* base_init */
			NULL,   /* base_finalize */
		};
		
		iface_type = g_type_register_static (G_TYPE_INTERFACE, 
						     "GsiIndenter",
						     &info, 0);
	}
		
	return iface_type;
}
				
void
gsi_indenter_indent_line (GsiIndenter	*self,
			  GtkTextView	*view,
			  GtkTextIter	*iter)
{
	g_return_if_fail (GSI_IS_INDENTER (self));
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));
	g_return_if_fail (iter != NULL);
	
	GSI_INDENTER_GET_INTERFACE (self)->indent_line (self, view, iter);
}

void
gsi_indenter_indent_region (GsiIndenter	*self,
			    GtkTextView	*view,
			    GtkTextIter	*start,
			    GtkTextIter	*end)
{
	g_return_if_fail (GSI_IS_INDENTER (self));
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);
	
	GSI_INDENTER_GET_INTERFACE (self)->indent_region (self, view, start, end);
}

const gchar*
gsi_indenter_get_relocators (GsiIndenter	*self,
			     GtkTextView	*view)
{
	g_return_val_if_fail (GSI_IS_INDENTER (self), NULL);
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);
	
	return GSI_INDENTER_GET_INTERFACE (self)->get_relocators (self, view);
}

gboolean
gsi_indenter_relocate (GsiIndenter	*self,
		       GtkTextView	*view,
		       GtkTextIter	*iter,
		       gchar		relocator)
{
	g_return_val_if_fail (GSI_IS_INDENTER (self), FALSE);
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	
	return GSI_INDENTER_GET_INTERFACE (self)->relocate (self, view, iter, relocator);
}

