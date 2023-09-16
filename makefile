CC      = gcc
PROF    = -O -g2
NOCRYPT =
C_FLAGS =  $(PROF) $(NOCRYPT)
L_FLAGS =  -lcrypt -lm $(PROF)

C_FILES = act_comm.c affects.c clan.c dump.c handler.c magic.c scan.c string.c \
      act_enter.c alias.c comm.c effects.c healer.c music.c recycle.c sex.c update.c \
      act_info.c ban.c const.c eqlist.c hunt.c note.c remort.c size.c utils.c \
      act_move.c bet.c db2.c fight.c interp.c objdump.c skills.c \
      act_obj.c bit.c db.c flags.c macro.c position.c room_magic.c special.c \
      act_wiz.c board.c deity.c games.c magic2.c quest.c save.c stats.c \
	mobprog/mob_cmds.c mobprog/mob_prog.c mobprog/mob_var.c \
	olc/menuolc.c olc/olc.c olc/olc_save.c olc/olc_act.c olc/olc_act_area.c olc/olc_act_board.c \
      olc/olc_act_help.c olc/olc_act_mob.c olc/olc_act_mprog.c olc/olc_act_object.c olc/olc_act_room.c

O_FILES = act_comm.c affects.o clan.o dump.o handler.o magic.o scan.o string.o \
      act_enter.o alias.o comm.o effects.o healer.o music.o recycle.o sex.o update.o \
      act_info.o ban.o const.o eqlist.o hunt.o note.o remort.o size.o utils.o \
      act_move.o bet.o db2.o fight.o interp.o objdump.o skills.o \
      act_obj.o bit.o db.o flags.o macro.o position.o room_magic.o special.o \
      act_wiz.o board.o deity.o games.o magic2.o quest.o save.o stats.o \
	mobprog/mob_cmds.o mobprog/mob_prog.o mobprog/mob_var.o \
	olc/menuolc.o olc/olc.o olc/olc_save.o olc/olc_act.o olc/olc_act_area.o olc/olc_act_board.o \
      olc/olc_act_help.o olc/olc_act_mob.o olc/olc_act_mprog.o olc/olc_act_object.o olc/olc_act_room.o

UBUNTU_O_FILES = act_comm.o affects.o clan.o dump.o handler.o magic.o scan.o string.o \
      act_enter.o alias.o comm.o effects.o healer.o music.o recycle.o sex.o update.o \
      act_info.o ban.o const.o eqlist.o hunt.o note.o remort.o size.o utils.o \
      act_move.o bet.o db2.o fight.o interp.o objdump.o skills.o \
      act_obj.o bit.o db.o flags.o macro.o position.o room_magic.o special.o \
      act_wiz.o board.o deity.o games.o magic2.o quest.o save.o stats.o \
      mob_cmds.o mob_prog.o mob_var.o \
      menuolc.o olc.o olc_save.o olc_act.o olc_act_area.o olc_act_board.o \
      olc_act_help.o olc_act_mob.o olc_act_mprog.o olc_act_object.o olc_act_room.o
	  
redemption: $(O_FILES)
	rm -f redemption
	rm -f ../area/redemption
	$(CC) -c $(C_FILES) $(C_FLAGS)
	mv menuolc.o olc.o olc_*.o ./olc/
	mv mob_*.o ./mobprog/
	$(CC) -o redemption $(O_FILES) $(L_FLAGS)
#	mv redemption ../area/

ubuntu: $(O_FILES)
	rm -f redemption 
	$(CC) -o redemption $(UBUNTU_O_FILES) $(L_FLAGS)

clean:
	rm -f *.o redemption core
	rm -f ./olc/*.o
	rm -f ./mobprog/*.o

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<
