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


/****************************************************************************
 *                             Alias module                                 *
 ****************************************************************************/

/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com
 ******************************************************/


#include "mud.h"
#include "alias.h"
#include "interpret.h"


/*
 * Variabili
 */
int		top_alias;		/* Numero di alias in memoria */


/*
 * Cerca nella lista degli alias del pg quello corrispondente al comando passato
 */
static ALIAS_DATA *find_alias( CHAR_DATA *ch, const char *command, bool exact )
{
	ALIAS_DATA *pal;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "find_alias: ch è NULL" );
		return NULL;
	}

	if ( !VALID_STR(command) )
	{
		send_log( NULL, LOG_BUG, "find_alias: command passato non è valido" );
		return NULL;
	}

	if ( IS_MOB(ch) )
		return NULL;

	if ( exact == TRUE || exact == ALL )
	{
		for ( pal = ch->pg->first_alias;  pal;  pal = pal->next )
		{
			if ( !str_cmp(command, pal->name) )
				return pal;
		}
	}

	if ( exact == FALSE || exact == ALL )
	{
		for ( pal = ch->pg->first_alias;  pal;  pal = pal->next )
		{
			if ( !str_prefix(command, pal->name) )
				return pal;
		}
	}

	return NULL;
}


/*
 * Richiamata da interpret() controlla che il comando passato sia un alias
 */
bool check_alias( CHAR_DATA *ch, const char *command, char *argument, bool exact )
{
	ALIAS_DATA	*alias;
	char		 arg[MIL];

	alias = find_alias( ch, command, exact );
	if ( !alias )
		return FALSE;

	if ( !VALID_STR(alias->cmd) )
		return FALSE;

	snprintf( arg, MIL, "%s", alias->cmd );

	if ( ch->cmd_recurse == -1 || ++ch->cmd_recurse > 25 )
	{
		if ( ch->cmd_recurse != -1 )
		{
			send_to_char( ch, "Impossibile continuare il comando, è troppo ricorsivo.\r\n" );
			ch->cmd_recurse = -1;
		}

		return FALSE;
	}

	if ( VALID_STR(argument) )
	{
		strncat( arg, " ", MIL );
		strncat( arg, argument, MIL );
	}

	interpret( ch, arg );
	return TRUE;
}


/*
 * Slinka e libera dalla memoria un solo alias
 */
static void free_alias( CHAR_DATA *ch, ALIAS_DATA *alias )
{
	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "free_alias: ch passato è NULL" );
		return;
	}

	if ( !alias )
	{
		send_log( NULL, LOG_BUG, "free_alias: alias passato è NULL" );
		return;
	}

	UNLINK( alias, ch->pg->first_alias, ch->pg->last_alias, next, prev );
	top_alias--;

	DISPOSE( alias->name );
	DISPOSE( alias->cmd );
	DISPOSE( alias );
}


/*
 * Libera da tutta la memoria gli alias
 */
void free_aliases( CHAR_DATA *ch )
{
	ALIAS_DATA	*alias;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "free_aliase: ch passato è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	for ( alias = ch->pg->first_alias;  alias;  alias = ch->pg->first_alias )
		free_alias( ch, alias );
}


/*
 * Comando per aggiungere, modificare o rimuovere un Alias
 */
DO_RET do_alias( CHAR_DATA *ch, char *argument )
{
	ALIAS_DATA	*pal = NULL;
	char		 arg[MIL];

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_alias: ch passato è NULL" );
		return;
	}

	if ( IS_MOB(ch) )
		return;

	if ( strchr(argument, '~') )
	{
		send_to_char( ch, "Il comando non è accettabile, non puoi utilizzare il carattere '~'.\r\n" );
		return;
	}		

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_pager( ch, "&YPer crearlo&w:       &Galias &w<&gabbreviazione&w> <&gcomando&w>\r\n" );
		send_to_pager( ch, "&YPer distruggerlo&w:  &Galias &w<&gabbreviazione&w>\r\n\r\n" );

		if ( !ch->pg->first_alias )
		{
			send_to_char( ch, "Non hai nessun alias definito.\r\n" );
			return;
		}

		pager_printf( ch, "%-20s Quello che fa\r\n", "Alias" );
		for ( pal = ch->pg->first_alias;  pal;  pal = pal->next )
			pager_printf( ch, "%-20s %s\r\n", pal->name, pal->cmd );

		return;
	}

	if ( !VALID_STR(argument) )
	{
		pal = find_alias( ch, arg, FALSE );
		if ( pal )
		{
			free_alias( ch, pal );
			send_to_char( ch, "Alias distrutto.\r\n" );
		}
		else
			send_to_char( ch, "Questo alias non esiste.\r\n" );

		return;
   }

	pal = find_alias( ch, arg, FALSE );
	if ( !pal )
	{
		CREATE( pal, ALIAS_DATA, 1 );
		pal->name = str_dup( arg );
		pal->cmd  = str_dup( argument );
		LINK( pal, ch->pg->first_alias, ch->pg->last_alias, next, prev );
		top_alias++;
		send_to_char( ch, "Alias creato.\r\n" );
	}
	else
	{
		DISPOSE( pal->cmd );
		pal->cmd = str_dup( argument );
		send_to_char( ch, "Alias modificato.\r\n" );
	}
}
