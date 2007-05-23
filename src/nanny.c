/*--------------------------------------------------------------------------------------*
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
 >				Modulo riguardo la creazione del pg e il login				<
\****************************************************************************/


#include <ctype.h>
#include <sys/stat.h>

#include "mud.h"
#include "adminhost.h"
#include "admin.h"
#include "affect.h"
#include "animation.h"
#include "archetype.h"
#include "ban.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "gramm.h"
#include "help.h"
#include "interpret.h"
#include "mccp.h"
#include "md5.h"
#include "msp.h"
#include "mxp.h"
#include "nanny.h"
#include "note.h"
#include "olc.h"
#include "question.h"
#include "reserved.h"
#include "room.h"
#include "save.h"
#include "timer.h"
#include "watch.h"
#include "web.h"


/* Tutti i messaggi di creazione di questo file hanno tendenzialmente le accenti
 *	apostrofate volutamente per una maggiore compatibilità con i più vari client,
 *	dai messaggi di connessione, creazione e prima della scelta riguardante gli
 *	accenti
 */


/*
 * Variabile locale che contiene i nomi casuali visualizzati da
 *	show_get_random_name() e poi scelti dal pg
 */
char *rand_names[5];


/*
 * Tabella codici sulle tipologie di connessione
 */
const CODE_DATA code_connected[] =
{
	{ CON_GET_NAME,				"CON_GET_NAME",				"get_name"				},
	{ CON_GET_PASSWORD,			"CON_GET_PASSWORD",			"get_password"			},
	{ CON_NEW_GET_NAME,			"CON_NEW_GET_NAME",			"new_get_name"			},
	{ CON_NEW_GET_PASSWORD,		"CON_NEW_GET_PASSWORD",		"new_get_password"		},
	{ CON_NEW_CONFIRM_PASSWORD,	"CON_NEW_CONFIRM_PASSWORD",	"new_confirm_password"	},
	{ CON_NEW_WANT_SEX,			"CON_NEW_WANT_SEX",			"new_want_sex"			},
	{ CON_NEW_WANT_CREATION,	"CON_NEW_WANT_CREATION",	"new_want_creation"		},
	{ CON_NEW_GET_RACE,			"CON_NEW_GET_RACE",			"new_get_race"			},
	{ CON_NEW_GET_ALIGNMENT,	"CON_NEW_GET_ALIGNMENT",	"new_get_alignment"		},
	{ CON_NEW_GET_CLASS,		"CON_NEW_GET_CLASS",		"con_new_get_class"		},
#ifdef T2_QUESTION
	{ CON_NEW_WANT_QUESTION,	"CON_NEW_WANT_QUESTION",	"new_want_question"		},
	{ CON_NEW_GET_QUESTION,		"CON_NEW_GET_QUESTION",		"new_get_question"		},
#endif                             
	{ CON_NEW_GET_WEIGHT,		"CON_NEW_GET_WEIGHT",		"new_get_weight"		},
	{ CON_NEW_GET_HEIGHT,		"CON_NEW_GET_HEIGHT",		"new_get_height"		},
	{ CON_NEW_GET_HAIRCOLOR,	"CON_NEW_GET_HAIRCOLOR",	"new_get_haircolor"		},
	{ CON_NEW_GET_HAIRTYPE,		"CON_NEW_GET_HAIRTYPE",		"new_get_hairtype"		},
	{ CON_NEW_GET_HAIRLENGTH,	"CON_NEW_GET_HAIRLENGTH",	"new_get_hairlength"	},
	{ CON_NEW_GET_EYECOLOR,		"CON_NEW_GET_EYECOLOR",		"new_get_eyecolor"		},
#ifdef T2_ALFA
	{ CON_NEW_GET_SKINCOLOR,	"CON_NEW_GET_SKINCOLOR",	"new_get_skincolor"		},
#endif                             
	{ CON_NEW_REROLL_ATTRS,		"CON_NEW_REROLL_ATTRS",		"new_reroll_attrs"		},
	{ CON_NEW_WANT_HAND,		"CON_NEW_WANT_HAND",		"new_want_hand"			},
	{ CON_NEW_GET_ARCHETYPE,	"CON_NEW_GET_ARCHETYPE",	"new_get_archetype"		},
	{ CON_NEW_WANT_LANGUAGE,	"CON_NEW_WANT_LANGUAGE",	"new_want_language"		},
	{ CON_NEW_WANT_ACCENT,		"CON_NEW_WANT_ACCENT",		"new_want_accent"		},
	{ CON_NEW_WANT_BACK,		"CON_NEW_WANT_BACK",		"new_want_back"			},
	{ CON_MENU,					"CON_MENU",					"menu"					},
	{ CON_PRESS_ENTER,			"CON_PRESS_ENTER",			"press_enter"			},
	{ CON_PLAYING,				"CON_PLAYING",				"playing"				},
	{ CON_EDITING,				"CON_EDITING",				"editing"				},
	{ CON_MEDIT,				"CON_MEDIT",				"medit"					},
	{ CON_OEDIT,				"CON_OEDIT",				"oedit"					},
	{ CON_REDIT,				"CON_REDIT",				"redit"					},
	{ CON_COPYOVER_RECOVER,		"CON_COPYOVER_RECOVER",		"copyover_recover"		}
};
const int max_check_connected = sizeof(code_connected)/sizeof(CODE_DATA);


/*
 * Controlla che la password sia accettabile
 * Ritorna NULL se non lo è
 * Se è accettabile ritorna la password criptata
 */
char *check_password( CHAR_DATA *ch, char *argument )
{
	static char	*pwdnew;
	char		*p;
#ifndef NOCRYPT
	static char  pwd[18];
#endif

	/* Se la password non è almeno di 6 caretteri non l'accetta. */
	if ( strlen(argument) < 6 )
	{
		write_to_buffer( ch->desc,
			"\r\nLa parola chiave deve essere composta da almeno 6 caratteri.\r\n", 0 );
		return NULL;
	}

	/* Se la password è uguale al nome non l'accetta. */
	if ( !str_cmp(argument, ch->name) )
	{
		write_to_buffer( ch->desc,
			"\r\nNon e' bene scegliere una parola chiave cosi' simile al nome del personaggio.\r\n", 0 );
		return NULL;
	}

	if ( argument[0] == '!' )
	{
		write_to_buffer( ch->desc, "La password non puo' iniziare con il carattere '!'.\r\n", 0 );
		return NULL;
	}

	pwdnew = cryptmd5( argument );
	for ( p = pwdnew;  *p;  p++ )
	{
		if ( *p == '~' )
		{
			write_to_buffer( ch->desc, "Parola chiave non accettabile, prova ancora.\r\n", 0 );
			return NULL;
		}
	}

#ifdef NOCRYPT
    return pwdnew;
#else
	sprintf( pwd, "!" );
	strncat( pwd, pwdnew, 16 );
	return pwd;
#endif
}


/*
 * Visualizza il menù per la scelta del nome
 */
void show_get_name( DESCRIPTOR_DATA *d, const char *name )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	ch_printf( ch, "\r\nIl nome da te scelto per il tuo personaggio e' &W%s&w, desideri:\r\n", name );
	send_to_char( ch,
		"1] Accettarlo\r\n"
		"2] Inserirne un'altro nuovo\r\n"
		"Quale è la tua scelta? " );
}


/*
 * Visualizza il messaggio di scelta per il sesso.
 */
void show_want_sex( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\nVorresti un personaggio:\r\n" );
	send_to_char( ch, " &W0&w] Con sessualita' casuale.\r\n" );
	send_to_char( ch, " &W1&w] &BMaschile&w.\r\n" );
	send_to_char( ch, " &W2&w] &PFemminile&w.\r\n" );
	send_to_char( ch, "Fai la tua scelta: " );
}


/*
 * Visualizza la lista delle razze possibili da scegliere.
 */
void show_get_race( DESCRIPTOR_DATA *d )
{
	CHAR_DATA  *ch;
	char		race_name[MIL];
	char		buf[MSL];
	int			x;
	int			count = 0;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );
	/* Visualizza la frase di scelta con la lista delle razze */
	send_to_char( ch, "\r\nLe &Wrazze&w disponibili sono:\r\n" );
	buf[0] = '\0';

	for ( x = 0;  x < MAX_RACE;  x++ )
	{
		if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
			continue;

		if ( ch->sex == SEX_FEMALE  )
			strcpy( race_name, table_race[x]->name_fema );
		else
			strcpy( race_name, table_race[x]->name_male );

		/* Dà la possibilità di scegliere casualmente la razza */
		if ( x == 0 )
			sprintf( buf+strlen(buf), "&W 0&w] %-13s", "Casuale" );

		sprintf( buf+strlen(buf), "&W%2d&w] &%c%-13s&w ",
			x+1, get_clr(table_race[x]->color->cat), race_name );

		count++;
		if ( count%4 == 0 && x != MAX_RACE-1 )
			strcat( buf, "\r\n" );
	}

	send_to_char( ch, buf );
#ifdef T2_ALFA
	send_to_char( ch, "\r\nScrivi  aiuto <nomerazza>  per avere piu' informazioni riguardo quella razza." );
#endif
	send_to_char( ch, "\r\n(PS: il Thepa praticamente e' un Uomo Lucertola)\r\n" );
	send_to_char( ch, "Scegli il numero della razza che vuoi dare al tuo personaggio: " );
}


void show_get_alignment( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	/* Visualizza le frasi di scelta con i possibili allineamenti */
	send_to_char( ch, "\r\n\r\nCome vorresti che fosse l'&Wallineamento&w del tuo personaggio?\r\n" );
	send_to_char( ch, " &W0&w] Allineamento casuale relativo alla razza.\r\n" );
	send_to_char( ch, " &W1&w] Piu' cattivo della media della sua razza.\r\n" );
	send_to_char( ch, " &W2&w] Nella media della sua razza.\r\n" );
	send_to_char( ch, " &W3&w] Piu' buono della media della sua razza.\r\n" );
	send_to_char( ch, "Scegli la tendenza che piu' ti aggrada: " );
}


/*
 * Visualizza la lista dei class possibili da scegliere.
 */
void show_get_class( DESCRIPTOR_DATA *d )
{
	CHAR_DATA  *ch;
	char		class_name[MIL];
	char		buf[MSL];
	int			x;
	int			count = 0;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );
	send_to_char( ch, "\r\nLe &Wclassi&w disponibili sono:\r\n" );
	buf[0] = '\0';

	for ( x = 0;  x < MAX_CLASS;  x++ )
	{
		if ( !HAS_BIT(table_class[x]->flags, CLASSFLAG_PLAYABLE) )
			continue;

		if ( ch->sex == SEX_FEMALE )
			strcpy( class_name, table_class[x]->name_fema );
		else
			strcpy( class_name, table_class[x]->name_male );

		/* Dà la possibilità di scegliersi la classe casualmente */
		if ( x == 0 )
			sprintf( buf+strlen(buf), "&W 0&w] %-13s ", "Casuale" );

		sprintf( buf+strlen(buf), "&W%2d&w] &%c%-13s&w ",
			x+1, get_clr(table_class[x]->color->cat), class_name );

		count++;
		if ( count%4 == 0 && x != MAX_CLASS-1 )
			strcat( buf, "\r\n" );
	}

	send_to_char( ch, buf );
	send_to_char( ch, "\r\nScrivi  aiuto <nomeclasse>  per avere piu' informazioni riguardo quella classe." );
	send_to_char( ch, "\r\nScegli il numero della classe che vuoi dare al tuo personaggio: " );
}


/*
 * Visualizza il testo per la scelta del peso
 */
void show_get_weight( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\n\r\nCome vorresti che fosse il &WPeso&w del tuo personaggio?\r\n" );
	send_to_char( ch, " &W0&w] Peso casuale relativo alla razza.\r\n" );
	send_to_char( ch, " &W1&w] Piu' pesante della media della sua razza.\r\n" );
	send_to_char( ch, " &W2&w] Nella media della sua razza.\r\n" );
	send_to_char( ch, " &W3&w] Piu' leggero della media della sua razza.\r\n" );
	send_to_char( ch, "Scegli il peso che piu' ti aggrada: " );
}


/*
 * Visualizza il testo per la scelta del peso
 */
void show_get_height( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\n\r\nCome vorresti che fosse l'&WAltezza&w del tuo personaggio?\r\n" );
	send_to_char( ch, " &W0&w] Altezza casuale relativa alla razza.\r\n" );
	send_to_char( ch, " &W1&w] Piu' basso della media della sua razza.\r\n" );
	send_to_char( ch, " &W2&w] Nella media della sua razza.\r\n" );
	send_to_char( ch, " &W3&w] Piu' alto della media della sua razza.\r\n" );
	send_to_char( ch, "Scegli l'altezza che piu' ti aggrada: " );
}


/*
 * Visualizza la lista dei tipi di capelli.
 */
void show_get_hairtype( DESCRIPTOR_DATA *d )
{
	CHAR_DATA  *ch;
	char		buf[MSL];
	int			x;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );
	send_to_char( ch, "\r\nChe &Wtipo di capelli&w vorresti che il tuo personaggio abbia?\r\n" );

	buf[0] = '\0';
	for ( x = 0;  x < MAX_HAIRTYPE;  x++ )
	{
		if ( x == 0 )
			sprintf( buf+strlen(buf), "&W 0&w] %-13s ", "Casuale" );

		sprintf( buf+strlen(buf), "&W%2d&w] %-13s ", x+1, code_name(NULL, x, CODE_HAIRTYPE) );

		if ( (x+1)%4 == 0 && x != 0 && x != MAX_HAIRTYPE-1 )
			strcat( buf, "\r\n" );
	}

	send_to_char( ch, buf );
	send_to_char( ch, "\r\nScegli il numero del tipo di capelli da dare al tuo personaggio: " );
}


/* (FF) magari in futuro passarle invece del nome della parte del corpo in numero attributivo se conviene
 * Visualizza la lista dei colori da scegliere per una determinata parte del corpo.
 */
void show_get_color( DESCRIPTOR_DATA *d, char *part_name, int type )
{
	CHAR_DATA	*ch;
	char		 buf[MSL];
	int			 x;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );
	ch_printf( ch, "\r\nEcco la lista del possibile &Wcolore&w per %s:\r\n",
		get_article(part_name, ARTICLE_PLURAL, ARTICLE_INDETERMINATE, -1) );

	buf[0] = '\0';
	for ( x = 0;  x < MAX_COLOR;  x++ )
	{
		if ( x == 0 )
			sprintf( buf+strlen(buf), "&W 0&w] %-13s ", "Casuale" );

		if ( type == CODE_COLOR )
		{
			sprintf( buf+strlen(buf), "&W%2d&w] &%c%-13s&w ",
				x+1, table_color[x].clr, table_color[x].code.name );
		}
		else if ( type == CODE_COLORHAIR )
		{
			sprintf( buf+strlen(buf), "&W%2d&w] &%c%-13s&w ",
				x+1, table_color[x].clr, table_color[x].hair.name );
		}
		else
		{
			send_log( NULL, LOG_BUG, "show_con_get_color: type parsing errato: %d", type );
			return;
		}

		if ( (x+1)%4 == 0 && x != 0 && x != 15 )
			strcat( buf, "\r\n" );
	}

	send_to_char( ch, buf );
	send_to_char( ch, "\r\nScegli il numero del colore che vorresti per il tuo personaggio: " );
}

/*
 * Ritorna la lunghezza massima normale per una razza
 */
int racial_hairlen( const int race )
{
	if ( race < 0 || race >= MAX_RACE )
	{
		send_log( NULL, LOG_BUG, "racial_hairlen: razza passata è errata: %d", race );
		return 100;
	}

	return ( table_race[race]->height * 0.40 );
}


/*
 * Visualizza la domanda di scelta per la lunghezza dei capelli
 */
void show_get_hairlenght( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\n\r\nQuanto lunghi vorresti i tuoi capelli?\r\n" );
	send_to_char( ch, "Digitando zero si otterra' una lunghezza casuale.\r\n" );
	ch_printf	( ch, "Scegli un numero tra 1 e %d, rapprensentano i centimetri: ", racial_hairlen(ch->race) );
}


/*
 * Visualizza la schermata di rerolling attributi
 */
void show_reroll_attrs( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;
	char	   buf[MSL];
	int		   x;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	ch_printf( ch,
		"\r\nOra puoi ridistribuire il punteggio dei tuoi attributi:\r\n"
		"Hai a disposizione %d crocett%c da distribuire:\r\n"
		"Se non capisci nulla di tutto cio' scrivi  &Wcontinua  &wper proseguire.\r\n\r\n",
		ch->desc->reroll, (ch->desc->reroll == 1) ? 'a' : 'e' );

	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		int attr;
		int y;

		/* Gli attributi qui sotto sono volutamente accentati con l'apostrofo */
		switch ( x )
		{
		  default:
			send_log( NULL, LOG_BUG, "show_reroll_attrs: attributo errato: %d", x );
			break;
		  case ATTR_STR:		send_to_char( ch, "&RForza          &G" );	break;
		  case ATTR_RES:		send_to_char( ch, "&RResistenza     &G" );	break;
		  case ATTR_CON:		send_to_char( ch, "&RCostituzione   &G" );	break;
		  case ATTR_INT:		send_to_char( ch, "&BIntelligenza   &G" );	break;
		  case ATTR_COC:		send_to_char( ch, "&BConcentrazione &G" );	break;
		  case ATTR_WIL:		send_to_char( ch, "&BVolonta'       &G" );	break;
		  case ATTR_AGI:		send_to_char( ch, "&GAgilita'       &G" );	break;
		  case ATTR_REF:		send_to_char( ch, "&GRiflessi       &G" );	break;
		  case ATTR_SPE:		send_to_char( ch, "&GVelocita'      &G" );	break;
		  case ATTR_SPI:		send_to_char( ch, "&CSpiritualita'  &G" );	break;
		  case ATTR_MIG:		send_to_char( ch, "&CMigrazione     &G" );	break;
		  case ATTR_ABS:		send_to_char( ch, "&CAssorbimento   &G" );	break;
		  case ATTR_EMP:		send_to_char( ch, "&YEmpatia        &G" );	break;
		  case ATTR_BEA:		send_to_char( ch, "&YBellezza       &G" );	break;
		  case ATTR_LEA:		send_to_char( ch, "&YComando        &G" );	break;
		}

		attr = ch->attr_perm[x] / 5;
		if ( attr < 0 )
		{
			send_log( NULL, LOG_BUG, "show_reroll_attrs: attributo negativo?" );
			continue;
		}

		buf[0] = '\0';
		for ( y = 0;  y < 20;  y++ )
		{
			if		( y ==  6 )		strcat( buf, "&W" );
			else if ( y == 12 )		strcat( buf, "&R" );

			if ( y < attr )
				strcat( buf, "+" );
		}
		ch_printf( ch, "%s\r\n", buf );
	}

	send_to_char( ch, "\r\n&wPuoi distribuire le crocette su tutti gli attributi, con alcuni limiti minimi e\r\n" );
	send_to_char( ch, "massimi, diminuendone ad uno ed aggiungendo ad un altro.\r\n" );
	send_to_char( ch, "Per diminuire un'attributo digitare: <&Wattributo&w> &R-&w     Es: &Wmigrazione &R-&w\r\n" );
	send_to_char( ch, "Per aumentare un'attributo digitare: <&Wattributo&w> &G+&w     Es: &Wforza &G+&w\r\n" );
	send_to_char( ch, "Per saperne di piu' riguardo agli attributi digitare:\r\n" );
	send_to_char( ch, "&Gaiuto&w <&gattributo&w>     Es: &Gaiuto&w &gspiritualita'&w\r\n" );
	send_to_char( ch, "Digita  &Wcontinua&w  quando hai finito la redistribuzione delle crocette. \r\n\r\n" );
}


/*
 * Visualizza il messaggio riguardante la scelta della mano principale
 */
void show_want_hand( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\nQuale e' la &Wmano principale&w del tuo personaggio?\r\n" );
	send_to_char( ch, " &W0&w] Mano principale casuale.\r\n" );
	send_to_char( ch, " &W1&w] La destra.\r\n" );
	send_to_char( ch, " &W2&w] La sinistra.\r\n" );
	send_to_char( ch, "Sceglila: " );
}


/*
 * Visualizza la lista per la scelta dell'archetipo
 */
void show_get_archetype( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "Scegli l'archetipo che pensi sia il piu' significativo per il carattere e la\r\n"
		"psicologia del tuo personaggio.\r\n" );
	send_to_char( ch, "L'archetipo ti puo' servire come linea guida per l'interpretazione GDR\r\n" );
	send_to_char( ch, "Se non vuoi scegliere ora l'archetipo digita &WNessuno&w, potrai sceglierlo quando\r\n" );
	ch_printf	( ch, "vorrai in gioco con il comando %s\r\n\r\n", translate_command(ch, "archetype") );

	ch_printf( ch, " 0] %-15s Scelta casuale dell'archetipo.\r\n", "Casuale" );
	show_archetype_list( ch );

	send_to_char( ch, "\r\nQuale archetipo preferisci? " );
}


/*
 * Visualizza il messaggio di scelta del linguaggio nel comando, richiamato in due punti dalla nanny.
 */
void show_want_language( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	write_to_buffer( d, ANSI_CLEAR, 4 );

	send_to_char( ch, "\r\nLe &Wlingue&w disponibili per l'invio dei comandi del gioco sono:\r\n" );
	send_to_char( ch, "&W1&w] Italiano (invierai i comandi come guarda, stato, chi, nord, comandi, config)\r\n" );
	send_to_char( ch, "&W2&w] Inglese (invierai i comandi come look, score, who, north, commands, config)\r\n" );
	send_to_char( ch, "Quale scegli? " );
}


/*
 * Controllo della validità della razza.
 */
bool check_race( CHAR_DATA *ch, const int x )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_race: ch è NULL" );
		return FALSE;
	}

	if ( x < 0 && x >= MAX_RACE )
		return FALSE;

	if ( !HAS_BIT(table_race[x]->flags, RACEFLAG_PLAYABLE) )
		return FALSE;

	if ( check_bans(ch, BAN_RACE) )
		 return FALSE;

	return TRUE;
}


/*
 * Controlla la validità del class.
 */
bool check_class( CHAR_DATA *ch, const int x )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_class: ch è NULL" );
		return FALSE;
	}

	if ( x < 0 && x >= MAX_CLASS )
		return FALSE;

	if ( !HAS_BIT(table_class[x]->flags, CLASSFLAG_PLAYABLE) )
		return FALSE;

	if ( check_bans(ch, BAN_CLASS) )
		return FALSE;

	return TRUE;
}


/*
 * Visualizza la schermata title post-login del mud.
 */
void show_title( DESCRIPTOR_DATA *d )
{
	CHAR_DATA *ch;

	ch = d->character;

	if ( HAS_BIT_PLR(ch, PLAYER_INTRO) )
		init_desc_ani( d );
	else
		show_file( d, TITLE_FILE, TRUE );

	d->connected = CON_PRESS_ENTER;
}


/*
 * Figured this belonged here seeing it involves players...
 * really simple little function to tax players with a large
 * amount of gold to help reduce the overall gold pool...

(FF) Cambiare anche questa funzione, fare in maniera che chi è cittadino
di una città debba pagare le tasse ogni mese, le tasse possono variare su
decisione di chi governa la città (mob o pg che sia)
Se il pg non deposita la tassa entro il mese sucessivo diventa un'evasore
e paga la multa in surplus se beccato dalle guardie (deve avere un po' di fama
per essere riconosciuto al volo)

 */
void tax_player( CHAR_DATA *ch )
{
	int		gold	= ch->gold;
	int		mgold	= get_level(ch) * 2000000;
	int		tax		= (int) (ch->gold * 0.05);

	if ( gold > mgold )
	{
		set_char_color( AT_WHITE, ch );
		ch_printf( ch, "You carry more than %d coins.\r\n", mgold );
		ch_printf( ch, "You are being taxed 5%% percent (%d coins) of your %d coins,\r\n", tax, ch->gold );
		ch_printf( ch, "and that leaves you with %d coins.\r\n", (ch->gold - tax) );
		ch->gold -= tax;
	}
}


/*
 * Calcola gli attributi del giocatore basandosi sul suo nome
 */
void compute_attrs( CHAR_DATA *ch )
{
	int x;
	int	a, b, c;

	for ( x = 0;  x < (int)strlen(ch->name);  x++ )
	{
		c = ch->name[x] + x;
		b = c % 30;
		a = (c % 1) + 1;

		/* Test scaramantico giusto per essere sicuri.. */
		if ( b >= 30 )
			send_log( NULL, LOG_BUG, "compute_attrs: b maggiore o uguale del doppio di %d", MAX_ATTR );

		if ( b < 15 )
			ch->attr_perm[b] = UMIN( DEF_MAX_ATTR, ch->attr_perm[b] + a );
		else
		{
			b -= 15;
			ch->attr_perm[b] = UMAX( DEF_MIN_ATTR, ch->attr_perm[b] - a );
		}
	}

	/* Sistema di generazione stat sulla base della classe scelta */
	for ( x = 0;  x < MAX_ATTR;  x++ )
	{
		send_log( NULL, LOG_RNDATTR, "compute_attrs: (%s) attributo %s prima della modifica di classe: %d",
			ch->name, code_name(NULL, x, CODE_ATTR), ch->attr_perm[x] );

		if ( table_class[ch->class]->avarage_attr[x] == 0 )
			continue;

		if ( ch->attr_perm[x] <= table_class[ch->class]->avarage_attr[x] )
			ch->attr_perm[x] += number_range( 5, 8 );
		else
			ch->attr_perm[x] -= number_range( 5, 8 );

		ch->attr_perm[x] = URANGE( DEF_MIN_ATTR, ch->attr_perm[x], DEF_MAX_ATTR );

		send_log( NULL, LOG_RNDATTR, "compute_attrs: (%s) attributo %s dopo la modifica di classe: %d",
			ch->name, code_name(NULL, x, CODE_ATTR), ch->attr_perm[x] );
	}
}


void enter_in_game( CHAR_DATA *ch, bool tutorial )
{
	char	buf[MSL];

	if ( tutorial )
		ch_printf( ch, "\r\nBenvenuto nel Tutorial di %s!\r\n\r\n", MUD_NAME );
	else
		ch_printf( ch, "\r\nBenvenuto nei Piani di %s!\r\n\r\n", MUD_NAME );

	LINK( ch, first_char, last_char, next, prev );
	LINK( ch, first_player, last_player, next_player, prev_player );

	set_char_color( AT_PLAIN, ch );
	/* Se il pg è stato appena creato lo rifinisce */
	if ( ch->level == 0 )
	{
/*		OBJ_DATA	*obj; */
		int			 x;

		ch->pg->clan			= NULL;
		ch->pg->creation_date	= current_time;
		ch->pg->practice        += get_curr_attr( ch, ATTR_INT ) / 16;

		for ( x = 0;  x < MAX_POINTS;  x++ )
		{
			ch->points_max[x]  = table_points[x].start;
			if ( table_race[ch->race]->points_limit[x] > 1000 )
				ch->points_max[x] += table_race[ch->race]->points_limit[x] / 1000;	/* + un millesimo del massimo */
			ch->points[x]	   = UMAX( 1, ch->points_max[x] );
			ch->points[x]	   = ch->points_max[x];
		}

		ch->level = 1;
		ch->exp = 0;

		/* Sceglie a caso se il pg ha potuto o no conoscere il proprio segno zodiacale */
		ch->pg->zodiac			= -1;				/* (FF) per ora non esistono */

		SET_BITS( ch->affected_by, table_race[ch->race]->affected );

		ch->armor			   += table_race[ch->race]->ac_plus;
		SET_BITS( ch->attacks, table_race[ch->race]->attacks );
		SET_BITS( ch->defenses, table_race[ch->race]->defenses );

		for ( x = 0;  x < MAX_SAVETHROW;  x++ )
			ch->saving_throw[x] = table_race[ch->race]->saving_throw[x];

/*		SET_BITS( ch->resist, table_race[ch->race]->resist ); */
/*		SET_BITS( ch->susceptible, table_race[ch->race]->suscept ); */

		/* (RR) da togliere appena viene fatto il sistema lingue correttamente */
		for ( x = 0;  x < MAX_LANGUAGE;  x++ )
		{
			if ( HAS_BIT(table_race[ch->race]->lang_natural, x) )
				ch->pg->learned_lang[x] = 100;
		}

		ch->gold = number_range( 10, 100 );

		reset_colors( ch );

		if ( ch->sex == SEX_FEMALE )
			set_title( ch, "la Viaggiatrice dei Piani" );
		else
			set_title( ch, "il Viaggiatore dei Piani" );

		/* Veste il pg con l'equip di base (TT) da togliere magari appena cambia un po' o lasciargli qualcosa */
		equip_newbieset( ch );
#ifdef T2_ALFA
		/* The object is the adventurer's guide, part of Academy.are. */
		if ( obj_ind )
		{
			obj = make_object( get_obj_proto(NULL, VNUM_OBJ_SCHOOL_GUIDE), 1 );
			obj_to_char( obj, ch );
			equip_char( ch, obj, WEARLOC_HOLD );
		}
#endif

		/* Piazza il pg nella scuola, poi se ha scelto il tutorial viene spostato in esso */
		char_to_room( ch, get_room_index(NULL, VNUM_ROOM_SCHOOL) );

		/* Salva subito il pg, se l'opzione lo permette */
		if ( get_level(ch) >= sysdata.save_level )
		{
			update_aris( ch );		/* Aggiorna gli affect e i RIS del pg */
			save_player( ch );
			add_offline( ch );
			saving_char = NULL;
		}

		if ( tutorial )
			char_to_room( ch, get_room_index(NULL, VNUM_ROOM_TUTORIAL) );
	}
	else if ( tutorial )
		char_to_room( ch, get_room_index(NULL, VNUM_ROOM_TUTORIAL) );
	else if ( !IS_ADMIN(ch)
	  && ch->pg->release_date > 0
	  && ch->pg->release_date > current_time )
	{
		if ( ch->in_room->vnum == 6			/* (RR) cambiare forse questi vnum in una VNUM_ROOM */
		  || ch->in_room->vnum == 8
		  || ch->in_room->vnum == 1206 )
		{
			char_to_room( ch, ch->in_room );
		}
		else
			char_to_room( ch, get_room_index(NULL, VNUM_ROOM_HELL) );
	}
	else if ( ch->in_room )
		char_to_room( ch, ch->in_room );
	else
		char_to_room( ch, get_room_index(NULL, VNUM_ROOM_TEMPLE) );

	if ( IS_ADMIN(ch) )
		add_ip_to_list( ch );

	ch->desc->connected = CON_PLAYING;

	if ( get_timer(ch, TIMER_SHOVEDRAG) > 0 )	remove_timer( ch, TIMER_SHOVEDRAG );
	if ( get_timer(ch, TIMER_KILLED)	> 0 )	remove_timer( ch, TIMER_KILLED );
	if ( get_timer(ch, TIMER_PKILLED)	> 0 )	remove_timer( ch, TIMER_PKILLED );

#ifdef T2_ALFA
	if ( HAS_BIT(ch->enflags, ENFLAG_FLYCHANGED) )
		REMOVE_BIT( ch->enflags, ENFLAG_FLYCHANGED );
#endif

	send_log( NULL, LOG_COMM, "enter_in_game: %s è entrat%c nel gioco alla stanza %s (#%d)",
		ch->name, gramm_ao(ch), ch->in_room->name, ch->in_room->vnum );

#ifdef T2_WEB
	create_html_webwho( );
#endif

	if ( HAS_BIT(ch->in_room->flags, ROOM_RENT) )
	{
		send_to_char( ch, "Riposato, esco dalla mia camera in affitto per nuove avventure!\r\n" );
		act( AT_ACTION, "$n esce da una camera in affitto, riposato.", ch, NULL, NULL, TO_ROOM );
		if ( ch->pg->mascotte )
		{
			act( AT_ACTION, "$N si sveglia e mi segue.", ch, NULL, ch->pg->mascotte, TO_CHAR );
			act( AT_ACTION, "$n si sveglia e segue il suo padrone.", ch->pg->mascotte, NULL, ch, TO_NOVICT );
		}
	}
	else
	{
		if ( VALID_STR(ch->pg->msg_login_you) )
			ch_printf( ch, "%s\r\n", ch->pg->msg_login_you );
		else
			send_to_char( ch, "Un piccolo portale si forma e giungo nelle terre dominate dal potere Astrale.\r\n" );

		if ( VALID_STR(ch->pg->msg_login_room) )
			strcpy( buf, ch->pg->msg_login_room );
		else
		{
			sprintf( buf, "Un piccolo portale si forma ed un%s dei Piani giunge.",
				(ch->sex == SEX_FEMALE) ? "a Viaggiatrice" : " Viaggiatore" );	/* (MPROG) */
		}
		act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );

		if ( ch->pg->mascotte )
		{
			act( AT_ACTION, "$n torna dal padrone.", ch->pg->mascotte, NULL, ch, TO_NOVICT );
			act( AT_ACTION, "$N torna da me.", ch, NULL, ch->pg->mascotte, TO_CHAR );
		}

		/* Probabilità di essere derubati dei soldi se non si ha affittato una camera */
		if ( number_range(0, 100) == 0 )
		{
			send_to_char( ch, "Porto la mano sul borsello sentendomelo più leggero del solito.. non c'è più!\r\n" );
			send_to_char( ch, "Mi hanno derubato nel sonno! La prossima volta forse è megli oche mi affitti una camera.\r\n" );
			ch->gold = 0;
		}
	}

	send_command( ch, "look", CO );

	tax_player( ch );		/* (FF) Da modificare. Here we go, let's tax players to lower the gold pool */

	note_count( ch, friendly_ctime_nohour(&current_time), NOTE_NOTE );
	note_count( ch, friendly_ctime_nohour(&current_time), NOTE_MAIL );
	note_count( ch, friendly_ctime_nohour(&current_time), NOTE_BOARD );

	if ( !ch->was_in_room && ch->in_room == get_room_index(NULL, VNUM_ROOM_TEMPLE) )
		ch->was_in_room = get_room_index( NULL, VNUM_ROOM_TEMPLE );
	else if ( ch->was_in_room == get_room_index(NULL, VNUM_ROOM_TEMPLE) )
		ch->was_in_room = get_room_index( NULL, VNUM_ROOM_TEMPLE );
	else if ( !ch->was_in_room )
		ch->was_in_room = ch->in_room;
}


/*
 * Parse a name for acceptability
 * Names checking should really only be done on new characters, otherwise
 * we could end up with people who can't access their characters.
 */
bool check_parse_name( char *name, bool newchar )
{
	char *pc;
	bool fIll;

	if ( is_reserved_name(name) && newchar )
		return FALSE;

	/* Length restrictions */
	if ( strlen(name) <  MIN_LEN_NAME_PG )
		return FALSE;

	if ( strlen(name) > MAX_LEN_NAME_PG )
		return FALSE;

	/* Alphanumerics only.
	 * Lock out IllIll twits */
	fIll = TRUE;
	for ( pc = name;  VALID_STR(pc);  pc++ )
	{
		if ( !isalpha(*pc) )
			return FALSE;
		if ( tolower(*pc) != 'i' && tolower(*pc) != 'l' )
			fIll = FALSE;
	}

	if ( fIll )
		return FALSE;

	return TRUE;
}


/*
 * Carica da un file pg solo la password per confrontarla con quella inserita dall'utente
 */
bool load_password( DESCRIPTOR_DATA *d, char *name )
{
	MUD_FILE *fp;

	DISPOSE( d->password );
	DISPOSE( d->name );

	d->name = str_dup( name );

	fp = mud_fopen( PLAYER_DIR, capitalize(name), "r", FALSE );
	if ( !fp )
		return FALSE;

	for ( ; ; )
	{
		char *word;

		word = fread_word( fp );

		if ( feof(fp->file) )
			break;
		if ( !strcmp(word, "#END") )
			break;
		
		if ( !strcmp(word, "Password") )
		{
			d->password = str_dup( fread_string(fp) );
			MUD_FCLOSE( fp );
			return TRUE;
		}

		fread_to_eol( fp );
	}

	send_log( fp, LOG_BUG, "load_password: trovato file player %s senza password", name );
	return FALSE;
}


/*
 * Controlla se il pg si debba riconnettere oppure no
 * Riconnette nello stesso stato precedente
 */
bool check_reconnect( DESCRIPTOR_DATA *d )
{
	DESCRIPTOR_DATA *dold;
	CHAR_DATA		*ldch;

	for ( dold = first_descriptor;  dold;  dold = dold->next )
	{
		if ( !dold->character )
			continue;
		if ( str_cmp(d->name, dold->character->name) )
			continue;

		if ( dold->connected == CON_MENU || dold->connected == CON_PRESS_ENTER )
		{
			write_to_buffer( d, "Già connesso... Chiusura della precedente connessione.\r\n", 0 );
			write_to_buffer( d, echo_on_str, 0 );
			d->character		= dold->character;
			d->character->desc	= d;
			d->character->timer	= 0;
			DISPOSE( d->character->pg->host );
			d->character->pg->host = str_dup( d->host );

			dold->character = NULL;

			write_to_buffer( dold, "La connessione precedente è stata chiusa.\r\n", 0 );
			close_socket( dold, FALSE );

			enter_in_game( d->character, FALSE );
			return TRUE;
		}
	}

	for ( ldch = first_player;  ldch;  ldch = ldch->next_player )
	{
		if ( !ldch->pg )
			continue;

		if ( !str_cmp(d->name, ldch->name) )
		{
			write_to_buffer( d, "Già connesso... Chiusura della precedente connessione.\r\n", 0 );
			write_to_buffer( d, echo_on_str, 0 );
			d->character	= ldch;
			ldch->timer		= 0;
			d->olc			= NULL;
			d->snoop_by		= NULL;
			d->connected	= CON_PLAYING;
			DISPOSE( d->character->pg->host );
			d->character->pg->host = str_dup( d->host );

			if ( ldch->desc )
			{
				ldch->desc->character	= NULL;
				d->olc					= ldch->desc->olc;
				d->snoop_by				= ldch->desc->snoop_by;
				ldch->desc->olc			= NULL;
				ldch->desc->snoop_by	= NULL;
				d->connected			= ldch->desc->connected;
				write_to_buffer( ldch->desc, "La connessione precedente è stata chiusa.\r\n", 0 );
				close_socket( ldch->desc, FALSE );
			}

			ldch->desc = d;
			if ( ldch->switched )
				send_command( ldch->switched, "return", CO );

			send_to_char( ldch, "Riconnessione.\r\n" );

			switch ( d->connected )
			{
			  default:
				close_socket( d, FALSE );
				send_log( NULL, LOG_BUG, "check_reconnect: tipo di connessione non inserita o errata: %d", d->connected );
				break;
			  case CON_PLAYING:
				cleanup_olc( ldch->desc );
				send_command( ldch, "look", CO );
				act( AT_ACTION, "Un Gorgo Astrale si forma di colpo, da esso $n sbuca entrando in questo Piano.", ldch, NULL, NULL, TO_ROOM );
				send_log( NULL, LOG_COMM, "%s@%s si è riconness%c alla stanza %s (#%d)",
					ldch->name, d->host, gramm_ao(ldch), ldch->in_room->name, ldch->in_room->vnum );
				break;
			  case CON_EDITING:
				editor_header( ldch );
				set_char_color( AT_PLAIN, ldch );
				edit_buffer( ldch, "\\l" );
				break;
			  case CON_MEDIT:
				medit_parse( ldch->desc, "" );
				break;
			  case CON_OEDIT:
				oedit_parse( ldch->desc, "" );
				break;
			  case CON_REDIT:
				redit_parse( ldch->desc, "" );
				break;
			}

			return TRUE;
		} /* chiusura if */
	} /* chiusura for */

	return FALSE;
}


/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
	CHAR_DATA	*ch;
	char		 buf[MSL];
	int			 x;

	ch = d->character;

	switch ( d->connected )
	{
	  default:
		send_log( NULL, LOG_BUG, "nanny: d->connected %d errato.", d->connected );
		close_socket( d, TRUE );
		return;

	  case CON_OEDIT:
		oedit_parse( d, argument );
		return;

	  case CON_REDIT:
		redit_parse( d, argument );
		return;

	  case CON_MEDIT:
		medit_parse( d, argument );
		return;

	  case CON_GET_NAME:
	  {
		if ( !VALID_STR(argument) )
		{
			close_socket( d, FALSE );
			return;
		}

		/* Old players can keep their characters. */
		if ( !check_parse_name(argument, TRUE) )
		{
			write_to_buffer( d,
				"Non puoi usare questo nome per interpretare il tuo personaggio, prova con un altro.\r\n"
				"  Ricordati che stai per entrare in un mondo dai tratti medievali e  fantastici.\r\n"
				"  Trova il modo quindi di inventarti un nome che sia concettualmente affine al\r\n"
				"  canone fantasy e che segua semplici regole di buon senso.\r\n"
				"  Se il nome che hai deciso sara' inaccettabile, assurdo o offensivo ti verra'\r\n"
				"  chiesto di cambiarlo.\r\n"
				"\r\nNome: ", 0 );
			return;
		}

		argument[0] = toupper( argument[0] );

		if ( load_password(d, argument) )
		{
			/* Old player */

			/* Make sure the admin host is from the correct place. */
			if ( sysdata.check_adm_host && VALID_STR(d->host) && !check_adm_domain(argument , d->host) )
			{
				write_to_buffer( d, "Questo tentativo da lamer è stato registrato.\r\n", 0 );
				send_log( NULL, LOG_COMM, "%s ha cercato di utilizzare l'Admin %s", d->host, argument );
				close_socket( d, FALSE );
				return;
			}

			write_to_buffer( d, "Parola chiave: ", 0 );
			write_to_buffer( d, echo_off_str, 0 );

			d->tempnum = 0;
			d->connected = CON_GET_PASSWORD;
			return;
		}
		else
		{
			/* Don't allow new players if deny_new_players is true */
/*			if ( sysdata.deny_new_players )
			{
				write_to_buffer( d, "Il Mud si sta organizzando per il riavvio."
					"\r\nNuovi giocatori non sono accettati in questo momento."
					"\r\nRiprova tra qualche tempo.\r\n", 0 );
				close_socket( d, FALSE );
				return;
			}*/

			CREATE( d->character, CHAR_DATA, 1 );
			init_char( d->character, FALSE );
			d->character->desc = d;
			CREATE( d->character->pg, PG_DATA, 1 );
			init_pg( d->character->pg );
			d->character->name		= str_dup( capitalize(argument) );
			d->character->pg->host	= str_dup( d->host );

			/* No such player */
			write_to_buffer( d,
				"\r\nNon esiste nessun avventuriero con tale nome.\r\n"
				"  Ricordati che stai per entrare in un mondo dai tratti medievali e  fantastici.\r\n"
				"  Trova il modo quindi di inventarti un nome che sia concettualmente affine al\r\n"
				"  canone fantasy e che segua semplici regole di buon senso.\r\n"
				"  Se il nome che hai deciso sara' inaccettabile, assurdo o offensivo ti verra'\r\n"
				"  chiesto di cambiarlo.\r\n\r\n", 0 );
			show_get_name( d, argument );
			d->connected = CON_NEW_GET_NAME;
		}
		return;
	  }	/* chiude CON_GET_NAME */


	  case CON_GET_PASSWORD:
	  {
		write_to_buffer( d, "\r\n", 2 );

		if ( !VALID_STR(argument) )
		{
			write_to_buffer( d, "Parola chiave errata.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

#ifdef NOCRYPT
		if ( strcmp(cryptmd5(argument), d->password) )
#else
		if ( strcmp(cryptmd5(argument), d->password+1) )
#endif
		{
			write_to_buffer( d, "Parola chiave errata.\r\n\r\n", 0 );
			send_log( NULL, LOG_WARNING, "nanny: parola di chiave errata per %s", d->name );

			/* Dopo tre tentativi esce */
			if ( d->tempnum >= 2 )
			{
				/* Clear descriptor pointer to get rid of bug message in log */
				if (d->character)
					d->character->desc = NULL;
				
				close_socket( d, FALSE );
			}
			else
			{
				write_to_buffer( d, "Parola chiave: ", 0 );
				write_to_buffer( d, echo_off_str, 0 );
				d->tempnum++;
				d->connected = CON_GET_PASSWORD;
			}
			return;
		}

		if ( check_reconnect(d) )
			return;
		else
			load_player( d, d->name, FALSE );

		if ( !d->character )
		{
			send_log( NULL, LOG_NORMAL, "File del giocatore corrotto %s@%s.", argument, d->host );
			write_to_buffer( d, "Non è stato possibile caricare il tuo personaggio.. Notificalo agli Amministratori del Mud.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

		ch = d->character;

		/* Controlla che il sito da cui si connette il giocatore non sia bannato */
		if ( check_bans(ch, BAN_SITE) )
		{
			write_to_buffer( d, "Il tuo sito è stato esiliato da questo Mud.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

		/* Se il file esiste controlla i ban di class e di razza */
		if ( check_bans(ch, BAN_CLASS) )
		{
			write_to_buffer( d, "La tua classe è stata esiliata da questo Mud.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

		if ( check_bans(ch, BAN_RACE) )
		{
			write_to_buffer( d, "La tua razza è stata esiliata da questo Mud.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

		/* Controlla se sia negato l'accesso al giocatore */
		if ( HAS_BIT_PLR(ch, PLAYER_DENY) )
		{
			send_log( NULL, LOG_COMM, "Accesso negato a %s@%s.", argument, d->host );
			write_to_buffer( d, "Ti è negato l'accesso.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

		if ( !IS_ADMIN(ch) && sysdata.wiz_lock )
		{
			write_to_buffer( d, "Il Mud è chiuso ai giocatori. Solo gli implementatori posso connettersi ora.\r\n"
				"Riprova tra qualche tempo.\r\n", 0 );
			close_socket( d, FALSE );
			return;
		}

#ifdef T2_MXP
		/* Negoziazione telnet per vedere se supporta l'MXP */
		write_to_buffer( d, (char *) will_mxp_str, 0 );
#endif

#ifdef T2_DETCLIENT
		/* telnet negotiation asking what their client is */
		write_to_buffer( d, do_termtype_str, 0 );
#endif

		d->tempnum = 0;	/* Reinizializza la variabile temporanea di ch per il conteggio dei tentativi di inserimento password */

		write_to_buffer( d, echo_on_str, 0 );

		if ( ch->position == POSITION_FIGHT
		  || ch->position == POSITION_EVASIVE
		  || ch->position == POSITION_DEFENSIVE
		  || ch->position == POSITION_AGGRESSIVE
		  || ch->position == POSITION_BERSERK )
		{
			ch->position = POSITION_STAND;
		}

		send_log( NULL, LOG_COMM, "%s@%s si è conness%c.", ch->name, d->host, gramm_ao(ch) );

		show_title( d );
		return;
	  }	/* chiude CON_GET_PASSWORD */


	  case CON_NEW_GET_NAME:
	  {
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "No, decidi tra &W1&w, &W2&w, &W3&w o &W4&w.\r\n" );
			return;
		}

	  	if ( !str_cmp(argument, "1") || !str_prefix(argument, "si") )
	  	{
	  		write_to_buffer( d, "In gioco poi potrai decidere il cognome del tuo personaggio con il comando:\r\n", 0 );
	  		send_to_char( d->character, "&Gcognome &w/ &Gsurname&w\r\n" );

			sprintf( buf, "\r\nAssicurati che la tua parola chiave sia difficilmente intuibile da chicchessia."
				"\r\nCrea una buona parola chiave per %s: %s", ch->name, echo_off_str );
			write_to_buffer( d, buf, 0 );
			d->connected = CON_NEW_GET_PASSWORD;
		}
	  	else if ( !str_cmp(argument, "2") || !str_prefix(argument, "no") )
	  	{
			write_to_buffer( d, "Va bene, allora come? ", 0 );
			/* clear descriptor pointer to get rid of bug message in log */
			d->character->desc = NULL;
			free_char( d->character );
			d->character = NULL;
			d->connected = CON_GET_NAME;
	  	}
	  	else
			send_to_char( ch, "No, decidi tra &W1&w o &W2&w.\r\n" );
		return;
	  }	/* chiude CON_NEW_CONFIRM_NAME */


	  case CON_NEW_GET_PASSWORD:
	  {
		char	*pwdnew;

		write_to_buffer( d, "\r\n", 2 );

		/* Se non passa il controllo sulla password la richiede */
		pwdnew = check_password( ch, argument );
		if ( !pwdnew )
		{
			write_to_buffer( d, "Dimmi ordunque la tua parola chiave: ", 0 );
			return;
		}

		DISPOSE( ch->pg->pwd );
		ch->pg->pwd	= str_dup( pwdnew );

		write_to_buffer( d, "Ridimmi la parola chiave per conferma: ", 0 );
		d->connected = CON_NEW_CONFIRM_PASSWORD;
		return;
	  }	/* chiude CON_NEW_GET_PASSWORD */


	  case CON_NEW_CONFIRM_PASSWORD:
	  {
		write_to_buffer( d, "\r\n", 2 );

#ifdef NOCRYPT
		if ( strcmp(cryptmd5(argument), ch->pg->pwd) )
#else
		if ( strcmp(cryptmd5(argument), ch->pg->pwd+1) )
#endif
		{
			write_to_buffer( d, "\r\nLa parola chiave non corrisponde.\r\nRidimmi la tua parola chiave: ", 0 );
			d->connected = CON_NEW_GET_PASSWORD;
			return;
		}
#ifdef T2_MSP
		send_audio( d, "creation.mid", TO_CHAR );
#endif
		write_to_buffer( d, echo_on_str, 0 );
		/* Invia quindi la frase per la scelta del sesso */
		show_want_sex( d );

		d->connected = CON_NEW_WANT_SEX;
		return;
	  }	/* chiude CON_NEW_CONFIRM_PASSWORD */


	  case CON_NEW_WANT_SEX:
	  {
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "No, scrivi &W0&w, &W1&w o &W2&w.\r\n" );
			return;
		}

	  	if ( !str_cmp(argument, "0") )
	  	{
	  		ch->sex = number_range( SEX_MALE, SEX_FEMALE );
	  	}
	  	else if ( !str_cmp(argument, "1") || is_name_prefix(argument, "maschile maschio") )
	  	{
	  		ch->sex = SEX_MALE;
	  	}
	  	else if ( !str_cmp(argument, "2") || is_name_prefix(argument, "femminile femmina") )
	  	{
	  		ch->sex = SEX_FEMALE;
	  	}
	  	else
	  	{
			send_to_char( ch, "Non e' una scelta idonea..\r\n" );
			send_to_char( ch, "Quale e' il genere che vuoi attribuire al tuo personaggio? &W1&w o &W2&w? " );
			return;
		}

		show_get_race( d );
		d->connected = CON_NEW_GET_RACE;
		return;
	  }	/* chiude CON_NEW_WANT_SEX */


	  case CON_NEW_GET_RACE:
	  {
#ifdef T2_ALFA
		if ( is_name(argument, "aiuto help ?") )
		{
			if ( is_name_race(argument, TRUE) )
			{
				do_help( ch, argument );
				send_to_char( ch, "\r\nScegli la razza che vorresti per il tuo personaggio: " );
				return;
			}

			send_to_char( ch, "\r\nNessun aiuto per questo argomento.\r\n" );
			send_to_char( ch, "Scegli la razza che vorresti per il tuo personaggio: " );
			return;
		}
#endif
		if ( is_number(argument) )
		{
			x = atoi( argument );
			x--;	/* Diminuisce di uno il valore passato */
		}
		else
		{
			send_to_char( ch, "Digita il numero corrispettivo alla razza che vorresti per il tuo personaggio.\r\n" );
			return;
		}

		if ( x == -1 )		/* Il pg ha scelto l'opzione casuale 0 */
		{
			do
			{
				x = number_range( 0, MAX_RACE-1 );
			} while ( !check_race(ch , x) );	/* (TT) */

			ch->race = x;
		}
		else if ( x >= 0 && x < MAX_RACE && check_race(ch , x) )
		{
			ch->race = x;
		}
		else
		{
			send_to_char( ch,
				"Questa razza non e' disponibile.\r\n"
				"Quale altra razza vorresti interpretare? " );
			return;
		}

		show_get_alignment( d );
		d->connected = CON_NEW_GET_ALIGNMENT;
		return;
	  }	/* chiude CON_NEW_GET_RACE */


	  case CON_NEW_GET_ALIGNMENT:
	  {
		/* Imposta l'allineamento razziale di default */
		ch->alignment = table_race[ch->race]->align_start;

		if ( is_number(argument) )
			x = atoi( argument );
		else
		{
			send_to_char( ch, "Digita il numero del tipo di allineamento che vorresti per il tuo personaggio.\r\n" );
			return;
		}

		switch ( x )
		{
		  default:
			send_to_char( ch, "Questa non e' una opzione valida: scegli &W0&w, &W1&w, &W2&w o &W3&w? " );
			return;

		  case 0:
			x = number_range( -101, 101 );
			ch->alignment += x;
			break;

		  case 1:
			ch->alignment -= 101;
			if ( ch->alignment < -1000 )
				ch->alignment = -1000;
			break;

		  case 2:
			break;

		  case 3:
			ch->alignment += 101;
			if ( ch->alignment > 1000 )
				ch->alignment = 1000;
			break;
		}

#ifdef T2_QUESTION
		if ( sysdata.question_creation )
		{
			show_want_question( d );
			d->connected = CON_NEW_WANT_QUESTION;
		}
		else
#endif
		{
			show_get_class( d );
			d->connected = CON_NEW_GET_CLASS;
		}
		break;
	  }	/* chiude CON_NEW_GET_ALIGNMENT */

#ifdef T2_QUESTION
	  case CON_NEW_WANT_QUESTION:
	  {
		if ( is_number(argument) )
		  	x = atoi( argument );
		else
		{
			send_to_char( ch, "Digita 1 o 2.\r\n" );
			return;
		}

		switch ( x )
		{
		  default:
			send_to_char( ch, "Questa non e' una risposta valida: scegli &W1&w o &W2&w? " );
			return;

		  case 1:
			show_get_class( d );
			d->connected = CON_NEW_GET_CLASS;
			break;

		  case 2:
			show_get_question( d );
			d->connected = CON_NEW_GET_QUESTION;
			break;
		}
		return;
	  }	/* chiude CON_NEW_WANT_QUESTION */


	  case CON_NEW_GET_QUESTION:
	  {
		/* Controlla la risposta */
		if ( !check_get_question(d, argument) )
			return;

	  	/* Controlla se debba inviare un'altra domanda */
		if ( get_another_question(d) )
			return;

		/* Acquisisce la classe controllando le risposte date */
		ch->class = answers_elaboration( d );
		if ( ch->class == CLASS_NONE )
		{
			ch_printf( ch, "Riscontrati degli errori, avvisare i Coder di %s, riprovare la creazione del personaggio.\r\n", MUD_NAME );
			close_socket( ch->desc, FALSE );
			return;
		}

		/* Visualizza la domanda sul tipo di capelli */
		show_get_weight( d );
		d->connected = CON_NEW_GET_WEIGHT;
		break;
	  } /* chiude CON_NEW_GET_QUESTION */
#endif

	  case CON_NEW_GET_CLASS:
	  {
		char	arg[MIL];

		argument = one_argument( argument, arg );
		if ( is_name(arg, "aiuto help ?") )
		{
			if ( is_name_class(argument, TRUE) )
			{
				do_help( ch, argument );
				send_to_char( ch, "\r\nScegli la classe che vorresti per il tuo personaggio: " );
				return;
			}

			send_to_char( ch, "\r\nNessun aiuto per questo argomento.\r\n" );
			send_to_char( ch, "Scegli la classe che vorresti per il tuo personaggio: " );
			return;
		}

		if ( is_number(arg) )
		{
			x = atoi( arg );
			x--;	/* Diminuisce di uno */
		}
		else
		{
			send_to_char( ch, "Digita il numero corrispondente alla classe che vorresti per il tuo personaggio.\r\n" );
			return;
		}

		if ( x == -1 )	/* il pg ha scelto lo 0, la scelta casuale della classe */
		{
			do
			{
				x = number_range( 0, MAX_CLASS-1 );
			} while ( !check_class(ch, x) );	/* (TT) */

			ch->class = x;
		}
		else if ( x >= 0 && x < MAX_CLASS && check_class(ch, x) )
		{
			ch->class = x;
		}
		else
		{
			send_to_char( ch, "Scelta non disponibile.\r\n" );
			send_to_char( ch, "Quale &Wclasse&w vorresti intraprendere? " );
			return;
		}

		/* Visualizza la domanda sul peso voluto */
		show_get_weight( d );
		d->connected = CON_NEW_GET_WEIGHT;
		return;
	  }	/* chiude CON_NEW_GET_CLASS */


	  case CON_NEW_GET_WEIGHT:
	  {
		/* Imposta il peso razziale di default */
		ch->weight = table_race[ch->race]->weight;

		if ( is_number(argument) )
			x = atoi( argument );
		else
		{
			send_to_char( ch, "Digita il numero del tipo di peso che vorresti per il tuo personaggio.\r\n" );
			return;
		}

		switch ( x )
		{
		  default:
			send_to_char( ch, "Questa non e' una opzione valida: scegli &W0&w, &W1&w, &W2&w o &W3&w? " );
			return;

		  case 0:
			ch->weight = number_range( table_race[ch->race]->weight * (1.00 - MAX_FACTOR_WEIGHT/2),
									   table_race[ch->race]->weight * (1.00 + MAX_FACTOR_WEIGHT/2) );
			break;

		  case 1:
			ch->weight = number_range( table_race[ch->race]->weight * (1.00 - MAX_FACTOR_WEIGHT/2),
									   table_race[ch->race]->weight );
			break;

		  case 2:
			ch->weight = number_range( table_race[ch->race]->weight * (1.00 - MAX_FACTOR_WEIGHT/4),
									   table_race[ch->race]->weight * (1.00 + MAX_FACTOR_WEIGHT/4) );
			break;

		  case 3:
			ch->weight = number_range( table_race[ch->race]->weight,
									   table_race[ch->race]->weight * (1.00 + MAX_FACTOR_WEIGHT/2) );
			break;
		}

		/* Visualizza la domanda sull'altezza */
		show_get_height( d );
		d->connected = CON_NEW_GET_HEIGHT;
		return;
	  }	/* chiude CON_NEW_GET_WEIGHT */


	  case CON_NEW_GET_HEIGHT:
	  {
		/* Imposta l'altezza razziale di default */
		ch->height = table_race[ch->race]->height;

		if ( is_number(argument) )
			x = atoi( argument );
		else
		{
			send_to_char( ch, "Digita il numero del tipo di altezza che vorresti per il tuo personaggio.\r\n" );
			return;
		}

		switch ( x )
		{
		  default:
			send_to_char( ch, "Questa non e' una opzione valida: scegli &W0&w, &W1&w, &W2&w o &W3&w? " );
			return;

		  case 0:
			ch->height = number_range( table_race[ch->race]->height * (1.00 - MAX_FACTOR_HEIGHT/2),
									   table_race[ch->race]->height * (1.00 + MAX_FACTOR_HEIGHT/2) );
			break;

		  case 1:
			ch->height = number_range( table_race[ch->race]->height * (1.00 - MAX_FACTOR_HEIGHT/2),
									   table_race[ch->race]->height );
			break;

		  case 2:
			ch->height = number_range( table_race[ch->race]->height * (1.00 - MAX_FACTOR_HEIGHT/4),
									   table_race[ch->race]->height * (1.00 + MAX_FACTOR_HEIGHT/4) );
			break;

		  case 3:
			ch->height = number_range( table_race[ch->race]->height,
									   table_race[ch->race]->height * (1.00 + MAX_FACTOR_HEIGHT/2) );
			break;
		}

		/* Visualizza la domanda sul tipo di capelli */
		show_get_hairtype( d );
		d->connected = CON_NEW_GET_HAIRTYPE;
		return;
	  }	/* chiude CON_NEW_GET_HEIGHT */


	  case CON_NEW_GET_HAIRTYPE:
	  {
		if ( is_number(argument) )
		{
			x = atoi( argument );
			x--;	/* Diminuisce di uno */
		}
		else
		{
			send_to_char( ch, "Digita il numero corrispettivo del tipo di capelli voluto per il proprio personaggio: " );
			return;
		}

		if ( x >= -1 && x < MAX_HAIRTYPE )
		{
			if ( x == -1 )	/* Scelta casuale */
				x = number_range( 0, MAX_HAIRTYPE-1 );

			/* Passa la scelta riguardo il tipo di capelli */
			ch->pg->hair->type->cat = x;
			DISPOSE( ch->pg->hair->type->syn );
			ch->pg->hair->type->syn = str_dup( code_name(NULL, x, CODE_HAIRTYPE) );
		}
		else
		{
			send_to_char( ch, "Scelta errata, quale e' il &Wtipo di capelli&w che vorresti attribuire al tuo personaggio? " );
			return;
		}

		/* Se calvo chiede il colore degli occhi altrimenti quello dei capelli */
		if ( ch->pg->hair->type->cat == HAIRTYPE_BALD )
		{
			ch->pg->hair->color->cat = AT_NONE;
			DISPOSE( ch->pg->hair->color->syn );
			ch->pg->hair->color->syn = str_dup( code_name(NULL, AT_NONE, CODE_COLOR) );

			show_get_color( d, "occhi", CODE_COLOR );
			d->connected = CON_NEW_GET_EYECOLOR;
		}
		else
		{
			show_get_color( d, "capelli", CODE_COLORHAIR );
			d->connected = CON_NEW_GET_HAIRCOLOR;
		}
		return;
	  }	/* case CON_NEW_GET_HAIRTYPE */


	  case CON_NEW_GET_HAIRCOLOR:
	  {
		if ( is_number(argument) )
		{
			x = atoi( argument );
			x--;	/* Diminuisce di uno */
		}
		else
		{
			send_to_char( ch, "Digita il numero corrispettivo del colore di capelli voluto per il proprio personaggio: " );
			return;
		}

		if ( x >= -1 && x < MAX_COLOR )
		{
			if ( x == -1 )	/* Scelta casuale */
				x = number_range( 0, MAX_COLOR-1 );

			/* Passa le scelte del pg alla struttura dei capelli inizializzata alla scelta del tipo di capelli */
			ch->pg->hair->color->cat = x;
			DISPOSE( ch->pg->hair->color->syn );
			ch->pg->hair->color->syn = str_dup( code_name(NULL, x, CODE_COLOR) );
		}
		else
		{
			send_to_char( ch, "Colore di capelli errato, quale e' il &Wcolore di capelli&w che vorresti per il tuo personaggio?.\r\n" );
			return;
		}

		/* Visualizza la domanda sulla lunghezza dei capelli */
		show_get_hairlenght( d );
		d->connected = CON_NEW_GET_HAIRLENGTH;
		return;
	  }	/* chiude CON_NEW_GET_HAIRCOLOR */

	  case CON_NEW_GET_HAIRLENGTH:
	  {
		if ( is_number(argument) )
		{
			x = atoi( argument );
		}
		else
		{
			send_to_char( ch, "Digita un numero indicante la lunghezza in centimetri dei capelli: " );
			return;
		}

		switch ( x )
		{
		  default:
			if ( x < 0 && x > racial_hairlen(ch->race) )
			{
				ch_printf( ch, "Lunghezza capelli errata, lunghezza valida tra 1 e %d centimetri: ", racial_hairlen(ch->race) );
				return;	/* Ed esce */
			}
			else
				ch->pg->hair->length = x;
			break;

		  case 0:
			ch->pg->hair->length = number_range( 1, racial_hairlen(ch->race) );
			break;
		}


		/* Visualizza la domanda sul colore degli occhi */
		show_get_color( d, "occhi", CODE_COLOR );
		d->connected = CON_NEW_GET_EYECOLOR;
		return;
	  }	/* chiude CON_NEW_GET_HAIRLENGTH */


	  case CON_NEW_GET_EYECOLOR:
	  {
		if ( is_number(argument) )
		{
			x = atoi( argument );
			x--;	/* Diminuisce di uno */
		}
		else
		{
			send_to_char( ch, "Digita il numero corrispettivo del colore di occhi voluto per il proprio personaggio: " );
			return;
		}

		if ( x >= -1 && x < MAX_COLOR )
		{
			if ( x == -1 )
				x = number_range( 0, MAX_COLOR-1 );

			/* E poi passa la scelta del pg */
			ch->pg->eye->color->cat = x;
			DISPOSE( ch->pg->eye->color->syn );
			ch->pg->eye->color->syn = str_dup( code_name(NULL, x, CODE_COLOR) );
		}
		else
		{
			send_to_char( ch, "Colore occhi errato, quale e' il &Wcolore degli occhi&w che vorresti per il tuo personaggio? " );
			return;
		}

		/* Visualizza lo schema per il rerolling delle stat */
		compute_attrs( ch );	/* Calcola gli attributi prima del rerolling  */
		show_reroll_attrs( d );

#ifdef T2_ALFA
		d->connected = CON_NEW_GET_SKIN;
#endif
		d->connected = CON_NEW_REROLL_ATTRS;

		return;
	  }	/* CON_NEW_GET_EYECOLOR */


#ifdef T2_ALFA
	  case CON_NEW_GET_SKINCOLOR:
	  {

		return;
	  }	/* chiude CON_NEW_GET_SKINCOLOR */
#endif

	  case CON_NEW_REROLL_ATTRS:
	  {
		char	arg[MIL];
		bool	found = FALSE;

		argument = one_argument( argument, arg );
		if ( is_name(arg, "aiuto help ?") )
		{
			for ( x = 0;  x < MAX_ATTR;  x++ )
			{
				if ( !str_prefix_acc(argument, code_name(NULL, x, CODE_ATTR))	/* Per gestire al meglio gli attributi accentati */
				  || !str_prefix	(argument, "attributi") )
				{
					do_help( ch, argument );
					send_to_char( ch, "\r\nQuale attributo vuoi diminuire o aumentare? " );
					return;
				}
			}

			send_to_char( ch, "\r\nNessun aiuto per questo argomento.\r\n" );
			send_to_char( ch, "Quale attributo vuoi diminuire o aumentare? " );
			return;
		}

		if ( !str_cmp(arg, "continua") )
		{
			if ( ch->desc->reroll != 0 )
			{
				send_to_char( ch, "Non hai ancora utilizzato tutte le tue crocette... spendile prima!\r\n" );
				return;
			}

			/* Visualizza la domanda su quale sia la mano principale del personaggio */
			show_want_hand( d );
			d->connected = CON_NEW_WANT_HAND;
			return;
		}

		for ( x = 0;  x < MAX_ATTR;  x++ )
		{
			if ( strlen(argument) != 1 )
				break;

			if ( !str_prefix_acc(arg, code_name(NULL, x, CODE_ATTR)) )	/* Per gestire al meglio gli attributi accentati */
			{
				if ( argument[0] == '+' && ch->desc->reroll > 0 && ch->attr_perm[x] < DEF_MAX_ATTR )
				{
					found = TRUE;
					ch->attr_perm[x] += 5;
					ch->desc->reroll--;
					break;
				}
				else if ( argument[0] == '-' && ch->attr_perm[x] > DEF_MIN_ATTR )
				{
					found = TRUE;
					ch->attr_perm[x] -= 5;
					ch->desc->reroll++;
					break;
				}
			}
		}

		show_reroll_attrs( d );

		if ( found == FALSE )
			send_to_char( ch, "Scrivi l'attributo che vorretti aumentare o diminuire seguito da + o -\r\n" );

		return;
	  } /* chiude CON_NEW_ROLL_ATTRS */

	  case CON_NEW_WANT_HAND:
	  {
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Mano errata, scegli &W1&w o &W2&w?" );
			return;
		}

	  	if ( !str_cmp(argument, "0") )
	  	{
	  		if ( number_range(0, 1) )
	  			ch->right_hand = TRUE;
	  		else
	  			ch->right_hand = FALSE;
	  	}
	  	else if	( !str_cmp(argument, "1") || !str_prefix(argument, "destra") )
		{
			ch->right_hand = TRUE;
		}
		else if ( !str_cmp(argument, "2") || !str_prefix(argument, "sinistra") )
		{
			ch->right_hand = FALSE;
		}
		else
		{
			send_to_char( ch, "Mano errata, scegli &W1&w o &W2&w?" );
			return;
		}

		/* Domanda quale archetipo si voglia */
		show_get_archetype( d );
		d->connected = CON_NEW_GET_ARCHETYPE;
		return;
	  }	/* chiude CON_NEW_WANT_HAND */

	  case CON_NEW_GET_ARCHETYPE:
	  {
	  	if ( !is_name(argument, "nessuno nessuna niente") )
		{
			if ( is_number(argument) )
			{
				x = atoi( argument );
				x--;		/* Diminuisce di uno */
			}
			else
			{
				send_to_char( ch, "Digita il numero corrispondente all'archetipo voluto. " );
				return;
			}

			if ( x >= -1 && x < MAX_ARCHETYPE )
			{
				if ( x == -1 )
					x = number_range( 0, MAX_ARCHETYPE-1 );

				ch->pg->archetype = x;
			}
			else
			{
				ch_printf( ch, "Archetipo errato, scegli un archetipo tra 1 e %d: ", MAX_ARCHETYPE );
				return;
			}
		}

		/* Domanda con quale linguaggi inviare i comandi */
		show_want_language( d );
		d->connected = CON_NEW_WANT_LANGUAGE;
		return;
	  }	/* chiude CON_NEW_WANT_HAND */

	  case CON_NEW_WANT_LANGUAGE:
	  {
		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Scelta non valida. &W1&w o &W2&w? " );
			return;
		}

	  	if ( !str_cmp(argument, "1") || !str_prefix(argument, "italiano") )
	  	{
	  		SET_BIT( ch->pg->flags, PLAYER_ITALIAN );
	  	}
	  	else if ( !str_cmp(argument, "2") || !str_prefix(argument, "inglese") )
	  	{
	  		REMOVE_BIT( ch->pg->flags, PLAYER_ITALIAN );
	  	}
	  	else
	  	{
			send_to_char( ch, "Scelta non valida. &W1&w o &W2&w? " );
			return;
		}

		/* Domanda se visualizzare gli accenti o meno, qui devo utilizzare la write_to_buffer per
		 *	forzare la visualizzazione degli accenti correttamente */
		write_to_descriptor( d, ANSI_CLEAR, 4 );

		write_to_descriptor( d,
			"\r\nOra, se dopo la freccia, vedi 6 lettere accentate scegli '1' altrimenti '2':\r\n", 0 );

		SET_BIT( ch->pg->flags, PLAYER_ACCENT );	/* attiva per il messaggio sotto gli accenti corretti */
		write_to_descriptor( d, "->à è é ì ò ù\r\n", 0 );
		REMOVE_BIT( ch->pg->flags, PLAYER_ACCENT );	/* disattiva l'invio di accenti */

		send_to_char( ch, "&W1&w] Accenti normali e corretti.\r\n"
			"&W2&w] Accenti \"muddosi\" con l'apostrofo a seguito delle vocali.\r\n"
			"Scegli: " );

		d->connected = CON_NEW_WANT_ACCENT;
		return;
	  }	/* chiude CON_NEW_WANT_LANGUAGE */

	  case CON_NEW_WANT_ACCENT:
	  {
		if ( is_number(argument) )
	  	  	x = atoi( argument );
	 	else
	 	{
			send_to_char( ch, "Digita un numero: &W1&w o &W2&w? " );
			return;
		}

		switch ( x )
		{
		  default:
			ch_printf( ch, "Scelta non valida: scegli &W1&w o &W2&w? " );
			return;
		  case 1:		SET_BIT( ch->pg->flags, PLAYER_ACCENT );		break;
		  case 2:		REMOVE_BIT( ch->pg->flags, PLAYER_ACCENT );	break;
		}

		/* Chiede al pg se voglia tornare indietro su qualche scelta errata (RR) farne altre e fare il supporto che faccia tornare subito qui dopo aver fatto quella scelta */
		write_to_buffer( d, ANSI_CLEAR, 4 );
		send_to_char( ch, "  &Y_\r\n" );
		ch_printf	( ch, "  &OH&w   Finito! Sei sicur%c delle tue scelte? Se si digita 'X' per continuare.\r\n", gramm_ao(ch) );
		send_to_char( ch, "  &OH&w   Altrimenti puoi tornare indietro ai seguenti punti:\r\n" );
		send_to_char( ch, "&Y==&R*&Y==&w\r\n" );
		ch_printf	( ch, " &C|&Wa&c|&w Cambiare il sesso del personaggio. (ora: %s)\r\n", (ch->sex == SEX_FEMALE) ? "&Pfemminile" : "&Bmaschile" );
		ch_printf	( ch, " &C|&Wb&c|&w Scegliere la razza. (ora: %s&w)\r\n", get_race(ch, TRUE) );
		ch_printf	( ch, " &C|&Wc&c|&w Scegliere l'allineamento.\r\n" );
		ch_printf	( ch, " &C|&Wd&c|&w Scegliere la classe. (ora: %s&w)\r\n", get_class(ch, TRUE) );
#ifdef T2_QUESTION
		send_to_char( ch, " &C|&We&c|&w Rifare le domande per la scelta della classe.\r\n" );
#endif
#ifdef T2_ALFA
		send_to_char( ch, " &C|&We&c|&w Redistribuire meglio il potenziale degli attributi.\r\n" );
#endif
		send_to_char( ch, " &C|&We&c|&w Scegliere le caratteristiche delle parti del corpo.\r\n" );
		ch_printf	( ch, " &C|&Wf&c|&w Il tipo di mano principale del personaggio (ora: %s)\r\n", (ch->right_hand) ? "destra" : "sinistra" );
		ch_printf	( ch, " &C|&Wg&c|&w Linguaggio dei comandi (ora: %s) o accentazione. (ora: %s)\r\n", (HAS_BIT_PLR(ch, PLAYER_ITALIAN)) ? "italiano" : "inglese", (HAS_BIT_PLR(ch, PLAYER_ACCENT)) ? "si" : "no" );
		send_to_char( ch, "  &C\\&c|&w\r\n" );

		send_to_char( ch, "Scegliendo un numero della lista sopra si dovrà rifare la creazione del tuo\r\n" );
		send_to_char( ch, "personaggio da quel punto in poi.\r\n\r\n" );
		send_to_char( ch, " &WX&w] Continua\r\n" );

		d->connected = CON_NEW_WANT_BACK;
		return;
	  }	/* chiude CON_NEW_WANT_ACCENT */


	  case CON_NEW_WANT_BACK:
	  {
		if ( strlen(argument) > 1 )
		{
			send_to_char( ch, "Opzione non valida, scegli una lettera dal menù della spada e digitala.\r\n" );
			send_to_char( ch, "Oppure premi X per continuare. " );
			return;
		}

		switch ( tolower(argument[0]) )
		{
		  default:
			ch_printf( ch,
				"Scelta non valida. Scegli un numero per tornare indietro ad un punto specifico"
				"della creazione oppure 'X' per continuare. " );
			return;
		  case 'a':	show_want_sex( d );			d->connected = CON_NEW_WANT_SEX;		return;
		  case 'b':	show_get_race( d );			d->connected = CON_NEW_GET_RACE;		return;
		  case 'c':	show_get_alignment( d );	d->connected = CON_NEW_GET_ALIGNMENT;	return;
		  case 'd':	show_get_class( d );		d->connected = CON_NEW_GET_CLASS;		return;
		  case 'e':	show_get_hairtype( d );		d->connected = CON_NEW_GET_HAIRTYPE;	return;
		  case 'f':	show_want_hand( d );		d->connected = CON_NEW_WANT_HAND;		return;
		  case 'g':	show_want_language( d );	d->connected = CON_NEW_WANT_LANGUAGE;	return;
		  case 'x':																		break;
		}

		send_log( NULL, LOG_COMM, "%s@%s nuov%c %s %s.",
			ch->name,
			d->host,
			gramm_ao(ch),
			get_race(ch, FALSE),
			get_class(ch, FALSE) );	/* (GR) inserimento della funzioncina di genere grammaticale */
		ch->level = 0;
		show_title( d );
		ch->position = POSITION_STAND;

	  	d->connected = CON_PRESS_ENTER;
		return;
	  } /* chiude CON_NEW_WANT_BACK */


	  case CON_PRESS_ENTER:
	  {
		if ( chk_watch(get_trust(ch), ch->name, d->host) )
			SET_BIT( ch->pg->flags, PLAYER_WATCH );
		else
			REMOVE_BIT( ch->pg->flags, PLAYER_WATCH );

		if ( ch->position == POSITION_MOUNT )
			ch->position = POSITION_STAND;
#ifdef T2_MSP
		/* msp */
		send_audio_url( d );
		send_audio( d, "login.mid", TO_CHAR );
#endif
		write_to_buffer( d, ANSI_CLEAR, 4 );
		show_file( d, MENU_FILE, TRUE );

#ifdef T2_MXP
		ch_printf( ch, "Visita il Sito del Mondo di %s: &W%s&w\r\n", MUD_NAME, MUD_URL );
/*		ch_printf( ch, "Visita il Sito del Mondo di %s: %s<A href=\"http://www.terra-secunda.net\">&Wwww.terra-secunda.net&w\r\n</A>", MXPMODE(1), MUD_NAME ); (FF) */
#else
		ch_printf( ch, "Visita il Sito del Mondo di %s: &W%s&w\r\n", MUD_NAME, MUD_URL );
#endif

		send_to_char( ch, "Attualmente il miglior Client per giocare a questo mud è &BzMud&w v7.04 e v7.05&w\r\n" );
#ifdef T2_MCCP
		ch_printf( ch, "Il tuo Client (&W%s&w) %ssupporta la compressione dati MCCP\r\n",
			ch->desc->client,
			(ch->desc->mccp && ch->desc->mccp->can_compress) ? "": "non " );
#endif
		send_to_char( ch, "Saremo grati di ogni vostra critica e/o segnalazione di errore\r\n" );
		if ( ch->level != 0 )
		{
			ch_printf( ch, "L'ultima tua connessione da %s è stata il %s\r\n",
				ch->desc->host_old,
				ch->pg->logon_old  ?  friendly_ctime(&ch->pg->logon_old)  :  "<non si sa>" );
			ch_printf( ch, "Hai creato il tuo personaggio il %s giocandolo per %d ore.\r\n",
				friendly_ctime(&ch->pg->creation_date), get_hours_played(ch) );
		}

		/* Le punizioni le possono visualizzare tutti ma non gli amministratori */
		if ( !IS_ADMIN(ch) )
		{
			set_char_color( AT_YELLOW, ch );
			if ( HAS_BIT_PLR(ch, PLAYER_SILENCE) )
				send_to_char( ch, "Per il tuo abuso nei canali, sei per ora silenziato.\r\n" );
			if ( HAS_BIT_PLR(ch, PLAYER_NOEMOTE) )
				send_to_char( ch, "Per il tuo abuso nelle espressioni per ora non le potrai utilizzare.\r\n" );
			if ( HAS_BIT_PLR(ch, PLAYER_NOTELL) )
				send_to_char( ch, "Per il tuo abuso nelle comunicazioni per ora non potrai utilizzarle.\r\n" );
			if ( HAS_BIT_PLR(ch, PLAYER_LITTERBUG) )
				send_to_char( ch, "Piccolo malefico sfruttabachi. Non potrai abbandonare nulla.\r\n" );
			if ( HAS_BIT_PLR(ch, PLAYER_THIEF) )		/* (FF) da togliere, thief rpg */
				send_to_char( ch, "Sei riconosciuto ladro, sarai cacciato dalle autorità.\r\n" );
			if ( HAS_BIT_PLR(ch, PLAYER_KILLER) )		/* (FF) da togliere, killer rpg */
				send_to_char( ch, "Per i tuoi assassini la tua sentenza sarà la morte..\r\n" );
		}

		d->connected = CON_MENU;
		return;
	  }	/* chiude CON_PRESS_ENTER */


	  case CON_MENU:
	  {
		if ( !str_cmp(argument, "q") )
		{
			write_to_buffer( d, "A presto! Un mondo e altri avventurieri attendono il tuo ritorno!\n\r", 0 );
			/* Clear descriptor pointer to get rid of bug message in log
			 * while noticing that you must set teh descriptor to NULL
			 * to do so.   I would advise NOT to tamper with this function
			 * i.e. the close_socket function.  It seems fine so no need to mess with it.
			 */
			d->character->desc = NULL;
			close_socket( d, FALSE );
			return;
		}

		if ( is_number(argument) )
		{
			x = atoi( argument );
		}
		else
		{
			ch_printf( ch, "\r\nDigita un numero del menu oppure 'Q' per uscire da %s.\r\n", MUD_NAME );
			return;
		}

		write_to_buffer( d, ANSI_CLEAR, 4 );
		switch ( x )
		{
		  default:
			show_file( d, MENU_FILE, TRUE );
			return;

		  case 1:
			enter_in_game( ch, FALSE );
			return;

		  case 2:
			send_command( ch, "news ultime", CO );
			return;

		  case 3:
			if ( ch->level == 0 )
				send_to_char( ch, "L'opzione scelta per i personaggi appena creati non è disponibile" );
			else
				send_command( ch, "who", CO );
			return;

		  case 4:
			send_to_char( ch, "&WPer ora i principali aiuti che ci vengono in mente sono:&w\r\n" );
			ch_printf   ( ch, "- Utilizzate spesso il comando &G%s&w.\r\n", translate_command(ch, "commands") );
			send_to_char( ch, "  Soprattuto digitate &G%s traduzione&w se non siete abituati alla\r\n" );
			send_to_char( ch, "  lingua di comandi che state utilizzando.\r\n" );
			send_to_char( ch, "- Utilizzate il comando &Gconfig&w per impostare le opzioni di vostro gradimento.\r\n" );
			send_to_char( ch, "- Anche se siamo ancora una beta alla fine della stessa NON cancelleremo i pg.\r\n" );
			return;

		  case 5:
			send_to_char( ch, "- Affidatevi maggiormente ai suggerimenti inviati che alle informazioni che otterrete\r\n" );
			send_to_char( ch, "  gironzolando in accademia.\r\n" );
			send_to_char( ch, "- Se cadete in un posto che vi sembra una DT e non riusci ad uscirne segnalala.\r\n" );
			send_to_char( ch, "- Il comando chi/who con argomento gruppi pare che non funzioni o che funzioni\r\n" );
			send_to_char( ch, "  a metà, se qualcuno ha un colpo di genio sul perché faccia sapere.\r\n" );
			return;

#ifdef T2_MSP
		  case 6:
			download_audio( ch );
			return;
#endif
		}
		return;
	  }	/* chiude CON_MENU */
	} /* chiude lo switch */
}
