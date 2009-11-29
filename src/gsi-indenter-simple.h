/* gsi-indenter-simple.h */

#ifndef _GSI_INDENTER_SIMPLE_H
#define _GSI_INDENTER_SIMPLE_H

#include "gsi-indenter.h"

G_BEGIN_DECLS

#define GSI_TYPE_INDENTER_SIMPLE gsi_indenter_simple_get_type()

#define GSI_INDENTER_SIMPLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTER_SIMPLE, GsiIndenterSimple))

#define GSI_INDENTER_SIMPLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  GSI_TYPE_INDENTER_SIMPLE, GsiIndenterSimpleClass))

#define GSI_IS_INDENTER_SIMPLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTER_SIMPLE))

#define GSI_IS_INDENTER_SIMPLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  GSI_TYPE_INDENTER_SIMPLE))

#define GSI_INDENTER_SIMPLE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  GSI_TYPE_INDENTER_SIMPLE, GsiIndenterSimpleClass))

typedef struct {
  GObject parent;
} GsiIndenterSimple;

typedef struct {
  GObjectClass parent_class;
} GsiIndenterSimpleClass;

GType gsi_indenter_simple_get_type (void);

GsiIndenter* gsi_indenter_simple_new (void);

G_END_DECLS

#endif /* _GSI_INDENTER_SIMPLE_H */
