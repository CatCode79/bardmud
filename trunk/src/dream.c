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


/***************************************************************************
 *   DREAM.C BY ANDREW TOLMASOFF 2004 (C) QuickMUD Advanced                *
 ***************************************************************************/

#include "mud.h"
#include "command.h"
#include "dream.h"
#include "db.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "nanny.h"
#include "room.h"
#include "save.h"


/*
 * Definizioni
 */
#define DREAM_FILE		TABLES_DIR "dreams.dat"		/* File con i Sogni */
#define MAX_DREAM		25							/* Numero massimo di sogni caricabili */


/*
 * Variabile globale
 */
int		top_dream = 0;


/*
 * Variabli locali
 */

DREAM_DATA	*first_dream = NULL;
DREAM_DATA	*last_dream	 = NULL;


/*
 * Legge un sogno
 */
void fread_dream( MUD_FILE *fp )
{
	DREAM_DATA	*dream;
	char		*word;
	int			 part = 0;
	int			 x;
	int			 y;

	CREATE( dream, DREAM_DATA, 1 );
	CLEAR_BITS( dream->classes );
	CLEAR_BITS( dream->races );
	dream->sex = -1;

	for ( ; ; )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Classes"	) )		dream->classes =		fread_code_bit( fp, CODE_CLASS );
		else if ( !str_cmp(word, "End"		) )		break;
		else if ( !str_cmp(word, "Name"		) )		dream->name =			str_dup(fread_word(fp) );
		else if ( !str_cmp(word, "NumPart"	) )		dream->num_part =		fread_number( fp );
		else if ( !str_cmp(word, "Nightmare") )		dream->nightmare =		fread_bool( fp );
		else if ( !str_cmp(word, "Part"		) )
		{
			dream->part[part] =												fread_string( fp );

			/* (FF) convertirlo con una fscanf() così da essere compatibile con la modifica del valore di MAX_DREAM_NEXT */
			for ( x = 0;  x < MAX_DREAM_NEXT;  x++ )
			{
				/* Diminuisce di uno perché ricava il numero partendo da 1 */
				dream->next_part[part][x] =										fread_number( fp );
			}

			part++;
		}
		else if ( !str_cmp(word, "Races"	) )		dream->races =			fread_code_bit( fp, CODE_RACE );
		else if ( !str_cmp(word, "Sex"		) )		dream->sex =			fread_number( fp );
		else
			send_log( fp, LOG_FREAD, "fread_dream: nessuna etichetta per la parola %s", word );
	}

	/*
	 * Controlla il sogno appena caricato
	 */

	if ( dream->num_part < 0 || dream->num_part >= MAX_DREAM_PART )
	{
		send_log( fp, LOG_FREAD, "fread_dream: numero di parti del sogno errato: %d", dream->num_part );
		exit( EXIT_FAILURE );
	}

	if ( !VALID_STR(dream->name) )
	{
		send_log( fp, LOG_FREAD, "fread_dream: nome del dream inesistente" );
		exit( EXIT_FAILURE );
	}

	/* Controlla che il valore raggiunto da part sia num_part */
	if ( part != dream->num_part )
	{
		send_log( fp, LOG_FREAD, "fread_dream: numero di parti acquisite (%d) diverso da quello in NumPart (%d).", part, dream->num_part );
		exit( EXIT_FAILURE );
	}

	if ( dream->sex < -1 && dream->sex > 2 )
	{
		send_log( fp, LOG_FREAD, "fream_dream: sesso acquisito errato: %d", dream->sex );
		exit( EXIT_FAILURE );
	}

	for ( x = 0;   x < dream->num_part;  x++ )
	{
		if ( !VALID_STR(dream->part[x]) )
		{
			send_log( fp, LOG_FREAD, "fread_dream: il testo della parte numero %d è vuota", x );
			exit( EXIT_FAILURE );
		}
	}

	for ( x = 0;  x < dream->num_part;  x++ )
	{
		for ( y = 0;  y < MAX_DREAM_NEXT;  y++ )
		{
			if ( dream->next_part[x][y] < -1 || dream->next_part[x][y] >= MAX_DREAM_NEXT )
			{
				send_log( fp, LOG_FREAD, "fread_dream: valori di next %d per la parte %d è errato: %d", y, x, dream->next_part[x][y] );
				exit( EXIT_FAILURE );
			}
		}
	}

	LINK( dream, first_dream, last_dream, next, prev );
	top_dream++;
}


/*
 * Libera dalla memoria tutti i sogni
 */
void free_all_dreams( void )
{
	DREAM_DATA *dream;

	for ( dream = first_dream;  dream;  dream = first_dream )
	{
		int x;

		UNLINK( dream, first_dream, last_dream, next, prev );
		top_dream--;

		DISPOSE( dream->name );
		destroybv( dream->classes );
		destroybv( dream->races );
		for ( x = 0;  x < MAX_DREAM_PART;  x++ )
			DISPOSE( dream->part[x] );
		DISPOSE( dream );
	}

	if ( top_dream != 0 )
		send_log( NULL, LOG_BUG, "free_all_dreams: top_dream non è 0 dopo aver liberato i sogni: %d", top_dream );
}


/*
 * Carica e controlla tutti i sogni
 */
void load_dreams( void )
{
	fread_section( DREAM_FILE, "DREAM", fread_dream, FALSE );

	/* Controllo generale dei sogni */
	if ( top_dream >= MAX_DREAM )
	{
		send_log( NULL, LOG_FREAD, "fread_dream: numero di sogni caricati: %d raggiunge o supera il massimo caricabile: %d",
			top_dream, MAX_DREAM );
		exit( EXIT_FAILURE );
	}
}


/*
 * Passata una stringa e ricava il sogno controllando i nomi
 */
DREAM_DATA *get_dream( const char *str, bool exact )
{
	DREAM_DATA *dream;

	if ( !VALID_STR(str) )
		return NULL;

	for ( dream = first_dream;  dream;  dream = dream->next )
	{
		if ( !str_cmp(str, dream->name) )
			return dream;
	}

	if ( exact )
		return NULL;

	for ( dream = first_dream;  dream;  dream = dream->next )
	{
		if ( !str_prefix(str, dream->name) )
			return dream;
	}

	return NULL;
}


/*
 * Ritorna un sogno
 */
DREAM_DATA *get_dream_from_num( const int num )
{
	DREAM_DATA *dream;
	int			x = 0;

	if ( num < 0 || num >= top_dream )
		return NULL;

	for ( dream = first_dream;  dream;  dream = dream->next )
	{
		if ( num == x )
			return dream;

		x++;
	}

	return NULL;
}


void dream_reset( CHAR_DATA *ch )
{
	if ( IS_MOB(ch) )
		return;

	ch->pg->dream_part = 0;
	ch->pg->dream = NULL;

	while ( !ch->pg->dream )
	{
		DREAM_DATA *dream;

		dream = get_dream_from_num( number_range(0, top_dream-1) );
		if ( !dream )
			continue;

		/* Controlla se il sogno lo può vedere quella classe */
		if ( !IS_EMPTY(dream->classes) && !HAS_BIT(dream->classes, ch->class) )
			continue;

		/* Controlla se il sogno lo può vedere quella razza */
		if ( !IS_EMPTY(dream->races) && !HAS_BIT(dream->races, ch->race) )
			continue;

		/* Controlla se il sogno lo può vedere quel sesso */
		if ( dream->sex != -1 && dream->sex != ch->sex )
			continue;

		ch->pg->dream = dream;
	}

	if ( get_level(ch) < LVL_LEGEND && ch->exp >= exp_level(ch, ch->level+1) )
		send_to_char( ch, "Non ho dormito abbastanza per assorbire le esperienze che ho avuto.\r\n" );

	save_player( ch );
}


void update_dream( void )
{
	DESCRIPTOR_DATA *d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		CHAR_DATA	*ch;
		int			 next_dream = -1;
		int			 x;

		ch = d->character;

		if ( !ch )							continue;
		if ( d->connected != CON_PLAYING )	continue;
		if ( IS_MOB(ch) )					continue;
		if ( ch->position != POSITION_SLEEP )	continue;

		if ( !ch->pg->dream )
			dream_reset( ch );

		/* Invia una parte di sogno, usa la change_command_tag per cambiare le flag °x di sesso */
		if ( VALID_STR(ch->pg->dream->part[ch->pg->dream_part]) )
			send_to_char( ch, change_command_tag(ch->pg->dream->part[ch->pg->dream_part], ch) );

		/* Se la parte appena inviata era l'ultima resetta il sogno */
		if ( ch->pg->dream_part == ch->pg->dream->num_part )
		{
			if ( ch->pg->dream->nightmare )
			{
				int rnd;

				rnd = number_range( 0 , 4 );

				switch ( rnd )
				{
				  case 0:	send_to_char( ch, "Capisco d'essermi svegliato allora.\r\n" );	break;
				  case 1:	send_to_char( ch, "Mi sveglio di soprassalto e rimango fermo, come paralizzato, per alcuni secondi..\r\n" );	break;
				  case 2:	send_to_char( ch, "Mi rendo conto solo ora di aver avuto un incubo.\r\n" );	break;
				  case 3:	send_to_char( ch, "Mi sveglio stringendo la mano su una parte dolorante del mio corpo..\r\n" );	break;
				  case 4:	send_to_char( ch, "Mi sveglio d'improvviso, con una sensazione di soffocamento che ti stringe il petto.\r\n" );	break;
				}

				/* (FF) botta al mental state, proporzionale al livello */
				send_command( ch, "wake", CO );
			}

			/* Da un regalino in px per aver visto il sogno
			 * Così facendo fa livellare se ce nè bisogno */
			gain_exp( ch, (LVL_LEGEND+1 - get_level(ch))/2, TRUE );

			/* Se il sogno è mononoke allora trasporta nell'area del villaggio di ashitaka */
			/* (bb) pare vi sia un problema, se un pg si addormenta e quitta, c'è una probabilità che svegliandosi si rirtovi a mononokolandia */
			if ( ch->in_room && ch->in_room->sector == SECTOR_FOREST && get_level(ch) > LVL_NEWBIE
			  && !HAS_BIT_PLR(ch, PLAYER_AFK) && !HAS_BIT_PLR(ch, PLAYER_AFK2) )
			{
				ROOM_DATA *location;

				location = get_room_index( NULL, VNUM_ROOM_ASHITAKA );
				if ( location )
				{
					char_from_room( ch );
					char_to_room( ch, location );
					act( AT_MAGIC, "Una bolla di luce avvolge $n mentre dorme, trasportandol%x via.", ch, NULL, NULL, TO_ROOM );
				}
				else
					send_log( NULL, LOG_BUG, "update_dream: locatione #%d per il villaggio di ashitaka inesistente", VNUM_ROOM_ASHITAKA );
			}

			return;		/* Esce, quindi invia un solo sogno per dormita */
		}

		/* Se invece non era l'ultima, conta quanti possibili next ci sono per la parte sognata */
		for ( x = 0;  x < MAX_DREAM_NEXT;  x++ )
		{
			if ( ch->pg->dream->next_part[ch->pg->dream_part][x] != 0 )
				next_dream++;
		}

		/* Se ci sono next ne sceglie casualmente uno */
		if ( next_dream != -1 )
			ch->pg->dream_part = ch->pg->dream->next_part[ch->pg->dream_part][number_range(0, next_dream)] - 1;
		else
			ch->pg->dream_part++;
	}
}


/*
 * Comando admin per inviare un determinato tipo di sogno
 */
DO_RET do_dream( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	DREAM_DATA	*dream;
	char		 arg[MIL];	/* Indica il nome della vittima da inviare */

	if ( !VALID_STR(argument) )
	{
		int col = 0;

		send_to_char( ch, "&YSintassi&w:  dream <nome pg> nessuno\r\n" );
		send_to_char( ch, "&YSintassi&w:  dream <nome pg> <nome sogno>\r\n\r\n" );

		send_to_char( ch, "Nomi dei sogni:\r\n" );
		for ( dream = first_dream;  dream;  dream = dream->next )
		{
			ch_printf( ch, "&%c%-19s ", (dream->nightmare) ? 'R' : 'w', dream->name );

			if ( (++col)%4 == 0 )
				send_to_char( ch, "\r\n" );
			if ( (col%4 != 0) && col == top_dream-1 )
				send_to_char( ch, "&w\r\n" );
		}

		return;
	}

	argument = one_argument( argument, arg );
	victim = get_player_mud( ch, arg, TRUE );
	if ( !victim )
	{
		ch_printf( ch, "%s non si trova online.\r\n", arg );
		return;
	}

	if ( victim->position <= POSITION_SLEEP )
	{
		ch_printf( ch, "%s sta dormendo o morendo ora, non si può settare il sogno in questo momento.\r\n", victim->name );
		return;
	}

	if ( !is_name(argument, "nessuno none nulla") )
	{
		/* Indica che il pg non sognerà così da permettere all'admin di inviare dei messaggi echo appositi */
		ch->pg->dream = NULL;
	}
	else
	{
		dream = get_dream( argument, FALSE );

		if ( !dream )
		{
			ch_printf( ch, "%s è un sogno inesistente.\r\n", argument );
			return;
		}

		ch->pg->dream = dream;
	}
}
