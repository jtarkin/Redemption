bool do_range_shot_ready(CHAR_DATA * ch,  CHAR_DATA **victim, OBJ_DATA **bow, OBJ_DATA **ammo, char * arg, int * door, int * distance) {
	int max_range = 0;
	char message[MAX_INPUT_LENGTH];

	if ((*ammo = get_eq_char(ch, WEAR_AMMO)) == NULL)
	{
		Cprintf(ch, "You aren't holding any ammunition.\n\r");
		return 0;
	}

	if ((*ammo)->item_type != ITEM_AMMO)
	{
		Cprintf(ch, "You aren't holding any ammunition.\n\r");
		return 0;
	}

	if (((*bow) = get_eq_char(ch, WEAR_RANGED)) == NULL)
	{
		Cprintf(ch, "You aren't wearing a ranged weapon.\n\r");
		return 0;
	}

	if((*bow)->item_type != ITEM_WEAPON || (*bow)->value[0] != WEAPON_RANGED)
	{
		Cprintf(ch, "You need a ranged weapon to shoot!\n\r");
		return 0;
	}

	// Kind of hokey, but match ammo/weapon by name.
	if(!((is_name("crossbow", (*bow)->name) && is_name("bolt", (*ammo)->name)) 
			|| (is_name("bow", (*bow)->name) && is_name("arrow", (*ammo)->name))
			|| (is_name("gun", (*bow)->name) && is_name("bullet", (*ammo)->name)))) {
		Cprintf(ch, "Your ammunition isn't compatible with that weapon.\n\r");
		return 0;
	}


	max_range = (*bow)->extra[2];

	if (arg == '\0') {
		*victim = ch->fighting;
		*distance = 0;
		*door = -1;
	} else {
		*victim = range_finder(ch, arg, max_range, door, distance, FALSE);
	}



	if (*victim == NULL)
	{
		if (ch->fighting != NULL) {
			*victim = ch->fighting;
			*distance = 0;
			*door = -1;
		} else {
			Cprintf(ch, "You can't find them.\n\r");
			return 0;
		}

	}


	if (!can_see(ch, *victim)) {
		Cprintf(ch, "There's no way you can hit someone you can't see.\n\r");
		return 0;
	}

	if (IS_NPC(*victim) &&
			((*victim)->spec_fun == spec_lookup("spec_executioner") || IS_CLAN_GOON((*victim))	|| IS_SET((*victim)->imm_flags,IMM_SUMMON)) && (ch)->fighting == NULL)
	{
		Cprintf(ch, "Your ranged attack fails.\n\r");
		act("$n easily deflects a ranged attack.\n\r", *victim, NULL, NULL, TO_ROOM, POS_RESTING);
		return 0;
	}

	if (is_safe(ch, *victim))
		return 0;

	// make sure that you can't use shoot to break charm
	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == *victim)
	{
		Cprintf(ch, "And hurt your beloved master?!\n\r");
		return 0;
	}

	if (IS_NPC(*victim) &&
			(*victim)->fighting != NULL &&
			!is_same_group(ch, (*victim)->fighting))
	{
		Cprintf(ch, "Kill stealing is not permitted.\n\r");
		return 0;
	}
	/*
			switch (*door)
			{
			case 0:
					Cprintf(ch, "You shoot to the north!\n\r");
					break;
			case 1:
					Cprintf(ch, "You shoot to the east!\n\r");
					break;
			case 2:
					Cprintf(ch, "You shoot to the south!\n\r");
					break;
			case 3:
					Cprintf(ch, "You shoot to the west!\n\r");
					break;
			case 4:
					Cprintf(ch, "You shoot up!\n\r");
					break;
			case 5:
					Cprintf(ch, "You shoot down!\n\r");
					break;
			case -1: 
					Cprintf(ch, "You shoot straight at them.\n\r");
					break;
			default:
					Cprintf(ch, "shoot error: bad direction\n\r");
					break;
			}*/
	if (*door == -1) {
		act("You shoot directly at $N.\n\r", ch, 0, *victim, TO_CHAR, POS_RESTING);
		act("$n shoots directly at $N.\n\r", ch, 0, *victim, TO_NOTVICT, POS_RESTING);
	}else if (*door <-1 && *door >5) {
		Cprintf(ch, "Bad shoot direction.\n\r");
		log_string("Bad shoot direction used.\n\r");
	} else {
		sprintf(message, "You shoot at $N to the %s.\n\r", dir_name[*door]);
		act(message, ch, 0, *victim, TO_CHAR, POS_RESTING);
		sprintf(message, "$n shoots to the %s.", dir_name[*door]);
		act(message, ch, 0, *victim, TO_NOTVICT, POS_RESTING);
	};


	return 1;

}

void finish_ranged_attack(CHAR_DATA * ch, CHAR_DATA * victim, CHAR_DATA * old_ch_fighting, CHAR_DATA *old_victim_fighting, int old_ch_position, int old_victim_position, int door, int victim_dead, int missed, int distance, int sn) {

	char * attack_message;
	char message[MAX_INPUT_LENGTH];

	ch->fighting = old_ch_fighting;
	ch->position = old_ch_position;

	if(victim_dead)
		return;


	if(victim->position > POS_SLEEPING) {
		victim->fighting = old_victim_fighting;
		victim->position = old_victim_position;
	}
	else {
		victim->fighting = NULL;
		victim->position = POS_STANDING;
	}

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_AGGRESSIVE))
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);

	if (!victim_dead
			&& missed
			&& number_percent() < 25
			&& IS_NPC(victim)
	&& victim->fighting == NULL) {
		Cprintf(ch, "You've been spotted by %s!\n\r", victim->short_descr);
		victim->hunting = ch;
		victim->hunt_timer = 10;
	}

	door = reverse_direction(door);

	// Don't tell them if they're surprised
	if (!can_see(victim, ch))
	{
		return;
	}

	//this was supposed to be an sn case statement, but got a weird compiler error
	if (!missed) {
		if (sn == gsn_barrage) {
			attack_message = str_dup("The barrage came from");
		} else if (sn == gsn_marksmanship) {
			attack_message = str_dup("The shot came from");

		} else if (sn == gsn_pindown) {
			attack_message = str_dup("You've been pinned down by a shot from");
		} else if (sn == gsn_bleeder) {
			attack_message = str_dup("You've been grazed by a shot from");
		}
	} else { // we don't care what kind of shot, since it missed, you won't know.
		attack_message = ("The shot came from");
	}
	switch (door)
	{
	case 0:
		Cprintf(victim, "%s the NORTH!\n\r", attack_message);
		break;
	case 1:
		Cprintf(victim, "%s the EAST!\n\r", attack_message);
		break;
	case 2:
		Cprintf(victim, "%s the SOUTH!\n\r", attack_message);
		break;
	case 3:
		Cprintf(victim, "%s the WEST!\n\r", attack_message);
		break;
	case 4:
		Cprintf(victim, "%s UP!\n\r", attack_message);
		break;
	case 5:
		Cprintf(victim, "%s DOWN!\n\r", attack_message);
		break;
	case -1:
	Cprintf(victim,"%s across the room\n\r", attack_message);
	break;
	default:
		Cprintf(victim, "Ranged Shot Error: bad direction\n\r");
		break;
	}

	if (ch->in_room->vnum != victim->in_room->vnum) { //we need to let others know where the shot is from
		sprintf(message, "$n was shot at from the %s.\n\r", dir_name[door]);
		act(message, victim, NULL, NULL, TO_ROOM, POS_RESTING);
	}




	//if they're fighting, they raise their bow and drop their guard for 1 round.
	if (ch->fighting != NULL && number_percent() > (get_skill(ch, sn)/4)) {
		AFFECT_DATA af;
		af.where = TO_AFFECTS;
		af.type = gsn_marksmanship;
		af.level = ch->level;
		af.duration = 0;
		af.modifier = 1;
		af.bitvector = 0;
		af.location = APPLY_NONE;
		affect_to_char(ch, &af); 
		Cprintf(ch, "You dropped your guard to make the shot.\n\r");
		Cprintf(victim, "%s had to drop their guard to make the shot.\n\r", (IS_NPC(ch)?ch->short_descr:ch->name));

	}
	if (!IS_NPC(victim) && ch != victim) {
		ch->no_quit_timer = 3;
		victim->no_quit_timer = 3;
	}


	if (ch->in_room == victim->in_room) {
		damage(ch, victim, 0, gsn_marksmanship, DAM_NONE, FALSE, TYPE_SKILL);
	}
	check_killer(ch, victim);

	return;

}



// Fires a single shot, generally high damage and accuracy.
void
do_ranged_attack(CHAR_DATA * ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL, *old_victim_fighting = NULL;
	CHAR_DATA *old_ch_fighting = NULL;
	OBJ_DATA *bow = NULL;
	OBJ_DATA *ammo = NULL;
	int door;
	//int max_range = 0;
	int chance, distance = 0;
	int dam = 0;
	int victim_dead = FALSE;
	int missed = FALSE, show_damage = TRUE;
	int parameter = TYPE_HIT;
	int spec_chance = 0;
	int spell_level;
	int old_victim_position, old_ch_position;

	one_argument(argument, arg);

	if (arg[0] == '\0' && ch->fighting == NULL)
	{
		Cprintf(ch, "Shoot at whom or what?\n\r");
		return;
	}

	//cool new sub function that abstract everything here before.  Note that it produces its own messages to the player.

	if (!do_range_shot_ready(ch, &victim, &bow, &ammo, arg, &door, &distance))
		return;



	WAIT_STATE(ch, PULSE_VIOLENCE);
	old_victim_fighting = victim->fighting;
	old_victim_position = victim->position;
	old_ch_fighting = ch->fighting;
	old_ch_position = ch->position;

	ammo->weight = ammo->weight - 2;
	ch->carry_weight -= 2;
	if(ammo->weight < 2) {
		act("Your $p has been used up.", ch, ammo, NULL, TO_CHAR, POS_RESTING);
		extract_obj(ammo);
	}

	// Determine the chance to hit.
	chance = (get_skill(ch, gsn_marksmanship) / 2);
	chance += get_curr_stat(ch, STAT_DEX);
	chance += get_ranged_hitroll(ch);
	chance += (victim->size - ch->size) * 3;
	chance -= distance * 5;
	if(chance > 95)
		chance = 95;

	dam = dice(bow->value[1], bow->value[2]);
	dam += dice(ammo->value[0], ammo->value[1]);
	dam += (dam * get_flag_damage_modifiers(ch, victim, bow, attack_table[bow->value[3]].damage)) / 100;

	dam += get_ranged_damroll(ch);
	dam += get_ranged_hitroll(ch) / 5;
	dam *= (float)get_skill(ch, gsn_marksmanship) / 100;

	spell_level = ammo->value[3];

	if (number_percent() < chance) { // a possible hit
		if ((ch->fighting == victim) && (number_percent() < get_skill(victim, gsn_dodge) / 4)) { //they dodge it
			dam = 0;
			check_improve(ch, gsn_marksmanship, TRUE, 2); // you made a good shot, so get to learn as if you hit
			missed = TRUE;
			Cprintf(ch, "%s dodges a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
			Cprintf(victim, "You nimbly dodge a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
		} else { // they don't dodge
			check_improve(ch, gsn_marksmanship, TRUE, 2);

		}
	} else { // you missed
		dam = 0;
		check_improve(ch, gsn_marksmanship, FALSE, 2);
		missed = TRUE;
		
	}

	// Special case, magical ammo doesn't do physical damage.
	if (ammo->value[0] < 1 || ammo->value[1] < 1) {
		dam = 0;
		show_damage = FALSE;
	}

	// Specialize shot
	if(!IS_NPC(ch)
			&& ch->pcdata->specialty == gsn_marksmanship) {

		spec_chance = ch->pcdata->learned[gsn_marksmanship] - 100;
		if(number_percent() < spec_chance) {
			parameter |= TYPE_SPECIALIZED;
			if(show_damage)
				dam = dam + (ch->level / 3);
			else
				spell_level += 6;

		}
	}

	if(show_damage) {
		victim_dead = damage(ch, victim, dam,
				ammo->value[2],
				attack_table[ammo->value[2]].damage,
				show_damage, parameter);
	}

	if (ammo->value[3] != 0
			&& ammo->value[4] != 0
			&& !victim_dead
			&& !missed) {
		obj_cast_spell(ammo->value[4], spell_level, ch, victim, ammo);
	}

	update_pos(victim);

	if(victim->position == POS_DEAD)
		return;

	if(!missed) {
		victim_dead = handle_weapon_flags(ch, victim, bow, gsn_marksmanship); 
	}
	//new finish up common command that appears at the end of every shot function.  replaces that which is below, commented out.

	finish_ranged_attack(ch, victim, old_ch_fighting, old_victim_fighting, old_ch_position, old_victim_position, door, victim_dead, missed, distance, gsn_marksmanship);

	return;
}

// do_barrage and fire_barrage are split for historical reasons, and should probably be merged.  There's no reason for
// them to be split.
void
do_barrage(CHAR_DATA * ch, char *argument)
{

	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL/*, *old_victim_fighting = NULL*/;
	/*CHAR_DATA *old_ch_fighting = NULL;*/
	OBJ_DATA *bow = NULL;
	OBJ_DATA *ammo = NULL;
	int door;

	int chance, distance = 0;
	/*int dam = 0;
	int victim_dead = FALSE;
	int missed = FALSE, show_damage = TRUE;
	int parameter = TYPE_HIT;
	int spec_chance = 0;

	int old_victim_position, old_ch_position;*/
	AFFECT_DATA *paf = NULL;
	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_barrage)) == 0)
	{
		Cprintf(ch, "The best you manage is a barrage of used tissues.\n\r");
		return;
	}

	if (is_affected(ch, gsn_marksmanship)) {
		if ((paf = affect_find(ch->affected, gsn_marksmanship)) != NULL) 		{
			if (paf->location == APPLY_NONE && paf->modifier == 0) {
				Cprintf(ch, "You need to wait a moment to gather the momentum to barrage again.\n\r");
				return;
			}
		}
	}	

	if (ch->mana < 50)
	{
		Cprintf(ch, "You can't get up enough energy.\n\r");
		return;
	}


	if (!do_range_shot_ready(ch, &victim, &bow, &ammo, arg, &door, &distance))
		return;

	chance = chance * 7 / 8;

	if (number_percent() < chance)
	{
		AFFECT_DATA af;
		//WAIT_STATE(ch, PULSE_VIOLENCE); // for now leave in prep + fire lags, in their respective spots
		ch->mana -= 50;
		fire_barrage(ch, victim, bow, ammo, door, distance);
		check_improve(ch, gsn_barrage, TRUE, 5);

		af.where = TO_AFFECTS;
		af.type = gsn_marksmanship; //prevents any refire of bleeder/barrage/pindown within 1 tick. shoot is okay
		af.level = ch->level;
		af.duration = 0;
		af.modifier = 0;
		af.bitvector = 0;
		af.location = APPLY_NONE;
		affect_to_char(ch, &af);

	}
	else
	{
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE / 2);
		ch->mana -= 25;

		Cprintf(ch, "You fumble with your ammo and drop it everywhere.\n\r");
		//Cprintf(ch, "You fail to ready a barrage.\n\r");
		check_improve(ch, gsn_barrage, FALSE, 4);

	}
	return;
}


// Fires 2-5 shots at once, but at reduce accuracy and damage.
void
fire_barrage(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *bow, OBJ_DATA *ammo, int door, int distance)
{
	CHAR_DATA *old_victim_fighting = NULL;
	CHAR_DATA *old_ch_fighting = NULL;
	int barrage_size = 0;
	int i, chance = 0;
	int dam = 0;
	int victim_dead = FALSE;
	int missed = FALSE, show_damage = TRUE, one_hit = FALSE;
	int parameter = TYPE_HIT;
	int spell_level;
	int spec_chance = 0;
	int old_victim_position;
	int old_ch_position;
	ROOM_INDEX_DATA *from_room;

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	old_victim_fighting = victim->fighting;
	old_victim_position = victim->position;
	old_ch_fighting = ch->fighting;
	old_ch_position = ch->position;
	barrage_size = number_range(2, 4);

	if(ammo->weight < barrage_size*2)
		barrage_size = ammo->weight / 2;

	ammo->weight -= (2 * barrage_size);
	ch->carry_weight -= (2 * barrage_size);
	if(ammo->weight < 2) {
		act("Your $p has been used up.", ch, ammo, NULL, TO_CHAR, POS_RESTING);
		extract_obj(ammo);
	}

	// Determine the chance to hit.
	chance = get_skill(ch, gsn_marksmanship) / 3;
	chance += (get_skill(ch, gsn_barrage) / 3);    
	chance += (get_curr_stat(ch, STAT_DEX) / 2);
	chance += (get_ranged_hitroll(ch) / 2);
	chance += ((victim->size - ch->size) * 5);
	chance -= distance * 10;
	if(chance > 90)
		chance = 90;

	for(i = 0; i < barrage_size; i++) {
		dam = dice(bow->value[1], bow->value[2]);
		dam += dice(ammo->value[0], ammo->value[1]);
		dam += (dam * get_flag_damage_modifiers(ch, victim, bow, attack_table[bow->value[3]].damage)) / 100;

		dam += get_ranged_damroll(ch);;
		dam += (get_ranged_hitroll(ch) / 10);
		dam *= get_skill(ch, gsn_marksmanship) / 100.0;

		spell_level = ammo->value[3];

		if (number_percent() < chance) { // a possible hit
			if ((ch->fighting == victim) && (number_percent() < get_skill(victim, gsn_dodge)/13)) { //they dodge it
				dam = 0;
				check_improve(ch, gsn_marksmanship, TRUE, 2); // you made a good shot, so get to learn as if you hit
				check_improve(ch, gsn_barrage, TRUE, 2);
				missed = TRUE;
				Cprintf(ch, "%s dodges a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
				Cprintf(victim, "You nimbly dodge a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
			} else { // they don't dodge
				check_improve(ch, gsn_marksmanship, TRUE, 2);
				check_improve(ch, gsn_barrage, TRUE, 2);
			}
		} else { // you missed
			dam = 0;
			check_improve(ch, gsn_marksmanship, FALSE, 2);
			check_improve(ch, gsn_barrage, FALSE, 2);
			missed = TRUE;
		}


		// Special case, magical ammo doesn't do physical damage.
		if (ammo->value[0] < 1 || ammo->value[1] < 1) {
			dam = 0;
			show_damage = FALSE;
		}

		// Specialize shot
		if(!IS_NPC(ch)
				&& ch->pcdata->specialty == gsn_marksmanship) {

			spec_chance = ch->pcdata->learned[gsn_marksmanship] - 100;
			if(number_percent() < spec_chance) {
				parameter |= TYPE_SPECIALIZED;
				if(show_damage)
					dam = dam + (ch->level / 3);
				else
					spell_level += 4;

			}
		}

		if(show_damage) {
			victim_dead = damage(ch, victim, dam,
					ammo->value[2],
					attack_table[ammo->value[2]].damage,
					show_damage, parameter);
		}
		if (ammo->value[3] != 0
				&& ammo->value[4] != 0
				&& !victim_dead
				&& !missed) 
		{
			if (ch != victim && ch->in_room != victim->in_room) //Deal with ranged area spells
			{
				from_room = ch->in_room; //Just teleports them to the victim room then back again, shouldn't break combat
				char_from_room(ch);
				char_to_room(ch,victim->in_room);
				obj_cast_spell(ammo->value[4], spell_level, ch, victim, ammo);
				char_from_room(ch);
				char_to_room(ch,from_room);
			}
			else
			{
				obj_cast_spell(ammo->value[4], spell_level, ch, victim, ammo);
			}
		}

		update_pos(victim);

		if(victim->position == POS_DEAD)
			victim_dead = TRUE;

		if(victim_dead)
			break;

		if(!missed) {
			victim_dead = handle_weapon_flags(ch, victim, bow, gsn_marksmanship); 
			one_hit = TRUE;
		}

		if(victim_dead)
			break;
	}

	finish_ranged_attack(ch, victim, old_ch_fighting, old_victim_fighting, old_ch_position, old_victim_position, door, victim_dead, !one_hit, distance, gsn_barrage);
	return;

}

// do_pindown 
void
do_pindown(CHAR_DATA * ch, char *argument)
{

	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL/*, *old_victim_fighting = NULL*/;
	/*CHAR_DATA *old_ch_fighting = NULL;*/
	OBJ_DATA *bow = NULL;
	OBJ_DATA *ammo = NULL;
	int door;

	int chance, distance = 0;
	/*int dam = 0;
	int victim_dead = FALSE;
	int missed = FALSE, show_damage = TRUE;
	int parameter = TYPE_HIT;
	int spec_chance = 0;

	int old_victim_position, old_ch_position;*/
	AFFECT_DATA *paf = NULL;
	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_pindown)) == 0)
	{
		Cprintf(ch, "You lack the skills to shoot a pin down shot that perfectly.\n\r");
		return;
	}

	if (is_affected(ch, gsn_marksmanship)) {
		if ((paf = affect_find(ch->affected, gsn_marksmanship)) != NULL) 		{
			if (paf->location == APPLY_NONE && paf->modifier == 0) {
				Cprintf(ch, "You need to wait a moment to gather the momentum to pin down again.\n\r");
				return;
			}
		}
	}	

	if ((ch->fighting != NULL && ch->mana < 40) || (ch->fighting == NULL && (ch->mana < 40 || ch->move < 15)))
	{
		Cprintf(ch, "You can't get up enough energy.\n\r");
		return;
	}

	if (!do_range_shot_ready(ch, &victim, &bow, &ammo, arg, &door, &distance))
		return;

	chance = chance * 6 / 7;

	if (number_percent() < chance)
	{
		AFFECT_DATA af;
		CHAR_DATA *old_victim_fighting = NULL;
		CHAR_DATA *old_ch_fighting = NULL;
		int /*i,*/chance = 0;
		int dam = 0;
		int victim_dead = FALSE;
		int missed = FALSE, show_damage = TRUE;
		int parameter = TYPE_HIT;
		int spell_level;
		int spec_chance = 0;
		int old_victim_position;
		int old_ch_position;

		af.where = TO_AFFECTS;
		af.type = gsn_marksmanship; //gsn_marksmanship means they can't shoot again for a tick.  Only 'shoot' itself is not affected
		af.level = ch->level;
		af.duration = 0;
		af.modifier = 0;
		af.bitvector = 0;
		af.location = APPLY_NONE;
		affect_to_char(ch, &af);

		if (ch->fighting == NULL) {
			WAIT_STATE(ch, PULSE_VIOLENCE);
			ch->mana -= 40;
			ch->move -= 15;
		} else {
			WAIT_STATE(ch, 2* PULSE_VIOLENCE);
			ch->mana -= 40;
		}

		old_victim_fighting = victim->fighting;
		old_victim_position = victim->position;
		old_ch_fighting = ch->fighting;
		old_ch_position = ch->position;


		ammo->weight = ammo->weight - 2;
		ch->carry_weight -= 2;
		if(ammo->weight < 2) {
			act("Your $p has been used up.", ch, ammo, NULL, TO_CHAR, POS_RESTING);
			extract_obj(ammo);
		}

		// Determine the chance to hit.
		chance = get_skill(ch, gsn_marksmanship) / 3;
		chance += (get_skill(ch, gsn_pindown) / 3);    
		chance += (get_curr_stat(ch, STAT_DEX) / 2);
		chance += (get_ranged_hitroll(ch) / 2);
		chance += ((victim->size - ch->size) * 5);
		chance -= (distance * 10);
		if(!IS_NPC(ch) && ch->pcdata->specialty == gsn_marksmanship) chance += 10;
		if(chance > 90)
			chance = 90;


		dam = dice(bow->value[1], bow->value[2]);
		dam += dice(ammo->value[0], ammo->value[1]);
		dam += (dam * get_flag_damage_modifiers(ch, victim, bow, attack_table[bow->value[3]].damage)) / 100;

		dam += get_ranged_damroll(ch);
		dam += (get_ranged_hitroll(ch) / 5); 
		dam *= (float)(get_skill(ch, gsn_marksmanship) / 100);

		dam *= (get_skill(ch, gsn_pindown) / 75);

		spell_level = ammo->value[3];

		if (number_percent() < chance) { // a possible hit
			if ((ch->fighting == victim) && (number_percent() < get_skill(victim, gsn_dodge) / 4)) { //they dodge it
				dam = 0;
				check_improve(ch, gsn_marksmanship, TRUE, 2); // you made a good shot, so get to learn as if you hit
				check_improve(ch, gsn_pindown, TRUE, 2);
				missed = TRUE;
				Cprintf(ch, "%s dodges a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
				Cprintf(victim, "You nimbly dodge a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
			} else { // they don't dodge
				check_improve(ch, gsn_marksmanship, TRUE, 2);
				check_improve(ch, gsn_pindown, TRUE, 2);
			}
		} else { // you missed
			dam = 0;
			check_improve(ch, gsn_marksmanship, FALSE, 2);
			check_improve(ch, gsn_pindown, FALSE, 2);
			missed = TRUE;
		}


		// Special case, magical ammo doesn't do physical damage.
		if (ammo->value[0] < 1 || ammo->value[1] < 1) {
			dam = 0;
			show_damage = FALSE;
		}

		// Specialize shot
		if(!IS_NPC(ch)
				&& ch->pcdata->specialty == gsn_marksmanship) {

			spec_chance = ch->pcdata->learned[gsn_marksmanship] - 100;
			if(number_percent() < spec_chance) {
				parameter |= TYPE_SPECIALIZED;
				if(show_damage)
					dam = dam + (ch->level / 3);
				else
					spell_level += 4;

			}
		}

		if(show_damage) {
			victim_dead = damage(ch, victim, dam,
					ammo->value[2],
					attack_table[ammo->value[2]].damage,
					show_damage, parameter);
		}
		if (ammo->value[3] != 0
				&& ammo->value[4] != 0
				&& !victim_dead
				&& !missed) {
			obj_cast_spell(ammo->value[4], spell_level, ch, victim, ammo);
		}

		update_pos(victim);

		if(victim->position == POS_DEAD)
			victim_dead = TRUE;


		if(!missed) {
			victim_dead = handle_weapon_flags(ch, victim, bow, gsn_marksmanship); 
		}

		if (!victim_dead && !missed) {
			chance = number_range(1, 2); // we'll just reuse chance now

			act("You pin down $N, sinking a shot deep in one of their joints.\n\r", ch, 0, victim, TO_CHAR, POS_RESTING);
			act("$n pins down $N, sinking a shot deep in one of their joints.\n\r", ch, 0, victim, TO_NOTVICT, POS_RESTING);
			act("$n pins you down, painfully restricting your movement.\n\r", ch, 0, victim, TO_VICT, POS_RESTING);
			WAIT_STATE(victim, chance * PULSE_VIOLENCE);
			if (ch->in_room != victim->in_room) {
				Cprintf(ch, "You took careful to aim for the legs.\n\r");
				af.where = TO_AFFECTS;
				af.type = gsn_pindown;
				af.level = ch->level;
				af.duration = 0;
				af.modifier = 1;
				af.bitvector = 0;
				af.location = APPLY_NONE;
				affect_to_char(victim, &af);
			}
		}

		finish_ranged_attack(ch, victim, old_ch_fighting, old_victim_fighting, old_ch_position, old_victim_position, door, victim_dead, missed, distance, gsn_pindown);


		check_improve(ch, gsn_pindown, TRUE, 5);

	}  else { // else didn't actually fire the shot
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE / 2);
		if (ch->fighting == NULL) {
			WAIT_STATE(ch, PULSE_VIOLENCE/2);
			ch->mana -= 20;
			ch->move -=7;
		} else {
			WAIT_STATE(ch, PULSE_VIOLENCE);
			ch->mana -= 20;
		}

		Cprintf(ch, "You accidently shoot yourself in the foot.\n\r");
		check_improve(ch, gsn_pindown, FALSE, 4);

	}
	return;	
}


// do_bleeder 
void
do_bleeder(CHAR_DATA * ch, char *argument)
{

	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim = NULL/*, *old_victim_fighting = NULL*/;
	/*CHAR_DATA *old_ch_fighting = NULL;*/
	OBJ_DATA *bow = NULL;
	OBJ_DATA *ammo = NULL;
	int door;

	int chance, distance = 0;
	/*int dam = 0;
	int victim_dead = FALSE;
	int missed = FALSE, show_damage = TRUE;
	int parameter = TYPE_HIT;
	int spec_chance = 0;

	int old_victim_position, old_ch_position;*/
	AFFECT_DATA *paf = NULL;
	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_bleeder)) == 0)
	{
		Cprintf(ch, "You're more likely to hit them than cause an intentional grazing shot.\n\r");
		return;
	}

	if (is_affected(ch, gsn_marksmanship)) {
		if ((paf = affect_find(ch->affected, gsn_marksmanship)) != NULL) 		{
			if (paf->location == APPLY_NONE && paf->modifier == 0) {
				Cprintf(ch, "You need to wait a moment to gather the momentum to fire a grazing shot again.\n\r");
				return;
			}
		}
	}	

	if ((ch->fighting != NULL && ch->mana < 30) || (ch->fighting == NULL && (ch->mana < 30 || ch->move < 10)))
	{
		Cprintf(ch, "You can't get up enough energy.\n\r");
		return;
	}	

	if (!do_range_shot_ready(ch, &victim, &bow, &ammo, arg, &door, &distance))
		return;

	chance = chance * 7 / 8;

	if (number_percent() < chance)
	{
		AFFECT_DATA af, *paf = NULL;
		CHAR_DATA *old_victim_fighting = NULL;
		CHAR_DATA *old_ch_fighting = NULL;
		int /*i,*/ chance = 0;
		int dam = 0;
		int victim_dead = FALSE;
		int missed = FALSE, show_damage = TRUE;
		int parameter = TYPE_HIT;
		int spell_level;
		int spec_chance = 0;
		int old_victim_position;
		int old_ch_position;


		af.where = TO_AFFECTS;
		af.type = gsn_marksmanship; //prevents any refire of bleeder/barrage/pindown within 1 tick. shoot is okay
		af.level = ch->level;
		af.duration = 0;
		af.modifier = 0;
		af.bitvector = 0;
		af.location = APPLY_NONE;
		affect_to_char(ch, &af); 

		if (ch->fighting == NULL) {
			WAIT_STATE(ch, skill_table[gsn_bleeder].beats);
			ch->mana -= 30;
			ch->move -=10;
		} else {
			WAIT_STATE(ch, 2* skill_table[gsn_bleeder].beats);
			ch->mana -= 30;
		}

		old_victim_fighting = victim->fighting;
		old_victim_position = victim->position;
		old_ch_fighting = ch->fighting;
		old_ch_position = ch->position;


		ammo->weight = ammo->weight - 2;
		ch->carry_weight -= 2;
		if(ammo->weight < 2) {
			act("Your $p has been used up.", ch, ammo, NULL, TO_CHAR, POS_RESTING);
			extract_obj(ammo);
		}

		// Determine the chance to hit.
		chance = get_skill(ch, gsn_marksmanship) / 3;
		chance += (get_skill(ch, gsn_bleeder) / 3 );    
		chance += (get_curr_stat(ch, STAT_DEX) / 2);
		chance += (get_ranged_hitroll(ch) / 2);
		chance += ((victim->size - ch->size) * 5);
		chance -= (distance * 10);
		if(chance > 90)
			chance = 90;


		dam = dice(bow->value[1], bow->value[2]);
		dam += dice(ammo->value[0], ammo->value[1]);
		dam += (dam * get_flag_damage_modifiers(ch, victim, bow, attack_table[bow->value[3]].damage)) / 100;

		dam += get_ranged_damroll(ch);
		dam += (get_ranged_hitroll(ch)) / 5; 
		dam *= (get_skill(ch, gsn_marksmanship) / 150);

		dam *= (get_skill(ch, gsn_bleeder) / 150); //bleeder arrows really don't hurt much at first

		spell_level = ammo->value[3];

		if (number_percent() < chance) { // a possible hit
			if ((ch->fighting == victim) && (number_percent() < get_skill(victim, gsn_dodge)/5)) { //they  dodge it
				dam = 0;
				check_improve(ch, gsn_marksmanship, TRUE, 2); // you made a good shot, so get to learn as if you hit
				check_improve(ch, gsn_bleeder, TRUE, 2);
				missed = TRUE;
				Cprintf(ch, "%s dodges a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
				Cprintf(victim, "You nimbly dodge a shot!\n\r", (IS_NPC(victim)?victim->short_descr:victim->name));
			} else { // they don't dodge
				check_improve(ch, gsn_marksmanship, TRUE, 2);
				check_improve(ch, gsn_bleeder, TRUE, 2);

			}
		} else { // you missed
			dam = 0;
			check_improve(ch, gsn_marksmanship, FALSE, 2);
			check_improve(ch, gsn_bleeder, FALSE, 2);
			missed = TRUE;
		}


		// Special case, magical ammo doesn't do physical damage.
		if (ammo->value[0] < 1 || ammo->value[1] < 1) {
			dam = 0;
			show_damage = FALSE;
		}

		// Specialize shot
		if(!IS_NPC(ch)
				&& ch->pcdata->specialty == gsn_marksmanship) {

			spec_chance = ch->pcdata->learned[gsn_marksmanship] - 100;
			if(number_percent() < spec_chance) {
				parameter |= TYPE_SPECIALIZED;
				if(show_damage)
					dam = dam + (ch->level / 3);
				else
					spell_level += 4;

			}
		}

		if(show_damage) {
			victim_dead = damage(ch, victim, dam,
					ammo->value[2],
					attack_table[ammo->value[2]].damage,
					show_damage, parameter);
		}
		if (ammo->value[3] != 0
				&& ammo->value[4] != 0
				&& !victim_dead
				&& !missed) {
			obj_cast_spell(ammo->value[4], spell_level, ch, victim, ammo);
		}

		update_pos(victim);

		if(victim->position == POS_DEAD)
			victim_dead = TRUE;


		if(!missed) {
			victim_dead = handle_weapon_flags(ch, victim, bow, gsn_marksmanship); 
		}

		if (!victim_dead && !missed) {

			act("Your shot intentionally grazes $N, blood runs freely.", ch, 0, victim, TO_CHAR, POS_RESTING);
			act("$n's shot grazes $N, causing blood to flow.", ch, 0, victim, TO_NOTVICT, POS_RESTING);
			act("$n grazes you with a shot.  It doesn't hurt much, but you're losing blood fast.", ch, 0, victim, TO_VICT, POS_RESTING);
			DAZE_STATE(victim, PULSE_VIOLENCE);
			if ((paf = affect_find(victim->affected, gsn_bleeder)) == NULL) {
				af.where = TO_AFFECTS;
				af.type = gsn_bleeder;
				af.level = ch->level;
				af.duration = 1;
				af.modifier = ch->level / 2;
				af.bitvector = 0;
				af.location = APPLY_NONE;
				affect_to_char(victim, &af);
			} else { //already hit by it
				paf->duration = 1;
				paf->modifier = UMAX(ch->level / 2, af.modifier);
			}

		}

		finish_ranged_attack(ch, victim, old_ch_fighting, old_victim_fighting, old_ch_position, old_victim_position, door, victim_dead, missed, distance, gsn_bleeder);


		check_improve(ch, gsn_bleeder, TRUE, 5);


	}  else { // else didn't actually fire the shot
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE / 2);
		if (ch->fighting == NULL) {
			WAIT_STATE(ch, PULSE_VIOLENCE/2);
			ch->mana -= 20;
			ch->move -=7;
		} else {
			WAIT_STATE(ch, PULSE_VIOLENCE);
			ch->mana -= 20;
		}

		Cprintf(ch, "You accidently shoot yourself in the foot.\n\r");
		check_improve(ch, gsn_bleeder, FALSE, 4);

	}
	return;	
}
