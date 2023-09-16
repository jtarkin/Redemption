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
 *  ROM 2.4 is copyright 1993-1996 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@pacinfo.com)                                  *
 *      Gabrielle Taylor (gtaylor@pacinfo.com)                             *
 *      Brian Moore (rom@rom.efn.org)                                      *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#include "clan.h"
#include "utils.h"
#include "recycle.h"



#define ROOM_VNUM_HELP 11240
#define ROOM_VNUM_HELP_DOM 31193
#define ROOM_VNUM_TR 11305
#define ROOM_VNUM_TR_DOM 31168
#define ROOM_VNUM_HELLIONS 11281
#define ROOM_VNUM_HELLIONS_DOM ROOM_VNUM_ALTAR_DOM
#define ROOM_VNUM_FELLOWS 11301
#define ROOM_VNUM_FELLOWS_DOM ROOM_VNUM_ALTAR_DOM

#define ASSAULT_MOB_HUMAN 6473

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                        \
                                if ( !str_cmp( word, literal ) )        \
                                {                                        \
                                    field  = value;                        \
                                    fMatch = TRUE;                        \
                                    break;                                \
                                }

void cedit_clan_spell(CHAR_DATA *ch, CLAN_DATA *clan, char *spell);
void cedit_clanner(CHAR_DATA *ch, CLAN_DATA *clan);
void cedit_create (CHAR_DATA *ch, char *cname);
void cedit_delete(CHAR_DATA *ch, CLAN_DATA *clan);
void cedit_gold(CHAR_DATA *ch, CLAN_DATA *clan, char *args);
void cedit_leader(CHAR_DATA *ch, CLAN_DATA *clan, char *leader);
void cedit_loner(CHAR_DATA *ch, CLAN_DATA *clan);
void cedit_name(CHAR_DATA *ch, CLAN_DATA *clan, char *cname);
void cedit_points(CHAR_DATA *ch, CLAN_DATA *clan, char *args);
void cedit_policy(CHAR_DATA *ch, CLAN_DATA *clan, char *args);
void cedit_policy_drop(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan);
void cedit_policy_show(CHAR_DATA *ch, CLAN_DATA *clan);
void cedit_policy_status(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan, char *args);
void cedit_policy_text(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan, char *args);
void cedit_recruiter(CHAR_DATA *ch, CLAN_DATA *clan, char *recruiter);
void cedit_show(CHAR_DATA *ch, CLAN_DATA *clan);
void cedit_show_all(CHAR_DATA *ch);
void cedit_spawn(CHAR_DATA *ch, CLAN_DATA *clan, char *vnum);
void cedit_spell_cost(CHAR_DATA *ch, CLAN_DATA *clan, char *cost);
void cedit_upkeep(CHAR_DATA *ch, CLAN_DATA *clan, char *args);
void cedit_vnum(CHAR_DATA *ch, CLAN_DATA *clan, char *vnum);
void cedit_who_name(CHAR_DATA *ch, CLAN_DATA *clan, char *whoname);


CLAN_DATA *clan_list;

/* for clans */
const CLAN_TYPE clan_table[MAX_CLAN] =
{
    /*  name,           who entry,      death-transfer room,    independent */
    /* independent should be FALSE if is a real clan */
   {"the guilds", "",            ROOM_VNUM_ALTAR,   ROOM_VNUM_ALTAR_DOM,       TRUE, FALSE, 0},
   {"loner",      "[  Loner ] ", ROOM_VNUM_ALTAR,   ROOM_VNUM_ALTAR_DOM,       TRUE, TRUE, 0},
   {"outcast",      "[ Outcast] ", ROOM_VNUM_ALTAR,   ROOM_VNUM_ALTAR_DOM,       TRUE, TRUE, 0},
   {"seeker",     "[ Seeker ] ", ROOM_VNUM_SEEKERS, ROOM_VNUM_SEEKERS_DOMINIA, FALSE, TRUE, JOIN_A},
   {"kindred",    "[ Kindred] ", ROOM_VNUM_KINDRED, ROOM_VNUM_KINDRED_DOMINIA, FALSE, TRUE, JOIN_B},
   {"venari",     "[ Venari ] ", ROOM_VNUM_VENARI,  ROOM_VNUM_VENARI_DOMINIA,  FALSE, TRUE, JOIN_C},
   {"kenshi",     "[ Kenshi ] ", ROOM_VNUM_KENSHI,  ROOM_VNUM_KENSHI_DOMINIA,  FALSE, TRUE, JOIN_D},
   {"dummyguild", "( A Guild) ", ROOM_VNUM_ALTAR,  ROOM_VNUM_ALTAR_DOM,    FALSE, FALSE, JOIN_E},
   {"help", "(  HELP  ) ", ROOM_VNUM_HELP, ROOM_VNUM_HELP_DOM, FALSE, FALSE, JOIN_F},
   {"treasure hunter", "(Treasure) ", ROOM_VNUM_TR, ROOM_VNUM_TR_DOM, FALSE, FALSE, JOIN_G},
   {"hellions", "(Hellions) ", ROOM_VNUM_HELLIONS, ROOM_VNUM_HELLIONS_DOM, FALSE, FALSE, JOIN_H},
   {"fellowship", "(Fellows) ", ROOM_VNUM_FELLOWS, ROOM_VNUM_FELLOWS_DOM, FALSE, FALSE, JOIN_I},
};

const ALLIANCE_TYPE hostility_table[7] = {
	{-3, "War", "{R"},
	{-2, "Aggressive", "{r"},
	{-1, "Defensive", "{m"},
	{-0, "Neutral", "{W"},
	{1, "Peaceful", "{c"},
	{2, "Friendly", "{b"},
	{3, "Allied", "{B"}
};
//rewrite of clan_lookup to use pointers

CLAN_DATA *clan_lookup (char *name)
{
	CLAN_DATA *clan;
	bool found = FALSE;
	for (clan = clan_list; clan != NULL; clan = clan->next)
	{
		if (!str_cmp(name,clan->name))
		{
			found = TRUE;
			break;
		}
	}
	
	// Clan no longer exists, return to loner.
	if (!found) {
		return clan_lookup("noclan"); 
	}

	return clan;
}

char *
clanName(const int index) {
    return capitalizeWords( clan_table[index].name );
}

int
clan_wiznet_lookup (int clan)
{
	return clan_table[clan].join_constant;
}

void display_clan_policy(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_POLICY *policy) {
	char pBuf[MAX_INPUT_LENGTH];
	char *p, source[MAX_INPUT_LENGTH], target[MAX_INPUT_LENGTH];
	/*Avoid NULLS, should never happen, but better to be safe...*/
	if (policy != NULL) {
		/*Format clan names*/
		strcpy(source, cap_str(clan->name));
		strcpy(target, cap_str(policy->name));
		/*Put clan names in the string*/
		sprintf(pBuf, "[%s%-8s  %8s{x] %s\n\r", hostility_table[policy->hostility + 3].color, source, target, policy->action);
		/*Find the white space*/
		for (p = pBuf;p[0] != '\0' && p[0] != ' '; p++);
		/*Draw the line of the arrow*/
		for (;p[1] == ' '; p++) {
			p[0] = '-';
		}
		/*Give the arrow a head*/
		p[0] = '>';
		/*Display*/
		Cprintf(ch, "%s", pBuf);
		return;
	}else{
		Cprintf(ch, "That policy does not exist!\n\r");
	}
}

void do_clanpolicy(CHAR_DATA *ch, char *argument) {
	char action[MAX_INPUT_LENGTH], clan[MAX_INPUT_LENGTH], *spolicy;
	char pBuf[MAX_INPUT_LENGTH];
	CLAN_POLICY *policy = NULL, *prevpolicy = NULL;
	argument = one_argument(argument, action);
	argument = one_argument(argument, clan);
	spolicy = argument;
	/*If no argument, show all clan policies your clan has defined*/
	if (action[0] == '\0') {
		policy = ch->clan->policy;
		/*No clan policies? What a useless leader!*/
		if (policy == NULL) {
			Cprintf(ch, "Your clan has not yet defined any relation policies towards other clans.\n\r");
			return;
		}else{
			Cprintf(ch, "Your clan has the following policies:\n\r");
		}
		/*Display the policies.*/
		while (policy != NULL) {
			display_clan_policy(ch, ch->clan, policy);
			policy = policy->next;
		}
		return;
	/*Add or edit a clan policy*/
	}else if (!str_cmp(action, "add")) {
		/*Mobs and non-leaders cannot set policies*/
		if (IS_NPC(ch) || ch->trust < 52)
		{
			Cprintf(ch, "You're not among clan leadership!\n\r");
			return;
		}
		/*Get the syntax right!*/
		if (clan[0] == '\0' || spolicy[0] == '\0') {
			Cprintf(ch, "You need to specify a clan and an action.\nSYNTAX: clanpolicy add <clan> <action>\n\r");
			return;
		}
		/*Cannot add policies for non-existant clans*/
		if (clan_lookup(clan) == clan_lookup("noclan")) {
			Cprintf(ch, "There is no such clan to add a policy for!\n\r");
			return;
		}
		if (clan_lookup(clan) == ch->clan) {
			Cprintf(ch, "You have no need to define a policy with your own clan!.\n\r");
			return;
		}
		
		/*check for old policies*/
		for (policy = ch->clan->policy; policy != NULL; policy = policy->next) {
			/*Already a policy? just change the action!*/
			if (clan_lookup(clan) == clan_lookup(policy->name)) {
				free_string(policy->action);
				spolicy[0] = UPPER(spolicy[0]);
				spolicy[MAX_INPUT_LENGTH - 50] = '\0';
				policy->action = str_dup(spolicy);
				Cprintf(ch,"You have changed your policy towards %s.\n\r", capitalize(clan));
				return;
			}
		}
		/*Policy is really new? Make a new policy!*/
		spolicy[0] = UPPER(spolicy[0]);
		spolicy[MAX_INPUT_LENGTH - 50] = '\0';
		sprintf(pBuf, "%s %s", clan, spolicy);
		new_clan_policy(ch->clan, pBuf, 0);
		Cprintf(ch,"You now have a policy towards %s.\n\r", capitalize(clan));
		return;
	/*Delete an existing policy*/
	}else if (!str_cmp(action, "drop")) {
		/*Mobs and non-leaders cannot set policies*/
		if (IS_NPC(ch) || ch->trust < 52)
		{
			Cprintf(ch, "You're not among clan leadership!\n\r");
			return;
		}
		/*Get the syntax right!*/
		if (clan[0] == '\0') {
			Cprintf(ch, "You need to specify a clan.\nSYNTAX: clanpolicy drop <clan>\n\r");
			return;
		}
		/*If the clan doesn't exist, it can't have a policy to drop.*/
		if (clan_lookup(clan) == clan_lookup("noclan")) {
			Cprintf(ch, "There is no such clan to drop a policy for!\n\r");
			return;
		}
		/*Look through the policies to find the one to delete*/
		for (policy = ch->clan->policy; policy != NULL; policy = policy->next) {
			/*Policy matches? Remove it!*/
			if (clan_lookup(clan) == clan_lookup(policy->name)) {
				if (prevpolicy == NULL) {
					ch->clan->policy = policy->next;
				}else{
					prevpolicy->next = policy->next;
				}
				free_clan_policy(policy);
				Cprintf(ch,"You no longer have a policy towards %s.\n\r", capitalize(clan));
				return;
			}
			prevpolicy = policy;
		}
		/*Someone tried to remove a non-existent policy!*/
		Cprintf(ch,"You never had a policy towards %s.\n\r", capitalize(clan));
	/*Set numeric hostility status*///good to here
	}else if (!str_cmp(action, "status")){
		int hostility;
		/*Mobs and non-leaders cannot set policies*/
		if (IS_NPC(ch) || ch->trust < 52)
		{
			Cprintf(ch, "You're not among the clan leadership!\n\r");
			return;
		}
		/*Get the syntax right!*/
		if (clan[0] == '\0' || spolicy[0] == '\0') {
			Cprintf(ch, "You need to specify a clan and an action.\nSYNTAX: clanpolicy status <clan> <value>\n\r");
			return;
		}
		/*Cannot set policy status for non-existant clans*/
		if (clan_lookup(clan) == clan_lookup("noclan")) {
			Cprintf(ch, "There is no such clan to set a policy status for!\n\r");
			return;
		}
		if (clan_lookup(clan) == ch->clan) {
			Cprintf(ch, "You have no need to define a hostility with your own clan!.\n\r");
			return;
		}
		/*Set hostility value*/
		if (is_number(spolicy)) {
			hostility = atoi(spolicy);
		}else{
			//set by name
			for (hostility = -3; hostility <= 4; hostility++) {
				/*If none match, return*/
				if (hostility == 4) {
					break;
				}
				/*If match found, break loop.*/
				if (!str_prefix(capitalize(spolicy), hostility_table[hostility+3].name)) {
					break;
				}
			}
		}
		if (hostility < -3 || hostility > 3) {
			Cprintf(ch,"Hostility values can only range from -3, %s, to 3, %s.\n\r", hostility_table[0].name, hostility_table[6].name);
			return;
		}


		/*check for old policies*/
		for (policy = ch->clan->policy; policy != NULL; policy = policy->next) {
			/*Already a policy? just change the value!*/
			if (clan_lookup(clan) == clan_lookup(policy->name)) {
				if (!str_cmp(hostility_table[policy->hostility+3].name, policy->action)) {
					free_string(policy->action);
					policy->action = str_dup(hostility_table[hostility+3].name);
				}
				policy->hostility = hostility;
				Cprintf(ch,"You have changed your policy towards %s.\n\r", capitalize(clan));
				return;
			}
		}

		/*Policy is new? Make a new policy!*/
		sprintf(pBuf, "%s %s", clan, hostility_table[hostility+3].name);
		new_clan_policy(ch->clan, pBuf, hostility);
		Cprintf(ch,"You now have a policy towards %s.\n\r", capitalize(clan));
		return;
	/*lookup policies for a specific clan*/
	}else{
		/*Make sure the clan exists!*/
		if (clan_lookup(action) == clan_lookup("noclan")) {
			Cprintf(ch,"Which clan's policy did you want to see?.\n\r");
			return;
		}
		if (clan_lookup(action)->loner == TRUE && clan_lookup(action) != ch->clan) {
			Cprintf(ch,"Those loners don't think of anyone but themselves.\n\r");
			return;
		}
		/*If it is your own clan, see what everyone else thinks of you.*/
		if (clan_lookup(action) == ch->clan) {
			CLAN_DATA *pClan;
			bool found;
			/*Check each clan for policies*/
			for (pClan = clan_list; pClan != NULL; pClan = pClan->next) {
				found = FALSE;
				/*Ignore policies of loner clans since they won't have any*/
				if (pClan->loner == TRUE)
					continue;
				/*Check each policy for your clan!*/
				for (policy = pClan->policy; policy != NULL; policy = policy->next) {
					/*They have a policy about you!*/
					if (clan_lookup(policy->name) == ch->clan) {
						display_clan_policy(ch, pClan, policy);
						found = TRUE;
					}
				}
				/*They don't have a policy about you... Inform and move on to next clan.*/
				if (!found) {
					strcpy(clan, capitalize(ch->clan->name));
					Cprintf(ch,"%s has no policies towards %s.\n\r", capitalize(pClan->name), clan);
				}
			}
		/*Look up some other clan's policies*/
		}else{
			bool found;
			CLAN_DATA *pClan = clan_lookup(action);
			found = FALSE;
			/*Display each of the clan's policies*/
			for (policy = pClan->policy; policy != NULL; policy = policy->next) {
				display_clan_policy(ch, pClan, policy);
				found = TRUE;
			}
			/*The clan has no policies*/
			if (!found) {
				Cprintf(ch,"%s has no policies.\n\r", capitalize(pClan->name));
			}
		}
		return;
	}
}

//this function looks up ch for a policy about victim's clan. Returns 0 for mobs and neutrals.
int determine_hostility(CHAR_DATA *ch, CHAR_DATA *victim) {
	CLAN_POLICY *policy = NULL;
	
	if (IS_NPC(victim) || IS_NPC(ch))
		return 0;
	
	if (ch->clan == victim->clan) { //you are always allied with your own clan
		return 3;
	}
	
	for (policy = ch->clan->policy; policy != NULL; policy = policy->next) {
			
		if (victim->clan == clan_lookup(policy->name)) {
			return policy->hostility;
		}
		
	}
	
	return 0;


}
void do_claninfo(CHAR_DATA *ch, char *argument)
{
	CLAN_DATA *clan;
	//bool found = FALSE;
	for (clan = clan_list; clan != NULL; clan = clan->next)
	{
		if (clan->name != NULL && clan->name[0] != '\0' && str_cmp(clan->name,"loner")
			&& str_cmp(clan->name,"outcast") && str_cmp(clan->name,"noclan"))
		{
			Cprintf(ch, "[%-10s] Leader: %-12s Recruiters: %-12s, %-12s\n\r",
				clan->name ? capitalize(clan->name) : "none",
				clan->leader ? clan->leader : "none",
				clan->recruiter1 ? clan->recruiter1 : "none",
				clan->recruiter2 ? clan->recruiter2 : "none");					
		}
	}

	return;	
}

/*
 * SYNTAX: clanedit <clan> <editing what?> <arguments>
 */
void do_clanedit (CHAR_DATA *ch, char *argument) {
	char clanName[MAX_INPUT_LENGTH];
	char edit[MAX_INPUT_LENGTH];
	CLAN_DATA *clan = NULL;

	if (argument[0] == '\0') {
		Cprintf(ch,"SYNTAX:\n\r");
		Cprintf(ch,"        cedit <clan name> clanner\n\r");
		Cprintf(ch,"        cedit <clan name> clanspell <spell>\n\r");
		Cprintf(ch,"        cedit <clan name> create\n\r");
		Cprintf(ch,"        cedit <clan name> delete\n\r");
		Cprintf(ch,"        cedit <clan name> gold <amount>\n\r");
		Cprintf(ch,"        cedit <clan name> gold  add <amount>\n\r");
		Cprintf(ch,"        cedit <clan name> leader <leader|none>\n\r");
		Cprintf(ch,"        cedit <clan name> loner\n\r");
		Cprintf(ch,"        cedit <clan name> name <new name>\n\r");
		Cprintf(ch,"        cedit <clan name> points <amount>\n\r");
		Cprintf(ch,"        cedit <clan name> points  add <amount>\n\r");
		Cprintf(ch,"        cedit <clan name> policy drop <target clan>\n\r");
		Cprintf(ch,"        cedit <clan name> policy show\n\r");
		Cprintf(ch,"        cedit <clan name> policy status <target clan> <relationship status>\n\r");
		Cprintf(ch,"        cedit <clan name> policy text <target clan> <policy text>\n\r");
		Cprintf(ch,"        cedit <clan name> recruiter <player|none>\n\r");
		Cprintf(ch,"        cedit show\n\r");
		Cprintf(ch,"        cedit <clan name> show\n\r");
		Cprintf(ch,"        cedit <clan name> spawn <room vnum>\n\r");
		Cprintf(ch,"        cedit <clan name> vnum <clan vnum>\n\r");
		Cprintf(ch,"        cedit <clan name> spellcost <deity points>\n\r");
		Cprintf(ch,"        cedit <clan name> upkeep <amount> <gold|points>\n\r");
		Cprintf(ch,"        cedit <clan name> whoname <text>\n\r");

		return;
	}

	argument = one_argument(argument, clanName);
	argument = one_argument(argument, edit);

	//Show instead of clan name, show all.
	//This is a hard check to avoid any clashes with clans.
	if (!str_cmp(clanName, "show")) {
		cedit_show_all(ch);
		return;
	}

	//Create put before check for valid clan name or else could not create clans.
	if (!str_prefix(edit, "create")) {
		cedit_create(ch, clanName);
		return;
	}

	//Clan does not exist.
	if (((clan = clan_lookup(clanName)) == clan_lookup("noclan"))) {
		Cprintf(ch,"%s is not an existing clan, use \"cedit <clan> create\" to create it.\n\r", capitalize(clanName));
		return;
	}

	//Clan exists, edit specified value
	//Keep edits in alphabetical order so soft compares continue to work.
	if (edit[0] == '\0') {
		Cprintf(ch,"What part of %s did you want to edit? See \"help clanedit\" for a list of clanedit commands.\n\r", capitalize(clanName));
		return;
	}else if (!str_prefix(edit, "clanner")) {
		cedit_clanner(ch, clan);
	}else if (!str_prefix(edit, "clanspell")) {
		cedit_clan_spell(ch, clan, argument);
	}else if (!str_prefix(edit, "delet")) {
		Cprintf(ch,"You must type in the full command to delete a clan.\n\r");
	}else if (!str_prefix(edit, "delete")) {
		cedit_delete(ch, clan);
	}else if (!str_prefix(edit, "gold")) {
		cedit_gold(ch, clan, argument);
	}else if (!str_prefix(edit, "leader")) {
		cedit_leader(ch, clan, argument);
	}else if (!str_prefix(edit, "loner")) {
		cedit_loner(ch, clan);
	}else if (!str_prefix(edit, "name")) {
		cedit_name(ch, clan, argument);
	}else if (!str_prefix(edit, "points")) {
		cedit_points(ch, clan, argument);
	}else if (!str_prefix(edit, "policy")) {
		cedit_policy(ch, clan, argument);
	}else if (!str_prefix(edit, "recruiter")) {
		cedit_recruiter(ch, clan, argument);
	}else if (!str_prefix(edit, "save")) {
		Cprintf(ch,"Clandata saved.\n\r");
		Cprintf(ch,"Clandata is saved automatically when editing, so this is superfluous.\n\r");
	}else if (!str_prefix(edit, "show")) {
		cedit_show(ch, clan);
	}else if (!str_prefix(edit, "spawn")) {
		cedit_spawn(ch, clan, argument);
	}else if (!str_prefix(edit, "vnum")) {
		cedit_vnum(ch, clan, argument);
	}else if (!str_prefix(edit, "spellcost")) {
		cedit_spell_cost(ch, clan, argument);
	}else if (!str_prefix(edit, "upkeep")) {
		cedit_upkeep(ch, clan, argument);
	}else if (!str_prefix(edit, "whoname")) {
		cedit_who_name(ch, clan, argument);
	}else{
		Cprintf(ch,"%s is not a valid clanedit command. See \"help clanedit\" for a list of clanedit commands.\n\r", capitalize(edit));
		return;
	}
	write_clandata();
	return;
}

void cedit_clan_spell(CHAR_DATA *ch, CLAN_DATA *clan, char *spell) {
	int value;

	//Must be at least level 59 to change clan spell
	if (get_trust(ch) < 59){
		Cprintf(ch, "You must be at least level 59 to change clan clanspells.n\r");
		return;
	}

	//Must specify a spell
	if (spell[0] == '\0')
	{
		Cprintf(ch,"Change it to what spell?\n\r");
		return;
	}
	value = skill_lookup(spell);
	//Must be an existing spell
	if (value < 0){
		Cprintf(ch,"That spell or skill does not exist.\n\r");
		return;
	}

	//Set clanspell
	clan->clanspell = value;
	Cprintf(ch,"Clanspell changed.\n\r");
	return;
}

void cedit_clanner(CHAR_DATA *ch, CLAN_DATA *clan) {
	//Must be at least level 59 to change clanner status
	if (get_trust(ch) < 59){
		Cprintf(ch, "You must be at least level 59 to change clan status.\n\r");
		return;
	}

	//Change clan status
	if (clan->clanner) {
		clan->clanner = FALSE;
	}else{
		clan->clanner = TRUE;
	}

	Cprintf(ch,"%s is now a %s.\n\r", capitalize(clan->name), (clan->clanner)?"clan":"guild");
	return;
}

void cedit_create (CHAR_DATA *ch, char *cname){
	CLAN_DATA *clan;
	//only IMPs can create new clans
	if (get_trust(ch) < 60){
		Cprintf(ch, "You must be at least level 60 to create clans.n\r");
		return;
	}

	//Cannot create a nameless clan
	if (cname == '\0'){
		Cprintf(ch,"Create a clan or guild with what name?\n\r");
		return;
	}

	if (!str_cmp(cname, "noclan")) {
		Cprintf(ch,"%s is not a valid clan name!\n\r", capitalize(cname));
		return;
	}
	//That clan already exists, don't want to overwrite it
	if ((clan_lookup(cname) != clan_lookup("noclan"))) {
		Cprintf(ch,"%s already exists!\n\r", capitalize(cname));
		return;
	}

	//Create it, defaults should already be set in new_clan in memory.c
	clan = new_clan();
	free_string(clan->name);
	clan->name = str_dup(cname);

	//Fix up the global clan_list
	clan->next = clan_list;
	clan_list = clan;

	Cprintf(ch,"Clan %s created!\n\r", capitalize(cname));
	return;
}

void cedit_delete(CHAR_DATA *ch, CLAN_DATA *clan) {
	CHAR_DATA *ich;

	//only IMPs can create new clans
	if (get_trust(ch) < 60){
		Cprintf(ch, "You must be at least level 60 to create clans.n\r");
		return;
	}

	//Cannot delete noclan (perma-crash) and not non-existant clans.
	if (clan == clan_lookup("noclan")){
		Cprintf(ch, "You cannot delete a clan that does not exist!\n\r");
		return;
	}

	//Cannot delete noclan (perma-crash) and not non-existant clans.
	if (clan == clan_lookup("Loner") || clan == clan_lookup("Loner")){
		Cprintf(ch, "You need Loner and Outcast! You cannot delete them.\n\r");
		return;
	}

	//clan list stuff
	if (clan == clan_list) {//it's on the list properly, yay, just remove it
		clan_list = clan->next;
	}else{ //it's not on the list properly, boo, have to do it manually
		CLAN_DATA *prev;

		for (prev = clan_list; prev != NULL; prev = prev->next){
			if (prev->next == clan){
				prev->next = clan->next;
				break;
			}
		}

		if (prev == NULL){ //uhoh
			bug("Delete clan: clan not found.", 0);
			return;
		}
	}
	//reset existing characters
	for (ich = char_list; ich != NULL; ich = ich->next){
		if (ich != NULL && ich->clan == clan){
			if (clan->clanner && !clan->loner){
				set_clan(ich,"loner");
			}else{
				set_clan(ich,"noclan");
			}

			//reset leaders
			if (!IS_NPC(ich) && ich->trust > 51){
				ich->trust = ich->level;
			}
		}
	}
	Cprintf(ch,"Clan deleted!\n\r");
	free_clan(clan); //free it from memory.c
	return;
}

void cedit_gold(CHAR_DATA *ch, CLAN_DATA *clan, char *args) {
	int value;
	char arg1[MAX_INPUT_LENGTH];
	args = one_argument(args, arg1);

	//Must provide an argument
	if (arg1[0] == '\0') {
		Cprintf(ch, "What did you want to set the gold to?!\n\r");
		return;
	}

	if (!str_cmp(arg1, "add")) {//add gold
		//must specify an amount to add
		if (args[0] == '\0') {
			Cprintf(ch, "You need to specify the amount to add!\n\r");
			return;
		}
		if (!is_number(args)) {
			Cprintf(ch, "You must specify the gold to add as a number.!\n\r");
			return;
		}
		//Add to clan gold
		value = atoi(args);
		clan->clangold += value;
	}else{//set gold
		if (arg1[0] == '\0') {
			Cprintf(ch, "You need to specify the amount of gold to set!\n\r");
			return;
		}
		if (!is_number(arg1)) {
			Cprintf(ch, "You must specify the gold to set as a number.!\n\r");
			return;
		}
		//Add to clan gold
		value = atoi(arg1);
		clan->clangold = value;
	}

	//done
	Cprintf(ch,"Clan gold changed to %ld.\n\r", clan->clangold);
	return;
}

void cedit_leader(CHAR_DATA *ch, CLAN_DATA *clan, char *leader) {
	CHAR_DATA *victim;

	//Need to choose someone...
	if (leader[0] == '\0'){
		Cprintf(ch,"Set the leader to whom?\n\r");
		return;
	}

	//To be leaderless
	if (!str_cmp(leader,"none")){
		//No one to take leadership from.
		if (!str_cmp(clan->leader,"none")){
			Cprintf(ch,"That clan leader is already blank!\n\r");
			return;
		}else{ //Strip him!
			victim = get_char_world(ch, clan->leader, TRUE);
			if (!victim){
				Cprintf(ch, "The existing leader, %s, needs to be present!\n\r", capitalize(clan->leader));
				return;
			}
		}
		demote_leader(victim);
		Cprintf(victim,"You are no longer a clan leader.\n\r");
		Cprintf(ch,"Clan leader stripped.\n\r");
		return;
	}

	victim = get_char_world(ch, leader, TRUE);

	//Who were you going to make leader?
	if (!victim){
		Cprintf(ch, "That is not a player!\n\r");
		return;
	}

	//Can't lead 2 clans at once!
	if (victim->level == 52 || victim->trust > 51 ){
		Cprintf(ch,"That person is already a leader! You can set only non-leaders.\n\r");
		return;
	}

	//This should never happen... Since we just stripped leader.
	if (str_cmp(clan->leader,"none")){
		Cprintf(ch,"That clan already has a leader. You must remove them first (clanedit leader <clan> none).\n\r");
		return;
	}

	//Free-for-alls have no leaders
	if (clan->loner){
		Cprintf(ch,"That clan can not have leaders.\n\r");
		return;
	}

	//Why lead a clan they aren't a member of?
	if (clan != victim->clan) {
		Cprintf(ch,"They are not a member of that clan.\n\r");
		return;
	}

	//You passed, enjoy your promotion
	advance_leader(victim);
	Cprintf(ch,"Leader assigned.\n\r");
	Cprintf(victim,"You have been chosen by the Gods to lead your clan to new heights!\n\r");
	return;
}

void cedit_loner(CHAR_DATA *ch, CLAN_DATA *clan) {

	//Must be at least level 59
	if (get_trust(ch) < 59)
	{
		Cprintf(ch, "You must be at least level 59 to change the cohesiveness of clans.\n\r");
		return;
	}

	//Change unification status
	if (clan->loner) {
		clan->loner = FALSE;
	}else{
		clan->loner = TRUE;
	}
	Cprintf(ch,"%s is now %s.\n\r", capitalize(clan->name), (!clan->loner)?"a unified group":"a mass of loners");
	return;
}

void cedit_name(CHAR_DATA *ch, CLAN_DATA *clan, char *cname) {
	//Only IMPS can delete clans
	if (get_trust(ch) < 60){
		Cprintf(ch, "You must be at least level 60 to delete clans.n\r");
		return;
	}

	//Must have a name
	if (cname[0] == '\0'){
		Cprintf(ch,"Rename it to what?\n\r");
		return;
	}

	//Cannot rename to noclan or an already existing clan or bugger up clanedit show
	if (!str_cmp(cname, "noclan") || !str_cmp(cname, "show") || clan_lookup("noclan") != clan_lookup(cname)) {
		Cprintf(ch,"%s is is currently unavailable as a clan name.\n\r", capitalize(cname));
		return;
	}

	//All's well, change name
	Cprintf(ch,"Clan name changed.\n\r");
	free_string(clan->name);
	clan->name = str_dup(cname);
	return;
}

void cedit_points(CHAR_DATA *ch, CLAN_DATA *clan, char *args) {
	int value;
	char arg1[MAX_INPUT_LENGTH];
	args = one_argument(args, arg1);

	//Must provie an argument
	if (arg1[0] == '\0') {
		Cprintf(ch, "What did you want to set the points to?!\n\r");
		return;
	}

	if (!str_cmp(arg1, "add")) {//add points
		//must specify an amount to add
		if (args[0] == '\0') {
			Cprintf(ch, "You need to specify the amount to add!\n\r");
			return;
		}
		if (!is_number(args)) {
			Cprintf(ch, "You must specify the number of points to add as a number.!\n\r");
			return;
		}
		//Add to clan points
		value = atoi(args);
		clan->clanpoints += value;
	}else{//set points
		if (arg1[0] == '\0') {
			Cprintf(ch, "You need to specify the amount of points to set!\n\r");
			return;
		}
		if (!is_number(arg1)) {
			Cprintf(ch, "You must specify the number of points to set as a number.!\n\r");
			return;
		}
		//Add to clan points
		value = atoi(arg1);
		clan->clanpoints = value;
	}

	//done
	Cprintf(ch,"Clan points changed to %ld.\n\r", clan->clanpoints);
	return;
}

void cedit_policy(CHAR_DATA *ch, CLAN_DATA *clan, char *args) {
	char action[MAX_INPUT_LENGTH];
	char target[MAX_INPUT_LENGTH];
	CLAN_DATA *pclan;

	//Get Action
	args = one_argument(args, action);

	//Must specify something to do with the policy
	if (action[0] == '\0') {
		Cprintf(ch, "Do what to %s's clan policies?!\n\r", capitalize(clan->name));
		return;
	}

	//display policies
	if (!str_prefix(action, "show")) {
		cedit_policy_show(ch, clan);
		return;
	}

	//Get target clan
	args = one_argument(args, target);
	if (action[0] == '\0') {
		Cprintf(ch, "Modify %s's policy towards who?!\n\r", capitalize(clan->name));
		return;
	}
	pclan = clan_lookup(target);

	//Gotta be a real clan
	if (pclan == clan_lookup("noclan")) {
		Cprintf(ch, "%s is not a clan that can be the target policies!\n\r", capitalize(target));
		return;
	}

	//Do what with the policy?
	if (!str_prefix(action, "drop")) {
		cedit_policy_drop(ch, clan, pclan);
		return;
	}else if (!str_prefix(action, "status")) {
		cedit_policy_status(ch, clan, pclan, args);
		return;
	}else if (!str_prefix(action, "text")) {
		cedit_policy_text(ch, clan, pclan, args);
		return;
	}else{
		Cprintf(ch, "%s is not an action that can be performed on clan policies!\n\r", capitalize(target));
		return;
	}
}

void cedit_policy_drop(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan) {
	CLAN_POLICY *policy, *prevpolicy;
	prevpolicy = NULL;
	/*Look through the policies to find the one to delete*/
	for (policy = clan->policy; policy != NULL; policy = policy->next) {
		/*Policy matches? Remove it!*/
		if (pclan == clan_lookup(policy->name)) {
			if (prevpolicy == NULL) {
				clan->policy = policy->next;
			}else{
				prevpolicy->next = policy->next;
			}
			free_clan_policy(policy);
			Cprintf(ch,"%s no longer has a policy towards %s.\n\r", capitalize(clan->name), pclan->name);
			return;
		}
		prevpolicy = policy;
	}
	/*Someone tried to remove a non-existent policy!*/
	Cprintf(ch,"%s never had a policy towards %s.\n\r", capitalize(clan->name), pclan->name);
}

void cedit_policy_show(CHAR_DATA *ch, CLAN_DATA *clan) {
	CLAN_POLICY *policy;
	if (clan->policy == NULL) {
		Cprintf(ch, "%s has not yet defined any relation policies towards other clans.\n\r", capitalize(clan->name));
		return;
	}else{
		Cprintf(ch, "%s has the following policies:\n\r", capitalize(clan->name));
	}
	for (policy = clan->policy; policy != NULL; policy = policy->next) {
		display_clan_policy(ch, clan, policy);
	}
}

void cedit_policy_status(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan, char *args) {
	int hostility;
	CLAN_POLICY *policy;
	char pbuf[MAX_STRING_LENGTH];

	/*Get the syntax right!*/
	if (args[0] == '\0') {
		Cprintf(ch, "What did you want to change the status to?\n\r");
		return;
	}

	//get hostility value
	if (is_number(args)) {
		hostility = atoi(args);
	}else{
		//get hostility by name
		for (hostility = -3; hostility <= 4; hostility++) {
			/*If none match, return*/
			if (hostility == 4) {
				break;
			}
			/*If match found, break loop.*/
			if (!str_prefix(capitalize(args), hostility_table[hostility+3].name)) {
				break;
			}
		}
	}

	//must be a valid value
	if (hostility < -3 || hostility > 3) {
		Cprintf(ch,"Hostility values can only range from -3, %s, to 3, %s.\n\r", hostility_table[0].name, hostility_table[6].name);
		return;
	}

	/*check for old policies*/
	for (policy = clan->policy; policy != NULL; policy = policy->next) {
		/*Already a policy? just change the value!*/
		if (pclan == clan_lookup(policy->name)) {
			if (!str_cmp(hostility_table[policy->hostility+3].name, policy->action)) {
				free_string(policy->action);
				policy->action = str_dup(hostility_table[hostility+3].name);
			}
			policy->hostility = hostility;
			Cprintf(ch,"%s has changed their policy towards %s.\n\r", capitalize(clan->name), pclan->name);
			return;
		}
	}

	/*Create new policy*/
	sprintf(pbuf, "%s %s", pclan->name, hostility_table[hostility + 3].name);
	new_clan_policy(clan, pbuf, hostility);
	Cprintf(ch,"%s a new policy towards %s.\n\r", capitalize(clan->name), pclan->name);
	return;
}

void cedit_policy_text(CHAR_DATA *ch, CLAN_DATA *clan, CLAN_DATA *pclan, char *args) {
	CLAN_POLICY *policy;
	char pBuf[MAX_STRING_LENGTH];
	/*Get the syntax right!*/
	if ( args[0] == '\0') {
		Cprintf(ch, "You need to specify the text of the policy.\n\r");
		return;
	}

	/*check for old policies*/
	for (policy = clan->policy; policy != NULL; policy = policy->next) {
		/*Already a policy? just change the action!*/
		if (pclan == clan_lookup(policy->name)) {
			free_string(policy->action);
			args[0] = UPPER(args[0]);
			if (strlen(args) >= MAX_INPUT_LENGTH - 50)
				args[MAX_INPUT_LENGTH - 50] = '\0';
			policy->action = str_dup(args);
			Cprintf(ch,"%s has changed their policy towards %s.\n\r", capitalize(clan->name), pclan->name);
			return;
		}
	}
	/*Policy is really new? Make a new policy!*/
	args[0] = UPPER(args[0]);
	if (strlen(args) >= MAX_INPUT_LENGTH - 50)
		args[MAX_INPUT_LENGTH - 50] = '\0';
	sprintf(pBuf, "%s %s", pclan->name, args);
	new_clan_policy(clan, pBuf, 0);
	Cprintf(ch,"%s now have a policy towards %s.\n\r", capitalize(clan->name), pclan->name);
	return;
}

void cedit_recruiter(CHAR_DATA *ch, CLAN_DATA *clan, char *recruiter) {
	CHAR_DATA *victim, *rec;

	//Need to specify a recruiter
	if (recruiter == '\0'){
		Cprintf(ch,"Set a recruiter to whom?\n\r");
		return;
	}

	//We don't need no recruiters
	if (!str_cmp(recruiter,"none")){
		if (str_cmp(clan->recruiter1,"none")){
			//Get rid of first recruiter
			victim = get_char_world(ch, clan->recruiter1, TRUE);
			if (!victim){
				Cprintf(ch, "The existing recruiter, %s, is not present!\n\r", capitalize(clan->recruiter1));
			}else{
				demote_recruiter(victim);
				Cprintf(victim,"You are no longer a clan recruiter.\n\r");
				Cprintf(ch,"Clan recruiter, %s, stripped.\n\r", capitalize(victim->name));
			}
		}else{
			//There is no first recruiter
			Cprintf(ch, "There is already no first recruiter!\n\r");
		}
		if (str_cmp(clan->recruiter1,"none")){
			//Get rid of first recruiter
			victim = get_char_world(ch, clan->recruiter2, TRUE);
			if (!victim){
				Cprintf(ch, "The existing recruiter, %s, is not present!\n\r", capitalize(clan->recruiter2));
			}else{
				demote_recruiter(victim);
				Cprintf(victim,"You are no longer a clan recruiter.\n\r");
				Cprintf(ch,"Clan recruiter, %s, stripped.\n\r", capitalize(victim->name));
			}
		}else{
			//There is no second recruiter
			Cprintf(ch, "There is already no second recruiter!\n\r");
		}
		return;
	}

	//Add recruiter
	victim = get_char_world(ch, recruiter, TRUE);

	//No imaginary friends...
	if (!victim){
		Cprintf(ch, "That is not a player!\n\r");
		return;
	}

	//Must be a member of the clan
	if (victim->clan != clan) {
		Cprintf(ch,"They cannot be a recruiter for a clan they are not a member of.\n\r");
		return;
	}

	//Gotta be a non-leader mortal
	if (victim->trust > 51 ){
		//If they are already a recruiter demote them
		rec = get_char_world(ch, clan->recruiter1, TRUE);
		if (victim == rec) {
			demote_recruiter(victim);
			Cprintf(victim,"You are no longer a clan recruiter.\n\r");
			Cprintf(ch,"Clan recruiter, %s, stripped.\n\r", capitalize(victim->name));
			return;
		}
		rec = get_char_world(ch, clan->recruiter2, TRUE);
		if (victim == rec) {
			demote_recruiter(victim);
			Cprintf(victim,"You are no longer a clan recruiter.\n\r");
			Cprintf(ch,"Clan recruiter, %s, stripped.\n\r", capitalize(victim->name));
			return;
		}
		Cprintf(ch,"That person is already a recruiter! You must demote them first.\n\r");
		return;
	}

	//All full, nothing to do
	if (str_cmp(ch->clan->recruiter1,"none") && str_cmp(ch->clan->recruiter2,"none")){
		Cprintf(ch,"That clan already has two recruiters.\n\r");
		return;
	}

	//Loners can recruit themselves
	if (clan->loner)
	{
		Cprintf(ch,"That clan can not have recruiters.\n\r");
		return;
	}

	advance_recruiter(victim);
	Cprintf(ch,"Recruiter 1 assigned.\n\r");
	Cprintf(victim,"You have been chosen by the Gods to recruit new members!\n\r");
	return;
}

void cedit_show(CHAR_DATA *ch, CLAN_DATA *clan) {
	//Display clan info
	Cprintf(ch, "Clan:          %s (%d)\n\r",clan->name, clan->vnum);
	Cprintf(ch, "Whoname:       %s\n\r",clan->who_name);
	Cprintf(ch, "Leader:        %s\n\r", clan->leader);
	Cprintf(ch, "Recruiters:    %s, %s\n\r", clan->recruiter1, clan->recruiter2);
	Cprintf(ch, "Points:        %ld\n\r", clan->clanpoints);
	Cprintf(ch, "Gold:          %ld\n\r", clan->clangold);
	Cprintf(ch, "Upkeep:        %ld gold %ld points\n\r", clan->upkeep_gold, clan->upkeep_points);
	Cprintf(ch, "Most Kills:    %s (%d)\n\r", clan->mostkills, clan->numkills);
	Cprintf(ch, "Most Deaths:   %s (%d)\n\r", clan->mostkilled, clan->numkilled);
	Cprintf(ch, "Spawn Terra:   %d\n\r", clan->spawn_terra);
	Cprintf(ch, "Spawn Dominia: %d\n\r", clan->spawn_dominia);
	Cprintf(ch, "Clan Type:     %s %s.\n\r", (clan->loner)?"independant":"unified",(clan->clanner)?"clan":"guild");
	Cprintf(ch, "Clan Spell:    %s\n\r", skill_table[clan->clanspell].name);
	Cprintf(ch, "Spell Cost:    %d\n\r", clan->spellcost);
	return;
}

void cedit_show_all(CHAR_DATA *ch) {
	CLAN_DATA *clan;
	//For each clan
	for (clan = clan_list; clan != NULL; clan = clan->next){
		//Show clan
		if (clan->name != NULL && clan->name[0] != '\0'){
			Cprintf(ch, "[%-12s]\n\r", clan->name);
			Cprintf(ch, "       Leader: %-10s Recruiter 1: %-10s Recruiter 2: %-10s\n\r", clan->leader, clan->recruiter1, clan->recruiter2);
			Cprintf(ch, "       Points: %ld Upkeep: %ld Gold: %ld Upkeep: %ld\n\r", clan->clanpoints, clan->upkeep_points, clan->clangold, clan->upkeep_gold);
		}
	}
	return;
}

void cedit_spawn(CHAR_DATA *ch, CLAN_DATA *clan, char *vnum) {
	int value;
	ROOM_INDEX_DATA *room;

	//Must be at least 59 to change spawn locations
	if (get_trust(ch) < 59){
		Cprintf(ch, "You do not have the ability yet to change clan spawn rooms.n\r");
		return;
	}

	//Must specify a vnum
	if (vnum == '\0'){
		Cprintf(ch,"Change it to what vnum?\n\r");
		return;
	}

	if (!is_number(vnum)) {
		Cprintf(ch,"Vnum must be a number?\n\r");
		return;
	}

	value = atoi(vnum);
	room = get_room_index(value);
	//Room must exist
	if (room == NULL) {
		Cprintf(ch,"Must respawn in an existing room?\n\r");
		return;
	}

	//Set spawn
	if (room->area->continent == 0) { //Terra
		clan->spawn_terra = value;
		Cprintf(ch,"Terra spawn for %s changed to %s.\n\r", capitalize(clan->name), room->name);
	}else{ //Dominia
		clan->spawn_dominia = value;
		Cprintf(ch,"Dominia spawn for %s changed to %s.\n\r", capitalize(clan->name), room->name);
	}

	return;
}

void cedit_spell_cost(CHAR_DATA *ch, CLAN_DATA *clan, char *cost) {
	int value;

	//Must be at least level 59 to change spell cost
	if (get_trust(ch) < 59){
		Cprintf(ch, "You must be at least level 59 to change clan spellcosts.n\r");
		return;
	}

	//Must specify a cost
	if (cost[0] == '\0'){
		Cprintf(ch,"Change it to what cost?\n\r");
		return;
	}

	//Cost must be a numbet
	if (!is_number(cost)) {
		Cprintf(ch,"Spell cost must be a number\n\r");
		return;
	}

	//No free dp for spells
	value = atoi(cost);
	if (value < 0) {
		Cprintf(ch,"This is a positive MUD, positive numbers only, please.\n\r");
		return;
	}

	//All is well, change the cost
	clan->spellcost = value;
	Cprintf(ch,"Spell cost changed.\n\r");
	return;
}

void cedit_upkeep(CHAR_DATA *ch, CLAN_DATA *clan, char *args) {
	//SYNTAX: cedit upkeep <clan> <amount>
	int value;
	struct tm *time;
	char amount[MAX_INPUT_LENGTH];

	args = one_argument(args, amount);

	//Must have magnitude
	if (amount[0] == '\0') {
		Cprintf(ch, "You must specify the magnitude of the upkeep!\n\r");
		return;
	}

	//Must have type
	if (amount[0] == '\0') {
		Cprintf(ch, "You must specify whether the upkeep is in gold or points!\n\r");
		return;
	}

	//amount must be a number
	if (!is_number(amount)) {
		Cprintf(ch, "Clans do not get an income! Upkeep must be positive or 0!!\n\r");
		return;
	}
	//Verify value
	value = atoi(amount);

	//upkeep must be positive
	if (value < 0) {
		Cprintf(ch, "Clans do not get an income! Upkeep must be positive or 0!!\n\r");
		return;
	}

	if (!str_prefix(args, "gold")) {
		clan->upkeep_gold = value;
	}else if (!str_prefix(args, "points")) {
		clan->upkeep_points = value;
	}else {
		Cprintf(ch, "They are paying how?!\n\r");
		return;
	}
	//Set upkeep
	Cprintf(ch,"Clan upkeep changed to %ld gold and %ld points.\n\r", clan->upkeep_gold, clan->upkeep_points);
	time = localtime(&current_time);
	clan->last_upkeep_month = time->tm_mon;
	return;
}

void cedit_vnum(CHAR_DATA *ch, CLAN_DATA *clan, char *vnum) {
	int value;
	CLAN_DATA *clan2;

	//Only imps can change vnums.
	if (get_trust(ch) < 60){
		Cprintf(ch, "You must be at least level 60 to change clan vnums.n\r");
		return;
	}

	//Must provide a new vnum
	if (vnum == '\0'){
		Cprintf(ch,"Change it to what vnum?\n\r");
		return;
	}

	//Vnum must be a number
	if (!is_number(vnum)) {
		Cprintf(ch,"The vnum must be a number\n\r");
		return;
	}

	value = atoi(vnum);

	//vnum must be positive
	if (vnum < 0) {
		Cprintf(ch,"The vnum must be a positive number\n\r");
		return;
	}

	//vnum must be unique
	for (clan2 = clan_list; clan2 != NULL; clan2 = clan2->next){
		if (value == clan2->vnum){
			Cprintf(ch,"That vnum is already used by %s.\n\r", capitalize(clan2->name));
			return;
		}
	}

	//done
	Cprintf(ch,"Vnum changed.\n\r");
	clan->vnum = value;
	return;
}

void cedit_who_name(CHAR_DATA *ch, CLAN_DATA *clan, char *whoname) {
	//No minors.
	if (get_trust(ch) < 58){
		Cprintf(ch, "You must be at least level 58 to rename clans.\n\r");
		return;
	}

	//Can't be blank, or how would we know?
	if (whoname[0] == '\0'){
		Cprintf(ch,"Rename it to what?\n\r");
		return;
	}

	//done
	Cprintf(ch,"Who name changed.\n\r");
	free_string(clan->who_name);
	clan->who_name = str_dup(whoname);
	return;
}

void new_clan_policy(CLAN_DATA *clan, char *clanpolicy, int hostility) {
	CLAN_POLICY *last_policy, *policy;
	char target_clan[MAX_INPUT_LENGTH];
	policy = alloc_mem(sizeof(struct clan_policy));
	policy->name = &str_empty[0];
	policy->action = &str_empty[0];
	policy->next = NULL;

	clanpolicy = one_argument(clanpolicy, target_clan);
	policy->name = str_dup(cap_str(target_clan));
	policy->action = str_dup(clanpolicy);
	policy->hostility = hostility;
	last_policy = clan->policy;
	if (last_policy == NULL) {
		clan->policy = policy;
		return;
	}
	while (last_policy->next != NULL) {
		last_policy = last_policy->next;
	}
	last_policy->next = policy;
	return;
}

void free_clan_policy(CLAN_POLICY *clan) {
	free_string(clan->action);
	free_string(clan->name);
	free_mem(clan, sizeof(CLAN_POLICY) + 1);
}

//boot up the clan data file
//clandata.txt MUST EXIST in /area/ or the game will refuse to boot!! Even if it is empty it must be there
void read_clandata()
{
	FILE *fp;
	char *word;
	bool fMatch;
	bool done = FALSE;
	CLAN_DATA *clan;

	if ((fp = fopen (CLAN_DATA_FILE, "r")) == NULL)
	{
		bug ("Error reading clan data file! Creating a new one.", 0); //ABORT ABORT!!!!
		clan = new_clan();
		free_string(clan->name);
		clan->name = str_dup("noclan"); //noclan must be present!
		clan->loner = TRUE;
		
		//Fix up the global clan_list
		clan->next = clan_list;
		clan_list = clan;
//		fclose(fp);
		
		write_clandata(); //save
		return;
	}
	for (;;)
	{
		word = feof (fp) ? "End" : fread_word (fp);
		fMatch = FALSE;

		switch (UPPER (word[0])) //read the txt file in
		{
		case '*': //comment in clan code
			fMatch = TRUE;
			fread_to_eol(fp);
			break;			
		case 'C':
			KEY("Clangold",clan->clangold,fread_number(fp));
			if (!str_cmp (word, "Clanname") ) //clanname starts the new clan process
			{
				char* temp_str;
				temp_str = fread_string(fp);
				clan = new_clan(); //the new clan <- most important step
				clan->next = clan_list; //link it to the global clan list
				clan_list = clan;
				free_string(clan->name);
				clan->name = str_dup(temp_str);				
				free_string( temp_str );
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Clanner"))
			{
				if (fread_number(fp) == 1)
					clan->clanner = TRUE;
				else
					clan->clanner = FALSE;
				fMatch = TRUE;
				break;
			}
			KEY("Clanpoints",clan->clanpoints,fread_number(fp));
			KEY("Clanspell",clan->clanspell,skill_lookup(fread_string(fp)));
			KEY("CtrCnt", clan->control_count, fread_number(fp));
			break;
		case 'D':
			KEY("Daily", clan->daily_upkeep, fread_number(fp));
			KEY("Defense", clan->defense_force, fread_number(fp));
			break;
		case 'L':
			KEY("Leader",clan->leader,fread_string(fp));
			if (!str_cmp(word, "Loner"))
			{
				if (fread_number(fp) == 1)
					clan->loner = TRUE;
				else
					clan->loner = FALSE;
				fMatch = TRUE;
				break;
			}		
			break;
		case 'M':
			KEY("Mostkilled",clan->mostkilled,fread_string(fp));
			KEY("Mostkills",clan->mostkills,fread_string(fp));
			break;
		case 'N':
			KEY("Numkilled",clan->numkilled,fread_number(fp));
			KEY("Numkills",clan->numkills,fread_number(fp));
			break;	
		case 'P':
			if (!str_cmp (word, "Policy"))
			{
				char *policy;
				int hostility;
				policy = fread_string(fp);
				hostility = fread_number(fp);
				new_clan_policy(clan, policy, hostility);
				free_string(policy);
				fMatch = TRUE;
				break;
			}
			KEY("Pkills",clan->pkills,fread_number(fp));
			KEY("Pdeaths",clan->pdeaths,fread_number(fp));
			break;			
		case 'R':
			KEY("R1",clan->recruiter1,fread_string(fp));
			KEY("R2",clan->recruiter2,fread_string(fp));		
			break;
		case 'S':
			KEY("Spawnterra",clan->spawn_terra,fread_number(fp));
			KEY("Spawndominia",clan->spawn_dominia,fread_number(fp));
			KEY("Spellcost",clan->spellcost,fread_number(fp));
			KEY("Safetimer", clan->safetimer, fread_number(fp));
			break;
		case 'T':
			KEY("Tempkills",clan->tempkills,fread_number(fp));
			KEY("Tempdeaths",clan->tempdeaths,fread_number(fp));
			break;			
		case 'U':
			KEY("Upkeep",clan->upkeep_points,fread_number(fp));
			KEY("UpkeepGold",clan->upkeep_gold,fread_number(fp));
			KEY("UpkeepMonth",clan->last_upkeep_month,fread_number(fp));
		case 'V':
			KEY("Vnum",clan->vnum,fread_number(fp));
			break;			
		case 'W':
			KEY("Whoname",clan->who_name,fread_string(fp));
			KEY("Wartimer",clan->wartimer, fread_number(fp));
			break;			
		case 'E':
			if (!str_cmp (word, "End") )
			{
				done = TRUE;
				fMatch = TRUE;
				break;
			}
		}

		if (!fMatch)
		{
			bug ("Fread_char: no match.", 0);
			bug ( word, 0 );
			fread_to_eol (fp);
		}

		if (done)
		{
			break;
		}
	}

	fclose (fp);
	return;
}

void write_clandata()
{
	FILE *fp;
	CLAN_DATA *clan;
	int temp;
	CLAN_POLICY *policy;

	if ((fp = fopen (CLAN_DATA_FILE, "w+")) == NULL)
	{
		bug ("write_clandata: fopen", 0);
		return;
	}
	for (clan = clan_list; clan != NULL; clan = clan->next)
	{
		fprintf (fp, "Clanname %s~\n", clan->name );
		fprintf (fp, "Clangold %ld\n", clan->clangold );
		fprintf (fp, "Clanpoints %ld\n", clan->clanpoints );
		if (clan->clanner == TRUE) temp = 1;
		else temp = 0;
		fprintf (fp, "Clanner %d\n", temp);		
		if (clan->loner  == TRUE) temp = 1;
		else temp = 0;
		fprintf (fp, "Loner %d\n", temp);				
		fprintf (fp, "Leader %s~\n", clan->leader);
		fprintf (fp, "R1 %s~\n", clan->recruiter1);
		fprintf (fp, "R2 %s~\n", clan->recruiter2);
		fprintf (fp, "Mostkilled %s~\n", clan->mostkilled);
		fprintf (fp, "Mostkills %s~\n", clan->mostkills);
		fprintf (fp, "Numkilled %d\n", clan->numkilled);
		fprintf (fp, "Numkills %d\n", clan->numkills);
		fprintf (fp, "Pkills %d\n", clan->pkills);		
		fprintf (fp, "Pdeaths %d\n", clan->pdeaths);					
		fprintf (fp, "Tempkills %d\n", clan->tempkills);
		fprintf (fp, "Tempdeaths %d\n", clan->tempdeaths	);
		fprintf (fp, "Upkeep %ld\n", clan->upkeep_points);
		fprintf (fp, "UpkeepGold %ld\n", clan->upkeep_gold);
		fprintf (fp, "UpkeepMonth %d\n", clan->last_upkeep_month);
		fprintf (fp, "Vnum %d\n", clan->vnum);							
		fprintf (fp, "Whoname %s~\n", clan->who_name);
		fprintf (fp, "Spawnterra %d\n", clan->spawn_terra);							
		fprintf (fp, "Spawndominia %d\n", clan->spawn_dominia);
		fprintf (fp, "Clanspell %s~\n", skill_table[clan->clanspell].name);
		fprintf (fp, "Spellcost %d\n", clan->spellcost);
		fprintf (fp, "Wartimer %d\n", clan->wartimer);
		fprintf(fp, "Defense %d\n", clan->defense_force);
		fprintf(fp, "Daily %d\n", clan->daily_upkeep);
		fprintf(fp, "CtrCnt %d\n", clan->control_count);
		fprintf(fp, "Safetimer %d\n", clan->safetimer);
		policy = clan->policy;
		while (policy != NULL) {
			fprintf (fp, "Policy %s %s~ %d\n", policy->name, policy->action, policy->hostility);
			policy = policy->next;
		}
	}
	fprintf (fp, "End\n");

	fclose (fp);
	return;
}

//set a player to a certain clan/guild -- used in guild, retire, and elsewhere
void set_clan(CHAR_DATA *ch, char *clanname)
{
	//logic checks
	if (ch == NULL) return;
	if (clanname[0] == '\0') return;
	ch->clan = clan_lookup(clanname);
	return;
}

//checks all clans for upkeep,
void clanupkeep() {
	CLAN_DATA *clan;
	struct tm *time;
	char noteText[MAX_NOTE_TEXT];

	/*Get current time/date*/
	time = localtime(&current_time);
	for (clan = clan_list; clan != NULL; clan = clan->next) {
		//Don't bother with clans without upkeep.
		if (clan->upkeep_gold == 0 && clan->upkeep_points == 0) {
			continue;
		}
		//Skip clans that have paid their dues for the month/
		if (clan->last_upkeep_month == time->tm_mon) {
			continue;
		}
		clan->clanpoints -= clan->upkeep_points;
		clan->clangold -= clan->upkeep_gold;
		clan->last_upkeep_month = time->tm_mon;
		/*If they are in debt, call in the debt collectors!*/
		if (clan->clanpoints < 0 || clan->clangold < 0) {
			sprintf(noteText, "The clan %s have failed to pay their monthly upkeep.\n\r"
					"They are now in debt for %ld gold and %ld clan points.\n\r\n\r"
					"Sincerely,\n\r\n\r"
					"The Debt Collector of the Clan Hall Janitorial Union\n\r", clan->name, (clan->clangold < 0)?clan->clangold*-1:0, (clan->clanpoints < 0)?clan->clanpoints*-1:0);
			make_note(getClanBoard(clan)->name, "{RDebt Collector{x", "52 IMM", "{RUpkeep Overdue!{x", 90, noteText);
		}
	}
}

void do_siege(CHAR_DATA *ch, char *argument) {

	CHAR_DATA *warman;
	
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int defence_force_size;
	
	
	
	argument = one_argument(argument, arg1);
	
	if ((!is_clan(ch) || ch->clan->loner) && !IS_IMMORTAL(ch))
		Cprintf(ch, "Huh?\n\r");
	
	if (arg1[0] == '\0')
		{
			Cprintf(ch, "What war-time activities did you have in mind?\n\r");
			return;
		}
	
	//first, find out if their warman lives, and on this continent
	warman = find_warleader(ch);
	
	if (warman == NULL) {
		Cprintf(ch, "You can take no war-time actions if your war leader is dead.\n\r");
		return;
	}
	
	if (!str_prefix(arg1, "banner"))
	{
			//is the warleader even in the room?
			
			if (ch->in_room != warman->in_room) {
				Cprintf(ch, "You have to collect your war banner from your clan's war leader.\n\r");
				return;
			}
			
			
			//make sure counter is set on clan
			if (ch->clan->wartimer > 0) {
				Cprintf(ch, "Your clan already has on-going war activities.  Try again later.\n\r");
				return;
			}
			
			//issue war banner
			obj = create_object(get_obj_index(OBJ_VNUM_CLAN_WARBANNER), ch->level); 
			
			free_string(obj->short_descr);
			sprintf(buf, "a %s war banner", ch->clan->name);
			obj->short_descr = str_dup(buf);
			
			free_string(obj->description);
			sprintf(buf, "A %s war banner has been planted here.", ch->clan->name);
			obj->description = str_dup(buf);
			
			obj_to_char(obj, ch);
			act("$n hands you your clan war banner.", warman, 0, ch, TO_VICT, POS_RESTING);
			act("$n passes $N your clan war banner.", warman, 0, ch, TO_NOTVICT, POS_RESTING);
			
			//set counter
			ch->clan->wartimer = 20;
			return;
	} else if (!str_prefix(arg1, "area")) {
		
		//check if they have a war banner
		bool obj_found = FALSE;
		DESCRIPTOR_DATA *d;
		int i;
		
		for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		{
			if (obj != NULL && obj->pIndexData->vnum == OBJ_VNUM_CLAN_WARBANNER)
			{
				obj_found = TRUE;
				break;
			}
		}
		
		if (!obj_found) {
			Cprintf(ch, "You need to be carrying your clan's war banner to begin a siege.\n\r");
			return;
		}
		
		//check if the area can be controllable, and if it is controlled by a rival clan
		if (ch->in_room->area->clan_control == ch->clan) {
			Cprintf(ch, "You already control this area!  Don't start a civil war!\n\r");
			return;
		}
		
		if (!ch->in_room->area->autocontrol) {
			Cprintf(ch, "You can't simply rile up the locals to gain control of this area.  Find another way.\n\r");
			return;
		}
		
		//take away the item
		unequip_char( ch, obj );
		extract_obj( obj );
		
		//announce the siege (stolen gecho code)
		
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->connected == CON_PLAYING)
			{
				Cprintf(d->character, "{g%s is invading %s!{x\n\r", ch->clan->name, ch->in_room->area->name);
			}
		}
		
		
		//spawn initial mob set
		
		defence_force_size = 5;
		
		if (ch->in_room->area->clan_control != clan_lookup("noclan")) { //i.e. a clan has it
		
			defence_force_size += ch->in_room->area->clan_control->defense_force;
		
		}
				
        for (i = 0; i < defence_force_size; i++) {									
            generate_clan_control_mob(ch, 54, ch->in_room);
        }
	
	
	} else if (!str_prefix(arg1, "defense") || !str_prefix(arg1, "defence")) {
		char arg2[MAX_INPUT_LENGTH];
		
		if (ch->trust < 52) {
			Cprintf(ch, "Only your leadership can assign the use of the clan faction points.\n\r");
			return;
		}
		argument = one_argument(argument, arg2);
		
		if (!str_prefix(arg2, "increase")) {
			if (ch->clan->clanpoints < 50) {
				Cprintf(ch, "You need at least 50 clan points to increase defense.\n\r");
				return;
			}
			Cprintf(ch, "You spend clan resources to send out more troops into the field.\n\r");
			ch->clan->defense_force++;
			ch->clan->daily_upkeep += 50;
			ch->clan->clanpoints -= 50;
			return;
		} else if (!str_prefix(arg2, "decrease")) {
			if (ch->clan->defense_force <=0) {
				ch->clan->defense_force = 0;
				Cprintf(ch, "You already are sending no additional troups out to the front lines.\n\r");
				return;
			}
			ch->clan->defense_force--;
			ch->clan->daily_upkeep = UMAX(ch->clan->daily_upkeep - 50, 0);
			Cprintf(ch, "You call back some troops from the front lines.\n\r");
			return;
				
		} else if (!str_prefix(arg2, "info")) {
			Cprintf(ch, "Your clan current has %d extra troops out in the field, for %d points/day.\n\r", ch->clan->defense_force, ch->clan->defense_force*50);
			return;
		} else {
			Cprintf(ch, "Syntax is \"Siege defense <increase/decrease/info>\"");
			return;
		}
		
	} else {
		Cprintf(ch, "Current options are 'banner', 'area', and 'defense'\n\r");
		return;
	}
	

}

void generate_clan_control_mob(CHAR_DATA *ch, int level, ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA *mob_room = NULL;
	CHAR_DATA *mob = NULL;
	char buf[MAX_STRING_LENGTH];
	//int i = 0;
	int rank = 1;
	//int roll = number_percent();
	int room_vnum;

	// Determine the mob rank and level
	/*if(roll < 50) {
		rank = 1;
		level -= 2;
	}
	else if(roll < 80) {
		rank = 2;
		level += 2;
	}
	else {
		rank = 3;
		level += 4;
	}

	// Find the actual data for this class of assault mob.
	for(i = 0; i < MAX_ASSAULT_MOBS; i++) {
		if(!str_cmp(assault_mobs[i].race_type, race)
		&& assault_mobs[i].rank == rank)
			break;
	}
*/
	// The i-th index should have all the data we need.
	mob = create_mobile(get_mob_index(ASSAULT_MOB_HUMAN)); //later change this to a clan property
	size_mob(ch, mob, level);
	mob->max_hit = mob->max_hit * 3;
	mob->hit = mob->max_hit;
	mob->damroll = mob->damroll + (9 * rank);

	if(mob->short_descr != NULL)
			free_string(mob->short_descr);
	if(mob->name != NULL)
			free_string(mob->name);
	if(mob->long_descr != NULL)
			free_string(mob->long_descr);
	
	if (room->area->clan_control == clan_lookup("noclan")) {
		
		sprintf(buf, "a local peasant");
		mob->short_descr = str_dup(buf);

		sprintf(buf, "local peasant soldier defender");
		mob->name = str_dup(buf);

		sprintf(buf,"{c[DEFENDER]{x A local peasant is here, trying to fight off a clan takeover.\n\r");
		mob->long_descr = str_dup(buf);

	}  else {
		
		sprintf(buf, "a %s soldier", ch->in_room->area->clan_control->name);
		mob->short_descr = str_dup(buf);

		sprintf(buf, "soldier defender %s", ch->in_room->area->clan_control->name);
		mob->name = str_dup(buf);

		sprintf(buf,"{c[DEFENDER]{x A soldier of %s is here, fending off an attack.\n\r",ch->in_room->area->clan_control->name);
		mob->long_descr = str_dup(buf);
	}

	mob->wander_timer = 19;
	mob->rot_timer = 20;

	mob->spec_fun = spec_lookup("spec_control_soldier"); //temporarily this is using the assault spec; that should change

	// Find a random room in the correct area
	room_vnum = number_range(room->area->min_vnum, room->area->max_vnum);
	while(1) {
		mob_room = get_room_index(room_vnum);
		if(mob_room != NULL
		&& !IS_SET(mob_room->room_flags, ROOM_NO_MOB)
		&& !IS_SET(mob_room->room_flags, ROOM_PET_SHOP)
		&& !IS_SET(mob_room->room_flags, ROOM_NO_KILL)
		&& !IS_SET(mob_room->room_flags, ROOM_PRIVATE)
		&& !IS_SET(mob_room->room_flags, ROOM_GODS_ONLY)
		&& !IS_SET(mob_room->room_flags, ROOM_SOLITARY)
		&& !IS_SET(mob_room->room_flags, ROOM_ARENA)
		&& !IS_SET(mob_room->room_flags, ROOM_FERRY))
			break;

		// Pick the next room down, since builders usually
		// start building at the lower vnums
		room_vnum--;

		// Last sanity check, what if no room is found?
		if(room_vnum < room->area->min_vnum) {
			mob_room = NULL;
			break;
		}
	}

	// There has been a big problem with the quest, don't use this mob.
	if(mob_room == NULL) {
		extract_char(mob, FALSE);
		return;
	}

	char_to_room(mob, mob_room);
}

CHAR_DATA *find_warleader(CHAR_DATA *ch) {
	ROOM_INDEX_DATA *room, *candidate_room;
	CHAR_DATA *warman = NULL;
	int i;
	
	if(ch->in_room->area->continent == CONTINENT_TERRA)
	{
		room = get_room_index(ch->clan->spawn_terra);
	} else { //dominia
		room = get_room_index(ch->clan->spawn_dominia);
	}
	
	for (i = room->area->min_vnum; i<room->area->max_vnum; ++i)
	{
		candidate_room = get_room_index(i);
		
		//in case not all vnums are used, or if there are blanks in the middle
		if (candidate_room == NULL)
			continue;
		
		for (warman = candidate_room->people; warman != NULL; warman = warman->next_in_room) {
			
			if (!IS_NPC(warman)) { //looking for a mob; move on if we hit a person
				continue;
			}
			
			if ((warman->spec_fun == spec_lookup("spec_clan_warleader"))) {
				return warman;
			}
			
		}
		
	};
	
	return NULL;
}

void clan_count_control_areas() {

	AREA_DATA *pArea;
	CLAN_DATA *clan;

	for (clan = clan_list; clan != NULL; clan = clan->next) {
		clan->control_count = 0;
	}

	for (pArea = area_first; pArea; pArea = pArea->next) {
		
		pArea->clan_control->control_count++;

	}
	
}
