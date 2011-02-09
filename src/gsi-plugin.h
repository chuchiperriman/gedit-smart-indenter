/*
 * geditsmartindenter-plugin.h - Type here a short description of your plugin
 *
 * Copyright (C) 2009 - Jesús Barbero Rodríguez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __GSI_PLUGIN_H__
#define __GSI_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

G_BEGIN_DECLS

#define TYPE_GSI_PLUGIN		(gsi_plugin_get_type ())
#define GSI_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_GSI_PLUGIN, GsiPlugin))
#define GSI_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_GSI_PLUGIN, GsiPluginClass))
#define IS_GSI_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_GSI_PLUGIN))
#define IS_GSI_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_GSI_PLUGIN))
#define GSI_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_GSI_PLUGIN, GsiPluginClass))

typedef struct _GsiPluginPrivate	GsiPluginPrivate;
typedef struct _GsiPlugin		GsiPlugin;
typedef struct _GsiPluginClass		GsiPluginClass;

struct _GsiPlugin
{
	PeasExtensionBase parent_instance;

	GsiPluginPrivate *priv;
};

struct _GsiPluginClass
{
	PeasExtensionBaseClass parent_class;
};

GType	gsi_plugin_get_type	(void) G_GNUC_CONST;

G_MODULE_EXPORT void	peas_register_types	(PeasObjectModule *module);

G_END_DECLS

#endif /* __GSI_PLUGIN_H__ */

