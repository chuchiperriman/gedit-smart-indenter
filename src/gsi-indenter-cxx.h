/* gsi-indenter-cxx.h */

#ifndef _GSI_INDENTER_CXX_H
#define _GSI_INDENTER_CXX_H

#include "gsi-indenter.h"

G_BEGIN_DECLS

#define GSI_TYPE_INDENTER_CXX gsi_indenter_cxx_get_type()

#define GSI_INDENTER_CXX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTER_CXX, GsiIndenterCxx))

#define GSI_INDENTER_CXX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  GSI_TYPE_INDENTER_CXX, GsiIndenterCxxClass))

#define GSI_IS_INDENTER_CXX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTER_CXX))

#define GSI_IS_INDENTER_CXX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  GSI_TYPE_INDENTER_CXX))

#define GSI_INDENTER_CXX_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  GSI_TYPE_INDENTER_CXX, GsiIndenterCxxClass))

typedef struct _GsiIndenterCxxPrivate GsiIndenterCxxPrivate;

typedef struct {
  GObject parent;
  GsiIndenterCxxPrivate *priv;
} GsiIndenterCxx;

typedef struct {
  GObjectClass parent_class;
} GsiIndenterCxxClass;

GType gsi_indenter_cxx_get_type (void);

GsiIndenter* gsi_indenter_cxx_new (void);

G_END_DECLS

#endif /* _GSI_INDENTER_CXX_H */
