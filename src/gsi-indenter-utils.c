/* gsi-indenter-utils.c */

#include "gsi-indenter-utils.h"

#include <string.h>

/* Copied from gtksourceview. gtksourceview doesn't free the returned text */
gchar*
gsi_indenter_utils_get_line_indentation (GtkTextBuffer	*buffer,
					 gint		 line,
					 gboolean	 ignore_empty_lines)
{
        GtkTextIter start;
        GtkTextIter end;
	gchar *indentation = NULL;
	
        gunichar ch;

	while (line >= 0)
	{
		if (!ignore_empty_lines || !gsi_indenter_utils_is_empty_line (buffer, line))
		{
			gtk_text_buffer_get_iter_at_line (buffer,
		                                          &start,
							  line);
			end = start;

			ch = gtk_text_iter_get_char (&end);

			while (g_unichar_isspace (ch) &&
			       (ch != '\n') &&
			       (ch != '\r'))
			{
				if (!gtk_text_iter_forward_char (&end))
				        break;

				ch = gtk_text_iter_get_char (&end);
			}

			if (gtk_text_iter_equal (&start, &end))
				return NULL;

			indentation = gtk_text_iter_get_slice (&start, &end);
			break;
		}
		line++;
	}
	
	return indentation;
}

gboolean
gsi_indenter_utils_move_to_no_space (GtkTextIter *iter,
				     gint direction,
				     gboolean ignore_new_line)
{
	gunichar c;
	gboolean moved = TRUE;
	
	g_return_val_if_fail (iter != NULL, FALSE);
	
	c = gtk_text_iter_get_char (iter);
	
	while (g_unichar_isspace (c) && (ignore_new_line || (c != '\n' && c != '\r')))
	{
		if (!gtk_text_iter_forward_chars (iter, direction))
		{
			moved = FALSE;
			break;
		}
		c = gtk_text_iter_get_char (iter);
	}
	
	return moved;
}

/* Returns new allocated string */
gchar*
gsi_indenter_utils_get_indent_from_tabs (guint tabs, guint spaces)
{
	gchar *str;

	str = g_malloc (tabs + spaces + 1);
	if (tabs > 0)
		memset (str, '\t', tabs);
	if (spaces > 0)
		memset (str + tabs, ' ', spaces);
	str[tabs + spaces] = '\0';

	return str;
}

/* Returns new allocated string */
gchar*
gsi_indenter_utils_get_indent_to_iter (GtkSourceView *view, GtkTextIter *iter)
{
	GtkTextIter start = *iter;
	guint tab_width = gtk_source_view_get_tab_width (view);
	guint total_size = 0;
	gchar *indent = NULL;
	
	gtk_text_iter_set_line_offset (&start, 0);
	do{
		if (gtk_text_iter_get_char (&start) == '\t')
			total_size += tab_width;
		else
			++total_size;
		
		gtk_text_iter_forward_char (&start);
	}while (gtk_text_iter_compare (&start, iter) != 0);
	
	/*+1 Is the pair,by example )*/
	total_size++;
	
	if (gtk_source_view_get_insert_spaces_instead_of_tabs (view))
	{
		indent = g_strnfill (total_size, ' ');
	}
	else
	{
		guint t, s;
		
		t = total_size / tab_width;
		s = total_size  % tab_width;
		
		indent = gsi_indenter_utils_get_indent_from_tabs (t, s);
	}

	return indent;
}

gboolean
gsi_indenter_utils_is_empty_line (GtkTextBuffer	*buffer,
				  gint		 line)
{
	GtkTextIter iter;
	gunichar c;
	gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
	c = gtk_text_iter_get_char (&iter);
	
	return c == '\n' || c == '\r';
}

void
gsi_indenter_utils_replace_indentation (GtkTextBuffer	*buffer,
				       gint		 line,
				       gchar		*indentation)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_iter_at_line (buffer,
					  &start,
					  line);
	
	end = start;
	
	if (gsi_indenter_utils_move_to_no_space (&end, 1, FALSE))
	{
		gtk_text_buffer_delete (buffer, &start, &end);
		if (indentation)
		{
			gtk_text_buffer_get_iter_at_line (buffer, &start, line);
			gtk_text_buffer_insert (buffer, &start, indentation, -1);
		}
	}
}

gboolean
gsi_indenter_utils_find_open_char (GtkTextIter *iter,
				   gchar open,
				   gchar close,
				   gboolean skip_first)
{
	GtkTextIter copy;
	gunichar c;
	gboolean moved = FALSE;
	gint counter = 0;
	
	g_return_val_if_fail (iter != NULL, FALSE);
	
	copy = *iter;
	
	/*
	 * FIXME: We have to take care of number of lines to go back
	 */
	c = gtk_text_iter_get_char (&copy);
	do
	{
		/*
		 * This algorithm has to work even if we have if (xxx, xx(),
		 */
		if (c == close)
		{
			if (skip_first)
				skip_first = FALSE;
			else			
				counter--;
		}
		if (c == open)
		{
			if (counter != 0)
			{
				counter++;
			}
			else
			{
				*iter = copy;
				moved = TRUE;
				break;
			}
		}
	}
	while (gtk_text_iter_backward_char (&copy) &&
	       (c = gtk_text_iter_get_char (&copy)));
	
	return moved;
}

