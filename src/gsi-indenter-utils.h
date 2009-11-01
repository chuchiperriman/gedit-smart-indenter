/* gsi-indenter.h */

#ifndef _GSI_INDENTER_UTILS_H
#define _GSI_INDENTER_UTILS_H 

#include <gtk/gtk.h>

G_BEGIN_DECLS

gchar		*gsi_indenter_utils_get_line_indentation	(GtkTextBuffer	*buffer,
								 gint		 line);

gboolean	 gsi_indenter_utils_move_to_no_space		(GtkTextIter *iter,
								 gint direction);

G_END_DECLS

#endif /* _GSI_INDENTER_UTILS_H */
