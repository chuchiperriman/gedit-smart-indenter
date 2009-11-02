/* gsi-indenter.h */

#ifndef _GSI_INDENTER_UTILS_H
#define _GSI_INDENTER_UTILS_H 

#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>

G_BEGIN_DECLS

gchar		*gsi_indenter_utils_get_line_indentation	(GtkTextBuffer	*buffer,
								 gint		 line);

gboolean	 gsi_indenter_utils_move_to_no_space		(GtkTextIter *iter,
								 gint direction,
								 gboolean ignore_new_line);

gchar		*gsi_indenter_utils_get_indent_from_tabs	(guint tabs,
								 guint spaces);

gchar		*gsi_indenter_utils_get_indent_to_iter		(GtkSourceView *view, 
								 GtkTextIter *iter);

G_END_DECLS

#endif /* _GSI_INDENTER_UTILS_H */
