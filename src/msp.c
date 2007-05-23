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


#ifdef T2_MSP

#include "mud.h"
#include "affect.h"
#include "editor.h"
#include "comm.h"
#include "mprog.h"
#include "msp.h"
#include "mxp.h"
#include "nanny.h"
#include "room.h"


/*
 * Per l'url bisogna utilizzare le '/' e non le '\'
 *	per evitare problemi con i caratteri di escape
 */
#define AUDIO_URL		"http://www.terra-secunda.net/t2snd/"
#define AUDIO_URL_MUSIC	"http://www.terra-secunda.net/t2snd/music/"


/*
 * Variabili esterne
 */
const unsigned char		msp_on_str[] = { IAC, WILL, TELOPT_MSP, '\0' };
	  unsigned char		enable_msp[] = { IAC, SB, TELOPT_MSP, IAC, SE, 0 };


/*
 * Struttura di un elemento sonoro.
 */
struct audio_data
{
	char   *filename;		/* Nome del file */
	int		volume;			/* Volume */
	int		loop;			/* Qaunte volte loopa il suono o la musica */
	int		priority;		/* Priorità del suono */
	int		contin;			/* Indica se riprendere la musica da dove lo si è fermata */
	char   *type;			/* Tipo di suono */
	char   *url;			/* Url da dove scarica l'audio */
	char   *area_to;		/* Area dalla quale se si entra viene attivato la musica */
	char   *area_from;		/* Area da cui si deve entrare per attivare la musica */
	VNUM	room_to;		/* vnum della stanza che attiva la musica (entrando) o il suono (ogni tanto) */
	VNUM	room_from;		/* Vnum da cui uno deve arrivare per attivare il suono o la musica */
};


/*
 * Tabella con tutti i suoni.
 * Attenzione ai nomi d'area che potrebbero cambiare in futuro
 * Valori di default dell'msp per:
 * volume = 100 (da 0 a 100)
 * loop = 1  (-1 loopa all'infinito)
 * priority (per suoni) = 50 (da 0 a 100)
 * continue (per musica) = 1 (0 rinizia da capo, 1 continua senza reiniziare)
 * type = sotto directory dove va l'audio
 * url = url da dove lo deve scaricare
 */
const struct audio_data table_audio[] =
{
/*	  Name				Volume	Loop	Priorit	Continue Type		url					area_to				area_from	room_to	room_from */
	{ "beep.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "deathf.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "deathm.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "doorlocked.wav",	100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "eatfruit.wav",	100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "midnight.wav",	100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "money.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "move0.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "poison.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "preye.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		21176,	0	},
	{ "preyg.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		21180,	0	},
	{ "quit.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "rain.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "sleep.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "sunset.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "unlock.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},
	{ "wake.wav",		100,	1,		50,		1,		 "",		AUDIO_URL,			NULL,				NULL,		0,		0	},

	{ "parry.wav",		100,	1,		50,		1,		 "combat",	AUDIO_URL,			NULL,				NULL,		0,		0	},

	{ "creation.mid",	100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "darkhaven.mid",	100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	"Darkhaven",		NULL,		0,		0	},
	{ "flee.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "gildabardi.mid",	100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,	 	21430,	0	},
	{ "gnomi.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	"Shattered Refuge",	NULL,		0,		0	},
	{ "level.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "login.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "mduel.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "mduel25.mid",	100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
	{ "mongolfiera.mid",100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		32821,	0	},
/*	{ "mononoke.mid",	100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		xxx,	0	},*/
/*	{ "moria.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	"Moria",			NULL,		0,		0	},*/
	{ "pduel.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	NULL,				NULL,		0,		0	},
/*	{ "wild.mid",		100,	1,		50,		1,		 "music",	AUDIO_URL_MUSIC,	"Theluin",			NULL,		0,		0	},*/

	{ NULL,				100,	1,		50,		1,		 NULL,		NULL,				NULL,				NULL,		0,		0	}
};


/*
 * Ritorna vero se il pg può e vuole utilizzare l'msp
 */
bool use_msp( DESCRIPTOR_DATA *d )
{
	if ( !d )
		return FALSE;

	if ( !d->msp )
		return FALSE;

	if ( d->character && IS_PG(d->character) && !HAS_BIT(d->character->pg->protocols, PROTOCOL_MSP) )
		return FALSE;

	return TRUE;
}


/*
 * Off è un nome di file riservato che serve a stoppare suono o musica
 */

void stop_sound( DESCRIPTOR_DATA *d )
{
	if ( use_msp(d) )
		write_to_buffer( d, "!!SOUND(Off)\r\n", 0 );
}

void stop_music( DESCRIPTOR_DATA *d )
{
	if ( use_msp(d) )
		write_to_buffer( d, "!!MUSIC(Off)\r\n", 0 );
}

void stop_music_combat( DESCRIPTOR_DATA *d )
{
	if ( !use_msp(d) )
		return;

	if ( d->character && IS_PG(d->character)
	  && (!str_cmp(d->character->pg->music_file, "mduel.mid")
	   || !str_cmp(d->character->pg->music_file, "mduel25.mid")
	   || !str_cmp(d->character->pg->music_file, "pduel.mid")) )
	{
		write_to_buffer( d, "!!MUSIC(Off)\r\n", 0 );
	}
}


/*
 * Invia l'url da cui scaricare i file audio
 */
void send_audio_url( DESCRIPTOR_DATA *d )
{
	char	buf[MSL];

	if ( !use_msp(d) )
		return;

	sprintf( buf, "!!SOUND(Off U=%s)\r\n", AUDIO_URL );
	write_to_buffer( d, buf, 0 );
}


void send_audio( DESCRIPTOR_DATA *d, const char *sound, int to )
{
	DESCRIPTOR_DATA	*desc;
	char			 tag[MSL];
	int				 x;

	/* d può essere NULL se l'invio del suono è rivolto a tutti i descrittori */
	if ( !d && to != TO_MUD )
		return;

	if ( !VALID_STR(sound) )
	{
		send_log( NULL, LOG_BUG, "send_audio: il nome file sound passato non è valido" );
		return;
	}

	if ( to != TO_CHAR  && to != TO_ROOM /*&& to != TO_AROUND*/
	  && to != TO_ARENA && to != TO_AREA )
	{
		send_log( NULL, LOG_BUG, "send_audio: tipo di invio di suono errato: %d", to );
		return;
	}

	/* capita con mob switchati */
	if ( IS_MOB(d->character) )
		return;

#ifdef T2_MXP
	if ( !use_msp(d) /*|| use_mxp(d) (FF) da attivare quando i suoni e la musica nell'mxp verrà integrata */ )
#else
	if ( !use_msp(d) )
#endif
		return;

	/* Cerca prima la corrisponza name tra i filename della tabella audio */
	for ( x = 0;  VALID_STR(table_audio[x].filename);  x++ )
	{
		if ( !str_cmp(table_audio[x].filename, sound) )
			break;
	}

	if ( !VALID_STR(table_audio[x].filename) )
	{
		send_log( NULL, LOG_BUG, "send_audio: file audio non trovato nella tabella: %s", sound );
		return;
	}

	if ( !str_cmp(table_audio[x].type, "music") )
	{
		if ( !HAS_BIT(d->character->pg->protocols, PROTOCOL_MUSIC) )
			return;
		
		sprintf( tag, "!!MUSIC(%s", table_audio[x].filename );
		if ( table_audio[x].contin != 1 )
			strcat( tag, " C=0" );

		if ( IS_PG(d->character) )
		{
			DISPOSE( d->character->pg->music_file );
			d->character->pg->music_file = str_dup( table_audio[x].filename );
		}
	}
	else
	{
		if ( !HAS_BIT(d->character->pg->protocols, PROTOCOL_SOUND) )
			return;
		if ( !str_cmp(table_audio[x].type, "combat") && !HAS_BIT(d->character->pg->protocols, PROTOCOL_COMBAT) )
			return;

		sprintf( tag, "!!SOUND(%s", table_audio[x].filename );
		if ( table_audio[x].priority != 50 )
			sprintf( tag+strlen(tag), " P=%d", table_audio[x].priority );
	}

	if ( table_audio[x].volume != 100 )
		sprintf( tag+strlen(tag), " V=%d", table_audio[x].volume );	/* (RR) MSP versione 0.3 supporta anche il V=left, right: PROTOCOL_SURROUND */

	if ( table_audio[x].loop != 1 )
		sprintf( tag+strlen(tag), " L=%d", table_audio[x].loop );

	if ( VALID_STR(table_audio[x].type) )
		sprintf( tag+strlen(tag), " T=%s", table_audio[x].type );

	/* Ci vuole per forza l'url perché pare che senza zmud non scarichi nulla -_- */
	sprintf( tag+strlen(tag), " U=%s)\r\n", table_audio[x].url );

	/* (FF) da finire */
#ifdef T2_ALFA
#ifdef T2_MXP
	sprintf( tag, "%s<SOUND \"%s\" %d 1 %d \"%s\" %s>",
		MXPMODE(1),					/* secure mode */
		table_audio[x].filename,
		table_audio[x].volume,
		table_audio[x].priority,
		table_audio[x].type,
		table_audio[x].url );
#endif
#endif

	if ( to == TO_CHAR )
	{
		write_to_buffer( d, tag, 0 );
		return;
	}

	for ( desc = first_descriptor;  desc;  desc = desc->next )
	{
		if ( !desc->character )					continue;
		if ( !desc->character->in_room )		continue;
		if ( desc->connected != CON_PLAYING )	continue;

		/* Invia a tutti quelli della stanza in cui si trova *d */
		if ( to == TO_ROOM && desc->character->in_room == d->character->in_room )
			write_to_buffer( d, tag, 0 );

		/* Invia a tutti quelli della stanza in cui si trova *d e a quelli delle stanze attorno (FF) da finire */
/*		if ( to == TO_AROUND && desc->character->in_room == d->character->in_room )
			write_to_buffer( d, tag, 0 );*/

		/* Invia a tutti quelli dell'arena in cui si trova *d */
		if ( to == TO_ARENA && desc->character->in_room->area == d->character->in_room->area
		  && HAS_BIT(desc->character->in_room->area->flags, AREAFLAG_ARENA) )
		{
			write_to_buffer( d, tag, 0 );
		}

		/* Invia a tutti quelli dell'area in cui si trova *d */
		if ( to == TO_AREA && desc->character->in_room->area == d->character->in_room->area )
			write_to_buffer( d, tag, 0 );

		if ( to == TO_MUD )
			write_to_buffer( d, tag, 0 );
	}
}


/*
 * Controlla se deve inviare una musica quando un pg entra in un'area o stanza
 */
void check_audio_location( CHAR_DATA *ch )
{
	int		x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_msp_location: ch è NULL" );
		return;
	}

	if ( !ch->in_room )
	{
		send_log( NULL, LOG_BUG, "check_msp_location: la stanza in cui si trova il pg è NULL" );
		return;
	}

	if ( !ch->was_in_room )
	{
		send_log( NULL, LOG_BUG, "check_msp_location: la stanza in cui si trovava il pg è NULL" );
		return;
	}

	if ( !ch->desc )
		return;

	if ( IS_MOB(ch) )
		return;

	for ( x = 0;  VALID_STR(table_audio[x].filename);  x++ )
	{
		/* Controlla prima lo spostamento da stanza a stanza,
		 *	per l'attivazione dei suoni di entrata room_from è obbligatioria */
		if ( table_audio[x].room_to != 0 && table_audio[x].room_to == ch->in_room->vnum )
		{
			/* Se è musica allora la room_from è facoltativa */
			if ( !str_cmp(table_audio[x].type, "music") )
			{
				if ( table_audio[x].room_from != 0 && table_audio[x].room_from != ch->was_in_room->vnum )
					continue;

				send_audio( ch->desc, table_audio[x].filename, TO_CHAR );
			}
			else
			{
				/* Per i suoni invece la room_from è obbligatiorio controllarla,
				 *	visto che la room_to da sola viene utilizzata dalla
				 *	update_sound() per i suoni d'atmosfera
				 */
				if ( table_audio[x].room_from == 0 && table_audio[x].room_from == ch->was_in_room->vnum )
					send_audio( ch->desc, table_audio[x].filename, TO_CHAR );
			}

			continue;
		}

		/* Poi controlla lo spostamento da area ad area */
		if ( ch->was_in_room->area != ch->in_room->area )
		{
			/* Procedura per l'invio della musica con area_from facoltativa */
			if (  VALID_STR(table_audio[x].area_to) && get_area(table_audio[x].area_to)	== ch->in_room->area
			  && !VALID_STR(table_audio[x].area_from) )
			{
				if ( str_cmp(ch->pg->music_file, table_audio[x].filename) )
					send_audio( ch->desc, table_audio[x].filename, TO_CHAR );
			}

			/* Invio di musica con anche area_from impostata */
			if ( VALID_STR(table_audio[x].area_to)	 && get_area(table_audio[x].area_to)   == ch->in_room->area
			  && VALID_STR(table_audio[x].area_from) && get_area(table_audio[x].area_from) == ch->was_in_room->area )
			{
				if ( str_cmp(ch->pg->music_file, table_audio[x].filename) )
					send_audio( ch->desc, table_audio[x].filename, TO_CHAR );
			}
		}
	}
}


/*
 * Invia la musica in combattimento
 */
void send_music_combat( CHAR_DATA *ch, CHAR_DATA *victim )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_music_combat: ch è NULL" );
		return;
	}

	if ( !victim )
	{
		send_log( NULL, LOG_BUG, "send_music_combat: victim è NULL" );
		return;
	}

	/* Niente musichetta per i mob */
	if ( IS_MOB(ch) && IS_MOB(victim) )
		return;

	if ( IS_PG(ch) && IS_MOB(victim) && victim->level - ch->level > 2 )
	{
		if ( VALID_STR(ch->pg->music_file) )
			return;

		if ( get_percent(ch->points[POINTS_LIFE], ch->points_max[POINTS_LIFE]) > 25 )
			send_audio( ch->desc, "mduel.mid", TO_CHAR );
		else
			send_audio( ch->desc, "mduel25.mid", TO_CHAR );
	}
	else if ( IS_MOB(ch) && IS_PG(victim) && ch->level - victim->level > 2 )
	{
		if ( VALID_STR(victim->pg->music_file) )
			return;

		if ( get_percent(victim->points[POINTS_LIFE], victim->points_max[POINTS_LIFE]) > 25 )
			send_audio( victim->desc, "mduel.mid", TO_CHAR );
		else
			send_audio( victim->desc, "mduel25.mid", TO_CHAR );
	}
	else if ( IS_PG(ch) && IS_PG(victim) )
	{
		/* Se tutti e due pg invia la musica apposta del pk */
		if ( !VALID_STR(ch->pg->music_file) )
			send_audio( ch->desc, "pduel.mid", TO_CHAR );
		if ( !VALID_STR(victim->pg->music_file) )
			send_audio( victim->desc, "pduel.mid", TO_CHAR );
	}
}


/*
 * Permette al pg di scaricare tutti i file sonori per il proprio client
 */
void download_audio( CHAR_DATA *ch )
{
	int		x;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "donwload_audio: ch è NULL" );
		return;
	}

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "donwload_audio: descrittore di %s è NULL", ch->name );
		return;
	}

	if ( !ch->desc->msp )
	{
		ch_printf( ch, "Il tuo Client (%s) non supporta l'MSP - Mud Sound Protocol che serve per"
			"poter ascoltare suoni e musica del Mud oppure tale protocollo e disattivato.\r\n",
			ch->desc->client );
		send_to_char( ch, "I Client Mud che supportano il protocollo MSP sono:\r\n" );
		send_to_char( ch, "zMUD 4.60+      - http://www.zuggsoft.com" );
		send_to_char( ch, "Mudmaster 2000  - http://www.mud-master.com" );
		send_to_char( ch, "GosClient       - http://gosclient.altervista.org" );
		send_to_char( ch, "  NB: per GosClient bisogna installare il plugin apposito" );
		send_to_char( ch, "Wintin 1.80     - http://www.wintin.org" );
		send_to_char( ch, "Se conoscete altri client che supportano l'MSP informateci che li\r\n" );
		send_to_char( ch, "aggiungeremo alla lista\r\n" );
	}

	ch_printf( ch, "Scaricamento della musica o del sonoro di %s:\r\n", MUD_NAME );
	for ( x = 0;  VALID_STR(table_audio[x].filename);  x++ )
	{
		char	tag[MSL];

		ch_printf( ch, "%s\r\n", table_audio[x].filename );

		if ( !str_cmp(table_audio[x].type, "music") )
			strcpy( tag, "!!MUSIC" );
		else
			strcpy( tag, "!!SOUND" );
		sprintf( tag+strlen(tag), "(%s V=0 L=0 T=%s U=%s)\r\n",
			table_audio[x].filename, table_audio[x].type, table_audio[x].url );

		write_to_buffer( ch->desc, tag, 0 );

		stop_music( ch->desc );
		stop_sound( ch->desc );
	}

	send_to_char( ch, "Download terminato.\r\n" );
}


/*
 * Aggiorna l'invio di suoni d'atmosfera quando il pg si trova in una stanza particolare
 */
void update_sound( void )
{
	DESCRIPTOR_DATA *d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		int x;

		if ( !d->character )				continue;
		if ( !d->character->in_room )		continue;
		if ( d->connected != CON_PLAYING )	continue;

		for ( x = 0;  VALID_STR(table_audio[x].filename);  x++ )
		{
			/* Invia i suoni di stanza */
			if ( !str_cmp(table_audio[x].type, "music") )					continue;
			if ( table_audio[x].room_to != d->character->in_room->vnum )	continue;
			if ( number_range(0, 3) == 0 )	/* (FF) Volendo si può aggiungere un campo alla table_audio con la probabilità di invio di ciascun suono */
			{
				send_audio( d, table_audio[x].filename, TO_CHAR );
				break;	/* Invia un solo suono per volta */
			}

			/* Invia i suoni atmosferici */
			if ( !is_outside(d->character) )				continue;
			if ( !is_awake(d->character) )					continue;
			if ( !d->character->in_room->area )				continue;
			if ( !d->character->in_room->area->weather )	continue;

			if ( number_range(0, 3) == 0 )
			{
				switch ( d->character->in_room->area->weather->sky )
				{
				  default:
					send_log( NULL, LOG_BUG, "update_sound: tipo di cielo inesistente: %d",
						d->character->in_room->area->weather->sky );
					break;
	
				  case SKY_CLOUDY:
				  case SKY_CLOUDLESS:
					break;
	
				  case SKY_RAINING:
					send_audio( d, "rain.wav", TO_CHAR );
					break;
	
				  case SKY_SNOW:
/*					send_audio( d, ".wav", TO_CHAR ); */
					break;
	
				  case SKY_LIGHTNING:
/*					send_audio( d, ".wav", TO_CHAR ); */
					break;
				}
	
				break;	/* invia un solo suono meteo per volta */
			}
		}
	}
}


/*
 * Prints message only to victim
 */
DO_RET do_mpaudio( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char	   arg1[MIL];	/* nome vittima */
	char	   arg2[MIL];	/* filename sonoro */

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_mplog( ch, "mpsound: primo o secondo argomento passato vuoto" );
		return;
	}

	victim = get_player_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "mpsound: la victim %s non è stata trovata", argument );
		return;
	}

	if ( !VALID_STR(argument) || !str_cmp(argument, "char") )
		send_audio( victim->desc, argument, TO_CHAR );
	else if ( !str_cmp(argument, "room") )
		send_audio( victim->desc, argument, TO_ROOM );
/*	else if ( !str_cmp(argument, "around") )
		send_audio( victim->desc, argument, TO_AROUND );
	else if ( !str_cmp(argument, "arena") )
		send_audio( victim->desc, argument, TO_ARENA );*/
	else if ( !str_cmp(argument, "area") )
		send_audio( victim->desc, argument, TO_AREA );
	else
		send_mplog( ch, "mpsound: tipo di invio audio errato, argument: %s", argument );
}


#endif	/* T2_MSP */
