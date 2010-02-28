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
move_to_first_non_blank (GtkTextIter *iter, gboolean forward)
{
	gint d = 1;
	gunichar ch;

	if (!forward)
		d = -1;

	do
	{
		ch = gtk_text_iter_get_char(iter);
		if (!g_unichar_isspace(ch))
			return TRUE;
	} while (gtk_text_iter_forward_chars(iter, d));

	return FALSE;
	
}

static gchar*
get_line_indentation (GtkTextIter *iter)
{
	gunichar ch;
	GtkTextIter start = *iter;
	GtkTextIter end = *iter;

	ch = gtk_text_iter_get_char (&end);

	while (g_unichar_isspace (ch))
	{
		g_debug("searching indent [%c]", ch);
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
gsi_indenter_indent_line_real (GsiIndenter *indenter,
			       GtkTextView *view,
			       GtkTextIter *iter)
{
	gboolean res = FALSE;
	GtkTextBuffer *buffer;
	GtkTextIter copy = *iter;
	buffer = gtk_text_view_get_buffer (view);
	gchar *indent;
	gunichar ch;

	g_debug ("line start %i", gtk_text_iter_get_line (iter));
	
	if (!gsi_indenter_utils_iter_backward_line_not_empty (&copy))
	{
		return FALSE;
	}
	
	g_debug ("line nor empty %i", gtk_text_iter_get_line (&copy));

	indent = get_line_indentation(&copy);

	gtk_text_iter_forward_to_line_end (&copy);
	if (move_to_first_non_blank(&copy, FALSE))
	{
		ch = gtk_text_iter_get_char (&copy);

		if (ch == '{')
		{
			if (indent)
			{
				gchar *temp = indent;
				//TODO append the real indent
				indent = g_strdup_printf ("%s\t", temp);
				g_free(temp);
			}
			else
			{
				indent = g_strdup ("\t");
			}
		}
	}
	
	if (!indent)
		indent = g_strdup ("");
	
	gtk_text_buffer_begin_user_action (buffer);

	g_debug ("Indent %i [%s]", gtk_text_iter_get_line (&copy), indent);
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
	return NULL;
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
