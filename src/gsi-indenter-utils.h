/* gsi-indenter.h */

#ifndef _GSI_INDENTER_UTILS_H
#define _GSI_INDENTER_UTILS_H 

#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>

#include "gsi-indenter.h"

G_BEGIN_DECLS

gint		 gsi_indenter_utils_view_get_real_indent_width	(GtkSourceView *view);

gchar		*gsi_indenter_utils_get_line_indentation	(GtkTextBuffer	*buffer,
								 gint		 line,
								 gboolean	 ignore_empty_lines);

gchar		*gsi_indenter_utils_get_indent_from_tabs	(guint tabs,
								 guint spaces);

gchar		*gsi_indenter_utils_get_indent_to_iter		(GtkSourceView *view, 
								 GtkTextIter *iter);

gboolean	 gsi_indenter_utils_is_empty_line		(GtkTextBuffer	*buffer,
								 gint		 line);

void		 gsi_indenter_utils_replace_indentation		(GtkTextBuffer	*buffer,
								 gint		 line,
								 gchar		*indentation);

gboolean	 gsi_indenter_utils_find_open_char		(GtkTextIter *iter,
								 gchar open,
				  				 gchar close,
								 gboolean skip_first,
								 const gchar *stopchars);

gchar		*gsi_indenter_utils_get_line_text		(GtkTextBuffer *buffer,
								 GtkTextIter *iter);

gchar		*gsi_indenter_utils_source_view_get_indent_text (GtkSourceView *view);

void		 gsi_indenter_utils_indent_region_by_line	(GsiIndenter *indenter,
								 GtkTextView *view,
								 GtkTextIter *start,
								 GtkTextIter *end);

gboolean	 gsi_indenter_utils_iter_backward_line_not_empty (GtkTextIter *iter);

gboolean	 gsi_indenter_utils_move_to_no_space		(GtkTextIter *iter,
								 gint direction,
								 gboolean ignore_new_line);

gboolean	 gsi_indenter_utils_move_to_no_comments		(GtkTextIter *iter);

gboolean	 gsi_indenter_utils_move_to_no_preprocessor	(GtkTextIter *iter);

gint		 gsi_indenter_utils_get_amount_indents		(GtkTextView *view,
								 GtkTextIter *cur);

gint		 gsi_indenter_utils_get_amount_indents_from_position
								(GtkTextView *view,
								 GtkTextIter *cur);

gint		 gsi_indenter_utils_add_indent			(GtkTextView *view,
								 gint current_level);

gchar		 *gsi_indenter_utils_get_indent_string_from_indent_level 
								(GtkSourceView *view,
								 gint level);

G_END_DECLS

#endif /* _GSI_INDENTER_UTILS_H */
