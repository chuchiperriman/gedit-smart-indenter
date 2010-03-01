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
is_end_sentence(gunichar ch, gpointer user_data)
{
	return ch == ';' || ch == ')' || ch == '{' || ch == ':';
}

static gboolean
move_to_first_blank (GtkTextIter *iter, gboolean forward, gboolean in_line)
{
	GtkTextIter copy = *iter;
	gint d = 1;
	gunichar ch;

	if (!forward)
		d = -1;

	do
	{
		if (in_line && gtk_text_iter_ends_line (&copy))
			return FALSE;
		
		ch = gtk_text_iter_get_char(&copy);
		if (g_unichar_isspace(ch))
		{
			gtk_text_iter_forward_char (&copy);
			*iter = copy;
			return TRUE;
		}
	} while (gtk_text_iter_forward_chars(&copy, d));

	return FALSE;
	
}

static gboolean
move_to_first_non_blank (GtkTextIter *iter, gboolean forward, gboolean in_line)
{
	GtkTextIter copy = *iter;
	gint d = 1;
	gunichar ch;

	if (!forward)
		d = -1;

	do
	{
		if (in_line && gtk_text_iter_ends_line (&copy))
			return FALSE;
		
		ch = gtk_text_iter_get_char(&copy);
		if (!g_unichar_isspace(ch))
		{
			*iter = copy;
			return TRUE;
		}
	} while (gtk_text_iter_forward_chars(&copy, d));

	return FALSE;
	
}

static gchar*
get_line_indentation (GtkTextIter *iter)
{
	gunichar ch;
	GtkTextIter start = *iter;
	GtkTextIter end = *iter;

	gtk_text_iter_set_line_offset (&start, 0);
	end = start;
	
	ch = gtk_text_iter_get_char (&end);

	while (g_unichar_isspace (ch))
	{
		if (gtk_text_iter_ends_line(&end))
			break;

		if (!gtk_text_iter_forward_char (&end))
		        break;

		ch = gtk_text_iter_get_char (&end);
	}

	if (gtk_text_iter_equal (&start, &end))
		return NULL;
	
	return g_strdup(gtk_text_iter_get_slice (&start, &end));
}

static gboolean
prev_word_is_sentence (GtkTextIter *iter)
{
	GtkTextIter end = *iter;

	//Search for an end of sentence (if, while, for...)
	if (gtk_text_iter_backward_char(&end) && move_to_first_non_blank (&end, FALSE, FALSE))
	{
		GtkTextIter start = end;
		gchar *word;

		move_to_first_blank (&start, FALSE, FALSE);
		gtk_text_iter_forward_char (&end);
		word = gtk_text_iter_get_slice (&start, &end);
					
		return g_utf8_collate (word, "if") == 0 ||
			g_utf8_collate (word, "while") == 0 ||
			g_utf8_collate (word, "for") == 0;
	}
	return FALSE;
}

static gboolean
process_relocators(GtkTextView *view,
		   GtkTextIter *iter)
{
	GtkTextIter copy = *iter;
	gunichar ch;

	gtk_text_iter_set_line_offset (&copy, 0);
	if (!move_to_first_non_blank (&copy, TRUE, TRUE))
		return FALSE;

	ch = gtk_text_iter_get_char (&copy);

	if (ch == '{')
	{
		gtk_text_iter_backward_char (&copy);
		if (move_to_first_non_blank (&copy, FALSE, FALSE))
		{
			ch = gtk_text_iter_get_char (&copy);

			if (ch == ')')
			{
				if (gsi_indenter_utils_find_open_char (&copy,
								       '(',
								       ')',
								       FALSE))
				{
					gchar *indent;
					GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);

					indent = get_line_indentation(&copy);

					if (!indent)
						indent = g_strdup ("");
	
					gtk_text_buffer_begin_user_action (buffer);

					gsi_indenter_utils_replace_indentation (buffer,
										gtk_text_iter_get_line (iter),
										indent);
					g_free (indent);

					gtk_text_buffer_end_user_action (buffer);

					return TRUE;
				}
			}
		}
	}
	else if (ch == '}')
	{
		if (gsi_indenter_utils_find_open_char (&copy,
						       '{',
						       '}',
						       FALSE))
		{
			gchar *indent;
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
			
			indent = get_line_indentation(&copy);
			
			if (!indent)
				indent = g_strdup ("");
			
			gtk_text_buffer_begin_user_action (buffer);
			
			gsi_indenter_utils_replace_indentation (buffer,
								gtk_text_iter_get_line (iter),
								indent);
			g_free (indent);
			
			gtk_text_buffer_end_user_action (buffer);
			
			return TRUE;
		}
	}
	
	return FALSE;
	
}

static gboolean
gsi_indenter_indent_line_real (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	gboolean res = FALSE;
	GtkTextBuffer *buffer;
	GtkTextIter copy = *iter;
	buffer = gtk_text_view_get_buffer (view);
	gchar *indent = NULL;
	gunichar ch;
	gint level_width = gsi_indenter_utils_view_get_real_indent_width (GTK_SOURCE_VIEW(view));
	gint level = -1;

	if (process_relocators (view, iter))
		return TRUE;
	
	if (!gsi_indenter_utils_iter_backward_line_not_empty (&copy))
	{
		return FALSE;
	}
	
	gtk_text_iter_forward_to_line_end (&copy);
	
	if (move_to_first_non_blank(&copy, FALSE, FALSE))
	{
		ch = gtk_text_iter_get_char (&copy);
		
		//Block start
		if (ch == '{')
		{
			level = gsi_indenter_utils_get_amount_indents(view, &copy);
			level += level_width;
		}
		
		if (level == -1 && ch == ')')
		{
			//Previous line indent by default
			level = gsi_indenter_utils_get_amount_indents(view, &copy);
			
			if (gsi_indenter_utils_find_open_char (&copy,
							       '(',
							       ')',
							       FALSE))
			{
				level = gsi_indenter_utils_get_amount_indents(view, &copy);
				if (prev_word_is_sentence(&copy))
				{
					
					level += level_width;
				}
			}
		}
		
		if (level == -1 && ch == ';')
		{
			GtkTextIter prev_iter = copy;
			gtk_text_iter_backward_char(&prev_iter);
			//Previous line indent by default
			level = gsi_indenter_utils_get_amount_indents(view, &copy);
			
			if (gtk_text_iter_backward_find_char(&copy,
							     is_end_sentence,
							     NULL,
							     NULL))
			{
				/*
				  Search if we are in:
				  if (TRUE)
				      a = 1;
				  |
				*/
				if (gtk_text_iter_get_char(&copy) == ')')
				{
					GtkTextIter close_iter = copy;
					if (gsi_indenter_utils_find_open_char (&copy,
									       '(',
									       ')',
									       FALSE))
					{
						/*
						  The prev_iter and close_iter
						  test something like:
						  func(param,
						       param);
						*/
						if (prev_word_is_sentence(&copy) ||
						    gtk_text_iter_equal(&prev_iter, &close_iter))
						{
							//Sentence line indent
							level = gsi_indenter_utils_get_amount_indents(view, &copy);
						}
					}
				}
			}
		}
		//Are we in something like: if (hello\n ?
		if (level == -1 && gsi_indenter_utils_find_open_char (&copy, '(', ')',
								      TRUE))
		{
			level = gsi_indenter_utils_get_amount_indents_from_position (view, &copy);

			//level is in ( and the position must be after the (
			level++;
		}

		if (level == -1)
		{
			level = gsi_indenter_utils_get_amount_indents(view, &copy);
		}
	}

	if (level == -1)
		level = 0;

	indent = gsi_indenter_utils_get_indent_string_from_indent_level	(GTK_SOURCE_VIEW (view), 
									 level);

	//if indent == NULL we must delete the current indentation
	if (!indent)
		indent = g_strdup ("");
	
	gtk_text_buffer_begin_user_action (buffer);

	gsi_indenter_utils_replace_indentation (buffer,
						gtk_text_iter_get_line (iter),
						indent);
	g_free (indent);

	gtk_text_buffer_end_user_action (buffer);
	
	
/*	if (res)
	{
		gtk_text_buffer_begin_user_action (buffer);
		res = FALSE;
		if (idata.level >= 0)
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
			}
			else
			{
				gsi_indenter_utils_replace_indentation (buffer,
									gtk_text_iter_get_line (iter),
									"");
			}
			res = TRUE;
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
*/
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
	//return "{}:#";
	return "{}";
}

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
	iface->indent_line = gsi_indenter_indent_line_impl;
	iface->get_relocators = gsi_indenter_get_relocators_impl;
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
