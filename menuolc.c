#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "menuolc.h"
#include "olc.h"
#include "olc_act_area.h"
#include "olc_act_room.h"
#include "utils.h"


/****************************************************************/
/* lookup table for olc menu */
/****************************************************************/
const olc_level_table level_table[] =
{
   /* main menus */
/* { level,       func }, */
   { MAIN,        olc_main },
   { PERSONAL,    olc_personal },
   { AREA,        olc_area },
   { ROOM,        olc_room },
   { MOB,         olc_mob },
   { OBJ,         olc_obj },
   { RESET,       olc_reset },
   { HELP,        olc_help },

   /* room sub menus */
   { ROOM_MOD,    olc_room_mod },
   { ROOM_FLAGS,  olc_room_flags },
   { DOOR,        olc_door },
   { DOOR_MOD,    olc_door_mod },
   { ROOM_SEC,    olc_room_sec },

   /* mob sub menus */
   { MOB_MOD,     olc_mob_mod },
   { MOB_ATT,     olc_mob_att },
   { MOB_ACT,     olc_mob_act },
   { MOB_IMM,     olc_mob_imm },
   { MOB_RES,     olc_mob_res },
   { MOB_VULN,    olc_mob_vuln },
   { MOB_PART,    olc_mob_part },
   { MOB_RACE,    olc_mob_race },
   { MOB_POS,     olc_mob_pos },
   { MOB_SIZE,    olc_mob_size },
   { MOB_AFF,     olc_mob_aff },
   { MOB_SPEC,    olc_mob_spec },
   { MOB_SHOP,    olc_mob_shop },
   { MOB_OFF,     olc_mob_off },
   { MOB_FORM,    olc_mob_form },
   { MOB_SEX,     olc_mob_sex },

   /* obj sub menus */
   { OBJ_MOD,     olc_obj_mod },
   { OBJ_TYPE,    olc_obj_type },
   { OBJ_FLAGS,   olc_obj_flags },
   { OBJ_WEAR,    olc_obj_wear },
   { OBJ_VALUE,   olc_obj_value },
   { OBJ_AFF,     olc_obj_aff },
   { OBJ_SPELL,   olc_obj_spell },
   { OBJ_MATIRIAL, olc_obj_matirial },
   { OBJ_CON,     olc_obj_con },

   /* end of table */
   { END_OF_TABLE, 0 }
};

/***********************************************************/
/* The entry command, this gets you in */
/***********************************************************/
void do_edit( CHAR_DATA *ch, char *argument )
{
   CHAR_DATA* mob;
   bool found;

   SET_BIT( ch->wiznet, WIZ_OLC );
   ch->pcdata->olc_depth = MAIN;
   Cprintf(ch, "You have entered olc.\n\r");
   show_olc_list( ch, MAIN, FALSE );
   ch->desc->pAEdit = ch->in_room->area;
   ch->desc->pREdit = ch->in_room;
   ch->desc->pOEdit = ch->in_room->contents->pIndexData;

   mob = ch->next_in_room;
   found = FALSE;
   while( ( mob != NULL ) && !found )
   {
      if( IS_NPC( mob ) )
      {
         ch->desc->pMEdit = mob->pIndexData;
         found = TRUE;
      }
      else
      {
         mob = mob->next_in_room;
      }
   }
   if( !found )
   {
      ch->desc->pMEdit = get_mob_index( MOB_VNUM_FIDO );
   }
}

/***********************************************************/
/* exit call, this gets you out
 * ALL functions that get you out of
 * olc MUST use this function */
/***********************************************************/
void olc_exit( CHAR_DATA *ch )
{
   REMOVE_BIT( ch->wiznet, WIZ_OLC );
   ch->pcdata->olc_depth = -1;
   Cprintf(ch, "You return back to the game.\n\r");
   edit_done( ch );
}


/***********************************************************/
/* menu command parser
 * this thing figures out where you are
 * and pipse the commands off to the
 * right spot */
/***********************************************************/
void interp_olc( CHAR_DATA *ch, char *arg )
{
   char command[MAX_INPUT_LENGTH];
   char logline[MAX_INPUT_LENGTH];
   char* argument;
   int cmd;
   bool found;

   argument = arg;

   /*
    * Strip leading spaces.
    */
   while ( isspace(*argument) )
      argument++;

   strcpy( logline, argument );
   argument = one_argument( argument, command );

   /* Find the olc_level in the table */
   found = FALSE;
   for( cmd = 0; level_table[cmd].level != END_OF_TABLE; cmd++ )
   {
      if( level_table[cmd].level == ch->pcdata->olc_depth )
      {
         found = TRUE;
         break;
      }
   }

   /* Log and snoop. */
   if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
     ||   fLogAll )
   {
      sprintf( log_buf, "Log %s: %s", ch->name, logline );
      wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
      log_string("%s", log_buf);
   }

   if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
   {
      write_to_buffer(ch->desc->snoop_by, "% ");
      write_to_buffer(ch->desc->snoop_by, logline);
      write_to_buffer(ch->desc->snoop_by, "\n\r");
   }

   if( command[0] == '/' || command[0] == '#' )
   {
      interpret( ch, arg + 1 );
      return;
   }

   if( found )
   {
      /* do the OLC tango, fucking menu system */
      /* screwed up, arg and argument are ass backwards, they each
       * have eachoethers name */
      (*level_table[cmd].func)( ch, command, arg, argument );
   }
   else
   {
      Cprintf(ch, "Huh?\n\r");
   }

   return;
}

/*************************************************************/
/* MAIN entry point to the menu system, bottom level */
/*************************************************************/
void olc_main( CHAR_DATA *ch, char *command, char *argument, char *arg )
{

   if( !str_prefix( command, "settings" ) || atoi(command) == 1 )
   {
      show_olc_list( ch, PERSONAL, FALSE );
      ch->pcdata->olc_depth = PERSONAL;
   }
   else if( !str_prefix( command, "area" ) || atoi(command) == 2 )
   {
      show_olc_list( ch, AREA, FALSE );
      ch->pcdata->olc_depth = AREA;
   }
   else if( !str_prefix( command, "room" ) || atoi(command) == 3 )
   {
      show_olc_list( ch, ROOM, FALSE );
      ch->pcdata->olc_depth = ROOM;
   }
   else if( !str_prefix( command, "mob" ) || atoi(command) == 4 )
   {
      show_olc_list( ch, MOB, FALSE );
      ch->pcdata->olc_depth = MOB;
   }
   else if( !str_prefix( command, "object" ) || atoi(command) == 5 )
   {
      show_olc_list( ch, OBJ, FALSE );
      ch->pcdata->olc_depth = OBJ;
   }
   else if( !str_prefix( command, "resets" ) || atoi(command) == 6 )
   {
      show_olc_list( ch, RESET, FALSE );
      ch->pcdata->olc_depth = RESET;
   }
   else if( !str_prefix( command, "help" ) || atoi(command) == 7 )
   {
      show_olc_list( ch, HELP, FALSE );
      ch->pcdata->olc_depth = HELP;
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 8 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, MAIN, TRUE );
   }
}

/*****************************************************************/
/* PERSONAL menus */
/*****************************************************************/
void olc_personal( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "room" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "mob" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "object" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "auto" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "door" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "brief" ) || atoi(command) == 6 )
   {
      if( ch->pcdata->brief == TRUE )
      {
         ch->pcdata->brief = FALSE;
         Cprintf(ch, "Brief toggled off, menus will now be displayed automaticaly\n\r");
      }
      else
      {
         ch->pcdata->brief = TRUE;
         Cprintf(ch, "Breif toggled on, menus will not be displayed automaticaly\n\r");
      }
   }
   else if( !str_prefix( command, "help" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "This option is not implemented yet\n\r");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 8 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 9 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, PERSONAL, TRUE );
   }
}

/****************************************************************/
/* AREA menus */
/****************************************************************/
void olc_area( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
/*   int j; */
   bool valid;
   char file[MAX_INPUT_LENGTH];
   AREA_DATA* pArea;

   if( !str_prefix( command, "select" ) || atoi(command) == 1 )
   {
      if( !is_number(arg) )
      {
         Cprintf(ch, "Syntax: select <vnum>\n\r");
      }
      else if( !IS_BUILDER( ch, ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You dont have the security to select that area\n\r");
      }
      else
      {
         do_aedit( ch, argument );
      }
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 2 )
   {
      aedit_show( ch, "" );
   }
   else if( !str_prefix( command, "reset" ) || atoi(command) == 3 )
   {
      if( IS_BUILDER( ch, ch->desc->pAEdit ) )
      {
         aedit_reset( ch, "" );
      }
      else
      {
         Cprintf(ch, "You dont haev the security to reset this area\n\r");
      }
   }
   else if( !str_prefix( command, "new" ) || atoi(command) == 4 )
   {
      if( !IS_NPC( ch ) && ( ch->pcdata->security < 9 ) )
      {
         Cprintf(ch, "You do not have the security to create a new aera\n\r");
      }
      else
      {
         aedit_create( ch, "" );
      }
   }
   else if( !str_prefix( command, "save" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "save area\n\r");
   }
   else if( !str_prefix( command, "rename" ) || atoi(command) == 6 )
   {
      if( arg[0] == '\0' || arg == NULL )
      {
         Cprintf(ch, "Syntax: rename <newname>\n\r");
      }
      else if( !IS_BUILDER( ch, ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to rename this area\n\r");
      }
      else
      {
         valid = TRUE;

         for( i = 0; arg[i] != '\0'; i++ )
         {
            if( !isalnum( arg[i] ) )
            {
               valid = FALSE;
            }
         }
         if( valid )
         {
            aedit_name( ch, arg );
            strcpy( file, arg );
            file[8] = '\0';
            aedit_file( ch, file );
         }
      }
   }
   else if( !str_prefix( command, "credits" ) || atoi(command) == 7 )
   {
      if( arg[0] == '\0' || arg == NULL )
      {
         Cprintf(ch, "Syntax: credits <credits>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to chaneg the credits\n\r");
      }
      else
      {
         aedit_credits( ch, arg );
      }
   }
   else if( !str_prefix( command, "security" ) || atoi(command) == 8 )
   {
      if( arg == NULL || !is_number(arg) || arg[0] == '\0' )
      {
         Cprintf(ch, "Syntax: security <level>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to modify the security\n\r");
      }
      else
      {
         aedit_security( ch, arg );
      }
   }
   else if( !str_prefix( command, "builder" ) || atoi(command) == 9 )
   {
      if( arg == NULL || arg[0] == '\0' )
      {
         Cprintf(ch, "Syntax: builder <name|all>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to add/remove builders\n\r");
      }
      else
      {
         aedit_builder( ch, arg );
      }
   }
   else if( !str_prefix( command, "age" ) || atoi(command) == 10 )
   {
      if( arg == NULL || arg[0] == '\0' || !is_number(arg) )
      {
         Cprintf(ch, "Syntax: age <newage>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to change the age\n\r");
      }
      else
      {
         aedit_age( ch, arg );
      }
   }
   else if( !str_prefix( command, "lower" ) || atoi(command) == 11 )
   {
      if( arg == NULL || arg[0] == '\0' || !is_number(arg) )
      {
         Cprintf(ch, "Syntax: lower <vnum>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to change the vnum range\n\r");
      }
      else
      {
         aedit_lvnum( ch, arg );
      }
   }
   else if( !str_prefix( command, "upper" ) || atoi(command) == 12 )
   {
      if( arg == NULL || arg[0] == '\0' || !is_number(arg) )
      {
         Cprintf(ch, "Syntax: upper <vnum>\n\r");
      }
      else if( !IS_BUILDER( ch , ch->desc->pAEdit ) )
      {
         Cprintf(ch, "You do not have the security to change the vnum range\n\r");
      }
      else
      {
         aedit_uvnum( ch, arg );
      }
   }
   else if( !str_prefix( command, "list" ) || atoi(command) == 13 )
   {
      pArea = area_first;
      for( i = 0; i < (top_area + 1); i++ )
      {
         if( pArea != NULL )
         {
            Cprintf(ch, "[%5d] %-30s", pArea->vnum, pArea->credits);
            pArea = pArea->next;
         }
         if( pArea != NULL )
         {
            Cprintf(ch, " [%5d] %-30s\n\r", pArea->vnum, pArea->credits);
            pArea = pArea->next;
            i++;
         }
         else
         {
            Cprintf(ch, "\n\r");
         }
      }
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 14 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 15 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, AREA, TRUE );
   }
}

/***************************************************************/
/* ROOM menus */
/***************************************************************/
void olc_room( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "select" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "select");
   }
   else if( !str_prefix( command, "list" ) || atoi(command) == 2 )
   {
      redit_rlist( ch, "" );
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "info");
   }
   else if( !str_prefix( command, "create" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "create");
   }
   else if( !str_prefix( command, "copy" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "copy");
   }
   else if( !str_prefix( command, "modify" ) || atoi(command) == 6 )
   {
      ch->pcdata->olc_depth = ROOM_MOD;
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
   else if( !str_prefix( command, "delete" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "delete");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 8 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 9 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, ROOM, FALSE );
   }
}

void olc_room_mod( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "name" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "name");
   }
   else if( !str_prefix( command, "description" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "description");
   }
   else if( !str_prefix( command, "extended" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "extended");
   }
   else if( !str_prefix( command, "remove" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "remove");
   }
   else if( !str_prefix( command, "flags" ) || atoi(command) == 5 )
   {
      ch->pcdata->olc_depth = ROOM_FLAGS;
      show_olc_list( ch, ROOM_FLAGS, FALSE );
   }
   else if( !str_prefix( command, "door" ) || atoi(command) == 6 )
   {
      ch->pcdata->olc_depth = DOOR;
      show_olc_list( ch, DOOR, FALSE );
   }
   else if( !str_prefix( command, "sector" ) || atoi(command) == 7 )
   {
      ch->pcdata->olc_depth = ROOM_SEC;
      show_olc_list( ch, ROOM_SEC, FALSE );
   }
   else if( !str_prefix( command, "heal" ) || atoi(command) == 8 )
   {
      Cprintf(ch, "heal");
   }
   else if( !str_prefix( command, "mana" ) || atoi(command) == 9 )
   {
      Cprintf(ch, "mana");
   }
   else if( !str_prefix( command, "owner" ) || atoi(command) == 10 )
   {
      Cprintf(ch, "owner");
   }
   else if( !str_prefix( command, "done" ) || atoi(command) == 11 )
   {
      ch->pcdata->olc_depth = ROOM;
      show_olc_list( ch, ROOM, FALSE );
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 12 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 13 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
}

void olc_room_flags( CHAR_DATA *ch, char *command, char *agument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( room_flags[i].name != NULL )
   {
      if( !str_prefix( command, room_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Flag %s toggled\n\r", attack_table[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = ROOM_MOD;
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, ROOM_FLAGS, FALSE );
   }
}

void olc_door( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "north" )
    || !str_prefix( command, "east" )
    || !str_prefix( command, "south" )
    || !str_prefix( command, "west" )
    || !str_prefix( command, "up" )
    || !str_prefix( command, "down" ) )
   {
      Cprintf(ch, "Selected:  %s\n\r", command);
      ch->pcdata->olc_depth = DOOR_MOD;
      show_olc_list( ch, DOOR_MOD, FALSE );
   }
   else
   {
      ch->pcdata->olc_depth = ROOM_MOD;
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
}

void olc_door_mod( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "mode" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "mode");
   }
   else if( !str_prefix( command, "select" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "select");
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "info");
   }
   else if( !str_prefix( command, "destination" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "destination");
   }
   else if( !str_prefix( command, "name" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "name");
   }
   else if( !str_prefix( command, "key" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "key");
   }
   else if( !str_prefix( command, "flags" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "flags");
   }
   else if( !str_prefix( command, "copy" ) || atoi(command) == 8 )
   {
      Cprintf(ch, "copy");
   }
   else if( !str_prefix( command, "remove" ) || atoi(command) == 9 )
   {
      Cprintf(ch, "remove");
   }
   else if( !str_prefix( command, "done" ) || atoi(command) == 10 )
   {
      ch->pcdata->olc_depth = ROOM_MOD;
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 11 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 12 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, DOOR_MOD, FALSE );
   }
}

void olc_room_sec( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( sector_flags[i].name != NULL )
   {
      if( !str_prefix( command, sector_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Sector set to %s\n\r", attack_table[i].name);
         ch->pcdata->olc_depth = ROOM_MOD;
         show_olc_list( ch, ROOM_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = ROOM_MOD;
      show_olc_list( ch, ROOM_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, ROOM_SEC, FALSE );
   }
}

/************************************************************/
/* MOB menus */
/************************************************************/
void olc_mob( CHAR_DATA *ch, char *command , char *argument, char *arg )
{
   if( !str_prefix( command, "select" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "select");
   }
   else if( !str_prefix( command, "list" ) || atoi(command) == 2 )
   {
      redit_mlist( ch, "all" );
   }
   else if( !str_prefix( command, "create" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "create");
   }
   else if( !str_prefix( command, "copy" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "copy");
   }
   else if( !str_prefix( command, "modify" ) || atoi(command) == 5 )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "info");
   }
   else if( !str_prefix( command, "instances" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "instaces");
   }
   else if( !str_prefix( command, "shop" ) || atoi(command) == 8 )
   {
      Cprintf(ch, "shop");
   }
   else if( !str_prefix( command, "delete" ) || atoi(command) == 9 )
   {
      Cprintf(ch, "delete");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 10 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 11 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, MOB, FALSE );
   }
}

void olc_mob_mod( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "name" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "name");
   }
   else if( !str_prefix( command, "level" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "level");
   }
   else if( !str_prefix( command, "short" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "long");
   }
   else if( !str_prefix( command, "long" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "long");
   }
   else if( !str_prefix( command, "description" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "description");
   }
   else if( !str_prefix( command, "hp" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "hp");
   }
   else if( !str_prefix( command, "mana" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "mana");
   }
   else if( !str_prefix( command, "ac" ) || atoi(command) == 8 )
   {
      Cprintf(ch, "ac");
   }
   else if( !str_prefix( command, "attack" ) || atoi(command) == 9 )
   {
      ch->pcdata->olc_depth = MOB_ATT;
      show_olc_list( ch, MOB_ATT, FALSE );
   }
   else if( !str_prefix( command, "damage" ) || atoi(command) == 10 )
   {
      Cprintf(ch, "damage");
   }
   else if( !str_prefix( command, "hitroll" ) || atoi(command) == 11 )
   {
      Cprintf(ch, "hitroll");
   }
   else if( !str_prefix( command, "act" ) || atoi(command) == 12 )
   {
      ch->pcdata->olc_depth = MOB_ACT;
      show_olc_list( ch, MOB_ACT, FALSE );
   }
   else if( !str_prefix( command, "offensive" ) || atoi(command) == 13 )
   {
      ch->pcdata->olc_depth = MOB_OFF;
      show_olc_list( ch, MOB_OFF, FALSE );
   }
   else if( !str_prefix( command, "immunities" ) || atoi(command) == 14 )
   {
      ch->pcdata->olc_depth = MOB_IMM;
      show_olc_list( ch, MOB_IMM, FALSE );
   }
   else if( !str_prefix( command, "resistances" ) || atoi(command) == 15 )
   {
      ch->pcdata->olc_depth = MOB_RES;
      show_olc_list( ch, MOB_RES, FALSE );
   }
   else if( !str_prefix( command, "vulnerabilities" ) || atoi(command) == 16 )
   {
      ch->pcdata->olc_depth = MOB_VULN;
      show_olc_list( ch, MOB_VULN, FALSE );
   }
   else if( !str_prefix( command, "parts" ) || atoi(command) == 17 )
   {
      ch->pcdata->olc_depth = MOB_PART;
      show_olc_list( ch, MOB_PART, FALSE );
   }
   else if( !str_prefix( command, "form" ) || atoi(command) == 18 )
   {
      ch->pcdata->olc_depth = MOB_FORM;
      show_olc_list( ch, MOB_FORM, FALSE );
   }
   else if( !str_prefix( command, "race" ) || atoi(command) == 19 )
   {
      ch->pcdata->olc_depth = MOB_RACE;
      show_olc_list( ch, MOB_RACE, FALSE );
   }
   else if( !str_prefix( command, "sex" ) || atoi(command) == 20 )
   {
      ch->pcdata->olc_depth = MOB_SEX;
      show_olc_list( ch, MOB_SEX, FALSE );
   }
   else if( !str_prefix( command, "positions" ) || atoi(command) == 21 )
   {
      ch->pcdata->olc_depth = MOB_POS;
      show_olc_list( ch, MOB_POS, FALSE );
   }
   else if( !str_prefix( command, "wealth" ) || atoi(command) == 22 )
   {
      Cprintf(ch, "wealth");
   }
   else if( !str_prefix( command, "size" ) || atoi(command) == 23 )
   {
      ch->pcdata->olc_depth = MOB_SIZE;
      show_olc_list( ch, MOB_SIZE, FALSE );
   }
   else if( !str_prefix( command, "alignment" ) || atoi(command) == 24 )
   {
      Cprintf(ch, "align");
   }
   else if( !str_prefix( command, "affects" ) || atoi(command) == 25 )
   {
      ch->pcdata->olc_depth = MOB_AFF;
      show_olc_list( ch, MOB_AFF, FALSE );
   }
   else if( !str_prefix( command, "special" ) || atoi(command) == 26 )
   {
      ch->pcdata->olc_depth = MOB_SPEC;
      show_olc_list( ch, MOB_SPEC, FALSE );
   }
   else if( !str_prefix( command, "done" ) || atoi(command) == 27 )
   {
      ch->pcdata->olc_depth = MOB;
      show_olc_list( ch, MOB, FALSE );
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 28 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 29 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, MOB_MOD, FALSE );
   }
}

void olc_mob_att( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( attack_table[i].name != NULL )
   {
      if( !str_prefix( command, attack_table[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Attack set to %s\n\r", attack_table[i].name);
         ch->pcdata->olc_depth = MOB_MOD;
         show_olc_list( ch, MOB_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_ATT, FALSE );
   }
}

void olc_mob_act( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( act_flags[i].name != NULL )
   {
      if( !str_prefix( command, act_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", act_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_ACT, FALSE );
   }
}

void olc_mob_imm( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( imm_flags[i].name != NULL )
   {
      if( !str_prefix( command, imm_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", imm_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_IMM, FALSE );
   }
}

void olc_mob_res( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( res_flags[i].name != NULL )
   {
      if( !str_prefix( command, res_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", res_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_RES, FALSE );
   }
}

void olc_mob_vuln( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( vuln_flags[i].name != NULL )
   {
      if( !str_prefix( command, vuln_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", vuln_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_VULN, FALSE );
   }
}

void olc_mob_part( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( part_flags[i].name != NULL )
   {
      if( !str_prefix( command, part_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", part_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_PART, FALSE );
   }
}

void olc_mob_form( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( form_flags[i].name != NULL )
   {
      if( !str_prefix( command, attack_table[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", form_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_FORM, FALSE );
   }
}

void olc_mob_race( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( race_table[i].name != NULL )
   {
      if( !str_prefix( command, race_table[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", race_table[i].name);
         ch->pcdata->olc_depth = MOB_MOD;
         show_olc_list( ch, MOB_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_RACE, FALSE );
   }
}

void olc_mob_pos( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( position_flags[i].name != NULL )
   {
      if( !str_prefix( command, position_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", position_flags[i].name);
         ch->pcdata->olc_depth = MOB_MOD;
         show_olc_list( ch, MOB_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_POS, FALSE );
   }
}

void olc_mob_size( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( size_flags[i].name != NULL )
   {
      if( !str_prefix( command, size_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", size_flags[i].name);
         ch->pcdata->olc_depth = MOB_MOD;
         show_olc_list( ch, MOB_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_SIZE, FALSE );
   }
}

void olc_mob_aff( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( affect_flags[i].name != NULL )
   {
      if( !str_prefix( command, affect_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", affect_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_AFF, FALSE );
   }
}

void olc_mob_spec( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( spec_table[i].name != NULL )
   {
      if( !str_prefix( command, spec_table[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", spec_table[i].name);
         ch->pcdata->olc_depth = MOB_MOD;
         show_olc_list( ch, MOB_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !str_prefix( command, "remove" ) || atoi(command) == (i+2) )
   {
      Cprintf(ch, "removed spec\n\r");
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_SPEC, FALSE );
   }
}

void olc_mob_shop( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "copy" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "copy");
   }
   else if( !str_prefix( command, "buy" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "buy");
   }
   else if( !str_prefix( command, "profits" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "profits");
   }
   else if( !str_prefix( command, "hours" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "hours");
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "info");
   }
   else if( !str_prefix( command, "remove" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "remove");
   }
   else if( !str_prefix( command, "done" ) || atoi(command) == 7 )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 8 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 9 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, MOB_SHOP, FALSE );
   }
}

void olc_mob_off( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( off_flags[i].name != NULL )
   {
      if( !str_prefix( command, off_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Bit %s toggled\n\r", off_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = MOB_MOD;
      show_olc_list( ch, MOB_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, MOB_OFF, FALSE );
   }
}

void olc_mob_sex( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   Cprintf(ch, "sex, more sex... ahhhhh\n\r");
   ch->pcdata->olc_depth = MOB_MOD;
   show_olc_list( ch, MOB_MOD, FALSE );
}

/*************************************************************/
/* OBJECT menus */
/*************************************************************/
void olc_obj( CHAR_DATA *ch, char *command , char *argument, char *arg )
{
   if( !str_prefix( command, "select" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "select");
   }
   else if( !str_prefix( command, "list" ) || atoi(command) == 2 )
   {
      redit_olist( ch, "all" );
   }
   else if( !str_prefix( command, "create" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "create");
   }
   else if( !str_prefix( command, "copy" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "copy");
   }
   else if( !str_prefix( command, "modify" ) || atoi(command) == 5 )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !str_prefix( command, "info" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "info");
   }
   else if( !str_prefix( command, "instances" ) || atoi(command) == 7 )
   {
      Cprintf(ch, "instaces");
   }
   else if( !str_prefix( command, "delete" ) || atoi(command) == 8 )
   {
      Cprintf(ch, "delete");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 9 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 10 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, OBJ, FALSE );
   }
}

void olc_obj_mod( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "name" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "name");
   }
   else if( !str_prefix( command, "level" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "level");
   }
   else if( !str_prefix( command, "short" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "short");
   }
   else if( !str_prefix( command, "long" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "long");
   }
   else if( !str_prefix( command, "extended" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "extended");
   }
   else if( !str_prefix( command, "remove" ) || atoi(command) == 6 )
   {
      Cprintf(ch, "remove");
   }
   else if( !str_prefix( command, "material" ) || atoi(command) == 7 )
   {
      ch->pcdata->olc_depth = OBJ_MATIRIAL;
      show_olc_list( ch, OBJ_MATIRIAL, FALSE );
   }
   else if( !str_prefix( command, "type" ) || atoi(command) == 8 )
   {
      ch->pcdata->olc_depth = OBJ_TYPE;
      show_olc_list( ch, OBJ_TYPE, FALSE );
   }
   else if( !str_prefix( command, "flags" ) || atoi(command) == 9 )
   {
      ch->pcdata->olc_depth = OBJ_FLAGS;
      show_olc_list( ch, OBJ_FLAGS, FALSE );
   }
   else if( !str_prefix( command, "wear" ) || atoi(command) == 10 )
   {
      ch->pcdata->olc_depth = OBJ_WEAR;
      show_olc_list( ch, OBJ_WEAR, FALSE );
   }
   else if( !str_prefix( command, "condition" ) || atoi(command) == 11 )
   {
      ch->pcdata->olc_depth = OBJ_CON;
      show_olc_list( ch, OBJ_CON, FALSE );
   }
   else if( !str_prefix( command, "weight" ) || atoi(command) == 12 )
   {
      Cprintf(ch, "weight");
   }
   else if( !str_prefix( command, "cost" ) || atoi(command) == 13 )
   {
      Cprintf(ch, "cost");
   }
   else if( !str_prefix( command, "values" ) || atoi(command) == 14 )
   {
      ch->pcdata->olc_depth = OBJ_VALUE;
      show_olc_list( ch, OBJ_VALUE, FALSE );
   }
   else if( !str_prefix( command, "add" ) || atoi(command) == 15 )
   {
      ch->pcdata->olc_depth = OBJ_AFF;
      show_olc_list( ch, OBJ_AFF, FALSE );
   }
   else if( !str_prefix( command, "delete" ) || atoi(command) == 16 )
   {
      Cprintf(ch, "delete");
   }
   else if( !str_prefix( command, "done" ) || atoi(command) == 17 )
   {
      ch->pcdata->olc_depth = OBJ;
      show_olc_list( ch, OBJ, FALSE );
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 18 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 19 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
}

void olc_obj_con( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   ch->pcdata->olc_depth = OBJ_MOD;
   show_olc_list( ch, OBJ_MOD, FALSE );
}

void olc_obj_matirial( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( material_type[i].name != NULL )
   {
      if( !str_prefix( command, material_type[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Matirial %s set\n\r", material_type[i].name);
         ch->pcdata->olc_depth = OBJ_MOD;
         show_olc_list( ch, OBJ_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_MATIRIAL, FALSE );
   }
}

void olc_obj_type( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( type_flags[i].name != NULL )
   {
      if( !str_prefix( command, type_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Type set to %s\n\r", type_flags[i].name);
         ch->pcdata->olc_depth = OBJ_MOD;
         show_olc_list( ch, OBJ_MOD, FALSE );
      }
      i++;
   }

   if( !str_prefix( command, "cancel" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_TYPE, FALSE );
   }
}

void olc_obj_flags( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( extra_flags[i].name != NULL )
   {
      if( !str_prefix( command, extra_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Extra bit %s toggled\n\r", type_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_TYPE, FALSE );
   }
}

void olc_obj_wear( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( wear_flags[i].name != NULL )
   {
      if( !str_prefix( command, wear_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         Cprintf(ch, "Wear bit %s toggled\n\r", wear_flags[i].name);
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_WEAR, FALSE );
   }
}

void olc_obj_value( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   ch->pcdata->olc_depth = OBJ_MOD;
   show_olc_list( ch, OBJ_MOD, FALSE );
}

void olc_obj_aff( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int found;

   i = 0;
   found = FALSE;
   while( apply_flags[i].name != NULL )
   {
      if( !str_prefix( command, apply_flags[i].name ) || atoi(command) == (i+1) )
      {
         found = TRUE;
         if( !str_prefix( apply_flags[i].name, "spellaffect" ) )
         {
            ch->pcdata->olc_depth = OBJ_SPELL;
            show_olc_list( ch, OBJ_SPELL, FALSE );
         }
         else
         {
            Cprintf(ch, "Affect bit %s set\n\r", apply_flags[i].name);
         }
      }
      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (i+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_AFF, FALSE );
   }
}

void olc_obj_spell( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   int i;
   int j;
   int found;

   i = 0;
   j = 0;
   found = FALSE;
   while( skill_table[i].name != NULL )
   {
      if( !str_prefix( command, skill_table[i].name ) || atoi(command) == (j+1) )
      {
         if( /* skill_table[i].target == TAR_CHAR_DEFENCIVE */
           skill_table[i].target == TAR_OBJ_CHAR_DEF
          || skill_table[i].target == TAR_IGNORE
          || skill_table[i].target == TAR_CHAR_SELF )
         {
            found = TRUE;
            Cprintf(ch, "Spell bit %s toggled\n\r", skill_table[i].name);
         }
      }

      if(/* skill_table[i].target == TAR_CHAR_DEFENCIVE */
        skill_table[i].target == TAR_OBJ_CHAR_DEF
       || skill_table[i].target == TAR_IGNORE
       || skill_table[i].target == TAR_CHAR_SELF )
         j++;

      i++;
   }

   if( !str_prefix( command, "done" ) || atoi(command) == (j+1) )
   {
      ch->pcdata->olc_depth = OBJ_MOD;
      show_olc_list( ch, OBJ_MOD, FALSE );
   }
   else if( !found )
   {
      show_olc_list( ch, OBJ_SPELL, FALSE );
   }
}

/***************************************************************/
/* RESET menus */
/***************************************************************/
void olc_reset( CHAR_DATA *ch, char *command , char *argument, char *arg )
{
   if( !str_prefix( command, "mob" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "mob");
   }
   else if( !str_prefix( command, "object" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "object");
   }
   else if( !str_prefix( command, "door" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "door");
   }
   else if( !str_prefix( command, "all" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "all");
   }
   else if( !str_prefix( command, "room" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "room");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 6 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 7 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, RESET, FALSE );
   }
}

/**************************************************************/
/* HELP menus */
/**************************************************************/
void olc_help( CHAR_DATA *ch, char *command, char *argument, char *arg )
{
   if( !str_prefix( command, "select" ) || atoi(command) == 1 )
   {
      Cprintf(ch, "select");
   }
   else if( !str_prefix( command, "list" ) || atoi(command) == 2 )
   {
      Cprintf(ch, "list");
   }
   else if( !str_prefix( command, "edit" ) || atoi(command) == 3 )
   {
      Cprintf(ch, "edit");
   }
   else if( !str_prefix( command, "create" ) || atoi(command) == 4 )
   {
      Cprintf(ch, "create");
   }
   else if( !str_prefix( command, "show" ) || atoi(command) == 5 )
   {
      Cprintf(ch, "show");
   }
   else if( !str_prefix( command, "main" ) || atoi(command) == 6 )
   {
      ch->pcdata->olc_depth = MAIN;
      show_olc_list( ch, MAIN, FALSE );
   }
   else if( !str_prefix( command, "exit" ) || atoi(command) == 7 )
   {
      olc_exit( ch );
   }
   else
   {
      show_olc_list( ch, HELP, FALSE );
   }
}

/*****************************************************************/
/* olc menu lists
 * this is responsible for displaying menus
 *
 * ONE big ASS switch/case statment
 * Their has to be a better way, but this works */
/*****************************************************************/
void show_olc_list( CHAR_DATA *ch, int menu_level, bool olc_auto )
{
   AREA_DATA *pArea;
   MOB_INDEX_DATA *pMob;
   OBJ_INDEX_DATA *pObj;
   ROOM_INDEX_DATA *pRoom;
   int  i;
   int  j;

   i = 0;
   j = 0;

   EDIT_AREA(ch, pArea);
   EDIT_ROOM(ch, pRoom);
   EDIT_MOB(ch, pMob);
   EDIT_OBJ(ch, pObj);

   switch( menu_level )
   {
      /*****************************************************************/
      /* Main Menu
       * Entry point to all sub menus */
      /*****************************************************************/
      case MAIN:
         Cprintf(ch, "-= Edit Menu =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto )
         {
            Cprintf(ch, "1.  Personal [Settings]\n\r");
            Cprintf(ch, "2.  Edit [Area]\n\r");
            Cprintf(ch, "3.  Edit [Room]\n\r");
            Cprintf(ch, "4.  Edit [Mob]\n\r");
            Cprintf(ch, "5.  Edit [Object]\n\r");
            Cprintf(ch, "6.  Edit [Resets]\n\r");
            Cprintf(ch, "7.  Edit [Helps]\n\r");
            Cprintf(ch, "8.  [Exit] OLC\n\r");
         }
         break;
      /*****************************************************************/
      /* Personal menu
       * Adjust default settings */
      /*****************************************************************/
      case PERSONAL:
         Cprintf(ch, "-= Personal Setting =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "1.  Toggle [Room]   - (Default Cur Room)\n\r");
            Cprintf(ch, "2.  Toggle [Mob]    - (Default First Mob in Room)\n\r");
            Cprintf(ch, "3.  Toggle [Object] - (Default First Obj in Room)\n\r");
            Cprintf(ch, "4.  Toggle [Auto]   - (Create Rooms when walking)\n\r");
            Cprintf(ch, "5.  Toggle [Door]   - (Double/Single door mode)\n\r");
            Cprintf(ch, "6.  [Brief] Menus   - (Toggle between brief and full menu)\n\r");
            Cprintf(ch, "7.  [Help] on the Above Options)\n\r");
            Cprintf(ch, "8.  Goto [Main]\n\r");
            Cprintf(ch, "9.  [Exit] OLC\n\r");
         }
         break;
      /*****************************************************************/
      /* Area Menu
       * Modify and veiw arreas */
      /*****************************************************************/
      case AREA:
         Cprintf(ch, "-= Area Menu =-\n\r");
         Cprintf(ch, "- Selected Area:  %s\n\r", pArea->name);
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Select] New Area\n\r");
            Cprintf(ch, "2.  Area [Info]\n\r");
            Cprintf(ch, "3.  [Rest] Area\n\r");
            Cprintf(ch, "4.  [New] Area\n\r");
            Cprintf(ch, "5.  [Save] Area\n\r");
            Cprintf(ch, "6.  [Rename] area\n\r");
            Cprintf(ch, "7.  Change [Credits] Area\n\r");
            Cprintf(ch, "8.  Set [Security]\n\r");
            Cprintf(ch, "9.  Add/Remove [Builder]\n\r");
            Cprintf(ch, "10. Change [Age]\n\r");
            Cprintf(ch, "11. Set [Lower] Vnum Range\n\r");
            Cprintf(ch, "12. Set [Upper] Vnum Range\n\r");
            Cprintf(ch, "13. [List] Areas\n\r");
            Cprintf(ch, "14. Goto [Main]\n\r");
            Cprintf(ch, "15. [Exit] OLC\n\r");
         }
         break;
      case ROOM:
         Cprintf(ch, "-= Room Menu =-\n\r");
         Cprintf(ch, " - Selected Room:   (Room Var)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Select] Room Vnum\n\r");
            Cprintf(ch, "2.  [List] Rooms in Area\n\r");
            Cprintf(ch, "3.  Room [Info]\n\r");
            Cprintf(ch, "4.  [Create] New Room\n\r");
            Cprintf(ch, "5.  [Copy] Room\n\r");
            Cprintf(ch, "6.  [Modify] Room\n\r");
            Cprintf(ch, "7.  [Delete] Room\n\r");
            Cprintf(ch, "8.  Goto [Main]\n\r");
            Cprintf(ch, "9.  [Exit] OLC\n\r");
         }
         break;
      case ROOM_MOD:
         Cprintf(ch, "-= Room Modify =-\n\r");
         Cprintf(ch, " - Selected Room: (current room)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  Modify [Name]\n\r");
            Cprintf(ch, "2.  Modify [Description]\n\r");
            Cprintf(ch, "3.  Add [Extended] Descrip\n\r");
            Cprintf(ch, "4.  [Remove] Extended Desrip\n\r");
            Cprintf(ch, "5.  Modify [Flags]\n\r");
            Cprintf(ch, "6.  Modify [Door]\n\r");
            Cprintf(ch, "7.  Modify [Sector]\n\r");
            Cprintf(ch, "8.  Modify [Heal] Rate\n\r");
            Cprintf(ch, "9.  Modify [Mana] Rate\n\r");
            Cprintf(ch, "10. Modify [Owner]\n\r");
            Cprintf(ch, "11. [Done] Modifying Room\n\r");
            Cprintf(ch, "12. [Main] Menu\n\r");
            Cprintf(ch, "13. [Exit] OLC\n\r");
         }
         break;
      case ROOM_FLAGS:
         Cprintf(ch, "-= Set Room Flags =-\n\r");
         Cprintf(ch, " - Flags:  (flags)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( room_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set Flag [%s]\n\r", i+1, room_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, room_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Flags\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Flags\n\r", i+1);
            }
         }
         break;
      case DOOR:
         Cprintf(ch, "Which exit [north,east,south,west,up,down]: ");
         break;
      case DOOR_MOD:
         Cprintf(ch, "-= Modify Door Menu =-\n\r");
         Cprintf(ch, " - Selected Exit: (curent exit) - Mode: (mode)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  Toggle Door [Mode] (Double/Single)\n\r");
            Cprintf(ch, "2.  [Select] Door\n\r");
            Cprintf(ch, "3.  Door [Info]\n\r");
            Cprintf(ch, "4.  Modify [Destination]\n\r");
            Cprintf(ch, "5.  Modify [Name]\n\r");
            Cprintf(ch, "6.  Modify [Key]\n\r");
            Cprintf(ch, "7.  Modify [Flags]\n\r");
            Cprintf(ch, "8.  [Copy] to Other Side\n\r");
            Cprintf(ch, "9.  [Remove] Exit\n\r");
            Cprintf(ch, "10. [Done] Modifying Door\n\r");
            Cprintf(ch, "11. [Main] Menu\n\r");
            Cprintf(ch, "12. [Exit] OLC\n\r");
         }
         break;
      case ROOM_SEC:
         Cprintf(ch, "-= Select Sector =-\n\r");
         Cprintf(ch, "Room: (current room) - Sector: (type)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( sector_flags[i].name != NULL )
            {
               Cprintf(ch, "%2d.  Set Sector [%s]\n\r", i+1, sector_flags[i].name);
               i++;
            }
            Cprintf(ch, "%2d.  [Cancel] Setting Bits\n\r", i+1);
         }
         break;
      case MOB:
         Cprintf(ch, "-= Mob Menu =-\n\r");
         Cprintf(ch, " - Selected Mob:   (Mob Var)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Select] Mob Vnum\n\r");
            Cprintf(ch, "2.  [List] Mobs in Area\n\r");
            Cprintf(ch, "3.  [Create] New Mob\n\r");
            Cprintf(ch, "4.  [Copy] Mob\n\r");
            Cprintf(ch, "5.  [Modify] Mob\n\r");
            Cprintf(ch, "6.  Mob [Info]\n\r");
            Cprintf(ch, "7.  Update [Instances] of the Mob\n\r");
            Cprintf(ch, "8.  Modify [Shop] Info\n\r");
            Cprintf(ch, "9.  [Delete] Mob\n\r");
            Cprintf(ch, "10. Goto [Main]\n\r");
            Cprintf(ch, "11. [Exit] OLC\n\r");
         }
         break;
      case MOB_MOD:
         Cprintf(ch, "-= Edit Mob =-\n\r");
         Cprintf(ch, "-= Selected Mob (mob) =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  Modify [Name]                 16.  Modify [Vulnerabilties]\n\r");
            Cprintf(ch, "2.  Modify [Level]                17.  Modify [Parts]\n\r");
            Cprintf(ch, "3.  Modify [Short] Description    18.  Modify [Form]\n\r");
            Cprintf(ch, "4.  Modify [Long] Description     19.  Modify [Race]\n\r");
            Cprintf(ch, "5.  Modify [Description]          20.  Modify [Sex]\n\r");
            Cprintf(ch, "6.  Modify [Hp]                   21.  Modify [Position]\n\r");
            Cprintf(ch, "7.  Modify [Mana]                 22.  Modify [Wealth]\n\r");
            Cprintf(ch, "8.  Modify [Ac]                   23.  Modify [Size]\n\r");
            Cprintf(ch, "9.  Modify [Attack] Type          24.  Modify [Alignment]\n\r");
            Cprintf(ch, "10. Modify [Damage]               25.  Modify [Affects]\n\r");
            Cprintf(ch, "11. Modify [Hitroll]              26.  Modify [Special]\n\r");
            Cprintf(ch, "12. Modify [Act] Flags            27.  [Done] Modifying Mob\n\r");
            Cprintf(ch, "13. Modify [Offensive] Flags      28.  Goto [Main]\n\r");
            Cprintf(ch, "14. Modify [Immunities]           29.  [Exit] OLC\n\r");
            Cprintf(ch, "15. Modify [Resistances]\n\r");
         }
         break;
      case MOB_OFF:
         Cprintf(ch, "-= Offencive Flags Menu =-\n\r");
         Cprintf(ch, " - Selected: <mob>  - Set Flags: <blah>\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto )
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( off_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set Flag [%s]\n\r", i+1, off_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, off_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Flags\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Flags\n\r", i+1);
            }
         }
         break;
      case MOB_ATT:
         Cprintf(ch, "-= Attack Type Menu =-\n\r");
         Cprintf(ch, " - Selected:  (mob) - Attack Type: (type)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( attack_table[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set Attack [%s]\n\r", i+1, attack_table[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Attack [%s]", i+1, attack_table[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Attack Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Attack selection\n\r", i+1);
            }
         }
         break;
      case MOB_ACT:
         Cprintf(ch, "-= Set Act Flags =-\n\r");
         Cprintf(ch, " - Flags:  (flags)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( act_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, act_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, act_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_IMM:
         Cprintf(ch, "-= Set Immune Flags =-\n\r");
         Cprintf(ch, " - Flags:  Poison\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( imm_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, imm_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, imm_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_RES:
         Cprintf(ch, "-= Set Resistant Flags =-\n\r");
         Cprintf(ch, " - Flags:  Fire, Cold\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( res_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, res_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, res_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_VULN:
         Cprintf(ch, "-= Set Vulnerable Flags =-\n\r");
         Cprintf(ch, " - Flags:  Holy\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( vuln_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, vuln_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, vuln_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_PART:
         Cprintf(ch, "-= Set Part Flags =-\n\r");
         Cprintf(ch, " - Flags:  Head, Arms, Legs, Heart, Brains, Guts\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( part_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, part_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, part_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_FORM:
         Cprintf(ch, "-= Set Form Flags =-\n\r");
         Cprintf(ch, " - Flags:  Edible, Sentient, Biped, Mammal\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( form_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, form_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, form_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Bits\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Bits\n\r", i+1);
            }
         }
         break;
      case MOB_RACE:
         Cprintf(ch, "-= Select a Race =-\n\r");
         Cprintf(ch, " - Current Race 'human'\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( race_table[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set Race [%s]\n\r", i+1, race_table[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Race [%s]", i+1, race_table[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Race Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Race Selection\n\r", i+1);
            }
         }
         break;
      case MOB_POS:
         Cprintf(ch, "-= Modify Mob Position =-\n\r");
         Cprintf(ch, " - Current starting position:  standing.\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( position_flags[i].name != NULL )
            {
               Cprintf(ch, "%2d.  Position [%s]\n\r", i+1, position_flags[i].name);
               i++;
            }
            Cprintf(ch, "%2d.  [Cancel] Operation\n\r", i+1);
         }
         break;
      case MOB_SIZE:
         Cprintf(ch, "-= Modify Mob Size Menu =-\n\r");
         Cprintf(ch, " - Current mob size 'Medium'\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( size_flags[i].name != NULL )
            {
               Cprintf(ch, "%2d.  Set mob size [%s]\n\r", i, size_flags[i].name);
               i++;
            }
            Cprintf(ch, "%2d.  [Cancel] Operation\n\r", i);
         }
         break;
      case MOB_SEX:
         Cprintf(ch, "Chose a sex (none, male, female, either): ");
         break;
      case MOB_AFF:
         Cprintf(ch, "-= Set Affect Flags =-\n\r");
         Cprintf(ch, " - Flags:  Detect_invis, Detect_hidden, Sanctuary\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( affect_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Toggle Flag [%s]\n\r", i+1, affect_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Toggle Flag [%s]", i+1, affect_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Flags\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Flags", i+1);
            }
         }
         break;
      case MOB_SPEC:
         Cprintf(ch, "-= Select Special =-\n\r");
         Cprintf(ch, " - Current Special 'spec_cast_mage'\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( spec_table[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, spec_table[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, spec_table[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Setting Selection\n\r%2d. [Remove] Special", i+1, i+2);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Setting Selection  %2d. [Remove[ Special\n\r", i+1, i+2);
            }
         }
         break;
      case MOB_SHOP:
         Cprintf(ch, "-= Mob Shop Menu =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Copy] Shop\n\r");
            Cprintf(ch, "2.  Modify Shop [Buy] Types\n\r");
            Cprintf(ch, "3.  Modify Shop [Profits]\n\r");
            Cprintf(ch, "4.  Modify Shop [Hours]\n\r");
            Cprintf(ch, "5.  Shop [Info]\n\r");
            Cprintf(ch, "6.  [Remove] Shop\n\r");
            Cprintf(ch, "7.  [Done] Modifing Shop\n\r");
            Cprintf(ch, "8.  Goto [Main]\n\r");
            Cprintf(ch, "9.  [Exit] OLC\n\r");
         }
         break;
      case OBJ:
         Cprintf(ch, "-= Object Menu =-\n\r");
         Cprintf(ch, " - Selected Object:   (Obj Var)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Select] Object Vnum\n\r");
            Cprintf(ch, "2.  [List] Objects in Area\n\r");
            Cprintf(ch, "3.  [Create] New Object\n\r");
            Cprintf(ch, "4.  [Copy] Object\n\r");
            Cprintf(ch, "5.  [Modify] Object\n\r");
            Cprintf(ch, "6.  Object [Info]\n\r");
            Cprintf(ch, "7.  Update [Instances] of the Object\n\r");
            Cprintf(ch, "8.  [Delete] Object\n\r");
            Cprintf(ch, "9.  Goto [Main]\n\r");
            Cprintf(ch, "10. [Exit] OLC\n\r");
         }
         break;
      case OBJ_MOD:
         Cprintf(ch, "-= Object Modify Menu =-\n\r");
         Cprintf(ch, "-= Selected Obj: \n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.   Modify [Name]                 11.  Modify [Condition]\n\r");
            Cprintf(ch, "2.   Modify [Level]                12.  Modify [Weight]\n\r");
            Cprintf(ch, "3.   Modify [Short] Description    13.  Modify [Cost]\n\r");
            Cprintf(ch, "4.   Modify [Long] Description     14.  Modify [Values]\n\r");
            Cprintf(ch, "5.   Add [Extended] Descrip        15.  [Add] Affect\n\r");
            Cprintf(ch, "6.   [Remove] Extended Desrip      16.  [Delete] Affect\n\r");
            Cprintf(ch, "7.   Modify [Material]             17.  [Done] Modifying Object\n\r");
            Cprintf(ch, "8.   Modify Item [Type]            18.  Goto [Main]\n\r");
            Cprintf(ch, "9.   Modify Extra [Flags]          19.  [Exit] OLC\n\r");
            Cprintf(ch, "10.  Modify [Wear] Flags\n\r");
         }
         break;
      case OBJ_TYPE:
         Cprintf(ch, "-= Object Type Menu =-\n\r");
         Cprintf(ch, "-= Selected Obj: (obj)  Type: (type)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( type_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, type_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, type_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
         }
         break;
      case OBJ_FLAGS:
         Cprintf(ch, "-= Set Extra Flags =-\n\r");
         Cprintf(ch, " - Flags:  Nopurge\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( extra_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, extra_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, extra_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Selection\n\r", i+1);
            }
         }
         break;
      case OBJ_WEAR:
         Cprintf(ch, "-= Set Wear Flags =-\n\r");
         Cprintf(ch, " - Flags:  None\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( wear_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, wear_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, wear_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
         }
         break;
      case OBJ_MATIRIAL:
         Cprintf(ch, "-= Object Matrial Selection =-\n\r");
         Cprintf(ch, "Current Object: <obj> Type: <type>\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto )
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( material_type[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, material_type[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, material_type[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Setting Selection\n\r", i+1);
            }
         }
         break;
      case OBJ_CON:
         Cprintf(ch, "-= Object Condition Menu =-\n\r");
         Cprintf(ch, "Selected Object: <obj>  COndition: <con>\n\r");
         break;
      case OBJ_VALUE:
         Cprintf(ch, "Value 0 (Not used) [0]:\n\r");
         Cprintf(ch, "Value 1 (Not used) [0]:\n\r");
         Cprintf(ch, "Value 2 (Not used) [0]:\n\r");
         Cprintf(ch, "Value 3 (Not used) [0]:\n\r");
         Cprintf(ch, "Value 4 (Not used) [0]:\n\r");
         break;
      case OBJ_SPELL:
         Cprintf(ch, "-= Object Spell Affects =- \n\r");
         Cprintf(ch, "Current Object: <obj> Affects <aff>\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            j = 0;
            while( skill_table[i].name != NULL )
            {
               if( /*skill_table[i].target == TAR_CHAR_DEFENCIVE*/
                 skill_table[i].target == TAR_OBJ_CHAR_DEF
                || skill_table[i].target == TAR_IGNORE
                || skill_table[i].target == TAR_CHAR_SELF )
               {
                  if( j % 2 )
                  {
                     Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", j+1, skill_table[i].name);
                  }
                  else
                  {
                     Cprintf(ch, "%2d.  Set Flag [%s]", j+1, skill_table[i].name);
                  }
                  j++;
               }
               i++;
            }
            if( j % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Cancel] Setting Selection\n\r", j+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Cancel] Setting Selection\n\r", j+1);
            }
         }
         break;
      case OBJ_AFF:
         Cprintf(ch, "-= Add Affect Menu =-\n\r");
         Cprintf(ch, "-= Selected Obj: (obj) Affects: (affs)\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            i = 0;
            while( apply_flags[i].name != NULL )
            {
               if( i % 2 )
               {
                  Cprintf(ch, "\t\t%2d.  Set [%s]\n\r", i+1, apply_flags[i].name);
               }
               else
               {
                  Cprintf(ch, "%2d.  Set Flag [%s]", i+1, apply_flags[i].name);
               }
               i++;
            }
            if( i % 2 )
            {
               Cprintf(ch, "\t\t%2d.  [Done] Setting Selection\n\r", i+1);
            }
            else
            {
               Cprintf(ch, "%2d.  [Done] Setting Selection\n\r", i+1);
            }
         }
         break;
      case RESET:
         Cprintf(ch, "-= Reset Menu =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto)
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  Reload [Mob] Resets\n\r");
            Cprintf(ch, "2.  Reload [Object] Resets\n\r");
            Cprintf(ch, "3.  Reload [Door] Resets\n\r");
            Cprintf(ch, "4.  Reload [All] Resets\n\r");
            Cprintf(ch, "5.  Add Current [Room] Resets\n\r");
            Cprintf(ch, "6.  Goto [Main]\n\r");
            Cprintf(ch, "7.  [Exit] OLC\n\r");
         }
         break;
      case HELP:
         Cprintf(ch, "-= Help Menu =-\n\r");
         if( ch->pcdata->brief == FALSE || olc_auto )
         {
            Cprintf(ch, "\n\r");
            Cprintf(ch, "1.  [Select] Help\n\r");
            Cprintf(ch, "2.  [List] Helps\n\r");
            Cprintf(ch, "3.  [Create] New Help\n\r");
            Cprintf(ch, "4.  [Edit] Help\n\r");
            Cprintf(ch, "5.  [Show] Help\n\r");
            Cprintf(ch, "6.  Goto [Main]\n\r");
            Cprintf(ch, "7.  [Exit] OLC\n\r");
         }
         break;
      default:
         Cprintf(ch, "*** ERROR ***, exiting OLC.\n\r");
         olc_exit( ch );
         break;
   }
}


