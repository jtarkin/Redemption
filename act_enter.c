/* revision 1.2 - August 1 1999 - making it compilable under g++ */
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

 /***************************************************************************
 *        ROM 2.4 is copyright 1993-1996 Russ Taylor                        *
 *        ROM has been brought to you by the ROM consortium                 *
 *            Russ Taylor (rtaylor@pacinfo.com)                             *
 *            Gabrielle Taylor (gtaylor@pacinfo.com)                        *
 *            Brian Moore (rom@rom.efn.org)                                 *
 *        By using this code, you have agreed to follow the terms of the    *
 *        ROM license, in the file Rom24/doc/rom.license                    *
 ***************************************************************************/

#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "mobprog/mob_prog.h"

/* command procedures needed */
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_stand);

/* random room generation procedure */
ROOM_INDEX_DATA *
get_random_room (CHAR_DATA * ch)
{
   ROOM_INDEX_DATA *room;

   for (;;)
   {
      room = get_room_index (number_range (0, 65535));
      if (room != NULL)
         if (can_see_room (ch, room)
             && !room_is_private (room)
             && !IS_SET (room->room_flags, ROOM_PRIVATE)
             && !IS_SET (room->room_flags, ROOM_SOLITARY)
             && !IS_SET (room->room_flags, ROOM_SAFE)
             && (IS_NPC (ch) || IS_SET (ch->act, ACT_AGGRESSIVE)
                 || !IS_SET (room->room_flags, ROOM_LAW)))
            break;
   }

   return room;
}

/* RT Enter portals */
void
do_enter(CHAR_DATA *ch, char *argument)
{
   ROOM_INDEX_DATA *location;

   if (ch->fighting != NULL)
      return;

   /* nifty portal stuff */
   if (argument[0] != '\0')
   {
      ROOM_INDEX_DATA *old_room;
      OBJ_DATA *portal;
      CHAR_DATA *fch, *fch_next;

      old_room = ch->in_room;

      portal = get_obj_list (ch, argument, ch->in_room->contents);

      if (portal == NULL)
      {
         Cprintf (ch, "You don't see that here.\n\r");
         return;
      }

      if (portal->item_type != ITEM_PORTAL || (IS_SET (portal->value[1], EX_CLOSED) && !IS_TRUSTED (ch, ANGEL)))
      {
         Cprintf (ch, "You can't seem to find a way in.\n\r");
         return;
      }
	if (room_is_affected(ch->in_room, gsn_abandon))
	{
		Cprintf (ch, "Something prevents you from leaving...\n\r");
		return;
	}
      
      if (!IS_TRUSTED (ch, ANGEL) && !IS_SET (portal->value[2], GATE_NOCURSE) && (IS_AFFECTED (ch, AFF_CURSE) || wearing_norecall_item(ch) ))
      {
         Cprintf (ch, "Something prevents you from leaving...\n\r");
         return;
      }

      if (IS_SET (portal->value[2], GATE_RANDOM) || portal->value[3] == -1)
      {
         location = get_random_room (ch);
         portal->value[3] = location->vnum;        /* for record keeping :) */
      }
      else if (IS_SET (portal->value[2], GATE_BUGGY) && (number_percent () < 5))
         location = get_random_room (ch);
      else
         location = get_room_index (portal->value[3]);

      /*
      if (old_room != NULL && location != NULL
	&& location->area->continent != old_room->area->continent) {
	  Cprintf(ch, "The other end of the portal is too far away.\n\r");
	  return;
      }*/

      if (location == NULL
          || location == old_room
          || !can_see_room (ch, location)
          || (room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR)))
      {
         act("$p doesn't seem to go anywhere.", ch, portal, NULL, TO_CHAR, POS_RESTING);
         return;
      }

      if (IS_NPC (ch) && IS_SET (ch->act, ACT_AGGRESSIVE) && IS_SET (location->room_flags, ROOM_LAW))
      {
         Cprintf (ch, "Something prevents you from leaving...\n\r");
         return;
      }

      act("$n steps into $p.", ch, portal, NULL, TO_ROOM, POS_RESTING);

      if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
         act("You enter $p.", ch, portal, NULL, TO_CHAR, POS_RESTING);
      else
         act("You walk through $p and find yourself somewhere else...", ch, portal, NULL, TO_CHAR, POS_RESTING);
      if (IS_SET (ch->toggles, TOGGLES_SOUND))
         Cprintf(ch, "!!SOUND(sounds/wav/portal*.wav V=80 P=20 T=admin)");

      char_from_room (ch);
      char_to_room (ch, location);

      if (IS_SET (portal->value[2], GATE_GOWITH))        /* take the gate along */
      {
         obj_from_room (portal);
         obj_to_room (portal, location);
      }

      if (IS_SET (portal->value[2], GATE_NORMAL_EXIT))
         act("$n has arrived.", ch, portal, NULL, TO_ROOM, POS_RESTING);
      else
         act("$n has arrived through $p.", ch, portal, NULL, TO_ROOM, POS_RESTING);

      do_look (ch, "auto");

      /* charges */
      if (portal->value[0] > 0)
      {
         portal->value[0]--;
         if (portal->value[0] == 0)
            portal->value[0] = -1;
      }

      /* protect against circular follows */
      if (old_room == location)
         return;

      for (fch = old_room->people; fch != NULL; fch = fch_next)
      {
         fch_next = fch->next_in_room;

         if (portal == NULL || portal->value[0] == -1)
            /* no following through dead portals */
            continue;

	 if (!IS_NPC(fch) && fch->leader != NULL && !IS_SET(fch->act, PLR_AUTOFOLLOW))
		 continue;
	 
         if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM) && fch->position < POS_STANDING)
            do_stand (fch, "");

         if (fch->master == ch && fch->position == POS_STANDING)
         {

            if (IS_SET (ch->in_room->room_flags, ROOM_LAW) && (IS_NPC (fch) && IS_SET (fch->act, ACT_AGGRESSIVE)))
            {
               act("You can't bring $N into the city.", ch, NULL, fch, TO_CHAR, POS_RESTING);
               act("You aren't allowed in the city.", fch, NULL, NULL, TO_CHAR, POS_RESTING);
               continue;
            }

            act("You follow $N.", fch, NULL, ch, TO_CHAR, POS_RESTING);
            do_enter (fch, argument);
         }
      }

      if (portal != NULL && portal->value[0] == -1)
      {
         act("$p fades out of existence.", ch, portal, NULL, TO_CHAR, POS_RESTING);
         if (ch->in_room == old_room)
            act("$p fades out of existence.", ch, portal, NULL, TO_ROOM, POS_RESTING);
         else if (old_room->people != NULL)
         {
            act("$p fades out of existence.", old_room->people, portal, NULL, TO_CHAR, POS_RESTING);
            act("$p fades out of existence.", old_room->people, portal, NULL, TO_ROOM, POS_RESTING);
         }
         extract_obj (portal);
      }

        /*
         * If someone is following the char, these triggers get activated
         * for the followers before the char, but it's safer this way...
         */
      if ( IS_NPC( ch ) && HAS_TRIGGER( ch, TRIG_ENTRY ) )
            mp_percent_trigger( ch, NULL, NULL, NULL, TRIG_ENTRY );
      if ( !IS_NPC( ch ) )
            mp_greet_trigger( ch );

      return;
   }

   Cprintf (ch, "Nope, can't do it.\n\r");
   return;
}
