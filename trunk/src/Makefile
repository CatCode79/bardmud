#Attenzione che se si sceglie di compilare con l'opzione -pg (che serve al gprof) al copyover si blocca
CC      = gcc -g3 -Wall -O
#PROF   = -p

#Leggi il file LEGGI.MI prima di avviare questo MUD o modificare questo file.
#La porta di gioco di default è la 5001, la puoi cambiare modificando il file "startup"

#Nome dell'eseguibile:
PROGNAME = bard

#Decommenta la riga sottostante per non volere le password dei player criptate
NOCRYPT = -DNOCRYPT

#Decommenta la riga sotto se compare un messaggio di warning di questo tipo:
#interpret.c:757: warning: int format, time_t arg (arg 7)
TIME = -DTIMEFORMAT

#Decommenta il messaggio sotto se si ha un messaggio tipo: "implicit declaration of re_exec"
REG = -DREGEX

#Decommenta la linea seguente se ottieni errori di questo tipo: undefined re_exec errors
#NEED_REG = -lgnuregex

#Decommentare la riga sottostante se compare l'errore di compilazione "undefined crypt errors" o simile
NEED_CRYPT = -lcrypt

#Attivandolo si compilano tutti gli include inseriti in mud.h è comodo nel caso non si
# vogliano inserire ad ogni file *.c i vari include, ingrandisce però un po' l'eseguibile
#ALL_HEADER


#Togliere il commento alla linea sotto per non avere i messaggi di debug di alcune procedure del codice;
# meglio compilare e testare con una opzione di debug per volta.
D_FLAGS = -DDEBUG -DDEBUG_MEMORY #-DDEBUG_MEMORYLEAK -DDEBUG_QUESTION #-DDEBUG_LOAD #-DDEBUG_MPROG -DDEBUG_RESET -DDEBUG_MOVECHAR


#Flag di compilazione proprietaria
T_FLAGS = -DT2_DETCLIENT -DT2_MSP -DT2_WEB -DT2_LOCKER -DT2_CHESS #-DT2_SAVECONVERT #-DT2_QUESTION #-DT2_MXP #-DT2_MCCP #-DT2_VAMPIRE #-DT2_CNV_SMAUG #-DT2_ALFA -DT2_SKILL


#Altri, vedere il codice:
#-DGOLD_MULT -DREAD_AREA -DFNDELAY -DEAGAIN -DUNDEFD


#Flag di compilazione warning
W_FLAGS = -Wformat-security -Winline -Wmissing-format-attribute -Wpointer-arith -Wredundant-decls -Wstrict-prototypes -Wshadow -Wuninitialized


C_FLAGS = $(OPT_FLAG) $(PROF) $(NOCRYPT) $(TIME) $(REG) -export-dynamic
L_FLAGS = $(OPT_FLAG) $(PROF) $(NEED_CRYPT) -ldl -export-dynamic -lm #-lz


C_FILES = adminhost.c       admin.c           affect.c          alias.c           animation.c       \
          archetype.c       autoquest.c       ban.c             bank.c            bitvector.c       \
          build.c           calendar.c        channel.c         clan.c            class.c           \
          code.c            color.c           comm.c            command.c         const.c           \
          convert.c         db.c              diagnose.c        dream.c           economy.c         \
          editor.c          experience.c      fight.c           fread.c           game_chess.c      \
          gramm.c           group.c           handler_ch.c      help.c            herb.c            \
          hint.c            information.c     interpret.c       language.c        liquid.c          \
          locker.c          lyric.c           magic.c           make_obj.c        mapper.c          \
          mccp.c            md5.c             memory.c          morph.c           movement.c        \
          mprog.c           mprog_ifcheck.c   mprog_trigger.c   msp.c             mxp.c             \
          nanny.c           news.c            note.c            object_handler.c  object_interact.c \
          olc_medit.c       olc_oedit.c       olc_redit.c       partcorp.c        player.c          \
          punition.c        question.c        quote.c           race.c            reboot.c          \
          reserved.c        reset.c           room.c            save.c            shop.c            \
          skills.c          social.c          special.c         starmap.c         sysdata.c         \
          timer.c           track.c           update.c          utility.c         values.c          \
          watch.c           weapon.c          weather.c         web.c             who.c

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES  = $(wildcard *.h)


all:
	make -s $(PROGNAME)

$(PROGNAME): $(O_FILES)
	$(CC) -o premud $(O_FILES) $(L_FLAGS)
	echo "O==[:::::::::::> Compilazione BARD completata <:::::::::::]==O";
	rm -f $(PROGNAME)
	mv premud $(PROGNAME)

# Check in all files
checkin:
	ci -l $(C_FILES) $(H_FILES) Makefile
	rm -f *~

# Make a unified diff of all files.
diff:
	rcsdiff -u $(C_FILES) $(H_FILES) Makefile > ../diff.new
	gzip -9 ../diff.new

o/%.o: %.c
	echo "BARD 0.6u -- Compilazione di $@";
	$(CC) -c $(C_FLAGS) $(D_FLAGS) $(T_FLAGS) $(W_FLAGS) $< -o $@

clean:
	rm -f o/*.o *~ lastcmd* $(PROGNAME)

backup:
	rm -f o/*.o $(PROGNAME) src.tar.gz
	tar -cvzf src.tar.gz *

restore:
	tar -xzvf src.tar.gz
	make

