/* gsi-indenter-cxx.c */

#include "gsi-indenter-cxx.h"
#include "gsi-indenter-utils.h"

#define GSI_INDENTER_CXX_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), GSI_TYPE_INDENTER_CXX, GsiIndenterCxxPrivate))

#define RELOCATORS "{}"

typedef gboolean (*apply_indent) (GsiIndenterCxx *indenter,
				  const gchar *indent);

typedef enum {
	INDENTATION_IGNORE,
	INDENTATION_BASIC,
	INDENTATION_APPEND
}IndentationType;

typedef struct{
	const gchar *match;
	IndentationType itype;
	const gchar *append;
	apply_indent apply;
} RegexpDef;

struct _GsiIndenterCxxPrivate
{
	GtkTextView *view;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	
	gboolean use_spaces;
	guint tab_width;
};

static void gsi_indenter_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GsiIndenterCxx,
                         gsi_indenter_cxx,
                         G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSI_TYPE_INDENTER,
                                                gsi_indenter_iface_init))

static gboolean apply_prev_indent (GsiIndenterCxx *indenter,
				   const gchar *append);

static gboolean find_open_char (GtkTextIter *iter,
				gchar open,
				gchar close,
				gboolean skip_first);

static gboolean gsi_indenter_relocate_impl (GsiIndenter	*self,
					    GtkTextView	*view,
					    GtkTextIter	*iter,
					    gchar	 relocator);

static RegexpDef regexp_list [] = {
//	{".*\\/\\*(?!.*\\*\\/)", INDENTATION_APPEND, " * ", apply_prev_indent},
//	{"^\\s*\\*[^/].*$", INDENTATION_APPEND, "* ", apply_prev_indent},
	{".*\\{(?!.*\\})", INDENTATION_BASIC, NULL, apply_prev_indent},
	{"(if|while|for)\\s*\\([^\\(]*\\)\\s*$", INDENTATION_BASIC, NULL, apply_prev_indent},
	{"^\\s*else\\s*$", INDENTATION_BASIC, NULL, apply_prev_indent}
};

static void
print_iter (GtkTextIter *iter)
{
	g_debug ("iter:\nline: %i\noffset: %i\nchar: %c",
		 gtk_text_iter_get_line (iter),
		 gtk_text_iter_get_line_offset (iter),
		 gtk_text_iter_get_char (iter));
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
find_open_char (GtkTextIter *iter,
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

static gboolean
apply_prev_indent (GsiIndenterCxx *self,
		   const gchar *append)
{
	gchar *final_indent;
	gint line = gtk_text_iter_get_line (&self->priv->iter);
	gchar *indent = gsi_indenter_utils_get_line_indentation (self->priv->buffer,
								 line -1,
								 TRUE);

	if (append)
	{
		if (indent)
			final_indent = g_strjoin (NULL, indent, append, NULL);
		else
			final_indent = g_strdup (append);
	}
	else
	{
		final_indent = g_strdup (indent);
	}
	gtk_text_buffer_begin_user_action (self->priv->buffer);
	gsi_indenter_utils_replace_indentation (self->priv->buffer, line, final_indent);
	gtk_text_buffer_end_user_action (self->priv->buffer);
	
	g_free (final_indent);
	return TRUE;
}

static gboolean
gsi_indenter_cxx_indent_regexp (GsiIndenterCxx *self,
				GtkTextIter *iter,
				RegexpDef *rd)
{
	gchar *line_text;
	gboolean ret = FALSE;
	GtkTextIter prev_iter = *iter;
	
	line_text = get_line_text (self->priv->buffer, &prev_iter);

	if (g_regex_match_simple (rd->match,
				  line_text,
				  0,
				  0))
	{
		switch (rd->itype)
		{
			case INDENTATION_APPEND:
			{
				ret = rd->apply (self, rd->append);
				break;
			}
			case INDENTATION_BASIC:
			{
				gchar *basic_indent = gsi_indenter_utils_source_view_get_indent_text (GTK_SOURCE_VIEW (self->priv->view));
				ret = rd->apply (self, basic_indent);
				g_free (basic_indent);
				break;
			}
		}
	}
	return ret;
}


static gboolean
gsi_indenter_cxx_indent_open_func (GsiIndenter *indenter,
				   GtkSourceView *view,
				   GtkTextIter *iter)
{
	GtkTextIter prev_iter = *iter;
	GtkTextIter current = *iter;
	gint line = gtk_text_iter_get_line (iter);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gchar *line_text;
	gchar *indent = NULL;
	
	gtk_text_iter_backward_line (&prev_iter);
	
	line_text = get_line_text (buffer, &prev_iter);
	g_debug ("open l [%s]",line_text);
	
	if (g_regex_match_simple ("\\([^\\)]*$",
				  line_text,
				  0,
				  0))
	{
		/*If we are in ), we search from the previous char*/
		gtk_text_iter_backward_char (&current);
		
		print_iter (&current);
		
		if (find_open_char (&current,
				    '(',
				    ')',
				    FALSE))
		{
			g_debug ("line %i", gtk_text_iter_get_line (&current));
			indent = gsi_indenter_utils_get_indent_to_iter (view,
									&current);
			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_utils_replace_indentation (buffer, line, indent);
			gtk_text_buffer_end_user_action (buffer);
			
			g_free (indent);
			return TRUE;
		}
	}
	return FALSE;
}

static gboolean
gsi_indenter_cxx_indent_close_func (GsiIndenter *indenter,
				    GtkSourceView *view,
				    GtkTextIter *iter)
{
	GtkTextIter prev_iter = *iter;
	GtkTextIter current = *iter;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gchar *line_text;
	gchar *indent = NULL;
	
	gtk_text_iter_backward_line (&prev_iter);
	
	line_text = get_line_text (buffer, &prev_iter);
	
	if (g_regex_match_simple ("\\)[\\s|;|,]*$",
				  line_text,
				  0,
				  0))
	{
		gtk_text_iter_backward_char (&current);
		g_debug ("close found");
		print_iter (&current);
		if (find_open_char (&current,
				    '(',
				    ')',
				    TRUE))
		{
			/*Search if we are inside another function or sentence*/
			GtkTextIter open = current;
			gchar c;
			do
			{
				gtk_text_iter_backward_char (&open);
				c = gtk_text_iter_get_char (&open);
				print_iter (&open);
				if (c == '(')
				{
					current = open;
					break;
				}
			}
			while (c != '\n' && c != '\r');
			
			if (c == '(')
			{
				/*Indent to the open braket*/
				indent = gsi_indenter_utils_get_indent_to_iter (view,
										&current);
			}
			else
			{
				/*TODO Check if the ( is a sentence*/
				
				/*Indent to the braket line*/
				indent = gsi_indenter_utils_get_line_indentation (buffer,
										  gtk_text_iter_get_line (&current),
										  TRUE);
			}
			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_utils_replace_indentation (buffer, gtk_text_iter_get_line (iter), indent);
			gtk_text_buffer_end_user_action (buffer);
		}
		return TRUE;
	}	
	return FALSE;
}

static gboolean
find_text (GtkTextIter *origin, const gchar *chars, GtkTextIter *result)
{
	GtkTextIter iter = *origin;
	GtkTextIter temp;
	gchar c = gtk_text_iter_get_char (&iter);
	const gchar *sc = chars;
	gboolean found = FALSE;
	
	while (c != '\r' && c != '\n')
	{
		sc = chars;
		found = FALSE;
		temp = iter;
		
		while (*sc != '\0')
		{
			if (c == *sc)
			{
				found = TRUE;
				sc++;
				if (*sc == '\0')
				{
					break;
				}
					
				if (!gtk_text_iter_forward_char (&temp))
				{
					found = FALSE;
					break;
				}
				c = gtk_text_iter_get_char (&temp);
			}
			else
			{
				found = FALSE;
				break;
			}
		}
		if (found)
		{
			if (result != NULL)
				*result = iter;
			break;
		}
		
		if (!gtk_text_iter_forward_char (&iter))
			break;
		c = gtk_text_iter_get_char (&iter);
	}
	
	return found;
}

static gboolean
process_multi_comment_real (GsiIndenterCxx *self, GtkTextIter *iter,
			    gboolean first)
{
	g_debug("reaaaaaaaaaaaaal");
	GtkTextIter copy = *iter;
	gboolean found;
	RegexpDef rd_start = {".*\\/\\*(?!.*\\*\\/)", INDENTATION_APPEND, " * ", apply_prev_indent};

	if (!first)
		rd_start.append = "* ";
	
	found = gsi_indenter_cxx_indent_regexp (self, &copy, &rd_start);
	
	if (!found)
	{
		gchar *text = get_line_text (self->priv->buffer, &copy);
		if (g_regex_match_simple ("^\\s*\\*(?!\\/)(?!.*\\*\\/)",
					  text,
					  0,
					  0))
		{
			if (gsi_indenter_utils_iter_backward_line_not_empty (&copy))
				found = process_multi_comment_real (self, &copy, FALSE);
		}
	}
		 
	return found;	
}

static gboolean
process_multi_comment (GsiIndenterCxx *self, GtkTextIter *iter)
{
	return process_multi_comment_real (self, iter, TRUE);
}

static void
gsi_indenter_indent_line_impl (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	GtkTextIter prev_iter = *iter;
	GsiIndenterCxx *self = GSI_INDENTER_CXX (indenter);
	gint line = gtk_text_iter_get_line (iter);
	gboolean found = FALSE;
	
	self->priv->view = view;
	self->priv->buffer = gtk_text_view_get_buffer (view);
	self->priv->iter = *iter;
	
	if (!gsi_indenter_utils_iter_backward_line_not_empty (&prev_iter))
		return;
	
	found = process_multi_comment (self, &prev_iter);
	
	/*TODO Previous indentation by the moment*/
	/*
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
	gint line = gtk_text_iter_get_line (iter);
	gchar *indentation = NULL;
	
	RegexpDef rd;
	gint i;
	GtkTextIter copy = *iter;
	
	gtk_text_iter_set_line_offset (&copy, 0);
	
	if (gsi_indenter_utils_move_to_no_space (&copy, 1, FALSE))
	{
		gchar c = gtk_text_iter_get_char (&copy);
		const gchar *relocators = RELOCATORS;
		while (*relocators != '\0')
		{
			if (c == *relocators)
			{
				if (gsi_indenter_relocate_impl (indenter,
								view,
								&copy,
								c))
				{
					return;
				}
				else
				{
					break;
				}
			}
			relocators++;
		}
	}
	
	copy = *iter;
	
	for (i = 0; i< (sizeof (regexp_list) / sizeof (RegexpDef)); i++)
	{
		rd = regexp_list [i];
		found = gsi_indenter_cxx_indent_regexp (self,
							&copy,
							&rd);
		if (found)
			break;
	}

	if (!found)
	{
		found = gsi_indenter_cxx_indent_open_func (indenter, GTK_SOURCE_VIEW (view), &copy);
	}
	
	if (!found)
	{
		found = gsi_indenter_cxx_indent_close_func (indenter, GTK_SOURCE_VIEW (view), &copy);
	}
	*/
	
	if (!found)
	{
		gchar *indentation = NULL;
		indentation = gsi_indenter_utils_get_line_indentation (self->priv->buffer, line - 1, TRUE);
	
		gtk_text_buffer_begin_user_action (self->priv->buffer);
		gsi_indenter_utils_replace_indentation (self->priv->buffer, line, indentation);
		gtk_text_buffer_end_user_action (self->priv->buffer);
	}
}

static void
gsi_indenter_indent_region_impl (GsiIndenter *indenter,
				 GtkTextView *view,
				 GtkTextIter *start,
				 GtkTextIter *end)
{
	gsi_indenter_utils_indent_region_by_line (indenter,
						  view,
						  start,
						  end);
}

static const gchar*
gsi_indenter_get_relocators_impl (GsiIndenter	*self,
				  GtkTextView	*view)
{
	return RELOCATORS;
}

static gboolean
gsi_indenter_relocate_impl (GsiIndenter	*indenter,
			    GtkTextView	*view,
			    GtkTextIter	*iter,
			    gchar	 relocator)
{
	if (relocator == '}')
	{
		GtkTextIter open_iter = *iter;
		if (find_open_char (&open_iter, '{', '}', TRUE))
		{
			gint open_line = gtk_text_iter_get_line (&open_iter);
			gint line = gtk_text_iter_get_line (iter);
			gchar *indentation;
			GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
			indentation = gsi_indenter_utils_get_line_indentation (buffer, open_line, FALSE);

			gtk_text_buffer_begin_user_action (buffer);
			gsi_indenter_utils_replace_indentation (buffer, line, indentation);
			gtk_text_buffer_end_user_action (buffer);
			
			return TRUE;
		}
	}
	else if (relocator == '{')
	{
		GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
		gchar *line_text;

		line_text = get_line_text (buffer, iter);
		if (g_regex_match_simple ("^\\s*\\{",
					  line_text,
					  0,
					  0))
		{
			GtkTextIter close_iter = *iter;
			if (!gsi_indenter_cxx_indent_close_func (indenter, GTK_SOURCE_VIEW (view), &close_iter))
			{
				gint line = gtk_text_iter_get_line (iter);
				gchar *indentation = gsi_indenter_utils_get_line_indentation (buffer, line - 1, TRUE);
				
				gtk_text_buffer_begin_user_action (buffer);
				gsi_indenter_utils_replace_indentation (buffer, line, indentation);
				gtk_text_buffer_end_user_action (buffer);
			}
			return TRUE;
		}
	}
	return FALSE;
}

static void
gsi_indenter_iface_init (gpointer g_iface,
                         gpointer iface_data)
{
        GsiIndenterInterface *iface = (GsiIndenterInterface *)g_iface;

        /* Interface data getter implementations */
        iface->indent_line = gsi_indenter_indent_line_impl;
        iface->indent_region = gsi_indenter_indent_region_impl;
	iface->get_relocators = gsi_indenter_get_relocators_impl;
	iface->relocate = gsi_indenter_relocate_impl;
}

static void
gsi_indenter_cxx_dispose (GObject *object)
{
	G_OBJECT_CLASS (gsi_indenter_cxx_parent_class)->dispose (object);
}

static void
gsi_indenter_cxx_class_init (GsiIndenterCxxClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	g_type_class_add_private (klass, sizeof (GsiIndenterCxxPrivate));
	
	object_class->dispose = gsi_indenter_cxx_dispose;
}

static void
gsi_indenter_cxx_init (GsiIndenterCxx *self)
{
	self->priv = GSI_INDENTER_CXX_GET_PRIVATE (self);
}

GsiIndenter*
gsi_indenter_cxx_new (void)
{
	return g_object_new (GSI_TYPE_INDENTER_CXX, NULL);
}
