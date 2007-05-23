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
 > 							Modulo delle Citazioni							<
\****************************************************************************/

#include "mud.h"
#include "fread.h"
#include "quote.h"


/*
 * File con i quote
 */
#define QUOTE_FILE	TABLES_DIR "quotes.dat"


/*
 * Definzione del limite di citazioni caricabili
 */
#define MAX_QUOTE	100


/*
 * Variabile esterna
 */
int			 top_quote	 = 0;		/* Numero delle Citazioni caricate */


/*
 * Variabili locali
 */
QUOTE_DATA	*first_quote = NULL;	/* Prima citazione */
QUOTE_DATA	*last_quote	 = NULL;	/* Ultima citazione */


/*
 * Lettura di un Quote
 */
void fread_quote( MUD_FILE *fp )
{
	QUOTE_DATA	*quote;

	if ( !fp )
	{
		send_log( NULL, LOG_FREAD, "fread_quote: il file passato è NULL" );
		exit( EXIT_FAILURE );
	}

	CREATE( quote, QUOTE_DATA, 1 );

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_quote: fine prematura del file" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "Author") )	quote->author =			fread_string( fp );
		else if ( !str_cmp(word, "Text"	 ) )	quote->text =			fread_string( fp );
		else if ( !str_cmp(word, "End"	 ) )	break;
		else
			send_log( fp, LOG_FREAD, "fread_quote: nessuna etichetta per la parola %s", word );
	}

	/*
	 * Dopo aver caricato un quote lo ricontrolla
	 */

	/* Il testo di un Quote ci deve essere sempre */
	if ( !VALID_STR(quote->text) )
	{
		send_log( fp, LOG_FREAD, "fread_quote: la citazione è senza testo" );
		exit( EXIT_FAILURE );
	}

	/* Una citazione può essere senza autore, quindi nessun test per quella stringa */

	LINK( quote, first_quote, last_quote, next, prev );
	top_quote++;
}


/*
 * Carica e poi controlla tutti i Quote
 */
void load_quotes( void )
{
	fread_section( QUOTE_FILE, "QUOTE", fread_quote, FALSE );

	/* Controlla la validità del numero caricato di quote */
	if ( top_quote < 0 || top_quote >= MAX_QUOTE )
	{
		send_log( NULL, LOG_FREAD, "load_quotes: il numero di citazioni caricate è errato: %d", top_quote );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memoria tutti i quote
 */
void free_all_quotes( void )
{
	QUOTE_DATA *quote;

	for ( quote = first_quote;  quote;  quote = first_quote )
	{
		UNLINK( quote, first_quote, last_quote, next, prev );
		top_quote--;

		DISPOSE( quote->author );
		DISPOSE( quote->text );
		DISPOSE( quote );
	}

	if ( top_quote != 0 )
		send_log( NULL, LOG_BUG, "free_all_quotes: top_quote non è 0 dopo aver liberato le citazioni: %d", top_quote );
}


/*
 * Restituisce il quote relativo al numero passato
 */
QUOTE_DATA *get_quote_from_num( const int num )
{
	QUOTE_DATA *quote;
	int			x = 0;

	for ( quote = first_quote;  quote;  quote = quote->next )
	{
		if ( num == x )
			return quote;

		x++;
	}

	return NULL;
}


/*
 * Invia un Quote
 */
void send_quote( CHAR_DATA *ch )
{
	QUOTE_DATA *quote;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_quote: ch è NULL" );
		return;
	}

	quote = get_quote_from_num( number_range(0, top_quote-1) );

	send_to_char( ch, "--------------------------------------------------------------------------------\r\n" );

	send_to_char( ch, quote->text );
	if ( VALID_STR(quote->author) )
		ch_printf( ch, "                              %s", quote->author );		/* (FF) in futuro fare una funzioni di allineamento destro del test per questo */

	send_to_char( ch, "\r\n--------------------------------------------------------------------------------\r\n\r\n" );
}
