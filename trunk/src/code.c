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

/*********************************************************************************
								SISTEMA DEI CODICI
 *********************************************************************************/

#include "mud.h"
#include "affect.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "fread.h"
#include "help.h"
#include "herb.h"
#include "movement.h"
#include "mprog.h"
#include "nanny.h"
#include "news.h"
#include "note.h"
#include "room.h"
#include "skills.h"


const struct table_code_type table_code[] =
{
	{ { CODE_AFFECT,		"CODE_AFFECT",		"affect"		},	code_affect,		MAX_AFFECT		},
	{ { CODE_APPLY,			"CODE_APPLY",		"apply"			},	code_apply,			MAX_APPLY		},
	{ { CODE_AREAFLAG,		"CODE_AREAFLAG",	"areaflag"		},	code_areaflag,		MAX_AREAFLAG	},
	{ { CODE_ATTACK,		"CODE_ATTACK",		"attack"		},	code_attack,		MAX_ATTACK		},
	{ { CODE_ATTR,			"CODE_ATTR",		"attr"			},	code_attr,			MAX_ATTR		},
	{ { CODE_CHANNEL,		"CODE_CHANNEL",		"channel"		},	NULL,				MAX_CHANNEL		},
	{ { CODE_CLANFLAG,		"CODE_CLANFLAG",	"clanflag"		},	code_clanflag,		MAX_CLANFLAG	},
	{ { CODE_CLANTYPE,		"CODE_CLANTYPE",	"clantype"		},	code_clantype,		MAX_CLANTYPE	},
	{ { CODE_CLASS,			"CODE_CLASS",		"class"			},	code_class,			MAX_CLASS		},
	{ { CODE_CLASSFLAG,		"CODE_CLASSFLAG",	"classflag"		},	code_classflag,		MAX_CLASSFLAG	},
	{ { CODE_CMDFLAG,		"CODE_CMDFLAG",		"cmdflag"		},	code_cmdflag,		MAX_CMDFLAG		},
	{ { CODE_CMDTYPE,		"CODE_CMDTYPE",		"cmdtype"		},	code_cmdtype,		MAX_CMDTYPE		},
	{ { CODE_CODE,			"CODE_CODE",		"code"			},	NULL,				MAX_CODE		},
	{ { CODE_COLOR,			"CODE_COLOR",		"color"			},	NULL,				MAX_COLOR+1		},
	{ { CODE_COLORHAIR,		"CODE_COLORHAIR",	"colorhair"		},	NULL,				MAX_COLOR+1		},
	{ { CODE_CONDITION,		"CODE_CONDITION",	"condition"		},	code_condition,		MAX_CONDITION	},
	{ { CODE_CONNECTED,		"CODE_CONNECTED",	"connected"		},	code_connected,		MAX_CONNECTED	},
	{ { CODE_CONTAINER,		"CODE_CONTAINER",	"container"		},	code_container,		MAX_CONTAINER	},
	{ { CODE_DAMAGE,		"CODE_DAMAGE",		"damage"		},	NULL,				MAX_DAMAGE		},
	{ { CODE_DEFENSE,		"CODE_DEFENSE",		"defense"		},	code_defense,		MAX_DEFENSE		},
	{ { CODE_DIR,			"CODE_DIR",			"dir"			},	NULL,				MAX_DIR			},
	{ { CODE_ENFLAG,		"CODE_ENFLAG",		"enflag"		},	code_enflag,		MAX_ENFLAG		},
	{ { CODE_EYE,			"CODE_EYE",			"eye"			},	code_eye,			MAX_EYE			},
	{ { CODE_EXIT,			"CODE_EXIT",		"exit"			},	code_exit,			MAX_EXIT		},
	{ { CODE_HAIRTYPE,		"CODE_HAIRTYPE",	"hairtype"		},	code_hairtype,		MAX_HAIRTYPE	},
	{ { CODE_HAIRSTYLE,		"CODE_HAIRSTYLE",	"hairstyle"		},	code_hairstyle,		MAX_HAIRSTYLE	},
	{ { CODE_HELPTYPE,		"CODE_HELPTYPE",	"helptype"		},	code_helptype,		MAX_HELPTYPE	},
	{ { CODE_INTENT,		"CODE_INTENT",		"intent"		},	code_intent,		MAX_INTENT		},
	{ { CODE_LANGUAGE,		"CODE_LANGUAGE",	"language"		},	code_language,		MAX_LANGUAGE	},
	{ { CODE_LIGHT,			"CODE_LIGHT",		"light"			},	code_light,			MAX_LIGHT		},
	{ { CODE_LOG,			"CODE_LOG",			"log"			},	NULL,				MAX_LOG			},
	{ { CODE_MOB,			"CODE_MOB",			"mob"			},	code_mob,			MAX_MOB			},
	{ { CODE_MPTRIGGER,		"CODE_MPTRIGGER",	"mptrigger"		},	code_mptrigger,		MAX_MPTRIGGER	},
	{ { CODE_NEWS,			"CODE_NEWS",		"news"			},	code_news,			MAX_NEWS		},
	{ { CODE_NOTE,			"CODE_NOTE",		"note"			},	code_note,			MAX_NOTE		},
	{ { CODE_OBJFLAG,		"CODE_OBJFLAG",		"objflag"		},	code_objflag,		MAX_OBJFLAG		},
	{ { CODE_OBJMAGIC,		"CODE_OBJMAGIC",	"objmagic"		},	code_objmagic,		MAX_OBJMAGIC	},
	{ { CODE_OBJTYPE,		"CODE_OBJTYPE",		"objtype"		},	code_objtype,		MAX_OBJTYPE		},
	{ { CODE_OBJWEAR,		"CODE_OBJWEAR",		"objwear"		},	code_objwear,		MAX_OBJWEAR		},
	{ { CODE_PART,			"CODE_PART",		"part"			},	NULL,				MAX_PART		},
	{ { CODE_PIPE,			"CODE_PIPE",		"pipe"			},	code_pipe,			MAX_PIPE		},
	{ { CODE_PLAYER,		"CODE_PLAYER",		"player"		},	NULL,				MAX_PLAYER		},
	{ { CODE_POINTS,		"CODE_POINTS",		"points"		},	NULL,				MAX_POINTS		},
	{ { CODE_POSITION,		"CODE_POSITION",	"position"		},	code_position,		MAX_POSITION	},
	{ { CODE_PROTOCOL,		"CODE_PROTOCOL",	"protocol"		},	NULL,				MAX_PROTOCOL	},
	{ { CODE_PULL,			"CODE_PULL",		"pull"			},	code_pull,			MAX_PULL		},
	{ { CODE_RACE,			"CODE_RACE",		"race"			},	code_race,			MAX_RACE		},
	{ { CODE_RACEFLAG,		"CODE_RACEFLAG",	"raceflag"		},	code_raceflag,		MAX_RACEFLAG	},
	{ { CODE_RIS,			"CODE_RIS",			"ris"			},	code_ris,			MAX_RIS			},
	{ { CODE_ROOM,			"CODE_ROOM",		"room"			},	code_room,			MAX_ROOM		},
	{ { CODE_SAVETHROW,		"CODE_SAVETHROW",	"savethrow"		},	code_savethrow,		MAX_SAVETHROW	},
	{ { CODE_SECTOR,		"CODE_SECTOR",		"sector"		},	NULL,				MAX_SECTOR		},
	{ { CODE_SENSE,			"CODE_SENSE",		"sense"			},	code_sense,			MAX_SENSE		},
	{ { CODE_SEX,			"CODE_SEX",			"sex"			},	code_sex,			MAX_SEX			},
	{ { CODE_SKELL,			"CODE_SKELL",		"skell"			},	code_skell,			MAX_SKELL		},
	{ { CODE_SKILLTYPE,		"CODE_SKILLTYPE",	"skilltype"		},	code_skilltype,		MAX_SKILLTYPE	},
	{ { CODE_SPELLACTION,	"CODE_SPELLACTION",	"spellaction"	},	code_spellaction,	MAX_SPELLACTION	},
	{ { CODE_SPELLDAMAGE,	"CODE_SPELLDAMAGE",	"spelldamage"	},	code_spelldamage,	MAX_SPELLDAMAGE	},
	{ { CODE_SPELLPOWER,	"CODE_SPELLPOWER",	"spellpower"	},	code_spellpower,	MAX_SPELLPOWER	},
	{ { CODE_SPELLSAVE,		"CODE_SPELLSAVE",	"spellsave"		},	code_spellsave,		MAX_SPELLSAVE	},
	{ { CODE_SPELLSPHERE,	"CODE_SPELLSPHERE",	"spellsphere"	},	code_spellsphere,	MAX_SPELLSPHERE	},
	{ { CODE_STYLE,			"CODE_STYLE",		"style"			},	code_style,			MAX_STYLE		},
	{ { CODE_TARGET,		"CODE_TARGET",		"target"		},	code_target,		MAX_TARGET		},
	{ { CODE_TRAPFLAG,		"CODE_TRAPFLAG",	"trapflag"		},	code_trapflag,		MAX_TRAPFLAG	},
	{ { CODE_TRIG,			"CODE_TRIG",		"trig"			},	code_trig,			MAX_TRIG		},
	{ { CODE_TRUST,			"CODE_TRUST",		"trust"			},	code_trust,			MAX_TRUST		},
	{ { CODE_VOTE,			"CODE_VOTE",		"vote"			},	code_vote,			MAX_VOTE		},
	{ { CODE_WEAPON,		"CODE_WEAPON",		"weapon"		},	code_weapon,		MAX_WEAPON		},
	{ { CODE_WEARFLAG,		"CODE_WEARFLAG",	"wearflag"		},	code_wearflag,		27				},
	{ { CODE_WEARLOC,		"CODE_WEARLOC",		"wearloc"		},	code_wearloc,		MAX_WEARLOC		}
};
const int max_check_code = sizeof(table_code)/sizeof(struct table_code_type);


/*
 * Ritorna il valore MAX_ di una tabella di codici
 */
int code_max( const int type )
{
	if ( type < 0 || type >= MAX_CODE )
	{
		send_log( NULL, LOG_CODE, "code_max: valore type passato errato: %d", type );
		return -1;
	}

	return table_code[type].max;
}


/*
 * Ritorna il numero di codice tramite la stringa passata e il tipo di tabella di codice
 * MUD_FILE fp   = se si sta passando un file allora si sta caricando un file o area di file,
 *              quindi vengono confrontati i code della struttura struct code_data.
 * char *word	= puntatore alla parola che ha letto da un file oppure che è stata passata.
 * int type		= tipo di tabella di codici da controllare.
 */
int code_get_number_handler( MUD_FILE *fp, const char *word, const int type, bool read_code )
{
	int	   x;
	int	   max;

	if ( !VALID_STR(word) )
	{
		send_log( fp, LOG_CODE, "code_get_number_handler: word passata non valida per il tipo %d", type );
		return -1;
	}

	if ( type < 0 || type >= MAX_CODE )
	{
		send_log( NULL, LOG_CODE, "code_get_number_handler: valore type passato errato: %d (con word = %s)",
			type, word );
		return -1;
	}

	max = code_max( type );

	for ( x = 0;  x < max;  x++ )
	{
		if ( table_code[type].table )
		{
			if ( read_code )
			{
				if ( !str_cmp(word, table_code[type].table[x].code) )
					return x;
			}
			else
			{
				if ( !str_cmp(word, table_code[type].table[x].name) )
					return x;
			}
		}
		else
		{
			switch ( type )
			{
			  default:
				send_log( fp, LOG_CODE, "code_get_number_handler: tipologia di codice mancante %s per tabella codici NULL", table_code[x].code.name );
				break;
			  case CODE_CHANNEL:	if ( !str_cmp(word, (read_code) ? table_channel[x].code.code	: table_channel[x].code.name ) )	return x;	break;
			  case CODE_CODE:		if ( !str_cmp(word, (read_code) ? table_code[x].code.code		: table_code[x].code.name	 ) )	return x;	break;
			  case CODE_COLOR:		if ( !str_cmp(word, (read_code) ? table_color[x].code.code		: table_color[x].code.name	 ) )	return x;	break;
			  case CODE_COLORHAIR:	if ( !str_cmp(word, (read_code) ? table_color[x].hair.code		: table_color[x].hair.name	 ) )	return x;	break;
			  case CODE_DAMAGE:		if ( !str_cmp(word, (read_code) ? table_damage[x].code.code		: table_damage[x].code.name	 ) )	return x;	break;
			  case CODE_DIR:		if ( !str_cmp(word, (read_code) ? table_dir[x].code.code		: table_dir[x].code.name	 ) )	return x;	break;
			  case CODE_LOG:		if ( !str_cmp(word, (read_code) ? table_log[x].code.code		: table_log[x].code.name	 ) )	return x;	break;
			  case CODE_PART:		if ( !str_cmp(word, (read_code) ? table_part[x].code.code		: table_part[x].code.name	 ) )	return x;	break;
			  case CODE_PLAYER:		if ( !str_cmp(word, (read_code) ? table_player[x].code.code		: table_player[x].code.name	 ) )	return x;	break;
			  case CODE_POINTS:		if ( !str_cmp(word, (read_code) ? table_points[x].code.code		: table_points[x].code.name	 ) )	return x;	break;
			  case CODE_PROTOCOL:	if ( !str_cmp(word, (read_code) ? table_protocol[x].code.code	: table_protocol[x].code.name) )	return x;	break;
			  case CODE_SECTOR:		if ( !str_cmp(word, (read_code) ? table_sector[x].code.code		: table_sector[x].code.name	 ) )	return x;	break;
			}
		}
	}

	/* Avverte di corrispondenze non trovate solo per la lettura dei code, ovvero nel caricamento aree o dati */
	if ( read_code )
	{
		send_log( fp, LOG_CODE, "code_get_number_handler: corrispondenza della parola %s non trovata per la tabella di codici %s",
			word, table_code[type].code.name );
		return -1;
	}

	/* Se fp è NULL allora prova a fare una ricerca prefix */
	for ( x = 0;  x < max;  x++ )
	{
		if ( table_code[type].table )
		{
			if ( !str_prefix(word, table_code[type].table[x].name) )
				return x;
		}
		else
		{
			switch ( type )
			{
			  default:
				send_log( fp, LOG_CODE, "code_get_number_handler: tipologia di codice mancante %s per tabella codici NULL", table_code[type].code.name );
				break;
			  case CODE_CHANNEL:	if ( !str_prefix(word, (read_code) ? table_channel[x].code.code		: table_channel[x].code.name ) )	return x;	break;
			  case CODE_CODE:		if ( !str_prefix(word, (read_code) ? table_code[x].code.code		: table_code[x].code.name	 ) )	return x;	break;
			  case CODE_COLOR:		if ( !str_prefix(word, (read_code) ? table_color[x].code.code		: table_color[x].code.name	 ) )	return x;	break;
			  case CODE_COLORHAIR:	if ( !str_prefix(word, (read_code) ? table_color[x].hair.code		: table_color[x].hair.name	 ) )	return x;	break;
			  case CODE_DAMAGE:		if ( !str_prefix(word, (read_code) ? table_damage[x].code.code		: table_damage[x].code.name	 ) )	return x;	break;
			  case CODE_DIR:		if ( !str_prefix(word, (read_code) ? table_dir[x].code.code			: table_dir[x].code.name	 ) )	return x;	break;
			  case CODE_LOG:		if ( !str_prefix(word, (read_code) ? table_log[x].code.code			: table_log[x].code.name	 ) )	return x;	break;
			  case CODE_PART:		if ( !str_prefix(word, (read_code) ? table_part[x].code.code		: table_part[x].code.name	 ) )	return x;	break;
			  case CODE_PLAYER:		if ( !str_prefix(word, (read_code) ? table_player[x].code.code		: table_player[x].code.name	 ) )	return x;	break;
			  case CODE_POINTS:		if ( !str_prefix(word, (read_code) ? table_points[x].code.code		: table_points[x].code.name	 ) )	return x;	break;
			  case CODE_PROTOCOL:	if ( !str_prefix(word, (read_code) ? table_protocol[x].code.code	: table_protocol[x].code.name) )	return x;	break;
			  case CODE_SECTOR:		if ( !str_prefix(word, (read_code) ? table_sector[x].code.code		: table_sector[x].code.name	 ) )	return x;	break;
			}
		}
	}

	return -1;
}

/* Ricava il numero di codice dalla stringa di codice (.code) */
int code_code( MUD_FILE *fp, const char *word, const int type )
{
	return code_get_number_handler( fp, word, type, TRUE );
}

/* Ricava il numero di codice dalla stringa di nome del codice (.name) */
int code_num( MUD_FILE *fp, const char *word, const int type )
{
	return code_get_number_handler( fp, word, type, FALSE );
}


/*
 * La funzione esegue un piccolo controllo sull'indice da passare alla tabella di codice,
 *	è preferibile utilizzare quindi questa e non richiamare la stringa tramite l'array stesso.
 *	(tipo: invece di scrivere code_pinco[x].name; utilizzare code_str(NULL, x, CODE_PINCO); )
 * MUD_FILE fp  = se si sta passando un file allora si sta caricando un file o area di file,
 *              	quindi vengono confrontati i code della struttura struct code_data, fp NULL è lecito.
 * int number	= indica il numero passato indice dell'array della tabella codici da restituire.
 * int table	= tipo do tabella codici da controllare
 * bool name	= tipo di dato da inviare, se codice o nome
 */
char *code_print_string_handler( MUD_FILE *fp, const int number, const int type, bool send_code )
{
	int		max;

	if ( type < 0 || type >= MAX_CODE )
	{
		send_log( NULL, LOG_CODE, "code_print_string_handler: tipo di tabella passata errata: %d", type );
		return "(errore)";
	}

	max = code_max( type );

	if ( number < 0 || number >= max )
	{
		send_log( fp, LOG_CODE, "code_print_string_handler: numero errato di %s per tabella %s: %d",
			(send_code) ? "nome" : "codice", table_code[type].code.name, number );
		return "(errore)";
	}

	if ( table_code[type].table )
	{
		if ( send_code )
			return table_code[type].table[number].code;
		else
			return table_code[type].table[number].name;
	}
	else
	{
		switch ( type )
		{
		  default:
			send_log( fp, LOG_CODE, "code_print_string_handler: tipologia di codice mancante %s per tabella codici NULL", table_code[type].code.name );
			break;
		  case CODE_CHANNEL:	return (send_code) ? table_channel[number].code.code  : table_channel[number].code.name;
		  case CODE_CODE:		return (send_code) ? table_code[number].code.code	  : table_code[number].code.name;
		  case CODE_COLOR:		return (send_code) ? table_color[number].code.code	  : table_color[number].code.name;
		  case CODE_COLORHAIR:	return (send_code) ? table_color[number].hair.code	  : table_color[number].hair.name;
		  case CODE_DAMAGE:		return (send_code) ? table_damage[number].code.code	  : table_damage[number].code.name;
		  case CODE_DIR:		return (send_code) ? table_dir[number].code.code	  : table_dir[number].code.name;
		  case CODE_LOG:		return (send_code) ? table_log[number].code.code	  : table_log[number].code.name;
		  case CODE_PART:		return (send_code) ? table_part[number].code.code	  : table_part[number].code.name;
		  case CODE_PLAYER:		return (send_code) ? table_player[number].code.code	  : table_player[number].code.name;
		  case CODE_POINTS:		return (send_code) ? table_points[number].code.code	  : table_points[number].code.name;
		  case CODE_PROTOCOL:	return (send_code) ? table_protocol[number].code.code : table_protocol[number].code.name;
		  case CODE_SECTOR:		return (send_code) ? table_sector[number].code.code	  : table_sector[number].code.name;
		}
	}

	send_log( fp, LOG_CODE, "code_print_string_handler: numero di stringa di tipo %s sconosciuta %d per il tipo %s",
		(send_code) ? "nome" : "codice", number, table_code[type].code.name );

	return "(errore)";
}

/* Invia la stringa del codice (.code) */
char *code_str( MUD_FILE *fp, const int number, const int type )
{
	return code_print_string_handler( fp, number, type, TRUE );
}

/* Invia il nome del codice (.name) */
char *code_name( MUD_FILE *fp, const int number, const int type )
{
	return code_print_string_handler( fp, number, type, FALSE );
}


/*
 * Ritorna una stringa da un bitvector formata da dati struct code_data
 * (CC) dopo la conversione inglobare qui il print_bitvector
 * (FF) effettuare dei controlli sui type, visto che qui solitamente non passano
 *	alcune tabella ext bitvector
 */
char *code_bit( MUD_FILE *fp, BITVECTOR *bitvector, const int type )
{
	static char buf[MSL];
	int			max;
	int			x;

	max = code_max( type );

	buf[0] = '\0';
	for ( x = 0;  x < max;  x++ )
	{
		if ( HAS_BIT(bitvector, x) )
		{
			/* Non concatena uno spazio se l'ultimo carattere è già uno spazio */
			if ( VALID_STR(buf) && buf[strlen(buf)-1] != ' ' )
				strcat( buf, " " );

			if ( table_code[type].table )
			{
				if ( fp )
					strcat( buf, table_code[type].table[x].code );
				else
					strcat( buf, table_code[type].table[x].name );
			}
			else
			{
				switch ( type )
				{
				  default:
					send_log( fp, LOG_CODE, "code_bit: tipologia di codice mancante %s per tabella codici NULL", table_code[x].code.name );
					break;
				  case CODE_CHANNEL:	strcat( buf, (fp) ? table_channel[x].code.code	: table_channel[x].code.name );		break;
				  case CODE_CODE:		strcat( buf, (fp) ? table_code[x].code.code	    : table_code[x].code.name );		break;
				  case CODE_COLOR:		strcat( buf, (fp) ? table_color[x].code.code	: table_color[x].code.name );		break;
				  case CODE_COLORHAIR:	strcat( buf, (fp) ? table_color[x].hair.code	: table_color[x].hair.name );		break;
				  case CODE_DAMAGE:		strcat( buf, (fp) ? table_damage[x].code.code	: table_damage[x].code.name );		break;
				  case CODE_DIR:		strcat( buf, (fp) ? table_dir[x].code.code		: table_dir[x].code.name );			break;
				  case CODE_LOG:		strcat( buf, (fp) ? table_log[x].code.code		: table_log[x].code.name );			break;
				  case CODE_PART:		strcat( buf, (fp) ? table_part[x].code.code	    : table_part[x].code.name );		break;
				  case CODE_PLAYER:		strcat( buf, (fp) ? table_player[x].code.code	: table_player[x].code.name );		break;
				  case CODE_POINTS:		strcat( buf, (fp) ? table_points[x].code.code	: table_points[x].code.name );		break;
				  case CODE_PROTOCOL:	strcat( buf, (fp) ? table_protocol[x].code.code : table_protocol[x].code.name );	break;
				  case CODE_SECTOR:		strcat( buf, (fp) ? table_sector[x].code.code	: table_sector[x].code.name );		break;
				}
			}
		}
	}

	/* (RR) aggiungere un check sui valori > max per vedere che siano empty */

	if ( (x = strlen(buf)) > 0 )
		buf[x] = '\0';

	return buf;
}

/*
 * Come sopra ma gestisce i bitvector interi
 */
char *code_bitint( MUD_FILE *fp, int bitvector, const int type )
{
	BITVECTOR *bits;

	if ( type < 0 || type >= MAX_CODE )
	{
		send_log( NULL, LOG_CODE, "code_bitint: tipo passato di tablella codici errata: %d", type );
		return NULL;
	}

	bits	= convert_bitvector( bitvector );
	return code_bit( fp, bits, type );
}


/*
 * Ritorna la lista di flag possibili associabili per quel bitvector o extended bitvector.
 * Utile per elencarle quando servono da digitare delle opzioni a riguaro.
 */
char *code_all( const int type, bool column )
{
	static char	buf[MSL];
	int			line;
	int			x;

	buf[0] = '\0';
	line = 1;
	for ( x = 0;  x < code_max(type);  x++ )
	{
		char	*str;

		str = code_name( NULL, x, type );

		if ( !column && strlen(buf)+strlen(str) > 79*line )
		{
			strcat( buf, "\r\n" );
			line++;
		}

		strcat( buf,  str );

		if ( column )
			strcat( buf, "\r\n" );
		else
			strcat( buf, " " );
	}

	/* Toglie lo spazio di troppo alla fine */
	buf[strlen(buf)-1] = '\0';

	return buf;
}


/*
 * Controlla che tutti gli array di code_data abbiano il massimo, calcolato con sizeof,
 *	identico a quelli definiti MAX_
 */
void check_code_table( void )
{
	int	x;

	for ( x = 0;  x < MAX_CODE;  x++ )
	{
		int	max_to_check = -1;
		int	y;

		switch ( x )
		{
		  default:
			send_log( NULL, LOG_CODE, "check_code_table: (%s) tipo di codice non inserito o inesistente: %d", table_code[x].code.name, x );
			exit( EXIT_FAILURE );
		  case CODE_AFFECT:		max_to_check = max_check_affect;		break;
		  case CODE_APPLY:		max_to_check = max_check_apply;			break;
		  case CODE_AREAFLAG:	max_to_check = max_check_areaflag;		break;
		  case CODE_ATTACK:		max_to_check = max_check_attack;		break;
		  case CODE_ATTR:		max_to_check = max_check_attr;			break;
		  case CODE_CHANNEL:	max_to_check = max_check_channel;		break;
		  case CODE_CLANFLAG:	max_to_check = max_check_clanflag;		break;
		  case CODE_CLANTYPE:	max_to_check = max_check_clantype;		break;
		  case CODE_CLASS:		max_to_check = max_check_class;			break;
		  case CODE_CLASSFLAG:	max_to_check = max_check_classflag;		break;
		  case CODE_CMDFLAG:	max_to_check = max_check_cmdflag;		break;
		  case CODE_CMDTYPE:	max_to_check = max_check_cmdtype;		break;
		  case CODE_CODE:		max_to_check = max_check_code;			break;
		  case CODE_COLOR:		max_to_check = max_check_color;			break;
		  case CODE_COLORHAIR:	max_to_check = max_check_color;			break;
		  case CODE_CONDITION:	max_to_check = max_check_condition;		break;
		  case CODE_CONNECTED:	max_to_check = max_check_connected;		break;
		  case CODE_CONTAINER:	max_to_check = max_check_container;		break;
		  case CODE_DAMAGE:		max_to_check = max_check_damage;		break;
		  case CODE_DEFENSE:	max_to_check = max_check_defense;		break;
		  case CODE_DIR:		max_to_check = max_check_dir;			break;
		  case CODE_ENFLAG:		max_to_check = max_check_enflag;		break;
		  case CODE_EYE:		max_to_check = max_check_eye;			break;
		  case CODE_EXIT:		max_to_check = max_check_exit;			break;
		  case CODE_HAIRTYPE:	max_to_check = max_check_hairtype;		break;
		  case CODE_HAIRSTYLE:	max_to_check = max_check_hairstyle;		break;
		  case CODE_HELPTYPE:	max_to_check = max_check_helptype;		break;
		  case CODE_INTENT:		max_to_check = max_check_intent;		break;
		  case CODE_LANGUAGE:	max_to_check = max_check_language;		break;
		  case CODE_LIGHT:		max_to_check = max_check_light;			break;
		  case CODE_LOG:		max_to_check = max_check_log;			break;
		  case CODE_MOB:		max_to_check = max_check_mob;			break;
		  case CODE_MPTRIGGER:	max_to_check = max_check_mptrigger;		break;
		  case CODE_NEWS:		max_to_check = max_check_news;			break;
		  case CODE_NOTE:		max_to_check = max_check_note;			break;
		  case CODE_OBJFLAG:	max_to_check = max_check_objflag;		break;
		  case CODE_OBJMAGIC:	max_to_check = max_check_objmagic;		break;
		  case CODE_OBJTYPE:	max_to_check = max_check_objtype;		break;
		  case CODE_OBJWEAR:	max_to_check = max_check_objwear;		break;
		  case CODE_PART:		max_to_check = max_check_part;			break;
		  case CODE_PIPE:		max_to_check = max_check_pipe;			break;
		  case CODE_PLAYER:		max_to_check = max_check_player;		break;
		  case CODE_POINTS:		max_to_check = max_check_points;		break;
		  case CODE_POSITION:	max_to_check = max_check_position;		break;
		  case CODE_PROTOCOL:	max_to_check = max_check_protocol;		break;
		  case CODE_PULL:		max_to_check = max_check_pull;			break;
		  case CODE_RACE:		max_to_check = max_check_race;			break;
		  case CODE_RACEFLAG:	max_to_check = max_check_raceflag;		break;
		  case CODE_RIS:		max_to_check = max_check_ris;			break;
		  case CODE_ROOM:		max_to_check = max_check_room;			break;
		  case CODE_SAVETHROW:	max_to_check = max_check_savethrow;		break;
		  case CODE_SECTOR:		max_to_check = max_check_sector;		break;
		  case CODE_SENSE:		max_to_check = max_check_sense;			break;
		  case CODE_SEX:		max_to_check = max_check_sex;			break;
		  case CODE_SKELL:		max_to_check = max_check_skell;			break;
		  case CODE_SKILLTYPE:	max_to_check = max_check_skilltype;		break;
		  case CODE_SPELLACTION:max_to_check = max_check_spellaction;	break;
		  case CODE_SPELLDAMAGE:max_to_check = max_check_spelldamage;	break;
		  case CODE_SPELLPOWER:	max_to_check = max_check_spellpower;	break;
		  case CODE_SPELLSAVE:	max_to_check = max_check_spellsave;		break;
		  case CODE_SPELLSPHERE:max_to_check = max_check_spellsphere;	break;
		  case CODE_STYLE:		max_to_check = max_check_style;			break;
		  case CODE_TARGET:		max_to_check = max_check_target;		break;
		  case CODE_TRAPFLAG:	max_to_check = max_check_trapflag;		break;
		  case CODE_TRIG:		max_to_check = max_check_trig;			break;
		  case CODE_TRUST:		max_to_check = max_check_trust;			break;
		  case CODE_VOTE:		max_to_check = max_check_vote;			break;
		  case CODE_WEAPON:		max_to_check = max_check_weapon;		break;
		  case CODE_WEARFLAG:	max_to_check = max_check_wearflag;		break;
		  case CODE_WEARLOC:	max_to_check = max_check_wearloc;		break;
		}

		if ( max_to_check != table_code[x].max )
		{
			send_log( NULL, LOG_CODE, "check_code_table: (%s) massimo MAX_ %d diverso dal valore sizeof %d",
				table_code[x].code.name, table_code[x].max, max_to_check );
			exit( EXIT_FAILURE );
		}

		/* Salta il resto dei controlli per la tabella di codici wearflag, che è particolare */
		if ( x == CODE_WEARFLAG )
			continue;

		/* Ora di ogni tabella di codici controlla che l'id sia uguale alla posizione nell'array */
		if ( table_code[x].table )
		{
			for ( y = 0;  y < table_code[x].max;  y++ )
			{
				if ( table_code[x].table[y].id != y )
				{
					send_log( NULL, LOG_CODE, "check_code_table: (%s) definizione id numero %d si trova al posto sbagliato nell'array di codici: %d",
						table_code[x].code.name, table_code[x].table[y].id, y );
					exit( EXIT_FAILURE );
				}
			}
		}
		else
		{
			for ( y = 0;  y < table_code[x].max;  y++ )
			{
				int id = -1;

				switch ( x )
				{
				  default:
					send_log( NULL, LOG_CODE, "check_code_table: tipologia di codice mancante %d per la tabella di codici NULL", x );
					exit( EXIT_FAILURE );
				  case CODE_CHANNEL:	id = table_channel[y].code.id;	break;
				  case CODE_CODE:		id = table_code[y].code.id;		break;
				  case CODE_COLOR:		id = table_color[y].code.id;	break;
				  case CODE_COLORHAIR:	id = table_color[y].hair.id;	break;
				  case CODE_DAMAGE:		id = table_damage[y].code.id;	break;
				  case CODE_DIR:		id = table_dir[y].code.id;		break;
				  case CODE_LOG:		id = table_log[y].code.id;		break;
				  case CODE_PART:		id = table_part[y].code.id;		break;
				  case CODE_PLAYER:		id = table_player[y].code.id;	break;
				  case CODE_POINTS:		id = table_points[y].code.id;	break;
				  case CODE_PROTOCOL:	id = table_protocol[y].code.id;	break;
				  case CODE_SECTOR:		id = table_sector[y].code.id;	break;
				}

				if ( id != y )
				{
					send_log( NULL, LOG_CODE, "check_code_table: (%s) definizione id numero %d si trova al posto sbagliato nell'array di codici: %d",
						table_code[x].code.name, id, y );
					exit( EXIT_FAILURE );
				}
			}
		}

		/* Poi controlla se i codice o i nomi dei codici sono duplicati nella stessa tabella */
		for ( y = 0;  y < table_code[x].max;  y++ )
		{
			int	z;

			for ( z = y+1;  z < table_code[x].max;  z++ )
			{
				if ( table_code[x].table )
				{
					if ( !str_cmp(table_code[x].table[y].code, table_code[x].table[z].code) )
					{
						send_log( NULL, LOG_CODE, "check_code_table: (%s) codici %s uguali alla posizione %d e %d",
							table_code[x].code.name, table_code[x].table[y].code, y, z );
						exit( EXIT_FAILURE );
					}
					if ( !str_cmp(table_code[x].table[y].name, table_code[x].table[z].name) )
					{
						send_log( NULL, LOG_CODE, "check_code_table: (%s) nomi di codice %s uguali alla posizione %d e %d",
							table_code[x].code.name, table_code[x].table[y].name, y, z );
						exit( EXIT_FAILURE );
					}
				}
				else
				{
					char	*code_y = NULL;
					char	*code_z = NULL;

					switch ( x )
					{
					  default:
						send_log( NULL, LOG_CODE, "check_code_table: tipologia di codice mancante %d per la tabella di codici NULL", x );
						exit( EXIT_FAILURE );
					  case CODE_CHANNEL:	code_y = table_channel[y].code.code;	code_z = table_channel[z].code.code;	break;
					  case CODE_CODE:		code_y = table_code[y].code.code;		code_z = table_code[z].code.code;		break;
					  case CODE_COLOR:		code_y = table_color[y].code.code;		code_z = table_color[z].code.code;		break;
					  case CODE_COLORHAIR:	code_y = table_color[y].hair.code;		code_z = table_color[z].hair.code;		break;
					  case CODE_DAMAGE:		code_y = table_damage[y].code.code;		code_z = table_damage[z].code.code;		break;
					  case CODE_DIR:		code_y = table_dir[y].code.code;		code_z = table_dir[z].code.code;		break;
					  case CODE_LOG:		code_y = table_log[y].code.code;		code_z = table_log[z].code.code;		break;
					  case CODE_PART:		code_y = table_part[y].code.code;		code_z = table_part[z].code.code;		break;
					  case CODE_PLAYER:		code_y = table_player[y].code.code;		code_z = table_player[z].code.code;		break;
					  case CODE_POINTS:		code_y = table_points[y].code.code;		code_z = table_points[z].code.code;		break;
					  case CODE_PROTOCOL:	code_y = table_protocol[y].code.code;	code_z = table_protocol[z].code.code;	break;
					  case CODE_SECTOR:		code_y = table_sector[y].code.code;		code_z = table_sector[z].code.code;		break;
					}

					if ( !str_cmp(code_y, code_z) )
					{
						send_log( NULL, LOG_CODE, "check_code_table: (%s) codici %s uguali alla posizione %d e %d",
							table_code[x].code.name, code_y, y, z );
						exit( EXIT_FAILURE );
					}
				}
			}
		}
	}
}


/*
 * Legge da un file la stringa di codici BITVECTOR terminati da una tilde
 */
BITVECTOR *fread_code_bit( MUD_FILE *fp, const int type )
{
	BITVECTOR	*bits;
	char		*pstr;
	char		 str[MSL];
	char		 flag[MIL];
	int			 value;

	strcpy( str, fread_string(fp) );
	pstr = str;

#ifdef DEBUG_MEMORYLEAK
	createbv( bits, __FILE__, __LINE__ );
#else
	createbv( bits );
#endif

	while ( VALID_STR(pstr) )
	{
		pstr = one_argument( pstr, flag );
		value = code_code( fp, flag, type );
		if ( value < 0 || value >= code_max(type) )
		{
			send_log( fp, LOG_CODE, "fread_code_bit: bit sconosciuto: %s per il tipo %s",
				flag, table_code[type].code.name );
		}
		else
			SET_BIT( bits, value );
	}

	return bits;
}


/*
 * Legge da un file la stringa di codici, per un bitvector intero, terminati da una tilde
 */
int fread_code_bitint( MUD_FILE *fp, const int type )
{
	char	*pstr;
	char	 str[MSL];
	char	 flag[MIL];
	int		 bits = 0;
	int		 value;

	strcpy( str, fread_string(fp) );
	pstr = str;

	while ( VALID_STR(pstr) )
	{
		pstr = one_argument( pstr, flag );
		value = code_code( fp, flag, type );
		if ( value < 0 || value >= code_max(type) )
		{
			send_log( fp, LOG_CODE, "fread_code_bitint: bit sconosciuto: %s per il tipo %s",
				flag, table_code[type].code.name );
		}
		else
			SET_BITINT( bits, value );
	}

	return bits;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = *\
 *						Sezione delle tabelle di tipo struct code_data			                       *
\* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/*
 * Tabella dei codici sulle flag di area
 */
const CODE_DATA code_areaflag[] =
{
	{ AREAFLAG_NOPKILL,		"AREAFLAG_NOPKILL",		"nopkill"		},
	{ AREAFLAG_FREEKILL,	"AREAFLAG_FREEKILL",	"freekill"		},
	{ AREAFLAG_NOTELEPORT,	"AREAFLAG_NOTELEPORT",	"noteleport"	},
	{ AREAFLAG_SPELL_LIMIT,	"AREAFLAG_SPELL_LIMIT",	"spell_limit"	},
	{ AREAFLAG_NOSAVE,		"AREAFLAG_NOSAVE",		"nosave"		},
	{ AREAFLAG_HELL,		"AREAFLAG_HELL",		"hell"			},
	{ AREAFLAG_ARENA,		"AREAFLAG_ARENA",		"arena"			},
	{ AREAFLAG_NOPOINTLOSS,	"AREAFLAG_NOPOINTLOSS",	"nopointloss"	},
	{ AREAFLAG_CITY,		"AREAFLAG_CITY",		"città"			},
	{ AREAFLAG_WEIGHTHEIGHT,"AREAFLAG_WEIGHTHEIGHT","weightheight"	},
	{ AREAFLAG_TAGGED,		"AREAFLAG_TAGGED",		"tagged"		},
	{ AREAFLAG_SECRET,		"AREAFLAG_SECRET",		"secret"		},
	{ AREAFLAG_WILD,		"AREAFLAG_WILD",		"wild"			},
	{ AREAFLAG_LOADED,		"AREAFLAG_LOADED",		"caricata"		},
	{ AREAFLAG_DELETED,		"AREAFLAG_DELETED",		"distrutta"		}
};
const int max_check_areaflag = sizeof(code_areaflag)/sizeof(CODE_DATA);



/*
 * Tabella dei codici sui tipi di attributi
 */
const CODE_DATA code_attr[] =
{
	{ ATTR_STR,	"ATTR_STR",	"forza"				},
	{ ATTR_RES,	"ATTR_RES",	"resistenza"		},
	{ ATTR_CON,	"ATTR_CON",	"costituzione"		},
	{ ATTR_INT,	"ATTR_INT",	"intelligenza"		},
	{ ATTR_COC,	"ATTR_COC",	"concentrazione"	},
	{ ATTR_WIL,	"ATTR_WIL",	"volontà"			},
	{ ATTR_AGI,	"ATTR_AGI",	"agilità"			},
	{ ATTR_REF,	"ATTR_REF",	"riflessi"			},
	{ ATTR_SPE,	"ATTR_SPE",	"velocità"			},
	{ ATTR_SPI,	"ATTR_SPI",	"spiritualità"		},
	{ ATTR_MIG,	"ATTR_MIG",	"migrazione"		},
	{ ATTR_ABS,	"ATTR_ABS",	"assorbimento"		},
	{ ATTR_EMP,	"ATTR_EMP",	"empatia"			},
	{ ATTR_BEA,	"ATTR_BEA",	"bellezza"			},
	{ ATTR_LEA,	"ATTR_LEA",	"comando"			}
};
const int max_check_attr = sizeof(code_attr)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle condizioni del pg
 */
const CODE_DATA code_condition[] =
{
	{ CONDITION_DRUNK,		"CONDITION_DRUNK",		"sobrietà"		 },
	{ CONDITION_FULL,		"CONDITION_FULL",		"fame"			 },
	{ CONDITION_THIRST,		"CONDITION_THIRST",		"sete"			 },
	{ CONDITION_BLOODTHIRST,"CONDITION_BLOODTHIRST","sete di sangue" }
};
const int max_check_condition = sizeof(code_condition)/sizeof(CODE_DATA);


/*
 * Tabella codici delle flag dei mob
 */
const CODE_DATA code_mob[] =
{
	{ MOB_IS_MOB,		"MOB_IS_MOB",		"npc"			},
	{ MOB_SENTINEL,		"MOB_SENTINEL",		"sentinel"		},
	{ MOB_SCAVENGER,	"MOB_SCAVENGER",	"scavenger"		},
	{ MOB_SING,			"MOB_SING",			"sing"			},
	{ MOB_MORGUE,		"MOB_MORGUE",		"morgue"		},
	{ MOB_AGGRESSIVE,	"MOB_AGGRESSIVE",	"aggressive"	},
	{ MOB_STAY_AREA,	"MOB_STAY_AREA",	"stayarea"		},
	{ MOB_WIMPY,		"MOB_WIMPY",		"wimpy"			},
	{ MOB_MASCOTTE,		"MOB_MASCOTTE",		"mascotte"		},
	{ MOB_TRAIN,		"MOB_TRAIN",		"train"			},
	{ MOB_PRACTICE,		"MOB_PRACTICE",		"practice"		},
	{ MOB_CARRY,		"MOB_CARRY",		"carry"			},
	{ MOB_DEADLY,		"MOB_DEADLY",		"deadly"		},
	{ MOB_POLYSELF,		"MOB_POLYSELF",		"polyself"		},
	{ MOB_META_AGGR,	"MOB_META_AGGR",	"meta_aggr"		},
	{ MOB_GUARDIAN,		"MOB_GUARDIAN",		"guardian"		},
	{ MOB_RUNNING,		"MOB_RUNNING",		"running"		},
	{ MOB_NOWANDER,		"MOB_NOWANDER",		"nowander"		},
	{ MOB_MOUNTABLE,	"MOB_MOUNTABLE",	"mountable"		},
	{ MOB_MOUNTED,		"MOB_MOUNTED",		"mounted"		},
	{ MOB_SCHOLAR,		"MOB_SCHOLAR",		"scholar"		},
	{ MOB_SECRETIVE,	"MOB_SECRETIVE",	"secretive"		},
	{ MOB_HARDHAT,		"MOB_HARDHAT",		"hardhat"		},
	{ MOB_INCOGNITO,	"MOB_INCOGNITO",	"incognito"		},
	{ MOB_NOASSIST,		"MOB_NOASSIST",		"noassist"		},
	{ MOB_AUTONOMOUS,	"MOB_AUTONOMOUS",	"autonomous"	},
	{ MOB_PACIFIST,		"MOB_PACIFIST",		"pacifist"		},
	{ MOB_NOATTACK,		"MOB_NOATTACK",		"noattack"		},
	{ MOB_ANNOYING,		"MOB_ANNOYING",		"annoying"		},
	{ MOB_DAY,			"MOB_DAY",			"diurno"		},
	{ MOB_NIGHT,		"MOB_NIGHT",		"notturno"		},
	{ MOB_BANKER,		"MOB_BANKER",		"bancario"		},
	{ MOB_LOCKER,		"MOB_LOCKER",		"locker"		}
};
const int max_check_mob = sizeof(code_mob)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle posizioni
 */
const CODE_DATA code_position[] =
{
	{ POSITION_DEAD,		"POSITION_DEAD",		"dead"				},
	{ POSITION_MORTAL,		"POSITION_MORTAL",		"mortallywounded"	},
	{ POSITION_INCAP,		"POSITION_INCAP",		"incapacitated"		},
	{ POSITION_STUN,		"POSITION_STUN",		"stunned"			},
	{ POSITION_SLEEP,		"POSITION_SLEEP",		"sleeping"			},
	{ POSITION_BERSERK,		"POSITION_BERSERK",		"berserk"			},
	{ POSITION_REST,		"POSITION_REST",		"resting"			},
	{ POSITION_AGGRESSIVE,	"POSITION_AGGRESSIVE",	"aggressive"		},
	{ POSITION_SIT,			"POSITION_SIT",			"sitting"			},
	{ POSITION_FIGHT,		"POSITION_FIGHT",		"fighting"			},
	{ POSITION_DEFENSIVE,	"POSITION_DEFENSIVE",	"defensive"			},
	{ POSITION_EVASIVE,		"POSITION_EVASIVE",		"evasive"			},
	{ POSITION_STAND,		"POSITION_STAND",		"standing"			},
	{ POSITION_MOUNT,		"POSITION_MOUNT",		"mounted"			},
	{ POSITION_SHOVE,		"POSITION_SHOVE",		"shoved"			},
	{ POSITION_DRAG,		"POSITION_DRAG",		"dragged"			}
};
const int max_check_position = sizeof(code_position)/sizeof(CODE_DATA);


/*
 * Tabella codici sui tiri di salvezza
 */
const CODE_DATA code_savethrow[] =
{
	{ SAVETHROW_POISON_DEATH,	"SAVETHROW_POISON_DEATH",	"poison_death"	},
	{ SAVETHROW_ROD_WANDS,		"SAVETHROW_ROD_WANDS",		"wands_rod"		},
	{ SAVETHROW_PARA_PETRI,		"SAVETHROW_PARA_PETRI",		"para_petri"	},
	{ SAVETHROW_BREATH,			"SAVETHROW_BREATH",			"breath"		},
	{ SAVETHROW_SPELL_STAFF,	"SAVETHROW_SPELL_STAFF",	"spell_staff"	}
};
const int max_check_savethrow = sizeof(code_savethrow)/sizeof(CODE_DATA);


/*
 * Tabella codici dei sensi
 */
const CODE_DATA code_sense[] =
{
	{ SENSE_SIGHT,		"SENSE_SIGHT",		"vista"		},
	{ SENSE_HEARING,	"SENSE_HEARING",	"udito"		},
	{ SENSE_SMELL,		"SENSE_SMELL",		"olfatto"	},
	{ SENSE_TASTE,		"SENSE_TASTE",		"gusto"		},
	{ SENSE_TOUCH,		"SENSE_TOUCH",		"tatto"		},
	{ SENSE_SIXTH,		"SENSE_SIXTH",		"intuito"	}
};
const int max_check_sense = sizeof(code_sense)/sizeof(CODE_DATA);


/*
 * Tabella codici sul sesso
 */
const CODE_DATA code_sex[] =
{
	{ SEX_MALE,		"SEX_MALE",		"maschio" },
	{ SEX_FEMALE,	"SEX_FEMALE",	"femmina" }
};
const int max_check_sex = sizeof(code_sex)/sizeof(CODE_DATA);


/*
 * Tabella codici sugli stili di combattimento
 */
const CODE_DATA code_style[] =
{
	{ STYLE_BERSERK,	"STYLE_BERSERK",	"berserk"		},
	{ STYLE_AGGRESSIVE,	"STYLE_AGGRESSIVE",	"aggressivo"	},
	{ STYLE_FIGHTING,	"STYLE_FIGHTING",	"normale"		},
	{ STYLE_DEFENSIVE,	"STYLE_DEFENSIVE",	"difensivo"		},
	{ STYLE_EVASIVE,	"STYLE_EVASIVE",	"evasivo"		}
};
const int max_check_style = sizeof(code_style)/sizeof(CODE_DATA);


/*
 * Tabella codici sui tipi di obiettivo
 */
const CODE_DATA code_target[] =
{
	{ TARGET_IGNORE,			"TARGET_IGNORE",			"ignore"			},
	{ TARGET_CHAR_OFFENSIVE,	"TARGET_CHAR_OFFENSIVE",	"char_offensive"	},
	{ TARGET_CHAR_DEFENSIVE,	"TARGET_CHAR_DEFENSIVE",	"char_defensive"	},
	{ TARGET_CHAR_SELF,			"TARGET_CHAR_SELF",			"char_self"			},
	{ TARGET_OBJ_INV,			"TARGET_OBJ_INV",			"obj_inv"			}
};
const int max_check_target = sizeof(code_target)/sizeof(CODE_DATA);


/*
 * Tabella codici sui valori della fiducia
 */
const CODE_DATA code_trust[] =
{
	{ TRUST_PLAYER,		"TRUST_PLAYER",		"Player"			},
	{ TRUST_NEOMASTER,	"TRUST_NEOMASTER",	"NeoMaster"			},
	{ TRUST_MASTER,		"TRUST_MASTER",		"Game Master"		},
	{ TRUST_NEOBUILDER,	"TRUST_NEOBUILDER",	"NeoCostruttore"	},
	{ TRUST_BUILDER,	"TRUST_BUILDER",	"Costruttore"		},
	{ TRUST_IMPLE,		"TRUST_IMPLE",		"Implementatore"	}
};
const int max_check_trust = sizeof(code_trust)/sizeof(CODE_DATA);


/*
 * Tabella codici sulle locazioni wear
 * Non c'è da inserire WEARLOC_NONE perché enumerato come -1
 */
const CODE_DATA code_wearloc[] =
{
	{ WEARLOC_LIGHT,	"WEARLOC_LIGHT",	"light"		},
	{ WEARLOC_FINGER_L,	"WEARLOC_FINGER_L",	"finger1"	},
	{ WEARLOC_FINGER_R,	"WEARLOC_FINGER_R",	"finger2"	},
	{ WEARLOC_NECK,		"WEARLOC_NECK",		"neck1"		},
	{ WEARLOC_NECK_2,	"WEARLOC_NECK_2",	"neck2"		},
	{ WEARLOC_BODY,		"WEARLOC_BODY",		"body"		},
	{ WEARLOC_HEAD,		"WEARLOC_HEAD",		"head"		},
	{ WEARLOC_LEGS,		"WEARLOC_LEGS",		"legs"		},
	{ WEARLOC_FEET,		"WEARLOC_FEET",		"feet"		},
	{ WEARLOC_HANDS,	"WEARLOC_HANDS",	"hands"		},
	{ WEARLOC_ARMS,		"WEARLOC_ARMS",		"arms"		},
	{ WEARLOC_SHIELD,	"WEARLOC_SHIELD",	"shield"	},
	{ WEARLOC_ABOUT,	"WEARLOC_ABOUT",	"about"		},
	{ WEARLOC_WAIST,	"WEARLOC_WAIST",	"waist"		},
	{ WEARLOC_WRIST_L,	"WEARLOC_WRIST_L",	"wrist1"	},
	{ WEARLOC_WRIST_R,	"WEARLOC_WRIST_R",	"wrist2"	},
	{ WEARLOC_WIELD,	"WEARLOC_WIELD",	"wield"		},
	{ WEARLOC_HOLD,		"WEARLOC_HOLD",		"hold"		},
	{ WEARLOC_DUAL,		"WEARLOC_DUAL",		"dual"		},
	{ WEARLOC_EARS,		"WEARLOC_EARS",		"ears"		},
	{ WEARLOC_EYES,		"WEARLOC_EYES",		"eyes"		},
	{ WEARLOC_MISSILE,	"WEARLOC_MISSILE",	"missile"	},
	{ WEARLOC_BACK,		"WEARLOC_BACK",		"back"		},
	{ WEARLOC_FACE,		"WEARLOC_FACE",		"face"		},
	{ WEARLOC_ANKLE_L,	"WEARLOC_ANKLE_L",	"ankle1"	},
	{ WEARLOC_ANKLE_R,	"WEARLOC_ANKLE_R",	"ankle2"	},
	{ WEARLOC_FLOAT,	"WEARLOC_FLOAT",	"float"		}
};
const int max_check_wearloc = sizeof(code_wearloc)/sizeof(CODE_DATA);
