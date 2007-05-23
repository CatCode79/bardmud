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


/***************************************************************************\
 >                    Modulo Comunicativo Giocatori                        <
\***************************************************************************/


#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>		/* getpid() */

#include "mud.h"
#include "affect.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"
#include "group.h"
#include "interpret.h"
#include "mprog.h"
#include "msp.h"
#include "mxp.h"
#include "nanny.h"
#include "olc.h"
#include "quote.h"
#include "room.h"
#include "save.h"
#include "timer.h"
#include "web.h"


/*
 * Tabella dei codici dei canali
 */
const struct channel_type table_channel[] =
{
/*		code				name				                    verb_I			verb_it			rpg		*/
	{ { CHANNEL_MURMUR,		"CHANNEL_MURMUR",	"mormorare"		},	"Mormoro",		"mormora",		TRUE	},
	{ { CHANNEL_WHISPER,	"CHANNEL_WHISPER",	"sussurrare"	},	"Sussurro",		"sussurra",		TRUE	},
	{ { CHANNEL_SAY,		"CHANNEL_SAY",		"dire"			},	"Dico",			"dice",			TRUE	},
	{ { CHANNEL_SING,		"CHANNEL_SING",		"cantare"		},	"Canto",		"canta",		TRUE	},
	{ { CHANNEL_YELL,		"CHANNEL_YELL",		"urlare"		},	"Urlo",			"urla",			TRUE	},
	{ { CHANNEL_SHOUT,		"CHANNEL_SHOUT",	"gridare"		},	"Grido",		"grida",		TRUE	},
	{ { CHANNEL_TELL,		"CHANNEL_TELL",		"comunicare"	},	"Comunichi",	"comunica",		FALSE	},
	{ { CHANNEL_GTELL,		"CHANNEL_GTELL",	"gcomunicare"	},	"GComunichi",	"gcomunica",	FALSE	},
	{ { CHANNEL_THINK,		"CHANNEL_THINK",	"pensare"		},	"Penso",		"pensa",		FALSE	},
	{ { CHANNEL_QTALK,		"CHANNEL_QTALK",	"qparlare"		},	"&gQparlo&Y",	"&gqparla&Y",	FALSE	},
	{ { CHANNEL_ADMIN,		"CHANNEL_ADMIN",	"AdminTalk"		},	"&GAdminTalk&Y","&GAdminTalk&Y",FALSE	}
};
const int max_check_channel = sizeof(table_channel)/sizeof(struct channel_type);


/*
 * Funzione generale dei canali non rpg.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel )
{
	DESCRIPTOR_DATA *d;
	char			 buf[MSL];
	char			 buf2[MSL];	/* Parte di messaggio da non colorizzare */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "talk_channel: ch è NULL" );
		return;
	}

	if ( channel < 0 || channel >= MAX_CHANNEL )
	{
		send_log( NULL, LOG_BUG, "talk_channel: valore di channel passato errato: %d", channel );
		return;
	}

	if ( table_channel[channel].rpg == TRUE )
	{
		send_log( NULL, LOG_BUG, "talk_channel: il canale passato è rpg: %s", code_name(NULL, channel, CODE_CHANNEL) );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Quale messaggio?\r\n" );
		return;
	}

	/* Forse inutile questo check */
	if ( IS_MOB(ch) && HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		if ( ch->master )
			send_to_char( ch->master, NOFOUND_MSG );

		return;
	}

	/* Se ch è sordo a quel canale utilizzandolo rimuove la sordità */
	if ( IS_PG(ch) )
		REMOVE_BIT( ch->pg->deaf, channel );

	set_char_color( AT_ADMIN, ch );

	ch_printf		 ( ch, "%s> ", table_channel[channel].verb_I );
	ch_printf_nocolor( ch, "'%s'\r\n", argument );

	sprintf( buf, "%s %s> ", ch->name, table_channel[channel].verb_it );
	sprintf( buf2, "'%s'\r\n", argument );

	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->original  ?  d->original  :  d->character;
		vch = d->character;

		if ( !is_in_game(d) )					continue;
		if ( vch == ch )						continue;
		if ( IS_MOB(och) )						continue;
		if ( HAS_BIT(och->pg->deaf, channel) )	continue;
		if ( get_trust(och) < TRUST_NEOMASTER )	continue;	/* Non viene utilizzato IS_ADMIN per evitare il check sulla flag PLAYER_NOADMIN */

		set_char_color( AT_ADMIN, vch );
		send_to_char		( vch, buf );
		send_to_char_nocolor( vch, buf2 );
	} /* chiude il for che cicla tra i descriptor */
}

/*
 * Canale Admtalk, permette la comunicazione tra Admin
 */
DO_RET do_admtalk( CHAR_DATA *ch, char *argument )
{
	talk_channel( ch, argument, CHANNEL_ADMIN );
}

/*
 * Canale Pensa, permette la comunicazione offgdr o di servizio tra giocatori e Admin.
 * Utile perché può focalizzare l'attenzione su scene rpg interessanti.
 */
DO_RET do_think( CHAR_DATA *ch, char *argument )
{
	talk_channel( ch, argument, CHANNEL_THINK );
}

/*
 * Canale di quest, permette ai pg di dialogare con i master che gestiscono la quest
 */
DO_RET do_qtalk( CHAR_DATA *ch, char *argument )
{
	talk_channel( ch, argument, CHANNEL_QTALK );
}


/*
 * Funzione che gestisce i differenti tipi di echo
 */
void echo_to( const char *argument, CHAR_DATA *victim, int to )
{
	DESCRIPTOR_DATA *d;
	static char		 buf[MSL];
	int				 i;

	strcpy( buf, argument );

	if ( !VALID_STR(buf) )
		return;

	if ( to == TO_MUD && victim != NULL )
	{
		send_log( NULL, LOG_BUG, "echo_to: TO_MUD con victim non a NULL" );
		return;
	}

	if ( to != TO_MUD && victim == NULL )
	{
		send_log( NULL, LOG_BUG, "echo_to: to non TO_MUD con victim a NULL" );
		return;
	}

	/* Salta l'eventuale colore ad inizio frase così da touppare il primo carattere */
	for ( i = 0;  buf[i];  i++ )
	{
		if ( buf[i] == '&' )
		{
			i++;
			if ( buf[i] )
				continue;
		}
		else
		{
			buf[i] = toupper( buf[i] );
		}

		break;
	}

	strcat( buf, "\r\n" );

	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( d->connected != CON_PLAYING )	continue;
		if ( !d->character )				continue;
#if 0
		if ( IS_MOB(d->character) )			continue;	/* This one is kinda useless except for switched.. non credo di volerlo, ma magari crasha (TT) */
#endif
		if ( to == TO_VICT && d->character != victim )									continue;
		if ( to == TO_ROOM && d->character->in_room != victim->in_room )				continue;
		if ( to == TO_AREA && d->character->in_room->area != victim->in_room->area )	continue;

		set_char_color( AT_RESET, d->character );
		send_to_char( d->character, buf );
	}
}


DO_RET do_gecho( CHAR_DATA *ch, char *argument )
{
	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Quale messaggio vuoi inviare globalmente a tutto il Mud?\r\n" );
		return;
	}

	echo_to( argument, NULL, TO_MUD );
	send_log( NULL, LOG_ECHO, "gecho: %s", argument );
	ch_printf( ch, "Invii l'echo globale: %s", argument );
}


DO_RET do_aecho( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: aecho <nomepg> <messaggio>\r\n" );
		send_to_char( ch, "&YIl messaggio si sentirà in tutta l'area di <nomepg>\r\n" );
		return;
	}

	if ( IS_ADMIN(ch) && HAS_BIT_PLR(ch, PLAYER_ECHOIN) )
	{
		victim = ch;
	}
	else
	{
		char	arg[MIL];

		argument = one_argument( argument, arg );
		victim = get_char_mud( ch, arg, TRUE );
		if ( !victim )
		{
			ch_printf( ch, "Non esiste nessun %s nel Mud.\r\n", arg );
			return;
		}
	}

	echo_to( argument, victim, TO_AREA );
	send_log( NULL, LOG_ECHO, "aecho (%s): %s", victim->name, argument );
	ch_printf( ch, "Invii l'echo area a %s: %s", victim->name, argument );
}


DO_RET do_recho( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: recho <nomepg> <messaggio>\r\n" );
		send_to_char( ch, "&YIl messaggio si sentirà nella room di <nomepg>\r\n" );
		return;
	}

	if ( IS_ADMIN(ch) && HAS_BIT_PLR(ch, PLAYER_ECHOIN) )
	{
		victim = ch;
	}
	else
	{
		char	arg[MIL];

		argument = one_argument( argument, arg );
		victim = get_char_mud( ch, arg, TRUE );
		if ( !victim )
		{
			ch_printf( ch, "Non esiste nessun %s nel Mud.\r\n", arg );
			return;
		}
	}

	echo_to( argument, victim, TO_ROOM );
	send_log( NULL, LOG_ECHO, "recho (%s): %s", victim->name, argument );
	ch_printf( ch, "Invii l'echo room a %s: %s", victim->name, argument );
}


DO_RET do_echo( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *victim;
	char		arg[MIL];

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "&YSintassi&w: echo <nomepg> <messaggio>\r\n" );
		send_to_char( ch, "&YIl messaggio lo sentirà solo <nomepg>\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	victim = get_char_mud( ch, arg, TRUE );
	if ( !victim )
	{
		ch_printf( ch, "Non esiste nessun %s nel Mud.\r\n", arg );
		return;
	}

	send_to_char( ch, "Ricordarsi di utilizzare la prima persona nell'invio di questo tipo di echo.\r\n" );
	echo_to( argument, victim, TO_VICT );
	send_log( NULL, LOG_ECHO, "echo (%s): %s", victim->name, argument );
	ch_printf( ch, "Invii l'echo a %s: %s", victim->name, argument );
}


/* (FF) Fare magari una analisi linguistica dell'ubriaco in italiano e poi modificare il parlato
 * Parlantina-Ubriaca.
 */
char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
	const char	*arg = argument;
	static char	 buf [MIL*2];
	char		 buf1[MIL*2];
	int			 drunk;
	char		*txt;
	char		*txt1;

	strcpy( buf, argument );	/* Copia temporaneamente argument in buf per ritornarlo in caso di errore */

	if ( !VALID_STR(argument) )
	{
		send_log( NULL, LOG_BUG, "drunk_speech: argument è NULL." );
		return "";
	}

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "drunk_speech: ch è NULL con argument %s", argument );
		return buf;
	}

	if ( IS_MOB(ch) )
		return buf;

	if ( !ch->pg )
	{
		send_log( NULL, LOG_BUG, "drunk_speech: struttura pg_data di %s è NULL", ch->name );
		return buf;
	}

	drunk = ch->pg->condition[CONDITION_DRUNK];
	if ( drunk <= 0 )
		return buf;

	buf[0]  = '\0';
	buf1[0] = '\0';
	txt  = buf;
	txt1 = buf1;

	while ( *arg != '\0' )
	{
		if ( toupper(*arg) == 'T' )
		{
			if ( number_percent( ) < (2 * drunk) )		/* add 'h' after an 'T' */
			{
				*txt++ = *arg;
				*txt++ = 'h';
			}
			else
				*txt++ = *arg;
		}
		else if ( toupper(*arg) == 'X' )
		{
			if ( number_percent() < (2 * drunk / 2) )
			{
				*txt++ = 'c';
				*txt++ = 's';
				*txt++ = 'h';
			}
			else
			{
				*txt++ = *arg;
			}
		}
		else if ( number_percent() < (2 * drunk / 5) )			/* slurred letters */
		{
			int slurn = number_range( 1, 2 );
			int currslur = 0;

			while ( currslur < slurn )
			{
				*txt++ = *arg;
				currslur++;
			}
		}
		else
		{
			*txt++ = *arg;
		}

		arg++;
	}
	*txt = '\0';
	 txt = buf;

	while ( *txt != '\0' )			/* Let's mess with the string's caps */
	{
		if ( number_percent() < (2 * drunk / 2.5) )
		{
			if ( isupper(*txt) )
				*txt1 = tolower( *txt );
			else
			{
				if ( islower(*txt) )
					*txt1 = toupper( *txt );
				else
					*txt1 = *txt;
			}
		}
		else
		{
			*txt1 = *txt;
		}

		txt1++;
		txt++;
	}
	*txt1 = '\0';
	 txt1 = buf1;
	 txt  = buf;

	while ( *txt1 != '\0' )		/* Let's make them stutter */
	{
		if ( *txt1 == ' ' )		/* If there's a space, then there's gotta be a along there somewhere soon */
		{
			while ( *txt1 == ' ' )	/* Don't stutter on spaces */
				*txt++ = *txt1++;

			if ( (number_percent() < (2 * drunk / 4)) && *txt1 != '\0' )
			{
				int offset = number_range( 0, 2 );
				int pos = 0;

				while ( *txt1 != '\0' && pos < offset )
				{
					*txt++ = *txt1++;
					pos++;
				}

				if ( *txt1 == ' ' )		/* Make sure not to stutter a space after the initial offset into the word */
				{
					*txt++ = *txt1++;
					continue;
				}

				pos = 0;
				offset = number_range( 2, 4 );
				while (	*txt1 != '\0' && pos < offset )
				{
					*txt++ = *txt1;
					pos++;

					if ( *txt1 == ' ' || pos == offset )	/* Make sure we don't stick a hyphen right before a space */
					{
						txt1--;
						break;
					}
					*txt++ = '-';
				}
				if ( *txt1 != '\0' )
					txt1++;
			}
		}
		else
		{
			*txt++ = *txt1++;
		}

	} /* chiude il while */

	*txt = '\0';
	return buf;
}


/* Tipi di riferimento */
#define	REF_ROOM	0
#define	REF_VICTIM	1
#define	REF_SELF	2
#define REF_GROUP	3

/* Tipi di espressioni */
#define EXP_NORMAL		0
#define EXP_EXCLAMATION	1
#define EXP_QUESTION	2

/*
 * Funzione madre di tutti i canali rpg con Sistema Faccine integrato
 */
void rpg_channel( CHAR_DATA *ch, char *argument, int channel )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA   *vch = NULL;
	CHAR_DATA   *rch = NULL;		/* pg di riferimento a cui si rivolge la parola */
	BITVECTOR	*memflags = NULL;	/* Variabile in cui memorizza le flag del mob o del pg per poi recuperarle */
	char		 arg [MSL];
	char		 temp[MSL];
	char		*dbuf;				/* drunk buf, salva il messaggio della stringa mischiata dalla parlata da ubriaco */
	char		*sbuf;				/* scrable buf, serve per salvare la stringa mischiata per le lingue */

	int			x;
	int			color;
	int			length;
	int			cutting		= -1;			/* indica a quale lunghezza bisogna troncare la stringa cancellando per esempio gli smile a fine frase */
	int			type_expres	= EXP_NORMAL;	/* tipo di espressione, cioè come parla */
	int			type_reference = REF_ROOM;	/* tipo di riferimento, cioè a chi parla */
	char		expres_smile[MIL];
	char		expres_voice[MIL];
	char		expression  [MIL];
	char		expres_I	[MIL];
	char		expres_it   [MIL];
	char		expres_ref  [MIL];
	char		lang		[MIL];
	bool		found_smile;			/* indica se ha trovato uno smile o social in argument */
	bool		found_voice;			/* indica se ha trovato uno tag di voce ** */

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "rpg_channel: ch è NULL" );
		return;
	}

	if ( channel < 0 || channel >= MAX_CHANNEL )
	{
		send_log( NULL, LOG_BUG, "rpg_channel: valore di channel passato errato: %d", channel );
		return;
	}

	if ( table_channel[channel].rpg == FALSE )
	{
		send_log( NULL, LOG_BUG, "rpg_channel: canale passato non rpg: %s", code_name(NULL, channel, CODE_CHANNEL) );
		return;
	}

	/* Controlla che sia stato passato un argomento valido */
	if ( !VALID_STR(argument) )
	{
		char	schan[MIL];	/* stringa di canale all'infinito */

		strcpy( schan, code_name(NULL, channel, CODE_CHANNEL) );
		ch_printf( ch, "%s che cosa?\r\n", capitalize(schan) );	/* (FF) Utilizzare i verbi della razza */
		return;
	}

	/* Copia l'argomento in una variabile temporanea per riprenderla nel caso in cui ch
	 *	non si stia rivolgendo a nessuno
	 */
	strcpy( temp, argument );

	/* Controlla se il ch che parla si stia rivolgendo a qualcuno */
	argument = one_argument( argument, arg );
	if ( VALID_STR(arg) && strlen(arg) == 1 )
	{
		if ( arg[0] == 'a' )	/* si riferisce ad un persona? */
		{
			/* Acquisizione del nome ch con cui si parla */
			argument = one_argument( argument, arg );

			if ( VALID_STR(arg) && VALID_STR(argument) )
			{
				rch = get_char_room( ch, arg, TRUE );
				if ( rch )
				{
					/* Se il pg e il riferimento sono uguali allora parla a se stesso altrimenti ad un'altra persona */
					if ( ch == rch )
						type_reference = REF_SELF;
					else
						type_reference = REF_VICTIM;	/* ad una persona */
				}
			}
		}
		else if ( arg[0] == ':' )	/* si riferisce ad un gruppo? */
		{
			if ( !VALID_STR(argument) )
			{
				char	schan[MIL];	/* stringa di canale all'infinito */

				strcpy( schan, code_name(NULL, channel, CODE_CHANNEL) );
				ch_printf( ch, "%s al gruppo che cosa?\r\n", capitalize(schan) );	/* (FF) Utilizzare i verbi della razza */
				return;
			}

			/* Controlla che almeno un membro del'eventuale gruppo sia nella stanza */
			if ( count_group_room(ch) == 0 )
			{
				send_to_char( ch, "Non ho nessun membro nel gruppo con cui comunicare.\r\n" );
				return;
			}
			else
				type_reference = REF_GROUP;
		}
	}

	if ( type_reference == REF_ROOM )
		argument = temp;


	/* (FF) se ci si trova in una stanza subacquea solo i lizard e altre razze "marine" riescono a parlare tra loro, flag di razza */
	if ( ch->in_room->sector == SECTOR_UNDERWATER )
	{
		if ( IS_ADMIN(ch) )
			send_to_char( ch, "[ADM] Anche se la stanza è Underwater gli Admin riescono a comunicare:\r\n" );
		else
		{
			ch_printf( ch, "Tento di %s qualcosa ma per poco non soffoco a causa dell'acqua che mi riempie i polmoni!\n\r",
				code_name(NULL, channel, CODE_CHANNEL) );
			ch->points[POINTS_LIFE] -= get_level( ch ) / 4;
			return;
		}
	}

	if ( HAS_BIT(ch->in_room->flags, ROOM_SILENCE) )
	{
		if ( IS_ADMIN(ch) )
			send_to_char( ch, "[ADM] Anche se la stanza è Silence gli admin possono comunicare:\r\n" );
		else
		{
			send_to_char( ch, "Il silenzio del luogo mi blocca impedendomi di parlare.\r\n" );
			return;
		}
	}

	if ( IS_MOB(ch) )
	{
		SET_BITS( memflags, ch->act );
		REMOVE_BIT( ch->act, MOB_SECRETIVE );	/* (FF) Per il canale murmur e whisper bisognerebbe aggiungere un controllo per togliere l'act_secretive */
	}
	else
		SET_BITS( memflags, ch->pg->flags );

	/* Elabora i punti esclamativi e interrogativi per il canale say.
	 * Controlla gli ultimi 8 caratteri */
	if ( channel == CHANNEL_SAY )
	{
		length = strlen( argument );
		for ( x = 0;  x < 8;  x++ )
		{
			/* Decrementa la lunghezza della stringa la prima volta per non fare il
			 *	controllo sullo '\0', se è minore di zero esce */
			if ( --length < 0 )
				break;

			switch ( argument[length] )
			{
			  case '!':		type_expres = EXP_EXCLAMATION;	cutting = length;	break;
			  case '?':		type_expres = EXP_QUESTION;		cutting = length;	break;
			}

			/* Esce subito se ha trovato qualcosa */
			if ( type_expres != EXP_NORMAL )
				break;
		}
	}


	/*
	 * Ricerca eventuali smiles nella stringa.
	 * Controlla gli ultimi 8 caratteri, saltando il controllo all'ultimo, alla ricerca di smile
	 */
	found_smile = FALSE;
	for ( x = 1;  x < 8;  x++ )
	{
		SOCIAL_DATA	*soc;

		for ( soc = first_social;  soc;  soc = soc->next )
		{
			char	arg_smile[8];

			if ( !VALID_STR(soc->smiles) )
				continue;

			/* Se argument è composto solo dallo smile allora inviare il social relativo */
			if ( is_name(argument, soc->smiles) )
			{
				char	cmd[MIL];

				if ( type_reference == REF_VICTIM || type_reference == REF_SELF )
					sprintf( cmd, "%s %s", soc->english, IS_MOB(rch) ? rch->short_descr : rch->name );
				else
					sprintf( cmd, "%s", soc->english );

				send_command( ch, cmd, CO );
				destroybv( memflags );
				return;
			}

			length = strlen( argument ) - 1;	/* lunghezza di argument-1 per non puntare al '\0'*/
			length -= x;						/* lunghezza - numero di caratteri a qui è giunto */
			if ( length < 0 )	/* Se la lunghezza è minore di zero, allora non deve accedervi ed esce */
				break;

			one_argument( argument+length, arg_smile );
			if ( is_name(arg_smile, soc->smiles) )
			{
				found_smile = TRUE;

				/* Piazzo cutting (la "forbice") per togliere via lo smile */
				if ( cutting < 0 || cutting > length )
				{
					cutting = --length;
					if ( !isspace(argument[cutting-1]) )
					{
						cutting++;
						length++;
					}
				}

				strcpy( expres_smile, soc->expression );
				break;
			}
		}

		if ( found_smile )
			break;
	}

	/*
	 * Cerca il tag ** per il tipo di voce
	 * Gli ultimi 24 caratteri penso siano sufficente per ricavarlo
	 */
	found_voice = FALSE;
	for ( x = 1;  x < 24;  x++ )
	{
		char	arg_voice[MIL];

		length = strlen( argument ) - 1;	/* lunghezza di argument-1 per non puntare al '\0'*/
		length -= x;						/* lunghezza - numero di caratteri a qui è giunto */
		if ( length - 2 < 0 )	/* Se la lunghezza è minore di zero, allora non deve accedervi ed esce */
			break;

		if ( argument[length]   == '*'
		  && argument[length-1] == '*'
		  && argument[length-2] == ' ' )
		{
			one_argument( argument+length+1, arg_voice );
			if ( !VALID_STR(arg_voice) )
				continue;

			found_voice = TRUE;

			/* Piazzo cutting (la "forbice") per togliere via il tipo di voce */
			if ( cutting < 0 || cutting > length )
				cutting = length-2;

			sprintf( expres_voice, "con voce %s ", arg_voice );
			break;
		}
	}

	/* Inizializza le stringhe da concatenare */
	expression	[0] = '\0';
	expres_I	[0] = '\0';
	expres_it	[0] = '\0';
	expres_ref	[0] = '\0';

	/* Elabora il primo pezzo di stringa da inviare */
	switch ( type_reference )
	{
	  case REF_ROOM:
	  {
		switch ( type_expres )
		{
		  case EXP_NORMAL:
			sprintf( expres_I, "%s ", table_channel[channel].verb_I );		/* (FF) anche qui rumore di razza */

			if ( channel == CHANNEL_WHISPER || channel == CHANNEL_MURMUR )
				sprintf( expres_it,  "%s qualcosa ", table_channel[channel].verb_it  );
			else
				sprintf( expres_it,  "%s ", table_channel[channel].verb_it  );
			break;

		  case EXP_EXCLAMATION:
			sprintf( expres_I,	"Esclamo " );
			sprintf( expres_it,	"esclama " );
			break;

		  case EXP_QUESTION:
			sprintf( expres_I,	"Domando " );
			sprintf( expres_it,	"domanda " );
			break;
		}
	  }
	  break;

	  case REF_VICTIM:
	  {
		switch ( type_expres )
		{
		  case EXP_NORMAL:
			sprintf( expres_I, "%s a %s ", table_channel[channel].verb_I, rch->name );

			if ( channel == CHANNEL_WHISPER || channel == CHANNEL_MURMUR )
				sprintf( expres_it,	"%s a %s qualcosa ", table_channel[channel].verb_it,  rch->name );
			else
				sprintf( expres_it,	"%s a %s ", table_channel[channel].verb_it,  rch->name );

			sprintf( expres_ref, "ti %s ",  table_channel[channel].verb_it	);
			break;

		  case EXP_EXCLAMATION:
			sprintf( expres_I,	 "Esclamo a %s ", rch->name );
			sprintf( expres_it,	 "esclama a %s ", rch->name );
			sprintf( expres_ref, "ti esclama " );
			break;

		  case EXP_QUESTION:
			sprintf( expres_I,	 "Domando a %s ", rch->name );
			sprintf( expres_it,	 "domanda a %s ", rch->name );
			sprintf( expres_ref, "ti domanda " );
			break;
		}
	  }
	  break;

	  case REF_SELF:
	  {
		switch ( type_expres )
		{
		  case EXP_NORMAL:
			sprintf( expres_I, "%s a me stess$x ", table_channel[channel].verb_I );
			if ( channel == CHANNEL_WHISPER || channel == CHANNEL_MURMUR )
				sprintf( expres_it,  "%s a se stess$x qualcosa ", table_channel[channel].verb_it  );
			else
				sprintf( expres_it,  "%s a se stess$x ", table_channel[channel].verb_it  );
			break;

		  case EXP_EXCLAMATION:
			sprintf( expres_I,	"Esclamo a me stess$x " );
			sprintf( expres_it,	"esclama a se stess$x " );
			break;

		  case EXP_QUESTION:
			sprintf( expres_I,	"Domando a me stess$x " );
			sprintf( expres_it,	"domanda a se stess$x " );
			break;
		}
	  }
	  break;

	  case REF_GROUP:
	  {
		CHAR_DATA	*gch;
		char		 group_I[64];
		char		 group_it[64];
		char		 group_ref[64];
		int			 ngroup;

		ngroup = count_group_room( ch );
		for ( gch = ch->in_room->first_person;  gch;  gch = gch->next_in_room )
		{
			if ( !is_same_group(gch, ch) )	continue;
			if ( gch == ch )				continue;

			if ( ngroup == 1 )
			{
				sprintf( group_I,  "a %s", gch->name );
				sprintf( group_it, "a %s", gch->name );
			}
			else if ( ngroup >= 2 && ngroup <= 5 )
			{
				sprintf( group_I,	"al %sgruppo", ch->leader ? "" : "mio " );
				sprintf( group_it,	"ad un gruppo" );
				sprintf( group_ref, "al %sgruppo", gch->leader ? "" : "mio " );
			}
			else
			{
				sprintf( group_I,	"al %sgruppo", ch->leader ? "" : "mio " );
				sprintf( group_it,	"ad un folto gruppo" );
				sprintf( group_ref, "al %sgruppo", gch->leader ? "" : "mio " );
			}
		}

		switch ( type_expres )
		{
		  case EXP_NORMAL:
			sprintf( expres_I, "%s %s ", table_channel[channel].verb_I, group_I );
			if ( channel == CHANNEL_WHISPER || channel == CHANNEL_MURMUR )
				sprintf( expres_it,  "%s %s qualcosa ", table_channel[channel].verb_it, group_it  );
			else
				sprintf( expres_it,  "%s %s ", table_channel[channel].verb_it, group_it  );

			if ( ngroup == 1 )
				sprintf( expres_ref, "ti %s ",  table_channel[channel].verb_it	);
			else
				sprintf( expres_ref, "%s %s ",  table_channel[channel].verb_it, group_ref );
			break;

		  case EXP_EXCLAMATION:
			sprintf( expres_I,	"Esclamo %s ", group_I );
			sprintf( expres_it,	"esclama %s ", group_it );
			if ( ngroup == 1 )
				sprintf( expres_ref, "ti esclama " );
			else
				sprintf( expres_ref, "esclama %s", group_ref );
			break;

		  case EXP_QUESTION:
			sprintf( expres_I,	"Domando %s ", group_I );
			sprintf( expres_it,	"domanda %s ", group_it );
			if ( ngroup == 1 )
				sprintf( expres_ref, "ti domanda " );
			else
				sprintf( expres_ref, "domanda %s", group_ref );
			break;
		}
	  }	/* Chiude il case di REF_GROUP */
	  break;
	}

	/* Attacca l'espressione faccina-social se ci vuole */
	if ( found_smile )
	{
		/* Messaggio visualizzato dal ch che ha inviato lo stesso */
		strcat( expres_I, expres_smile );
		/* Messaggio visualizzato dai ch nella stanza e/o area */
		strcat( expres_it, expres_smile );
		/* Messaggio visualizzato dal ch a cui è stato inviato */
		if ( type_reference == REF_VICTIM )
			strcat( expres_ref, expres_smile );
	}

	/* Attacca il tipo di voce se ci vuole */
	if ( found_voice )
	{
		/* Messaggio visualizzato dal ch che ha inviato lo stesso */
		strcat( expres_I, expres_voice );
		/* Messaggio visualizzato dai ch nella stanza e/o area */
		strcat( expres_it, expres_voice );
		/* Messaggio visualizzato dal ch a cui è stato inviato */
		if ( type_reference == REF_VICTIM )
			strcat( expres_ref, expres_voice );
	}

	/* Taglio la parte argument finale con il cutting se ci vuole */
	if ( cutting != -1 && (found_smile || found_voice) )
	{
		/* Se trova uno spazio precedentemente scala il cutting verso sinistra nella stringa */
		while ( argument[cutting-1] == ' ' )
			cutting--;

		if ( type_expres == EXP_NORMAL )
		{
			argument[cutting] = '\0';
		}
		else if ( type_expres == EXP_EXCLAMATION )
		{
			argument[cutting]   = '!';
			argument[cutting+1] = '\0';
		}
		else if ( type_expres == EXP_QUESTION )
		{
			argument[cutting]   = '?';
			argument[cutting+1] = '\0';
		}
	}

	/* Crea la stringa per indicare con che lingua ch parla */
	if ( ch->speaking == LANGUAGE_COMMON || ch->speaking == -1 )
		lang[0] = '\0';
	else
		sprintf( lang, "(in %s) ", table_language[ch->speaking]->name );

	/* Setta il colore per i vari canali */
	if		( channel == CHANNEL_MURMUR	 )	color = AT_MURMUR;
	else if ( channel == CHANNEL_WHISPER )	color = AT_WHISPER;
	else if ( channel == CHANNEL_SAY	 )	color = AT_SAY;
	else if ( channel == CHANNEL_YELL	 )	color = AT_YELL;
	else if ( channel == CHANNEL_SHOUT	 )	color = AT_SHOUT;
	else if ( channel == CHANNEL_SING	 )	color = AT_SING;
	else
	{
		send_log( NULL, LOG_BUG, "rpg_channel: canale errato: %d", channel );
		color = AT_SAY;
	}

	/* Mischia il testo se si è ubriachi.
	 * E' fuori dal ciclo for per avere un testo pronunciato uguale per tutti.
	 * Però non bisogna utilizzare per i check sui speechprog sbuf ma argument
	 */
	dbuf = drunk_speech( argument, ch );

#ifdef T2_ARENA
	/* Il casino degli spalti di un'arena.. */
	if ( in_terrace(ch) )
	{
		send_to_char( ch, "Come faccio a sussurrare con questa confusione!\r\n" );
		/* (TT) se si troverà qui bisognerà liberare memflags */
		return;
	}
#endif

	/* Il pg che parla la lingua la impara anche un poco
	 * Non c'è il supporto per quelli che ascoltano però, per evitare orecchie da ciuccio
	 * Solo per il canale say, sempre per evitare aumento sconsiderato della lingua
	 */
	if ( channel == CHANNEL_SAY )
		learn_language( ch, ch->speaking );

	/*
	 * Invia il messaggio
	 */
	for ( d = first_descriptor;  d;  d = d->next )
	{
    	CHAR_DATA	*och;
    	int			 speakswell;

		och = d->original  ?  d->original  :  d->character;
		vch = d->character;

		if ( d->connected != CON_PLAYING )				continue;
		if ( vch == ch )								continue;
		if ( !vch )										continue;
		if ( !vch->in_room )							continue;
		if ( !ch->in_room )								continue;
		if ( vch->in_room->area != ch->in_room->area )	continue;	/* visto che il massimo che un canale può fare è sentirsi nell'area */

		/* Mischia in base a chi dei due conosce meno la lingua parlata */
		speakswell = UMIN( knows_language(vch, ch->speaking,  ch),
						   knows_language( ch, ch->speaking, vch) );

		if ( speakswell < 90 && !IS_ADMIN(vch) && !IS_ADMIN(och) )
			sbuf = translate( speakswell, dbuf, ch->speaking );
		else
			sbuf = dbuf;

		/* Disabilita i mobprog per i canali */
		MOBtrigger = FALSE;

		/* Gestione output canali murmur, whisper, say */
		if ( (channel == CHANNEL_SAY || channel == CHANNEL_SING
		   || channel == CHANNEL_WHISPER || channel == CHANNEL_MURMUR)
		  &&  vch->in_room == ch->in_room )
		{
			if ( (type_reference == REF_VICTIM && !str_cmp(rch->name, vch->name))
			  || (type_reference == REF_GROUP && is_same_group(vch, ch)) )
			{
				sprintf( expression, "$n %s%s&w'$t'", expres_ref, lang );
				act( color, expression, ch, sbuf, vch, TO_VICT );
			}
			else
			{
				if ( channel == CHANNEL_SAY || channel == CHANNEL_SING )
				{
					sprintf( expression, "$n %s%s&w'$t'", expres_it, lang );
					act( color, expression, ch, sbuf, vch, TO_VICT );
				}
				else
				{
					sprintf( expression, "$n %s.&w", expres_it );
					act( color, expression, ch, NULL, vch, TO_VICT );

					if ( IS_ADMIN(vch) )
					{
						send_to_char( vch, "[ADM] Da admin riesco a sentire murmur e whisper diretti:" );
						sprintf( expression, "[ADM] $n %s%s&w'$t'", expres_it, lang );
						act( color, expression, ch, sbuf, vch, TO_VICT );
					}
				}
			}
		}
		/* Gestione canali urla e grida */
		else if ( channel == CHANNEL_YELL || channel == CHANNEL_SHOUT )
		{
			/* Urlando consuma energia */
			if ( channel == CHANNEL_YELL )
				ch->points[POINTS_MOVE] = UMAX( 0, ch->points[POINTS_MOVE] - ch->level/4 );
			else
				ch->points[POINTS_MOVE] = UMAX( 0, ch->points[POINTS_MOVE] - ch->level/2 );

			if ( type_reference == REF_VICTIM && !str_cmp(rch->name, vch->name)
			  && vch->in_room == ch->in_room )	/* forse è un controllo inutile, ma se nell'area ci sono pg con nome simile? */
			{
				sprintf( expression, "$n %s%s&w'$t'", expres_ref, lang );
				act( color, expression, ch, sbuf, vch, TO_VICT );
			}
			else if ( vch->position == POSITION_SLEEP && vch->in_room != ch->in_room )
			{
				if ( vch->in_room == ch->in_room )
					sprintf( expression, "Qualcuno disturba il mio sonno e %s&w'$t'", table_channel[channel].verb_it );
				else
					sprintf( expression, "Il mio sonno viene disturbato da fastidiose %s.\r\n",
						code_name(NULL, channel, CODE_CHANNEL) );
				act( color, expression, ch, sbuf, vch, TO_VICT );
			}
			else
			{
				if ( vch->in_room == ch->in_room )
					sprintf( expression, "$n %s%s&w'$t'", expres_it, lang );
				else
					sprintf( expression, "$n %s %s&w'$t'", table_channel[channel].verb_it, lang );
				act( color, expression, ch, sbuf, vch, TO_VICT );
			}

			send_log( NULL, LOG_YELLSHOUT, "%s: %s", ch->name, argument );
		}
	} /* chiude il for */

	if ( IS_MOB(ch) )
		SET_BITS( ch->act, memflags );
	else
		SET_BITS( ch->pg->flags, memflags );
	destroybv( memflags );

	sprintf( expression, "%s%s&w'$T'", expres_I, lang );
	act( color, expression, ch, NULL, dbuf, TO_CHAR );

	if ( HAS_BIT(ch->in_room->flags, ROOM_LOGSPEECH) )
	{
		send_log( NULL, LOG_SPEECH, "%s (%s) %s",
			IS_MOB(ch)  ?  ch->short_descr  :  ch->name, table_channel[channel].verb_it, argument );
	}

	if ( channel == CHANNEL_WHISPER || channel == CHANNEL_SAY )
	{
		mprog_speech_trigger( argument, ch );
		if ( char_died(ch) )
			return;

		oprog_speech_trigger( argument, ch );
		if ( char_died(ch) )
			return;

		rprog_speech_trigger( argument, ch );
	}
}


/*
 * Smistamento dei vari canali alla rpg_channel.
 */
DO_RET do_murmur( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_MURMUR );
}

DO_RET do_whisper( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_WHISPER );
}

DO_RET do_say( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_SAY );
}

DO_RET do_yell( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_YELL );
}

DO_RET do_shout( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_SHOUT );
}

DO_RET do_sing( CHAR_DATA *ch, char *argument )
{
	rpg_channel( ch, argument, CHANNEL_SING );
}

/*
 * Smistamento dei vari canali, che si riferiscono a qualcuno, alla rpg_channel.
 */
 DO_RET do_murmur_ref( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, "a %s", argument );
	rpg_channel( ch, buf, CHANNEL_MURMUR );
}

DO_RET do_whisper_ref( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, "a %s", argument );
	rpg_channel( ch, buf, CHANNEL_WHISPER );
}

DO_RET do_say_ref( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, "a %s", argument );
	rpg_channel( ch, buf, CHANNEL_SAY );
}

DO_RET do_yell_ref( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, "a %s", argument );
	rpg_channel( ch, buf, CHANNEL_YELL );
}

DO_RET do_shout_ref( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, "a %s", argument );
	rpg_channel( ch, buf, CHANNEL_SHOUT );
}


/*
 * Smistamento dei vari canali, che si riferiscono al gruppo, alla rpg_channel.
 */
DO_RET do_murmur_group( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, ": %s", argument );
	rpg_channel( ch, buf, CHANNEL_MURMUR );
}

DO_RET do_whisper_group( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, ": %s", argument );
	rpg_channel( ch, buf, CHANNEL_WHISPER );
}

DO_RET do_say_group( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, ": %s", argument );
	rpg_channel( ch, buf, CHANNEL_SAY );
}

DO_RET do_yell_group( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, ": %s", argument );
	rpg_channel( ch, buf, CHANNEL_YELL );
}

DO_RET do_shout_group( CHAR_DATA *ch, char *argument )
{
	char	buf[MSL];

	sprintf( buf, ": %s", argument );
	rpg_channel( ch, buf, CHANNEL_SHOUT );
}


/*
 * Ritorna vero se ch sta ignorando ign_ch
 */
bool is_ignoring( CHAR_DATA *ch, CHAR_DATA *ign_ch )
{
	IGNORE_DATA *temp;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "is_ignoring: ch è NULL" );
		return FALSE;
	}

	if ( !ign_ch )
	{
		send_log( NULL, LOG_BUG, "is_ignoring: ign_ch è NULL" );
		return FALSE;
	}

	if ( IS_MOB(ch) || IS_MOB(ign_ch) )
		return FALSE;

	for ( temp = ch->pg->first_ignored;  temp;  temp = temp->next )
	{
		if ( nifty_is_name(temp->name, ign_ch->name) )
			return TRUE;
	}

	return FALSE;
}


/*
 * Il comando ignore permette di ignorare fino ad un massimo di
 *	MAX_IGNORE giocatori riguardo ai tell e ai reply.
 * Syntax:
 *	ignore		-	lists players currently ignored
 *	ignore none	-	sets it so no players are ignored
 *	ignore <player>	-	start ignoring player if not already
 *				ignored otherwise stop ignoring player
 */
void do_ignore( CHAR_DATA *ch, char *argument )
{
	IGNORE_DATA	*temp;
	IGNORE_DATA	*next;
	IGNORE_DATA *ignore;
	CHAR_DATA	*victim;
	char		 arg[MIL];
	int			 count;

	if ( IS_MOB(ch) )
		return;

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Stai ignorando:\r\n" );
		send_to_char( ch, "&g----------------------------------------&w\r\n" );

		if ( !ch->pg->first_ignored )
		{
			send_to_char( ch, "Nessuno\r\n" );
			return;
		}

		for ( temp = ch->pg->first_ignored;  temp;  temp = temp->next )
			ch_printf( ch,"- %s\r\n", temp->name );

		return;
	}

	argument = one_argument( argument, arg );

	if ( is_name(arg, "nessuno none") )
	{
		for ( temp = ch->pg->first_ignored;  temp;  temp = next )
		{
			next = temp->next;

			UNLINK( temp, ch->pg->first_ignored, ch->pg->last_ignored, next, prev );
			DISPOSE( temp->name );
			DISPOSE( temp );
		}

		send_to_char( ch, "Hai rimosso tutti i nomi ignorati.\r\n" );
		return;
	}

	/* Conta quanti sono stati ignorati */
	/* Rimuove dalla lista l'ignorato se l'argomento corrisponde al suo nome */
	for( temp = ch->pg->first_ignored, count = 0;  temp;  temp = temp->next, count++ )
	{
		if ( !str_cmp(temp->name, arg) )
		{
			UNLINK( temp, ch->pg->first_ignored, ch->pg->last_ignored, next, prev );
			ch_printf( ch,"Non ignori più %s.\r\n", temp->name );
			DISPOSE( temp->name );
			DISPOSE( temp );
			return;
		}
	}

	/* Cerca se il giocatore esiste */
	victim = get_char_mud( ch, arg, FALSE );
	if ( !victim )
		victim = get_offline( arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch,"Non esiste giocatore con quel nome.\r\n" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "Ignorarsi da soli? Non è il massimo.\r\n" );
		return;
	}

	/* If its valid and the list size limit has not been */
	/* reached create a node and at it to the list	     */
	if ( count > MAX_IGNORE )
	{
		ch_printf( ch,"Puoi ignorare al massimo %d giocatori.\r\n", MAX_IGNORE );
		return;
	}

	CREATE( ignore, IGNORE_DATA, 1 );
	ignore->name = str_dup( victim->name );
	ignore->next = NULL;	/* (TT) create dovrebbe già piazzarli a NULL, da provare */
	ignore->prev = NULL;
	LINK( ignore, ch->pg->first_ignored, ch->pg->last_ignored, next, prev );

	ch_printf( ch,"Da ora ignori %s.\r\n", ignore->name );
}


/*
 * Canale Tell.
 * I mob non si può tellarli. (FF) rifarlo con dei get_original
 */
DO_RET do_tell( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	char		 toch[MSL];
	char		 arg[MIL];
	char		 afk[MIL];
	int			 position;

	afk[0] = '\0';

	/* Se si era sordi al Comunica iniziando a comunicare si perde tale status */
	if ( IS_PG(ch) )
		REMOVE_BIT( ch->pg->deaf, CHANNEL_TELL );

	/* Se manca il messaggio o il destinatario allora esce */
	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_to_char( ch, "Comunicare cosa a chi?\r\n" );
		return;
	}

	/* Se il player è silenziato */
	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_NOTELL) )
	{
		send_to_char( ch, "Non puoi comunicare.\r\n" );
		return;
	}

	victim = get_player_mud( ch, arg, FALSE );
	if ( !victim )
	{
		ch_printf( ch, "Non si trova su %s.\r\n", MUD_NAME );
		return;
	}

	/* Fa finta di non trovare nessuno se ha la vittima è un admin con incognito */
	if ( !IS_ADMIN(ch) && victim->pg->incognito >= get_level(ch) )
	{
		ch_printf( ch, "Non si trova su %s.\r\n", MUD_NAME );
		return;
	}

	if ( ch == victim )
	{
		send_to_char( ch, "Comunicare a te stesso??\r\n" );
		return;
	}

	if ( IS_PG(victim) )
	{
		CHAR_DATA *switched_victim = NULL;

		if ( victim->switched && IS_ADMIN(ch) && !HAS_BIT(victim->switched->affected_by, AFFECT_POSSESS) )
		{
			send_to_char( ch, "Questo personaggio è switched.\r\n" );
			return;
		}

		if ( victim->switched && HAS_BIT(victim->switched->affected_by, AFFECT_POSSESS) )
		{
			switched_victim = victim->switched;
		}
		else if ( !victim->desc )
		{
			send_to_char( ch, "Questo personaggio è in link-dead.\r\n" );
			return;
		}

		/* Prepara il messaggio di afk se ce n'è bisogno */
		if ( HAS_BIT_PLR(victim, PLAYER_AFK) || HAS_BIT_PLR(victim, PLAYER_AFK2) )
		{
			if ( VALID_STR(victim->pg->afk_message) )
				sprintf( afk, "(in %s: %s) ", translate_command(ch, "afk"), victim->pg->afk_message);
			else
				sprintf( afk, "(in %s) ", translate_command(ch, "afk"));
		}

		/* Se la vittima è sorda ai tell il messaggio non giunge */
		if ( HAS_BIT(victim->pg->deaf, CHANNEL_TELL) && !IS_ADMIN(ch) )
		{
			ch_printf( ch, "Ha il canale '%s' chiuso.", translate_command(ch, "tell") );
			return;
		}

		if ( (is_editing(ch->desc) || is_in_olc(ch->desc)) && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Questo personaggio è impegnato a scrivere. Riprova tra un po'.\r\n" );
			return;
		}

		/* Controlla se la vittima sta ignorando il mittente del messaggio */
		if ( is_ignoring(victim, ch) )
		{
			/* Se il mittente è un admin non può essere ignorato */
			if ( IS_ADMIN(ch) || get_trust(ch) >= get_trust(victim) )
				ch_printf( victim, "Cerchi di ignorare %s, ma ti è impossibile.\r\n", PERS(ch, victim) );
			else
			{
				ch_printf( ch, "%s ti ignora.\r\n", PERS(ch, victim) );
				return;
			}
		}

		if ( switched_victim )
			victim = switched_victim;
	}

	set_char_color( AT_TELL, ch );

	sprintf( toch, "[OFFGDR] Comunichi a %s %s'%s'\r\n", victim->name, afk, argument );
	write_to_buffer_private( ch->desc, toch, 0 );	/* evita gli snoop */

	position		 = victim->position;
	victim->position = POSITION_STAND;

	set_char_color( AT_TELL, victim );
    MOBtrigger = FALSE;		/* Disattiva temporaneamente i Trigger MobProg */

#ifdef T2_MXP
	sprintf( toch, "[OFFGDR] " MXPTAG("plr %s") "%s" MXPTAG ("/plr") " ti comunica '%s'\r\n",
		ch->name,
		ch->name,
		argument );
#else
	sprintf( toch, "[OFFGDR] %s ti comunica '%s'\r\n", ch->name, argument );
#endif
	write_to_buffer_private( victim->desc, toch, 0 );	/* evita gli snoop */

#ifdef T2_MSP
	if ( IS_PG(victim) && HAS_BIT(victim->pg->protocols, PROTOCOL_BEEP) )
		send_audio( victim->desc, "beep.wav", TO_CHAR );
#endif

	victim->position = position;
	victim->reply	 = ch;
}


/*
 * Comando Reply.
 * Con i mob si può fare reply.
 */
DO_RET do_reply( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   toch[MSL];
	char	   afk[MIL];
	int		   position;

	afk[0] = '\0';

	/* Se si era sordi al Comunica iniziando a comunicare si perde tale status */
	if ( IS_PG(ch) )
		REMOVE_BIT( ch->pg->deaf, CHANNEL_TELL );

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Rispondere che cosa?\r\n" );
		return;
	}

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_SILENCE) )
	{
		send_to_char( ch, "Non riesci a inviare il tuo messaggio.\r\n" );
		return;
	}

	if ( (victim = ch->reply) == NULL )
	{
		ch_printf( ch, "Non si trova su %s.\r\n", MUD_NAME );
		return;
	}

	if ( IS_PG(victim) )
	{
		if ( victim->switched && IS_ADMIN(ch) )
		{
			send_to_char( ch, "Questo personaggio è switched.\r\n" );
			return;
		}
		else if ( !victim->desc )
		{
			send_to_char( ch, "Questo personaggio è in link-dead.\r\n" );
			return;
		}

		/* Prepara il messaggio di afk se ce n'è bisogno */
		if ( (HAS_BIT_PLR(victim, PLAYER_AFK) || HAS_BIT_PLR(victim, PLAYER_AFK2)) )
		{
			if ( VALID_STR(victim->pg->afk_message) )
				sprintf( afk, "(in %s: %s) ", translate_command(ch, "afk"), victim->pg->afk_message);
			else
				sprintf( afk, "(in %s) ", translate_command(ch, "afk"));
		}

		if ( HAS_BIT(victim->pg->deaf, CHANNEL_TELL)
		  && (!IS_ADMIN(ch) || (get_trust(ch) < get_trust(victim))) )
		{
			ch_printf( ch, "Ha il canale '%s' chiuso.", translate_command(ch, "tell") );
			return;
		}

		if ( (is_editing(ch->desc) || is_in_olc(ch->desc)) && !IS_ADMIN(ch) )
		{
			send_to_char( ch, "Questo personaggio è impegnato a scrivere. Riprova tra un po'." );
			return;
		}

		/* Controlla se la vittima sta ignorando il mittente del messaggio */
		if ( is_ignoring(victim, ch) )
		{
			/* Se il mittente è un admin non può essere ignorato */
			if ( IS_ADMIN(ch) || get_trust(ch) >= get_trust(victim) )
				ch_printf( victim, "Cerchi di ignorare %s, ma ti è impossibile.\r\n", PERS(ch, victim) );
			else
			{
				ch_printf( ch, "%s ti ignora.\r\n", PERS(ch, victim) );
				return;
			}
		}
	}

	set_char_color( AT_TELL, ch );
	sprintf( toch, "[OFFGDR] Comunichi a %s %s'%s'\r\n", victim->name, afk, argument );
	write_to_buffer_private( ch->desc, toch, 0 );	/* evita gli snoop */

	position		 = victim->position;
	victim->position = POSITION_STAND;

	set_char_color( AT_TELL, victim );
	MOBtrigger = FALSE;  /* Disabilita i mobprog per i canali */

#ifdef T2_MXP
	sprintf( toch, "[OFFGDR] " MXPTAG("plr %s") "%s" MXPTAG("/plr") " ti comunica '%s'\r\n",
		ch->name,
		ch->name,
		argument );
#else
	sprintf( toch, "[OFFGDR] %s ti comunica '%s'\r\n", ch->name, argument );
#endif
	write_to_buffer_private( victim->desc, toch, 0 );

#ifdef T2_MSP
	if ( IS_PG(victim) && HAS_BIT(victim->pg->protocols, PROTOCOL_BEEP) )
		send_audio( victim->desc, "beep.wav", TO_CHAR );
#endif

	victim->position = position;
	victim->reply	 = ch;
}


/*
 * Comando di comunicazione gruppo off-gdr.
 */
DO_RET do_gtell( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA  *gch;
	char	    toch[MSL];

	if ( count_group(ch) <= 0 )
	{
		ch_printf( ch, "Sei tu da sol%c nel gruppo.\r\n", gramm_ao(ch) );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Comunicare al tuo gruppo che cosa?\r\n" );
		return;
	}

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_NOTELL) )
	{
		send_to_char( ch, "Il tuo messaggio non può essere inviato.\r\n" );
		return;
	}

	/* Se si era sordi al gcomunica iniziando a comunicare con il gruppo si perde tale status */
	if ( IS_PG(ch) )
		REMOVE_BIT( ch->pg->deaf, CHANNEL_GTELL );

	set_char_color( AT_GTELL, ch );
	sprintf( toch, "[OFFGDR] Comunichi al gruppo '%s'.\r\n", argument );
	write_to_buffer_private( ch->desc, toch, 0 );	/* evita gli snoop */


	sprintf( toch, "[OFFGDR] %s comunica al gruppo '%s'.\r\n", ch->name, argument );
    /* Note use of ch_printf, so gtell works on sleepers. */
	for ( gch = first_char;  gch;  gch = gch->next )
	{
		/* Si può gtellare ai mob del proprio gruppo, nel caso si switch */

		if ( gch == ch )											continue;
		if ( IS_PG(gch) && HAS_BIT(gch->pg->deaf, CHANNEL_GTELL) )	continue;
		if ( is_same_group(gch, ch) )
		{
			set_char_color( AT_GTELL, gch );
			write_to_buffer_private( gch->desc, toch, 0 );
		}
	}
}


/*
 * Comando Emote.
 */
DO_RET do_emote( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*vch;
	BITVECTOR 	*memflags = NULL;	/* Variabile in cui memorizza le flag del mob o del pg */
	int			 length;
	char		 buf[MSL];

	if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_NOEMOTE) )
	{
		send_to_char( ch, "Una Forza interiore mi impedisce di esprimere le emozioni.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_to_char( ch, "Cosa dovrei esprimere?\r\n" );
		return;
	}

	/* Copia le flag di act in una variabile temporanea che riassegna poi al pg */
	if ( IS_MOB(ch) )
	{
		SET_BITS( memflags, ch->act );
		REMOVE_BIT( ch->act, MOB_SECRETIVE );
	}
	else
		SET_BITS( memflags, ch->pg->flags );

	strcpy( buf, argument );
	/* Ci aggiunge un punto se alla fine non c'è */
	length = strlen( argument );
	if ( isalpha(argument[length-1]) )
		strcat( buf, "." );

	for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
	{
		if ( vch == ch )
			act( AT_ACTION, "$n $t", ch, buf, vch, TO_CHAR );
		else
		{
			MOBtrigger = FALSE;
			act( AT_ACTION, "$n $t", ch, buf, vch, TO_VICT );
		}
	} /* chiude il for */

	if ( IS_MOB(ch) )
		SET_BITS( ch->act, memflags );
	else
		SET_BITS( ch->pg->flags, memflags );
	destroybv( memflags );

	if ( HAS_BIT(ch->in_room->flags, ROOM_LOGSPEECH) )
		send_log( NULL, LOG_SPEECH, "%s (esprime) %s ", IS_MOB(ch)  ?  ch->short_descr  :  ch->name, argument );
}


/*
 * Comando Channels: visualizza i canali attivi e attivabili
 */
DO_RET do_channels( CHAR_DATA *ch, char *argument )
{
	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Non per i mob.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		if ( HAS_BIT_PLR(ch, PLAYER_SILENCE) )
		{
			send_to_char( ch, "&gSei stato zittito.&w\r\n" );
			return;
		}

		send_to_char( ch, "&gCanali privati e off-gdr:&w\r\n" );
		if ( HAS_BIT(ch->pg->deaf, CHANNEL_TELL ) )
			ch_printf( ch, "[&r-&w] &Wcomunica     &wPossiedi il canale %s disattivato\r\n", translate_command(ch, "tell") );
		else
			ch_printf( ch, "[&G+&w] &WCOMUNICA     &wPossiedi il canale %s attivato\r\n", translate_command(ch, "tell") );

		if ( HAS_BIT(ch->pg->deaf, CHANNEL_GTELL) )
			ch_printf( ch, "[&r-&w] &Wgcomunica    &wPossiedi il canale %s disattivato\r\n", translate_command(ch, "gtell") );
		else
			ch_printf( ch, "[&G+&w] &WGCOMUNICA    &wPossiedi il canale %s attivato\r\n", translate_command(ch, "gtell") );

		if ( IS_ADMIN(ch) )
		{
			send_to_char( ch, "\r\n&gCanali degli amministratori&w:\r\n" );

			if ( HAS_BIT(ch->pg->deaf, CHANNEL_THINK) )
				ch_printf( ch, "[&r-&w] &wpensa     &wPossiedi il canale %s disattivato\r\n", translate_command(ch, "think") );
			else
				ch_printf( ch, "[&G+&w] &WPENSA     &wPossiedi il canale %s attivato\r\n", translate_command(ch, "think") );

			if ( HAS_BIT(ch->pg->deaf, CHANNEL_QTALK) )
				ch_printf( ch, "[&r-&w] &wqparla    &wPossiedi il canale %s disattivato\r\n", translate_command(ch, "qtalk") );
			else
				ch_printf( ch, "[&G+&w] &WQPARLA    &wPossiedi il canale %s attivato\r\n", translate_command(ch, "qtalk") );

			if ( HAS_BIT(ch->pg->deaf, CHANNEL_ADMIN) )
				ch_printf( ch, "[&r-&w] &wadmtalk   &wPossiedi il canale %s disattivato\r\n", translate_command(ch, "admtalk") );
			else
				ch_printf( ch, "[&G+&w] &WADMTALK   &wPossiedi il canale %s attivato\r\n", translate_command(ch, "admtalk") );

		}
		send_to_char( ch, "\r\n" );
	}
	else
	{
		char	arg[MIL];
		int		bit;
		bool	fClear;

		bit = 0;

		one_argument( argument, arg );
		if		( arg[0] == '+' )	fClear = TRUE;
		else if ( arg[0] == '-' )	fClear = FALSE;
		else
		{
			ch_printf( ch, "%s -canale o +canale?\r\n", translate_command(ch, "channels") );
			return;
		}

		/* Se c'è lo spazio tra +/- e l'opzione allora ricontrolla il valore arg */
		if ( strlen(arg) == 1 )
			one_argument( argument, arg );
		else
			strcpy( arg, arg+1 );

		if		( !str_cmp(arg, "comunica"	) || !str_cmp(arg, "tell"	) )		bit = CHANNEL_TELL;
		else if ( !str_cmp(arg, "gcomunica"	) || !str_cmp(arg, "gtell"	) )		bit = CHANNEL_GTELL;
		else if ( !str_cmp(arg, "admtalk"	) )									bit = CHANNEL_ADMIN;
		else if ( !str_cmp(arg, "qparla"	) || !str_cmp(arg, "qtalk"	) )		bit = CHANNEL_QTALK;
		else if ( !str_cmp(arg, "pensa"		) || !str_cmp(arg, "think"	) )		bit = CHANNEL_THINK;
		else
		{
			send_to_char( ch, "+/- a quale canale?\r\n" );
			return;
		}

		if ( fClear )
		{
			REMOVE_BIT( ch->pg->deaf, bit );
			send_to_char( ch, "Canale attivato.\r\n" );
		}
		else
		{
			SET_BIT( ch->pg->deaf, bit );
			send_to_char( ch, "Canale disattivato.\r\n" );
		}
	} /* chiude l'else */
}


/*
 * Gestione comandi ldt (afk) e ldt2 (afk2)
 */
DO_RET afk_handler( CHAR_DATA *ch, char *argument, bool afk2 )
{
	if ( IS_MOB(ch) )
		return;

	DISPOSE( ch->pg->afk_message );
	if ( VALID_STR(argument) )
	{
		if ( strlen(argument) > 45 )
		{
			send_to_char( ch, "Messaggio di afk troppo lungo, supera i 45 caratteri.\r\n" );
			return;
		}
		ch->pg->afk_message = str_dup( argument );
	}

	if ( HAS_BIT_PLR(ch, PLAYER_AFK) || HAS_BIT_PLR(ch, PLAYER_AFK2) )
	{
		send_to_char( ch, "Ritorni alla tastiera.\r\n" );
		if ( !afk2 )
		{
			act( AT_PLAIN, "$n esce dalla trance e riprende possesso della sua coscienza.",
				ch, NULL, NULL, TO_ROOM );	/* (GG) AT_AFK */
		}
		REMOVE_BIT( ch->pg->flags, PLAYER_AFK );
		REMOVE_BIT( ch->pg->flags, PLAYER_AFK2 );
	}
	else
	{
		if ( VALID_STR(ch->pg->afk_message) )
			ch_printf( ch, "Ti allontani dalla tastiera. (%s)\r\n", ch->pg->afk_message );
		else
			send_to_char( ch, "Ti allontani dalla tastiera.\r\n" );
		if ( !afk2 )
		{
			act( AT_PLAIN, "Sbuffi d'energia astrale vorticano attorno all'inattivo $n mandandol$x in trance.",
				ch, NULL, NULL, TO_ROOM );	/* (GG) AT_AFK */
			SET_BIT( ch->pg->flags, PLAYER_AFK );
		}
		else
		{
			SET_BIT( ch->pg->flags, PLAYER_AFK2 );
		}
	}
}

DO_RET do_afk( CHAR_DATA *ch, char *argument )
{
	afk_handler( ch, argument, FALSE );
}

/* Per admin, non fa perdere lo status di afk anche digitando comandi */
DO_RET do_afk2( CHAR_DATA *ch, char *argument )
{
	afk_handler( ch, argument, TRUE );
}


/*
 * Gestisce liste di messaggi bug, idea, typo visualizzandole o cancellando i file.
 * Aggiunge una messaggio ad un file.
 * Inserisce il numero di stanza, il mese, il giorno e il nome del pg mittente.
 */
void manage_append_msg( CHAR_DATA *ch, char *file, char *argument )
{
	MUD_FILE *fp;
	char	 *msg_type;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "manage_append_msg: ch è NULL" );
		return;
	}

	if ( !VALID_STR(file) )
	{
		send_log( NULL, LOG_BUG, "manage_append_msg: file passato non è valido" );
		return;
	}

	set_char_color( AT_PLAIN, ch );

	if		( !strcmp(file, PBUG_FILE)	 )	msg_type = "bug";
	else if ( !strcmp(file, IDEA_FILE)	 )	msg_type = "idea";
	else if ( !strcmp(file, COMMENT_FILE))	msg_type = "commento";
	else if ( !strcmp(file, TYPO_FILE)	 )	msg_type = "typo";
	else if ( !strcmp(file, FIXED_FILE)	 )	msg_type = "correzione";
	else
	{
		send_log( NULL, LOG_BUG, "manage_append_msg: file non in elenco: %s",
			(file)  ?  file  :  "?" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		ch_printf( ch, "Cosa vorresti segnalare agli amministratori come %s?\r\n", msg_type );
		send_to_char( ch, "La segnalazione verrà salvata su file che controlliamo periodicamente.\r\n" );

		ch_printf( ch, "\r\nUtilizzo:  '%s <messaggio>'\r\n", msg_type );
		if ( IS_ADMIN(ch) )
			ch_printf( ch, "Utilizzo:  '%s lista' per visualizzare l'attuale lista.\r\n", msg_type );
		if ( get_trust(ch) == TRUST_IMPLE )
			ch_printf( ch, "Utilizzo:  '%s cancella' per cancellare l'attuale lista.\r\n", msg_type );

		return;
	}

#ifdef T2_WEB
	ch->pg->report++;
#endif

	/* Se si è imple allora si può cancellare il file */
	if ( get_trust(ch) == TRUST_IMPLE && (!str_cmp(argument, "cancella") || !str_cmp(argument, "reset")) )
	{
		fp = mud_fopen( "", file, "w", TRUE );
		if ( fp )
			MUD_FCLOSE( fp );

		send_to_char( ch, "File ripulito.\r\n" );
		return;
	}

	/* Se si è dello staff allora si può visualizzare il file */
	if ( IS_ADMIN(ch) && (!str_cmp(argument, "lista") || !str_cmp(argument, "list")) )
	{
		send_to_char( ch, "\r\n[VNum - Data] Nome: Messaggio\r\n" );
		send_to_char( ch, "---------------------------------------------\r\n" );
		show_file( ch->desc, file, FALSE );
		return;
	}

	/*
	 * Altrimenti aggiunge al file la stringa passata
	 */
	fp = mud_fopen( "", file, "a", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "[#%5.5d - %s] %s: %s\n",
		(ch->in_room)  ?  ch->in_room->vnum  :  -1,
		friendly_ctime(&current_time),
		ch->name,
		argument );

	MUD_FCLOSE( fp );

	send_log( NULL, LOG_REPORT, "manage_append_msg: %s segnalato da %s: %s",
		msg_type, ch->name, argument );

	send_to_char( ch, "Grazie! Controlleremo la tua segnalazione.\r\n\r\n" );
}

/*
 * Comando per inviare segnalazioni di Bug
 */
DO_RET do_bug( CHAR_DATA *ch, char *argument )
{
	manage_append_msg( ch, PBUG_FILE, argument );
}


/*
 * Comando per inviare segnalazioni di idee
 */
DO_RET do_idea( CHAR_DATA *ch, char *argument )
{
	manage_append_msg( ch, IDEA_FILE, argument );
}

/*
 * Comando per inviare segnalazioni di typo
 */
DO_RET do_typo( CHAR_DATA *ch, char *argument )
{
	manage_append_msg( ch, TYPO_FILE, argument );
}

/*
 * Comando per inviare commenti riguardo al mud
 */
DO_RET do_comment( CHAR_DATA *ch, char *argument )
{
	manage_append_msg( ch, COMMENT_FILE, argument );
}

/*
 * Comando admin per inviare un messaggio con le piccole correzioni fatte
 */
DO_RET do_fixed( CHAR_DATA *ch, char *argument )
{
	manage_append_msg( ch, FIXED_FILE, argument );
}


/*
 * Gestisce l'uscita dal gioco
 */
void quit_handler( CHAR_DATA *ch, bool rent )
{
	int	x;
	int	y;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "quit_handler: ch passato è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( ch->position == POSITION_FIGHT
	  || ch->position == POSITION_EVASIVE
	  || ch->position == POSITION_DEFENSIVE
	  || ch->position == POSITION_AGGRESSIVE
	  || ch->position == POSITION_BERSERK )
	{
		set_char_color( AT_RED, ch );
		send_to_char( ch, "Non posso! Sto combattendo.\r\n" );
		return;
	}

	if ( ch->position < POSITION_STUN  )
	{
		set_char_color( AT_DRED, ch );
		send_to_char( ch, "Per il momento non sono ancora cadavere.\r\n" );
		return;
	}

	if ( get_timer(ch, TIMER_RECENTFIGHT) > 0 && !IS_ADMIN(ch) )
	{
		set_char_color( AT_RED, ch );
		send_to_char( ch, "La mia adrenalina è veramente troppa per andarteme da qui ora!\r\n" );
		return;
	}

	set_char_color( AT_GREY, ch );

	/* Conteggio alla rovescia a meno che non stia quittando perché in idle */
	if ( !rent )
	{
		if ( ch->pg->counter_quit == 0 && ch->desc )
		{
			ch->pg->counter_quit = 6;
			send_to_char( ch, "Sento la mia anima disgiungersi dall'esperienza sensibile.." );
			return;
		}

		/* Gli amministratori non visualizzano il conteggio alla rovescia */
		if ( !IS_ADMIN(ch) )
		{
			ch->pg->counter_quit--;
			if ( ch->pg->counter_quit > 0 )
			{
				for ( x = 0;  x < ch->pg->counter_quit;  x++ )
					send_to_char( ch, "." );
				return;
			}
		} else {
			if (ch->switched)
			{
				send_command(ch->switched, "return", CO);
				set_char_color(AT_PLAIN, ch);
			}
		}

		if ( VALID_STR(ch->pg->msg_logout_you) )
			ch_printf( ch, "%s\r\n\r\n", ch->pg->msg_logout_you );
		else
			send_to_char( ch, "Inizio a rilassarmi e subito comincio a vagare su altri Piani.\r\n\r\n" );

		if ( VALID_STR(ch->pg->msg_logout_room) )
			act( AT_ACTION, ch->pg->msg_logout_room, ch, NULL, NULL, TO_ROOM );
		else
			act( AT_ACTION, "$n si rilassa ed inizia a vagare in altri Piani.", ch, NULL, NULL, TO_ROOM );
	}
	else
	{
		if ( ch->in_room && HAS_BIT(ch->in_room->flags, ROOM_RENT) )
		{
			if ( ch->gold < 1000 )
			{
				send_to_char( ch, "Non ho abbastanza soldi con me per prendere in affitto una camera.\r\n" );
				return;
			}
			ch->gold -= 1000;
			send_to_char( ch, "Mi dirigo verso la mia camera per riposare.\r\n" );
			act( AT_ACTION, "$n si dirige verso la sua camera a riposare.", ch, NULL, NULL, TO_ROOM );
		}
		else
		{
			send_to_char( ch, "Non mi trovo in una locanda in cui possa affittare una camera.\r\n" );
			return;
		}
	}

	/* Smonta il pg dalla cavalcatura fino a che non ha finito di salvare */
	if ( ch->position == POSITION_MOUNT )
		send_command( ch, "dismount", CO );

#ifdef T2_MSP
	send_audio( ch->desc, "quit.wav", TO_CHAR );
#endif
	send_quote( ch );

	send_log( NULL, LOG_COMM, "%s(%s) è uscit%c dal gioco dalla stanza %s (#%d)",
		ch->name,
		get_host(ch),
		gramm_ao(ch),
		(ch->in_room) ? ch->in_room->name : "NULL",
		(ch->in_room)  ?  ch->in_room->vnum  :  0 );
	quitting_char = ch;

	save_player( ch );
	refresh_offline( ch );

	if ( sysdata.mascotte_save && ch->pg->mascotte )
	{
		act( AT_BYE, "$N segue $n.", ch, NULL, ch->pg->mascotte, TO_ROOM );
		extract_char( ch->pg->mascotte, TRUE );
	}

	/* Salva i dati del clan quando uno dei membri quitta. */
	if ( ch->pg->clan )
		fwrite_clan( ch->pg->clan );

	saving_char = NULL;
	extract_char( ch, TRUE );

	for ( x = 0;  x < MAX_WEARLOC;  x++ )
	{
		for ( y = 0;  y < MAX_LAYERS;  y++ )
			save_equipment[x][y] = NULL;
	}

#ifdef T2_WEB
	/* Una volta quittato refresha il who */
	create_html_webwho( );
#endif
}

/*
 * Gestisce il comando Rent
 */
DO_RET do_rent( CHAR_DATA *ch, char *argument )
{
	quit_handler( ch, TRUE );
}

/*
 * Comando di uscita dal gioco.
 */
DO_RET do_quit( CHAR_DATA *ch, char *argument )
{
	quit_handler( ch, FALSE );
}


/*
 * Contatore di do_quit.
 * Al giocatore viene inviato mano a mano a video un conto alla rovescia,
 *	il conto smette se il giocatore invia un qualunque input.
 */
void update_counter_quit( void )
{
	DESCRIPTOR_DATA	*d;
	DESCRIPTOR_DATA	*dnext;

	for ( d = first_descriptor;  d;  d = dnext )
	{
		CHAR_DATA	*och;

		dnext = d->next;

		och = (d->original)  ?  d->original  :  d->character;
		if ( och && IS_PG(och) && och->pg->counter_quit > 0 )
			do_quit( och, "" );
	}
}


/*
 * Tabella dei log
 */
const struct log_type table_log[] =
{
/*		id				code			   	name			trust 				file   lastcmd	default	descr */
	{ { LOG_NORMAL,		"LOG_NORMAL",		"normal"	},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log normale"														},
	{ { LOG_ALWAYS,		"LOG_ALWAYS",		"always"	},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Logga sempre"														},
	{ { LOG_NEVER,		"LOG_NEVER",		"never"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Logga mai"															},
	{ { LOG_BUILD,		"LOG_BUILD",		"build"		},	TRUST_NEOBUILDER,	TRUE,	TRUE,	TRUE,	"Log di building"													},
	{ { LOG_ADMIN,		"LOG_ADMIN",		"admin"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log per amministratori"											},
	{ { LOG_COMM,		"LOG_COMM",			"comm"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log di comunicazione"												},
	{ { LOG_WARNING,	"LOG_WARNING",		"warning"	},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log di warning"													},
	{ { LOG_MONI,		"LOG_MONI",			"moni"		},	TRUST_NEOMASTER,	FALSE,	TRUE,	FALSE,	"Log di monitoraggio azioni pg"										},
	{ { LOG_ALL,		"LOG_ALL",			"all"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Logga tutto"														},
	{ { LOG_MPROG,		"LOG_MPROG",		"mprog"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log sui mudprog"													},
	{ { LOG_TRIGGER,	"LOG_TRIGGER",		"trigger"	},	TRUST_NEOMASTER,	TRUE,	TRUE,	FALSE,	"Log riguardante i trigger dei mudprog"								},
	{ { LOG_SUPERMOB,	"LOG_SUPERMOB",		"supermob"	},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log riguardante il supermob"										},
	{ { LOG_LOAD,		"LOG_LOAD",			"load"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	FALSE,	"Log sulle procedure di avvio"										},
	{ { LOG_OLC,		"LOG_OLC",			"olc"		},	TRUST_NEOBUILDER,	TRUE,	TRUE,	FALSE,	"Log sulle procedure di OCL"										},
	{ { LOG_DAMAGE,		"LOG_DAGAME",		"damage"	},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log sulla quantità di danno in combattimento"						},
	{ { LOG_DEATH,		"LOG_DEATH",		"death"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log sulle morti dei pg (DT e non) e sulla perdita di stat"			},
	{ { LOG_PK,			"LOG_PK",			"pk"		},	TRUST_NEOMASTER,	TRUE,	FALSE,	TRUE,	"Log sulle uccisioni tra pg"										},
	{ { LOG_BUG,		"LOG_BUG",			"bug"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log di bug"														},
	{ { LOG_PUNY,		"LOG_PUNY",			"puny"		},	TRUST_MASTER,		TRUE,	FALSE,	TRUE,	"Log sulle punizioni"												},
	{ { LOG_SPEECH,		"LOG_SPEECH",		"speech"	},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log riguardante le parlate gdr registrate in alcune stanze"		},
	{ { LOG_FP,			"LOG_FP",			"fp"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log riguardante l'apertura e la gestione dei file"					},
	{ { LOG_FREAD,		"LOG_FREAD",		"fread"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log sugli errori di lettura delle funzioni fread"					},
	{ { LOG_FWRITE,		"LOG_FWRITE",		"fwrite"	},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log sugli errori di scrittura delle funzioni fwrite"				},
	{ { LOG_BOT,		"LOG_BOT",			"bot"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	FALSE,	"Log sui bot"														},
	{ { LOG_MEMORY,		"LOG_MEMORY",		"memory"	},	TRUST_NEOBUILDER,	TRUE,	TRUE,	TRUE,	"Log riguardante i bachi sulla memoria e i memory leak"				},
	{ { LOG_BIT,		"LOG_BIT",			"bit"		},	TRUST_NEOBUILDER,	FALSE,	TRUE,	TRUE,	"Log riguardante i bachi dei bitvector"								},
	{ { LOG_REPORT,		"LOG_REPORT",		"report"	},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log sulle segnalazioni fatte dai pg: bug, idee, typo e commenti"	},
	{ { LOG_CHECK,		"LOG_CHECK",		"check"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log sui check di caricamento file area o altro"					},
	{ { LOG_RNDATTR,	"LOG_RNDATTR",		"rndattr"	},	TRUST_BUILDER,		FALSE,	FALSE,	FALSE,	"Log di debug riguardo l'assegnazione random degli attributi"		},
	{ { LOG_SPACEFIGHT,	"LOG_SPACEFIGHT",	"spacefight"},	TRUST_BUILDER,		FALSE,	FALSE,	FALSE,	"Log do debug sulla space_to_fight() function"						},
	{ { LOG_RESET,		"LOG_RESET",		"reset"		},	TRUST_NEOBUILDER,	TRUE,	FALSE,	TRUE,	"Log riguardanti i reset"											},
	{ { LOG_CODE,		"LOG_CODE",			"code"		},	TRUST_NEOMASTER,	TRUE,	TRUE,	TRUE,	"Log riguardante i codici di tipologie e/o flag"					},
	{ { LOG_APPLY,		"LOG_APPLY",		"apply"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	FALSE,	"Log di debug riguardante gli apply"								},
	{ { LOG_CMDMOB,		"LOG_CMDMOB",		"cmdmob"	},	TRUST_NEOMASTER,	TRUE,	FALSE,	FALSE,	"Log riguardante i comandi non riusciti dei mob"					},
	{ { LOG_CMDPG,		"LOG_CMDPG",		"cmdpg"		},	TRUST_NEOMASTER,	TRUE,	FALSE,	TRUE,	"Log riguardante i comandi non riusciti dei pg"						},
	{ { LOG_ORDER,		"LOG_ORDER",		"order"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log riguardante gli ordini impartiti dai pg"						},
	{ { LOG_LAG,		"LOG_LAG",			"lag"		},	TRUST_NEOMASTER,	TRUE,	FALSE,	TRUE,	"Log riguardanti i lag dei comandi"									},
	{ { LOG_TITLE,		"LOG_TITLE",		"title"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log riguardante i title modificati dai pg"							},
	{ { LOG_WHO,		"LOG_WHO",			"who"		},	TRUST_NEOMASTER,	FALSE,	FALSE,	FALSE,	"Log per tracciare statisticamente quanto i pg usano il who"		},
	{ { LOG_SAVE,		"LOG_SAVE",			"save"		},	TRUST_NEOMASTER,	TRUE,	FALSE,	TRUE,	"Log riguardanti i save dei pg"										},
	{ { LOG_SHOVEDRAG,	"LOG_SHOVEDRAG",	"shovedrag"	},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log riguardante il shove e il drag"								},
	{ { LOG_LEVEL,		"LOG_LEVEL",		"level"		},	TRUST_MASTER,		TRUE,	FALSE,	TRUE,	"Log riguardante l'avanzamento dei livelli dei pg"					},
	{ { LOG_YELLSHOUT,	"LOG_YELLSHOUT",	"yellshout"	},	TRUST_NEOMASTER,	FALSE,	FALSE,	TRUE,	"Log riguardante gli yell e shout inviati dai giocatori"			},
	{ { LOG_ECHO,		"LOG_ECHO",			"echo"		},	TRUST_NEOMASTER,	TRUE,	FALSE,	TRUE,	"Log riguardante gli echo inviati dagli amministratori"				},
	{ { LOG_LEARN,		"LOG_LEARN",		"learn"		},	TRUST_MASTER,		FALSE,	FALSE,	TRUE,	"Log riguardante l'aumento o l'abbassamento di skill"				},
	{ { LOG_EXP,		"LOG_EXP",			"exp"		},	TRUST_MASTER,		FALSE,	FALSE,	FALSE,	"Log riguardante il gain di px"										},
	{ { LOG_SHARE,		"LOG_SHARE",		"share"		},	TRUST_MASTER,		FALSE,	FALSE,	TRUE,	"Log riguardante le fluttuazioni delle azioni"						},
	{ { LOG_LYRIC,		"LOG_LYRIC",		"lyric"		},	TRUST_MASTER,		FALSE,	FALSE,	TRUE,	"Log riguardante l'attivazione di mob che cantano liriche"			},
#ifdef T2_MCCP
	{ { LOG_MCCP,		"LOG_MCCP",			"mccp"		},	TRUST_MASTER,		TRUE,	FALSE,	TRUE,	"Log riguardante il protocollo di compressione MCCP"				},
#endif
	{ { LOG_AUTOQUEST,	"LOG_AUTOQUEST",	"autoquest"	},	TRUST_MASTER,		TRUE,	FALSE,	TRUE,	"Log riguardante le quest automatiche"								},
	{ { LOG_SNOOP,		"LOG_SNOOP",		"snoop"		},	TRUST_IMPLE,		FALSE,	FALSE,	FALSE,	"Log riguardante gli snoop"	        								}
};
const int max_check_log = sizeof(table_log)/sizeof(struct log_type);


DO_RET do_msglog( CHAR_DATA *ch, char *argument )
{
	int	x;

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "Non per i mob.\r\n" );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		ch_printf	( ch, "&YSintassi&w: %s default\r\n", translate_command(ch, "msglog") );
		send_to_char( ch, "&gMessaggi di Log attivi:&w\r\n" );
		for ( x = 0;  x < MAX_LOG;  x++ )
		{
			char code[MIL];

			if ( get_trust(ch) < table_log[x].trust )
				continue;

			strcpy( code, code_name(NULL, x, CODE_LOG) );
			if ( HAS_BIT(ch->pg->msglog, x) )
				ch_printf( ch, "[&G+&w] &W%-10s &w%s\r\n", strupper(code), table_log[x].descr );
			else
				ch_printf( ch, "[&R-&w] &W%-10s &w%s\r\n", code, table_log[x].descr );
		}
		return;
	}
	else
	{
		char	arg[MIL];
		int		bit;
		bool	fClear;

		bit = -1;

		argument = one_argument( argument, arg );
		if		( arg[0] == '+' )	fClear = TRUE;
		else if ( arg[0] == '-' )	fClear = FALSE;
		else if ( !str_prefix(arg, "default") )
		{
			/* Imposta i msglog di default */
			for ( x = 0;  x < MAX_LOG;  x++ )
			{
				if (  table_log[x].trust > get_trust(ch) )	continue;
				if ( !table_log[x].def )					continue;

				SET_BIT( ch->pg->msglog, x );
			}
			send_to_char( ch, "Impostati i msglog di default per la tua trust.\r\n" );
			return;
		}
		else
		{
			ch_printf( ch, "%s -log o +log?\r\n", translate_command(ch, "msglog") );
			return;
		}

		/* Se c'è lo spazio tra +/- e l'opzione allora ricontrolla il valore arg */
		if ( strlen(arg) == 1 )
			one_argument( argument, arg );
		else
			strcpy( arg, arg+1 );

		for ( x = 0;  x < MAX_LOG;  x++ )
		{
			if ( get_trust(ch) < table_log[x].trust )
				continue;

			if ( !str_prefix(arg, code_name(NULL, x, CODE_LOG)) )
				bit = x;
		}

		if ( bit == -1 )
		{
			send_to_char( ch, "+/- a quale tipologia di log?\r\n" );
			return;
		}

		if ( fClear )
		{
			SET_BIT( ch->pg->msglog, bit );
			send_to_char( ch, "Tipologia di Log attivata.\r\n" );
		}
		else
		{
			REMOVE_BIT( ch->pg->msglog, bit );
			send_to_char( ch, "Tipologia di Log disattivata.\r\n" );
		}
	}
}


/*
 * Writes a string to the log, extended version
 */
void send_log( MUD_FILE *fp, const int log_type, const char *fmt, ... )
{
	DESCRIPTOR_DATA *d;
	char			*strtime;
	char			 buf[MSL*2];
	va_list			 args;

	if ( log_type > 0 && log_type < MAX_LOG )
		sprintf( buf, "[%-10s]", table_log[log_type].code.name );	/* Qui meglio usare la table_log[] al posto della code_name(), per evitare ricorsività di send_log() */
	else
		strcpy( buf, "[   NONE   ]" );

	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	/* Cerca la riga ove è stato trovato l'errore se c'è un file passato */
	if ( fp && fp->file )
	{
		char	buf_file[MSL];
		int		x;
		int		iLine;
		int		iChar;

		if ( fp->file == stdin )
		{
			iLine = 0;
		}
		else
		{
			char c;

			iLine = 1;
			iChar = ftell( fp->file );		/* Acquisisce la posizione */
			fseek( fp->file, iChar-1, 0);
			c = fgetc(fp->file);
			if (c == '\n')
				iLine--;
			else
			{
				if (c == '\r')
				{
					fseek( fp->file, iChar-2, 0);
					c = fgetc(fp->file);
					if (c == '\n')
						iLine--;
				}
			}

			fseek( fp->file, 0, 0 );		/* Posiziona lo stream all'inizio */
			/* Fino a che non arriva nella posizone precedentemente acquisita calcola le righe */
			for ( x = 0;  x < iChar;  x++ )
			{
				c = fgetc( fp->file );

				if ( c == EOF )
					break;

				if ( c == '\n' )
					iLine++;
			}

			fseek( fp->file, iChar, 0 );	/* Riposiziona lo stream ove era originariamente */
		}
		sprintf( buf_file, " [FILE: %s - LINE: %d]",
			(fp->path)  ?  fp->path  :  "(sconosciuto)", iLine );
		strcat( buf, buf_file );
	}

	/*
	 * Stampa l'errore nei file rispettivi di log
	 */
	strtime = friendly_ctime( &current_time );
	if ( table_log[log_type].file == TRUE )
		fprintf( stderr, "%s: %s\n", strtime, buf );

	/*
	 * Invia il messaggio agli imple
	 */
	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA *och;		/* indica il pg originale  */
		CHAR_DATA *vch;		/* indica il pg che riceverà il msg */

		och = d->original  ?   d->original  :  d->character;
		vch = d->character;

		if ( !och )											continue;
		if ( !vch )											continue;
		if ( !IS_ADMIN(och) )								continue;
		if ( !och->desc )									continue;
		if ( och->desc->connected == CON_GET_PASSWORD )		continue;	/* Altrimenti uno che si logga con un nome di admin senza inserire la password legge i messaggi di log attivi per quell'admin */
		if ( och->desc->connected == CON_COPYOVER_RECOVER )	continue;	/* Altrimenti crasha nel copyover se viene inviato un log durante il ripristino */
		if ( get_trust(och) < table_log[log_type].trust )	continue;

		if ( HAS_BIT(och->pg->msglog, log_type) )
		{
			set_char_color( AT_LOG, vch );
			if ( table_log[log_type].lastcmd && HAS_BIT_PLR(vch, PLAYER_LASTCMD) )
				ch_printf_nocolor( vch, "LastCmd: %s\r\n", lastcmd_typed );
			ch_printf_nocolor( vch, "%s: %s\r\n", capitalize(table_log[log_type].code.code), buf );	/* Ache qui meglio la table_log della funzione code_str() per evitare ricorsività della send_log() */
			set_char_color( AT_PLAIN, vch );
		}
	}
}
