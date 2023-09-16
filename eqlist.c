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
 *  ROM 2.4 is copyright 1993-1996 Russ Taylor                              *
 *  ROM has been brought to you by the ROM consortium                       *
 *      Russ Taylor (rtaylor@pacinfo.com)                                   *
 *      Gabrielle Taylor (gtaylor@pacinfo.com)                              *
 *      Brian Moore (rom@rom.efn.org)                                       *
 *  By using this code, you have agreed to follow the terms of the          *
 *  ROM license, in the file Rom24/doc/rom.license                          *
 ***************************************************************************/

 /***************************************************************************
 *  This automatic equipment list generation system was created by Vexatin  *
 *  and specifically tailored to the objects and flags uniquely used by     *
 *  Redemption.                                                             *
 *                                                                          *
 *  Done July 2010 by Vexatin: vexatin@redemptionmud.com                    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <fcntl.h>

#include "merc.h"
#include "utils.h"
#include "eqlist.h"
#include "flags.h"
#include "olc/olc.h"

LIST_ITEM * add_item(LIST_ITEM*);
void free_item(LIST_ITEM*);
char* getApplyShort(int);
void make_flat_file(LIST_ITEM*);
void sort_item(LIST_ITEM*);
/*
 * Allocates a new empty list item and sets it to be the last one in relation
 * to the rest of the list.
 */
LIST_ITEM * add_item(LIST_ITEM * lastItem) {
	LIST_ITEM *newItem;

	/*Make sure that last item is in fact the last item
	 *or else it will break and likely cause massive leakage
	 *and make the entire unwieldy process useless.*/
	if (lastItem != NULL){
		while (lastItem->next != NULL){
			lastItem = lastItem->next;
		}
	}

	/*Allocate memory and create a blank list item at the
	 * end of the list.*/
	newItem = (LIST_ITEM *) malloc (sizeof(LIST_ITEM));
	newItem->next = NULL;
	if (lastItem != NULL) {
		lastItem->next = newItem;
	}
	newItem->prev = lastItem;

	return newItem;
}

/*
 * Frees the memory of a list item and safely pops it out of the
 * list without breaking internal structure.
 */
void free_item(LIST_ITEM * item) {
	LIST_ITEM *next, *prev;
	/*Remove links above and below and reconnect
	 * if not null.*/
	next = item->next;
	prev = item->prev;
	if (prev != NULL) {
		prev->next = next;
	}
	if (next != NULL) {
		next->prev = prev;
	}

	/*free memory of list item.*/
	free(item);
	return;
}

/*
 * Reorders the last item by floating it up to items of the same level.
 */
void sort_item(LIST_ITEM *item){
	LIST_ITEM *prev, *first;
	if (item->prev == NULL) {
		return;
	}
	/*Don't bother sorting if is highest item.*/
	if (item->level >= item->prev->level) {
		return;
	}

	/*Find the correct location for the item.*/
	for (prev = item->prev; item->level < prev->level; prev = prev->prev) {
		if (prev->prev == NULL) {
			first = prev;
			break;
		}
	}

	/*Pop the item into the correct position.*/
	if (prev == first) {
		item->prev->next = NULL;
		item->prev = NULL;
		item->next = first;
		item->next->prev = item;
	}else if (prev->next == NULL) {
		return;
	}else{
		item->prev->next = NULL;
		item->prev = prev;
		item->next = prev->next;
		item->prev->next = item;
		item->next->prev = item;
	}
	return;
}

/*
 * Goes through every item and generates a list item, adding
 * it to a list then writing to a flat file database
 */
void generate_eqlist(int format) {
	int vnum, found;
	extern int top_obj_index;
	OBJ_INDEX_DATA *list_object;
	LIST_ITEM *item = NULL;
	ROOM_INDEX_DATA * room;
	AFFECT_DATA *paf;	
	int cnt, sip, maxsip;
	/*Iterate through every vnum, as in ofind, and create a list
	 * ordering the items by level for easy storage and use.*/
	for (vnum = 0, found = 0; found < top_obj_index; vnum ++) {
		list_object = get_obj_index(vnum);

		/*Do not list objects that:
		 * Do not exist,
		 * are imm only,
		 * cannot be picked up
		 * or from unfinished areas.*/
		if (list_object == NULL) continue;
		found++;
		if (list_object->level > 53) continue;
		if (!IS_SET(list_object->wear_flags, ITEM_TAKE)) continue;
		if (list_object->area->security < 9) continue;
		/*Skip arena items and items from clan halls*/
		if (strcmp(list_object->area->name, "Capture the Flag") == 0) continue;
		if (strcmp(list_object->area->name, "Kindred Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Justice Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Venari Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Seekers Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Eclipse Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Kenshi Clan Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Prophets of Litazia Hall") == 0) continue;
		if (strcmp(list_object->area->name, "Kindred Stronghold") == 0) continue;
		if (strcmp(list_object->area->name, "Venari Stronghold") == 0) continue;
		if (strcmp(list_object->area->name, "Justice Stronghold") == 0) continue;
		if (strcmp(list_object->area->name, "Seeker Stronghold") == 0) continue;
		if (strcmp(list_object->area->name, "Kenshi Stronghold") == 0) continue;
		if (strcmp(list_object->area->name, "Prophets of Litazia Stronghold") == 0) continue;
		/*Skip blank items and toggles*/
		if (strcasecmp(list_object->short_descr, "(no short description)") == 0) continue;
		if (strcasecmp(list_object->description, "") == 0) continue;
		if (strcasecmp(list_object->short_descr, "{x") == 0) continue;
		if (strcasecmp(list_object->description, "{x") == 0) continue;
		if (list_object->short_descr[0] == '\0') continue;
		if (IS_OBJ_STAT(list_object, ITEM_MELT_DROP)
				&& IS_OBJ_STAT(list_object, ITEM_NOLOCATE)
				&& IS_OBJ_STAT(list_object, ITEM_ROT_DEATH))continue;
		/*Skip spell and skill items and other generics.*/
		if (list_object->vnum >= 1 && list_object->vnum <= 19) continue;
		if (list_object->vnum >= 24 && list_object->vnum <= 30) continue;
		if (list_object->vnum >= 10016 && list_object->vnum <= 10028) continue;

		item = add_item(item);

		/*Set item supertype*/
		sprintf(item->type, "%s", item_name(list_object->item_type));
		/*If any items have an unrecognized type, remove it.*/
		if (strcmp("none", item->type) == 0) {
			item = item->prev;
			free_item(item->next);
			continue;
		}

		/*Set level and sort*/
		item->level = list_object->level;
		sort_item(item);

		/*Set name and area*/
		item->name = list_object->name;
		strcpy(item->area, list_object->area->name);
		/*Compare type and extract type-specific data*/

		switch (list_object->item_type) {
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			/*Scrolls potions and pills have spells.*/
			sprintf(item->stats, "%s lvl%d:'%s'.'%s','%s','%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0], /*spell level*/
					(list_object->value[1] < MAX_SKILL && list_object->value[1] > 0)?skill_table[list_object->value[1]].name:"none",/*spell 1*/
					(list_object->value[2] < MAX_SKILL && list_object->value[2] > 0)?skill_table[list_object->value[2]].name:"none",/*spell 2*/
					(list_object->value[3] < MAX_SKILL && list_object->value[3] > 0)?skill_table[list_object->value[3]].name:"none",/*spell 3*/
					(list_object->value[4] < MAX_SKILL && list_object->value[4] > 0)?skill_table[list_object->value[4]].name:"none"/*spell 4*/
					);
			break;
		case ITEM_WAND:
		case ITEM_STAFF:
			/*Wands and staves have charges of spells*/
			sprintf(item->stats, "%s %d/%dlvl%d:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[2],/*Charges remaining*/
					list_object->value[1],/*Charges max*/
					list_object->value[0],/*Spell level*/
					(list_object->value[3] < MAX_SKILL && list_object->value[3] > 0)?skill_table[list_object->value[3]].name:"none"/*spell*/
					);
			break;
		case ITEM_DRINK_CON:
			/*drink containers have sips of liquid (16 units = 1 sip)*/
			sip = list_object->value[1]/liq_table[list_object->value[2]].liq_affect[4];
			if (list_object->value[1]%liq_table[list_object->value[2]].liq_affect[4] > 0) {
				sip++;
			}
			maxsip = list_object->value[0]/liq_table[list_object->value[2]].liq_affect[4];
			if (list_object->value[0]%liq_table[list_object->value[2]].liq_affect[4] > 0) {
				maxsip++;
			}
			sprintf(item->stats, "%s %d/%dsips:'%s%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					sip,/*number of liquid sips left*/
					maxsip,/*sip capacity*/
					(list_object->value[3] == 0)?"":"poisoned ",/*whether it is initially poisoned*/
					liq_table[list_object->value[2]].liq_name
					);
			break;
		case ITEM_CONTAINER:
			/*Containers have a weight capacity and multiplier*/
			sprintf(item->stats, "%s %dlbs/%dX%d%%%s%s",/*&#37 = html %*/
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*Capacity weight*/
					list_object->value[3],/*Weight per item*/
					list_object->value[4],/*Weight multiplier*/
					(list_object->value[1] & 1 == 1)?" close":"",/*Closeable*/
					(list_object->value[2]==0)?"":" lock"/*Lockable*/
					);
			break;
		case ITEM_RECALL:
			/*Recall crystals have charges of a recall location.*/
			room = get_room_index(list_object->value[0]);
			sprintf(item->stats, "%s %d/%drecalls:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[1],/*Charges remaining*/
					list_object->value[2],/*Charges max*/
					(room == NULL)?"nowhere":room->name/*Recall location*/
					);
			break;
		case ITEM_PORTAL:
			/*Portals have a target*/
			room = get_room_index(list_object->value[3]);
			sprintf(item->stats, "%s %dportals:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*Charges max*/
					(room == NULL)?"nowhere":room->name/*Portal location*/
					);
			break;
		case ITEM_FURNITURE:
			/*Furniture has a regen rate*/
			sprintf(item->stats, "%s %dhp/%dmp",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[3],/*Charges remaining*/
					list_object->value[4]/*Charges max*/
					);
			break;
		case ITEM_FOUNTAIN:
			/*Fountains have a liquid*/
			sprintf(item->stats, "%s fount:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					liq_table[list_object->value[2]].liq_name
					);
			break;
		case ITEM_THROWING:
			/*Throwing items have damage and a spell*/
			sprintf(item->stats, "%s %dd%d(%d) %s lvl%d:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*number of dice*/
					list_object->value[1],/*sides per die*/
					((1 + list_object->value[1]) * list_object->value[0] / 2),/*Average damage (y+1)x/2*/
					attack_table[list_object->value[2]].name,/*Damage noun*/
					list_object->value[3],/*Spell level*/
					(list_object->value[4] < MAX_SKILL && list_object->value[4] > 0)?skill_table[list_object->value[4]].name:"none"/*spell*/
					);
			break;
		case ITEM_AMMO:
			/*Ammunition has a amount, a type, damage and spell*/
			sprintf(item->stats, "%s %d%s %dd%d(%d) %s lvl%d:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->weight/10,/*Number of projectiles, each weighting 10*/
					(is_name("bullet", list_object->name))?"bullets":/*Type of ammunition*/
							(is_name("bolt", list_object->name))?"bolts":
									(is_name("arrow", list_object->name))?"arrows":"projectiles",
					list_object->value[0],/*Number of dice*/
					list_object->value[1],/*Sides of dice*/
					((1 + list_object->value[1]) * list_object->value[0] / 2),/*Average damage (y+1)x/2*/
					attack_table[list_object->value[2]].name,/*damage noun*/
					list_object->value[3],/*Spell level*/
					(list_object->value[4] < MAX_SKILL && list_object->value[4] > 0)?skill_table[list_object->value[4]].name:"none"/*spell 4*/
			);
			break;
		case ITEM_FOOD:
			/*Food has fullness and poison.*/
			sprintf(item->stats, "%s %dfood %dfull%s",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*Amount of food*/
					list_object->value[1],/*Amount full*/
					(list_object->value[2]==0)?"":" poison"
					);
			break;
		case ITEM_MONEY:
			/*Money has value.*/
			sprintf(item->stats, "%s %dsilver %dgold",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*Amount of silver*/
					list_object->value[1]/*Amount of gold*/
					);
			break;
		case ITEM_CHARM:
			/*Charms have a damage noun and a vulntype and damage.*/
			sprintf(item->stats, "%s %dd%d(%d) %s vuln:%s",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[1],/*Number of dice*/
					list_object->value[2],/*Sides of dice*/
					((1 + list_object->value[2]) * list_object->value[1] / 2),/*Average damage (y+1)x/2*/
					attack_table[list_object->value[3]].name,/*damage noun*/
					flag_string(vuln_flags, list_object->value[4])
					);
			break;
		case ITEM_WEAPON:
			/*Weapons have weapon types (a sub type), damage, and flags*/
			sprintf(item->type,"%s %s",
					item->type, /*Save 'weapon' supertype'*/
					flag_list(weapon_class, list_object->value[0])/*Weapon class*/
					);
			sprintf(item->stats, "%s %dd%d(%d) %s %s",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[1],/*Number of dice*/
					list_object->value[2],/*Sides of dice*/
					((1 + list_object->value[2]) * list_object->value[1] / 2),/*Average damage (y+1)x/2*/
					flag_list(weapon_flags, list_object->value[3]),/*damage noun*/
					(list_object->value[4] != 0)?weapon_bit_name(list_object->value[4]):""
					);
			break;
		case ITEM_ARMOR:
			/*Armor has AC*/
			sprintf(item->stats, "%s AC:%d/%d/%d/%d",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[0],/*Pierce*/
					list_object->value[1],/*Bash*/
					list_object->value[2],/*Slash*/
					list_object->value[3]/*Magic*/
					);
			break;
		case ITEM_LIGHT:
			/*Light has a duration*/
			sprintf(item->stats, "%s %dhrs",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[2]/*Light duration, -1 for infinite*/);
			break;
		case ITEM_BLOCK:
			/*Blocks have material*/
			sprintf(item->stats, "%s %s",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->material
					);
			break;
		case ITEM_SHEATH:
			/*Sheathes are fitted and have draw effects.*/
			if(list_object->value[3] == 0) {
				sprintf(item->stats, "%s weapon:'%s'",
						item->stats,/*make sure nothing anyone puts earlier than this is lost*/
						weapon_name(list_object->value[0])/*weapon type*/
						);
			} else{
				sprintf(item->stats, "%s weapon:'unique'",
						item->stats/*make sure nothing anyone puts earlier than this is lost*/
						);
			}
			switch(list_object->value[1]) {
			case SHEATH_FLAG:
				printf(item->stats, "%s draw:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					weapon_bit_name(list_object->value[2])/*weapon flag*/
					);
				break;
			case SHEATH_DICETYPE:
				printf(item->stats, "%s draw:'dside'",
					item->stats /*make sure nothing anyone puts earlier than this is lost*/
					);
				break;
			case SHEATH_DICECOUNT:
				printf(item->stats, "%s draw:'dcount'",
					item->stats /*make sure nothing anyone puts earlier than this is lost*/
					);
				break;
			case SHEATH_HITROLL:
				printf(item->stats, "%s draw:'%dhit'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[2]/*hitroll*/
					);
				break;
			case SHEATH_DAMROLL:
				printf(item->stats, "%s draw:'%ddam'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					list_object->value[2]/*damroll*/
					);
				break;
			case SHEATH_QUICKDRAW:
				printf(item->stats, "%s draw:'quickdraw'",
					item->stats /*make sure nothing anyone puts earlier than this is lost*/
					);
				break;
			case SHEATH_SPELL:
				printf(item->stats, "%s draw:'%s'",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					skill_table[list_object->value[2]].name/*spell*/
					);
				break;
			}
		}
		/*Set wear locations*/
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_FINGER)) {
			sprintf(item->wear, "%s finger", item->wear);
		}
		if (list_object->item_type == ITEM_LIGHT) {
			sprintf(item->wear, "%s light", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_NECK)) {
			sprintf(item->wear, "%s neck", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_BODY)) {
			sprintf(item->wear, "%s torso", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_HEAD)) {
			sprintf(item->wear, "%s head", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_LEGS)) {
			sprintf(item->wear, "%s legs", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_FEET)) {
			sprintf(item->wear, "%s feet", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_HANDS)) {
			sprintf(item->wear, "%s hands", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_ARMS)) {
			sprintf(item->wear, "%s arms", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_SHIELD)) {
			sprintf(item->wear, "%s shield", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_ABOUT)) {
			sprintf(item->wear, "%s body", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_WAIST)) {
			sprintf(item->wear, "%s waist", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_WRIST)) {
			sprintf(item->wear, "%s wrist", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_HOLD)) {
			sprintf(item->wear, "%s hold", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WEAR_FLOAT)) {
			sprintf(item->wear, "%s float", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_WIELD)) {
			sprintf(item->wear, "%s wield", item->wear);
		}
		if (IS_SET(list_object->wear_flags, ITEM_CHARGED)/* && (list_object->special[4] > 0)*/) {
			sprintf(item->stats, "%s %d/%dlvl%d:%d%%'%s'",
					item->stats,
					list_object->special[2],
					list_object->special[1],
					list_object->special[0],
					UMIN(100, 2 * list_object->special[0]),
					(list_object->special[3] < MAX_SKILL && list_object->special[3] > 0)?skill_table[list_object->special[3]].name:"none"/*spell*/
					);
		}
		if (IS_SET(list_object->wear_flags, ITEM_NOGATE)) {
			sprintf(item->stats, "%s no_gate", item->stats);
		}
		if (IS_SET(list_object->wear_flags, ITEM_NORECALL)) {
			sprintf(item->stats, "%s no_recall", item->stats);
		}
		if (IS_SET(list_object->wear_flags, ITEM_QUEST)) {
			sprintf(item->area, "quest");
		}
		if (IS_SET(list_object->wear_flags, ITEM_PELT)) {
			sprintf(item->area, "pelt");
		}
		if (IS_SET(list_object->wear_flags, ITEM_CRAFTED)) {
			sprintf(item->area, "craft");
		}
		if (IS_SET(list_object->wear_flags, ITEM_NEWBIE)) {
			sprintf(item->wear, "%s newbie", item->wear);
		}
		if (item->wear[0] == '\0') {
			sprintf(item->wear, "none", item->wear);
		}
		/*Add apply flags*/
	
		
		for (cnt = 0, paf = list_object->affected; paf; paf = paf->next){
			sprintf(item->stats, "%s %s%s%d%s",
					item->stats,/*make sure nothing anyone puts earlier than this is lost*/
					(IS_SET(paf->bitvector, AFF_DARKLIGHT))?"ngt:":"",
					(IS_SET(paf->bitvector, AFF_DAYLIGHT))?"day:":"",
					paf->modifier,/*Amount stat is changed*/
					getApplyShort(paf->location)/*get short name of modified field*/
					);
			cnt++;
		}
		/*Add extra flags*/
		sprintf(item->stats, "%s %s%s%s",
				item->stats,/*make sure nothing anyone puts earlier than this is lost*/
				(list_object->extra_flags == 0)?"":"(",
				(list_object->extra_flags == 0)?"":flag_string(extra_flags, list_object->extra_flags),
				(list_object->extra_flags == 0)?"":")"
				);
	}
	/*return to beginning of list*/
	while (item->prev != NULL){
		item = item->prev;
	}
	make_flat_file(item);
}

void make_flat_file(LIST_ITEM *item){
	FILE *fd;
	fd = fopen("../area/eqlist.txt", "w");
	if (fd == NULL) {
		perror("Unable to open eqlist.txt for writing!");
		return;
	}
	do{
		fprintf(fd, "%d~%s~%s~%s~%s~%s\n\r",
				item->level, /*write level*/
				item->name,/*write stats*/
				(item->type[0] == ' ')?item->type + 1:item->type, /*write type*/
				(item->wear[0] == ' ')?item->wear + 1:item->wear,/*write wear location*/
				item->area,	/*write area*/
				(item->stats[0] == ' ')?item->stats + 1:item->stats);/*write stats*/
		item = item->next;
		free_item(item->prev);
	}while(item->next != NULL);
	fclose(fd);
	return;
}
/*
 * Converts apply flags into short forms.
 */
char * getApplyShort(int ap) {
	switch (ap) {
	default:
		return "nostat";
	case APPLY_STR:
		return "str";
	case APPLY_DEX:
		return "dex";
	case APPLY_INT:
		return "int";
	case APPLY_WIS:
		return "wis";
	case APPLY_CON:
		return "con";
	case APPLY_SEX:
		return "sex";
	case APPLY_LEVEL:
		return "lvl";
	case APPLY_AGE:
		return "age";
	case APPLY_MANA:
		return "mp";
	case APPLY_HIT:
		return "hp";
	case APPLY_MOVE:
		return "mv";
	case APPLY_AC:
		return "ac";
	case APPLY_HITROLL:
		return "hit";
	case APPLY_DAMROLL:
		return "dam";
	case APPLY_SAVES:
		return "sav";
	case APPLY_SAVING_ROD:
		return "rodSav";
	case APPLY_SAVING_PETRI:
		return "petSav";
	case APPLY_SAVING_BREATH:
		return "breathSav";
	case APPLY_SAVING_SPELL:
		return "spelSav";
	case APPLY_SPELL_AFFECT:
		return "spelAff";
	case APPLY_DAMAGE_REDUCE:
		return "damRed";
	case APPLY_SPELL_DAMAGE:
		return "sDam";
	case APPLY_MAX_STR:
		return "mStr";
	case APPLY_MAX_DEX:
		return "mDex";
	case APPLY_MAX_CON:
		return "mCon";
	case APPLY_MAX_INT:
		return "mInt";
	case APPLY_MAX_WIS:
		return "mWis";
	case APPLY_ATTACK_SPEED:
		return "atkSpd";
	}
}
