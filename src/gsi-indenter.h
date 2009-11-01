/* gsi-indenter.h */

#ifndef _GSI_INDENTER_H
#define _GSI_INDENTER_H 

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSI_TYPE_INDENTER gsi_indenter_get_type()

#define GSI_INDENTER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTER, GsiIndenter))

#define GSI_IS_INDENTER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTER))

#define GSI_INDENTER_GET_INTERFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
  GSI_TYPE_INDENTER, GsiIndenterInterface))


typedef struct _GsiIndenter               GsiIndenter; /* dummy object */
typedef struct _GsiIndenterInterface      GsiIndenterInterface;

struct _GsiIndenterInterface
{
  GTypeInterface parent_iface;

  void (*indent_new_line) (GsiIndenter	*self,
			   GtkTextView	*view,
			   GtkTextIter	*iter);
};

GType gsi_indenter_get_type (void);

void gsi_indenter_indent_new_line (GsiIndenter *self,
				   GtkTextView	*view,
				   GtkTextIter	*iter);

G_END_DECLS

#endif /* _GSI_INDENTER_H */
