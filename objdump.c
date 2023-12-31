/* revision 1.1 - August 1 1999 - making it compilable under g++ */
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefiting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *        ROM 2.4 is copyright 1993-1995 Russ Taylor                       *
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
#include "utils.h"


/*
   ===========================================================================
   This snippet was written by Erwin S. Andreasen, 4u2@aabc.dk. You may use
   this code freely, as long as you retain my name in all of the files. You
   also have to mail me telling that you are using it. I am giving this,
   hopefully useful, piece of source code to you for free, and all I require
   from you is some feedback.

   Please mail me if you find any bugs or have any new ideas or just comments.

   All my snippets are publically available at:

   http://login.dknet.dk/~ea/

   If you do not have WWW access, try ftp'ing to login.dknet.dk and examine
   the /pub/ea directory.
   ===========================================================================

   Note that function prototypes are not included in this code - remember to add
   them to merc.h yourself.

   Last update: October 27, 1995

   Should work on : ROM2.4+

   Comments:

   The object lister writes every object *reset* on the mud to a file. The
   list is created in comma-delimited format, perfect for importing into
   a spreadsheet or database and further analyzing, allowing to quickly answer
   questions like: Which weapon does the most damage? What spells are available
   in wands? Which levels have the least items? What resets where and on who?
   Which equipment is missing or has too many wear-bits? Etc., etc.

   NOTE: This is the ROM 2.4+ version. There is also a MERC version avaiable.

   CHANGES FOR ROM
   ===============

   Added weapon bits.
   Declared do_help.
   Removed wear_name, using wear_bit_name.
   Removed check for valid liquid number.
   Added 4 more AC values and bulk. (the latter does not look good).
   Extra pill/scroll/potion slot.
   Support for the bits-affecting objs. I have no way to check if this is OK..
   Weapon damage shown.
   Material shown.
   Level is taken from object_index if new format.
   Added portal. Flags are not parsed yet, just printed as numbers.


   TODO:
   Pills/potion/scrolls reset level is different in ROM!
   Object reset count/limit ?
   Add explanation of new functions.


   LIMITATIONS
   ===========

   P supports only one level of recursion, i.e.
   obj inside something on a mob
   NOT  obj inside obj inside obj on a mob
   if obj is inside obj inside obj, room vnum cannot be found

   INSTRUCTIONS
   ============

   to customize this lister to your ROM2.4+ MUD:

   1) Add other relevant APPLY_XXX to number_app.

   2) Add relevant other item_types to item_type
   Also add them in the big switch () in printobj (around line 600), which
   prints object-specific data.
   Perhaps change your item_type_name to instead call THIS item_type
   so new item types have only to be added in this place only.

   3) Add other object specific information that does not fit anywhere in the
   get_other_information() function

   4) Add other flags to the extra_flags_name table

   5) Add a help entry for 'objdump' (The functions shows that entry if wrong
   or none parameters are entered.

   FIELD KEY
   =========

   1    Short Desc              Contains short_desc of the object

   2    Keywords                Keywords, separated by a space

   3    Level                   The level the object is reset at (can vary +/-2).
   Objects with new format have a set level.

   4    Vnum                    The vnum of the object

   5    Room                    The vnum of the room the object resets in

   6    Location                Text desc of the location (on mob X, in room: Y, inside Z)

   7    Flags                   GENVMDRghdlvbPVRLSBU#*
   anti-Good, anti-Evil, anti-Neutral, inVisible, Magic, noDrop,
   noRemove, Glowing, Humming, Dark, Lock, eVil, Bless, ,
   noPurge, Visdeath, Rotdeath, noLocate, Sellexract,
   Burnproof, noUncurse.
   If set, the letter is there, if not, a '-' appears.
   Important flags are in uppercase.
   # flag is set, if the object resets on a shopkeeper and the
   shopkeeper sells that particural object.
   Note that those objects' levels are differently derived.
   * denotes a reboot-only object. An object is reboot if
   either it or the mob it resets on has a reset-count of
   -1 (note that this is not a standard MERC feature).


   8    Weight                  Weight of object, in tenths of pounds.

   9    Value                   Value/cost of object

   10   Material                Material. 'oldstyle' if old style and has no material.

   11   Worn                    Where the object can be worn. Be on lookout for objects
   without Take flag, if they are armor, weapons etc. Also
   watch out for Weapons with other than (Take Wield)

   12   Item Type               Armor, Wand, Staff, Weapon etc.

   13   AC/SLevel               For Armor: contains the 4 AC numbers.
   For wand,staff,pill,potion,scroll: contains spell level
   For containers: Contains key number.
   For drink containers and food: non-zero means it is
   poisoned
   For weapons: ?d? damage if new format, approx damage range
   if old.
   For Food: number of hour it will keep character full.


   14   Capacity                For wand/staves contains number of charges
   For containers (drink, too) contains capacity in the form
   X lbs, Y items (Z% weight).
   For lights: contains number of hours it can burn (more
   explanation is given in the Spells field).
   For food: number of hours it will keep char from getting
   hungry.
   For armor: contains bulk number.

   15   Spells                  For spell-using objects, contains spell(s) cast by obj.
   For light: contains explanation on Capacity (dead,
   unlimited, limited).
   For containers: contains flags (lockable, closeable etc.)
   For drink containers: contains liquid type.

   16  Affects                  Contains all affects of the obj, in one long field. Useful
   if you have to produce a text-only version of the list.

   17+  Affects                 Contains modifier to that affect

   ??   Bits                    Contains various bits that the object bestows, for example
   detect invis, immunity to something etc.

   ??   Other                   Contains other information, gathered from the
   get_other_information function.

   Use objdump command, then load the file in your favorite database or spread-
   sheet. I use Excel. Sorting by: Item Type, Worn, Level gives me the best
   results. The start analyzing. Find objects with silly names, misspelled names,
   missing Take worn_flags, too many worn_flags, too high affects (just sort after
   damage roll or hit roll field!). Or perhaps add some formula to evaluate the
   reality of an object (i know Farside had one in its builder docs?). Try also
   searching for a '!' - it indicates objects resetting on non-existing mobs and
   other errors (but your bug file should hopefullly already have told you that).

   If you need to distribute a pure-text form version of the database, there is a
   field which includes all the affects. You will still need some spreadsheet or
   database to export it though. I do not plan on making the function do pure
   text (i.e. not comma delimetered) output - it is easier for you to use a spread-
   sheet to organize the database to fit exactly your needs.

   Note that the command is best run just after reboot, with no players on, so
   the contains for P resets can be found properly. The P resets look for the
   first and best object of the appropriate type, so if a player has it, the
   file will say that the object resets on a player :()

   FUNCTIONS AND STRUCTURES CONTAINED WITHIN THIS FILE
   ===================================================

   const int number_app [];

   contains a list of APPLY_XXX constants that are to be printed, with the number
   they apply with (e.g. damage roll). Not all have to be here, some like
   APPLY_AGE, APPLY_GOLD or the like are irrelevant.

   char * get_other_information (OBJ_INDEX_DATA*)

   Returns whatever else is relevant for this object. Special functions, powers,
   etc. By default this function returns nothing, it is up to you to add something
   you feel is relevant.

   struct obj_reset;

   A list containing pairs of a obj vnum and a level. Used by printobj
   to avoid printing the same object at the same level several times to the file.
   If the same obj resets at several _different_ levels, they are printed.

   dispose_list()
   new_list()
   add_to_list()
   is_in_list()

   Functions to manipulate such a list. Not much to say about those...

   const char * wear_table [];

   Contains names of all possible places to wear eq. You should update this
   with your own locations.

   const char * wear_name (int);

   When given a wear_flag, returns a string containining the places the obj
   with such a wear flag can be worn. Standard MERC did not have such a function,
   so I had to make one myself.

   const char * item_type (int);

   This version of item_type_name does the same, but takes just a int as a
   parameter - the handler.c:item_type_name takes an OBJ_DATA*, and what I
   needed was to find the type name of a OBJ_INDEX_DATA*

   int get_affect_total (AFFECT_DATA*, int);

   Returns the total modifier of a certain affect type in this linked list
   of AFFECT_DATAs.

   bool is_affect (AFFECT_DATA*, int);

   Returns TRUE if a given affect is in the list.

   char *extra_flag_string (int)

   Returns a string like this: "GEVN-----g---' for a given object's extra_flag
   string. looks up in the extra_flag_names table.

   The extra_flag_names table consists of constants for the flags and a letter,
   for each of the flags you want displayed.

   char * affect_str (AFFECT_DATA*, char buffer*);

   Returns a string in the form "+40hp,+2dex,-1wis" containing all the number_app
   and bool_app affects in the given affect-list. Useful for quick overview.
   As the list can get pretty long, the caller must provide a buffer of reasonable
   length. (MAX_STRING_LENGTH should do?)

   printheader (FILE*)

   Prints the header rows

   printobj (FILE*, OBJ_RESET*, where, int, int, int, int, int);

   Prints out the specified object into FILE*. Arguments depend on whether the
   obj is reset on ground, inside a container, or on a mob. Check function
   source code for more info.

   printarea (FILE*, AREA_DATA*);

   Prints the objs reset in that area to FILE*, which has to have been
   opened in mode "w" before then.

   do_objdump (CHAR_DATA*, char *);

   Core function, calls printarea for either all or just one area, depending
   on users' wishes. Syntax is:

   objdump <ALL|AREA> <filename>

   AREA just prints the area the character is in. Undefined result if
   ch->in_room is NULL (but when is it?)

   ALL runs through the list of areas and dumps every one to the file.


 */

/* Now, without further interruptions... the code itself */

extern AREA_DATA *area_first;        /* snarf the local fro db.c */
typedef enum
{
   on_ground, inside, on_mob
}
where;
typedef struct obj_reset OBJ_RESET;

DECLARE_DO_FUN (do_help);

#define MAX_NUMBER_APP   (sizeof(number_app) / sizeof (number_app[0]))
#define MAX_EXTRA_FLAG (sizeof(flag_names) / sizeof (flag_names[0]))

struct obj_reset
{
   int vnum;
   int level;
   OBJ_RESET *next;
};

struct extra_flag_names
{
   char name;
   int value;
};

/* applys where value matters. They are displayed in exactly this order: */
const int number_app[] =
{
   APPLY_HITROLL,
   APPLY_DAMROLL,
   APPLY_HIT,
   APPLY_MANA,
   APPLY_MOVE,
   APPLY_AC,
   APPLY_STR,
   APPLY_DEX,
   APPLY_INT,
   APPLY_WIS,
   APPLY_CON,
};

/* extra flags and corresponding characters in the flag list */
const struct extra_flag_names flag_names[] =
{
        /* important flags, in uppercase */
   {'G', ITEM_ANTI_GOOD},
   {'E', ITEM_ANTI_EVIL},
   {'N', ITEM_ANTI_NEUTRAL},
   {'V', ITEM_INVIS},
   {'M', ITEM_MAGIC},
   {'D', ITEM_NODROP},
   {'R', ITEM_NOREMOVE},
        /* less important flags, in lowercase */
   {'g', ITEM_GLOW},
   {'h', ITEM_HUM},
   {'d', ITEM_DARK},                /* Diku? */
   {'l', ITEM_LOCK},
   {'v', ITEM_EVIL},
   {'b', ITEM_BLESS},
   {'P', ITEM_NOPURGE},
   {'V', ITEM_VIS_DEATH},
   {'R', ITEM_ROT_DEATH},
   {'L', ITEM_NOLOCATE},
   {'S', ITEM_SELL_EXTRACT},
   {'B', ITEM_BURN_PROOF},
   {'U', ITEM_NOUNCURSE}

};

/* return other information relevant to your MUD */
char *
get_other_information (OBJ_INDEX_DATA * obj)
{
   return "";
}


/* manipulation of a small list to hold vnums & levels they were reset at
   to avoid redundancy */
void
dispose_list (OBJ_RESET * list)
{
   OBJ_RESET *next;

   for (; list != NULL; list = next)
   {
      next = list->next;
      free (list);
   }
}

OBJ_RESET *
new_list (void)                        /* create a new list */
{
   OBJ_RESET *list;
   /* create an empty list head */
   list = (OBJ_RESET *) malloc (sizeof (OBJ_RESET));
   list->next = NULL;
   return list;
}

void
add_to_list (OBJ_RESET * list, int vnum, int level)
{
   OBJ_RESET *new_item;
   new_item = (OBJ_RESET *) malloc (sizeof (OBJ_RESET));
   new_item->next = list->next;
   list->next = new_item;
   new_item->vnum = vnum;
   new_item->level = level;
}

bool
is_in_list (OBJ_RESET * list, int vnum, int level)
{
   for (list = list->next; list != NULL; list = list->next)
      if ((list->vnum == vnum) && (list->level == level))
         return TRUE;
   return FALSE;
}


/* standard item_type_names accepts only OBJ_DATA* */
char *
item_index_type_name (int item_type)
{
   switch (item_type)
   {
   case ITEM_LIGHT:
      return "light";
   case ITEM_SCROLL:
      return "scroll";
   case ITEM_WAND:
      return "wand";
   case ITEM_STAFF:
      return "staff";
   case ITEM_WEAPON:
      return "weapon";
   case ITEM_TREASURE:
      return "treasure";
   case ITEM_ARMOR:
      return "armor";
   case ITEM_POTION:
      return "potion";
   case ITEM_FURNITURE:
      return "furniture";
   case ITEM_TRASH:
      return "trash";
   case ITEM_RECALL:
      return "recall_crystal";
   case ITEM_ORB:
      return "orb";
   case ITEM_CONTAINER:
      return "container";
   case ITEM_DRINK_CON:
      return "drink container";
   case ITEM_KEY:
      return "key";
   case ITEM_FOOD:
      return "food";
   case ITEM_MONEY:
      return "money";
   case ITEM_BOAT:
      return "boat";
   case ITEM_CORPSE_NPC:
      return "npc corpse";
   case ITEM_CORPSE_PC:
      return "pc corpse";
   case ITEM_FOUNTAIN:
      return "fountain";
   case ITEM_PILL:
      return "pill";
   case ITEM_PORTAL:
      return "portal";
   case ITEM_CLOTHING:
      return "clothing";
   case ITEM_MAP:
      return "map";
   case ITEM_WARP_STONE:
      return "warpstone";
   case ITEM_ROOM_KEY:
      return "room key";
   case ITEM_GEM:
      return "gem";
   case ITEM_JEWELRY:
      return "jewelry";
   case ITEM_JUKEBOX:
      return "jukebox";

   }

   bug ("Item_type_name: unknown type %d.", item_type);
   return "(unknown)";
}

/* how much affect_num is there in pAffectr */
int
get_affect_total (AFFECT_DATA * pAffect, int affect_num)
{
   int total = 0;

   for (; pAffect != NULL; pAffect = pAffect->next)
      if (pAffect->location == affect_num)
         total = total + pAffect->modifier;

   return total;
}

/* is affect_num contained in pAffect somewhere ? */
bool
is_affect (AFFECT_DATA * pAffect, int affect_num)
{
   for (; pAffect != NULL; pAffect = pAffect->next)
      if (pAffect->location == affect_num)
         return TRUE;

   return FALSE;
}

/* shows all the extra flags like imm(dam_type), res(dam_type), DetInvis, DetMagic etc. */
char *
get_bit_flags (AFFECT_DATA * pAffect)
{
   static char buffer[512];
   static char buf2[512];

   strcpy (buffer, "");

   for (; pAffect != NULL; pAffect = pAffect->next)
   {
      if (pAffect->bitvector)
      {
         switch (pAffect->where)
         {
         case TO_AFFECTS:
            sprintf (buf2, "%s", affect_bit_name (pAffect->bitvector));
            break;
         case TO_OBJECT:
            sprintf (buf2, "obj(%s)",
                     extra_bit_name (pAffect->bitvector));
            break;
         case TO_IMMUNE:
            sprintf (buf2, "imm(%s)",
                     imm_bit_name (pAffect->bitvector));
            break;
         case TO_RESIST:
            sprintf (buf2, "res(%s)",
                     imm_bit_name (pAffect->bitvector));
            break;
         case TO_VULN:
            sprintf (buf2, "vul(%s)",
                     imm_bit_name (pAffect->bitvector));
            break;
         default:
            sprintf (buf2, "unknown(%d,%ld)",
                     pAffect->where, pAffect->bitvector);
            break;

         }                        /* switch */

         strcat (buffer, buf2);
         strcat (buffer, ",");
      }                                /* if */
   }                                /* for */

   if (strlen (buffer))
      buffer[strlen (buffer) - 1] = '\0';        /* cut off the trailing comma */


   return buffer;
}

char *
get_damage_string (OBJ_INDEX_DATA * obj, int level)
{
   static char buffer[10];

   if (obj->new_format)                /* new format: damage given in file */
      sprintf (buffer, "%dd%d", obj->value[1], obj->value[2]);
   else
      sprintf (buffer, "%d-%d", 1 * level / 4 + 2, 3 * level / 4 + 6);

   return buffer;
}


/* returns a string of extra flags */
/* strings looks like this: -----V--b-E, i.e.. set flags have their letter,
   flags not set, have just a '-'
 */

char *
extra_flag_string (int extra)
{
   unsigned int i;
   static char buffer[MAX_EXTRA_FLAG + 1];        /* 1 for each flag + term NUL */

   for (i = 0; i < MAX_EXTRA_FLAG; i++)
      if (IS_SET (extra, flag_names[i].value))
         buffer[i] = flag_names[i].name;        /* it's my buffer and I'll do whatever I want - nyah! */
      else
         buffer[i] = '-';        /* flag not there */

   /* insert the terminating NUL ! */

   buffer[MAX_EXTRA_FLAG] = '\0';
   return buffer;
}

/* returns a string with all relevant (i.e. contained in number_app/bool_app
   affects in pAffect. Only three first letters of location-name are used.
   Example: +4dam,+1dex,-1str
 */
void
affect_str (AFFECT_DATA * pAffect, char *buffer)
{
   unsigned int i, total;
   char buf1[20];                /* just a small buffer, should not hold more than +9999999999nam, */

   strcpy (buffer, "");                /* clear the buffer */

   for (i = 0; i < MAX_NUMBER_APP; i++)
   {
      total = get_affect_total (pAffect, number_app[i]);
      if (total)                /* total != 0 */
      {
         /* i hope this works on non-linux? */
         /* 'arm' abbrev for armor class is not good BTW */
         sprintf (buf1, "%+d%-2.3s,", total, affect_loc_name (number_app[i]));        /* there is no itoa on unix ? */
         strcat (buffer, buf1);
      }
   }

   if (strlen (buffer) > 0)        /* if we actually have printed something */
      buffer[strlen (buffer) - 1] = '\0';
}                                /* affect_str */


/* print a comma-delimetered line into a file */
void
printobj (FILE * fp, OBJ_RESET * list, where location, int vnum_what,
          int vnum_where, int vnum_who, int vnum_inside,
          bool reboot)
{
   char buf[MAX_STRING_LENGTH];
   char buf1[MAX_STRING_LENGTH];
   MOB_INDEX_DATA *mob;
   unsigned int i;
   int level;
   bool shopkeeper = FALSE;

   OBJ_INDEX_DATA *obj = get_obj_index (vnum_what);

   if (!obj)                        /* could not find that vnum! */
   {
      printf ("printobj: could not find obj %d\n", vnum_what);
      return;
   }                                /* if */

   switch (location)                /* write something about WHERE the objects is reset */
   {

   default:
      bug ("printobj: invalid location:%d", location);
      return;

   case on_ground:                /* where is used */
      {
         ROOM_INDEX_DATA *room = get_room_index (vnum_where);

         if (room)                /* room != NULL */
            sprintf (buf1, "\"in room: %s\"", room->name);
         else
            sprintf (buf1, "\"nowhere(!)\"");
         break;
      }

   case inside:                /* inside is used. where is derived */
      {
         OBJ_INDEX_DATA *obj_inside_index;
         OBJ_DATA *obj_inside;


         obj_inside_index = get_obj_index (vnum_inside);

         if (!obj_inside_index)        /* could not even find the obj index?! */
         {
            bug ("printobj: container-obj %d non-existant", vnum_inside);
            return;
         }

         /* find an instance of the object. Note we can risk finding one on a player.. :( */

         obj_inside = get_obj_type (obj_inside_index);

         if (!obj_inside)        /* couldnt find where one of those is */
            vnum_where = 0;        /* could not locate */
         else if (obj_inside->in_room)        /* check for valid room */
            vnum_where = obj_inside->in_room->vnum;
         else
            /* not in a room.. check inventory of a mob ? */ if (obj_inside->carried_by && obj_inside->carried_by->in_room)
            vnum_where = obj_inside->carried_by->in_room->vnum;

         if (vnum_where)
         {
            ROOM_INDEX_DATA *room = get_room_index (vnum_where);
            if (room)
               if (obj_inside->carried_by)        /* the container is carried by a mob */
                  sprintf (buf1, "\"inside %s on %s\"", obj_inside->short_descr,
                           IS_NPC (obj_inside->carried_by)
                           ? obj_inside->carried_by->short_descr
                           : "(a player)");
               else
                  sprintf (buf1, "\"inside %s in %s\"", obj_inside->short_descr, room->name);
            else
               sprintf (buf1, "\"inside %s somewhere(!)\"", obj_inside->short_descr);
         }                        /* if vnum_where */
         else
            sprintf (buf1, "\"inside something(!), somewhere(!)\"");

         break;
      }                                /* case inside */

   case on_mob:                /* where, who is used */
      {
         mob = get_mob_index (vnum_who);
         if (mob)
            sprintf (buf1, "\"on %s\"", mob->short_descr);
         else
            sprintf (buf1, "\"on someone(!)\"");

         break;
      }                                /* on mob */

   }                                /* switch where */

   mob = get_mob_index (vnum_who);        /* vnum_who */

   if (mob != NULL)
   {
      level = URANGE (0, mob->level - 2, LEVEL_HERO);
      /* level of obj ranges from (mob - 4) to (mob - 0), clipped to 0-HERO */
      /* however, this number is number_fuzzied, resulting in -1 (!) to HERO+1 */
   }
   else
      level = 0;                /* when there is no last mob, level is set to 0 */

   if (obj->new_format)
      level = obj->level;

/* check if item is the part of a shop's inventory, and if so adjust level */
/* snarfed from reset_area, changed to only average level */
/* note that we can have a valid mob pointer even if the item is not to be
   given to a shopkeeper, therefore check for location
   Problem: Are items equipped on shopkeeper set to shop-level or normal level?
   Seems to be the former..
 */

   if ((location == on_mob) &&
       (mob != NULL) &&
       (mob->pShop != NULL))        /* loaded on mob, valid mob, valid shopkeeper */
   {

      shopkeeper = TRUE;

      switch (obj->item_type)
      {
      default:
         level = 0;
         break;
      case ITEM_PILL:
         level = 5;
         break;                        /* 0 - 10 */
      case ITEM_POTION:
         level = 5;
         break;                        /* number_range(  0, 10 ); */
      case ITEM_SCROLL:
         level = 10;
         break;                        /* number_range(  5, 15 ); */
      case ITEM_WAND:
         level = 15;
         break;                        /* number_range( 10, 20 ); */
      case ITEM_STAFF:
         level = 20;
         break;                        /* number_range( 15, 25 ); */
      case ITEM_ARMOR:
         level = 10;
         break;                        /* number_range(  5, 15 ); */
      case ITEM_WEAPON:
         level = 10;
         break;                        /* number_range(  5, 15 ); */
      }                                /* switch */
   }
/* check if we already have written an object like this of the same level */
   if (is_in_list (list, vnum_what, level))
      return;                        /* only write obj of same level once */

   add_to_list (list, vnum_what, level);

/* short descr, keywords, level, vnum_what, vnum_where */

   sprintf (buf, "\"%s\",\"%s\",%d,%d,%d,",
            obj->short_descr, obj->name, level, vnum_what, vnum_where);
   strcat (buf, buf1);
   strcat (buf, ",");                /* append textual description of place where it is reset */

/* append extra-flags. if item on a shopkeeper, append the extra # flag,
   if item reboot, append * (asterix) */
   sprintf (buf1, "\"%s%c%c\",", extra_flag_string (obj->extra_flags),
            shopkeeper ? '#' : '-',
            reboot ? '*' : '-');
   strcat (buf, buf1);

/* weight, cost, material, wear flags */
   sprintf (buf1, "%d,%d,\"%s\",\"(%s)\",",
            obj->weight, obj->cost,
            obj->material ? obj->material : "none",        /* material bit */
            wear_bit_name (obj->wear_flags));        /* weight&value of object */
   strcat (buf, buf1);

/* data specific for objects */

/* name, AC, charges, level, spell1, spell2, spell3 */
   switch (obj->item_type)
   {
   case ITEM_WEAPON:                /* weapon (damage_type), damagegiven, weapon extra flags */
      sprintf (buf1, "\"weapon(%s)\",\"%s\",0,\"%s\",",
               attack_table[obj->value[3]].name,
               get_damage_string (obj, level),
               weapon_bit_name (obj->value[4]));        /* charges, spell level, weapon flags */
      break;                        /* weapon */

   case ITEM_ARMOR:                /* armor, ACs and bulk */
      sprintf (buf1, "\"armor\",\"%d/%d/%d/%d\",%d,\"\",",
               obj->value[0], obj->value[1], obj->value[2], obj->value[3],
               obj->value[5]
       );                        /* AC x 4, bulk */
      break;                        /* armor */

   case ITEM_WAND:                /* wand|staff, spell-level, charges, spellname */
   case ITEM_STAFF:
      sprintf (buf1, "\"%s\",%d,%d,\"%s\",",
               item_index_type_name (obj->item_type),
               obj->value[0],
               obj->value[1],
               ((obj->value[3] > 0) && (obj->value[3] < MAX_SKILL)) ?
               skill_table[obj->value[3]].name
               : "unknown spell"
       );
      break;                        /* wand, staff */

   case ITEM_PILL:
   case ITEM_SCROLL:
   case ITEM_POTION:                /* pill|scroll|potion, spell-level, 0, spell-name(s) */
      sprintf (buf1, "\"%s\",%d,0,\"",
               item_index_type_name (obj->item_type),
               obj->value[0]
       );

      /* pill/scroll/potion, spell level, no charges, */
      for (i = 1; i < 5; i++)        /* extra slot for ROM */
      {
         /* valid skills range from 1 .. MAX_SKILL-1 */
         if ((obj->value[i] > 0) && (obj->value[i] < MAX_SKILL))        /* spell name, if any */
         {
            strcat (buf1, skill_table[obj->value[i]].name);
            strcat (buf1, ",");        /* add a comma after, should be OK inside a string ?? */
         }
      }
      if (buf1[strlen (buf1) - 1] == ',')
         buf1[strlen (buf1) - 1] = '\0';        /* remove trailing comma */
      strcat (buf1, "\",");        /* terminating quote, only one! */

      break;                        /* pill, scroll, potion */

   case ITEM_LIGHT:                /* number of hours the light lasts */
      if (obj->value[2] == 0)
         sprintf (buf1, "\"light\",0,0,\"(dead)\",");
      else if (obj->value[2] == -1)
         sprintf (buf1, "\"light\",0,-1,\"(unlimited)\",");
      else
         sprintf (buf1, "\"light\",0,%d,\"(limited)\",", obj->value[2]);

      break;                        /* forgot */

   case ITEM_CONTAINER:
      {
         sprintf (buf1, "\"container\",%d,\"%dlbs, %d max (%d%% bulk),\"",
                  obj->value[2], obj->value[0],
                  obj->value[3], obj->value[4]);
         if (IS_SET (obj->value[1], CONT_CLOSEABLE))
            strcat (buf1, "close ");
         if (IS_SET (obj->value[1], CONT_PICKPROOF))
            strcat (buf1, "nopick ");
         if (IS_SET (obj->value[1], CONT_CLOSED))
            strcat (buf1, "closed ");
         if (IS_SET (obj->value[1], CONT_LOCKED))
            strcat (buf1, "locked");

         strcat (buf1, "\",");
         break;
      }

   case ITEM_DRINK_CON:        /* drink container */
      {
         /* print out poison factor, number of drinks and liquid type */
         /* only print liq type if it is a valid type */
         /* Valid liquids range from 0 .. LIQ_MAX-1 */
         sprintf (buf1, "\"drink container\",%d,%d,\"%s\",",
                  obj->value[3], obj->value[0],
                  (obj->value[2] >= 0)
                  ? liq_table[obj->value[2]].liq_name
                  : "uknown liquid"
          );
         break;
      }

   case ITEM_TREASURE:                /* treasure, nothing extra */
   case ITEM_FURNITURE:        /* do */
   case ITEM_RECALL:
   case ITEM_ORB:
   case ITEM_TRASH:                /* do */
      sprintf (buf1, "\"%s\",0,0,\"\",",
               item_index_type_name (obj->item_type)
       );
      break;

   case ITEM_FOOD:
      sprintf (buf1, "\"food\",%d,%d,\"%s\",",
               obj->value[0], obj->value[1],
               obj->value[3] ? "poisoned" : ""
       );
      break;
   case ITEM_PORTAL:
      {
         ROOM_INDEX_DATA *to_room;

         to_room = get_room_index (obj->value[3]);

         sprintf (buf1, "\"portal to %s\",%d,%d,\"%d\",",
                  to_room ? to_room->name : "nowhere?",
                  obj->value[1],
                  obj->value[0],
                  obj->value[2]);
         break;
      }


   default:
      return;                        /* do not save those (food,key,money,boat,corpse,fountain */
   }

   strcat (buf, buf1);                /* add obj-specific data */

/* now: affects */
/* first: a textual list of all affects */

   strcat (buf, "\"");
   affect_str (obj->affected, buf1);
   strcat (buf, buf1);
   strcat (buf, "\",");

/* then one field for each numeral affect */

   for (i = 0; i < MAX_NUMBER_APP; i++)
   {
      sprintf (buf1, "%d,", get_affect_total (obj->affected, number_app[i]));
      strcat (buf, buf1);
   }
   sprintf (buf1, "\"%s\",", get_bit_flags (obj->affected));        /* NOT TESTED */
   strcat (buf, buf1);

/* add other information. This could be... anything ? */
   sprintf (buf1, ",\"%s\"", get_other_information (obj));
   strcat (buf, buf1);

   fprintf (fp, "%s\n", buf);        /* lets gooo! */
}                                /* printobj */


/* print a header */
void
printheader (FILE * fp)
{
   unsigned int i;

   fprintf (fp,
            "\"Short desc\",\"Keywords\",\"Level\",\"Vnum\",\"Room\",\"Location\",\
\"Flags\",\"Weight\",\"Value\",\"Material\",\"Worn\",\"Item type\",\"AC/SLevel\",\"Capacity\",\
\"Spells\",\"Affects\",");        /* ouch! */

   for (i = 0; i < MAX_NUMBER_APP; i++)
   {
      fprintf (fp, "\"%s\",", affect_loc_name (number_app[i]));
   }                                /* for i */

   /* bit affects */
   fprintf (fp, "\"Bit affects\",");        /* was bool_app before */
   fprintf (fp, "\",\"Other\"\n");        /* finally, "other" field (?) */

}                                /* printheader */


/* print one area to the file fp */
/* file must be open for w and pArea != NULL */
void
printarea (FILE * fp, AREA_DATA * pArea)
{
   MOB_INDEX_DATA *last_mob;        /* remember the last mob */
   ROOM_INDEX_DATA *last_room;
   RESET_DATA *pReset;
   OBJ_RESET *list;
   bool mob_reboot;                /* TRUE if the last loaded mob reboots at reboot only */

   list = new_list ();                /* allocate memory for a new list */
   last_mob = NULL;
   last_room = NULL;
   mob_reboot = FALSE;

   /* run through all the resets in that area */
   for (pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next)
   {
      /* check what kind of reset it is */

      switch (pReset->command)        /* always in uppercase */
      {

      case 'M':                /* loads a mob */
         {
            if ((last_mob = get_mob_index (pReset->arg1)) == NULL)
            {                        /* mob doesnt seem to exist ? */
               bug ("objdump: 'M': bad vnum %d.\n\r", pReset->arg1);
               continue;        /* next reset */
            }

            if (pReset->arg2 == -1)        /* -1 = load mob/obj at reboot only */
               mob_reboot = TRUE;

            if ((last_room = get_room_index (pReset->arg3)) == NULL)
            {                        /* room doesn't seem to exist ? */
               bug ("objdump: 'R': bad vnum %d.\n\r", pReset->arg3);
               continue;
            }

            break;                /* case M */
         }

      case 'O':                /* load an obj on the ground */
         {
            /*       file, location,   what,          where,   who    inside what */

            if (last_mob)        /* do we have a last mob ? */
               printobj (fp, list, on_ground, pReset->arg1, pReset->arg3,
                         last_mob->vnum, 0, (pReset->arg2 == -1));
            else
               printobj (fp, list, on_ground, pReset->arg1, pReset->arg3, 0, 0, (pReset->arg2 == -1));

            break;                /* case O */
         }

      case 'P':                /* put it inside some obj */
         {
            /* current version is limited, it can only find the vnum of a room
               if the object that this one is inside exists */

            /*      file,         locat,  what,         where, who,            inside what */
            if (last_mob)
               printobj (fp, list, inside, pReset->arg1, 0, last_mob->vnum, pReset->arg3, (pReset->arg2 == -1));
            else
               printobj (fp, list, inside, pReset->arg1, 0, 0, pReset->arg3, (pReset->arg2 == -1));

            break;                /* case P */
         }

      case 'G':                /* create then give to mob */
      case 'E':                /* with an optional equip, but we don't really care here */
         {
            if ((last_mob != NULL) && (last_room != NULL))
               printobj (fp, list, on_mob, pReset->arg1, last_room->vnum, last_mob->vnum, 0,
                         mob_reboot);
            else
               printf ("objdump: 'G' or 'E': no last mob\n\r");

            break;                /* case G,E */
         }

      default:  ;                      /* uninteresting: exits */
      }                                /* case */
   }                                /* for pReset */

   dispose_list (list);                /* dispose of the temporary list of vnum/level pairs */
}                                /* printarea */

/*
   objdump dumps a (commadelimetered) list of _reset_ objs in either the
   world or in one specific area. Not much feedback to user.
 */
void
do_objdump (CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea;
   FILE *fp;

   /* for parsing the command line */
   char filename[MAX_INPUT_LENGTH];
   char dumptype[MAX_INPUT_LENGTH];

   argument = one_argument (argument, dumptype);        /* parse */
   argument = one_argument (argument, filename);

   /* check for valid arguments */
   if ((strlen (filename) == 0) || (strlen (dumptype) == 0))
   {
      do_help (ch, "objdump");        /* show syntax */
      return;
   }

   if (
         (!str_cmp (dumptype, "all")) ||
         (!str_cmp (dumptype, "area"))
    )                                /* valid argument */
   {

      fp = fopen (filename, "w");        /* open for writing */

      if (!fp)                        /* could not create file */
      {
         Cprintf(ch, "Could not open the specified file for output.\n\r");
         return;
      }

      printheader (fp);                /* print ... the header */

      /* now print whatever the player wants */
      if (!str_cmp (dumptype, "all"))
         for (pArea = area_first; pArea != NULL; pArea = pArea->next)
            printarea (fp, pArea);
      else
         printarea (fp, ch->in_room->area);

      fclose (fp);

      Cprintf(ch, "Probably successful.\n\r");
   }                                /* if */
   else                                /* not 'all' or 'area' */
      do_help (ch, "objdump");

}                                /* do_objdump */
