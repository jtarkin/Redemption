/*********************************************************/
/* defines for olc depth */
/*********************************************************/

#ifndef MENUOLC_H
#define MENUOLC_H

/* main menus */
#define MAIN        1
#define PERSONAL    2
#define AREA        3
#define ROOM        4
#define MOB         5
#define OBJ         6
#define RESET       7
#define HELP        8

/* room sub menus */
#define ROOM_MOD   10
#define ROOM_FORM  11
#define DOOR       12
#define DOOR_MOD   13
#define ROOM_SEC   14
#define ROOM_FLAGS 15

/* mob sub menus */
#define MOB_MOD    30
#define MOB_ATT    31
#define MOB_ACT    32
#define MOB_IMM    33
#define MOB_RES    34
#define MOB_VULN   35
#define MOB_PART   36
#define MOB_FORM   37
#define MOB_RACE   38
#define MOB_POS    39
#define MOB_SIZE   40
#define MOB_AFF    41
#define MOB_SPEC   42
#define MOB_SHOP   43
#define MOB_OFF    44
#define MOB_SEX    45

/* obj sub menus */
#define OBJ_MOD    60
#define OBJ_TYPE   61
#define OBJ_FLAGS  62
#define OBJ_WEAR   63
#define OBJ_VALUE  64
#define OBJ_AFF    65
#define OBJ_SPELL  66
#define OBJ_MATIRIAL 67
#define OBJ_CON    68

#define END_OF_TABLE 10000

/*********************************************************/
/* OLC macroes */
/*********************************************************/
#define DEC_OLC_FUN( fun ) MENU_OLC_FUN fun
typedef void MENU_OLC_FUN ( CHAR_DATA *ch, char *command, char *argument, char *arg );


/*********************************************************/
/* OLC menu selection struct */
/*********************************************************/
typedef struct
{
   int level;
   MENU_OLC_FUN * func;
} olc_level_table;

/*********************************************************/
/* OLC spec's */
/*********************************************************/
void interp_olc( CHAR_DATA *ch, char *argument );
void show_olc_list( CHAR_DATA *ch, int menu_level, bool olc_auto );
void olc_exit( CHAR_DATA *ch );

/*********************************************************/
/* OLC funcs */
/*********************************************************/
/* main levels */
DEC_OLC_FUN( olc_main );
DEC_OLC_FUN( olc_personal );
DEC_OLC_FUN( olc_area );
DEC_OLC_FUN( olc_room );
DEC_OLC_FUN( olc_mob );
DEC_OLC_FUN( olc_obj );
DEC_OLC_FUN( olc_reset );
DEC_OLC_FUN( olc_help );

/* room sub levels */
DEC_OLC_FUN( olc_room_mod );
DEC_OLC_FUN( olc_room_flags );
DEC_OLC_FUN( olc_door );
DEC_OLC_FUN( olc_door_mod );
DEC_OLC_FUN( olc_room_sec );

/* mob sub levels */
DEC_OLC_FUN( olc_mob_mod );
DEC_OLC_FUN( olc_mob_att );
DEC_OLC_FUN( olc_mob_act );
DEC_OLC_FUN( olc_mob_imm );
DEC_OLC_FUN( olc_mob_res );
DEC_OLC_FUN( olc_mob_vuln );
DEC_OLC_FUN( olc_mob_part );
DEC_OLC_FUN( olc_mob_flags );
DEC_OLC_FUN( olc_mob_race );
DEC_OLC_FUN( olc_mob_pos );
DEC_OLC_FUN( olc_mob_size );
DEC_OLC_FUN( olc_mob_aff );
DEC_OLC_FUN( olc_mob_spec );
DEC_OLC_FUN( olc_mob_shop );
DEC_OLC_FUN( olc_mob_off );
DEC_OLC_FUN( olc_mob_form );
DEC_OLC_FUN( olc_mob_sex );

/* obj sub menus */
DEC_OLC_FUN( olc_obj_mod );
DEC_OLC_FUN( olc_obj_type );
DEC_OLC_FUN( olc_obj_flags );
DEC_OLC_FUN( olc_obj_wear );
DEC_OLC_FUN( olc_obj_value );
DEC_OLC_FUN( olc_obj_aff );
DEC_OLC_FUN( olc_obj_spell );
DEC_OLC_FUN( olc_obj_matirial );
DEC_OLC_FUN( olc_obj_con );

#endif

