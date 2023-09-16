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
 *        ROM 2.4 is copyright 1993-1996 Russ Taylor                       *
 *        ROM has been brought to you by the ROM consortium                *
 *            Russ Taylor (rtaylor@pacinfo.com)                            *
 *            Gabrielle Taylor (gtaylor@pacinfo.com)                       *
 *            Brian Moore (rom@rom.efn.org)                                *
 *        By using this code, you have agreed to follow the terms of the   *
 *        ROM license, in the file Rom24/doc/rom.license                   *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "recycle.h"

bool damage_effect( OBJ_DATA* obj )
{
    AFFECT_DATA af;
    CHAR_DATA *victim;

    if ( !obj )
	return FALSE; // -K
    if (obj_is_affected(obj,gsn_damaged))
	return FALSE;

    af.where = TO_OBJECT;
    af.type = gsn_damaged;
    af.level = 0;
    af.modifier = 0;
    af.duration = number_range(1,3);
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_obj(obj, &af);

    if ( obj->carried_by )
    {
	victim = obj->carried_by;
	unequip_char( victim, obj ); // -K - cleaner on inspection and execution
//	obj_from_char(obj);
//	obj_to_char(obj,victim);
    }

    return TRUE;
}

/*
 * Adjustement of the above to allow for IS_HARDCORE checking and
 * custom durations.  Currently this uses the tick system.
 * -K
 */
bool damage_effect_pc( CHAR_DATA* ch, OBJ_DATA* obj, int duration )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;

    if ( !obj )
	return FALSE; // oops!
    if (obj_is_affected(obj,gsn_damaged))
	return FALSE; // already affected
    if ( duration <= 0 )
	return FALSE; // invalid affect time; sanity exit
    if ( !ch
      || IS_NPC(ch) )
	return damage_effect( obj ); // not a valid player initiating

    if ( (victim = obj->carried_by) == NULL )
    { // not carried--sanity checking, move along
	af.where = TO_OBJECT;
	af.type = gsn_damaged;
	af.level = 0;
	af.modifier = 0;
	af.duration = duration;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_v_to_obj( obj, &af );
    }
    else if ( IS_HARDCORE(ch)
      && IS_HARDCORE(victim) )
    { // hardcore on hardcore
	unequip_char( victim, obj );
	obj_from_char( obj );
	//usion while we are looking at adding some new effects, this should most likely transfer to the ground (and leave a message that 
	//you drop it) rather than go immediatly to the person utilizing the effect. The only spell that should allow this to go straight
	//to the person using the effect is attraction. Attraction might already have a message already, but should be checked just in case.
//	obj_to_char( obj, ch );
	obj_to_room( obj, victim->in_room );
	act( "$n loses $s grip and drops $p!", ch, NULL, obj, TO_CHAR, POS_RESTING );
	act( "$n loses $s grip and drops $p!", ch, NULL, obj, TO_ROOM, POS_RESTING );
    }
    else
    { // everyone else
	af.where = TO_OBJECT;
	af.type = gsn_damaged;
	af.level = 0;
	af.modifier = 0;
	af.duration = duration;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_v_to_obj( obj, &af );
	unequip_char( victim, obj );
    }

    return TRUE; // successful affecting of the object
}

// Eats magical items, worsens AC slowly
void acid_effect(void *vo, int level, int dam, int target)
{
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			acid_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;

		// Typically 5% weapon hit, 25% breath.
		chance = 8 + dam / 8;

		// Burn some flesh
		if(number_percent() < chance) {
			Cprintf(victim, "Chunks of your flesh melt away, exposing vital areas!\n\r");
			act("$n's flesh burns away, revealing vital areas!", victim, NULL, NULL, TO_ROOM, POS_RESTING);

			af.where = TO_AFFECTS;
			af.type = gsn_acid_breath;
			af.level = level;
			af.duration = number_range(1, 4);
			af.modifier = 25;
			af.location = APPLY_AC;
			af.bitvector = 0;

			affect_join(victim, &af);
		}

		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			acid_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* toast an object */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj, *n_obj;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE))
			return;

		chance = (level - obj->level);
		
		chance = URANGE(1, chance, 6);

		if (number_percent() > chance)
			return;
		
		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_CONTAINER:
		case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
			msg = "$p fumes and dissolves.";
			break;
		case ITEM_CLOTHING:
			if( damage_effect(obj)) {
				msg = "$p is temporarily damaged by the acid.";
				if (obj->carried_by != NULL)
					act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
				else if (obj->in_room != NULL && obj->in_room->people != NULL)
					act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);
			}
			return;
		case ITEM_STAFF:
		case ITEM_WAND:
			msg = "$p corrodes and breaks.";
			break;
		case ITEM_SCROLL:
			msg = "$p is burned into waste.";
			break;
		}

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);

		/* get rid of the object */
		if (obj->contains)		/* dump contents */
		{
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
			{
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj, obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}

				acid_effect(t_obj, level / 2, dam / 2, TARGET_OBJ);
			}
		}

		extract_obj(obj);
		return;
	}
}


// Slowly eats strength and movement, and shatters potions/drinks.
void cold_effect(void *vo, int level, int dam, int target)
{
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			cold_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* whack a character */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;

		// Typically 5% weapon hit, 25% breath.
		chance = 5 + dam / 8;

		/* chill touch effect */
		if (number_percent() <= chance)
		{
			victim->move -= (dice(4, 6) + (level / 2));

			act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			act("A chill sinks deep into your bones.", victim, NULL, NULL, TO_CHAR, POS_RESTING);

			af.where = TO_AFFECTS;
			af.type = gsn_frost_breath;
			af.level = level;
			af.duration = number_range(1, 4);
			af.location = APPLY_STR;
			af.modifier = -1;
			af.bitvector = 0;

			affect_join(victim, &af);
		}

		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			cold_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* toast an object */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE))
			return;

		chance = level - obj->level;

		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_POTION:
			msg = "$p freezes and shatters!";
			break;
		case ITEM_DRINK_CON:
			msg = "$p freezes and shatters!";
			break;
		}

		chance = URANGE(5, chance, 20);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);

		extract_obj(obj);
		return;
	}
}



void fire_effect(void *vo, int level, int dam, int target)
{
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			fire_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;

		// Typically 5% weapon hit, 20% breath.
		chance = 5 + dam / 10;

		/* chance of blindness */
		if (!IS_AFFECTED(victim, AFF_BLIND)
		&& number_percent() <= chance)
		{
			act("$n is blinded by smoke!", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			act("Your eyes tear up from smoke...you can't see a thing!", victim, NULL, NULL, TO_CHAR, POS_RESTING);

			af.where = TO_AFFECTS;
			af.type = gsn_fire_breath;
			af.level = level;
			af.duration = URANGE(0, (level - victim->level) / 5, 2);
			af.location = APPLY_HITROLL;
			af.modifier = -4;
			af.bitvector = AFF_BLIND;

			affect_to_char(victim, &af);
		}

		/* getting thirsty */
		if (!IS_NPC(victim))
			gain_condition(victim, COND_THIRST, dam / 20);

		/* let's toast some gear! */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			fire_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* toast an object */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj, *n_obj;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE))
			return;

		chance = level - obj->level;

		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_CONTAINER:
			msg = "$p ignites and burns!";
			break;
		case ITEM_POTION:
			msg = "$p bubbles and boils!";
			break;
		case ITEM_SCROLL:
			msg = "$p crackles and burns!";
			break;
		case ITEM_STAFF:
			msg = "$p smokes and chars!";
			break;
		case ITEM_WAND:
			msg = "$p sparks and sputters!";
			break;
		case ITEM_FOOD:
			msg = "$p blackens and crisps!";
			break;
		case ITEM_PILL:
			msg = "$p melts and drips!";
			break;
		}

		chance = URANGE(1, chance, 6);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);

		if (obj->contains)
		{
			/* dump the contents */

			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
			{
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj, obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}
				fire_effect(t_obj, level / 2, dam / 2, TARGET_OBJ);
			}
		}

		extract_obj(obj);
		return;
	}
}

void earth_effect (void *vo, int level, int dam, int target)
{
	/*the main idea behind this effect is to strip fly off a player (if they have it)
	we will have to remember to add fly back in later to races that auto get it, we should look at i think shifting sands for an example of how this is done.
	for objects (not in containers), we can have potions and pills and food become filled with sand and be destroyed similar to above examples
	*/
#if (0)
	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;

		// Typically 5% weapon hit, 20% breath.
		chance = 5 + dam / 10;
	
		/* chance to strip fly */
		if (IS_AFFECTED(victim, AFF_FLYING)
		&& number_percent() <= chance)
		{
			act("The earth rises up around $n and grounds them!", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			act("The earth rises up around you, and grounds you!", victim, NULL, NULL, TO_CHAR, POS_RESTING);
			affect_strip(victim, gsn_fly);
			REMOVE_BIT(victim->affected_by, AFF_FLYING);
			af.where = TO_AFFECTS;
			af.type = gsn_earth_breath;
			af.level = level;
			af.duration = URANGE(0, (level - victim->level) / 5, 2);
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = 0;
			affect_to_char(victim, &af);
		}
	}
	//note: we need to add item effects here (check fire_effect for examples)
#endif
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			earth_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* whack a character */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;

		// Typically 5% weapon hit, 25% breath.
		chance = 5 + dam / 8;

		/* dex reduction effect */
		if (number_percent() <= chance)
		{
			victim->move -= (dice(4, 6) + (level / 2));

			act("$n stumbles as the earth claws at $s feet.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			act("The very earth itself tugs and pulls at your feet!", victim, NULL, NULL, TO_CHAR, POS_RESTING);

			af.where = TO_AFFECTS;
			af.type = gsn_earth_breath;
			af.level = level;
			af.duration = number_range(1, 4);
			af.location = APPLY_DEX;
			af.modifier = -1;
			af.bitvector = 0;

			affect_join(victim, &af);
		}

		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			earth_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* toast an object */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE))
			return;

		chance = level - obj->level;

		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_POTION:
			msg = "$p is packed full of sand and silt!";
			break;
		case ITEM_DRINK_CON:
			msg = "$p is packed full of sand and silt!";
			break;
		}

		chance = URANGE(5, chance, 20);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);

		extract_obj(obj);
		return;
	}

}

void wind_effect (void *vo, int level, int dam, int target)
{
	//this effect should blow objects into nearby rooms, but probably needs to be checked if they are actually lootable options (potions, pills, vouchers, ect) or if they are hardcore or not)
	//if the player is NOT hardcore, and it chooses an equipment item, this should set that item to damaged and relay the message to the player accordingly.
	//the effect of this element should be similar to trip, but not as strong as bash. 
	
	/*this code might not be what we need...unsure*/
		
	if ( target == TARGET_ROOM )	/* fling objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj;

		for ( obj = room->contents; obj; obj = obj->next_content )
			wind_effect( obj, level, dam, TARGET_OBJ );
		return;
	}
	if ( target == TARGET_CHAR )
	{
	
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA af;
		int chance;
	


		// Typically 5% weapon hit, 25% breath.
		chance = 8 + dam / 8;

		// mess up their combat rounds; all combat happens at the same time and cannot be delayed for one person so this will cause a chance to skip a round of combat.
		if ( number_percent() < chance )
		{
			act ("The wind blows you around, delaying your actions!", victim, NULL, NULL, TO_CHAR, POS_RESTING );
			act("$n is buffeted by the wind!", victim, NULL, NULL, TO_ROOM, POS_RESTING );

			af.where = TO_AFFECTS;
			af.type = gsn_wind_breath;
			af.level = level;
			af.duration = number_range( 1, 4 );
			af.modifier = 25;
			af.location = APPLY_AC;
			af.bitvector = 0;

			affect_join( victim, &af );
			for ( obj = victim->carrying; obj; obj = obj->next_content )
			    if ( number_percent() < chance )
				wind_effect( obj, level, dam, TARGET_OBJ );
		}

		return;
	}
	
	if ( target == TARGET_OBJ )	 /*fling an object */
	{
	    OBJ_DATA* obj = (OBJ_DATA*)vo;
	    ROOM_INDEX_DATA* pRoom = NULL;
	    EXIT_DATA* exit = NULL;
	    CHAR_DATA* target = NULL;
		int count;
		int i;
		
	    char* shoots[] = { "to the north", "to the east", "to the south", "to the west", "upwards", "downwards" };
	    char* flies[] = { "the south", "the west", "the north", "the east", "below", "above" };

	    if ( obj->wear_loc != WEAR_NONE )
		return; // worn items are safe
	    if ( obj->in_obj )
		return; // contents are safe
	    if ( obj->item_type != ITEM_POTION
	      && obj->item_type != ITEM_PILL )
	      //&& obj->item_type != ITEM_VOUCHER ) // add more lootable types in the same format
		return; // if it;s not lootable, ignore it.
	    if ( obj->carried_by )
	    {
		target = obj->carried_by;
		if ( obj->carried_by->in_room )
		{
		    pRoom = target->in_room;
		    obj_from_char( obj );
		    if ( IS_HARDCORE( target ) )
		    {
			obj_to_room( obj, pRoom );
			
			act( "$p flies from you!", target, obj, NULL, TO_CHAR, POS_RESTING );
			act( "$p flies from $n!", target, obj, NULL, TO_ROOM, POS_RESTING );
		    }
		    else
		    {
			damage_effect( obj );
			act( "You fumbles $p, and cannot make use of it!", target, obj, NULL, TO_CHAR, POS_RESTING );
			act( "$n fumbles $p, and cannot make use of it!", target, obj, NULL, TO_ROOM, POS_RESTING );
			return; // skip the room flinging aspect; that is for hardcore players
		    }
		}
	    }
	    if ( obj->in_room )
	    {
		target = obj->in_room->people;
		pRoom = obj->in_room;
	    }
	    for ( i = 0; i < MAX_DIR; i++ )
		if ( pRoom->exit[i] )
		    count++;

	    if ( count == 0 )
		    return; // no exits!
	    i = number_range( 0, MAX_DIR - 1 );
	    exit = pRoom->exit[i];
	    while ( !exit )
	    {
		i = number_range( 0, MAX_DIR - 1 );
		exit = pRoom->exit[i];
	    }

	    if ( exit )
		pRoom = exit->u1.to_room;
	    else
		pRoom = NULL;

	    if ( pRoom )
	    { // just becuase there is an exit does not mean there is a valid room
		act( "$p shoots off $T!", target, obj, shoots[i], TO_CHAR, POS_RESTING );
		act( "$p shoots off $T!", target, obj, shoots[i], TO_ROOM, POS_RESTING );
		obj_from_room( obj );
		obj_to_room( obj, pRoom );
		act( "$p flies in from $T!", pRoom->people, obj, flies[rev_dir[i]], TO_CHAR, POS_RESTING );
		act( "$p flies in from $T!", pRoom->people, obj, flies[rev_dir[i]], TO_ROOM, POS_RESTING );
	    }
	    else
	    {
		act( "$p shoots off and falls back down to the ground!", target, obj, NULL, TO_CHAR, POS_RESTING );
		act( "$p shoots off and falls back down to the ground!", target, obj, NULL, TO_ROOM, POS_RESTING );
	    }
	    return;
	}

	return;
}

void shadow_effect (void *vo, int level, int dam, int target)
{
	/*
	I think that I would rather have this effect Drain movement. Note: character movement shouldn't ever go into the negatives. (We've had issues with this before)
	Another note: by drain I don't mean give it to the person who is causing the effect, I simply mean to have the victim loose movement. I think something along the lines of
	5-10 move per effect hit would be best (at max level) so maybe level / 10 min or level / 5 max .. not sure abuot the numbers here, but at max level it looks good
	
	for items, do we need to have it do anything with items?  I think the movement loss is pretty huge. We might come back to items later, depending on thoughts about elements as they
	are coded.
*/

	if ( target == TARGET_CHAR )
	{
	    CHAR_DATA *victim = (CHAR_DATA *) vo;

	    victim->move -= level / 10;
	    if ( victim->move < 0 )
		victim->move = 0;
	    Cprintf(victim, "You are hit with a moment of crippling exhaustion!\n\r" );
	    // there is no tracking of who cast it, so it cannot be given to the caster through this system. - K

	    return;
	}
    return;
}

bool divine_effect (void *vo, int level, int dam, int target)
{
/*This code also needs to be changed. I don't like what was this idea at all.
I think what I would rather have this code do is check to see if their target has damage reduction.
If they do have damage reduction, this element should bypass it and do normal damage as if the target didn't have any damage reduction.
If they don't have damage reduction, this element should do some extra damage. It also needs a good message. Something along the lines of the powers of the divine heavens bypass your damage reduction
or you draw power from the divine to deal extra damage. 

I think for items, we might come back to this as well, depending on thoughts about these elements as they are coded.


	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		int chance;

		// Typically 5% weapon hit, 25% breath.
		chance = 8 + dam / 8;
		if ( victim->race == race_lookup( "shadow dragon" ) ) // I am guessing here.
		    chance *= 2; // opposites magnify
		if ( victim->race == race_lookup( "divine dragon" ) )
		    chance /= 2; // otherwise it is blunted

		// apply the damage
		if( number_percent() < chance )
		{
			Cprintf(victim, "The touch of the divine sears through your resistances!\n\r");
			act("$n's flesh burns from the touch of the divine!", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			return TRUE;
		}

		return FALSE;
	}

    return FALSE;
}

void poison_effect(void *vo, int level, int dam, int target)
{
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			poison_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		int chance;

		// Typically 5% weapon hit, 25% breath.
		chance = 6 + dam / 8;

		/* chance of poisoning */
		if (number_percent() <= chance)
		{
			AFFECT_DATA af;

			Cprintf(victim, "You feel poison coursing through your veins.\n\r");
			act("$n looks very ill.", victim, NULL, NULL, TO_ROOM, POS_RESTING);

			af.where = TO_AFFECTS;
			af.type = gsn_poison;
			af.level = level;
			af.duration = number_range(1, 4);
			af.location = APPLY_STR;
			af.modifier = -1;
			af.bitvector = AFF_POISON;
			affect_join(victim, &af);

			damage(victim, victim, 1 + (victim->hit / number_range(15, 30)), gsn_poison, DAM_POISON, FALSE, TYPE_MAGIC);
		}

		/* equipment */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			poison_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* do some poisoning */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;


		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_BLESS))
			return;

		chance = level - obj->level;

		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_FOOD:
			break;
		case ITEM_DRINK_CON:
			if (obj->value[0] == obj->value[1])
				return;
			break;
		}

		chance = URANGE(5, chance, 25);

		if (number_percent() > chance)
			return;

		obj->value[3] = 1;
		return;
	}
}

void shock_effect(void *vo, int level, int dam, int target)
{	
	if (target == TARGET_ROOM)
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			shock_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		int chance;

		// Typically 5% weapon hit, 35% breath.
		chance = 6 + dam / 5;

		/* daze and confused? */
		if (number_percent() <= chance)
		{
			Cprintf(victim, "Your muscles stop responding.\n\r");
			act("$n jerks and twitches from the shock!", victim, NULL, NULL, TO_ROOM, POS_RESTING);
			victim->wait += dice(1, 6);
			victim->daze += 2 * PULSE_VIOLENCE;
		}

		/* toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			shock_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE))
			return;

		chance = level - obj->level;


		chance = URANGE(1, chance, 10);

		if (number_percent() > chance)
			return;
		
		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_WAND:
		case ITEM_STAFF:
			msg = "$p overloads and explodes!";
			extract_obj(obj);
			break;
		case ITEM_JEWELRY:
			if(damage_effect(obj)) {
				msg = "$p is temporarily damaged by the lightning.";
				if (obj->carried_by != NULL)
					act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
				else if (obj->in_room != NULL && obj->in_room->people != NULL)
					act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);
			}
			return;
		}

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);
		
		return;
	}
}

void water_effect(void *vo, int level, int dam, int target)
{
	if (target == TARGET_ROOM)	/* nail objects on the floor */
	{
		ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
		OBJ_DATA *obj, *obj_next;

		for (obj = room->contents; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			water_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_CHAR)	/* do the effect on a victim */
	{
		CHAR_DATA *victim = (CHAR_DATA *) vo;
		OBJ_DATA *obj, *obj_next;
		int chance;

		// Typically 5% weapon hit, 25% spell.
		chance = 5 + dam / 8;

		/* slowed by water */
		if (number_percent() <= chance)
		{
			AFFECT_DATA af;
			if (is_affected(victim,gsn_quicksilver))
			{
				Cprintf(victim, "The water clings to you, slowing you down.\n\r");
				act("$n is slowed by the water.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
				affect_strip(victim, gsn_quicksilver);				
			}
			else if (IS_AFFECTED(victim, AFF_HASTE))
			{
				Cprintf(victim, "The water clings to you, slowing you down.\n\r");
				act("$n is slowed by the water.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
				affect_strip(victim, gsn_haste);
			}
			else
			{
				Cprintf(victim, "You cough and sputter on the water.\n\r");
				act("$n coughes and chokes on the water.", victim, NULL, NULL, TO_ROOM, POS_RESTING);
				af.where = TO_AFFECTS;
				af.type = gsn_slow;
				af.level = level;
				af.duration = number_range(1, 4);
				af.location = APPLY_DEX;
				af.modifier = -1;
				af.bitvector = 0;
				affect_join(victim, &af);
			}
		}

		/* let's toast some gear */
		for (obj = victim->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			water_effect(obj, level, dam, TARGET_OBJ);
		}
		return;
	}

	if (target == TARGET_OBJ)	/* toast an object */
	{
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		OBJ_DATA *t_obj, *n_obj;
		int chance;
		char *msg;

		if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)
			|| IS_OBJ_STAT(obj, ITEM_NOPURGE)
			|| number_range(0, 4) == 0)
			return;

		chance = level - obj->level;

		switch (obj->item_type)
		{
		default:
			return;
		case ITEM_CONTAINER:
		case ITEM_DRINK_CON:
			msg = "$p fills and bursts!";
			break;
		case ITEM_POTION:
			msg = "$p dilutes and overflows.";
			break;
		case ITEM_FOOD:
		case ITEM_PILL:
			msg = "$p dissolves into an icky sludge.";
			break;
		case ITEM_SCROLL:
			msg = "$p's ink melts and runs.";
			break;
		}

		chance = URANGE(5, chance, 10);

		if (number_percent() > chance)
			return;

		if (obj->carried_by != NULL)
			act(msg, obj->carried_by, obj, NULL, TO_ALL, POS_RESTING);
		else if (obj->in_room != NULL && obj->in_room->people != NULL)
			act(msg, obj->in_room->people, obj, NULL, TO_ALL, POS_RESTING);

		/* get rid of the object */
		if (obj->contains)		/* dump contents */
		{
			for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj)
			{
				n_obj = t_obj->next_content;
				obj_from_obj(t_obj);
				if (obj->in_room != NULL)
					obj_to_room(t_obj, obj->in_room);
				else if (obj->carried_by != NULL)
					obj_to_room(t_obj, obj->carried_by->in_room);
				else
				{
					extract_obj(t_obj);
					continue;
				}

				water_effect(t_obj, level / 2, dam / 2, TARGET_OBJ);
			}
		}

		extract_obj(obj);
		return;
	}
}

/* The new code for the madness effect can go here.
For refrence, someone hit with the madness effect will do most of what i originally was thinking shadow_effect would do. 
It will have a chance to do randomly a few diffrent things:
1) Strip an item that the victim is wearing and either fling it to the ground (if hardcore) or damage it using the damage eq effect (if not hardcore).
2) Cause the victim to cast a random spell that they currently know upon themselves. Even if they can't cast that spell in combat, they should still loose the mana for it. (But be sure not to charge for mana twice here)
3) Cause the victim to be affected by a stun, such as the stun that bash causes. 
4) Cause the victim to think they are taking massive damage, even if they are not. Perhaps they see double damage outputs? (Usion's strike DECIMATES You! x2) ? Not sure if this is possible.
5) Cause the victim to have some sort of confusion effect, making their directions backwards, N = S, W = E, ect.
 
Note that this element will be an ultamate flag that I plan on adding to the most powerful weapons. It probably won't be called with simple spells.
*/
void madness_effect (void *vo, int level, int dam, int target)
{

	if ( target == TARGET_CHAR )
	{
	    CHAR_DATA *victim = (CHAR_DATA *) vo;
	    OBJ_DATA* obj;
	    int roll;

	    roll = dice( 1, 2 ); // change the second value based on how many possibilities there are.
	    switch ( roll )
	    {
	    default: // to shut up the compiler
	    case 1: // remove and throw something on the floor
		roll = 0;
		for ( obj = victim->carrying; obj; obj = obj->next_content )
		    if ( obj->wear_loc != WEAR_NONE )
			roll++;
		if ( roll > 0 )
		{ // they have something to remove
		    int count = 0;
		    roll = dice( 1, roll + 1 ) - 1; // pick one.
		    for ( obj = victim->carrying; obj; obj = obj->next_content )
		    { // this is a destructive loop, but we are stopping as soon as we find the right item.
			if ( obj->wear_loc != WEAR_NONE )
			{
			    if ( count == roll )
			    {
				obj_from_char( obj );
				if ( IS_HARDCORE( victim ) )
				{
				    obj_to_room( obj, victim->in_room );
				    act( "$n rips $p clean off and throws it on the ground!", victim, obj, NULL, TO_CHAR, POS_RESTING );
				    act( "$n rips $p clean off and throws it on the ground!", victim, obj, NULL, TO_ROOM, POS_RESTING );
				}
				else
				{
				    damage_effect( obj );
				    act( "$n rips $p clean off!", victim, obj, NULL, TO_CHAR, POS_RESTING );
				    act( "$n rips $p clean off!", victim, obj, NULL, TO_ROOM, POS_RESTING );
				}
				break;
			    }
			    else
				count++;
			}
		    }
		} // end of the if checking to see if they had clothes
		break; // end of the remove and drop an item case in the switch
	    case 2: // trip; copied from the depths of do_trip
		act( "$n trips $mself in a moment's frothing insanity!", victim, NULL, NULL, TO_CHAR, POS_RESTING );
		act( "$n trips $mself in a moment's frothing insanity!", victim, NULL, NULL, TO_ROOM, POS_RESTING );
		victim->position = POS_RESTING;
		WAIT_STATE(victim, skill_table[gsn_trip].beats);
		damage( victim, victim, number_range(2, 5 * victim->size + 2), gsn_trip, DAM_BASH, TRUE, TYPE_SKILL );
		break; // end of trip
	    } // end of the switch
	    return;
	}
    return;
}


/* Other things to think about while we are here:
We are re-desiging the metallic dragon elemental wheel.
Mettalic dragons should have the same claw and breath attack as their element, and be resistant to their own element that they deal.
Metallic dragons should be weak against the opposite element on their wheel.
New spells will have to be coded to go with the new elements we are creating. Stuff that can create wind, divine, shadow, earth, water, will have to be added to spell lists, need const.c and merc.h stubs along with any additional code for the spells. 
The current metallic wheel looks something like this:
Divine -- Shadow (opposites)
Wind -- > Earth -- > Water -- > Wind
This means wind dragons should be vuln water resist wind, and water dragons should be vuln earth resist water
Earth dragons should be vuln wind resist earth.
Each current metallic dragon will be REPLACED by these new elements.
Silver = Water (No change needed)
Copper = Earth
Platinum == Divine
Iron == Shadow
Gold == Wind
*/

// this begs the question of adjusting the damage() function directly for the resistances.
