/* gsi-indenters-manager.c */

#include "gsi-indenters-manager.h"

G_DEFINE_TYPE (GsiIndentersManager, gsi_indenters_manager, G_TYPE_OBJECT)

#define INDENTERS_MANAGER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTERS_MANAGER, GsiIndentersManagerPrivate))

struct _GsiIndentersManagerPrivate
{
	GHashTable *indenters;
};

static void
gsi_indenters_manager_dispose (GObject *object)
{
	GsiIndentersManager *self = GSI_INDENTERS_MANAGER (object);
	
	g_hash_table_destroy (self->priv->indenters);
	
	G_OBJECT_CLASS (gsi_indenters_manager_parent_class)->dispose (object);
}

static void
gsi_indenters_manager_class_init (GsiIndentersManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GsiIndentersManagerPrivate));

	object_class->dispose = gsi_indenters_manager_dispose;
}

static void
gsi_indenters_manager_init (GsiIndentersManager *self)
{
	self->priv->indenters = g_hash_table_new_full (g_str_hash,
						       g_str_equal,
						       g_free,
						       g_object_unref);
}

GsiIndentersManager*
gsi_indenters_manager_new (void)
{
	return g_object_new (GSI_TYPE_INDENTERS_MANAGER, NULL);
}

void
gsi_indenters_manager_register (GsiIndentersManager *self,
				const gchar *lang_id,
				GsiIndenter *indenter)
{
	g_object_ref (indenter);
	g_hash_table_insert (self->priv->indenters,
			     g_strdup (lang_id),
			     indenter);
}

GsiIndenter*
gsi_indenters_manager_get_indenter (GsiIndentersManager *self,
				    const gchar *lang_id)
{
	return (GsiIndenter*)g_hash_table_lookup (self->priv->indenters, lang_id);
}





