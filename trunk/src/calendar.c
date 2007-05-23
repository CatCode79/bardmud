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


/* Modulo riguardante lo scorrere del tempo, il calendario i compleanni e la cronologia storica  */

#include "mud.h"
#include "calendar.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "msp.h"
#include "nanny.h"
#include "reboot.h"
#include "room.h"


/*
 * Definizioni locali
 */
#define	CALENDAR_FILE	TABLES_DIR "calendar.dat"	/* File con la data dell'anniversario */
#define	MAX_ANNIVERSARY	30							/* Massimo degli anniversary caricabili */


/*
 * Variabili esterne
 */
CALENDAR_DATA		 calendar;
ANNIVERSARY_DATA	*first_anniversary;
ANNIVERSARY_DATA	*last_anniversary;
int					 top_anniversary;


/*
 * Carica le info di default del tempo
 */
void load_calendar_default( void )
{
	long	lhour;
	long	lday;
	long	lmonth;

	lhour = (current_time - 650336715) / (PULSE_TICK/PULSE_IN_SECOND);

	calendar.hour	= lhour  % HOURS_IN_DAY;
	lday			= lhour  / HOURS_IN_DAY;
	calendar.day	= lday   % DAYS_IN_MONTH;
	lmonth			= lday   / DAYS_IN_MONTH;
	calendar.month	= lmonth % MONTHS_IN_YEAR;
	calendar.year	= lmonth / MONTHS_IN_YEAR;
}


/*
 * Lettura del file contenente le informazioni temporali del calendario
 */
void fread_calendar( MUD_FILE *fp )
{
	for ( ; ; )
	{
		char *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_calendar: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Day"		) )		calendar.day =		fread_number( fp );
		else if ( !str_cmp(word, "End"		) )		break;
		else if ( !str_cmp(word, "Hour"		) )		calendar.hour =		fread_number( fp );
		else if ( !str_cmp(word, "Month"	) )		calendar.month =	fread_number( fp );
		else if ( !str_cmp(word, "Year"		) )		calendar.year =		fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_calendar: etichetta non trovata: %s", word );
	}

	/*
	 * Finito di leggere il calendario ne controlla i valori
	 */

	if ( calendar.hour < 0 || calendar.hour > HOURS_IN_DAY)
	{
		send_log( fp, LOG_FREAD, "fread_calendar: ora del calendario errata: %d", calendar.hour );
		exit( EXIT_FAILURE );
	}

	if ( calendar.day < 0 || calendar.day > DAYS_IN_MONTH )
	{
		send_log( fp, LOG_FREAD, "fread_calendar: giorno del calendario errato: %d", calendar.day );
		exit( EXIT_FAILURE );
	}

	if ( calendar.month < 0 || calendar.month > MONTHS_IN_YEAR )
	{
		send_log( fp, LOG_FREAD, "fread_calendar: mese del calendario errato: %d", calendar.month );
		exit( EXIT_FAILURE );
	}

	if ( calendar.year < 0 )
	{
		send_log( fp, LOG_FREAD, "fread_calendar: anno del calendario errato: %d", calendar.year );
		exit( EXIT_FAILURE );
	}

	/* Imposta la fascia d'ora dopo aver caricato le info del calendario */
	if		( calendar.hour < HOUR_SUNRISE-1 )	calendar.sunlight = SUN_DARK;
	else if ( calendar.hour < HOUR_SUNRISE	 )	calendar.sunlight = SUN_RISE;
	else if ( calendar.hour < HOUR_SUNSET	 )	calendar.sunlight = SUN_LIGHT;
	else if ( calendar.hour < HOUR_SUNSET+1  )	calendar.sunlight = SUN_SET;
	else										calendar.sunlight = SUN_DARK;
}


/* Salva le informazioni del tempo per tenr conto del suo scorrere */
void save_calendar( void )
{
	MUD_FILE	*fp;

	fp = mud_fopen( "", CALENDAR_FILE, "w", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "#CALENDAR\n" );
	fprintf( fp->file, "Hour         %d\n", calendar.hour );
	fprintf( fp->file, "Day          %d\n", calendar.day );
	fprintf( fp->file, "Month        %d\n", calendar.month );
	fprintf( fp->file, "Year         %d\n", calendar.year );
	fprintf( fp->file, "End\n\n" );
	fprintf( fp->file, "#END\n" );

	MUD_FCLOSE( fp );
}


/*
 * Carica le informazioni
 */
void load_calendar( void )
{
	load_calendar_default( );

	fread_section( CALENDAR_FILE, "CALENDAR", fread_calendar, TRUE );
}


const char *day_name[] =
{
	"della &WLuna&w",
	"del &RToro&w",
	"dell'&zInganno&w",
	"del &rTuono&w",
	"della &CLibertà&w",
	"degli &BDei&w",
	"del &YSole&w"
};

const char *month_name[] =
{
	"dell'&WInverno del Lupo&w",
	"del &CGigante di Ghiaccio&w",
	"dell'&BArcano Passato&w",
	"della &GNatura&w",
	"della &RGrande Lotta&w",
	"della &OFutilità&w",
	"del &rDragone&w",
	"del &YSole&w",
	"della &rBattaglia&w",
	"delle &zLunghe Ombre&w",
	"dell'&bAntica Oscurità&w",
	"del &xGrande Male&w"
};


char *get_calendar_string( void )
{
	static char	str[MSL];
	int			wday;

	/* Serve per il calcolo di quale giorno della settimana ci si trova
	 *	per accedere correttamente all'array day_name[]
	 *	di default si considera il primo giorno dell'anno come primo giorno della settimana
	 */
	wday  = calendar.month * DAYS_IN_MONTH;
	wday += calendar.day;
	wday %= DAYS_IN_WEEK;

	sprintf( str, "Sono le ore %d, del giorno %s, %d° del mese %s\r\n",
		(calendar.hour%HOURS_IN_DAY == 0)  ?  HOURS_IN_DAY  :  calendar.hour % HOURS_IN_DAY,
		day_name[wday],
		calendar.day+1,
		month_name[calendar.month] );
	sprintf( str+strlen(str), "%d° nell'anno %d.\r\n", calendar.month+1, calendar.year );

	return str;
}


DO_RET do_calendar( CHAR_DATA *ch, char *argument )
{

	set_char_color( AT_PLAIN, ch );
	send_to_char( ch, get_calendar_string() );

	if ( IS_ADMIN(ch) )
	{
		/* (FF) bisognerebbe farle bene rimandando un \r\n ma bisogna togliere quindi a tutti l'\n finale. */
		ch_printf( ch, "\r\n%s :    %21.21s\r\n", MUD_NAME, str_boot_time );
		ch_printf( ch, "Ora Locale Server: %21.21s\r\n", friendly_ctime(&current_time) );
		ch_printf( ch, "Prossimo reboot previsto:   %s", reboot_time );
	}
}


/*
 * Get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 */
void send_time_echo( void )
{
	AREA_DATA		*pArea;
	DESCRIPTOR_DATA *d;

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		char			 echo[MSL];
		int				 color = AT_GREY;
		int				 n;
		int				 pindex;

		if ( !pArea->weather )
			continue;

		echo[0] = '\0';
		n = number_bits( 2 );
		pindex = ( pArea->weather->precip + (meteo.weath_unit*3) - 1 ) / meteo.weath_unit;
	
		switch ( calendar.hour )
		{
		  case HOUR_SUNRISE-1:
		  {
			char *echo_strings[4] =
			{
				"Comincia un nuovo giorno.\r\n",
				"E' un nuovo giorno.\r\n",
				"Il cielo lentamente si rischiara, nell'alba di un nuovo giorno.\r\n",
				"S'affaccia adagio il sole, al giorno appena nato.\r\n"
			};
			calendar.sunlight = SUN_RISE;
			strcpy( echo, echo_strings[n] );
			color = AT_YELLOW;
			break;
		  }

		  case HOUR_SUNRISE:
		  {
			char *echo_strings[4] =
			{
				"Il sole nasce di raggi tiepidi, sorgendo ad est..\r\n",
				"L'Oriente si rischiara: il sole sta sorgendo.\r\n",
				"Un sole fosco alza lo sguardo sul piatto dell'orizzonte..\r\n",
				"Un giorno nuovo saluta il mondo all'ascesa di un pallido sole..\r\n"
			};
			calendar.sunlight = SUN_LIGHT;
			strcpy( echo, echo_strings[n] );
			color = AT_ORANGE;
			break;
		  }

		  case HOUR_NOON:
		  {
			if ( pindex > 0 )
				strcpy( echo, "E' mezzogiorno.\r\n" );
			else
			{
				char *echo_strings[2] =
				{
					"Il sole è alto nel cielo, l'intensità del suo diadema infuocato annuncia il mezzogiorno di luce..\r\n",
					"La luce del sole è vigorosa, nel cielo disegna un bagliore acceso: è mezzogiorno.\r\n"
				};
				strcpy( echo, echo_strings[n%2] );
			}
			calendar.sunlight = SUN_LIGHT;
			color  = AT_WHITE;
			break;
		  }
	
		  case HOUR_SUNSET:
		  {
			char *echo_strings[4] =
			{
				"L'occidente riluce dell'abbraccio infuocato del sole che tramonta..\r\n",
				"L'orizzonte è tagliato dalla corona rossa del sole in tramonto..\r\n",
				"Il cielo si dipinge d'oro rosso brillante, il sole ruotando adagio tramonta oltre lo sguardo.. \r\n",
				"Il sole finisce il suo viaggio portando i raggi splendenti nel sonno del tramonto..\r\n"
			};
	
			calendar.sunlight = SUN_SET;
			strcpy( echo, echo_strings[n] );
			color = AT_RED;

			break;
		  }
	
		  case HOUR_SUNSET+1:
		  {
			if ( pindex > 0 )
			{
				char *echo_strings[2] =
				{
					"Cala la sera.\r\n",
					"Avanza lento il crepuscolo..\r\n"
				};
				strcpy( echo, echo_strings[n%2] );
			}
			else
			{
				char *echo_strings[2] =
				{
					"Il chiarore gentile della luna si diffonde attraverso il cielo, annunciando la sera..\r\n",
					"Mille punti di luci tenui occhieggiano nel cielo serale, contornando una pallida luna..\r\n"
				};
				strcpy( echo, echo_strings[n%2] );
			}
			calendar.sunlight = SUN_DARK;
			color = AT_BLUE;
			break;
		  }
		} /* chiude lo switch su calendar.hour */

		for ( d = first_descriptor;  d;  d = d->next )
		{
			if ( d->connected != CON_PLAYING )			continue;
			if ( !is_outside(d->character) )			continue;
			if ( !is_awake(d->character) )				continue;
			if ( !d->character->in_room )				continue;
			if ( d->character->in_room->area != pArea )	continue;

			/* Se è stato creato un'echo viene inviato */
			if ( VALID_STR(echo) )
			{
				set_char_color( color, d->character );
				send_to_char( d->character, echo );
			}
#ifdef T2_MSP
			/* Invia i suoni */
			if ( calendar.hour == HOUR_SUNSET )
				send_audio( d, "sunset.wav", TO_CHAR );
#endif
		}	/* ciclo for dei descrittori */
	}	/* ciclo for delle aree */
}


/*
 * Update the time
 */
void update_calendar( void )
{
	calendar.hour++;

	if ( calendar.hour == HOUR_MIDNIGHT )
	{
		DESCRIPTOR_DATA *d;

		calendar.hour = 0;
		calendar.day++;

		if ( calendar.day >= DAYS_IN_MONTH )
		{
			calendar.day = 0;
			calendar.month++;
	
			if ( calendar.month >= MONTHS_IN_YEAR )
			{
				calendar.month = 0;
				calendar.year++;
			}
		}

		for ( d = first_descriptor;  d;  d = d->next )
		{
			ANNIVERSARY_DATA *anniversary;

			bool msend = FALSE;	/* Indica se ha inviato un messaggio al pg evitando di visualizzare il cielo */

			if ( d->connected != CON_PLAYING )	continue;
			if ( !d->character->in_room )		continue;

			for ( anniversary = first_anniversary;  anniversary;  anniversary = anniversary->next )
			{
				if ( anniversary->month != calendar.month )		continue;
				if ( anniversary->day != calendar.day )			continue;

				if ( HAS_BIT(anniversary->races, d->character->race) )
					ch_printf( d->character, "%s\r\n", anniversary->message );
			}

			if ( calendar.day == 0 )
			{
				if ( calendar.month == 0 )
				{
					set_char_color( AT_GREEN, d->character );
					send_to_char( d->character, "Oggi è il primo dell'anno!\r\n" );
				}
				else
				{
					set_char_color( AT_DGREEN, d->character );
					ch_printf( d->character, "Oggi è il primo giorno del mese %s\r\n", month_name[calendar.month] );
				}
				msend = TRUE;
			}

			if ( !is_outside(d->character) )	continue;
			if ( !is_awake(d->character) )		continue;
#ifdef T2_MSP
			send_audio( d, "midnight.wav", TO_CHAR );
#endif
			if ( !msend && d->character->in_room->area && d->character->in_room->area->weather )
			{
				int precip;

				precip = ( d->character->in_room->area->weather->precip + (meteo.weath_unit*3) - 1 ) / meteo.weath_unit;
				if ( precip <= 1 )
					send_command( d->character, "sky", CO );
			}
		}
	}

	/* Invia l'echo sulla base dell'ora trascorsa e del weather dell'area */
	send_time_echo( );

	/* Salva il calendario ogni ora-mud */
	save_calendar( );
}


/*
 * Legge una sezione di anniversari
 */
void fread_anniversary( MUD_FILE *fp )
{
	ANNIVERSARY_DATA *anniversary;

	CREATE( anniversary, ANNIVERSARY_DATA, 1 );

	for ( ; ; )
	{
		char *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_anniversary: fine prematura del file durante la lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );

		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Name"		) )		str_dup( fread_word(fp) );
		else if ( !str_cmp(word, "End"		) )		break;
		else if ( !str_cmp(word, "Festivita") )		fread_bool( fp );
		else if ( !str_cmp(word, "Message"	) )		fread_string( fp );
		else if ( !str_cmp(word, "Day"		) )		fread_number( fp );
		else if ( !str_cmp(word, "Month"	) )		fread_number( fp );
		else if ( !str_cmp(word, "Year"		) )		fread_number( fp );
		else if ( !str_cmp(word, "Race"		) )		fread_code_bit( fp, CODE_RACE );
		else
			send_log( fp, LOG_FREAD, "fread_anniversary: etichetta non trovata: %s", word );
	}

	/*
	 * Finito di leggere il comando ne controlla i valori
	 */

	if ( !VALID_STR(anniversary->name) )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: nome dell'anniversario non valido" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(anniversary->message) )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: messaggio dell'anniversario non valido" );
		exit( EXIT_FAILURE );
	}

	if ( anniversary->day < 0 || anniversary->day >= DAYS_IN_MONTH )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: giorno dell'anniversario errato: %d", anniversary->day );
		exit( EXIT_FAILURE );
	}

	if ( anniversary->month < 0 || anniversary->month >= MONTHS_IN_YEAR )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: mese dell'anniversario errato: %d", anniversary->month );
		exit( EXIT_FAILURE );
	}

	if ( !anniversary->festivity && anniversary->year < 0 )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: anno dell'anniversario probabilmente errato o precedente al portale: %d", anniversary->year );
		exit( EXIT_FAILURE );
	}

	if ( anniversary->festivity && anniversary->year != 0 )
	{
		send_log( fp, LOG_FREAD, "fread_anniversary: anno di una festività non zero" );
		exit( EXIT_FAILURE );
	}

	LINK( anniversary, first_anniversary, last_anniversary, next, prev );
	top_anniversary++;
}


/*
 * Carica tutti gli anniversari
 */
void load_anniversary( void )
{
	fread_section( "../tables/anniversary.dat", "ANNIVERSARY", fread_anniversary, FALSE );
}


/*
 * Libera dalla memoria tutti gli anniversari
 */
void free_all_anniversaries( void )
{
	ANNIVERSARY_DATA	*anniversary;

	for ( anniversary = first_anniversary;  anniversary;  anniversary = first_anniversary )
	{
		UNLINK( anniversary, first_anniversary, last_anniversary, next, prev );
		top_anniversary--;

		DISPOSE( anniversary->name );
		DISPOSE( anniversary->message );
		destroybv( anniversary->races );
		DISPOSE( anniversary );
	}

	if ( top_anniversary != 0 )
		send_log( NULL, LOG_BUG, "free_all_annivesary: top_anniversary non è 0 dopo aver libearto tutti gli anniversari: %d", top_anniversary );
}


/* (RR) con l'età iniziale scelta dal giocatore e la divisione del return differente basata sul nostro time
 * Ritorna l'età di un giocatore.
 */
int get_age( CHAR_DATA *ch, int type )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_age: ch è NULL" );
		return 17;
	}

	if ( IS_MOB(ch) )
		return 17;

	switch ( type )
	{
	  default:
		send_log( NULL, LOG_BUG, "tipo di unità temporale errata per %s: %d", ch->name, type );
		return 17;

	  case TIME_YEAR:	return 17 + (  (current_time - ch->pg->creation_date) / SECONDS_IN_YEAR );						break;
	  case TIME_MONTH:	return  1 + ( ((current_time - ch->pg->creation_date) / SECONDS_IN_MONTH) % MONTHS_IN_YEAR );	break;
	  case TIME_DAY:	return  1 + ( ((current_time - ch->pg->creation_date) / SECONDS_IN_YEAR ) % DAYS_IN_MONTH  );	break;
	}
}


#ifdef T2_ALFA
/*
 * Ritorna una stringa con la data di nascita del pg
 */
char *get_date( CHAR_DATA *ch )
{

}
#endif


/* (RR) Anche questa come sopra rifarla con la data di inizio gioco.
 * Ritorna le ore giocate da un pg.
 */
int get_hours_played( CHAR_DATA *ch )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "get_hours_played: ch è NULL" );
		return 0;
	}

	if ( IS_MOB(ch) )
		return 0;

	return ( ch->pg->played + (current_time - ch->pg->logon) ) / 3600;
}
