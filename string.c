/******************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Sefert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.       *
 *                                                                            *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael            *
 *  Chastain, Michael Quan, and Mitchell Tse.                                 *
 *                                                                            *
 *  Rivers of Mud (ROM 2.3) improvements copyright (C) 1993, 1994 by          *
 *  Russ Taylor.                                                              *
 *                                                                            *
 *  Turns of the Wheel (ToW 1.1) improvements copyright (C) 1995, 1998        *
 *  by Geoff Cook and Shawn Faucher.                                          *
 *                                                                            *
 *  In order to use any part of this Mud, you must comply with all license    *
 *  agreements, including Diku (license.doc), Merc (license.txt), and ROM     *
 *  (license.rom)  In particular, you may not remove  any of these copyright  *
 *  and all license agreements must be present and unmodified.                 *
 *                                                                            *
 *     Much time and thought has gone into this software and you are          *
 *     benefitting.  We hope that you share your changes too.  What goes      *
 *     around, comes around.                                                  *
 *                                                                            *
 *  File: string.c                                                            *
 *                                                                            *
 *  This code was freely distributed with the The Isles 1.1 source code,      *
 *  and has been used here for OLC - OLC would not be what it is without      *
 *  all the previous coders who released their source code.                   *
 *                                                                            *
 ******************************************************************************/
//Modifications by Avacar to fit within RD framework.  2010


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
extern char* help_greeting;


/*
 * Called from act_comm.c for when someone is adjusting the body of 
 * a note that has already been posted. 
 */

//void    write_all_notes (void);

void    spell_check (CHAR_DATA * ch, char *string);


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void    string_edit (CHAR_DATA * ch, char **pString)
{
	Cprintf (ch, "-========- Entering the Editor -=========-\n\r");
	Cprintf (ch, "    Type .h on a new line for help\n\r");
	Cprintf (ch, " Finish with a ~ or @ on a blank line.\n\r");
	Cprintf (ch, "-=======================================-\n\r");

	if (*pString == NULL)
	{
		*pString = str_dup ("");
	}
	else
	{
		**pString = '\0';
	}

	ch->desc->pString = pString;

	return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void    string_append (CHAR_DATA * ch, char **pString)
{
	char   *rdesc;
	int     i;
	
	
	Cprintf (ch, "-=======- Entering the Editor  -========-\n\r");
	Cprintf (ch, "    Type .h on a new line for help\n\r");
	Cprintf (ch, " Finish with a ~ or @ on a blank line.\n\r");
	Cprintf (ch, "-=======================================-\n\r");

	if (*pString == NULL)
	{
		*pString = str_dup ("");
	} else {
	
		i= 1;

		Cprintf (ch, "%2d: ", i);

		for (rdesc = *pString; *rdesc; rdesc++)
		{
			if (*rdesc != '`')
				Cprintf (ch, "%c", rdesc[0]);
			else
			{
				if (rdesc[1] == 'Z')
					Cprintf (ch, "{Z}");
				else
					Cprintf (ch, "%c%c", rdesc[0], rdesc[1]);
				rdesc++;
			}


			if (*rdesc == '\r' && *(rdesc + 1))
			{
				i++;
				Cprintf (ch, "%2d: ", i);
			}
		}


		//return;
	
	//Cprintf (ch, *pString);
	}
	
	if (*(*pString + strlen (*pString) - 1) != '\r')
		Cprintf (ch, "\n\r");

	ch->desc->pString = pString;

	return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_replace (char *orig, char *old, char *new)
{
	char    xbuf[MAX_STRING_LENGTH];
	int     i;

	xbuf[0] = '\0';
	strcpy (xbuf, orig);
	if (strstr (orig, old) != NULL)
	{
		i = strlen (orig) - strlen (strstr (orig, old));
		xbuf[i] = '\0';
		strcat (xbuf, new);
		strcat (xbuf, &orig[i + strlen (old)]);
		free_string (orig);
	}

	return str_dup (xbuf);
}

/*****************************************************************************
 Name:		string_replace2
 Purpose:	Replaces a line of text.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_replace2 (CHAR_DATA * ch, char *orig, int line, char *new)
{
	char   *rdesc;
	char    xbuf[MAX_STRING_LENGTH];
	int     current_line = 1;
	int     i;
	bool    fReplaced = FALSE;

	xbuf[0] = '\0';
	strcpy (xbuf, orig);

	i = 0;

	for (rdesc = orig; *rdesc; rdesc++)
	{
		if (current_line == line && !fReplaced)
		{
			xbuf[i] = '\0';

			if (*new)
				strcat (xbuf, new);
			strcat (xbuf, "\n\r");
			fReplaced = TRUE;
		}

		if (current_line == line + 1)
		{
			strcat (xbuf, &orig[i]);
			free_string (orig);

			Cprintf (ch, "Line replaced.\n\r");

			return str_dup (xbuf);
		}

		i++;

		if (*rdesc == '\r')
			current_line++;
	}

	if (current_line - 1 != line)
	{
		Cprintf (ch, "That line does not exist.\n\r");
		return str_dup (xbuf);
	}

	free_string (orig);
	Cprintf (ch, "Line replaced.\n\r");

	return str_dup (xbuf);
}

/*****************************************************************************
 Name:		string_insertline
 Purpose:	Inserts a line, blank or containing text.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_insertline (CHAR_DATA * ch, char *orig, int line, char *addstring)
{
	char   *rdesc;
	char    xbuf[MAX_STRING_LENGTH];
	int     current_line = 1;
	int     i;

	xbuf[0] = '\0';
	strcpy (xbuf, orig);

	i = 0;

	for (rdesc = orig; *rdesc; rdesc++)
	{
		if (current_line == line)
			break;

		i++;

		if (*rdesc == '\r')
			current_line++;
	}

	if (!*rdesc)
	{
		Cprintf (ch, "That line does not exist.\n\r");
		return str_dup (xbuf);
	}

	xbuf[i] = '\0';

	if (*addstring)
		strcat (xbuf, addstring);
	strcat (xbuf, "\n\r");

	strcat (xbuf, &orig[i]);
	free_string (orig);

	Cprintf (ch, "Line inserted.\n\r");

	return str_dup (xbuf);
}


/*****************************************************************************
 Name:		string_deleteline
 Purpose:	Deletes a specified line of the string.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char   *string_deleteline (char *orig, int line)
{
	char   *rdesc;
	char    xbuf[MAX_STRING_LENGTH];
	int     current_line = 1;
	int     i = 0;

	xbuf[0] = '\0';

	for (rdesc = orig; *rdesc; rdesc++)
	{
		if (current_line != line)
		{
			xbuf[i] = *rdesc;
			i++;
		}

		if (*rdesc == '\r')
			current_line++;
	}

	free_string (orig);
	xbuf[i] = 0;

	return str_dup (xbuf);
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void    string_add (CHAR_DATA * ch, char *argument)
{
	char    buf[MAX_STRING_LENGTH];

	/*
	 * Thanks to James Seng
	 */
	smash_tilde (argument);

	if (*argument == '.')
	{
		char    arg1[MAX_INPUT_LENGTH];
		char    arg2[MAX_INPUT_LENGTH];
		char    arg3[MAX_INPUT_LENGTH];

		argument = one_argument (argument, arg1);
		
		if (!str_cmp (arg1, "./") && IS_IMMORTAL(ch))
		{
			
			interpret(ch, argument);
			Cprintf (ch, "Command performed.\n\r");
			return;
			
		}
		
		argument = first_arg (argument, arg2, FALSE);
		argument = first_arg (argument, arg3, FALSE);

		if (!str_cmp (arg1, ".c"))
		{
			Cprintf (ch, "String cleared.\n\r");
			**ch->desc->pString = '\0';
			return;
		}

		if (!str_cmp (arg1, ".s"))
		{
			char   *rdesc;
			int     i = 1;

			Cprintf (ch, "%2d: ", i);

			for (rdesc = *ch->desc->pString; *rdesc; rdesc++)
			{
				if (*rdesc != '`')
					Cprintf (ch, "%c", rdesc[0]);
				else
				{
					if (rdesc[1] == 'Z')
						Cprintf (ch, "{Z}");
					else
						Cprintf (ch, "%c%c", rdesc[0], rdesc[1]);
					rdesc++;
				}


				if (*rdesc == '\r' && *(rdesc + 1))
				{
					i++;
					Cprintf (ch, "%2d: ", i);
				}
			}
/*            send_to_char( *ch->desc->pString, ch ); */

			return;
		}

		/*if (!str_cmp (arg1, ".sp"))
		{
			spell_check (ch, *ch->desc->pString);
			return;
		}*/

		if (!str_cmp (arg1, ".r"))
		{
			if (arg2[0] == '\0')
			{
				Cprintf (ch, 
				   "usage:  .r \"old string\" \"new string\"\n\r");
				return;
			}

			smash_tilde (arg3);	/* Just to be sure -- Hugin */
			*ch->desc->pString =
			   string_replace (*ch->desc->pString, arg2, arg3);
			sprintf (buf, "'%s' replaced with '%s'.\n\r", arg2, arg3);
			Cprintf (ch, buf);
			return;
		}

		if (!str_cmp (arg1, ".rl"))
		{
			if (arg2[0] == '\0' || !is_number (arg2))
			{
				Cprintf (ch,
				   "usage:  .rl <line> <text>\n\r");
				return;
			}

			smash_tilde (arg3);	/* Just to be sure -- Hugin */
			*ch->desc->pString =
			   string_replace2 (ch, *ch->desc->pString, atoi (arg2), arg3);
			return;
		}


		if (!str_cmp (arg1, ".i"))
		{
			if (arg2[0] == '\0' || !is_number (arg2))
			{
				Cprintf (ch,
				   "usage:  .i <line> {text}\n\r");
				return;
			}

			smash_tilde (arg3);	/* Just to be sure -- Hugin */
			*ch->desc->pString =
			   string_insertline (ch, *ch->desc->pString, atoi (arg2), arg3);
			return;
		}


		if (!str_cmp (arg1, ".d"))
		{
			if (arg2[0] == '\0' || !is_number (arg2))
			{
				Cprintf (ch, 
				   "usage:  .d <line>\n\r");
				return;
			}

			*ch->desc->pString =
			   string_deleteline (*ch->desc->pString, atoi (arg2));
			sprintf (buf, "Line %d deleted.\n\r", atoi (arg2));
			Cprintf (ch, buf);
			return;
		}


		if (!str_cmp (arg1, ".f"))
		{
			*ch->desc->pString = format_string (*ch->desc->pString);
			Cprintf (ch, "String formatted.\n\r");
			return;
		}

		if (!str_cmp (arg1, ".h"))
		{
			Cprintf (ch, "Sedit help (commands on blank line):   \n\r");
			Cprintf (ch, ".r 'old' 'new'   - replace a substring \n\r");
			Cprintf (ch, "                   (requires '', \"\") \n\r");
			Cprintf (ch, ".rl <line> <text> - replaces a line    \n\r");
			Cprintf (ch, ".h               - get help (this info)\n\r");
			Cprintf (ch, ".s               - show string so far  \n\r");
			//Cprintf (ch, ".sp              - spell check string  \n\r");
			Cprintf (ch, ".f               - (word wrap) string  \n\r");
			Cprintf (ch, ".c               - clear string so far \n\r");
			Cprintf (ch, ".d <line>        - deletes a line      \n\r");
			Cprintf (ch, ".i <line> <text> - inserts a line	  \n\r");
			if (IS_IMMORTAL(ch))
				Cprintf (ch, "./ <command>     - do a regular command\n\r");
			Cprintf (ch, "@                - end string          \n\r");
			return;
		}


		Cprintf (ch, "SEdit:  Invalid dot command.\n\r");
		return;
	}

	if (*argument == '~' || *argument == '@')
	{
		/*these next 2 lines seems to be for a note adjustment editor
		 * that would involve changes to the note system...which don't 
		 * match RD's setup.
		if (ch->desc->string_editor == SE_NOTE_ADJUST)
			write_all_notes ();
		*/
		if (ch->desc->connected == CON_NOTE_FINISH) {
			Cprintf(ch, "(C)ontinue, (V)iew, (P)ost or (F)orget it?");
		}
		ch->desc->pString = NULL;
		//ch->desc->string_editor = 0;
		if ( ch->desc->pHEdit
		  && !str_cmp( ch->desc->pHEdit->keyword, "greeting" ) )
		    help_greeting = ch->desc->pHEdit->text;
		return;
	}

	strcpy (buf, *ch->desc->pString);

	/*
	 * Truncate strings to MAX_STRING_LENGTH.
	 * --------------------------------------
	 */
	if (strlen (buf) + strlen (argument) >= (MAX_STRING_LENGTH - 4))
	{
		Cprintf (ch, "String too long, last line skipped.\n\r");

		/* Force character out of editing mode. */
		ch->desc->pString = NULL;
		return;
	}

	/*
	 * Ensure no tilde's inside string.
	 * --------------------------------
	 */
	smash_tilde (argument);

	strcat (buf, argument);
	strcat (buf, "\n\r");
	free_string (*ch->desc->pString);
	*ch->desc->pString = str_dup (buf);
	return;
}


/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 * Modifications for color codes and blank lines by Geoff.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char   *format_string (char *oldstring /*, bool fSpace */ )
{
	char    xbuf[MAX_STRING_LENGTH];
	char    xbuf2[MAX_STRING_LENGTH];
	char   *rdesc;
	int     i = 0;
	int     end_of_line;
	bool    cap = TRUE;
	bool    bFormat = TRUE;

	xbuf[0] = xbuf2[0] = 0;

	i = 0;

	for (rdesc = oldstring; *rdesc; rdesc++)
	{

		if (*rdesc != '`')
		{
			if (bFormat)
			{
				if (*rdesc == '\n')
				{
					if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 3) == '\n' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						xbuf[i + 2] = '\n';
						xbuf[i + 3] = '\r';
						i += 4;
						rdesc += 2;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						i += 2;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						xbuf[i + 2] = '\n';
						xbuf[i + 3] = '\r';
						i += 4;
						rdesc += 1;
					}
					else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
					{
						xbuf[i] = '\n';
						xbuf[i + 1] = '\r';
						i += 2;
					}
					else if (xbuf[i - 1] != ' ' && xbuf[i - 1] != '\r')
					{
						xbuf[i] = ' ';
						i++;
					}
				}
				else if (*rdesc == '\r') ;
				else if (*rdesc == 'i' && *(rdesc + 1) == '.' && *(rdesc + 2) == 'e' && *(rdesc + 3) == '.')
				{
					xbuf[i] = 'i';
					xbuf[i + 1] = '.';
					xbuf[i + 2] = 'e';
					xbuf[i + 3] = '.';
					i += 4;
					rdesc += 3;
				}
				else if (*rdesc == ' ')
				{
					if (xbuf[i - 1] != ' ')
					{
						xbuf[i] = ' ';
						i++;
					}
				}
				else if (*rdesc == ')')
				{
					if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
					   && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
					{
						xbuf[i - 2] = *rdesc;
						xbuf[i - 1] = ' ';
						xbuf[i] = ' ';
						i++;
					}
					else if (xbuf[i - 1] == ' ' && (xbuf[i - 2] == ',' || xbuf[i - 2] == ';'))
					{
						xbuf[i - 1] = *rdesc;
						xbuf[i] = ' ';
						i++;
					}
					else
					{
						xbuf[i] = *rdesc;
						i++;
					}
				}
				else if (*rdesc == ',' || *rdesc == ';')
				{
					if (xbuf[i - 1] == ' ')
					{
						xbuf[i - 1] = *rdesc;
						xbuf[i] = ' ';
						i++;
					}
					else
					{
						xbuf[i] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i + 1] = ' ';
							i += 2;
						}
						else
						{
							xbuf[i + 1] = '\"';
							xbuf[i + 2] = ' ';
							i += 3;
							rdesc++;
						}
					}

				}
				else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
				{
					if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' '
					   && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
					{
						xbuf[i - 2] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i - 1] = ' ';
							xbuf[i] = ' ';
							i++;
						}
						else
						{
							xbuf[i - 1] = '\"';
							xbuf[i] = ' ';
							xbuf[i + 1] = ' ';
							i += 2;
							rdesc++;
						}
					}
					else
					{
						xbuf[i] = *rdesc;
						if (*(rdesc + 1) != '\"')
						{
							xbuf[i + 1] = ' ';
							xbuf[i + 2] = ' ';
							i += 3;
						}
						else
						{
							xbuf[i + 1] = '\"';
							xbuf[i + 2] = ' ';
							xbuf[i + 3] = ' ';
							i += 4;
							rdesc++;
						}
					}
					cap = TRUE;
				}
				else
				{
					xbuf[i] = *rdesc;
					if (cap)
					{
						cap = FALSE;
						xbuf[i] = UPPER (xbuf[i]);
					}
					i++;
				}
			}
			else
			{
				xbuf[i] = *rdesc;
				i++;
			}
		}
		else
		{
			if (*(rdesc + 1) == 'Z')
				bFormat = !bFormat;
			xbuf[i] = *rdesc;
			i++;
			rdesc++;
			xbuf[i] = *rdesc;
			i++;
		}
	}
	xbuf[i] = 0;
	strcpy (xbuf2, xbuf);

	rdesc = xbuf2;

	xbuf[0] = 0;

	for (;;)
	{
		end_of_line = 77;
		for (i = 0; i < end_of_line; i++)
		{
			if (*(rdesc + i) == '`')
			{
				end_of_line += 2;
				i++;
			}

			if (!*(rdesc + i))
				break;

			if (*(rdesc + i) == '\r')
				end_of_line = i;
		}
		if (i < end_of_line)
		{
			break;
		}
		if (*(rdesc + i - 1) != '\r')
		{
			for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)
			{
				if (*(rdesc + i) == ' ')
					break;
			}
			if (i)
			{
				*(rdesc + i) = 0;
				strcat (xbuf, rdesc);
				strcat (xbuf, "\n\r");
				rdesc += i + 1;
				while (*rdesc == ' ')
					rdesc++;
			}
			else
			{
				bug ("`5Wrap_string: `@No spaces``", 0);
				*(rdesc + (end_of_line - 2)) = 0;
				strcat (xbuf, rdesc);
				strcat (xbuf, "-\n\r");
				rdesc += end_of_line - 1;
			}
		}
		else
		{
			*(rdesc + i - 1) = 0;
			strcat (xbuf, rdesc);
			strcat (xbuf, "\r");
			rdesc += i;
			while (*rdesc == ' ')
				rdesc++;
		}
	}
	while (*(rdesc + i) && (*(rdesc + i) == ' ' ||
		  *(rdesc + i) == '\n' ||
		  *(rdesc + i) == '\r'))
		i--;
	*(rdesc + i + 1) = 0;
	strcat (xbuf, rdesc);
	if (xbuf[strlen (xbuf) - 2] != '\n')
		strcat (xbuf, "\n\r");

	free_string (oldstring);
	return (str_dup (xbuf));
}

/*****************************************************************************
 Name:		wrap_string
 Purpose:	String word-wrapping for those whose terms don't have it.
 Called by:	(many)act_comm.c (act_new)comm.c
 ****************************************************************************/

char   *wrap_string ( char *oldstring, int length )
{
	char    xbuf[MAX_STRING_LENGTH];
	static char    xbuf2[MAX_STRING_LENGTH];
	char   *rdesc;
	int     i = 0;
	int     end_of_line;
	
	if (!length)
	{
		strcpy (xbuf2, oldstring);
		return xbuf2;
	}
	
	xbuf[0] = xbuf2[0] = '\0';

	i = 0;

	rdesc = oldstring;

	for (;;)
	{
		end_of_line = length;
		for (i = 0; i < end_of_line; i++)
		{
			if (*(rdesc + i) == '`')
			{
				end_of_line += 2;
				i++;
			}

			if (!*(rdesc + i))
				break;

			if (*(rdesc + i) == '\r')
				end_of_line = i;
		}
		if (i < end_of_line)
		{
			break;
		}
		if (*(rdesc + i - 1) != '\r')
		{
			for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)
			{
				if (*(rdesc + i) == ' ')
					break;
			}
			if (i)
			{
				*(rdesc + i) = 0;
				strcat (xbuf, rdesc);
				strcat (xbuf, "\n\r");
				rdesc += i + 1;
				while (*rdesc == ' ')
					rdesc++;
			}
			else
			{
				bug ("No spaces");
				*(rdesc + (end_of_line - 2)) = 0;
				strcat (xbuf, rdesc);
				strcat (xbuf, "-\n\r");
				rdesc += end_of_line - 1;
			}
		}
		else
		{
			*(rdesc + i - 1) = 0;
			strcat (xbuf, rdesc);
			strcat (xbuf, "\r");
			rdesc += i;
			while (*rdesc == ' ')
				rdesc++;
		}
	}
	while (*(rdesc + i) && (*(rdesc + i) == ' ' ||
		  *(rdesc + i) == '\n' ||
		  *(rdesc + i) == '\r'))
		i--;
	*(rdesc + i + 1) = 0;
	strcat (xbuf, rdesc);
	if (xbuf[strlen (xbuf) - 2] != '\n')
		strcat (xbuf, "\n\r");
	strcpy(xbuf2, xbuf);
	return (xbuf2);
}

/*****************************************************************************
  
 Name:		spell_check
 Purpose:	Spell-check strings with ispell (*nix only)
 Called by:	(string_add)string.c
 ****************************************************************************/

void    spell_check (CHAR_DATA * ch, char *string)
{
	char    buf[MAX_STRING_LENGTH];
	char    newstr[MAX_STRING_LENGTH];
	char    line[MAX_STRING_LENGTH];
	char    chr;
	FILE   *fp;
	char   *rstr;
	int     i = 0;

	newstr[0] = 0;
	line[0] = 0;

	if (string == NULL)
		return;

	for (rstr = string; *rstr; rstr++)
	{
		if (*rstr != '\r' && *rstr != '~')
		{
			if (*rstr == '`')
			{
				rstr++;
			}
			else if (*rstr == '\n')
			{
				newstr[i] = ' ';
				i++;
			}
			else
			{
				newstr[i] = *rstr;
				i++;
			}
		}
	}
	newstr[i] = 0;

	fp = fopen ("spell_check.txt", "w");
	fprintf (fp, "!\n\r%s\n\r", newstr);
	fclose (fp);
	sprintf (buf, "cat spell_check.txt | ispell -a -f spell_check.txt");
	if (system(buf) != 0) fprintf(stderr, "System call failed in spell_check: buf\n");
	if ((fp = fopen ("spell_check.txt", "r")) != NULL)
	{
		i = 0;
		while (!feof (fp))
		{
			chr = getc (fp);
			line[i] = chr;
			i++;
		}
		line[i] = 0;
		fclose (fp);
		if (system("rm spell_check.txt") != 0) fprintf(stderr, "System call failed in load_char_obj: rm spell_check.txt\n");

		newstr[0] = 0;
		i = 0;
		for (rstr = str_dup (line); *rstr; rstr++)
		{
			if (*rstr == '\n' && *(rstr + 1) == '\n')
			{
				rstr++;
			}
			else
			{
				if (*rstr == '\r')
					Cprintf (ch, "`5*``");
				newstr[i] = *rstr;
				i++;
			}
		}
		free_string(rstr);
		newstr[i] = 0;
		strcat (newstr, "\n\r\n\r");
		Cprintf (ch, newstr);
	}
	return;
}




/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char   *first_arg (char *argument, char *arg_first, bool fCase)
{
	char    cEnd;

	while (*argument == ' ')
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"'
	   || *argument == '%' || *argument == '(')
	{
		if (*argument == '(')
		{
			cEnd = ')';
			argument++;
		}
		else
			cEnd = *argument++;
	}

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		if (fCase)
			*arg_first = LOWER (*argument);
		else
			*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (*argument == ' ')
		argument++;

	return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char   *string_unpad (char *argument)
{
	char    buf[MAX_STRING_LENGTH];
	char   *s;

	s = argument;

	while (*s == ' ')
		s++;

	strcpy (buf, s);
	s = buf;

	if (*s != '\0')
	{
		while (*s != '\0')
			s++;
		s--;

		while (*s == ' ')
			s--;
		s++;
		*s = '\0';
	}

	free_string (argument);
	return str_dup (buf);
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char   *string_proper (char *argument)
{
	char   *s;

	s = argument;

	while (*s != '\0')
	{
		if (*s != ' ')
		{
			*s = UPPER (*s);
			while (*s != ' ' && *s != '\0')
				s++;
		}
		else
		{
			s++;
		}
	}

	return argument;
}
