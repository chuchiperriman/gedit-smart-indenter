/* gsi-indenter-utils.c */

#include "gsi-indenter-utils.h"

/* Copied from gtksourceview. gtksourceview doesn't free the returned text */
gchar*
gsi_indenter_utils_get_line_indentation (GtkTextBuffer	*buffer,
					 gint		 line)
{
        GtkTextIter start;
        GtkTextIter end;

        gunichar ch;

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

        return gtk_text_iter_get_slice (&start, &end);
}

gboolean
gsi_indenter_utils_move_to_no_space (GtkTextIter *iter,
				     gint direction)
{
	gunichar c;
	gboolean moved = TRUE;
	
	g_return_val_if_fail (iter != NULL, FALSE);
	
	c = gtk_text_iter_get_char (iter);
	
	while (g_unichar_isspace (c))
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


