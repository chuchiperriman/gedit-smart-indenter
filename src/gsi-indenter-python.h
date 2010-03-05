/* gsi-indenter-python.h */

#ifndef _GSI_INDENTER_PYTHON_H
#define _GSI_INDENTER_PYTHON_H

#include "gsi-indenter.h"

G_BEGIN_DECLS

#define GSI_TYPE_INDENTER_PYTHON gsi_indenter_python_get_type()

#define GSI_INDENTER_PYTHON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  GSI_TYPE_INDENTER_PYTHON, GsiIndenterPython))

#define GSI_INDENTER_PYTHON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  GSI_TYPE_INDENTER_PYTHON, GsiIndenterPythonClass))

#define GSI_IS_INDENTER_PYTHON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  GSI_TYPE_INDENTER_PYTHON))

#define GSI_IS_INDENTER_PYTHON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  GSI_TYPE_INDENTER_PYTHON))

#define GSI_INDENTER_PYTHON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  GSI_TYPE_INDENTER_PYTHON, GsiIndenterPythonClass))

typedef struct {
  GObject parent;
} GsiIndenterPython;

typedef struct {
  GObjectClass parent_class;
} GsiIndenterPythonClass;

GType gsi_indenter_python_get_type (void);

GsiIndenter* gsi_indenter_python_new (void);

G_END_DECLS

#endif /* _GSI_INDENTER_PYTHON_H */
