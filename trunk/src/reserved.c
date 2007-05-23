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


/* Modulo dei nomi riservati */

#include "mud.h"
#include "fread.h"


/*
 * Tipo di struttura Reserved
 */
typedef struct	reserve_data	RESERVE_DATA;


/*
 * File con la lista dei nomi riservati
 */
#define RESERVED_LIST	SYSTEM_DIR	"reserved.lst"	/* Lista nomi riservati						*/


/*
 * Struttura di un Reserved Name
 */
struct reserve_data
{
	RESERVE_DATA *next;
	RESERVE_DATA *prev;
	char		 *name;
};


/*
 * Variabili locali
 */
RESERVE_DATA	*first_reserved 	 = NULL;
RESERVE_DATA	*last_reserved		 = NULL;
int				 top_reserved		 = 0;


/*
 * Rebuilt from broken copy, but bugged - commented out for now
 */
void sort_reserved( RESERVE_DATA *pRes )
{
	RESERVE_DATA *res;

	if ( !pRes )
	{
		send_log( NULL, LOG_BUG, "sort_reserved: NULL pRes" );
		return;
	}

	for ( res = first_reserved;  res;  res = res->next )
	{
		if ( strcmp(pRes->name, res->name) <= 0 )
		{
			INSERT( pRes, res, first_reserved, next, prev );
			break;
		}
	}

	if ( !res )
		LINK( pRes, first_reserved, last_reserved, next, prev );
}


/*
 * Non supporta i commenti perché il carattere * viene utilizzato
 *	per questo: "*are" trova tutte le parole che iniziano finiscono per "are"
 */
void load_reserved( void )
{
	MUD_FILE		 *fp;

	fp = mud_fopen( "", RESERVED_LIST, "r", FALSE );
	if ( !fp )
		return;

	for ( ; ; )
	{
		RESERVE_DATA *res;
		char		 *word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_BUG, "load_reserved: End non trovata." );
			MUD_FCLOSE( fp );
			return;
		}

		word = fread_word( fp );

		if ( !str_cmp(word, "End") )
			break;

		CREATE( res, RESERVE_DATA, 1 );
		res->name = str_dup( word );
		top_reserved++;

		sort_reserved( res );
	}

	MUD_FCLOSE( fp );
}


void save_reserved( void )
{
	RESERVE_DATA *res;
	MUD_FILE		 *fp;

	fp = mud_fopen( "", RESERVED_LIST, "w", TRUE );
	if ( !fp )
		return;

	for ( res = first_reserved;  res;  res = res->next )
		fprintf( fp->file, "%s\n", res->name );
	fprintf( fp->file, "End\n" );

	MUD_FCLOSE( fp );
}


/*
 * Libera dalla memoria tutti i nomi riservati
 */
void free_all_reserveds( void )
{
	RESERVE_DATA *reserved;

	for ( reserved = first_reserved;  reserved;  reserved = first_reserved )
	{
		UNLINK( reserved, first_reserved, last_reserved, next, prev );
		top_reserved--;

		DISPOSE( reserved->name );
		DISPOSE( reserved );
	}

	if ( top_reserved != 0 )
		send_log( NULL, LOG_BUG, "free_all_reserveds: top_reserved non è 0 dopo aver liberato i nomi riservati: %d", top_reserved );
}


DO_RET do_reserve( CHAR_DATA *ch, char *argument )
{
	RESERVE_DATA *res;

	set_char_color( AT_PLAIN, ch );

	if ( !VALID_STR(argument) )
	{
		int wid = 0;

		send_to_char( ch, "-- Reserved Names --\r\n" );
		for ( res = first_reserved;  res;  res = res->next )
		{
			ch_printf( ch, "%c%-17s ",
				(*res->name == '*')  ?  '*'  :  ' ',
				(*res->name == '*')  ?  res->name+1  :  res->name );

			if ( (++wid % 4) == 0 )
				send_to_char( ch, "\r\n" );
		}
		if ( (wid % 4) != 0 )
			send_to_char( ch, "\r\n" );

		return;
	}

	for ( res = first_reserved;  res;  res = res->next )
	{
		if ( !str_cmp(argument, res->name) )
		{
			UNLINK( res, first_reserved, last_reserved, next, prev );
			DISPOSE( res->name );
			DISPOSE( res );
			save_reserved( );
			send_to_char( ch, "Name no longer reserved.\r\n" );
			return;
		}
	}

	CREATE( res, RESERVE_DATA, 1 );
	res->name = str_dup( argument );
	top_reserved++;
	sort_reserved( res );
	save_reserved( );
	ch_printf( ch, "Nome %s reso riservato.\r\n", argument );
}


/*
 * Purtroppo ha il supporto limitato per il carattere '*'
 *	sarebbe bene gestirlo in qualunque posto della parola. (FF) ma mi pare di aver creato una funzione str_exception che farebbe a questo caso
 */
bool is_reserved_name( char *name )
{
	RESERVE_DATA *res;

	for ( res = first_reserved;  res;  res = res->next )
	{
		if ( (res->name[0] == '*' && !str_infix(res->name+1, name) )
		  || !str_cmp(res->name, name) )
		{
			return TRUE;
		}
	}

	return FALSE;
}
