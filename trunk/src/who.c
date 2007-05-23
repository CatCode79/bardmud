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
 >								Modulo del Who								<
\****************************************************************************/

#include <unistd.h>

#include "mud.h"
#include "db.h"
#include "editor.h"
#include "interpret.h"
#include "mccp.h"
#include "nanny.h"
#include "olc.h"


/*
 * Shows adms separately.
 */

/*
 * do_who output structure
 */
typedef struct who_data			WHO_DATA;
typedef struct who_group_data	WHO_GROUP_DATA;


struct who_group_data
{
	WHO_GROUP_DATA	*next;
	WHO_GROUP_DATA	*prev;
	CHAR_DATA		*ch;
	bool			 follower;
	bool			 sameip;
};


struct who_data
{
	WHO_DATA	*prev;
	WHO_DATA	*next;
	char		*text;
	int			 type;
};


char *get_who_line( CHAR_DATA *ch, bool adminview, bool ident, bool sameip )
{
	static char	buf[MSL];
	char		align[8];		/* Carattere che serve al buf_status */
	char		buf_status[MIL];
	char		buf_race  [MIL];
	char		buf_name  [MIL];
	char		buf_class [MIL];
	char		buf_level [MIL];
	char		buf_incog [MIL];
	char		buf_temp  [MIL];

	align	  [0] = '\0';
	buf_status[0] = '\0';
	buf_race  [0] = '\0';
	buf_class [0] = '\0';
	buf_level [0] = '\0';
	buf_incog [0] = '\0';
	buf_temp  [0] = '\0';

	/* Stato */
	if		( is_align(ch, ALIGN_GOOD) )	strcpy( align, "&WG" );
	else if ( is_align(ch, ALIGN_NEUTRAL) )	strcpy( align, "&wN" );
	else if ( is_align(ch, ALIGN_EVIL) )	strcpy( align, "&zE" );

	sprintf( buf_status, "%s%s%s%s%s%s%s%s",
		(HAS_BIT_PLR(ch, PLAYER_AFK) || HAS_BIT_PLR(ch, PLAYER_AFK2))		? "&CL"	: " ",	/* lontano dalla tastiera */
		(ch->desc && (is_editing(ch->desc) || is_in_olc(ch->desc)))	? "&BS"	: " ",	/* editando */
		(!ch->desc && !ch->switched)								? "&OD" : " ",
		(adminview)													? align : " ",
		(adminview && HAS_BIT_PLR(ch, PLAYER_ATTACKER))				? "&rA"	: " ",
		(adminview && HAS_BIT_PLR(ch, PLAYER_KILLER))					? "&RK"	: " ",
		(adminview && HAS_BIT_PLR(ch, PLAYER_THIEF))					? "&OT"	: " ",
		(adminview && HAS_BIT_PLR(ch, PLAYER_LITTERBUG))		   		? "&YB"	: " " );

	/* Razza */
	sprintf( buf_race, "%s%*s", get_race(ch, TRUE), 13 - strlen_nocolor(get_race(ch, TRUE)), "" );

	/* Nome */
	if ( !ident )
		sprintf( buf_name, "%s%-12s", (adminview && sameip) ? "&R" : "", ch->name );
	else
		sprintf ( buf_name, "|_%s%-10s", (adminview && sameip) ? "&R" : "", ch->name );

	if ( adminview )
	{
		/* Classe */
		switch ( get_trust(ch) )
		{
		  default:
				send_log( NULL, LOG_BUG, "get_who_line: trust inesistende: %d", get_trust(ch) );
				sprintf( buf_class, " (errore) " );
				break;
		  case TRUST_PLAYER:
	  			sprintf( buf_class, " %-9.9s&w", get_class(ch, TRUE) );
	  			break;
		  case TRUST_IMPLE:			sprintf( buf_class, " &YImple  &w" );	break;
		  case TRUST_BUILDER:		sprintf( buf_class, " &YBuilder&w" );	break;
		  case TRUST_NEOBUILDER:	sprintf( buf_class, " &YNeoBuil&w" );	break;
		  case TRUST_MASTER:		sprintf( buf_class, " &YMaster &w" );	break;
		  case TRUST_NEOMASTER:		sprintf( buf_class, " &YNeoMast&w" );	break;
		}

		/* Livello */
		sprintf( buf_level, " %3d", ch->level );	/* niente get_level() qui */

		/* Incognito */
		sprintf( buf_incog, " %3d", ch->pg->incognito );

		/* Client - Protocol */
		sprintf( buf_temp, " Client &W%s", "NC" );	/* (bb) per ora lo disattivo per quel baco del client */
		strcat( buf_temp, " &wProt: " );
#ifdef T2_MCCP
		sprintf( buf_temp+strlen(buf_temp), "%c", (use_mccp(ch->desc)) ? 'C' : ' ' );
#else
		strcat( buf_temp, " " );
#endif
	}

	/* (bb) nel who degli admin coloro che hanno title con molti colori sballa la visualizzazione, è perché hano la stringa del title più lunga del campo, è un bachetto poco correggibile a meno di troncare il resto della stringa*/
	sprintf( buf, "%s %s &w%s&w%s%*s&w%s%s%s%s\r\n",
		buf_status,
		buf_race,
		buf_name,
		ch->pg->title,
		MAX_LEN_TITLE - strlen_nocolor(ch->pg->title),	/* Spazi dopo il title, serve per non sballare con i colori */
		"",
		buf_class,
		buf_level,
		buf_incog,
		buf_temp );

	return buf;
}


/*
 * Funzione che crea un po' di ordine, fa i check del for
 */
bool for_checks( CHAR_DATA *sch, CHAR_DATA *wch )
{
	if ( IS_MOB(wch) )
		return TRUE;

	if ( wch->desc
	  && wch->desc->connected != CON_PLAYING
	  && wch->desc->connected != CON_EDITING
	  && wch->desc->connected != CON_MEDIT
	  && wch->desc->connected != CON_OEDIT
	  && wch->desc->connected != CON_REDIT )
	{
		return TRUE;
	}

	if ( !IS_ADMIN(sch) )
	{
		if ( wch->pg->incognito > sch->level )
			return TRUE;
		if ( IS_ADMIN(wch) && HAS_BIT_PLR(wch, PLAYER_INVISWHO) )
			return TRUE;
	}

	if ( wch->desc && wch->desc->original )
		return TRUE;

	return FALSE;
}


/*
 * Visualizza il footer ai due comandi di who
 */
void who_footer( CHAR_DATA *ch, char *buf_ld, const int nMatch, bool group )
{
	char clr;

	if ( IS_ADMIN(ch) && VALID_STR(buf_ld) )
		pager_printf( ch, "LinkDead:%s\r\n", buf_ld );

	if ( get_trust(ch) == TRUST_IMPLE )
		pager_printf( ch, "PID %d\r\n", getpid() );

	clr = random_color( );
	pager_printf( ch, "\r\n&%c[ &W%s &wha &W%d &wAvventurieri%s &%c]",
		clr, MUD_NAME, nMatch, group ? " in gruppo" : "",
		clr );

	send_to_pager( ch, "&w\r\n" );
}


/*
 * Comando che permette di visualizzare
 */
DO_RET do_whogroups( CHAR_DATA *ch, char *argument )
{
	WHO_GROUP_DATA	*first_who = NULL;
	WHO_GROUP_DATA	*last_who = NULL;
	WHO_GROUP_DATA	*stwho;
	WHO_GROUP_DATA	*scwho;
	WHO_GROUP_DATA	*scwho_next;
	CHAR_DATA		*wch;
	int				 nMatch = 0;
	char			 buf_ld[MIL];	/* informazioni di link death */

	buf_ld[0] = '\0';

	send_log( NULL, LOG_WHO, "do_who: %s %s", ch->name, argument );

	for ( wch = last_player;  wch;  wch = wch->prev_player )
	{
		if ( for_checks(ch, wch) )
			continue;

		if ( !wch->desc )
		{
			if ( !wch->switched )
				sprintf( buf_ld+strlen(buf_ld), " %s(%d)", wch->name, wch->timer );
		}

		if ( wch->leader && wch->leader != wch )
			continue;

		CREATE( stwho, WHO_GROUP_DATA, 1 );
		nMatch++;
		stwho->ch = wch;
		stwho->follower = FALSE;
		stwho->sameip = FALSE;
		LINK( stwho, first_who, last_who, next, prev );
	}

	if ( first_who )
	{
		for ( wch = last_player;  wch;  wch = wch->prev_player )
		{
			if ( for_checks(ch, wch) )
				continue;

			if ( !wch->leader || wch->leader == wch )
				continue;

			for ( scwho = first_who;  scwho;  scwho = scwho->next )
			{
				if ( wch->leader == scwho->ch )
				{
					CREATE( stwho, WHO_GROUP_DATA, 1 );
					stwho->ch = wch;
					stwho->sameip = FALSE;
					stwho->follower = TRUE;
					nMatch++;
					if ( scwho->next )
					{
						scwho = scwho->next;
						INSERT( stwho, scwho, first_who, next, prev );
					}
					else
					{
						LINK ( stwho, first_who, last_who, next, prev );
					}
					break;
				}
			}
		}

		/* adesso abbiamo una lista con all'interno i gruppi in ordine decente */

		for ( scwho = first_who;  scwho;  )
		{
			if ( !scwho->follower && (!scwho->next || !scwho->next->follower) )
			{
				WHO_GROUP_DATA	*tmpstwho;

				tmpstwho = scwho->next;
				UNLINK( scwho, first_who, last_who, next, prev );
				scwho = tmpstwho;
				nMatch--;
			}
			else
			{
				scwho = scwho->next;
			}
		}

		/* controllo rapido all'interno del gruppo se ci sono ip uguali */
		for ( scwho = first_who;  scwho;  scwho = scwho->next )
		{
			if ( !scwho->next )
				continue;

			for ( stwho = scwho->next;  stwho && stwho->follower;  stwho = stwho->next )
			{
				if ( is_same_ip(scwho->ch, stwho->ch) )
				{
					stwho->sameip = TRUE;
					scwho->sameip = TRUE;
				}
			}
		}
	}

	pager_printf( ch, "\r\n&w --=&g/><\\&w=-=&G/><\\&w=-=&g/><\\&w=--  &WGruppi &wsu %s  &w--=&g/><\\&w=-=&G/><\\&w=-=&g/><\\&w=--\r\n", MUD_NAME );

	if ( nMatch > 0 )
	{
		if ( IS_ADMIN(ch) )
			send_to_pager( ch, "Stato    Razza         Nome         Titolo                         Classe  Lvl Inc\r\n" );
		else
			send_to_pager( ch, "Stato    Razza         Nome         Titolo\r\n" );
	}

	for ( scwho = first_who;  scwho;  scwho = scwho_next )
	{
		send_to_pager( ch, get_who_line(scwho->ch, (IS_ADMIN(ch) ? TRUE : FALSE), scwho->follower, scwho->sameip) );
		
		scwho_next = scwho->next;
		DISPOSE( scwho );
	}

	who_footer( ch, buf_ld, nMatch, TRUE );
}


/*
 * 	Defines for new do_who
 */
#define WT_PLAYER	0
#define WT_ADMIN	1

DO_RET do_who( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*wch;
	WHO_DATA	*cur_who	 = NULL;
	WHO_DATA	*next_who	 = NULL;
	WHO_DATA	*first_pg	 = NULL;
	WHO_DATA	*first_admin = NULL;
	char		 buf_ld[MIL];	/* informazioni di link death */
	int			 nMatch = 0;

	buf_ld[0] = '\0';

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "do_who: ch è NULL" );
		return;
	}

	if ( VALID_STR(argument) )
	{
		if ( is_name_prefix(argument, "gruppo gruppi groups") )
			do_whogroups( ch, argument );
	}

	send_log( NULL, LOG_WHO, "do_who: %s %s", ch->name, argument );

	/* Start from last to first to get it in the proper order */
	buf_ld[0] = '\0';

	for ( wch = last_player;  wch;  wch = wch->prev_player )
	{
		if ( !wch->desc )
		{
			if ( !wch->switched )
				sprintf( buf_ld+strlen(buf_ld), " %s(%d)", wch->name, wch->timer );
		}

		if ( for_checks(ch, wch) )
			continue;

		/* First make the structure */
		CREATE( cur_who, WHO_DATA, 1 );
		cur_who->text = str_dup( get_who_line(wch, IS_ADMIN(ch), FALSE, FALSE) );
		if ( get_trust(wch) >= TRUST_NEOMASTER )	/* Non usa IS_ADMIN per mantenere gli admin nella loro sezione di who evitando il check della PLAYER_NOADMIN */
			cur_who->type = WT_ADMIN;
		else
			cur_who->type = WT_PLAYER;

		nMatch++;

		/* Then put it into the appropriate list */
		switch ( cur_who->type )
		{
		  default:
			send_log( NULL, LOG_BUG, "do_who: tipo di who errato: %d", cur_who->type );
			cur_who->next = first_pg;	/* Fa finta che sia un player */
			first_pg = cur_who;
			break;
		  case WT_PLAYER:
			cur_who->next = first_pg;
			first_pg = cur_who;
			break;
		  case WT_ADMIN:
			cur_who->next = first_admin;
			first_admin = cur_who;
			break;
		}
	} /* chiude il for */


	/* Visualizza il who dei personaggi non Admin */
	if ( first_pg )
	{
		pager_printf( ch, "\r\n&w --=&g/><\\&w=-=&G/><\\&w=-=&g/><\\&w=--  &WGiocatori &wsu %s  &w--=&g/><\\&w=-=&G/><\\&w=-=&g/><\\&w=--\r\n", MUD_NAME );
		if ( IS_ADMIN(ch) )
			send_to_pager( ch, "Stato    Razza         Nome         Titolo                         Classe  Lvl Inc\r\n" );
		else
			send_to_pager( ch, "Stato    Razza         Nome         Titolo\r\n" );
	}

	for ( cur_who = first_pg;  cur_who;  cur_who = next_who )
	{
		send_to_pager( ch, cur_who->text );

		next_who = cur_who->next;
		DISPOSE( cur_who->text );
		DISPOSE( cur_who );
	}

	/* Visualizza gli Admin */
	if ( first_admin )
	{
		pager_printf( ch, "\r\n&O ==&Y={&C===============-   &WAmministratori &wdi %s   &C-===============&Y}=&O==&w\r\n", MUD_NAME );
		if ( IS_ADMIN(ch) )
			send_to_pager( ch, "Stato    Razza         Nome         Titolo                         Trust   Lvl Inc\r\n" );
		else
			send_to_pager( ch, "Stato    Razza         Nome         Titolo\r\n" );
	}

	for ( cur_who = first_admin;  cur_who;  cur_who = next_who )
	{
		send_to_pager( ch, cur_who->text );

		next_who = cur_who->next;
		DISPOSE( cur_who->text );
		DISPOSE( cur_who );
	}

	who_footer( ch, buf_ld, nMatch, FALSE );
}
