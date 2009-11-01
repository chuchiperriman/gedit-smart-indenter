/* gsi-indenter.c */

#include "gsi-indenter.h"

static void
gsi_indenter_base_init (gpointer g_class)
{
  static gboolean is_initialized = FALSE;

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
        gsi_indenter_base_init,   /* base_init */
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

  GSI_INDENTER_GET_INTERFACE (self)->indent_new_line (self, view, iter);
}
