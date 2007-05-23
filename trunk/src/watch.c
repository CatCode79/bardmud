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
 > 						Modulo riguardante i watch							<
\****************************************************************************/

#include <ctype.h>
#include <errno.h>

#include "mud.h"
#include "command.h"
#include "db.h"
#include "fread.h"
#include "interpret.h"
#include "save.h"
#include "watch.h"


/*
 * Variabili esterne
 */
WATCH_DATA	*first_watch = NULL;
WATCH_DATA	*last_watch	 = NULL;
int			 top_watch	 = 0;


void load_watchlist( void )
{
	COMMAND_DATA *cmd;
	WATCH_DATA	 *pwatch;
	MUD_FILE	 *fp;
	int			  number;

	fp = mud_fopen( SYSTEM_DIR, WATCH_LIST, "r", TRUE );
	if ( !fp )
		exit( EXIT_FAILURE );

	for ( ; ; )
	{
		if ( feof(fp->file) )
		{
			send_log( fp, LOG_BUG, "load_watchlist: no -1 found." );
			MUD_FCLOSE( fp );
			return;
		}

		number = fread_number( fp );
		if ( number == -1 )
		{
			MUD_FCLOSE( fp );
			return;
		}

		CREATE( pwatch, WATCH_DATA, 1 );
		pwatch->adm_trust	= number;
		pwatch->admin_name	= fread_string( fp );
		pwatch->target_name	= fread_string( fp );

		if ( strlen(pwatch->target_name) < 2 )
			DISPOSE( pwatch->target_name );

		pwatch->player_site = fread_string( fp );

		if ( strlen(pwatch->player_site) < 2 )
			DISPOSE( pwatch->player_site );

		/* Check for command watches */
		if ( VALID_STR(pwatch->target_name) )
		{
			bool	found = FALSE;
			int		lang = IT;

			while ( !found && lang <= EN )
			{
				for ( cmd = (lang == IT) ? first_command_it : first_command_en;  cmd;  cmd = cmd->next )
				{
					if ( is_name(pwatch->target_name, cmd->name) )
					{
						SET_BIT( cmd->fun->flags, CMDFLAG_WATCH );
						found = TRUE;
						break;
					}
				}
			}

			if ( !found )
			{
				send_log( NULL, LOG_BUG, "load_watchlist: comando non trovato: %s", pwatch->target_name );
				DISPOSE( pwatch->admin_name );
				DISPOSE( pwatch->target_name );
				DISPOSE( pwatch->player_site );
				DISPOSE( pwatch );
				continue;
			}
		}

		LINK( pwatch, first_watch, last_watch, next, prev );
	} /* chiude il for */
}


void save_watchlist( void )
{
	WATCH_DATA *pwatch;
	MUD_FILE	   *fp;

	fp = mud_fopen( SYSTEM_DIR, WATCH_LIST, "w", TRUE );
	if ( !fp )
		return;

	for ( pwatch = first_watch;  pwatch;  pwatch = pwatch->next )
	{
		fprintf( fp->file, "%d %s~%s~%s~\n", pwatch->adm_trust, pwatch->admin_name,
			pwatch->target_name  ?  pwatch->target_name  :  " ",
			pwatch->player_site  ?  pwatch->player_site  :  " " );
	}
	fprintf( fp->file, "-1\n" );

	MUD_FCLOSE( fp );
}


/*
 * Libera dalla memoria tutti i watch
 */
void free_all_watchs( void )
{
	WATCH_DATA	*watch;

	for ( watch = first_watch;  watch;  watch = first_watch )
	{
		UNLINK( watch, first_watch, last_watch, next, prev );
		top_watch--;

		DISPOSE( watch->admin_name );
		DISPOSE( watch->target_name );
		DISPOSE( watch->player_site );
		DISPOSE( watch );
	}

	if ( top_watch != 0 )
		send_log( NULL, LOG_BUG, "free_all_watchs: top_watch non è 0 dopo aver liberato i watch: %d", top_watch );
}


/*
 * The "watch" facility allows adms to specify the name of a player or
 * the name of a site to be watched. It is like "logging" a player except
 * the results are written to a file in the "watch" directory named with
 * the same name as the adm. The idea is to allow lower level adms to
 * watch players or sites without having to have access to the log files.
 */
DO_RET do_watch( CHAR_DATA *ch, char *argument )
{
	WATCH_DATA *pw;
	char		arg1[MIL];
	char		arg2[MIL];

	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono watchare\r\n" );
		return;
	}

	set_char_color( AT_PLAIN, ch );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( is_name(arg1, "pulisci clear") )	/* Clear watch file */
	{
		char fname[MIL];

		sprintf( fname, "%s%s", WATCH_DIR, strlower(ch->name) );
		if ( remove(fname) == 0 )
		{
			send_to_pager( ch, "Ok. Your watch file has been cleared.\r\n" );
			return;
		}

		pager_printf( ch, "You have no valid watch file to clear. (errno: %s)\r\n", strerror(errno) );
		return;
	}
	else if ( is_name(arg1, "dimensioni size") )	/* Display size of watch file */
	{
		MUD_FILE *fp;
		char	  s[MSL];
		int		  rec_count = 0;

		fp = mud_fopen( WATCH_DIR, strlower(ch->name), "r", TRUE );
		if ( !fp )
			return;

		fgets( s, MSL, fp->file );
		while( !feof(fp->file) )
		{
			rec_count++;
			fgets( s, MSL, fp->file );
		}

		pager_printf( ch, "Vi sono %d linee nel tuo file di watch.\r\n", rec_count );
		MUD_FCLOSE( fp );
		return;
	}
	else if ( is_name(arg1, "stampa print") )	/* Print watch file */
	{
		MUD_FILE	 *fp;
		char	  s[MSL];
		const int MAX_DISPLAY_LINES = 1000;
		int		  start;
		int		  limit;
		int		  disp_count = 0;
		int		  rec_count = 0;

		if ( !VALID_STR(arg2) || !is_number(arg2) )
		{
			send_to_pager( ch, "Sorry. You must specify a starting line number.\r\n" );
			return;
		}

		start = atoi( arg2 );
		if ( !VALID_STR(argument) )
			limit = MAX_DISPLAY_LINES;
		else
			limit = atoi( argument );
		limit = UMIN( limit, MAX_DISPLAY_LINES );

		fp = mud_fopen( WATCH_DIR, strlower(ch->name), "r", TRUE );
		if ( !fp )
			return;

		fgets( s, MSL, fp->file );

		while ( (disp_count<limit) && (!feof(fp->file)) )
		{
			if ( ++rec_count >= start )
			{
				send_to_pager( ch, s );
				disp_count++;
			}
			fgets( s, MSL, fp->file );
		}
		send_to_pager( ch, "\r\n" );

		if ( disp_count >= MAX_DISPLAY_LINES )
		{
			send_to_pager( ch, "Maximum display lines exceeded. List is terminated.\r\n" );
			send_to_pager( ch, "Type 'help watch' to see how to print the rest of the list.\r\n\r\n" );
			send_to_pager( ch, "Your watch file is large. Perhaps you should clear it?\r\n" );
		}

		MUD_FCLOSE( fp );
		return;
	}
	else if ( get_trust(ch) == TRUST_IMPLE && is_name(arg1, "mostra show")
	  && !str_cmp_all(arg2, FALSE) )	/* Display all watches */
	{
		pager_printf( ch, "%-12s %-14s %-15s\r\n",
			"Adm Name", "Player/Command", "Player Site" );

		if ( first_watch )
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( get_trust(ch) >= pw->adm_trust )
				{
					pager_printf( ch, "%-14s %-12s %-15s\r\n",
						pw->admin_name,
						pw->target_name  ?  pw->target_name  :  " ",
						pw->player_site  ?  pw->player_site  :  " " );
				}
			}
		}
		return;
	}
	else if ( is_name(arg1, "mostra show")
	  && !VALID_STR(arg2) )	/* Display only those watches belonging to the requesting adm */
	{
		int cou = 0;
		pager_printf( ch, "%-3s %-12s %-14s %-15s\r\n",
			" ", "Adm Name", "Player/Command", "Player Site");

		if ( first_watch )
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( !str_cmp(ch->name, pw->admin_name) )
				{
					pager_printf( ch, "%3d %-12s %-14s %-15s\r\n",
						++cou,
						pw->admin_name,
						pw->target_name ? pw->target_name : " ",
						pw->player_site ? pw->player_site : " " );
				}
			}
		}
		return;
	}
	else if ( is_name(arg1, "distruggi delete")
	  && isdigit(arg2[0]) )		/* Delete a watch belonging to the requesting adm */
	{
		int cou = 0;
		int num;

		num = atoi( arg2 );
		if ( first_watch )
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( !str_cmp(ch->name, pw->admin_name) )
				{
					if ( num == ++cou )
					{
						/* Oops someone forgot to clear up the memory */
						if ( pw->admin_name	 )		DISPOSE( pw->admin_name	 );
						if ( pw->player_site )		DISPOSE( pw->player_site );
						if ( pw->target_name )		DISPOSE( pw->target_name );

						/* Now we can unlink and then clear up that final pointer */
						UNLINK( pw, first_watch, last_watch, next, prev );
						DISPOSE( pw );
						save_watchlist( );
						send_to_pager( ch, "Deleted.\r\n" );
						return;
					}
				}
			}
		}

		send_to_pager( ch, "Sorry. I found nothing to delete.\r\n" );
		return;
	}
	else if ( is_name(arg1, "giocatore pg player")
	  && VALID_STR(arg2) )		/* Watch a specific player */
	{
		WATCH_DATA *pinsert;
		CHAR_DATA  *vic;

		if ( first_watch )				/* check for dups */
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( !str_cmp(ch->name, pw->admin_name)
				  && VALID_STR(pw->target_name)
				  && !str_cmp(arg2, pw->target_name) )
				{
					send_to_pager( ch, "You are already watching that player.\r\n" );
					return;
				}
			}
		}

		CREATE( pinsert, WATCH_DATA, 1 );		/* create new watch */
		pinsert->adm_trust	  = get_trust( ch );
		pinsert->admin_name	  = str_dup( strlower(ch->name) );
		pinsert->target_name  = str_dup( strlower(arg2) );
		pinsert->player_site  = NULL;

		vic = get_player_mud( ch, arg2, TRUE );
		if ( vic )
		{
			SET_BIT( vic->pg->flags, PLAYER_WATCH );
			save_player( vic );
		}
		else
		{
			send_to_char( ch , "Questo giocatore non esiste.\r\n" );
			return;
		}

		if ( first_watch )							/* ins new watch if app */
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( strcmp(pinsert->admin_name, pw->admin_name) < 0 )
				{
					INSERT( pinsert, pw, first_watch, next, prev );
					save_watchlist( );
					send_to_pager( ch, "Ok. That player will be watched.\r\n" );
					return;
				}
			}
		}

		LINK( pinsert, first_watch, last_watch, next, prev );  /* link new watch */
		save_watchlist( );
		send_to_pager( ch, "Ok. That player will be watched.\r\n" );
		return;
	}
	else if ( is_name(arg1, "sito site")
	  && VALID_STR(arg2) )		/* Watch a specific site */
	{
		WATCH_DATA *pinsert;
		CHAR_DATA  *vic;

		if ( first_watch )					/* check for dups */
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( !str_cmp(ch->name, pw->admin_name)
				  && pw->player_site
				  && !str_cmp(arg2, pw->player_site) )
				{
					send_to_pager( ch, "You are already watching that site.\r\n" );
					return;
				}
			}
		}

		CREATE( pinsert, WATCH_DATA, 1 );		/* create new watch */
		pinsert->adm_trust   = get_trust( ch );
		pinsert->admin_name    = str_dup( strlower( ch->name ) );
		pinsert->player_site = str_dup( strlower( arg2 ) );
		pinsert->target_name = NULL;

		for ( vic = first_player;  vic;  vic = vic->next_player )
		{
			if ( !vic->desc )							continue;
			if ( get_trust(vic) >= pinsert->adm_trust )	continue;
			if ( !VALID_STR(pinsert->player_site) )		continue;

			if ( !str_prefix(pinsert->player_site, vic->desc->host) )
				SET_BIT( vic->pg->flags, PLAYER_WATCH );
		}

		if ( first_watch )						/* ins new watch if app */
		{
			for ( pw = first_watch;  pw;  pw = pw->next )
			{
				if ( strcmp(pinsert->admin_name, pw->admin_name) < 0 )
				{
					INSERT( pinsert, pw, first_watch, next, prev );
					save_watchlist( );
					send_to_pager( ch, "Ok. That site will be watched.\r\n" );
					return;
				}
			}
		}

		LINK( pinsert, first_watch, last_watch, next, prev );
		save_watchlist( );
		send_to_pager( ch, "Ok. That site will be watched.\r\n" );
		return;
	}
	else if ( is_name(arg1, "comando command")
	  && VALID_STR(arg2) )		/* Watch a specific command */
	{
		WATCH_DATA *pinsert;
		COMMAND_DATA   *cmd;

		for ( pw = first_watch;  pw;  pw = pw->next )
		{
			if ( !str_cmp(ch->name, pw->admin_name)
			  && VALID_STR(pw->target_name)
			  && !str_cmp(arg2, pw->target_name) )
			{
				send_to_pager( ch, "You are already watching that command.\r\n" );
				return;
			}
		}

		/* check exact a TRUE, cerca il comando in tutte le lingue */
		cmd = find_command( ch, arg2, TRUE, ALL );
		if ( !cmd )
		{
			send_to_pager( ch, "No such command exists.\r\n" );
			return;
		}

		SET_BIT( cmd->fun->flags, CMDFLAG_WATCH );

		CREATE( pinsert, WATCH_DATA, 1 );
		pinsert->adm_trust	 = get_trust( ch );
		pinsert->admin_name	 = str_dup( strlower(ch->name) );
		pinsert->player_site = NULL;
		pinsert->target_name = str_dup( arg2 );

		for ( pw = first_watch;  pw;  pw = pw->next )
		{
			if ( strcmp(pinsert->admin_name, pw->admin_name) < 0 )
			{
				INSERT( pinsert, pw, first_watch, next, prev );
				save_watchlist( );
				send_to_pager( ch, "Ok, That command will be watched.\r\n" );
				return;
			}
		}

		LINK( pinsert, first_watch, last_watch, next, prev );
		save_watchlist( );
		send_to_pager( ch, "Ok. That site will be watched.\r\n" );
		return;
	}
	else
	{
		set_char_color( AT_ADMIN, ch );

		send_to_pager( ch, "&YSintassi&w &YEsempio&w:\r\n" );
		send_to_pager( ch, "   watch show all          show all watches\r\n" );
		send_to_pager( ch, "   watch show              show all my watches\r\n" );
		send_to_pager( ch, "   watch size              show the size of my watch file\r\n" );
		send_to_pager( ch, "   watch player joe        add a new player watch\r\n" );
		send_to_pager( ch, "   watch site 2.3.123      add a new site watch\r\n" );
		send_to_pager( ch, "   watch command make      add a new command watch\r\n" );
		send_to_pager( ch, "   watch site 2.3.12       matches 2.3.12x\r\n" );
		send_to_pager( ch, "   watch site 2.3.12.      matches 2.3.12.x\r\n" );
		send_to_pager( ch, "   watch delete n          delete my nth watch\r\n" );
		send_to_pager( ch, "   watch print 500         print watch file starting at line 500\r\n" );
		send_to_pager( ch, "   watch print 500 1000    print 1000 lines starting at line 500\r\n" );
		send_to_pager( ch, "   watch clear             clear my watch file\r\n" );
	}
}


/*
 * Determine if this input line is eligible for writing to a watch file.
 * Non vogliamo scrivere i comandi di movimento direzionale
 */
bool valid_watch( CHAR_DATA *ch, char *logline )
{
	int		len = strlen( logline );
	char	c   = logline[0];

	if ( IS_MOB(ch) )
		return FALSE;

	/* Se il pg utilizza i comandi in italiano ignora le direzioni in italiano */
	if ( HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
	{
		if ( len == 1 && (c=='n' || c=='s' || c=='e' || c=='o' || c=='a' || c=='b') )
			return FALSE;
		if ( len == 2 && c == 'n' && (logline[1]=='e' || logline[1]=='o') )
			return FALSE;
		if ( len == 2 && c == 's' && (logline[1]=='e' || logline[1]=='o') )
			return FALSE;
	}
	else	/* Altrimenti ignora le direzione digitate in inglese */
	{
		if ( len == 1 && (c=='n' || c=='s' || c=='e'  || c=='w' || c=='u' || c=='d') )
			return FALSE;
		if ( len == 2 && c == 'n' && (logline[1]=='e' || logline[1]=='w') )
			return FALSE;
		if ( len == 2 && c == 's' && (logline[1]=='e' || logline[1]=='w') )
			return FALSE;
	}

	return TRUE;
}


/*
 * Write input line to watch files if a	licable
 */
void write_watch_files( CHAR_DATA *ch, COMMAND_DATA *cmd, char *logline )
{
	WATCH_DATA *pw;
	MUD_FILE	   *fp;
	char		buf[MSL];
	struct tm  *t = localtime( &current_time );

	if ( !first_watch )		/* no active watches */
		return;

	/* if we're watching a command we need to do some special stuff to avoid
	 *	duplicating log lines - relies upon watch list being sorted by adm name. */
	if ( cmd )
	{
		char *cur_adm;
		bool found;

		pw = first_watch;
		while ( pw )
		{
			found = FALSE;

			for ( cur_adm = pw->admin_name;  pw && !strcmp(pw->admin_name, cur_adm);  pw = pw->next )
			{
				/* (RR) Sistema Traduzione */
				if ( !found && ch->desc && get_trust(ch) < pw->adm_trust
				  && ((VALID_STR(pw->target_name) && !strcmp(cmd->name, pw->target_name))
				  || (pw->player_site
				  && !str_prefix(pw->player_site, ch->desc->host))) )
				{
					fp = mud_fopen(WATCH_DIR, strlower(pw->admin_name), "a+", TRUE );
					if ( !fp )
						return;

					sprintf( buf, "%.2d/%.2d %.2d:%.2d %s: %s\r\n",
						t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, ch->name, logline );
					fputs( buf, fp->file );
					MUD_FCLOSE( fp );
					found = TRUE;
				}
			}
		} /* chiude il while */
	}
	else
	{
		for ( pw = first_watch;  pw;  pw = pw->next )
		{
			if ( ((VALID_STR(pw->target_name) && !str_cmp(pw->target_name, ch->name))
			  || (pw->player_site
			  && !str_prefix(pw->player_site, ch->desc->host)) )
			  && get_trust(ch) < pw->adm_trust
			  && ch->desc )
			{
				fp = mud_fopen( WATCH_DIR, strlower(pw->admin_name), "a+", TRUE );
				if ( !fp )
					return;

				sprintf( buf, "%.2d/%.2d %.2d:%.2d %s: %s\r\n",
					t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, ch->name, logline );
				fputs( buf, fp->file );
				MUD_FCLOSE( fp );
			}
		} /* chiude il for */
		return;
	} /* chiude l'else */
}


/*
 * Determina se questo giocatore deve essere controllato.
 */
bool chk_watch( int player_level, char *player_name, char *player_site )
{
	WATCH_DATA *pw;

/*	send_log( NULL, LOG_NORMAL, "chk_watch entry: plev = %d pname = %s psite = %s",
		player_level, player_name, player_site );
*/
	if ( !first_watch )
		return FALSE;

	for ( pw = first_watch;  pw;  pw = pw->next )
	{
		if ( pw->target_name )
		{
			if ( !str_cmp(pw->target_name, player_name) && player_level < pw->adm_trust )
				 return TRUE;
		}
		else if ( pw->player_site )
		{
			if ( !str_prefix(pw->player_site, player_site) && player_level < pw->adm_trust )
				return TRUE;
		}
	}

	return FALSE;
}
