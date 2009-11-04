/* gsi-indenter.c */

#include "gsi-indenter.h"

static void
gsi_indenter_indent_new_line_default (GsiIndenter	*self,
				      GtkTextView	*view,
				      GtkTextIter	*iter)
{
}

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

static gboolean
gsi_indenter_has_relocators_default (GsiIndenter *self)
{
	return FALSE;
}

static void
gsi_indenter_init (GsiIndenterInterface *iface)
{
	static gboolean is_initialized = FALSE;
	
	iface->indent_new_line = gsi_indenter_indent_new_line_default;
	iface->indent_line = gsi_indenter_indent_line_default;
	iface->indent_region = gsi_indenter_indent_region_default;
	iface->has_relocators = gsi_indenter_has_relocators_default;

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

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "GsiIndenter",
                                           &info, 0);
    }

  return iface_type;
}

void
gsi_indenter_indent_new_line (GsiIndenter	*self,
			      GtkTextView	*view,
			      GtkTextIter	*iter)
{
  g_return_if_fail (GSI_IS_INDENTER (self));
  g_return_if_fail (GTK_IS_TEXT_VIEW (view));
  g_return_if_fail (iter != NULL);

  GSI_INDENTER_GET_INTERFACE (self)->indent_new_line (self, view, iter);
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

gboolean
gsi_indenter_has_relocators (GsiIndenter *self)
{
	g_return_val_if_fail (GSI_IS_INDENTER (self), FALSE);
	return GSI_INDENTER_GET_INTERFACE (self)->has_relocators (self);
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
	g_return_val_if_fail (relocator != NULL, FALSE);
	
	return GSI_INDENTER_GET_INTERFACE (self)->relocate (self, view, iter, relocator);
}




