/* gsi-indenter-utils.c */

#include "gsi-indenter-utils.h"

#include <string.h>

gint
gsi_indenter_utils_view_get_real_indent_width (GtkSourceView *view)
{
	gint indent_width = gtk_source_view_get_indent_width (view);
	guint tab_width = gtk_source_view_get_tab_width (view);

	return indent_width < 0 ? tab_width : indent_width;
}

gint
gsi_indenter_utils_get_amount_indents (GtkTextView *view,
					GtkTextIter *cur)
{
	GtkTextIter start;
	gunichar c;
	
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), 0);
	g_return_val_if_fail (cur != NULL, 0);
	
	start = *cur;
	gtk_text_iter_set_line_offset (&start, 0);
	
	c = gtk_text_iter_get_char (&start);
	
	while (g_unichar_isspace (c) &&
	       c != '\n' &&
	       c != '\r')
	{
		if (!gtk_text_iter_forward_char (&start))
			break;
		
		c = gtk_text_iter_get_char (&start);
	}

	return gsi_indenter_utils_get_amount_indents_from_position (view, &start);
}

gint
gsi_indenter_utils_get_amount_indents_from_position (GtkTextView *view,
						     GtkTextIter *cur)
{
	gint indent_width;
	GtkTextIter start;
	gunichar c;
	gint amount = 0;
	gint rest = 0;
	
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), 0);
	g_return_val_if_fail (cur != NULL, 0);
	
	indent_width = gsi_indenter_utils_view_get_real_indent_width (GTK_SOURCE_VIEW (view));
	
	start = *cur;
	gtk_text_iter_set_line_offset (&start, 0);
	
	c = gtk_text_iter_get_char (&start);
	
	while (gtk_text_iter_compare (&start, cur) < 0)
	{
		if (c == '\t')
		{
			if (rest != 0)
				rest = 0;
			amount += indent_width;
		}
		else
		{
			rest++;
		}
		
		if (rest == indent_width)
		{
			amount += indent_width;
			rest = 0;
		}
		
		if (!gtk_text_iter_forward_char (&start))
			break;
		
		c = gtk_text_iter_get_char (&start);
	}
	
	return amount + rest;
}

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
	
	/*+1 Is the pair,by example (*/
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
gsi_indenter_utils_iter_backward_line_not_empty (GtkTextIter *iter)
{
	gunichar c;

	gtk_text_iter_set_line_offset (iter, 0);
	
	while (gtk_text_iter_backward_char (iter))
	{
		c = gtk_text_iter_get_char (iter);

		if (!g_unichar_isspace (c))
		{
			gtk_text_iter_set_line_offset (iter, 0);
			return TRUE;
		}
	}
		
	return FALSE;
}

/*TODO Check blanks and tabs*/
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


gboolean
gsi_indenter_utils_move_to_no_comments (GtkTextIter *iter)
{
	gunichar c;
	
	g_return_val_if_fail (iter != NULL, FALSE);
	
	c = gtk_text_iter_get_char (iter);
	
	if (c == '/' && gtk_text_iter_backward_char (iter))
	{
		c = gtk_text_iter_get_char (iter);
		
		if (c == '*')
		{
			/*
			 * We look backward for '*' '/'
			 */
			for (;;)
			{
				if (!gtk_text_iter_backward_char (iter))
					return FALSE;
				c = gtk_text_iter_get_char (iter);
				
				if (c == '*')
				{
					if (!gtk_text_iter_backward_char (iter))
						return FALSE;
					c = gtk_text_iter_get_char (iter);
					
					if (c == '/')
					{
						/*
						 * We reached to the beggining of the comment,
						 * now we have to look backward for non spaces
						 */
						if (!gtk_text_iter_backward_char (iter))
							return FALSE;
						c = gtk_text_iter_get_char (iter);
						
						while (g_unichar_isspace (c) && gtk_text_iter_backward_char (iter))
						{
							c = gtk_text_iter_get_char (iter);
						}
						
						break;
					}
				}
			}
		}
	}
	
	return TRUE;
}

gboolean
gsi_indenter_utils_move_to_no_preprocessor (GtkTextIter *iter)
{
	gunichar c;
	GtkTextIter copy;
	gboolean moved = TRUE;
	
	copy = *iter;
	
	gtk_text_iter_set_line_offset (&copy, 0);
	gsi_indenter_utils_move_to_no_space (&copy, 1, TRUE);
	
	c = gtk_text_iter_get_char (&copy);
	
	if (c == '#')
	{
		/*
		 * Move back until we get a no space char
		 */
		do
		{
			if (!gtk_text_iter_backward_char (&copy))
				moved = FALSE;
			c = gtk_text_iter_get_char (&copy);
		} while (g_unichar_isspace (c));
		
		*iter = copy;
	}
	else
	{
		moved = FALSE;
	}
	
	return moved;
}

gboolean
gsi_indenter_utils_find_open_char (GtkTextIter *iter,
				   gchar open,
				   gchar close,
				   gboolean skip_first)
{
	GtkSourceBuffer *buffer;
	GtkTextIter copy;
	gunichar c;
	gboolean moved = FALSE;
	gint counter = 0;
	
	g_return_val_if_fail (iter != NULL, FALSE);
	
	copy = *iter;
	buffer = GTK_SOURCE_BUFFER(gtk_text_iter_get_buffer(iter));
	
	/*
	 * FIXME: We have to take care of number of lines to go back
	 */
	c = gtk_text_iter_get_char (&copy);
	do
	{

		//TODO Fix single char like '{'
		
		if (gtk_source_buffer_iter_has_context_class(buffer, &copy, "string") ||
		    gtk_source_buffer_iter_has_context_class(buffer, &copy, "comment") ||
		    gtk_source_buffer_iter_has_context_class(buffer, &copy, "char"))
		{
			continue;
		}
		/*
		 * This algorithm has to work even if we have if (xxx, xx(),
		 */
		if (c == close || skip_first)
                {
                        counter--;
                        skip_first = FALSE;
                }

                if (c == open && counter != 0)
                {
                        counter++;
                }

                if (counter == 0)
                {
                        *iter = copy;
                        moved = TRUE;
                        break;
                }
	}
	while (gtk_text_iter_backward_char (&copy) &&
	       (c = gtk_text_iter_get_char (&copy)));
	
	return moved;
}

gchar*
gsi_indenter_utils_get_line_text (GtkTextBuffer *buffer, GtkTextIter *iter)
{
	GtkTextIter start, end;
	gchar c;
		
	start = *iter;
	gtk_text_iter_set_line_offset (&start, 0);
	c = gtk_text_iter_get_char (&start);
	if (c == '\r' || c == '\n')
		return "";

	end = start;
	gtk_text_iter_forward_to_line_end (&end);
	
	return gtk_text_buffer_get_text (buffer,
					 &start,
					 &end,
					 FALSE);
}

/*New allocated gchar*/
gchar*
gsi_indenter_utils_source_view_get_indent_text (GtkSourceView *view)
{
	gchar *indent = NULL;
	if (gtk_source_view_get_insert_spaces_instead_of_tabs (view))
	{
		indent = g_strnfill (gtk_source_view_get_tab_width (view), ' ');
	}
	else
	{
		indent = g_strdup ("\t");
	}
	return indent;
}

void
gsi_indenter_utils_indent_region_by_line (GsiIndenter *indenter,
					  GtkTextView *view,
					  GtkTextIter *start,
					  GtkTextIter *end)
{
	gint start_line = gtk_text_iter_get_line (start);
	gint end_line = gtk_text_iter_get_line (end);
	GtkTextIter iter = *start;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	
	/*We indent based on the first selected line*/
	start_line++;
	while (start_line <= end_line)
	{
		gtk_text_buffer_get_iter_at_line (buffer, &iter, start_line);
		gsi_indenter_indent_line (indenter, view, &iter);
		start_line++;
		
	}
}

gint
gsi_indenter_utils_add_indent (GtkTextView *view,
				gint current_level)
{
	gint indent_width;
	
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), 0);
	
	indent_width = gsi_indenter_utils_view_get_real_indent_width (GTK_SOURCE_VIEW (view));
	
	return current_level + indent_width;
}

gchar *
gsi_indenter_utils_get_indent_string_from_indent_level (GtkSourceView *view,
							gint level)
{
        gint tabs;
        gint spaces;
        gchar *indent = NULL;
        gboolean insert_spaces = gtk_source_view_get_insert_spaces_instead_of_tabs (view);
	gint indent_width;
	
        indent_width = gsi_indenter_utils_view_get_real_indent_width (view);
        tabs = level / indent_width;
        spaces = level % indent_width;

        if (insert_spaces)
        {
                indent = g_strnfill (indent_width * tabs + spaces, ' ');
        }
        else
        {
                indent = gsi_indenter_utils_get_indent_from_tabs (tabs, spaces);
        }

        return indent;
}
								  
