/* gsi-indenter-c.h */

#ifndef _GSI_INDENTER_C_H
#define _GSI_INDENTER_C_H

#include "gsi-indenter.h"

G_BEGIN_DECLS

#define GSI_TYPE_INDENTER_C gsi_indenter_c_get_type()

#define GSI_INDENTER_C(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTER_C, GsiIndenterC))

#define GSI_INDENTER_C_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  GSI_TYPE_INDENTER_C, GsiIndenterCClass))

#define GSI_IS_INDENTER_C(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTER_C))

#define GSI_IS_INDENTER_C_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  GSI_TYPE_INDENTER_C))

#define GSI_INDENTER_C_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  GSI_TYPE_INDENTER_C, GsiIndenterCClass))

typedef struct {
  GObject parent;
} GsiIndenterC;

typedef struct {
  GObjectClass parent_class;
} GsiIndenterCClass;

GType gsi_indenter_c_get_type (void);

GsiIndenter* gsi_indenter_c_new (void);

G_END_DECLS

#endif /* _GSI_INDENTER_C_H */
