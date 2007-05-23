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

/* Modulo per la gestione economica */


#include "mud.h"
#include "calendar.h"
#include "db.h"
#include "fread.h"
#include "room.h"


/*
 * Definizioni dei file
 */
#define SHARE_FILE		TABLES_DIR "share.dat"	/* File in cui viene salvato il valore delle azioni da un reboot ad un altro */


/*
 * Definizioni
 */
#define DEF_SHAREVALUE	  200										/* Valore iniziale/di default/medio delle azioni */
#define MIN_SHAREVALUE	(  DEF_SHAREVALUE / 2 )						/* Valore minimo del valore delle azioni */
#define MAX_SHAREVALUE	( (DEF_SHAREVALUE / 2) + DEF_SHAREVALUE )	/* Valore massimo del valore delle azioni */


/*
 * Variabili
 */
int		share_value	= DEF_SHAREVALUE;


/*
 * Load the bank information (economy info)
-------------------------------------------
 * By Maniac from Mythran Mud
-------------------------------------------
 */
void load_share( void )
{
	MUD_FILE	*fp;

	fp = mud_fopen( "", SHARE_FILE, "r", TRUE );
	if ( !fp )
	{
		share_value = DEF_SHAREVALUE;
		return;
	}

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "load_share: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if ( !str_cmp(word, "ShareValue") )
		{
			share_value = fread_number( fp );
			if ( share_value <= 0 )
			{
				send_log( fp, LOG_FREAD, "load_share: valore delle azioni zero o negativo" );
				exit( EXIT_FAILURE );
			}
			break;
		}
		else
			send_log( fp, LOG_FREAD, "fread_language: nessuna etichetta per la parola %s.", word );
	} /* chiude il for */

	MUD_FCLOSE( fp );
}


/*
 * Ritorna semplicemente il valore della variabile locale
 */
int get_share_value( void )
{
	return share_value;
}


/*
 * Ritorna la fascia di appartenenza del valore delle azioni
 */
int get_share_band( void )
{
	if ( share_value < MIN_SHAREVALUE || share_value > MAX_SHAREVALUE )
	{
		send_log( NULL, LOG_BUG, "get_share_band: valore delle azioni errato: %d", share_value );
		return 3;	/* fascia media */
	}

	if		( share_value < DEF_SHAREVALUE * 4/6 )	return 1;	/* Azioni bassissime */
	else if ( share_value < DEF_SHAREVALUE * 5/6 )	return 2;	/* Azioni in ribasso */
	else if ( share_value < DEF_SHAREVALUE * 7/6 )	return 3;	/* Medio			 */
	else if ( share_value < DEF_SHAREVALUE * 8/6 )	return 4;	/* Azioni in rialzo	 */
	else											return 5;	/* Azioni altissime	 */
}


/*
 * This updates the shares value (I hope)
-------------------------------------------
 * Update the bank system
 * (C) 1996 The Maniac from Mythran Mud
-------------------------------------------
 */
void update_share( void )
{
	MUD_FILE	*fp;
	int		 band;
	int		 value = 0;

	/* Bank is closed, no market */
	if ( calendar.hour < 8 || calendar.hour > 18 )
		return;

	band = get_share_band( );
	switch ( band )
	{
	  case 1:		value = number_range(  -8, 12 );	break;
	  case 2:		value = number_range(  -9, 11 );	break;
	  case 3:		value = number_range( -10, 10 );	break;
	  case 4:		value = number_range( -11,  9 );	break;
	  case 5:		value = number_range( -12,  8 );	break;
	}

	share_value += value;

	if ( share_value < MIN_SHAREVALUE )
		share_value = MIN_SHAREVALUE;
	if ( share_value > MAX_SHAREVALUE )
		share_value = MAX_SHAREVALUE;

	send_log( NULL, LOG_SHARE, "update_share: valore azioni: %d", share_value );

	fp = mud_fopen( "", SHARE_FILE, "w", TRUE );
	if ( !fp )
		return;

	fprintf( fp->file, "ShareValue   %d\n", share_value );

	MUD_FCLOSE( fp );
}


/*
 * Funzione di base che varia il costo di una cosa in base a diversi fattori
 * Usata per il calcolo delle spese nelle lezioni e nei negozi
 */
int compute_cost( CHAR_DATA *ch, CHAR_DATA *mob, int cost )
{
	/* Se le azioni vanno male aumenta, se vanno bene sconta */
	if ( cost > 1 )
	{
		switch ( get_share_band() )
		{
		  case 1:	(int) ( cost *= 1.02 );		break;
		  case 2:	(int) ( cost *= 1.01 );		break;
		  case 4:	(int) ( cost /= 1.01 );		break;
		  case 5:	(int) ( cost /= 1.02 );		break;
		}
	}

	/* Sconto del 3% se il maestro e il pg hanno la stessa razza */
	if ( cost > 1 && ch->race == mob->race )
		(int) ( cost /= 1.03 );		/* (TT) ma ha senso questo cast? */

	/* (FF) Aggiungerci anche un aumento o altro per le razze che i ch odiano */

	/* (FF) E fare la skill haggle */

	return cost;
}


/*
 * Add gold to an area's economy.
 */
void boost_economy( AREA_DATA *tarea, int gold )
{
	while ( gold >= 1000000000 )
	{
		++tarea->economy_high;
		gold -= 1000000000;
	}

	tarea->economy_low += gold;
	while ( tarea->economy_low >= 1000000000 )
	{
		++tarea->economy_high;
		tarea->economy_low -= 1000000000;
	}
}


/*
 * Take gold from an area's economy				-Thoric
 */
void lower_economy( AREA_DATA *tarea, int gold )
{
	while ( gold >= 1000000000 )
	{
		--tarea->economy_high;
		gold -= 1000000000;
	}
	tarea->economy_low -= gold;
	while ( tarea->economy_low < 0 )
	{
		--tarea->economy_high;
		tarea->economy_low += 1000000000;
	}
}


/*
 * Check to see if economy has at least this much gold		   -Thoric
 */
bool economy_has( AREA_DATA *tarea, int gold )
{
	int hasgold = ( (tarea->economy_high > 0)  ?  1  :  0 ) * 1000000000 + tarea->economy_low;

	if ( hasgold >= gold )
		return TRUE;

    return FALSE;
}


/*
 * Used in db.c when resetting a mob into an area		    -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold( CHAR_DATA *mob )
{
	int gold;
	AREA_DATA *tarea;

	/* Make sure it isn't way too much */
	mob->gold = UMIN( mob->gold, get_level(mob)/2 * get_level(mob)/2 * 400 );

	if ( !mob->in_room->area )
		return;

	tarea = mob->in_room->area;

	gold = ((tarea->economy_high > 0) ? 1 : 0) * 1000000000 + tarea->economy_low;
	mob->gold = URANGE( 0, mob->gold, gold / 10 );
	if ( mob->gold )
		lower_economy( tarea, mob->gold );
}


/*
 * Comando admin per visualizzare le informazioni economico-bancarie
 */
DO_RET do_economy( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*banker;

	ch_printf( ch, "Valore azioni: %d\r\n\r\n", get_share_value() );

	send_to_char( ch, "Elenco delle Banche:\r\n" );
	for ( banker = first_char;  banker;  banker = banker->next )
	{
		if ( IS_PG(banker) )						continue;
		if ( !HAS_BIT(banker->act, MOB_BANKER) )	continue;

		ch_printf( ch, "Bancario %s #%d alla stanza %s #%d\r\n",
			banker->short_descr, banker->pIndexData->vnum,
			banker->in_room->name, banker->in_room->vnum );
	}
}
