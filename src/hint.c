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

/****************************************************************************************\
 >							Modulo relativo Sistema di Hint								<
\****************************************************************************************/

#include "mud.h"
#include "fread.h"
#include "hint.h"
#include "interpret.h"
#include "nanny.h"


/*
 * Definizioni
 */
#define	HINT_FILE	"../helps/hints.dat"	/* File degli Hint */
#define MAX_HINT	100						/* Massimo di hint caricabili */


/*
 * Variabili locali
 */
static	HINT_DATA	*first_hint = NULL;
static	HINT_DATA	*last_hint	= NULL;
		int			 top_hint	= 0;	/* Numero di hint effettivamente caricati */


/*
 * Legge una sezione hint dal file dei suggerimenti
 */
void fread_hint( MUD_FILE *fp )
{
	HINT_DATA  *hint;

	CREATE( hint, HINT_DATA, 1 );
	hint->level	= -1;
	hint->trust	= -1;

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_hint: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "End"	  ) )	break;
		else if ( !str_cmp(word, "Classes") )	hint->classes =		fread_code_bit( fp, CODE_CLASS );
		else if ( !str_cmp(word, "Races"  ) )	hint->races =		fread_code_bit( fp, CODE_RACE );
		else if ( !str_cmp(word, "Level"  ) )	hint->level =		fread_number( fp );
		else if ( !str_cmp(word, "Trust"  ) )	hint->trust =		code_code( fp, fread_word(fp), CODE_TRUST );
		else if ( !str_cmp(word, "Text"	  ) )	hint->text =		fread_string( fp );
		else
        {
        	send_log( fp, LOG_FREAD, "fread_hint: nessuna etichetta trovata per la parola %s", word );
			exit( EXIT_FAILURE );
        }
    }

	/*
	 * Una volta letto l'hint ne controlla i valori acquisiti
	 */

	if ( !VALID_STR(hint->text) )
	{
		send_log( fp, LOG_FREAD, "fread_hint: stringa di testo dell'hint vuota" );
		exit( EXIT_FAILURE );
	}

	/* Controlla la validità del livello dell'hint */
	if ( hint->level < 1 || hint->level > LVL_LEGEND )
	{
		send_log( fp, LOG_FREAD, "fread_hint: livello dell'hint errato" );
		exit( EXIT_FAILURE );
	}

	LINK( hint, first_hint, last_hint, next, prev );
	top_hint++;
}


/*
 * Carica e controlla tutti gli hint
 */
void load_hints( void )
{
	fread_section( HINT_FILE, "HINT", fread_hint, FALSE );

	if ( top_hint >= MAX_HINT )
	{
		send_log( NULL, LOG_FREAD, "load_hints: hint caricati: %d ha raggiunto o superato il massimo di hint caricabile: %d",
			top_hint, MAX_HINT );
		exit( EXIT_FAILURE );
	}
}


/*
 * Libera dalla memoria tutti gli hint
 */
void free_all_hints( void )
{
	HINT_DATA *hint;

	for ( hint = first_hint;  hint;  hint = first_hint )
	{
		DISPOSE( hint->text );
		destroybv( hint->classes );
		destroybv( hint->races );
		UNLINK( hint, first_hint, last_hint, next, prev );
		DISPOSE( hint );
		top_hint--;
	}

	if ( top_hint != 0 )
		send_log( NULL, LOG_BUG, "free_all_hints: top_hint non è 0 dopo aver liberato gli hint: %d", top_hint );
}


/*
 * Ritorna l'int corrispondente al numero passato
 */
HINT_DATA *get_hint_from_num( const int num )
{
	HINT_DATA *hint;
	int		   x = 0;

	for ( hint = first_hint;  hint;  hint = hint->next )
	{
		if ( num == x )
			return hint;

		x++;
	}

	return NULL;
}


/*
 * Visualizza un suggerimento
 */
void send_hint( CHAR_DATA *ch )
{
	HINT_DATA *hint;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "send_hint: ch è NULL" );
		return;
	}

	hint = get_hint_from_num( number_range(0, top_hint-1) );	/* Sceglie a caso un hint */

	/* Se il livello non è compatibile con quello del pg salta e non invia nulla */
	if ( get_level(ch) < hint->level )
		return;

	/* Salta se la fiducia dell'Hint non è compatibilecon quella del pg */
	if ( get_trust(ch) < hint->trust )
		return;

	/* Se il pg non è della razza adatta non lo invia */
	if ( !IS_EMPTY(hint->classes) && !HAS_BIT(hint->classes, ch->class) )
		return;

	/* Se il pg non è della classe adatta non lo invia */
	if ( !IS_EMPTY(hint->races) && !HAS_BIT(hint->races, ch->race) )
		return;

	/* Se si sta combattendo non li invia */
	if ( ch->fighting )
		return;

	/* Più è alto il livello e meno visualizza gli hint di basso livello di PLAYER */
	if ( hint->trust == TRUST_PLAYER && number_range(number_range(1, hint->level), LVL_LEGEND+1) < get_level(ch) )
		return;

	send_to_char( ch, "&w-&W[ &gSuggerimento &W]&w---------------------------------------------------------------\r\n" );
	send_to_char( ch, change_command_tag(hint->text, ch) );
	send_to_char( ch, "&w--------------------------------------------------------------------------------\r\n" );
}


/*
 * Update che controlla se inviare o no degli hint ai giocatori
 */
void update_hint( void )
{
	DESCRIPTOR_DATA *d;

	for ( d = first_descriptor;  d;  d = d->next )
	{
		if ( d->connected != CON_PLAYING )		continue;
		if ( !is_awake(d->character) )			continue;	/* Solo quando il pg è sveglio */

		/* Se ha attivo l'invio degli Hint ne visualizza uno */
		if ( HAS_BIT_PLR(d->character, PLAYER_HINT) )
			send_hint( d->character );
	}
}
