/* gsi-indenter-simple.c */

#include "gsi-indenter-simple.h"

#define INDENTER_SIMPLE_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_SIMPLE, GsiIndenterSimplePrivate))

typedef struct _GsiIndenterSimplePrivate GsiIndenterSimplePrivate;

struct _GsiIndenterSimplePrivate
{
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterSimple,
                         gsi_indenter_simple,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))

static void
gsi_indenter_indent_new_line_impl (GsiIndenter *indenter,
				   GtkTextView *view,
				   GtkTextIter *iter)
{
	
}

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
        iface->indent_new_line = gsi_indenter_indent_new_line_impl;
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
