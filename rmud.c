/**************************************************************************
 *     InterMUD Communication Addon for Merc/Envy/ROM base MUD (1995)     *
 *                                                                        *
 * In order to use any part of this Addon, you must comply with both the  *
 * original Diku as well as the Merc license. Please refer to those       *
 * licenses for details.                                                  *
 *                                                                        *
 * Permission is granted for the use of this Addon free of charge.        *
 * You may also redistribute this Addon so long as it remains intact.     *
 *                                                                        *
 * You may NOT distribute your own changes of the this Addon without      *
 * written permission of the Author.                                      *
 *                                      James Seng (jseng@pobox.org.sg)   *
 **************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "merc.h"

void
rmud_boardcast (char *argument)
{

    DESCRIPTOR_DATA * d;

    CHAR_DATA * victim;


    if (argument[0] == '\0')
      return;


    for (d = descriptor_list; d != NULL; d = d->next)

   {

       victim = d->original ? d->original : d->character;


#if defined(ROM)
       if (d->connected == CON_PLAYING &&
	   !IS_SET (victim->comm, COMM_NORMUD) &&
	   !IS_SET (victim->comm, COMM_QUIET))

#else   /*  */
       if (d->connected == CON_PLAYING &&
	   !IS_SET (victim->deaf, CHANNEL_NORMUD))

#endif  /*  */
      {

	  send_to_char (argument, victim);

      }

   }


    return;

}


CHAR_DATA * rmud_pecho (char *name, char *argument)
{

    CHAR_DATA * victim;

    DESCRIPTOR_DATA * d;


    if (name[0] == '\0')
      return NULL;


    for (d = descriptor_list; d != NULL; d = d->next)

   {

       victim = d->original ? d->original : d->character;


       if (d->connected == CON_PLAYING
	   &&victim != NULL
	   &&is_name (name, victim->name))
      {

	  send_to_char (argument, victim);

	  return victim;

      }

   }


    return NULL;

}


void
rmud_rboardcast (char *argument, int ignore)
{

    int d;


    if (ignore >= 0
	&&ignore < MAX_RMUD
	&&IS_SET (rmud_info[ignore].flag, RF_NOBOARDCAST))

       return;


    for (d = 0; d < MAX_RMUD; d++)

   {

       if (d != ignore
	   &&rmud_desc[d].state == SR_CONNECTED
	   &&rmud_desc[d].connected)
      {

	  write_to_rbuffer (d, argument, 0);

      }

   }


    return;

}


void
rmud_connect (int ninfo)
{

    char buf[MAX_STRING_LENGTH];

    int d;


    d = new_outconnection (rmud_info[ninfo].host, rmud_info[ninfo].port);

    if (d >= MAX_RMUD)
      return;


    sprintf (buf, "PW %s %s\n\r", RMUD_NAME, rmud_info[ninfo].rpasswd);

    write_to_rbuffer (d, buf, 0);


    return;

}


void
do_rchat (CHAR_DATA * ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rchat.\n\r", ch);

       return;

   }


    if (IS_SET (ch->comm, COMM_NOCHANNELS))
   {
      send_to_char ("The gods have revoked your channel priviledges\n\r", ch);
      return;

   }

    if (argument[0] == '\0')
   {

#if defined(ROM)
       if (IS_SET (ch->comm, COMM_NORMUD))
      {

	  REMOVE_BIT (ch->comm, COMM_NORMUD);

	  send_to_char ("RMUD channel is ON\n\r", ch);

      }
      else
      {

	  SET_BIT (ch->comm, COMM_NORMUD);

	  send_to_char ("RMUD channel is OFF\n\r", ch);

      }

#else   /*  */
       if (IS_SET (ch->deaf, CHANNEL_NORMUD))
      {

	  REMOVE_BIT (ch->deaf, CHANNEL_NORMUD);

	  send_to_char ("RMUD channel is ON\n\r", ch);

      }
      else
      {

	  SET_BIT (ch->deaf, CHANNEL_NORMUD);

	  send_to_char ("RMUD channel is OFF\n\r", ch);

      }

#endif  /*  */
       return;

   }


#if defined(ROM)
    if (IS_SET (ch->comm, COMM_NORMUD))

       REMOVE_BIT (ch->comm, COMM_NORMUD);

#else   /*  */
    if (IS_SET (ch->deaf, CHANNEL_NORMUD))

       REMOVE_BIT (ch->deaf, CHANNEL_NORMUD);

#endif  /*  */

	if( ch->incog_level > 53 || ch->invis_level > 53 )
	{
		strcpy( buf2, "someone" );
	}
	else
	{
		strcpy( buf2, ch->name );
	}


    sprintf (buf, "MS %s %s %s %s\n\r", RMUD_NAME, buf2, RMUD_NAME, argument);

    rmud_rboardcast (buf, -1);

    sprintf (buf, "%s@%s rchats '%s'.\n\r", buf2, RMUD_NAME, argument);

    rmud_boardcast (buf);

}


void
do_rtell (CHAR_DATA * ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    char name[MAX_STRING_LENGTH];

    char host[MAX_STRING_LENGTH];


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rtell.\n\r", ch);

       return;

   }


    if (IS_SET (ch->comm, COMM_NOCHANNELS))
   {
      send_to_char ("The gods have revoked your channel priviledges\n\r", ch);
      return;

   }

    argument = one_argument (argument, arg);

    splitname (arg, name, host);


    if (argument[0] == '\0' || name[0] == '\0' || host[0] == '\0')
   {

       send_to_char ("rtell who@host what?\n\r", ch);

       return;

   }


    free_string (ch->rreply);

    ch->rreply = str_dup (arg);

   	if( ch->incog_level > 53 || ch->invis_level > 53 )
	{
		strcpy( buf2, "someone" );
	}
	else
	{
		strcpy( buf2, ch->name );
	}

    sprintf (buf, "You rtell %s '%s'\n\r", arg, argument);

    send_to_char (buf, ch);


    sprintf (buf, "TE %s %s %s %s %s\n\r",
	     RMUD_NAME, buf2, RMUD_NAME, arg, argument);


    rmud_rboardcast (buf, -1);


    return;

}


void
do_rreply (CHAR_DATA * ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];

    char host[MAX_STRING_LENGTH];


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rreply.\n\r", ch);

       return;

   }


    if (argument[0] == '\0')
   {

       send_to_char ("rreply what?\n\r", ch);

       return;

   }


    splitname (ch->rreply, name, host);

    if (name[0] == '\0' || host[0] == '\0')
   {

       send_to_char ("rreply to who?\n\r", ch);

       return;

   }


	if( ch->incog_level > 53 || ch->invis_level > 53 )
	{
		strcpy( buf2, "someone" );
	}
	else
	{
		strcpy( buf2, ch->name );
	}


    sprintf (buf, "You rtell %s '%s'\n\r", ch->rreply, argument);

    send_to_char (buf, ch);


    sprintf (buf, "TE %s %s %s %s %s\n\r",
	     RMUD_NAME, buf2, RMUD_NAME, ch->rreply, argument);


    rmud_rboardcast (buf, -1);


    return;

}



void
do_remote (CHAR_DATA * ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot remote.\n\r", ch);

       return;

   }


#if defined(ROM)
    if (IS_SET (ch->comm, COMM_NORMUD))

       REMOVE_BIT (ch->comm, COMM_NORMUD);

#else   /*  */
    if (IS_SET (ch->deaf, CHANNEL_NORMUD))

       REMOVE_BIT (ch->deaf, CHANNEL_NORMUD);

#endif  /*  */

    if (argument[0] == '\0')
   {

       send_to_char ("remote what?\n\r", ch);

       return;

   }

	if( ch->incog_level > 53 || ch->invis_level > 53 )
	{
		strcpy( buf2, "someone" );
	}
	else
	{
		strcpy( buf2, ch->name );
	}


    sprintf (buf, "AC %s %s %s %s\n\r", RMUD_NAME, buf2, RMUD_NAME, argument);

    rmud_rboardcast (buf, -1);


    sprintf (buf, "%s@%s %s. (remote)\n\r", buf2, RMUD_NAME, argument);

    rmud_boardcast (buf);

    return;

}


void
do_rwho (CHAR_DATA * ch, char *argument)
{

    char buf[MAX_STRING_LENGTH];

	char arg1[MAX_STRING_LENGTH];

    char arg2[MAX_STRING_LENGTH];

    char arg3[MAX_STRING_LENGTH];


    argument = one_argument (argument, arg1);

    argument = one_argument (argument, arg2);

    argument = one_argument (argument, arg3);


    if (arg1[0] == '\0')
   {

       send_to_char ("rwho mudid [next] <number>\n\r", ch);

       return;

   }
   else if (arg2[0] == '\0')
   {

       sprintf (buf, "WH %s %s %s\n\r",
		arg1, ch->name, RMUD_NAME);

       rmud_rboardcast (buf, -1);

   }
   else if (!str_cmp (arg2, "next") && is_number (arg3))
   {

       sprintf (buf, "WH %s %s %s next %s\n\r",
		arg1, ch->name, RMUD_NAME, arg3);

       rmud_rboardcast (buf, -1);

   }
   else
   {

       send_to_char ("rwho mudid [next] <number>\n\r", ch);

       return;

   }


    return;

}


void
do_rlist (CHAR_DATA * ch, char *argument)
{

    int connected[MAX_RMUD];

    int i, j;

    char buf[MAX_STRING_LENGTH];


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rlist.\n\r", ch);

       return;

   }


    for (i = 0; i < MAX_RMUD; i++)
      connected[i] = 0;


    for (i = 0; i < MAX_RMUD; i++)
   {

       if (rmud_desc[i].state == SR_CONNECTED
	   &&rmud_desc[i].connected)
      {

	  for (j = 0; j <= rmud_ninfo; j++)
	 {

	     if (!str_cmp (rmud_desc[i].info->name, rmud_info[j].name))

		connected[j] = 1;

	 }

      }

   }


    if (ch->level >= MAX_LEVEL)

       for (i = 0; i <= rmud_ninfo; i++)
      {

	  sprintf (buf, "%-10s %-20s %4d %-3s\n\r",
		   rmud_info[i].name,
		   rmud_info[i].host, rmud_info[i].port,
		   connected[i] ? "YES" : "NO");

	  send_to_char (buf, ch);

      }

    else

       for (i = 0; i <= rmud_ninfo; i++)
      {

	  sprintf (buf, "%-10s %-20s %-3s\n\r",
		   rmud_info[i].name, rmud_info[i].host,
		   connected[i] ? "YES" : "NO");

	  send_to_char (buf, ch);

      }


    sprintf (buf, "\n\rInterMUD Communication version %s by James Seng.\n\r",
	     RMUD_VERSION);

    send_to_char (buf, ch);


    return;

}


void
do_rconnect (CHAR_DATA * ch, char *argument)
{

    int ninfo;


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rconnect.\n\r", ch);

       return;

   }


    for (ninfo = 0; ninfo <= rmud_ninfo; ninfo++)
   {

       if (!str_cmp (rmud_info[ninfo].name, argument))
	 break;

   }


    if (ninfo > rmud_ninfo)
   {

       send_to_char ("There is no such remote mud list.\n\r", ch);

       return;

   }


    if (IS_SET (rmud_info[ninfo].flag, RF_CLIENT))
   {

       send_to_char ("Remote mud is a client only.\n\r", ch);

       return;

   }


    if (is_rconnected (rmud_info[ninfo].name) < MAX_RMUD)
   {

       send_to_char ("Remote mud is already connected\n\r", ch);

       return;

   }


    rmud_connect (ninfo);

    send_to_char ("Attempting to connect. Please use rlist to check state.\n\r",
		  ch);


    return;

}


void
do_rdisconnect (CHAR_DATA * ch, char *argument)
{

    int ninfo, desc;


    if (IS_NPC (ch))
   {

       send_to_char ("You cannot rdisconnect.\n\r", ch);

       return;

   }


    for (ninfo = 0; ninfo <= rmud_ninfo; ninfo++)
   {

       if (!str_cmp (rmud_info[ninfo].name, argument))
	 break;

   }


    if (ninfo > rmud_ninfo)
   {

       send_to_char ("There is no such remote mud list.\n\r", ch);

       return;

   }


    desc = is_rconnected (rmud_info[ninfo].name);


    if (desc >= MAX_RMUD)
   {

       send_to_char ("Remote mud is not connected.\n\r", ch);

       return;

   }


    close_rmud (desc);

    send_to_char ("Attempting to close remote mud. Use rlist to check state.\n\r", ch);


    return;

}

