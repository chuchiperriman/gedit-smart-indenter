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
 */

#ifndef __GEDITSMARTINDENTER_PLUGIN_H__
#define __GEDITSMARTINDENTER_PLUGIN_H__

#include <glib.h>
#include <glib-object.h>
#include <gedit/gedit-plugin.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define TYPE_GEDITSMARTINDENTER_PLUGIN		(geditsmartindenter_plugin_get_type ())
#define GEDITSMARTINDENTER_PLUGIN(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPlugin))
#define GEDITSMARTINDENTER_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPluginClass))
#define IS_GEDITSMARTINDENTER_PLUGIN(o)	(G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_GEDITSMARTINDENTER_PLUGIN))
#define IS_GEDITSMARTINDENTER_PLUGIN_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), TYPE_GEDITSMARTINDENTER_PLUGIN))
#define GEDITSMARTINDENTER_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_GEDITSMARTINDENTER_PLUGIN, GeditsmartindenterPluginClass))

/* Private structure type */
typedef struct _GeditsmartindenterPluginPrivate	GeditsmartindenterPluginPrivate;

/*
 * Main object structure
 */
typedef struct _GeditsmartindenterPlugin		GeditsmartindenterPlugin;

struct _GeditsmartindenterPlugin
{
	GeditPlugin parent_instance;

	/*< private >*/
	GeditsmartindenterPluginPrivate *priv;
};

/*
 * Class definition
 */
typedef struct _GeditsmartindenterPluginClass	GeditsmartindenterPluginClass;

struct _GeditsmartindenterPluginClass
{
	GeditPluginClass parent_class;
};

/*
 * Public methods
 */
GType	geditsmartindenter_plugin_get_type	(void) G_GNUC_CONST;

/* All the plugins must implement this function */
G_MODULE_EXPORT GType register_gedit_plugin (GTypeModule *module);

G_END_DECLS

#endif /* __GEDITSMARTINDENTER_PLUGIN_H__ */
