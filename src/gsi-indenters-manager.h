/* gsi-indenters-manager.h */

#ifndef _GSI_INDENTERS_MANAGER_H
#define _GSI_INDENTERS_MANAGER_H

#include <glib-object.h>
#include "gsi-indenter.h"

G_BEGIN_DECLS

#define GSI_TYPE_INDENTERS_MANAGER gsi_indenters_manager_get_type()

#define GSI_INDENTERS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTERS_MANAGER, GsiIndentersManager))

#define GSI_INDENTERS_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  GSI_TYPE_INDENTERS_MANAGER, GsiIndentersManagerClass))

#define GSI_IS_INDENTERS_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTERS_MANAGER))

#define GSI_IS_INDENTERS_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  GSI_TYPE_INDENTERS_MANAGER))

#define GSI_INDENTERS_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  GSI_TYPE_INDENTERS_MANAGER, GsiIndentersManagerClass))

typedef struct _GsiIndentersManager GsiIndentersManager;
typedef struct _GsiIndentersManagerClass GsiIndentersManagerClass;
typedef struct _GsiIndentersManagerPrivate GsiIndentersManagerPrivate;

struct _GsiIndentersManager{
	GObject parent;
	GsiIndentersManagerPrivate *priv;
};

struct _GsiIndentersManagerClass{
	GObjectClass parent_class;
};

GType 			 gsi_indenters_manager_get_type 	(void) G_GNUC_CONST;

GsiIndentersManager	*gsi_indenters_manager_new		(void);

void			 gsi_indenters_manager_register		(GsiIndentersManager *self,
								 const gchar *lang_id,
								 GsiIndenter *indenter);
GsiIndenter		*gsi_indenters_manager_get_indenter	(GsiIndentersManager *self,
								 const gchar *lang_id);
G_END_DECLS

#endif /* _GSI_INDENTERS_MANAGER_H */
