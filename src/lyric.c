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


#include "mud.h"
#include "command.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "lyric.h"
#include "nanny.h"
#include "room.h"


/*
 * Definizioni
 */
#define	LYRIC_FILE		TABLES_DIR "lyrics.dat"		/* File contenente le liriche */
#define	MAX_PENTAGRAM	30							/* Massimo di pentagrammi caricabili in una lirica */
#define MAX_LYRIC		10							/* Massimo di liriche caricabili */


/*
 * Variabili
 */
LYRIC_DATA	*first_lyric	= NULL;
LYRIC_DATA	*last_lyric		= NULL;
int			 top_lyric		= 0;


/*
 * Carica una sezione di lyric
 */
void fread_lyric( MUD_FILE *fp )
{
	LYRIC_DATA	*lyric;
	PENTAGRAM_DATA	*pentatest;

	CREATE( lyric, LYRIC_DATA, 1 );

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
			send_log( fp, LOG_FREAD, "fread_lyric: fine prematura del file durante la lettura" );

		word = fread_word( fp );

		if ( word[0] == '*' )
			fread_to_eol( fp );

		if		( !str_cmp(word, "End"			) )		break;
		else if ( !str_cmp(word, "Language"		) )		lyric->language =	code_code( fp, fread_word(fp), CODE_LANGUAGE );
		else if ( !str_cmp(word, "Name"			) )		lyric->name =		fread_string( fp );
		else if ( !str_cmp(word, "Pentagram") )
		{
			PENTAGRAM_DATA *pentagram;

			CREATE( pentagram, PENTAGRAM_DATA, 1 );

			pentagram->rhythm = fread_float( fp ) * PULSE_IN_SECOND;	/* Rende intero il float dei secondi convertendolo in pulse */
			pentagram->action = fread_string( fp );

			LINK( pentagram, lyric->first_pentagram, lyric->last_pentagram, next, prev );
		}
		else if ( !str_cmp(word, "Races"		) )		lyric->races =		fread_code_bit( fp, CODE_RACE );
		else if ( !str_cmp(word, "Secret"		) )		lyric->secret =		fread_bool( fp );
		else if ( !str_cmp(word, "Title"		) )		lyric->title =		fread_string( fp );
		else if ( !str_cmp(word, "VnumMob"		) )		lyric->vnum_mob =	fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_lyrics: etichetta non trovata: %s", word );
	}

	/*
	 * Controlla la lirica caricata
	 */

	if ( !VALID_STR(lyric->name) )
	{
		send_log( fp, LOG_FREAD, "fread_lyric: nome della lirica errato" );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(lyric->title) )
	{
		send_log( fp, LOG_FREAD, "fread_lyric: titolo della lirica errato" );
		exit( EXIT_FAILURE );
	}

	/* Se è zero significa che non ha un mob preciso che canti quella canzone, quindi il valore è lecito */
	if ( lyric->vnum_mob < 0 || lyric->vnum_mob >= MAX_VNUM )
	{
		send_log( fp, LOG_FREAD, "fread_lyric: vnum del mob che deve cantare la canzone errato: %d", lyric->vnum_mob );
		exit( EXIT_FAILURE );
	}

	for ( pentatest = lyric->first_pentagram;  pentatest;  pentatest = pentatest->next )
	{
		if ( pentatest->rhythm < 0.1 * PULSE_IN_SECOND )
		{
			send_log( NULL, LOG_FREAD, "Tempo in secondi minore di 0,1" );
			exit( EXIT_FAILURE );
		}

		if ( pentatest->rhythm > 100 * PULSE_IN_SECOND )
		{
			send_log( NULL, LOG_FREAD, "Tempo in secondi maggiore di 100" );
			exit( EXIT_FAILURE );
		}

		if ( pentatest->action )
		{
			char	arg[MIL];

			one_argument( pentatest->action, arg );

			/* Il comando dell'azione del pentagramma deve essere uno di questi e non altri */
			if ( !is_name(arg, "whisper murmur say yell shout emote sing") )
			{
				send_log( NULL, LOG_FREAD, "Il comando %s nelle liriche non è permesso", arg );
				exit( EXIT_FAILURE );
			}
		}
	}

	LINK( lyric, first_lyric, last_lyric, next, prev );
	top_lyric++;
}


/*
 * Carica tutti le liriche
 */
void load_lyrics( void )
{
	fread_section( LYRIC_FILE, "LYRIC", fread_lyric, FALSE );

	/* Controlla il caricamento delle liriche */
	if ( top_lyric >= MAX_LYRIC )
	{
		send_log( NULL, LOG_FREAD, "load_lyrics: numero di liriche caricate %d maggiore o uguale a MAX_LYRIC %d",
			top_lyric, MAX_LYRIC );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memroia tutte le liriche
 */
void free_all_lyrics( void )
{
	LYRIC_DATA *lyric;

	for ( lyric = first_lyric;  lyric;  lyric = first_lyric )
	{
		PENTAGRAM_DATA *pentagram;

		UNLINK( lyric, first_lyric, last_lyric, next, prev );
		top_lyric--;

		DISPOSE( lyric->name );
		DISPOSE( lyric->title );
		destroybv( lyric->races );
		for ( pentagram = lyric->first_pentagram;  pentagram;  pentagram = lyric->first_pentagram )
		{
			UNLINK( pentagram, lyric->first_pentagram, lyric->last_pentagram, next, prev );
			DISPOSE( pentagram->action );
			DISPOSE( pentagram );
		}
		DISPOSE( lyric );
	}

	if ( top_lyric != 0 )
		send_log( NULL, LOG_BUG, "free_all_lyrics: top_lyric non è 0 dopo aver liberato le liriche: %d", top_lyric );
}


/*
 * Ritorna una lyric passandogli un numero
 */
LYRIC_DATA *get_lyric_from_num( const int num )
{
	LYRIC_DATA *lyric;
	int			x = 0;

	for ( lyric = first_lyric;  lyric;  lyric = lyric->next )
	{
		if ( num == x )
			return lyric;

		x++;
	}

	return NULL;
}


/*
 * Decide se inviare una nuova lirica per i mob bardi
 * Invia ogni tot un pezzo di lirica
 */
void update_lyric( void )
{
	DESCRIPTOR_DATA *d;

	/* Cicla tra i descrittori così invia solo in presenza di pg ed è più veloce */
	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA *singer;

		if ( d->connected != CON_PLAYING )	continue;
		if ( !d->character->in_room )		continue;

		for ( singer = d->character->in_room->first_person;  singer;  singer = singer->next_in_room )
		{
			if ( IS_PG(singer) )					continue;
			if ( !HAS_BIT_ACT(singer, MOB_SING) )	continue;
			if ( who_fighting(singer) )				continue;

			/*
			 * Se non ha una lirica gliene dà una a caso
			 */
			if ( !singer->lyric )
			{
				LYRIC_DATA *lyric;
				int			num;

				/* Visto che la update_lyric passa ogni pulse la probabilità è PULSE_IN_SECOND dipendente */
				if ( number_range(0, PULSE_IN_SECOND*480) != 0 )
					continue;

				num = number_range( 0, top_lyric-1 );
				lyric = get_lyric_from_num( num );

				if ( !lyric )			continue;
				if ( lyric->secret )	continue;	/* Se la lirica è segreta salta */

				/* Se non conosce la lingua della lirica la salta */
				if ( !IS_EMPTY(singer->speaks) && !HAS_BIT(singer->speaks, lyric->language) )
					continue;

				/* Se non è una razza che canterebbe quella lirica salta */
				if ( !IS_EMPTY(lyric->races) && !HAS_BIT(lyric->races, singer->race) )
					continue;

				send_log( NULL, LOG_LYRIC, "update_lyric: passa al mob %d in %d", singer->pIndexData->vnum, singer->in_room->vnum );

				singer->lyric = lyric;
				continue;
			}

			/* Se non ha un pentagramma da cantare allora è al primo della lirica */
			if ( !singer->pentagram )
			{
				singer->pentagram = singer->lyric->first_pentagram;
				singer->rhythm = singer->pentagram->rhythm;
			}

			/* Controlla se è giunto il tempo di inviare il pentagramma */
			if ( --singer->rhythm <= 0 )
			{
				/* (RR) dovrei switchare nella lingua della lirica se la ha */
				send_command( singer, singer->pentagram->action, CO );
				if ( singer->pentagram->next )
				{
					singer->pentagram = singer->pentagram->next;	/* Procede al pentagramma seguente */
					singer->rhythm = singer->pentagram->rhythm;
				}
				else
				{
					singer->lyric = NULL;
					singer->pentagram = NULL;
					singer->rhythm = 0;
				}
			}
		}	/* chiude il for delle persone nella stanza */
	} /* chiude il for dei descriptor */
}
