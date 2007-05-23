/*--------------------------------------------------------------------------------------*\
                                      Copyright 2002, 2003, 2004, 2005 by T2Team         |
  Bard Code                           The Bard Code has been developed by T2Team:        |
  Driad Release     ..;;;;;.      (_) - Amlugil, Celyr, Miciko, Onirik, Raul, Sharas     |
  BARD v0.6u      .~`" >------___/  / Older Members of T2Team:                           |
               ,'"  J"!||||||||:/  /  - Jamirithiana, Qaba, Tanegashima                  |
    -~,      ,'  ,;"!|||||||||!/  /   ---------------------------------------------------|
 ;;/   `^~`~_._/!|||||||||||!//  /    The Bard Code is derived from:                     |
 "".  =====!/!|||||||||||||!//  /     - DikuMUD coded by: Hans Staerfeldt, Katja Nyboe,  |
 .'  / |||||||||||||||||||//   /        Tom Madsen, Michael Seifert and Sebastian Hammer |
.'  /  ||||||||||||||||!//    /       - MERC 2.1 coded by Hatchet, Furey, and Kahn       |
;  !   ||||||||||||||!/X/    /        - SMAUG 1.4a coded by Thoric (Derek Snider) with:  |
:  !   ||||||||||||!/X/     /           Altrag, Blodkai, Haus, Narn, Scryn, Swordbearer, |
:  !   |||||||||||/X/      /            Tricops, Gorog, Rennard, Grishnakh, Fireblade    |
:  !   |||||||||!/X/      /             and Nivek                                        |
:   \  |||||||!/X/       /                                                               |
 "   \ |||||!/x/        /                                                                |
  "   \|||!/X/         /                                                                 |
   \   \|/X/          /                                                                  |
    "   !/           /                ... and the blood!                                 |
     ;'/            /                                                                    |
      "------------(                  --------------------------------------------------*/


/****************************************************************************\
 >				Modulo dei comandi di Punizione degli Admin		   			<
\****************************************************************************/

#include <errno.h>
#include <sys/stat.h>

#include "mud.h"
#include "admin.h"
#include "bank.h"
#include "build.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "interpret.h"
#include "locker.h"
#include "mprog.h"
#include "room.h"
#include "save.h"


/*
 * Typedef di una funzione puny_ e remove_
 * (FF) inglobarla nella DO_FUN quando verrà convertita
 */
typedef		DO_RET	PUNY( CHAR_DATA *ch, CHAR_DATA *victim, char *argument );


/*
 * Definizione del file contenente l'immagine ascii dello slay
 */
#define SLAY_FILE	SYSTEM_DIR "slay.file"	/* File testuale della slay */


/*
 * Struttura di una tipologia di punizione per la tabella delle puzioni più sotto
 */
struct punition_data
{
	char	*type;			/* Stringa identificatrice del tipo di punizione */
	int		 trust;			/* Fiducia minima per utilizzare la punizione */
	int		 log;			/* Tipo di log con cui viene loggata la punizione */
	bool	 world;			/* Indica se la vittima va cercata nel mondo o nella stanza */
	bool	 mob;			/* Indica se la punzione può essere fatta sui mob o no */
	PUNY	*fun_puny;		/* Indirizzo alla funzione di punizione */
	PUNY	*fun_remove;	/* Indirizzo alla funzione di rimozione punizione, se esiste */
};


/*
 *
 */
DO_RET remove_attacker( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_ATTACKER) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_ATTACKER );
		ch_printf( ch, "Attacker flag removed from %s.\r\n", victim->name );
		send_to_char( victim, "You are no longer an ATTACKER.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta la flag di attacker ad un pg
 */
DO_RET puny_attacker( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_ATTACKER) )
	{
		SET_BIT( victim->pg->flags, PLAYER_ATTACKER );
		ch_printf( ch, "Attacker flag set for %s.\r\n", victim->name );
		send_to_char( victim, "You are now an ATTACKER.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Rimuove la flag di thief ad un pg
 */
DO_RET remove_thief( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_THIEF) )
	{
		REMOVE_BIT(victim->pg->flags, PLAYER_THIEF );
		ch_printf( ch, "Thief flag removed from %s.\r\n", victim->name );
		send_to_char( victim, "You are no longer a THIEF.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta la flag di thief ad un pg
 */
DO_RET puny_thief( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_THIEF) )
	{
		SET_BIT(victim->pg->flags, PLAYER_THIEF );
		ch_printf( ch, "Thief flag set for %s.\r\n", victim->name );
		send_to_char( victim, "You are now a THIEF.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Rimuove la flag di killer ad un pg
 */
DO_RET remove_killer( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_KILLER) )
	{
		REMOVE_BIT(victim->pg->flags, PLAYER_KILLER );
		ch_printf( ch, "Killer flag removed from %s.\r\n", victim->name );
		send_to_char( victim, "You are no longer a KILLER.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta la flag di killer ad un pg
 */
DO_RET puny_killer( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_KILLER) )
	{
		SET_BIT(victim->pg->flags, PLAYER_KILLER );
		ch_printf( ch, "Killer flag set for %s.\r\n", victim->name );
		send_to_char( victim, "You are now a KILLER.\r\n" );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Teleporta il pg a caso in una stanza
 */
DO_RET puny_scatter( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	ROOM_DATA *pRoom;

	set_char_color( AT_ADMIN, ch );

	for ( ; ; )
	{
		pRoom = get_room_index( NULL, number_range(1, MAX_VNUM_AREA-1) );
		if ( pRoom )
		{
			if ( !HAS_BIT(pRoom->flags, ROOM_NOASTRAL) )
				break;
		}
	}

	if ( victim->fighting )
		stop_fighting( victim, TRUE );

	act( AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOVICT );
	act( AT_MAGIC, "With the sweep of an arm, $n flings you to the astral winds.", ch, NULL, victim, TO_VICT );
	act( AT_MAGIC, "With the sweep of an arm, you fling $N to the astral winds.", ch, NULL, victim, TO_CHAR );

	char_from_room( victim );
	char_to_room( victim, pRoom );
	victim->position = POSITION_REST;

	act( AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM );
	send_command( victim, "look", CO );
}


/*
 * Sparge l'inventario o le monete del pg a caso nell'area in cui si trova il pg
 */
DO_RET puny_strew( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	OBJ_DATA	*obj_next;
	OBJ_DATA	*obj_lose;
	ROOM_DATA	*pRoom;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Strew their 'coins' or 'inventory'?\r\n" );
		return;
	}

	if ( is_name(argument, "monete oro soldi money gold coin coins") )
	{
		if ( victim->gold < 1 )
		{
			send_to_char( ch, "Drat, this one's got no gold to start with.\r\n" );
			return;
		}
		victim->gold = 0;
		act( AT_MAGIC, "An unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_ROOM );
		act( AT_MAGIC, "An unearthly gale sends your currency flying!", ch, NULL, victim, TO_VICT );
		return;
	}

	for ( ; ; )
	{
		pRoom = get_room_index( NULL, number_range(victim->in_room->area->vnum_low, victim->in_room->area->vnum_high) );
		if ( pRoom )
		{
			if ( !HAS_BIT(pRoom->flags, ROOM_NOASTRAL) )
				break;
		}
	}

	if ( is_name(argument, "inventario inventory") )
	{
		act( AT_MAGIC, "An unearthly gale sends $N's possessions flying!", ch, NULL, victim, TO_ROOM );
		act( AT_MAGIC, "An unearthly gale sends your possessions flying!", ch, NULL, victim, TO_VICT );

		for ( obj_lose = victim->first_carrying;  obj_lose;  obj_lose = obj_next )
		{
			obj_next = obj_lose->next_content;
			obj_from_char( obj_lose );
			obj_to_room( obj_lose, pRoom );
			pager_printf( ch, "\t&w%s sent to %d\r\n", capitalize(obj_lose->short_descr), pRoom->vnum );
		}

		return;
	}

	send_to_char( ch, "Strew their 'coins' or 'inventory'?\r\n" );
}


/*
 * Rimuove un pg dall'Inferno
 */
DO_RET remove_hell( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	ROOM_DATA	*location;

	set_char_color( AT_ADMIN, ch );

	if ( victim->in_room->vnum != VNUM_ROOM_HELL
	  && victim->in_room->vnum != 1206
	  && victim->in_room->vnum != 6 )
	{
		send_to_char( ch, "No one like that is in hell.\r\n" );
		return;
	}

	if ( victim->pg->clan )
		location = victim->pg->clan->recall;
	else
		location = get_room_index( NULL, VNUM_ROOM_TEMPLE );

	if ( !location )
		location = ch->in_room;

	MOBtrigger = FALSE;
	act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOVICT );
	char_from_room( victim );
	char_to_room( victim, location );
	send_to_char( victim, "The gods have smiled on you and released you from hell early!\r\n" );
	send_command( victim, "look", CO );

	if ( victim != ch )
		send_to_char( ch, "They have been released.\r\n" );

	if ( victim->pg->helled_by )
	{
		if ( str_cmp(ch->name, victim->pg->helled_by) )
		{
			ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\r\n",
				victim->pg->helled_by );
		}
		DISPOSE( victim->pg->helled_by );
		victim->pg->helled_by = NULL;
	}
	MOBtrigger = FALSE;
	act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOVICT );
	victim->pg->release_date = 0;

	save_player( victim );
}


/*
 * Manda un pg in un inferno
 */
DO_RET puny_hell( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	char	   arg[MIL];
	int		   hell_time;
	bool	   h_d = FALSE;
	struct tm *tms;

	set_char_color( AT_ADMIN, ch );

	if ( victim->pg->release_date != 0 )
	{
		ch_printf( ch, "They are already in hell until %21.21s, by %s.\r\n",
			friendly_ctime(&victim->pg->release_date), victim->pg->helled_by );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !is_number(arg) )
	{
		send_to_char( ch, "Hell them for how long?\r\n" );
		return;
	}

	hell_time = atoi( arg );
	if ( hell_time <= 0 )
	{
		send_to_char( ch, "You cannot hell for zero or negative time.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if		( !VALID_STR(arg) || is_name(arg, "ora ore hour hours") )
		h_d = TRUE;
	else if ( is_name(arg, "giorno giorni day days") )
	{
		send_to_char( ch, "Is that value in hours or days?\r\n" );
		return;
	}
	else if ( hell_time > 30 )
	{
		send_to_char( ch, "You may not hell a person for more than 30 days at a time.\r\n" );
		return;
	}

	tms = localtime( &current_time );
	if ( h_d )
		tms->tm_hour += hell_time;
	else
		tms->tm_mday += hell_time;

	victim->pg->release_date = mktime( tms );
	DISPOSE( victim->pg->helled_by );
	victim->pg->helled_by = str_dup( ch->name );

	ch_printf( ch, "%s will be released from hell at %21.21s.\r\n", victim->name,
		friendly_ctime(&victim->pg->release_date) );
	act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOVICT );

	char_from_room( victim );
	char_to_room( victim, get_room_index(NULL, VNUM_ROOM_HELL) );

	act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOVICT );
	send_command( victim, "look", CO );
	ch_printf( victim, "The admins are not pleased with your actions.\r\n"
		"You shall remain in hell for %d %s%s.\r\n", hell_time,
		(h_d  ?  "hour"  :  "day"), (hell_time == 1)  ?  ""  :  "s" );

	save_player( victim );
}


/*
 * Disconnette la tal vittima
 */
DO_RET puny_disconnect( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );

	if ( victim->desc == NULL )
	{
		ch_printf( ch, "%s doesn't have a descriptor.", victim->name );
		return;
	}

	close_socket( victim->desc, FALSE );
	ch_printf( ch, "Hai disconnesso %s.\r\n", victim->name );
}


/*
 * Rimuove il delay ad un pg
 */
DO_RET remove_delay( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	victim->wait = 0;
	ch_printf( ch, "Annulli il delay a %s.\r\n", victim->name );
}

/*
 * Imposta il delay di turni ad un pg
 */
DO_RET puny_delay( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	int	ndelay;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "Quanti round vuoi di delay su %s?\r\n", victim->name );
		return;
	}

	if ( str_cmp(argument, "0 zero rimuovi remove") )
	{
		remove_delay( ch, victim, "" );
		return;
	}

	ndelay = atoi( argument );
	if ( ndelay < 1 )
	{
		send_to_char( ch, "Inserisci un numero positivo.\r\n" );
		return;
	}

	if ( ndelay > 99 )
	{
		send_to_char( ch, "Troppo alto.\r\n" );
		return;
	}

	WAIT_STATE( victim, ndelay * PULSE_IN_SECOND );
	ch_printf( ch, "Delay di %d turni per il giocatore %s.\r\n", ndelay, victim->name );
}


/*
 * Rimuove la flag di litterbug
 */
DO_RET remove_litterbug( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_LITTERBUG) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_LITTERBUG );
		send_to_char( victim, "You can drop items again.\r\n" );
		ch_printf( ch, "LITTERBUG removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Imposta la flag di litterbug
 */
DO_RET puny_litterbug( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_LITTERBUG) )
	{
		SET_BIT( victim->pg->flags, PLAYER_LITTERBUG );
		send_to_char( victim, "A strange force prevents you from dropping any more items!\r\n" );
		ch_printf( ch, "LITTERBUG set on %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Permette nuovamente al pg di utilizzare il comando title
 */
DO_RET remove_notitle( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );

	if ( HAS_BIT(victim->pg->flags, PLAYER_NOTITLE) )
	{
		REMOVE_BIT(victim->pg->flags, PLAYER_NOTITLE);
		ch_printf( ch, "NOTITLE removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Impedisce al pg di settare un proprio title.
 */
DO_RET puny_notitle( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );

	if ( !HAS_BIT(victim->pg->flags, PLAYER_NOTITLE) )
	{
		SET_BIT( victim->pg->flags, PLAYER_NOTITLE );
		if ( victim->sex == SEX_FEMALE )
			set_title( victim, "la Viaggiatrice dei Piani" );
		else
			set_title( victim, "il Viaggiatore dei Piani" );
		ch_printf( ch, "NOTITLE set on %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Permette al pg di utilizzare nuovamente gli emote
 */
DO_RET remove_noemote( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_NOEMOTE) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_NOEMOTE );
		send_to_char( victim, "You can emote again.\r\n" );
		ch_printf( ch, "NOEMOTE removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Impedisce al pg di utilizzare gli emote
 */
DO_RET puny_noemote( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_NOEMOTE) )
	{
		SET_BIT(victim->pg->flags, PLAYER_NOEMOTE);
		send_to_char( victim, "You can't emote!\r\n" );
		ch_printf( ch, "NOEMOTE applied to %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Permette al pg di utilizzare nuovamente i social
 */
DO_RET remove_nosocial( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_NOSOCIAL) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_NOSOCIAL );
		send_to_char( victim, "You can social again.\r\n" );
		ch_printf( ch, "NOSOCIAL removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Impedisce al pg di utilizzare gli emote
 */
DO_RET puny_nosocial( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_NOSOCIAL) )
	{
		SET_BIT(victim->pg->flags, PLAYER_NOSOCIAL);
		send_to_char( victim, "You can't emote!\r\n" );
		ch_printf( ch, "NOSOCIAL applied to %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Permette al pg di utilizzare i tell
 */
DO_RET remove_notell( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_NOTELL) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_NOTELL );
		send_to_char( victim, "You can use tells again.\r\n" );
		ch_printf( ch, "NOTELL removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Impedisce al pg di utilizzare i tell
 */
DO_RET puny_notell( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_NOTELL) )
	{
		SET_BIT( victim->pg->flags, PLAYER_NOTELL );
		send_to_char( victim, "You can't use tells!\r\n" );
		ch_printf( ch, "NOTELL applied to %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Rimuove alla vittima la punizione di exphalve
 */
DO_RET remove_exphalve( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_EXPHALVE) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_EXPHALVE );
		ch_printf( victim, "Puoi comunicare nuovamente con i comandi %s, %s e %s ora.\r\n",
			translate_command(victim, "tell"),
			translate_command(victim, "reply"),
			translate_command(victim, "gtell") );
		ch_printf( ch, "NOTELL removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta alla vittima la punzione di exphalve, che dimezza i px guadagnati
 */
DO_RET puny_exphalve( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_EXPHALVE) )
	{
		SET_BIT( victim->pg->flags, PLAYER_EXPHALVE );
		ch_printf( ch, "Guadagno di esperienza dimezzato per %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Rimuove ad un pg la flag di nuisance
 */
DO_RET remove_nuisance( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_NUISANCE) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_NUISANCE );
		ch_printf( ch, "NUISANCE removed to %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta ad un pg la flag di nuisance
 */
DO_RET puny_nuisance( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_NUISANCE) )
	{
		SET_BIT( victim->pg->flags, PLAYER_NUISANCE );
		ch_printf( ch, "NOTELL applied to %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Rimuove la flag di silenziatura ad un pg
 */
DO_RET remove_silence( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_SILENCE) )
	{
		REMOVE_BIT( victim->pg->flags, PLAYER_SILENCE );
		send_to_char( victim, "You can use channels again.\r\n" );
		ch_printf( ch, "SILENCE removed from %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}

/*
 * Imposta la flag di silenziatura ad un pg
 */
DO_RET puny_silence( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_SILENCE) )
	{
		SET_BIT( victim->pg->flags, PLAYER_SILENCE );
		send_to_char( victim, "You can't use channels!\r\n" );
		ch_printf( ch, "You SILENCE %s.\r\n", victim->name );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


DO_RET do_slay( CHAR_DATA *ch, char *argument )
{
	char	 buf[MIL];
	char	 arg[MIL];
	char	*msg;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Puoi personallizare lo slay tramite la seguente sintassi:\r\n" );
		send_to_char( ch, "slay vittima <messaggio>\r\n" );
		send_to_char( ch, "slay stanza <messaggio>\r\n" );
		send_to_char( ch, "slay cancella\r\n" );
		send_to_char( ch, "Utilizza dove serve le tag di act, le rpincipali qui ricordate sono:\r\n" );
		send_to_char( ch, "$n = nome tuo\r\n" );
		send_to_char( ch, "$N = nome della vittima\r\n" );
		send_to_char( ch, "$x = o/a dipende dal sesso tuo\r\n" );
		send_to_char( ch, "$X = o/a dipende dal sesso della vittima\r\n" );
		send_to_char( ch, "esempio: viv$x risulta vivo se tu sei maschio viva se femmina\r\n\r\n" );

		send_to_char( ch, "PS: tutta sta roba è meglio spostarla in un help più decente di questo, vero Sharas?\r\n\r\n" );

		send_to_char( ch, "Attualmente la tua slay è:\r\n" );
		send_to_char( ch, "Messaggio alla vittima:\r\n" );
		ch_printf( ch, "%s\r\n", VALID_STR(ch->pg->msg_slay_victim) ? ch->pg->msg_slay_victim : "nessuno" );
		send_to_char( ch, "Messaggio alla stanza:\r\n" );
		ch_printf( ch, "%s\r\n", VALID_STR(ch->pg->msg_slay_room) ? ch->pg->msg_slay_room : "nessuno" );

		return;
	}

	msg = one_argument( argument, arg );
	if ( VALID_STR(arg) )
	{
		if ( is_name(arg, "vittima victim") )
		{
			DISPOSE( ch->pg->msg_slay_victim );
			ch->pg->msg_slay_victim = str_dup( msg );
			return;
		}

		if ( is_name(arg, "stanza room") )
		{
			DISPOSE( ch->pg->msg_slay_room );
			ch->pg->msg_slay_room = str_dup( msg );
			return;
		}

		if ( is_name(arg, "reset cancella") )
		{
			DISPOSE( ch->pg->msg_slay_victim );
			DISPOSE( ch->pg->msg_slay_room );
			return;
		}
	}

	sprintf( buf, "punition slay %s", argument );
	send_command( ch, buf, CO );
}


/*
 * Uccide un mob o pg a sangue freddo, invia ai pg un file con disegnato un demone
 */
DO_RET puny_slay( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	/* Se ha uno slay personalizzato invia quello, altrimenti quello di default */
	if ( VALID_STR(ch->pg->msg_slay_victim) || VALID_STR(ch->pg->msg_slay_room) )
	{
		act( AT_ADMIN, ch->pg->msg_slay_victim, ch, NULL, victim, TO_VICT );
		act( AT_ADMIN, ch->pg->msg_slay_room,	ch, NULL, victim, TO_NOVICT );
	}
	else
	{
		act( AT_ADMIN, "Con un gesto appare un Demone dei Piani Inferiori.", ch, NULL, victim, TO_CHAR );
		act( AT_ADMIN, "Con un ghigno satanico, l'orribile creatura salta addosso a $N, che grida di terrore prima di essere mangiat$x viv$x.",
			ch, NULL, victim, TO_CHAR );

		act( AT_ADMIN, "$n fa un gesto e appare un Demone. L'orribile creatura mi salta addosso!", ch, NULL, victim, TO_VICT );
		act( AT_ADMIN, "Sto gridando! STO GRIDANDO! Ma è inutile mi sta mangiando viv$x.", ch, NULL, victim, TO_VICT );
		if ( IS_PG(victim) )
			show_file( victim->desc, SLAY_FILE, TRUE );

		act( AT_ADMIN, "$n fa un gesto e appare un Demone.", ch, NULL, victim, TO_NOVICT );
		act( AT_ADMIN, "Con un ghigno satanico, l'orribile creatura salta addosso a $N, che grida di terrore prima di essere mangiat$x viv$x.",
			ch, NULL, victim, TO_NOVICT );
	}

	set_cur_char( victim );
	raw_kill( ch, victim );
}


/*
 * Rimuove il freeze da un pg
 */
DO_RET remove_freeze( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_LCYAN, ch );
	set_char_color( AT_LCYAN, victim );

	if ( HAS_BIT_PLR(victim, PLAYER_FREEZE) )
	{
		REMOVE_BIT(victim->pg->flags, PLAYER_FREEZE );
		send_to_char( victim, "Your frozen form suddenly thaws.\r\n" );
		ch_printf( ch, "%s is now unfrozen.\r\n", victim->name );
		save_player( victim );
		return;
	}

	send_to_char( ch, "Il giocatore è già libero da questa punizione.\r\n" );
}


/*
 * Imposta il freeze ad un pg
 */
DO_RET puny_freeze( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_LCYAN, ch );
	set_char_color( AT_LCYAN, victim );

	if ( !HAS_BIT_PLR(victim, PLAYER_FREEZE) )
	{
		SET_BIT(victim->pg->flags, PLAYER_FREEZE );
		send_to_char( victim, "A godly force turns your body to ice!\r\n" );
		ch_printf( ch, "You have frozen %s.\r\n", victim->name );
		save_player( victim );
		return;
	}

	send_to_char( ch, "Il giocatore ha già questa punizione.\r\n" );
}


/*
 * Impedisce ad un pg di riconnettersi nuovamente
 */
DO_RET puny_deny( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	set_char_color( AT_ADMIN, ch );
	set_char_color( AT_ADMIN, victim );

	SET_BIT( victim->pg->flags, PLAYER_DENY );

	send_to_char( victim, "You are denied access!\r\n" );
	ch_printf( ch, "You have denied access to %s.\r\n", victim->name );

	if ( victim->fighting )
		stop_fighting( victim, TRUE );

	send_command( victim, "quit", CO );
}


/*
 * Distrugge un file di save del pg, tanti saluti pg
 */
DO_RET puny_destroy( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
	struct stat fst;
	char		buf[MSL];
	char		buf2[MSL];
	int			x;
	int			y;

	set_char_color( AT_RED, ch );

	/* Set the file points */
	sprintf( buf, "%s%s", PLAYER_DIR, victim->name );
	sprintf( buf2, "%s%s", BACKUP_DIR, victim->name );

	victim = get_player_mud( ch, victim->name, FALSE );
	if ( !victim )
	{
		send_to_char( ch, "Non esiste nessun pg con questo nome.\r\n" );
		return;
	}

	/* This check makes sure the file is there */
	if ( stat(buf, &fst) == -1 )
	{
		ch_printf( ch, "Nessun file per il giocatore dal nome %s.\r\n", victim->name );
		return;
	}

	quitting_char = victim;

	save_player( victim );
	remove_offline( victim );

	remove_all_accounts_player( ch );	/* Rimuove gli eventuali i conti bancari */
	remove_all_file_locker( victim );	/* Rimuove gli eventuali locker */

	saving_char = NULL;
	extract_char( victim, TRUE );

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;
	}

	if ( rename(buf, buf2) == 0 )
	{
		AREA_DATA *pArea;

		set_char_color( AT_RED, ch );
		ch_printf(ch, "Player %s destroyed.  Pfile saved in backup directory.\r\n", victim->name );	/* (RR) si come no... */

		sprintf( buf, "%s%s", ADMIN_DIR, victim->name );
		if ( !remove(buf) )
		{
			send_to_char( ch, "Player's admin data destroyed.\r\n" );
		}
		else if ( errno != ENOENT )
		{
			send_log( NULL, LOG_FP, "puny_destroy: unknown error #%d - %s (admin data %s)",
				errno, strerror(errno), buf );
			send_log( NULL, LOG_WARNING, "%s destroying %s", ch->name, buf );
		}

		sprintf( buf2, "%s.are", victim->name );
		for ( pArea = first_build;  pArea;  pArea = pArea->next )
		{
			if ( !str_cmp(pArea->filename, buf2) )
			{
				sprintf( buf, "%s%s", BUILD_DIR, buf2 );
				if ( HAS_BIT(pArea->flags, AREAFLAG_LOADED) )
					fold_area( pArea, buf, FALSE );
				close_area( pArea );

				sprintf( buf2, "%s.bak", buf );
				set_char_color( AT_RED, ch ); /* Log message changes colors */
				if ( rename(buf, buf2) == 0 )
					send_to_char( ch, "Player's area data destroyed.  Area saved as backup.\r\n" );
				else if ( errno != ENOENT )
				{
					ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Coder.\r\n",
						errno, strerror(errno) );
					send_log( NULL, LOG_WARNING, "%s destroying %s", ch->name, buf );
				}
				break;
			}
		}
	}
	else if ( errno == ENOENT )
	{
		set_char_color( AT_PLAIN, ch );
		send_to_char( ch, "Player does not exist.\r\n" );
	}
	else
	{
		set_char_color( AT_WHITE, ch );
		ch_printf( ch, "Unknown error #%d - %s.  Report to Coder.\r\n",
			errno, strerror(errno) );
		send_log( NULL, LOG_WARNING, "%s destroying %s", ch->name, victim->name );
	}
}


/*
 * Tabella delle punizioni
 */
struct punition_data const table_puny[] =
{
	/* type 		trust				log			world	mob		fun_puny			fun_remove */
	{ "attacker",	TRUST_NEOMASTER,	LOG_PUNY,	TRUE,	FALSE,	puny_attacker,		remove_attacker		},
	{ "thief",      TRUST_NEOMASTER,	LOG_PUNY,	TRUE,	FALSE,	puny_thief,			remove_thief		},
	{ "killer",		TRUST_NEOMASTER,	LOG_PUNY,	TRUE,	FALSE,	puny_killer,		remove_killer		},
	{ "scatter",	TRUST_MASTER,		LOG_PUNY,	TRUE,	TRUE,	puny_scatter,		NULL				},
	{ "strew",		TRUST_MASTER,		LOG_PUNY,	TRUE,	TRUE,	puny_strew,			NULL				},
	{ "hell",		TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_hell,			remove_hell			},
	{ "disconnect",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_disconnect,	NULL				},
	{ "delay",		TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_delay,			remove_delay		},
	{ "litterbug",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_litterbug,		remove_litterbug	},
	{ "notitle",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_notitle,		remove_notitle		},
	{ "noemote",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_noemote,		remove_noemote		},
	{ "nosocial",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_nosocial,		remove_nosocial		},
	{ "notell",		TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_notell,		remove_notell		},
	{ "exphalve",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_exphalve,		remove_exphalve		},
	{ "nuisance",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_nuisance,		remove_nuisance		},
	{ "silence",	TRUST_MASTER,		LOG_PUNY,	TRUE,	FALSE,	puny_silence,		remove_silence		},
	{ "slay",		TRUST_MASTER,		LOG_PUNY,	FALSE,	TRUE,	puny_slay,			NULL				},
	{ "freeze",		TRUST_BUILDER,		LOG_PUNY,	TRUE,	FALSE,	puny_freeze,		remove_freeze		},
	{ "deny",		TRUST_BUILDER,		LOG_PUNY,	TRUE,	FALSE,	puny_deny,			NULL				},
	{ "destroy",	TRUST_IMPLE,		LOG_PUNY,	TRUE,	FALSE,	puny_destroy,		NULL				},
	{ "",			0,					0,			FALSE,	FALSE,	NULL,				NULL				}
};


/*
 * Gestisce tutti i comandi di punizione e di rimozione delle punizioni
 */
DO_RET do_punition( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	char		 arg_log[MSL];
	char		 arg1[MIL];
	char		 arg2[MIL];
	bool		 found = FALSE;
	int			 x;

	set_char_color( AT_ADMIN, ch );

	strcpy( arg_log, argument );	/* Copia per il log della punizione poi */

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		for ( x = 0;  VALID_STR(table_puny[x].type);  x++ )
		{
			if ( get_trust(ch) >= table_puny[x].trust )
			{
				ch_printf( ch, "&YSintassi&w:  %-10s <nomepg>%s\r\n",
					table_puny[x].type,
					(table_puny[x].fun_remove) ? " (togli)" : "" );
			}
		}

		return;
	}

	for ( x = 0;  VALID_STR(table_puny[x].type);  x++ )
	{
		if ( get_trust(ch) < table_puny[x].trust )
			continue;

		if ( str_cmp(arg1, table_puny[x].type) )
			continue;

		if ( table_puny[x].world )
		{
			if ( table_puny[x].mob == TRUE )
				victim = get_mob_mud( ch, arg2, TRUE );
			else
				victim = get_char_mud( ch, arg2, TRUE );
		}
		else
		{
			if ( table_puny[x].mob == TRUE )
				victim = get_mob_room( ch, arg2, TRUE );
			else
				victim = get_char_room( ch, arg2, TRUE );
		}

		if ( !victim )
		{
			send_to_char( ch, "Non trovi la tua vittima.\r\n" );
			return;
		}

		if ( get_trust(victim) >= get_trust(ch) )
		{
			send_to_char( ch, "Non puoi, ha una fiducia uguale o più alta alla tua.\r\n" );
			return;
		}

		if ( !str_cmp(argument, "togli") )
		{
			if ( !table_puny[x].fun_remove )
				continue;
			( *table_puny[x].fun_remove )	( ch, victim, argument );
		}
		else
		{
			if ( !table_puny[x].fun_puny )
				continue;
			( *table_puny[x].fun_puny )	( ch, victim, argument );
		}

		found = TRUE;
		break;
	}

	if ( found )
	{
		send_log( NULL, table_puny[x].log, "do_punition: %s %s",
			table_puny[x].type, arg_log );
		return;
	}

	send_to_char( ch, "Tipo di punizione inesistente o non supportatabile.\r\n" );
}
