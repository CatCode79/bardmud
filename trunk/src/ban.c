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


/*****************************************************************************\
 >                            Ban module by Shaddai                          <
\*****************************************************************************/


#include <ctype.h>

#include "mud.h"
#include "ban.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"


/*
 * Definizione della struttura di Ban
 */
typedef struct	ban_data			BAN_DATA;


/*
 * Definizione del file di Ban
 */
#define BAN_FILE	SYSTEM_DIR "ban.txt"


/*
 * Struttura di un Ban
 */
struct ban_data
{
	BAN_DATA *next;
	BAN_DATA *prev;
	int		  type;			/* Tipo: classe, razza, sito			*/
	char	 *name;			/* Nome del sito/classe/razza bannata/o	*/
	char	 *note;			/* Perchè è stato bannato				*/
	char	 *ban_by;		/* Chi ha bannato questo sito			*/
	char	 *ban_time;		/* Data in cui è stato bannato			*/
	int		  flag;			/* Numero di classe or razza			*/
	int		  unban_date;	/* Quando il ban scade					*/
	int		  duration;		/* Quanto tempo è stato bannato			*/
	int		  trust;		/* Fiducia di chi ha bannato			*/
	bool	  warn;			/* Echo per il canale warn				*/
	bool	  prefix;		/* Prefisso generico per *site			*/
	bool	  suffix;		/* Suffisso generico per site*			*/
};


/*
 * Variabili
 */
BAN_DATA	*first_ban	= NULL;
BAN_DATA	*last_ban	= NULL;
int			 top_ban	= 0;


/*
 * Libera dalla memoria un ban
 */
void free_ban( BAN_DATA *ban )
{
	if ( !ban )
	{
		send_log( NULL, LOG_BUG, "free_ban: ban passato è NULL" );
		return;
	}

	DISPOSE( ban->name );
	DISPOSE( ban->note );
	DISPOSE( ban->ban_by );
	DISPOSE( ban->ban_time	);
	DISPOSE( ban );
}

/*
 * Libera un ban dalla memorai e dalla lista
 */
void remove_ban( BAN_DATA *ban )
{
	if ( !ban )
	{
		send_log( NULL, LOG_BUG, "remove_ban: ban passato è NULL" );
		return;
	}

	UNLINK( ban, first_ban, last_ban, next, prev );
	top_ban--;

	free_ban( ban );
}


/*
 * Libera dalla memoria tutti i ban
 */
void free_all_bans( void )
{
	BAN_DATA *ban;

	for ( ban = first_ban;  ban;  ban = first_ban )
		remove_ban( ban );

	if ( top_ban != 0 )
		send_log( NULL, LOG_BUG, "free_all_bans: top_ban diverso da zero dopo aver liberato i ban: %d", top_ban );
}


/*
 * Carica un Ban
 */
void fread_ban( MUD_FILE *fp )
{
	BAN_DATA *pban;
	int		  x;

	CREATE( pban, BAN_DATA, 1 );
	pban->type	 = -1;

	for ( ; ; )
	{
		char	*word;

		if ( feof(fp->file) )
		{
			send_log( fp, LOG_FREAD, "fread_ban: fine del file prematuro nella lettura" );
			exit( EXIT_FAILURE );
		}

		word = fread_word( fp );
		if ( word[0] == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		if		( !str_cmp(word, "BanBy"	 ) )	pban->ban_by		=	fread_string( fp );
		else if ( !str_cmp(word, "BanTime"	 ) )	pban->ban_time		=	fread_string( fp );
		else if ( !str_cmp(word, "Duration"	 ) )	pban->duration		=	fread_number( fp );
		else if ( !str_cmp(word, "End"		 ) )	break;
		else if ( !str_cmp(word, "Name"		 ) )	pban->name		 	=	fread_string( fp );
		else if ( !str_cmp(word, "Note"		 ) )	pban->note			=	fread_string( fp );
		else if ( !str_cmp(word, "SitePrefix") )	pban->prefix		=	fread_bool( fp );		/* Valore di BAN_SITE */
		else if ( !str_cmp(word, "SiteSuffix") )	pban->suffix		=	fread_bool( fp );		/* Valore di BAN_SITE */
		else if ( !str_cmp(word, "Type"		 ) )	pban->type			=	fread_number( fp );
		else if ( !str_cmp(word, "Trust"	 ) )	pban->trust			=	fread_number( fp );
		else if ( !str_cmp(word, "UnbanDate" ) )	pban->unban_date	=	fread_number( fp );
		else if ( !str_cmp(word, "Warn"		 ) )	pban->warn			=	fread_bool( fp );
		else
			send_log( fp, LOG_FREAD, "fread_ban: etichetta non trovata %s.", word );
	} /* chiude il for */

	/* Controllo sul valore type */
	if ( pban->type == -1 || pban->type < BAN_SITE || pban->type > BAN_RACE )
	{
		send_log( NULL, LOG_FREAD, "fread_ban: typo di ban errato al ban %s: %d",
		(pban->name)  ?  pban->name  :  "(sconociuto)", pban->type  );
		free_ban( pban );
		return;
	}
	else if ( pban->type == BAN_CLASS )		/* Need to lookup the class number if it is of that type */
	{
		x = get_class_num( pban->name, TRUE );

		if ( x == -1  )
		{
			/* The file is corrupted throw out this ban structure */
			send_log( fp, LOG_FREAD, "fread_ban: bad class structure" );
			free_ban( pban );
			return;
		}
	}
	else if ( pban->type == BAN_RACE )		/* Need to lookup the race number if it is of that type */
	{
		x = get_race_num( pban->name, TRUE );
		if ( x == -1 )
		{
			/* The file is corrupted throw out this ban structure */
			send_log( fp, LOG_FREAD, "fread_ban: bad race structure" );
			free_ban( pban );
			return;
		}
	}

	LINK( pban, first_ban, last_ban, next, prev );
	top_ban++;
}


/*
 * Carica tutti i Ban
 */
void load_bans( void )
{
	fread_section( BAN_FILE, "BAN", fread_ban, FALSE );
}


/*
 * Saves all bans, for sites, class and races.
 */
void save_bans( void )
{
	BAN_DATA	*pban;
	MUD_FILE	*fp;

	fp = mud_fopen( "", BAN_FILE, "w", TRUE );
	if ( !fp )
		return;

	/* Print out all the site bans */
	for ( pban = first_ban;  pban;  pban = pban->next )
	{
		fprintf( fp->file, "BAN\n" );
		fprintf( fp->file, "Type       %d\n",	pban->type );
		fprintf( fp->file, "Name       %s~\n",	pban->name );
		fprintf( fp->file, "Trust      %d\n",	pban->trust );
		fprintf( fp->file, "Duration   %d\n",	pban->duration );
		fprintf( fp->file, "UnbanDate  %d\n",	pban->unban_date );
		fprintf( fp->file, "SitePrefix %s\n",	(pban->type == BAN_SITE)  ?  print_bool(pban->prefix)  :  "False" );
		fprintf( fp->file, "SiteSuffix %s\n",	(pban->type == BAN_SITE)  ?  print_bool(pban->suffix)  :  "False" );
		fprintf( fp->file, "Warn       %s\n",	print_bool(pban->warn) );
		fprintf( fp->file, "BanBy      %s~\n",	pban->ban_by );
		fprintf( fp->file, "BanTime    %s~\n",	pban->ban_time );
		fprintf( fp->file, "Note       %s~\n",	pban->note );
		fprintf( fp->file, "End\n\n"		);
	}

	fprintf( fp->file, "END\n" );		/* File must have an END even if empty */

	MUD_FCLOSE( fp );
}


/*
 * This actually puts the new ban into the proper linked
 *	list and initializes its data.
 */
int add_ban( CHAR_DATA *ch, char *arg1, char *arg2, int bantime, int type )
{
	BAN_DATA  *pban;
	BAN_DATA  *temp;
	struct tm *tms;
	char	  *name;
	char	   arg[MSL];
	char	   buf[MSL];
	int		   trust;
	int		   value;

	/* Should we check to see if they have dropped link sometime in between
	 * writing the note and now?  Not sure but for right now we won't since
	 * do_ban checks for that. */
	switch ( ch->substate )
	{
	  default:
		send_log( NULL, LOG_BUG, "add_ban: illegal substate" );
		return 0;

	  case SUB_RESTRICTED:
		send_to_char( ch, "You cannot use this command from within another command.\r\n" );
		return 0;

	  case SUB_NONE:	/* rifare il sistema da livelli a trust */
	  {
		one_argument( arg1, arg );
		smash_tilde ( arg );		/* Make sure the admins don't put a ~ in it. */

		if ( !VALID_STR(arg) || !VALID_STR(arg2) )
			return 0;

		if ( is_number(arg2) )
		{
			trust = atoi( arg2 );
			if ( trust < 0 || trust >= MAX_TRUST )
			{
				ch_printf( ch, "Trust range is from 0 to %d.\r\n", MAX_TRUST-1 );
				return 0;
			}
		}
		else if ( !str_cmp_all(arg2, FALSE)								)	trust = MAX_TRUST-1;
		else if ( !str_cmp(arg2, "mortale") || !str_cmp(arg2, "mortal") )	trust = TRUST_PLAYER;
		else if ( !str_cmp(arg2, "warn"   )								)	trust = BAN_WARN;
		else
		{
			send_log( NULL, LOG_BUG, "add_ban: bad string for flag in add_ban." );
			return 0;
		}

		switch ( type )
		{
		  case BAN_CLASS:
			if ( !VALID_STR(arg) )
				return 0;

			if ( is_number(arg) )
				value = atoi( arg );
			else
				value = get_class_num( arg, FALSE );

			if ( value < 0 || value >= MAX_CLASS )
			{
				send_to_char( ch, "Unknown class.\r\n" );
				return 0;
			}

			for ( temp = first_ban;  temp;  temp = temp->next )
			{
				if ( temp->flag == value )
				{
					if ( temp->trust == trust )
					{
						send_to_char( ch, "That entry already exists.\r\n" );
						return 0;
					}
					else
					{
						temp->trust = trust;
						if ( temp->trust == BAN_WARN )
							temp->warn = TRUE;

						sprintf( buf, "%21.21s", friendly_ctime(&current_time) );
						DISPOSE( temp->ban_time );
						temp->ban_time = str_dup( buf );

						DISPOSE( temp->ban_by );
						temp->ban_by = str_dup( ch->name );

						send_to_char( ch, "Updated entry.\r\n" );
						save_bans( );
						return 1;
					}
				}
			} /* chiude il for */

			CREATE( pban, BAN_DATA, 1 );
			pban->type	 = type;
			pban->name	 = str_dup( table_class[value]->name_male );
			pban->flag	 = value;
			pban->trust	 = trust;
			pban->ban_by = str_dup( ch->name );
			LINK( pban, first_ban, last_ban, next, prev );
			break;

		  case BAN_RACE:
			if ( is_number(arg) )
				value = atoi( arg );
			else
				value = get_race_num( arg, FALSE );

			if ( value < 0 || value >= MAX_RACE )
			{
				send_to_char( ch, "Unknown race.\r\n" );
				return 0;
			}

			for ( temp = first_ban;  temp;  temp = temp->next )
			{
				if ( temp->flag == value )
				{
					if ( temp->trust == trust )
					{
						send_to_char( ch, "That entry already exists.\r\n" );
						return 0;
					}
					else
					{
						temp->trust = trust;
						if ( temp->trust == BAN_WARN )
							temp->warn = TRUE;

						sprintf( buf, "%21.21s", friendly_ctime(&current_time) );
						DISPOSE( temp->ban_time );
						temp->ban_time = str_dup( buf );

						DISPOSE( temp->ban_by );
						temp->ban_by = str_dup( ch->name );

						send_to_char( ch, "Updated entry.\r\n" );
						save_bans( );
						return 1;
					}
				}
			} /* chiude il for */

			CREATE( pban, BAN_DATA, 1 );
			pban->type	 = type;
			pban->name	 = str_dup( table_race[value]->name_male );
			pban->flag	 = value;
			pban->trust	 = trust;
			pban->ban_by = str_dup( ch->name );
			LINK( pban, first_ban, last_ban, next, prev );
			break;

		  case BAN_SITE:
		  {
			bool	 prefix	   = FALSE;
			bool	 suffix	   = FALSE;

			name = arg;
			if ( name[0] == '*' )
			{
				prefix = TRUE;
				name++;
			}

			if ( name[strlen(name)-1] == '*' )
			{
				suffix = TRUE;
				name[strlen(name)-1] = '\0';
			}

			for ( temp = first_ban;  temp;  temp = temp->next )
			{
				if ( str_cmp(temp->name, name) )
					continue;

				if ( temp->trust == trust
				  && (prefix && temp->prefix)
				  && (suffix && temp->suffix) )
				{
					send_to_char( ch, "That entry already exists.\r\n" );
					return 0;
				}
				else
				{
					temp->suffix = suffix;
					temp->prefix = prefix;

					if ( temp->trust == BAN_WARN )
						temp->warn = TRUE;
					temp->trust = trust;

					sprintf( buf, "%21.21s", friendly_ctime(&current_time) );
					DISPOSE( temp->ban_time );
					temp->ban_time = str_dup( buf );

					DISPOSE( temp->ban_by );
					temp->ban_by = str_dup( ch->name );

					send_to_char( ch, "Updated entry.\r\n" );
					save_bans( );
					return 1;
				}
			} /* chiude il for */
			CREATE( pban, BAN_DATA, 1 );
			pban->type = type;
			pban->ban_by = str_dup( ch->name );
			pban->suffix = suffix;
			pban->prefix = prefix;
			pban->name = str_dup( name );
			pban->trust = trust;

			LINK( pban, first_ban, last_ban, next, prev );
			break;
		  } /* chiude il case */

		  default:
			send_log( NULL, LOG_BUG, "add_ban: bad type in add_ban: %d.", type );
			return 0;
		} /* chiude lo switch */

		sprintf( buf, "%21.21s", friendly_ctime(&current_time) );
		DISPOSE( pban->ban_time );
		pban->ban_time = str_dup( buf );
		if ( bantime > 0 )
		{
			pban->duration = bantime;
			tms = localtime( &current_time );
			tms->tm_mday += bantime;
			pban->unban_date = mktime( tms );
		}
		else
		{
			pban->duration	 = -1;
			pban->unban_date = -1;
		}

		if ( pban->trust == BAN_WARN )
			pban->warn = TRUE;

		ch->substate = SUB_BAN_DESCR;
		ch->dest_buf = pban;

		if ( !pban->note )
			pban->note = str_dup ("");;

		start_editing( ch, pban->note );
		return 1;
	  } /* chiude il case */

	  case SUB_BAN_DESCR:
		pban = ch->dest_buf;
		if ( !pban )
		{
			send_log( NULL, LOG_BUG, "do_ban: sub_ban_desc: NULL ch->dest_buf" );
			ch->substate = SUB_NONE;
			return 0;
		}

		if ( pban->note )
			DISPOSE( pban->note );

		pban->note = copy_buffer( ch );
		stop_editing( ch );
		ch->substate = ch->tempnum;
		save_bans( );

		if ( pban->duration > 0 )
			ch_printf( ch, "%s banned for %d days.\r\n", pban->name, pban->duration );
		else
			ch_printf (ch, "%s banned forever.\r\n", pban->name );

		return 1;
	} /* chiude lo switch */

	return 1;
}


/*
 * Print the bans out to the screen.
 */
void show_bans( CHAR_DATA *ch, int type )
{
	BAN_DATA *pban;
	int		  bnum;

	set_char_color( AT_ADMIN, ch );

	switch ( type )
	{
	  case BAN_SITE:
		send_to_pager( ch, "Banned sites:\r\n" );
		send_to_pager( ch, "[ #] Warn (Lv) Time                     By              For   Site\r\n" );
		send_to_pager( ch, "---- ---- ---- ------------------------ --------------- ----  ---------------\r\n" );
		break;

	  case BAN_CLASS:
		send_to_pager( ch, "Banned races:\r\n" );
		send_to_pager( ch, "[ #] Warn (Lv) Time                     By              For   Race\r\n" );
		send_to_pager( ch, "---- ---- ---- ------------------------ --------------- ----  ---------------\r\n" );
		break;

	  case BAN_RACE:
		send_to_pager( ch, "Banned classes:\r\n" );
		send_to_pager( ch, "[ #] Warn (Lv) Time                     By              For   Class\r\n" );
		send_to_pager( ch, "---- ---- ---- ------------------------ --------------- ----  ---------------\r\n" );
		break;

	  default:
		send_log( NULL, LOG_BUG, "show_bans: bad type %d", type );
		break;
	}

	set_char_color( AT_PLAIN, ch );

	for ( bnum = 1, pban = first_ban;  pban;  bnum++, pban = pban->next )
	{
		if ( pban->type != type )
			continue;

		switch ( type )
		{
		  case BAN_SITE:
			pager_printf (ch, "[%2d] %-2s (%2d) %-24s %-15s %4d  %c%s%c\r\n",
				bnum,
				(pban->warn) ? "SI" : "no",
				pban->trust,
				pban->ban_time,
				pban->ban_by,
				pban->duration,
				(pban->prefix)  ?  '*'  :  ' ',
				pban->name,
				(pban->suffix)  ?  '*'  :  ' ' );
			break;

		  case BAN_CLASS:
		  case BAN_RACE:
			pager_printf (ch, "[%2d] %-2s (%2d) %-24s %-15s %4d  %s\r\n",
				bnum,
				(pban->warn)  ?  "SI"  :  "no",
				pban->trust,
				pban->ban_time,
				pban->ban_by,
				pban->duration,
				pban->name );
			break;
		}
	}
}


/*
 * Messaggio di sintassi per il comando do_ban.
 */
void show_syntax_ban( CHAR_DATA *ch )
{
	send_to_char( ch, "&YSintassi&w: ban sito   <address> <type> <duration>\r\n" );
	send_to_char( ch, "      ban razza  <race>    <type> <duration>\r\n"		 );
	send_to_char( ch, "      ban class    <class>     <type> <duration>\r\n"	 );
	send_to_char( ch, "      ban mostra <field>   <number>\r\n"					 );
	send_to_char( ch, "Ban site lists current bans.\r\n"						 );
	send_to_char( ch, "Duration is the length of the ban in days.\r\n"			 );
	send_to_char( ch, "Type can be:  newbie, mortal, all, warn or trust.\r\n"	 );
	send_to_char( ch, "In ban show, the <field> is site, race or class,"		 );
	send_to_char( ch, "  and the <number> is the ban number.\r\n"				 );
}


/*
 * The main command for ban, lots of arguments so be carefull what you change here.
 */
DO_RET do_ban( CHAR_DATA *ch, char *argument )
{
	BAN_DATA *pban = NULL;
	char	 *temp;
	char	  arg1[MIL];
	char	  arg2[MIL];
	char	  arg3[MIL];
	char	  arg4[MIL];
	int		  value = 0;
	int		  bantime;

	if ( IS_MOB(ch) )	/* Don't want mobs banning sites ;) */
	{
		send_to_char( ch, "Monsters are too dumb to do that!\r\n" );
		return;
	}

	if ( ch->desc == NULL )		/* No desc means no go :) */
	{
		send_log( NULL, LOG_BUG, "do_ban: no descriptor" );
		return;
	}

	set_char_color( AT_ADMIN, ch );
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );
	argument = one_argument( argument, arg4 );

	/* Do we have a time duration for the ban? */
	if ( VALID_STR(arg4) && is_number(arg4) )
		bantime = atoi( arg4 );
	else
		bantime = -1;

	/* -1 is default, but no reason the time should be greater than 1000 or less
	 *	than 1, after all if it is greater than 1000 you are talking around 3 years */

	if ( bantime != -1 && (bantime < 1 || bantime > 1000) )
	{
		send_to_char( ch, "Time value is -1 (forever) or from 1 to 1000.\r\n" );
		return;
	}

	/*
	 * Need to be carefull with sub-states or everything will get messed up.
	 */
	switch ( ch->substate )
	{
	  default:
		send_log( NULL, LOG_BUG, "do_ban: illegal substate" );
		return;

	  case SUB_RESTRICTED:
		send_to_char( ch, "You cannot use this command from within another command.\r\n" );
		return;

	  case SUB_NONE:
		ch->tempnum = SUB_NONE;
		break;

	  /* Returning to end the editing of the note */
	  case SUB_BAN_DESCR:
		add_ban( ch, "", "", 0, 0 );
		return;
	}

	if ( !VALID_STR(arg1) )
		show_syntax_ban( ch );

	/* If no args are sent after the class/site/race, show the current banned items. */
	if ( !str_cmp(arg1, "sito") || !str_cmp(arg1, "site") )
	{
		if ( !VALID_STR(arg2) )
		{
			show_bans( ch, BAN_SITE );
			return;
		}

		if ( !VALID_STR(arg3) )
			show_syntax_ban( ch );

		if ( !add_ban(ch, arg2, arg3, bantime, BAN_SITE) )
			return;
	}
	else if ( !str_cmp(arg1, "razza") || !str_cmp(arg1, "race") )
	{
		if ( !VALID_STR(arg2) )
		{
			show_bans( ch, BAN_RACE );
			return;
		}

		if ( !VALID_STR(arg3) )
			show_syntax_ban( ch );

		if ( !add_ban(ch, arg2, arg3, bantime, BAN_RACE) )
			return;
	}
	else if ( !str_cmp(arg1, "classe") || !str_cmp(arg1, "class") )
	{
		if ( !VALID_STR(arg2) )
		{
			show_bans( ch, BAN_CLASS );
			return;
		}

		if ( !VALID_STR(arg3) )
			show_syntax_ban( ch );

		if ( !add_ban(ch, arg2, arg3, bantime, BAN_CLASS ) )
			return;
	}
	else if ( !str_cmp(arg1, "mostra") || !str_cmp ( arg1, "show" ) )
	{
		int type;	/* per mostrare il tipo di ban scelto */

		/* This will show the note attached to a ban */
		if ( !VALID_STR(arg2) || !VALID_STR(arg3) )
			show_syntax_ban( ch );

		temp = arg3;

		if ( arg3[0] == '#' )		/* Use #1 to show the first ban*/
		{
			temp = arg3;
			temp++;
			if ( !is_number(temp) )
			{
				send_to_char( ch, "Which ban # to show?\r\n" );
				return;
			}
			value = atoi( temp );

			if ( value < 1 )
			{
				send_to_char( ch, "You must specify a number greater than 0.\r\n" );
				return;
			}
		}

		if ( !str_cmp(arg2, "sito") || !str_cmp(arg2, "site") )
		{
			type = BAN_SITE;

			if ( temp[0] == '*' )
				temp++;

			if ( temp[strlen(temp)-1] == '*' )
				temp[strlen(temp)-1] = '\0';
		}
		else if ( !str_cmp(arg2, "classe") || !str_cmp(arg2, "class") )
			type = BAN_CLASS;
		else if ( !str_cmp(arg2, "razza") || !str_cmp (arg2, "race") )
			type = BAN_RACE;
		else
		{
			show_syntax_ban( ch );
			return;
		}

		for ( pban = first_ban;  pban;  pban = pban->next )
		{
			if ( pban->type != type )
				continue;

			if ( value == 1 || !str_cmp(pban->name, temp) )
				break;
			else if ( value > 1 )
				value--;
		}

		if ( !pban )
		{
			send_to_char( ch, "No such ban.\r\n" );
			return;
		}

		ch_printf	( ch, "Banned by: %s\r\n", pban->ban_by );
		send_to_char( ch, pban->note );

		return;
	}
	else
	{
		show_syntax_ban( ch );
	}
}


/*
 * Visualizza il messaggio di sintassi per il comando allow.
 */
void show_syntax_allow( CHAR_DATA *ch )
{
	send_to_char( ch, "&YSintassi&w: allow site  <address>\r\n"	);
	send_to_char( ch, "          allow race <race>\r\n"			);
	send_to_char( ch, "          allow class <class>\r\n"		);
}


/*
 * Allow a already banned site/class or race
 */
DO_RET do_allow( CHAR_DATA *ch, char * argument )
{
	BAN_DATA *pban;
	char	  arg1[MIL];
	char	  arg2[MIL];
	char	 *temp = NULL;
	bool	  fMatch = FALSE;
	int		  value = 0;

	if ( IS_MOB (ch) )	/* No mobs allowing sites */
	{
		send_to_char( ch, "Monsters are too dumb to do that!\r\n" );
		return;
	}

	if ( !ch->desc )	/* No desc is a bad thing */
	{
		send_log( NULL, LOG_BUG, "do_allow: no descriptor" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	set_char_color( AT_ADMIN, ch );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
		show_syntax_allow( ch );

	if ( arg2[0] == '#' )	/* Use #1 to ban the first ban in the list specified */
	{
		temp = arg2;
		temp++;
		if ( !is_number( temp ) )
		{
			send_to_char( ch, "Which ban # to allow?\r\n" );
			return;
		}
		value = atoi(temp);
	}

	if ( !str_cmp(arg1, "sito") || !str_cmp(arg1, "site")
	  || !str_cmp(arg1, "razza") || !str_cmp(arg1, "race")
	  || !str_cmp(arg1, "classe") || !str_cmp(arg1, "class") )
	{
		int type;

		if ( !str_cmp(arg1, "sito") || !str_cmp(arg1, "site") )
		{
			type = BAN_SITE;

			if ( !value )
			{
				if ( strlen(arg2) < 2 )
				{
					send_to_char( ch, "You have to have at least 2 chars for a ban\r\n" );
					send_to_char( ch, "If you are trying to allow by number use #\r\n" );
					return;
				}

				temp = arg2;

				if ( arg2[0] == '*' )
					temp++;

				if ( temp[strlen(temp) - 1] == '*' )
					temp[strlen(temp) - 1] = '\0';
			}
		}
		else if ( !str_cmp(arg1, "razza") || !str_cmp(arg1, "race") )
		{
			type = BAN_RACE;
			arg2[0] = toupper( arg2[0] );
		}
		else
		{
			type = BAN_CLASS;
			arg2[0] = toupper( arg2[0] );
		}

		for ( pban = first_ban;  pban;  pban = pban->next )
		{
			if ( pban->type != type )
				continue;

			/* Need to make sure we dispose properly of the ban_data
			 * or memory problems will be created. */
			if ( value == 1 || !str_cmp(pban->name, temp) )
			{
				fMatch = TRUE;
				remove_ban( pban );
				break;
			}
			if ( value > 1 )
				value--;
		}
	}
	else
		show_syntax_allow( ch );

	if ( fMatch )
	{
		save_bans( );
		ch_printf( ch, "%s is now allowed.\r\n", arg2 );
	}
	else
		ch_printf( ch, "%s was not banned.\r\n", arg2 );
}


/*
 * Visualizza il messaggio di sintassi per il comando warn.
 */
void show_syntax_warn( CHAR_DATA *ch )
{
	send_to_char( ch, "&YSintassi&w:  warn class <field>\r\n"						);
	send_to_char( ch, "           warn razza <field>\r\n"							);
	send_to_char( ch, "           warn sito  <field>\r\n"							);
	send_to_char( ch, "Field is either #(ban_number) or the site/class/race.\r\n"	);
	send_to_char( ch, "&YEsempio&w:  warn class #1\r\n"								);
}


/*
 *  Sets the warn flag on bans.
 */
DO_RET do_warn( CHAR_DATA *ch, char *argument )
{
	BAN_DATA   *pban = NULL;
	char	   *name;
	char		arg1[MSL];
	char		arg2[MSL];
	int			count = -1;
	int			type;

	/*
	 * Don't want mobs or link-deads doing this.
	 */
	if ( IS_MOB (ch) )
	{
		send_to_char( ch, "Monsters are too dumb to do that!\r\n" );
		return;
	}

	if ( !ch->desc )
	{
		send_log( NULL, LOG_BUG, "do_warn: no descriptor" );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
		show_syntax_warn(ch);

	if ( arg2[0] == '#' )
	{
		name = arg2;
		name++;
		if ( !is_number(name) )
			show_syntax_warn( ch );

		count = atoi( name );

		if ( count < 1 )
		{
			send_to_char( ch, "The number has to be above 0.\r\n" );
			return;
		}
	}

	/*
	 *  We simply set up which ban list we will be looking at here.
	 */
	if		( !str_cmp(arg1, "classe")	|| !str_cmp(arg1, "class")	)	{ type = BAN_CLASS;		arg2[0] = toupper( arg2[0] ); }	/* occhio a non perdere per strada le parentesti grafe qui dell'if come sotto */
	else if ( !str_cmp(arg1, "razza")	|| !str_cmp(arg1, "race")	)	{ type = BAN_RACE;		arg2[0] = toupper( arg2[0] ); }
	else if ( !str_cmp(arg1, "sito"	)	|| !str_cmp(arg1, "site")	)	type = BAN_SITE;
	else
	{
		show_syntax_warn( ch );
		return;
	}

	for ( pban  = first_ban;  pban && count != 0;  count--, pban = pban->next )
	{
		if ( count == -1 && !str_cmp(pban->name, arg2) )
			break;
	}

	if ( pban )
	{
		/* If it is just a warn delete it, otherwise remove the warn flag. */
		if ( pban->warn )
		{
			if ( pban->trust == BAN_WARN )
			{
				remove_ban( pban );
				send_to_char( ch, "Warn has been deleted.\r\n" );
			}
			else
			{
				pban->warn = FALSE;
				send_to_char( ch, "Warn turned off.\r\n" );
			}
		}
		else
		{
			pban->warn = TRUE;
			send_to_char( ch, "Warn turned on.\r\n" );
		}
		save_bans( );
	}
	else
	{
		ch_printf( ch, "%s was not found in the ban list.\r\n", arg2 );
	}
}


bool check_expire( BAN_DATA *pban )
{
	if ( pban->unban_date < 0 )
		return FALSE;

	if ( pban->unban_date <= current_time )
	{
		send_log( NULL, LOG_WARNING, "%s ban has expired.", pban->name );
		return TRUE;
	}
	return FALSE;
}


/*
 * Check for totally banned sites.
 * Need this because we don't have a char struct yet.
 */
bool check_total_bans( DESCRIPTOR_DATA *d )
{
	BAN_DATA *pban;
	char	  new_host[MSL];
	int		  x;

	for ( x = 0;  x < (int) strlen( d->host );  x++ )
		new_host[x] = tolower( d->host[x] );

	new_host[x] = '\0';

	for ( pban = first_ban;  pban;  pban = pban->next )
	{
		if ( pban->type != BAN_SITE )
			continue;

		if ( pban->trust != TRUST_NEOMASTER )	/* (TT) provare al limite ad alzare la trust? */
			continue;

		if ( pban->prefix && pban->suffix
		  && strstr(pban->name, new_host) )
		{
			if ( check_expire(pban) )
			{
				remove_ban( pban );
				save_bans( );
				return FALSE;
			}
			else
				return TRUE;
		}

		if ( pban->suffix && !str_prefix(pban->name, new_host) )
		{
			if ( check_expire(pban) )
			{
				remove_ban( pban );
				save_bans( );
				return FALSE;
			}
			else
				return TRUE;
		}

		if ( pban->prefix && !str_suffix(pban->name, new_host) )
		{
			if ( check_expire(pban) )
			{
				remove_ban( pban );
				save_bans( );
				return FALSE;
			}
			else
				return TRUE;
		}

		if ( !str_cmp(pban->name, new_host) )
		{
			if ( check_expire(pban) )
			{
				remove_ban( pban );
				save_bans( );
				return FALSE;
			}
			else
				return TRUE;
		}
	}

	return FALSE;
}


/*
 * The workhose, checks for bans on sites/classes and races.
 */
bool check_bans( CHAR_DATA *ch, int type )
{
	BAN_DATA *pban;
	char	  new_host[MSL];
	int		  x;
	bool	  fMatch = FALSE;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "check_bans: ch è NULL" );
		return FALSE;
	}

	/* Controllo sul type passato */
	if ( type < BAN_SITE || type > BAN_RACE )
	{
		send_log( NULL, LOG_BUG, "check_bans: bad type %d.", type );
		return FALSE;
	}

	if ( IS_MOB(ch) )
	{
		send_log( NULL, LOG_BUG, "check_bans: ch è un mob: %s #%d", ch->short_descr, ch->pIndexData->vnum );
		return FALSE;
	}

	if ( type == BAN_SITE )
	{
		for ( x = 0;  x < (int) (strlen(ch->desc->host));  x++ )
			new_host[x] = tolower( ch->desc->host[x] );

		new_host[x] = '\0';
	}

	for ( pban = first_ban;  pban;  pban = pban->next )
	{
		if ( pban->type != type )
			continue;

		if ( type == BAN_SITE )
		{
			if		( pban->prefix && pban->suffix && strstr(pban->name, new_host) )	fMatch = TRUE;
			else if ( pban->prefix && !str_suffix(pban->name, new_host) )				fMatch = TRUE;
			else if ( pban->suffix && !str_prefix(pban->name, new_host) )				fMatch = TRUE;
			else if ( !str_cmp(pban->name, new_host) )									fMatch = TRUE;
		}

		if ( (type == BAN_CLASS && pban->flag == ch->class)
		  || (type == BAN_RACE	&& pban->flag == ch->race)
		  || (type == BAN_SITE	&& fMatch == TRUE) )
		{
			if ( check_expire(pban) )
			{
				remove_ban( pban );
				save_bans( );
				return FALSE;
			}

			if ( get_trust(ch) > pban->trust )
			{
				if ( pban->warn )
					send_log( NULL, LOG_WARNING, "check_bans: %s %s si connette da %s.",
						(type == BAN_SITE)  ?  "sito"  :  (type == BAN_CLASS)  ?  "classe"  :  "razza",
						pban->name, ch->desc->host );
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
