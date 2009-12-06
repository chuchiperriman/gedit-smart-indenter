/* gsi-indenter-c.c */

#include <string.h>
#include "gsi-indenter-c.h"
#include "gsi-indenter-utils.h"

#define GSI_INDENTER_C_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_C, GsiIndenterCPrivate))
  
typedef struct _GsiIndenterCPrivate GsiIndenterCPrivate;

struct _GsiIndenterCPrivate
{
	gboolean dummy;
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterC,
                         gsi_indenter_c,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))

static const gchar * regexes[] =
{
	"^\\s*(if|while|else if|for|switch)\\s*\\(.*\\)\\s*$",
	"^\\s*(else|do)\\s*$",
	NULL
};

static const gchar * case_regexes[] =
{
	"^\\s*(default|case [^ ]*)\\s*:\\s*(if|while|else if|for|switch)\\s*\\(.*\\)\\s*$",
	"^\\s*(default|case [^ ]*)\\s*:\\s*do\\s*$",
	NULL
};

typedef struct
{
	gint level;
	gchar *append;
} IndentData;

static gboolean
match_regexes (GtkTextIter *iter,
	       const gchar * const *regexes)
{
	gint i = 0;
	gboolean match = FALSE;
	gchar *string;
	GtkTextIter start;
	
	start = *iter;
	
	gsi_indenter_utils_find_open_char (&start, '(', ')', FALSE);
	
	gtk_text_iter_set_line_offset (&start, 0);
	gtk_text_iter_forward_char (iter);
	
	string = gtk_text_iter_get_text (&start, iter);
	gtk_text_iter_backward_char (iter);
	
	while (regexes[i] != NULL)
	{
		GRegex *regex;
		GMatchInfo *match_info;
		
		regex = g_regex_new (regexes[i], G_REGEX_DOTALL, 0, NULL);
		g_regex_match (regex, string, 0, &match_info);
		
		if (g_match_info_matches (match_info))
			match = TRUE;
		
		g_match_info_free (match_info);
		g_regex_unref (regex);
		
		if (match)
		{
			break;
		}
		i++;
	}
	
	g_free (string);
	
	return match;
}

static gboolean
is_caselabel (const gchar *label)
{
	gboolean is_case = FALSE;
	
	if (g_str_has_prefix (label, "case") ||
	    g_str_has_prefix (label, "default"))
		is_case = TRUE;
	
	return is_case;
}

static void
clean_line_to_regexp (gchar *text)
{
	guint i;
	gboolean quotes = FALSE;
	gboolean dquotes = FALSE;
	
	if (text == NULL) 
		return;
	
	for (i = 0; text[i]; i++)
	{
		if (text[i] == '\\' && text[i+1])
		{
			if (text[i+1] == '\\')
			{
				text[i] = ' ';
				text[i + 1] = ' ';
				continue;
			}
			else if (text[i+1] == '\'' || text[i+1] == '"')
			{
				text[i] = ' ';
				text[i + 1] = ' ';
				continue;
			}
		}
		
		if (!quotes && text[i] == '"')
		{
			dquotes = !dquotes;
			continue;
		}
		
		if (!dquotes && text[i] == '\'')
		{
			quotes = !quotes;
			continue;
		}
		
		if (quotes || dquotes)
		{
			text[i] = ' ';
		}
	}
}

static void
move_to_no_simple_comment (GtkTextIter *iter)
{
	gunichar c;
	gunichar cprev = '\0';
	GtkTextIter copy = *iter;
	gboolean in_multi = FALSE;
	
	gtk_text_iter_set_line_offset (&copy, 0);
	
	c = gtk_text_iter_get_char (&copy);
	do
	{
		if (c == '/')
		{
			if (in_multi && cprev == '*')
			{
				in_multi = FALSE;
			}
			else
			{
				if (!in_multi)
				{
					if (!gtk_text_iter_forward_char (&copy))
						break;
					
					cprev = c;
					c = gtk_text_iter_get_char (&copy);
					
					if (c == '/')
					{
						gtk_text_iter_backward_char (&copy);
						gtk_text_iter_backward_char (&copy);
						break;
					}
					if (c == '*')
					{
						in_multi = TRUE;
					}
				}
			}
		}
		if (!gtk_text_iter_forward_char (&copy))
			break;
		cprev = c;
		c = gtk_text_iter_get_char (&copy);
	} while (c != '\n' && c != '\r');
	
	*iter = copy;
}

static gchar*
get_line_text (GtkTextBuffer *buffer, GtkTextIter *iter)
{
	GtkTextIter start, end;
	gchar c;
	gchar *text = NULL;
	
	start = *iter;
	gtk_text_iter_set_line_offset (&start, 0);
	c = gtk_text_iter_get_char (&start);
	if (c == '\r' || c == '\n')
		return "";

	end = start;

	/*Ignore the "//comment" part: if (TRUE) //comment */
	
	move_to_no_simple_comment (&end);
	
	text = gtk_text_buffer_get_text (buffer,
					 &start,
					 &end,
					 FALSE);

	clean_line_to_regexp (text);
	
	return text;
}

static gboolean
find_char_inline (GtkTextIter *iter,
		  gunichar c)
{
	gunichar f;
	gboolean found = FALSE;
	
	f = gtk_text_iter_get_char (iter);
	
	while (f != c && gtk_text_iter_get_line_offset (iter) != 0)
	{
		gtk_text_iter_backward_char (iter);
		f = gtk_text_iter_get_char (iter);
	}
	
	if (f == c)
	{
		found = TRUE;
	}
	
	return found;
}

static gboolean
process_multi_comment (GtkTextView *view,
		       GtkTextIter *iter,
		       IndentData *idata,
		       gboolean first)
{
	GtkTextIter copy = *iter;
	gchar *line_text;
	GtkTextBuffer *buffer;
	gboolean res = FALSE;
	
	buffer = gtk_text_view_get_buffer (view);
	
	line_text = get_line_text (buffer, &copy);

	if (g_regex_match_simple (".*\\/\\*(?!.*\\*\\/)",
				  line_text,
				  0,
				  0))
	{
		idata->level = gsi_indenter_utils_get_amount_indents (view,
								&copy);
		idata->level++;
		idata->append = g_strdup ("* ");
		res = TRUE;
	}
	
	if (!res)
	{
		gchar *text = get_line_text (buffer, &copy);
		if (g_regex_match_simple ("^\\s*\\*(?!\\/)(?!.*\\*\\/)",
					  text,
					  0,
					  0))
		{
			if (gsi_indenter_utils_iter_backward_line_not_empty (&copy))
			{
				res = process_multi_comment (view, &copy, idata, FALSE);
				//idata->append = g_strdup ("* ");
			}
		}
	}

	return res;
}

static gboolean
c_indenter_get_indentation_level (GsiIndenter *indenter,
				  GtkTextView *view,
				  GtkTextIter *cur,
				  IndentData *idata)
{
	/*
	 * The idea of this algorithm is just move the iter to the right position
	 * and manage the iter to get a context to get the right amount of indents
	 */
	GtkTextIter iter;
	
	gunichar c;
	gint amount = -1;
	gint original_line = gtk_text_iter_get_line (cur);
	
	iter = *cur;
	
	if (gsi_indenter_utils_iter_backward_line_not_empty (&iter))
	{
		if (process_multi_comment (view, &iter, idata, TRUE))
		{
			return TRUE;
		}
	}
	iter = *cur;
		
	/*
	 * Move to the start line because the <control>j can be
	 * pressed in the middle of a line
	 */
	gtk_text_iter_set_line_offset (&iter, 0);
	if (!gsi_indenter_utils_move_to_no_space (&iter, 1, FALSE))
		return FALSE;

	c = gtk_text_iter_get_char (&iter);
	
	/* # is always indent 0. Example: #ifdef */
	if (c == '#')
		return FALSE;
	
	/* Skip all preprocessor sentences */
	//while (!relocating && gsi_indenter_utils_move_to_no_preprocessor (&iter))
	while (gsi_indenter_utils_move_to_no_preprocessor (&iter))
		continue;
	
	if (!gsi_indenter_utils_move_to_no_space (&iter, -1, TRUE))
		return FALSE;

	/*
	 * Check for comments
	 */
	if (!gsi_indenter_utils_move_to_no_comments (&iter))
		return FALSE;
	
	c = gtk_text_iter_get_char (&iter);
	
	if (c == '*')
	{
		gunichar ch;
		
		/*
		 * We are in a comment
		 */
		
		amount = gsi_indenter_utils_get_amount_indents (view,
								&iter);
		
		/* We are in the case "/ *" so we have to add an space */
		gtk_text_iter_backward_char (&iter);
		ch = gtk_text_iter_get_char (&iter);
		
		if (ch == '/')
		{
			amount++;
		}
	}
	else if (c == ';')
	{
		GtkTextIter copy;
		
		copy = iter;
		
		/*
		 * We have to check that we are not in something like:
		 * hello (eoeo,
		 *        eoeo);
		 */
		if (find_char_inline (&copy, ')') &&
		    gsi_indenter_utils_find_open_char (&copy, '(', ')', FALSE))
		{
			amount = gsi_indenter_utils_get_amount_indents (view,
									 &copy);
			
			/*
			 * We have to check if we are in just one line block
			 */
			while (!gtk_text_iter_ends_line (&copy) &&
			       gtk_text_iter_backward_char (&copy))
				continue;
		
			gtk_text_iter_backward_char (&copy);
			
			if (match_regexes (&copy, regexes))
			{
				gsi_indenter_utils_find_open_char (&copy, '(', ')', FALSE);
			
				amount = gsi_indenter_utils_get_amount_indents (view,
										 &copy);
			}
		}
		else
		{
			amount = gsi_indenter_utils_get_amount_indents (view,
									 &iter);
			
			/*
			 * We have to check if we are in just one line block
			 */
			while (!gtk_text_iter_ends_line (&iter) &&
			       gtk_text_iter_backward_char (&iter))
				continue;
		
			gtk_text_iter_backward_char (&iter);
		
			if (match_regexes (&iter, regexes))
			{
				gsi_indenter_utils_find_open_char (&iter, '(', ')', FALSE);
			
				amount = gsi_indenter_utils_get_amount_indents (view,
										 &iter);
			}
		}
	}
	else if (c == '}')
	{
		amount = gsi_indenter_utils_get_amount_indents (view,
								&iter);
		
		/*
		 * We need to look backward for {.
		 * FIXME: We need to set a number of lines to look backward.
		 */
		//if (relocating && gsi_indenter_utils_find_open_char (&iter, '{', '}', FALSE))
		if (gsi_indenter_utils_find_open_char (&iter, '{', '}', FALSE))
		{
			amount = gsi_indenter_utils_get_amount_indents (view,
									&iter);
		}
	}
	else if (c == '{')
	{
		amount = gsi_indenter_utils_get_amount_indents (view,
								&iter);
		

		/*Is a relocation because the user writes { or control+j*/
		if (original_line == gtk_text_iter_get_line (&iter))
		{
			/*
			 * Check that the previous line match regexes
			 */
			while (gtk_text_iter_backward_char (&iter) &&
			       !gtk_text_iter_ends_line (&iter))
			{
				continue;
			}
			
			gtk_text_iter_backward_char (&iter);
			
			if (match_regexes (&iter, regexes))
			{
				gsi_indenter_utils_find_open_char (&iter, '(', ')', FALSE);
				
				amount = gsi_indenter_utils_get_amount_indents (view,
										&iter);
			}
			else if (match_regexes (&iter, case_regexes))
			{
				gunichar ch;
				
				/* We are in a case label like: case 0: if (hello)
				 * so we first look backward for the ':' and then look forward
				 * for the first char
				 */
				find_char_inline (&iter, ':');
				gtk_text_iter_forward_char (&iter);
				ch = gtk_text_iter_get_char (&iter);
		
				while (g_unichar_isspace (ch))
				{
					gtk_text_iter_forward_char (&iter);
					ch = gtk_text_iter_get_char (&iter);
				}
		
				amount = gsi_indenter_utils_get_amount_indents_from_position (view, &iter);
			}
			else
			{
				GtkTextIter start;
				gchar *label;
	
				start = iter;
				gtk_text_iter_set_line_offset (&start, 0);
				gsi_indenter_utils_move_to_no_space (&start, 1, TRUE);
	
				label = gtk_text_iter_get_text (&start, &iter);
				
				if (is_caselabel (label))
				{
					amount = gsi_indenter_utils_get_amount_indents (view, &iter);
				}
				
				g_free (label);
			}
		}
		else
		{
			/*Is a line after the braket*/
			amount = gsi_indenter_utils_add_indent (view, amount);
		}
	}
	else if (c == ',' || c == '&' || c == '|')
	{
		GtkTextIter s;
		
		amount = gsi_indenter_utils_get_amount_indents (view,
								 &iter);
		
		s = iter;
		if (gsi_indenter_utils_find_open_char (&s, '(', ')', TRUE))
		{
			amount = gsi_indenter_utils_get_amount_indents_from_position (view, &s);
			amount++;
		}
	}
	/*
	else if (c == ')' && relocating)
	//else if (c == ')')
	{
		amount = gsi_indenter_utils_get_amount_indents (view,
								 &iter);
		
		if (gsi_indenter_utils_find_open_char (&iter, '(', ')', FALSE))
		{
			amount = gsi_indenter_utils_get_amount_indents_from_position (view, &iter);
		}
	}
	*/
	else if (c == ':')
	{
		/*
		GtkTextIter start;
		gchar *label;
		
		start = iter;
		gtk_text_iter_set_line_offset (&start, 0);
		gsi_indenter_utils_move_to_no_space (&start, 1, TRUE);
		
		label = gtk_text_iter_get_text (&start, &iter);
		
		if (relocating)
		{
			if (!is_caselabel (label))
			{
				amount = 0;
			}
			else
			{
				amount = gsi_indenter_utils_get_amount_indents (view, &iter);
			}
		}
		else
		{
			if (is_caselabel (label))
			{
				amount = gsi_indenter_utils_get_amount_indents (view, &iter);
				amount = gsi_indenter_utils_add_indent (view, amount);
			}
		}
		
		g_free (label);
		*/
	}
	else if (match_regexes (&iter, regexes))
	{
		gsi_indenter_utils_find_open_char (&iter, '(', ')', FALSE);
		
		amount = gsi_indenter_utils_get_amount_indents (view, &iter);
		amount = gsi_indenter_utils_add_indent (view, amount);
	}
	else if (match_regexes (&iter, case_regexes))
	{
		gunichar ch;
		
		/* We are in a case label like: case 0: if (hello)
		 * so we first look backward for the ':' and then look forward
		 * for the first char
		 */
		find_char_inline (&iter, ':');
		gtk_text_iter_forward_char (&iter);
		ch = gtk_text_iter_get_char (&iter);
		
		while (g_unichar_isspace (ch))
		{
			gtk_text_iter_forward_char (&iter);
			ch = gtk_text_iter_get_char (&iter);
		}
		
		amount = gsi_indenter_utils_get_amount_indents_from_position (view, &iter);
		amount = gsi_indenter_utils_add_indent (view, amount);
	}
	else
	{
		GtkTextIter copy;
		
		amount = gsi_indenter_utils_get_amount_indents (view, &iter);
		
		copy = iter;
		
		/*
		 * We tried all cases, so now look if we are at the end of a
		 * func declaration
		 */
		if (c == ')')
		{
			if (gsi_indenter_utils_find_open_char (&copy, '(', ')',
								FALSE))
			{
				amount = gsi_indenter_utils_get_amount_indents (view,
										 &copy);
			}
		}
		/*
		 * Are we in something like: if (hello\n
		 */
		else if (gsi_indenter_utils_find_open_char (&copy, '(', ')',
							     TRUE))
		{
			amount = gsi_indenter_utils_get_amount_indents_from_position (view,
										       &copy);
			amount += 1;
		}
	}
	idata->level = amount;
	return amount != -1;
}

static gboolean
gsi_indenter_indent_line_real (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	gchar *indent;
	GtkTextBuffer *buffer;
	gboolean res = FALSE;
	IndentData idata;

	idata.level = 0;
	idata.append = NULL;
	
	buffer = gtk_text_view_get_buffer (view);
	res = c_indenter_get_indentation_level (indenter,
						view,
						iter,
						&idata);

	if (res)
	{
		gtk_text_buffer_begin_user_action (buffer);
		res = FALSE;
		if (idata.level > 0)
		{
			indent = gsi_indenter_utils_get_indent_string_from_indent_level
   					(GTK_SOURCE_VIEW (view), 
					 idata.level);
			if (indent)
			{
				gsi_indenter_utils_replace_indentation (buffer,
									gtk_text_iter_get_line (iter),
									indent);
				g_free (indent);
				res = TRUE;
			}
		}
		if (idata.append != NULL)
		{
			gtk_text_buffer_insert_at_cursor (buffer,
							  idata.append,
							  -1);
			g_free (idata.append);
			res = TRUE;
		}
		gtk_text_buffer_end_user_action (buffer);
	}
	return res;
}

static void
gsi_indenter_indent_line_impl (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	gsi_indenter_indent_line_real (indenter, view, iter);
}

static const gchar*
gsi_indenter_get_relocators_impl (GsiIndenter	*self,
			     	  GtkTextView	*view)
{
	return "{}:#";
}

/*
static gboolean
gsi_indenter_relocate_impl (GsiIndenter	*self,
			    GtkTextView	*view,
			    GtkTextIter	*iter,
			    gchar	 relocator)
{
	return gsi_indenter_indent_line_real (self, view, iter, TRUE);
}
*/

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
        iface->indent_line = gsi_indenter_indent_line_impl;
	iface->get_relocators = gsi_indenter_get_relocators_impl;
	//iface->relocate = gsi_indenter_relocate_impl;
}

static void
gsi_indenter_c_dispose (GObject *object)
{
	G_OBJECT_CLASS (gsi_indenter_c_parent_class)->dispose (object);
}

static void
gsi_indenter_c_class_init (GsiIndenterCClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (GsiIndenterCPrivate));
	
	object_class->dispose = gsi_indenter_c_dispose;
}

static void
gsi_indenter_c_init (GsiIndenterC *self)
{
}

GsiIndenter*
gsi_indenter_c_new (void)
{
	return g_object_new (GSI_TYPE_INDENTER_C, NULL);
}
