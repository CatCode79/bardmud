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
 >  The MUDprograms are heavily based on the original MOBprogram code that	<
 >  was written by N'Atas-ha.												<
\****************************************************************************/

#include <stdarg.h>

#include "mud.h"
#include "admin.h"
#include "affect.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "nanny.h"
#include "object_interact.h"
#include "morph.h"
#include "room.h"
#include "timer.h"


/*
 * Legge un comando del mud prog
 */
void fread_mudprog_command( MUD_FILE *fp, MPROG_COM_LIST **first_mpcom, MPROG_COM_LIST **last_mpcom )
{
	MPROG_COM_LIST	*mpcom;
#ifdef T2_ALFA
	COMMAND_DATA	*cmd = NULL;
	SOCIAL_DATA		*soc = NULL;
	char			 arg[MIL];
#endif
	char			 line[MIL];

	CREATE( mpcom, MPROG_COM_LIST, 1 );
	strcpy( line, fread_line(fp) );
	strcpy( line, strip_char(line, '\r') );
	strcpy( line, strip_char(line, '\n') );
	mpcom->com = str_dup( line );

#ifdef T2_ALFA
	one_argument( mpcom->com, arg );

	cmd = find_command( NULL, arg, TRUE );

	if ( !cmd )
		soc = find_social( NULL, arg, TRUE );

	if ( !cmd && !soc && !is_name(arg, "if else endif or and break continue") )		/* (FF) Check anche per la parola chiave silent */
	{
		send_log( fp, LOG_FREAD, "fread_mudprog_command: comando non trovato %s", arg );
		exit( EXIT_FAILURE );
	}
#endif

	LINK( mpcom, *first_mpcom, *last_mpcom, next, prev );
}


/*
 * This procedure is responsible for reading any in_file MUDprograms.
 */
void fread_mudprog( MUD_FILE *fp, MPROG_DATA **first_mprog, MPROG_DATA **last_mprog, bool save )
{
	MPROG_DATA	*mprg;
	MPROG_DATA	*smprg;		/* variabile per la ricerca dei muprog uguali */
	MPROG_DATA	*cmprg;		/* variabile per la ricerca dei muprog uguali */
	char		 letter;
	bool		 done = FALSE;

	if ( !save )
	{
		letter = fread_letter( fp );
		if ( letter != '>' )
		{
			send_log( fp, LOG_FREAD, "fread_mudprog: carattere di mud prog > mancante" );
			exit( EXIT_FAILURE );
		}
	}

	CREATE( mprg, MPROG_DATA, 1 );
	LINK( mprg, *first_mprog, *last_mprog, next, prev );

	while ( !done )
	{
		mprg->type = code_num( fp, fread_word(fp), CODE_MPTRIGGER );
		switch ( mprg->type )
		{
		  case MPTRIGGER_ERROR:
			send_log( fp, LOG_FREAD, "fread_mudprog: tipo di trigger di mud prog errato" );
			exit( EXIT_FAILURE );
			break;

		  default:
			/* Acquisisce l'argomento del trigger */
			mprg->arglist = fread_string( fp );

			/* Acquisisce riga per riga i comandi da inviare del mud prog fino alla ~ (CC) la ~ mom serve c'è la > o la | a fermare la lettura */
			while ( (letter = fread_letter(fp)) != '~' )
			{
				ungetc( letter, fp->file );
				fread_mudprog_command( fp, &mprg->first_mpcom, &mprg->last_mpcom );
			}

			letter = fread_letter( fp );
			switch ( letter )
			{
			  default:
				send_log( fp, LOG_FREAD, "fread_mudprog: carattere di chiusura mud prog errato: %c", letter );
				exit( EXIT_FAILURE );
				break;
			  case '>':		/* Se incontra un altro > prepara la lettura per il while del prossimo mud prog */
				CREATE( mprg->next, MPROG_DATA, 1 );
				LINK( mprg->next, *first_mprog, *last_mprog, next, prev );
				mprg = mprg->next;
				break;
			  case '|':
				done = TRUE;
				break;
			}
			break;
		} /* chiude lo switch */
	} /* chiude il while */

  	for ( cmprg = *first_mprog;  cmprg;  cmprg = cmprg->next )
  	{
	  	for ( smprg = *first_mprog;  smprg;  smprg = smprg->next )
	  	{
	  		if ( cmprg == smprg )				continue;
	  		if ( cmprg->type != smprg->type )	continue;

	  		if ( !strcmp(cmprg->arglist, smprg->arglist) )
			{
				send_log( fp, LOG_FREAD, "fread_mudprog: trovata etichetta trigger doppia: %s %s",
					code_name(fp, smprg->type, CODE_MPTRIGGER), smprg->arglist );
				exit( EXIT_FAILURE );
			}
		}
	}
}


/*
 * Scrive su file un mud prog
 * Abbisogna di una etichetta che lo preceda, tipo MudProg o NoMudProg
 */
void fwrite_mudprog( MUD_FILE *fp, MPROG_DATA *mprg )
{
	MPROG_COM_LIST	*mpcom;

	fprintf( fp->file, "%s %s~\n",
		code_name(fp, mprg->type, CODE_MPTRIGGER),
		mprg->arglist );

	for ( mpcom = mprg->first_mpcom;  mpcom;  mpcom = mpcom->next )
	{
		/* (FF) da aggiungere l'indent per gli if nestati */
		fprintf( fp->file, "%s\n", mpcom->com );
	}

	fputs( "~\n", fp->file );
}


/*
 * Libera la memoria da un mud prog
 */
void free_mudprog( MPROG_DATA *mudprog )
{
	MPROG_DATA		*mprg;
	MPROG_DATA		*mprg_next;

	if ( !mudprog )
		return;

	for ( mprg = mudprog;  mprg;  mprg = mprg_next )
	{
		MPROG_COM_LIST	*mpcom;
		MPROG_COM_LIST	*mpcom_next;

		mprg_next = mprg->next;

		DISPOSE( mprg->arglist );
		for ( mpcom = mprg->first_mpcom;  mpcom;  mpcom = mpcom_next )
		{
			mpcom_next = mpcom->next;

			DISPOSE( mpcom->com );
			DISPOSE( mpcom );
		}
		DISPOSE( mprg );
	}
}


/*
 * Copia una struttura di mudprog in un'altra
 */
MPROG_DATA *copy_mudprog( MPROG_DATA *source )
{
	MPROG_DATA		*dest;
	MPROG_COM_LIST	*mpcom;
	MPROG_COM_LIST	*mpcom_dest = NULL;

	if ( !source )
	{
		send_log( NULL, LOG_BUG, "copy_mudprog: mudprog source passato è NULL" );
		return NULL;
	}

	CREATE( dest, MPROG_DATA, 1 );
	dest->type 		= source->type;
	dest->arglist	= str_dup( source->arglist );

	/* Copia la lista di comandi mudprog */
	CREATE( mpcom_dest, MPROG_COM_LIST, 1 );
	for ( mpcom = source->first_mpcom;  mpcom;  mpcom = mpcom->next )
	{
		mpcom_dest->com = str_dup( mpcom->com );
		LINK( mpcom_dest, dest->first_mpcom, dest->last_mpcom, next, prev );

		if ( mpcom->next )
		{
			CREATE( mpcom_dest->next, MPROG_COM_LIST, 1 );
			mpcom_dest = mpcom_dest->next;
		}
	}

	return dest;
}


/*
 * Confronta i due mprog e ritorna vero se uguali
 */
bool is_same_mudprog( MPROG_DATA *mprog1, MPROG_DATA *mprog2 )
{
	MPROG_COM_LIST	*mpcom1;
	MPROG_COM_LIST	*mpcom2;

	if ( !mprog1 )
	{
		send_log( NULL, LOG_BUG, "is_same_mudprog: mprog1 è NULL" );
		return FALSE;
	}

	if ( !mprog2 )
	{
		send_log( NULL, LOG_BUG, "is_same_mudprog: mprog2 è NULL" );
		return FALSE;
	}

	if ( mprog1->type != mprog2->type )
		return FALSE;

	if ( str_cmp(mprog1->arglist, mprog2->arglist) )
		return FALSE;

	mpcom1 = mprog1->first_mpcom;
	mpcom2 = mprog2->first_mpcom;
	while ( mpcom1 && mpcom2 )
	{
		if ( str_cmp(mpcom1->com, mpcom2->com) )
			return FALSE;

		mpcom1 = mpcom1->next;
		mpcom2 = mpcom2->next;
	}

	if ( mpcom1 || mpcom2 )
		return FALSE;

	return TRUE;
}


/*
 * Invia a ch i mudprog passati, se che ne sono
 */
void show_mudprog( CHAR_DATA *ch, MPROG_DATA *mudprog )
{
	MPROG_DATA *mprg;

	if ( !ch )
	{
		send_log( NULL, LOG_BUG, "show_mudprog: ch è NULL" );
		return;
	}

	if ( !mudprog )
	{
		send_to_char( ch, "Non ha nessun Mud Program.\r\n" );
		return;
	}

	for ( mprg = mudprog;  mprg;  mprg = mprg->next )
	{
		MPROG_COM_LIST	*mpcom;

		ch_printf( ch, ">%s %s~\r\n",
			code_name(NULL, mprg->type, CODE_MPTRIGGER),
			mprg->arglist );
		for ( mpcom = mprg->first_mpcom;  mpcom;  mpcom = mpcom->next )
			ch_printf( ch, "%s\r\n", mpcom->com );
		send_to_char( ch, "~\r\n" );
	}
}


/*
 * A trivial rehack of do_mstat.
 * This doesnt show all the data, but just enough to identify the mob and give
 *	its basic condition.
 * It does however, show the MUDprograms which are set.
 */
DO_RET do_mpstat( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	char		 arg[MIL];

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Vedere le statistiche di MobProg di chi?\r\n" );
		return;
	}

	victim = get_mob_mud( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Non c'è qui.\r\n" );
		return;
	}

	ch_printf( ch, "Nome: %s.  Vnum: %d.\r\n", victim->name, victim->pIndexData->vnum );

	ch_printf( ch, "Descrizione corta: %s.\r\nDescrizione lunga: %s",
		victim->short_descr,
		(VALID_STR(victim->long_descr))  ?  victim->long_descr  :  "(none).\r\n" );

	ch_printf( ch, "Life: %d/%d.  Mana: %d/%d.  Move: %d/%d  Soul: %d/%d.\r\n",
		victim->points[POINTS_LIFE],	victim->points_max[POINTS_LIFE],
		victim->points[POINTS_MANA],	victim->points_max[POINTS_MANA],
		victim->points[POINTS_MOVE],	victim->points_max[POINTS_MOVE],
		victim->points[POINTS_SOUL],	victim->points_max[POINTS_SOUL] );

	ch_printf( ch,
		"Liv: %d.  Classe: %d.  Allin: %d.  AC: %d.  Monete: %d.  Esper: %d.\r\n",
		victim->level,		victim->class,		victim->alignment,
		get_ac(victim),		victim->gold,		victim->exp );

	show_mudprog( ch, victim->pIndexData->first_mudprog );
}


/*
 * Opstat
 */
DO_RET do_opstat( CHAR_DATA *ch, char *argument )
{
	OBJ_DATA *obj;
	char	  arg[MIL];

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "OProg stat what?\r\n" );
		return;
	}

	if ( (obj = get_obj_world(ch, arg)) == NULL )
	{
		send_to_char( ch, "You cannot find that.\r\n" );
		return;
	}

	ch_printf( ch, "Name: %s.  Vnum: %d.\r\n",	 obj->name, obj->vnum );
	ch_printf( ch, "Short description: %s.\r\n", obj->short_descr );
	show_mudprog( ch, obj->first_mudprog );
}


/*
 * Rpstat
 */
DO_RET do_rpstat( CHAR_DATA *ch, char *argument )
{
	ch_printf( ch, "Name: %s  Vnum: %d\r\n", ch->in_room->name, ch->in_room->vnum );
	show_mudprog( ch, ch->in_room->first_mudprog );
}


/*
 * Displays the help screen for the "mpfind/opfind/rpfind" commands
 */
void mudprog_find_help( CHAR_DATA *ch, const char *type )
{
	char	command[MIL];

	if		( !str_cmp(type, "mob") )	strcpy( command, "mpfind" );
	else if ( !str_cmp(type, "obj") )	strcpy( command, "opfind" );
	else if ( !str_cmp(type, "room") )	strcpy( command, "rpfind" );
	else
	{
		send_log( NULL, LOG_BUG, "mudprog_find_help: tipo di ricerca mudprog passata errata: %s", type );
		return;
	}

	send_to_char( ch, "&YSintassi&w:\r\n" );
	ch_printf	( ch, "%s n lo_vnum hi_vnum text\r\n", command );
	ch_printf	( ch, "   Search %s vnums between lo_vnum and hi_vnum\r\n", type );
	ch_printf	( ch, "   for %sprogs that contain an occurrence of text.\r\n", type );
	send_to_char( ch, "   Display a maxiumu of n lines.\r\n\r\n" );

	ch_printf	( ch, "%s n mud text\r\n", command );
	ch_printf	( ch, "   Search all the %s in the mud for\r\n", type );
	ch_printf	( ch, "   %sprogs that contain an occurrence of text.\r\n", type );
	send_to_char( ch, "   Display a maxiumu of n lines.\r\n\r\n" );

	send_to_char( ch, "&YEsempio&w:\r\n" );
	ch_printf	( ch, "%s 20 901 969 if isnpc\r\n", command );
	ch_printf	( ch, "   Search all %s progs in Olympus (vnums 901 thru 969)\r\n", type );
	ch_printf	( ch, "   and display all %s that contain the text \"if isnpc\".\r\n", type );
	send_to_char( ch, "   Display a maximum of 20 lines.\r\n\r\n" );

	send_to_char( ch, "&YEsempio&w:\r\n" );
	ch_printf	( ch, "%s 100 mud mpslay\r\n", command );
	ch_printf	( ch, "   Search all %sprogs in the entire mud\r\n", type );
	ch_printf	( ch, "   and display all %s that contain the text \"mpslay\".\r\n", type );
	send_to_char( ch, "   Display a maximum of 100 lines.\r\n\r\n" );
}


/*
 * Gestisce i tre comando mpfind, opfind e rpfind in uno solo
 */
void handler_mudprog_find( CHAR_DATA *ch, char *argument, const char *type )
{
	MPROG_DATA		*pProg;
	char			 arg1[MIL];
	char			 arg2[MIL];
	char			 arg3[MIL];
	VNUM			 lo_vnum = 1;
	VNUM			 hi_vnum = MAX_VNUM-1;
	int				 tot_vnum;
	int				 tot_hits = 0;
	int				 x;
	int				 disp_cou = 0;
	int				 disp_limit;

	argument = one_argument( argument, arg1 );		/* display_limit */
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !is_number(arg1) )
	{
		mudprog_find_help( ch, type );
		return;
	}

	disp_limit = atoi (arg1);
	disp_limit = UMAX(0, disp_limit);

	if ( str_cmp(arg2, "mud") )
	{
		argument = one_argument( argument, arg3 );
		if ( !VALID_STR(arg3) || !VALID_STR(argument)
			|| !is_number(arg2) || !is_number(arg3) )
		{
			mudprog_find_help( ch, type );
			return;
		}
		else
		{
			lo_vnum = URANGE( 1, atoi(arg2), MAX_VNUM-1 );
			hi_vnum = URANGE( 1, atoi(arg3), MAX_VNUM-1 );
			if ( lo_vnum > hi_vnum )
			{
				mudprog_find_help( ch, type );
				return;
			}
		}
	}

	if ( !VALID_STR(argument) )
	{
		mudprog_find_help( ch, type );
		return;
	}

	for ( x = lo_vnum;  x <= hi_vnum;  x++ )
	{
		MOB_PROTO_DATA	*pMob;
		OBJ_PROTO_DATA	*pObj;
		ROOM_DATA		*pRoom;

		if ( !str_cmp(type, "mob") )
		{
			pMob = get_mob_index( NULL, x );
			pProg = pMob->first_mudprog;
			if ( !pMob || !pProg )
				continue;
		}
		else if ( !str_cmp(type, "obj") )
		{
			pObj = get_obj_proto( NULL, x );
			pProg = pObj->first_mudprog;
			if ( !pObj || !pProg )
				continue;
		}
		else if ( !str_cmp(type, "room") )
		{
			pRoom = get_room_index( NULL, x );
			pProg = pRoom->first_mudprog;
			if ( !pRoom || !pProg )
				continue;
		}
		else
		{
			send_log( NULL, LOG_BUG, "handler_mudprog_find: tipo di comando passato errato: %s", type );
			return;
		}

		tot_vnum = 0;
		for ( ;  pProg;  pProg = pProg->next )
		{
			MPROG_COM_LIST	*mpcom;

			for ( mpcom = pProg->first_mpcom;  mpcom;  mpcom = mpcom->next )
				tot_vnum = str_count( mpcom->com, argument );
		}

		tot_hits += tot_vnum;
		if ( tot_vnum && ++disp_cou <= disp_limit )
			pager_printf( ch, "%5d %5d %5d\r\n", disp_cou, x, tot_vnum );
	}

	pager_printf( ch, "Total: %10d\r\n", tot_hits );
}


/*
 * Search in mob, obj and rooms for room progs containing a specified text string
 */
DO_RET do_mpfind( CHAR_DATA *ch, char *argument )
{
	handler_mudprog_find( ch, argument, "mob" );
}

DO_RET do_opfind( CHAR_DATA *ch, char *argument )
{
	handler_mudprog_find( ch, argument, "obj" );
}

DO_RET do_rpfind( CHAR_DATA *ch, char *argument )
{
	handler_mudprog_find( ch, argument, "room" );
}


/*
 * Funzione di invio messaggio bug/log, fa da ponte per la send_log().
 */
void send_mplog( CHAR_DATA *mob, char *str, ... )
{
	va_list	args;
	char	buf[MSL*2];
	VNUM	vnum = mob->pIndexData  ?  mob->pIndexData->vnum  :  0;

	if ( !mob )
	{
		send_log( NULL, LOG_MPROG, "send_mplog: mob è NULL" );
		return;
	}

	if ( !VALID_STR(str) )
	{
		send_log( NULL, LOG_MPROG, "send_mplog: stringa str non è valida (con mob #%d)",
			vnum );
		return;
	}

	va_start( args, str );
	vsprintf( buf, str, args );
	va_end( args );

	/*
	 * Check if we're dealing with supermob, which means the bug occurred
	 * in a room or obj prog.
	 */
	if ( vnum == VNUM_MOB_SUPERMOB )
	{
		/*
		 * It's supermob.  In set_supermob and rset_supermob, the description
		 * was set to indicate the object or room, so we just need to show
		 * the description in the bug message.
		 */
		send_log( NULL, LOG_MPROG, "%s, %s",
			buf, (!VALID_STR(mob->description)) ? "(sconosciuto)" : mob->description );
	}
	else
	{
		send_log( NULL, LOG_MPROG, "%s, Mob #%d", buf, vnum );
	}
}


/*
 * Woowoo
 */
DO_RET do_mpasupress( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg1[MIL];
	char	   arg2[MIL];
	int		   rnds;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "do_mpasupress who?\r\n" );
		send_mplog( ch, "do_mpasupress:  invalid (nonexistent?) argument" );
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		send_to_char( ch, "Supress their attacks for how many rounds?\r\n" );
		send_mplog( ch, "do_mpasupress:  invalid (nonexistent?) argument" );
		return;
	}

	victim = get_char_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "No such victim in the room.\r\n" );
		send_mplog( ch, "do_mpasupress:  victim not present" );
		return;
	}

	rnds = atoi( arg2 );
	if ( rnds < 0 || rnds > 32000 )
	{
		send_to_char( ch, "Invalid number of rounds to supress attacks.\r\n" );
		send_mplog( ch, "do_mpsupress:  invalid (nonexistent?) argument" );
		return;
	}

	add_timer( victim, TIMER_ASUPRESSED, PULSE_VIOLENCE * rnds, NULL, 0 );
}


/*
 * Prints the message to all in the room other than the mob and victim.
 */
DO_RET do_mpechoaround( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	BITVECTOR	*actflags = NULL;
	char		 arg[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_mplog( ch, "mpechoaround: No argument" );
		return;
	}

	argument = one_argument( argument, arg );
	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "mpechoaround: victim does not exist" );
		return;
	}

	SET_BITS( actflags, ch->act );
	REMOVE_BIT( ch->act, MOB_SECRETIVE );

	act( AT_ACTION, argument, ch, NULL, victim, TO_NOVICT );

	SET_BITS( ch->act, actflags );
	destroybv( actflags );
}


/*
 * Prints message only to victim
 */
DO_RET do_mpechoat( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*victim;
	BITVECTOR	*actflags = NULL;
	char		 arg[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_mplog( ch, "mpechoat: No argument" );
		return;
	}

	argument = one_argument( argument, arg );
	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "mpechoat: victim does not exist" );
		return;
	}

	SET_BITS( actflags, ch->act );
	REMOVE_BIT( ch->act, MOB_SECRETIVE );

	if ( !VALID_STR(argument) )
		act( AT_ACTION, " ", ch, NULL, victim, TO_VICT );
	else
		act( AT_ACTION, argument, ch, NULL, victim, TO_VICT );

	SET_BITS( ch->act, actflags );
	destroybv( actflags );
}


/*
 * Prints message to room at large.
 */
DO_RET do_mpecho( CHAR_DATA *ch, char *argument )
{
	BITVECTOR	*actflags = NULL;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	SET_BITS( actflags, ch->act );
	REMOVE_BIT( ch->act, MOB_SECRETIVE );

	if ( !VALID_STR(argument) )
		act( AT_ACTION, " ", ch, NULL, NULL, TO_ROOM );
	else
		act( AT_ACTION, argument, ch, NULL, NULL, TO_ROOM );

	SET_BITS( ch->act, actflags );
	destroybv( actflags );
}


/*
 * Lets the mobile load an item or mobile.  All items
 * are loaded into inventory.  you can specify a level with
 * the load object portion as well.
 */
DO_RET do_mpmload( CHAR_DATA *ch, char *argument )
{
	char			arg[MIL];
	MOB_PROTO_DATA *pMobIndex;
	CHAR_DATA	   *victim;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) || !is_number(arg) )
	{
		send_mplog( ch, "do_mpmload: Bad vnum as arg" );
		return;
	}

	if ( (pMobIndex = get_mob_index(NULL, atoi(arg))) == NULL )
	{
		send_mplog( ch, "do_mpmload: Bad mob vnum" );
		return;
	}
	victim = make_mobile( pMobIndex );
	char_to_room( victim, ch->in_room );
}


DO_RET do_mpoload( CHAR_DATA *ch, char *argument )
{
	OBJ_PROTO_DATA *pObj;
	OBJ_DATA	   *obj;
	char			arg1[MIL];
	char			arg2[MIL];
	int				level;
	int				timer = 0;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !is_number(arg1) )
	{
		send_mplog( ch, "mpoload: Bad syntax" );
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		level = get_level(ch);
	}
	else
	{
		if ( !is_number(arg2) )
		{
			send_mplog( ch, "mpoload: livello non numerico: %s", arg2 );
			return;
		}

		level = atoi( arg2 );
		if ( level < 0 || level > LVL_LEGEND )
		{
			send_mplog( ch, "mpoload: Bad level" );
			return;
		}

		timer = atoi( argument );
		if ( timer < 0 )
		{
			send_mplog( ch, "mpoload: Bad timer" );
			return;
		}
	}

	pObj = get_obj_proto( NULL, atoi(arg1) );
	if ( !pObj )
	{
		send_mplog( ch, "mpoload: Bad vnum arg" );
		return;
	}

	obj = make_object( pObj, level );
	obj->timer = timer;

	if ( HAS_BIT( obj->wear_flags, OBJWEAR_TAKE) )
		obj_to_char( obj, ch );
	else
		obj_to_room( obj, ch->in_room );
}


/*
 * Just a hack of do_pardon from act_wiz.c
 */
DO_RET do_mppardon( CHAR_DATA *ch, char *argument )
{
	char	   arg1[MIL];
	char	   arg2[MIL];
	CHAR_DATA *victim;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_mplog( ch, "do_mppardon:  missing argument" );
		send_to_char( ch, "do_mppardon who for what?\r\n" );
		return;
	}

	victim = get_char_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mppardon: offender not present" );
		send_to_char( ch, "They aren't here.\r\n" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_mplog( ch, "do_mppardon:  trying to pardon NPC" );
		send_to_char( ch, "Not on NPC's.\r\n" );
		return;
	}

	if ( !str_cmp(arg2, "attacker") )
	{
		if ( HAS_BIT_PLR(victim, PLAYER_ATTACKER) )
		{
			REMOVE_BIT( victim->pg->flags, PLAYER_ATTACKER );
			send_to_char( ch, "Attacker flag removed.\r\n" );
			send_to_char( victim, "Your crime of attack has been pardoned.\r\n" );
		}
		return;
	}
	else if ( !str_cmp(arg2, "pk") || !str_cmp(arg2, "killer") )
	{
		if ( HAS_BIT_PLR(victim, PLAYER_KILLER) )
		{
			REMOVE_BIT( victim->pg->flags, PLAYER_KILLER );
			send_to_char( ch, "Killer flag removed.\r\n" );
			send_to_char( victim, "Your crime of murder has been pardoned.\r\n" );
		}
		return;
	}
	else if ( !str_cmp(arg2, "litterbug") )
	{
		if ( HAS_BIT_PLR(victim, PLAYER_LITTERBUG) )
		{
			REMOVE_BIT( victim->pg->flags, PLAYER_LITTERBUG );
			send_to_char( ch, "Litterbug flag removed.\r\n" );
			send_to_char( victim, "Your crime of littering has been pardoned.\r\n" );
		}
		return;
	}
	else if ( !str_cmp(arg2, "thief") || !str_cmp(arg2, "ladro") )
	{
		if ( HAS_BIT_PLR(victim, PLAYER_THIEF) )
		{
			REMOVE_BIT( victim->pg->flags, PLAYER_THIEF );
			send_to_char( ch, "Thief flag removed.\r\n" );
			send_to_char( victim, "Your crime of theft has been pardoned.\r\n" );
		}
		return;
	}

	send_to_char( ch, "Pardon who for what?\r\n" );
	send_mplog( ch, "do_mppardon: Invalid argument" );
}


/*
 * Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room.  It can purge
 * itself, but this had best be the last command in the MUDprogram
 * otherwise ugly stuff will happen
 */
DO_RET do_mppurge( CHAR_DATA *ch, char *argument )
{
	char	   arg[ MIL ];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		/* 'purge' */
		CHAR_DATA *vnext;

		for ( victim = ch->in_room->first_person;  victim;  victim = vnext )
		{
			vnext = victim->next_in_room;

			if ( IS_MOB(victim) && victim != ch )
				extract_char( victim, TRUE );
		}

		while ( ch->in_room->first_content )
			free_object( ch->in_room->first_content );

		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		obj = get_obj_here( ch, arg );
		if ( obj )
			free_object( obj );
		else
			send_mplog( ch, "do_mppurge: Bad argument" );
		return;
	}

	if ( IS_PG(victim) )
	{
		send_mplog( ch, "do_mppurge: Trying to purge a PC" );
		return;
	}

	if ( victim == ch )
	{
		send_mplog( ch, "do_mppurge: Trying to purge oneself" );
		return;
	}

	if ( IS_MOB(victim) && victim->pIndexData->vnum == VNUM_MOB_SUPERMOB )
	{
		send_mplog( ch, "do_mppurge: trying to purge supermob" );
		return;
	}

	extract_char( victim, TRUE );
}


/*
 * Allow mobiles to go wizinvis with programs
 */
DO_RET do_mpinvis( CHAR_DATA *ch, char *argument )
{
	char	arg[MIL];
	int		level;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( VALID_STR(arg) )
	{
		if ( !is_number(arg) )
		{
			send_mplog( ch, "do_mpinvis: Non numeric argument " );
			return;
		}

		level = atoi( arg );
		if ( level < 2 || level > LVL_LEGEND )
		{
			send_mplog( ch, "do_mpinvis: Invalid level " );
			return;
		}

		ch->mobinvis = level;
		ch_printf( ch, "do_mobinvis level set to %d.\r\n", level );
		return;
	}

	if ( ch->mobinvis < 2 )
		ch->mobinvis = get_level(ch);

	if ( HAS_BIT_ACT(ch, MOB_INCOGNITO) )
	{
		REMOVE_BIT( ch->act, MOB_INCOGNITO );
		act(AT_ADMIN, "$n slowly fades into existence.", ch, NULL, NULL,TO_ROOM );
		send_to_char( ch, "You slowly fade back into existence.\r\n" );
	}
	else
	{
		SET_BIT(ch->act, MOB_INCOGNITO);
		act( AT_ADMIN, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
		send_to_char( ch, "You slowly vanish into thin air.\r\n" );
	}
}


/*
 * Lets the mobile goto any location it wishes that is not private.
 */
/* Mounted chars follow their mobiles now */
DO_RET do_mpgoto( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	CHAR_DATA	*fch;
	CHAR_DATA	*fch_next;
	ROOM_DATA	*in_room;
	char		 arg[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch, "do_mpgoto: No argument" );
		return;
	}

	if ( (location = find_location(ch, arg)) == NULL )
	{
		send_mplog( ch, "do_mpgoto: No such location" );
		return;
	}

	in_room = ch->in_room;
	if ( ch->fighting )
		stop_fighting( ch, TRUE );

	char_from_room( ch );
	char_to_room( ch, location );

	for ( fch = in_room->first_person; fch; fch = fch_next )
	{
		fch_next = fch->next_in_room;

		if ( fch->mount && fch->mount == ch )
		{
			char_from_room( fch );
			char_to_room( fch, location );
		}
	}
}


/*
 * Lets the mobile do a command at another location. Very useful
 */
DO_RET do_mpat( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*location;
	ROOM_DATA	*original;
	char		 arg[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_mplog( ch, "do_mpat: Bad argument" );
		return;
	}

	if ( (location = find_location(ch, arg)) == NULL )
	{
		send_mplog( ch, "do_mpat: No such location" );
		return;
	}

	original = ch->in_room;
	char_from_room( ch );
	char_to_room( ch, location );

	send_command( ch, argument, CO );

	if ( !char_died(ch) )
	{
		char_from_room( ch );
		char_to_room( ch, original );
	}
}


/*
 * MobProg, dà la possibilità ad un Mob di aumentare il livello del ch.
 */
DO_RET do_mpadvance( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	int		   level;
	int		   x;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch, "do_mpadvance: arg passato non è valido" );
		return;
	}

	victim = get_player_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mpadvance: la vittima non si trova qui" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_mplog( ch, "do_mpadvance: la vittima è un mob" );
		return;
	}

	if ( get_level(victim) >= LVL_LEGEND )
		return;

	if ( !is_number(argument) )
	{
		send_mplog( ch, "do_mpadvance: livello errato: %s", argument );
		return;
	}

	level = atoi( argument );
	if ( level <= 0 || level >= LVL_LEGEND )	/* >= LVL_LEGEND e non > LVL_LEGEND, difatti l'ultimo livello da leggenda bisogna guadagnarselo */
	{
		send_mplog( ch, "do_mpadvance: argomento livello errato: %d", level );
		return;
	}

	if ( get_level(victim) > get_level(ch) )
	{
		char	buf[MSL];

		sprintf( buf, "di a %s Spiacente, devi chiedere a qualcuno più potente di me.", victim->name );
		send_command( ch, buf, CO );
		return;
	}

	set_char_color( AT_ADMIN, victim );
	act( AT_ADMIN, "$n esegue degli arcani cenni con le sue mani, quindi punta le dita verso di me!",
		ch, NULL, victim, TO_VICT );
	act( AT_ADMIN, "$n esegue dei movimenti arcani con le sue mani, poi punta le dita verso $N!",
		ch, NULL, victim, TO_NOVICT );
	set_char_color( AT_WHITE, victim );
	send_to_char( victim, "Improvvisamente mi sento molto strano..\r\n\r\n" );
	set_char_color( AT_LCYAN, victim );

	for ( x = victim->level;  x < level;  x++ )
	{
		victim->level++;
		advance_level( victim );
	}

	victim->exp = 0;
}


/* Lets the mobile transfer people.
 * The all argument transfers everyone in the current room to the specified location
 * The area argument transfers everyone in the current area to the specified location
 */
DO_RET do_mptransfer( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA		*location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA		*victim;
	CHAR_DATA		*nextinroom;
	CHAR_DATA		*admin;
	char			 arg1[MIL];
	char			 arg2[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) )
	{
		send_mplog( ch, "do_mptransfer: Bad syntax" );
		return;
	}

	/* Put in the variable nextinroom to make this work right */
	if ( !str_cmp_all(arg1, FALSE) )
	{
		for ( victim = ch->in_room->first_person;  victim;  victim = nextinroom )
		{
			char	buf[MSL];

			nextinroom = victim->next_in_room;

			if ( IS_ADMIN(ch) && ch->master )	continue;
			if ( victim == ch )					continue;

			if ( can_see(ch, victim) )
			{
				sprintf( buf, "mptransfer %s %s", victim->name, arg2 );
				send_command( ch, buf, CO );
			}
		}

		return;
	}

	/* This will only transfer PC's in the area not Mobs. */
	if ( !str_cmp(arg1, "area") )
	{
		for ( d = first_descriptor;  d;  d = d->next )
		{
			char	buf[MSL];

			victim = d->character;

			if ( d->connected != CON_PLAYING )	continue;
			if ( !victim )						continue;
			if ( !can_see(ch, victim) )			continue;
			if ( victim->in_room == NULL )		continue;

			if ( ch->in_room->area == victim->in_room->area )
			{
				sprintf( buf, "mptransfer %s %s", d->character->name, arg2 );
				send_command( ch, buf, CO );
			}
		}

		return;
	}

	/* Optional location parameter. */
	if ( !VALID_STR(arg2) )
	{
		location = ch->in_room;
	}
	else
	{
		location = find_location( ch, arg2 );
		if ( !location )
		{
			send_mplog( ch, "do_mptransfer: No such location" );
			return;
		}
	}

	victim = get_char_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mptransfer: No such person" );
		return;
	}

	if ( !victim->in_room )
	{
		send_mplog( ch, "do_mptransfer: stanza di %s NULL, vittima spostata nel Limbo", victim->name );
		return;
	}

	if ( victim->fighting )
		stop_fighting( victim, TRUE );

	/* Hey... if an admin's following someone, they should go with a mortal
	 *	when they're mptransfer'd, don't you think? */
	for ( admin = victim->in_room->first_person;  admin;  admin = nextinroom )
	{
		nextinroom = admin->next_in_room;

		if ( IS_MOB(admin) )			continue;
		if ( !IS_ADMIN(admin) )			continue;
		if ( admin->master != victim )	continue;

		if ( admin->fighting )
			stop_fighting( admin, TRUE );

		char_from_room( admin );
		char_to_room( admin, location );
	}

	char_from_room( victim );
	char_to_room( victim, location );
}


/*
 * Lets the mobile force someone to do something.
 * Must be mortal level and the all argument only affects those in the room with the mobile.
 */
DO_RET do_mpforce( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !VALID_STR(argument) )
	{
		send_mplog( ch, "do_mpforce: Bad syntax" );
		return;
	}

	if ( !str_cmp_all(arg, FALSE) )
	{
		CHAR_DATA *vch;

		for ( vch = ch->in_room->first_person;  vch;  vch = vch->next_in_room )
		{
			if ( !can_see(ch, vch) )
				continue;

			if ( get_trust(vch) < get_trust(ch) )
			{
				/* Invia il comando secondo la lingua della vittima */
				if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
					send_command( vch, argument, IT );
				else
					send_command( vch, argument, EN );
			}
		}
	}
	else
	{
		CHAR_DATA *victim;

		victim = get_char_room( ch, arg, TRUE );
		if ( !victim )
		{
			send_mplog( ch, "do_mpforce: No such victim" );
			return;
		}

		if ( victim == ch )
		{
			send_mplog( ch, "do_mpforce: Forcing oneself" );
			return;
		}

		if ( IS_PG(victim) && (!victim->desc) && IS_ADMIN(ch) )
		{
			send_mplog( ch, "do_mpforce: Attempting to force link dead admin" );
			return;
		}

		/* Invia il comando secondo la lingua della vittima */
		if ( IS_PG(ch) && HAS_BIT_PLR(ch, PLAYER_ITALIAN) )
			send_command( victim, argument, IT );
		else
			send_command( victim, argument, EN );
	}
}


/*
 * Mpbodybag for mobs to do cr's
 */
DO_RET do_mpbodybag( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg[MSL];
	char buf2[MSL];
	char buf3[MSL];

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch, "do_mpbodybag: called w/o enough argument(s)" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Victim must be in room.\r\n" );
		send_mplog( ch, "do_mpbodybag: victim not in room" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_mplog( ch, "mpbodybag: bodybagging a npc corpse" );
		return;
	}
	sprintf( buf3, " " );
	sprintf( buf2, "il cadavere di %s", arg );
	for ( obj = first_object;  obj;  obj = obj->next )
	{
		if ( obj->in_room && !str_cmp(buf2, obj->short_descr)
		  && obj->vnum == VNUM_OBJ_CORPSE_PC )
		{
			obj_from_room( obj );
			obj = obj_to_char( obj, ch );
			obj->timer = -1;
		}
	}

	send_mplog( ch, "do_mpbodybag: Grabbed %s", buf2 );
}


/*
 * Mpmorph and mpunmorph for morphing people with mobs.
 */
DO_RET do_mpmorph( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	MORPH_DATA *morph;
	char arg1[ MIL ];
	char arg2[ MIL ];

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) )
	{
		send_mplog( ch, "do_mpmorph: called w/o enough argument(s)" );
		return;
	}

	victim = get_player_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mpmorph: victim not in room" );
		return;
	}


	if ( !is_number(arg2) )
		morph = get_morph( arg2 );
	else
		morph = get_morph_vnum( atoi(arg2) );

	if ( !morph )
	{
		send_mplog( ch, "do_mpmorph: unknown morph" );
		return;
	}

	if ( victim->morph )
	{
		send_mplog( ch, "do_mpmorph: victim already morphed" );
		return;
	}
	do_morph_char( victim, morph );
}


DO_RET do_mpunmorph( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char arg[MSL];

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch, "do_mpmorph: called w/o an argument" );
		return;
	}

	victim = get_player_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mpunmorph: victim not in room" );
		return;
	}

	if ( !victim->morph )
	{
		send_mplog( ch, "do_mpunmorph: victim not morphed" );
		return;
	}
	do_unmorph_char( victim );
}


DO_RET do_mpechozone( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA	*vch;
	CHAR_DATA	*vch_next_player;
	BITVECTOR	*actflags = NULL;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	SET_BITS( actflags, ch->act );
	REMOVE_BIT( ch->act, MOB_SECRETIVE );

	for ( vch = first_player;  vch;  vch = vch_next_player )
	{
		vch_next_player = vch->next_player;

		if ( !is_awake(vch) )
			continue;

		if ( vch->in_room->area == ch->in_room->area )
		{
			if ( !VALID_STR(argument) )
				act( AT_ACTION, " ", vch, NULL, NULL, TO_CHAR );
			act( AT_ACTION, argument, vch, NULL, NULL, TO_CHAR );
		}
	}

	SET_BITS( ch->act, actflags );
	destroybv( actflags );
}


/*
 * Haus's toys follow:
 */

/*
 * syntax:  mppractice victim spell_name max%
 * (FF) cambiarlo in learn, da mob a pg
 */
DO_RET do_mppractice( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	  *skill_name;
	char	   buf[MSL];
	char	   arg1[MIL];
	char	   arg2[MIL];
	char	   arg3[MIL];
	int		   sn;
	int		   max;
	int		   tmp;
	int		   adept;

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(arg3) )
	{
		send_mplog( ch, "do_mppractice: Bad syntax" );
		return;
	}

	victim = get_player_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mppractice: Invalid student not in room" );
		return;
	}

	/* (FF) da fare anche per le lingue */
	if ( (sn = skill_lookup(arg2)) < 0 )
	{
		send_mplog( ch, "do_mppractice: Invalid spell/skill name" );
		return;
	}

	if ( IS_MOB(victim) )
	{
		send_mplog( ch, "do_mppractice: Can't train a mob" );
		return;
	}

	skill_name = table_skill[sn]->name;

	max = atoi( arg3 );
	if ( max < 0 || max > MAX_LEARN_SKELL )
	{
		send_mplog( ch, "do_mppractice: Invalid maxpercent: %d", max );
		return;
	}

	if ( get_level(victim) < table_skill[sn]->skill_level[victim->class] )
	{
		sprintf( buf,"$n attempts to tutor you in %s, but it's beyond your comprehension.", skill_name );
		act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
		return;
	}

	/*
	 * Adept is how high the player can learn it
	 */
	adept = get_adept_skell( victim, sn );

	if ( (victim->pg->learned_skell[sn] >= adept)
	  || (victim->pg->learned_skell[sn] >= max  ) )
	{
		sprintf( buf, "$n shows some knowledge of %s, but yours is clearly superior.", skill_name );
		act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
		return;
	}

	/* past here, victim learns something (TT) per ora non ha molto senso questo check tmp è inutilizzato */
	tmp = UMIN( victim->pg->learned_skell[sn] + get_curr_attr(victim, ATTR_INT)/4, max );

	act( AT_ACTION, "$N demonstrates $t to you.  You feel more learned in this subject.",
		victim, table_skill[sn]->name, ch, TO_CHAR );

	victim->pg->learned_skell[sn] = max;

	if ( victim->pg->learned_skell[sn] >= adept )
	{
		victim->pg->learned_skell[sn] = adept;
		act( AT_TELL, "$n tells you, 'You have learned all I know on this subject...'",
			ch, NULL, victim, TO_VICT );
	}
}


/*
 * syntax: mpslay (character)
 */
DO_RET do_mpslay( CHAR_DATA *ch, char *argument )
{
	char arg1[ MIL ];
	CHAR_DATA *victim;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "mpslay whom?\r\n" );
		send_mplog( ch, "do_mpslay: invalid (nonexistent?) argument" );
		return;
	}

	victim = get_char_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Victim must be in room.\r\n" );
		send_mplog( ch, "do_mpslay: victim not in room" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "You try to slay yourself.  You fail.\r\n" );
		send_mplog( ch, "do_mpslay: trying to slay self" );
		return;
	}

	if ( IS_MOB(victim) && victim->pIndexData->vnum == VNUM_MOB_SUPERMOB )
	{
		send_to_char( ch, "You cannot slay supermob!\r\n" );
		send_mplog( ch, "do_mpslay: trying to slay supermob" );
		return;
	}

	if ( !IS_ADMIN(victim) )
	{
		act( AT_ADMIN, "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR);
		act( AT_ADMIN, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
		act( AT_ADMIN, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOVICT);
		set_cur_char(victim);
		raw_kill( ch, victim );
		stop_fighting( ch, FALSE );
		stop_hating( ch );
		stop_fearing( ch );
		stop_hunting( ch );
	}
	else
	{
		act( AT_ADMIN, "You attempt to slay $M and fail!",  ch, NULL, victim, TO_CHAR);
		act( AT_ADMIN, "$n attempts to slay you.  What a kneebiter!", ch, NULL, victim, TO_VICT);
		act( AT_ADMIN, "$n attempts to slay $N.  Needless to say $e fails.",  ch, NULL, victim, TO_NOVICT);
	}
}


/*
 * Inflict damage from a mudprogram
 *  note: should be careful about using victim afterwards
 * (FF) Controllare se si possa accorpare con la damage() in figth.c
 */
ch_ret simple_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
	OBJ_DATA *damobj;
	int		  dameq;
	ch_ret	  retcode;

	retcode = rNONE;

	if ( !ch )
	{
		send_log( NULL, LOG_MPROG, "Damage: null ch!" );
		return rERROR;
	}

	if ( !victim )
	{
		send_mplog( ch, "Damage: null victim!" );
		return rVICT_DIED;
	}

	if ( victim->position == POSITION_DEAD )
		return rVICT_DIED;

	if ( dam )
	{
		if		( is_spelldamage(dt, SPELLDAMAGE_FIRE) )		dam = ris_damage( victim, dam, RIS_FIRE		   );
		else if ( is_spelldamage(dt, SPELLDAMAGE_COLD) )		dam = ris_damage( victim, dam, RIS_COLD		   );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ACID) )		dam = ris_damage( victim, dam, RIS_ACID		   );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ELECTRICITY) )	dam = ris_damage( victim, dam, RIS_ELECTRICITY );
		else if ( is_spelldamage(dt, SPELLDAMAGE_ENERGY) )		dam = ris_damage( victim, dam, RIS_ENERGY	   );
		else if ( is_spelldamage(dt, SPELLDAMAGE_DRAIN) )		dam = ris_damage( victim, dam, RIS_DRAIN	   );
		else if ( is_spelldamage(dt, SPELLDAMAGE_POISON)
		  || dt == gsn_poison )									dam = ris_damage( victim, dam, RIS_POISON	   );
		else if ( dt == (TYPE_HIT+7) || dt == (TYPE_HIT+8)  )	dam = ris_damage( victim, dam, RIS_BLUNT	   );
		else if ( dt == (TYPE_HIT+2) || dt == (TYPE_HIT+11) )	dam = ris_damage( victim, dam, RIS_PIERCE	   );
		else if ( dt == (TYPE_HIT+1) || dt == (TYPE_HIT+3)  )	dam = ris_damage( victim, dam, RIS_SLASH	   );

		if ( dam < 0 )
			dam = 0;
	}

	if ( victim != ch )
	{
		/*
		 * Damage modifiers.
		 */
		if ( HAS_BIT(victim->affected_by, AFFECT_SANCTUARY) )
			dam /= 2;

		if ( HAS_BIT(victim->affected_by, AFFECT_PROTECT) && is_align(ch, ALIGN_EVIL) )
			dam -= (int) (dam / 4);

		if ( dam < 0 )
			dam = 0;

/*		dam_message( ch, victim, dam, dt ); */
	}

	/*
	 * Check for EQ damage.... ;)
	 */
	if ( dam > 10 )
	{
		/* get a random body eq part */
		dameq  = number_range( WEARLOC_LIGHT, WEARLOC_EYES );
		damobj = get_eq_char( victim, dameq );
		if ( damobj )
		{
			if ( dam > get_obj_resistance(damobj) )
			{
				set_cur_obj( damobj );
				damage_obj( damobj );
			}
		}
	}

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	victim->points[POINTS_LIFE] -= dam;

	update_position( victim );

	switch ( victim->position )
	{
	  case POSITION_MORTAL:
		act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
			victim, NULL, NULL, TO_ROOM );
		act( AT_DANGER, "You are mortally wounded, and will die soon, if not aided.",
			victim, NULL, NULL, TO_CHAR );
		break;

	  case POSITION_INCAP:
		act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
			victim, NULL, NULL, TO_ROOM );
		act( AT_DANGER, "You are incapacitated and will slowly die, if not aided.",
			victim, NULL, NULL, TO_CHAR );
		break;

	  case POSITION_STUN:
		if ( !HAS_BIT(victim->affected_by, AFFECT_PARALYSIS) )
		{
			act( AT_ACTION, "$n is stunned, but will probably recover.",
				victim, NULL, NULL, TO_ROOM );
			act( AT_HURT, "You are stunned, but will probably recover.",
				victim, NULL, NULL, TO_CHAR );
		}
		break;

	  case POSITION_DEAD:
		act( AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM );
		act( AT_DEAD, "You have been KILLED!!\r\n", victim, 0, 0, TO_CHAR );
		break;

	  default:
		if ( dam > victim->points_max[POINTS_LIFE] / 5 )
			act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
		if ( victim->points[POINTS_LIFE] < victim->points_max[POINTS_LIFE]/5 )
		{
			act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
				victim, 0, 0, TO_CHAR );
		}
		break;
	}

	/*
	 * Payoff for killing things.
	 */
	if ( victim->position == POSITION_DEAD )
	{
		if ( IS_PG(victim) )
		{
			send_log( NULL, LOG_MPROG, "simple:damage: %s (%d) killed by %s at %d",
				victim->name,
				victim->level,
				(IS_MOB(ch))  ?  ch->short_descr  :  ch->name,
				victim->in_room->vnum );

			exp_death_hole( victim );
		}
		set_cur_char( victim );
		raw_kill( ch, victim );
		victim = NULL;

		return rVICT_DIED;
	}

	if ( victim == ch )
		return rNONE;

	/*
	 * Wimp out?
	 */
	if ( IS_MOB(victim) && dam > 0 )
	{
		if ( (HAS_BIT_ACT(victim, MOB_WIMPY) && number_bits(1) == 0
		  && victim->points[POINTS_LIFE] < victim->points_max[POINTS_LIFE]/2)
		  || (HAS_BIT(victim->affected_by, AFFECT_CHARM) && victim->master
		  && victim->master->in_room != victim->in_room) )
		{
			start_fearing( victim, ch );
			stop_hunting( victim );
			send_command( victim, "flee", CO );
		}
	}

	if ( IS_PG(victim)
	  && victim->points[POINTS_LIFE] > 0
	  && get_percent(victim->points[POINTS_LIFE], victim->points_max[POINTS_LIFE]) <= victim->wimpy
	  && victim->wait == 0 )
	{
		send_command( victim, "flee", CO );
	}

	tail_chain( );
	return rNONE;
}


/*
 * Syntax: mpdamage (character) (#hps)
 */
DO_RET do_mpdamage( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	CHAR_DATA *nextinroom;
	char	   arg1[MIL];
	char	   arg2[MIL];
	int		   dam;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if ( !VALID_STR(arg1) )
	{
		send_to_char( ch, "mpdamage whom?\r\n" );
		send_mplog( ch, "do_mpdamage: invalid argument1" );
		return;
	}

	/* Am I asking for trouble here or what?  But I need it. */
	if ( !str_cmp_all(arg1, FALSE) )
	{
		for ( victim = ch->in_room->first_person; victim; victim = nextinroom )
		{
			nextinroom = victim->next_in_room;

			if ( !can_see(ch, victim) )
				continue;

			if ( victim != ch )		/* Could go either way */
			{
				char	buf[MSL];

				sprintf( buf, "mpdamage '%s' %s", victim->name, arg2 );
				send_command( ch, buf, CO );
			}
		}
		return;
	}

	if ( !VALID_STR(arg2) )
	{
		send_to_char( ch, "mpdamage inflict how many hps?\r\n" );
		send_mplog( ch, "do_mpdamage: invalid argument2" );
		return;
	}

	victim = get_char_room( ch, arg1, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "Victim must be in room.\r\n" );
		send_mplog( ch, "do_mpdamage: victim not in room" );
		return;
	}

	if ( victim == ch )
	{
		send_to_char( ch, "You can't mpdamage yourself.\r\n" );
		send_mplog( ch, "do_mpdamage: trying to damage self" );
		return;
	}

	dam = atoi( arg2 );
	if ( dam < 0 || dam > 32000 )
	{
		send_to_char( ch, "do_mpdamage how much?\r\n" );
		send_mplog( ch, "do_mpdamage: invalid (nonexistent?) argument" );
		return;
	}

	/* this is kinda begging for trouble		*/
	/*
	 * Note from Thoric to whoever put this in...
	 * Wouldn't it be better to call damage( ch, ch, dam, dt )?		(FF)
	 * I hate redundant code
	 */
	if ( simple_damage(ch, victim, dam, TYPE_UNDEFINED ) == rVICT_DIED )
	{
		stop_fighting( ch, FALSE );
		stop_hating ( ch );
		stop_fearing( ch );
		stop_hunting( ch );
	}
}


/*
 * Syntax: mprestore (character) (#hps)
 */
DO_RET do_mprestore( CHAR_DATA *ch, char *argument )
{
	char	   arg[MIL];
	CHAR_DATA *victim;
	int		   life;

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	if ( !VALID_STR(argument) )
	{
		send_mplog( ch, "do_mprestore: invalid argument1" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch, "do_mprestore: invalid argument2" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mprestore: victim not in room" );
		return;
	}

	life = atoi( argument );
	if ( life < 0 || life > 32000 )
	{
		send_to_char( ch, "do_mprestore how much?\r\n" );
		send_mplog( ch, "do_mprestore: invalid (nonexistent?) argument" );
		return;
	}

	/* Ristora fame, sete, sete di sangue (bb) temporaneamente fino a che non si crea il sistema di comandi admin-mobprog libero e con il do_restore potenziato, serve ad evitare di utilizzare l'mpmset che ormai è KO */
	if ( victim->pg )
	{
		int x;

		for ( x = 0;  x < MAX_CONDITION;  x++ )
			victim->pg->condition[x] = 0;
	}

	life += victim->points[POINTS_LIFE];
	if ( life < 0 || life > 32000 || life > victim->points[POINTS_LIFE] )
		victim->points[POINTS_LIFE] = victim->points_max[POINTS_LIFE];
	else
		victim->points[POINTS_LIFE] = life;
}


/*
 * Syntax mp_open_passage x y z
 * opens a 1-way passage from room x to room y in direction z
 *  won't mess with existing exits
 */
DO_RET do_mpopenpassage( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*targetRoom;
	ROOM_DATA	*fromRoom;
	EXIT_DATA	*pexit;
	char		 arg1[MIL];
	char		 arg2[MIL];
	char		 arg3[MIL];
	int			 targetRoomVnum;
	int			 fromRoomVnum;
	int			 exit_num;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(arg3) )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	if ( !is_number(arg1) )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	fromRoomVnum = atoi(arg1);
	if ( (fromRoom = get_room_index(NULL, fromRoomVnum)) == NULL )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	if ( !is_number(arg2) )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	targetRoomVnum = atoi(arg2);
	if ( (targetRoom = get_room_index(NULL, targetRoomVnum)) == NULL )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	if ( !is_number(arg3) )
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	exit_num = atoi( arg3 );
	if ( exit_num < 0 || exit_num >= MAX_DIR )	/* non ci vuole la DIR_SOMEWHERE */
	{
		send_mplog( ch, "do_mpOpenPassage: Bad syntax" );
		return;
	}

	if ( (pexit = get_exit(fromRoom, exit_num )) != NULL )
	{
		if ( !HAS_BIT(pexit->flags, EXIT_PASSAGE) )
			return;
		send_mplog( ch, "do_mpOpenPassage: Exit exists" );
		return;
	}

	pexit = make_exit( fromRoom, targetRoom, exit_num );
	DISPOSE( pexit->keyword );
	DISPOSE( pexit->description );
	pexit->keyword		= str_dup( "" );
	pexit->description	= str_dup( "" );
	pexit->key			= -1;
	pexit->flags		= meb( EXIT_PASSAGE );

/*	act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
/*	act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */
}


/*
 * Syntax mp_fillin x
 * Simply closes the door
 */
DO_RET do_mpfill_in( CHAR_DATA *ch, char *argument )
{
	char	   arg[MIL];
	EXIT_DATA *pexit;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( (pexit = find_door(ch, arg, TRUE)) == NULL  )
	{
		send_mplog( ch, "do_mpFillIn: Exit does not exist" );
		return;
	}

	SET_BIT( pexit->flags, EXIT_CLOSED );
}


/*
 * Syntax mp_close_passage x y
 * closes a passage in room x leading in direction y
 * the exit must have EXIT_PASSAGE set
 */
DO_RET do_mpclosepassage( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*fromRoom;
	EXIT_DATA	*pexit;
	char		 arg1[MIL];
	char		 arg2[MIL];
	char		 arg3[MIL];
	int			 fromRoomVnum;
	int			 exit_num;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
	argument = one_argument( argument, arg3 );

	if ( !VALID_STR(arg1) || !VALID_STR(arg2) || !VALID_STR(arg3) )
	{
		send_mplog( ch, "do_mpClosePassage: Bad syntax" );
		return;
	}

	if ( !is_number(arg1) )
	{
		send_mplog( ch, "do_mpClosePassage: Bad syntax" );
		return;
	}

	fromRoomVnum = atoi( arg1 );
	if ( (fromRoom = get_room_index(NULL, fromRoomVnum)) == NULL )
	{
		send_mplog( ch, "do_mpClosePassage: Bad syntax" );
		return;
	}

	if ( !is_number(arg2) )
	{
		send_mplog( ch, "do_mpClosePassage: Bad syntax" );
		return;
	}

	exit_num = atoi(arg2);
	if ( exit_num < 0 || exit_num >= MAX_DIR )	/* non ci vuole la DIR_SOMEWHERE */
	{
		send_mplog( ch, "do_mpClosePassage: Bad syntax" );
		return;
	}

	if ( (pexit = get_exit(fromRoom, exit_num)) == NULL )
	{
		return;		/* already closed, ignore...  so rand_progs can close without spam */
	}

	if ( !HAS_BIT(pexit->flags, EXIT_PASSAGE) )
	{
		send_mplog( ch, "do_mpClosePassage: Exit not a passage" );
		return;
	}
	extract_exit( fromRoom, pexit );

/*	act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
/*	act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */
}


/*
 * Does nothing.  Used for scripts.
 */
DO_RET do_mpnothing( CHAR_DATA *ch, char *argument )
{
	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}
}


/*
 * Sends a message to sleeping character.  Should be fun with room sleep_progs
 */
DO_RET do_mpdream( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg1[MSL];

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg1 );
	victim = get_char_mud( ch, arg1, TRUE );
	if ( !victim )
	{
		send_mplog( ch, "do_mpdream: No such character" );
		return;
	}

	if ( victim->position <= POSITION_SLEEP )
		ch_printf( victim, "%s\r\n", argument );
}


/*
 * Deposit some gold into the current area's economy.
 */
DO_RET do_mpdeposit( CHAR_DATA *ch, char *argument )
{
	char	arg[MSL];
	int		gold;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_mplog( ch,"Mpdeposit: bad syntax" );
		return;
	}

	gold = atoi( arg );
	if ( gold <= ch->gold && ch->in_room )
	{
		ch->gold -= gold;
		boost_economy( ch->in_room->area, gold );
	}
}


/*
 * Withdraw some gold from the current area's economy
 */
DO_RET do_mpwithdraw( CHAR_DATA *ch, char *argument )
{
	char	arg[MSL];
	int		gold;

	if ( IS_PG(ch) || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	one_argument( argument, arg );

	if ( !VALID_STR(arg) )
	{
		send_mplog( ch,"Mpwithdraw: bad syntax" );
		return;
	}

	gold = atoi( arg );
	if ( ch->gold < 1000000000 && gold < 1000000000 && ch->in_room
	  && economy_has(ch->in_room->area, gold) )
	{
		ch->gold += gold;
		lower_economy( ch->in_room->area, gold );
	}
}


DO_RET do_mpdelay( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *victim;
	char	   arg[MIL];
	int		   ndelay;

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Delay for how many rounds?n\r" );
		send_mplog( ch, "do_mpdelay: no duration specified" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "They aren't here.\r\n" );
		send_mplog( ch, "do_mpdelay: target not in room" );
		return;
	}

	if ( !VALID_STR(argument) || !is_number(argument) )
	{
		send_to_char( ch, "Delay them for how many rounds?\r\n" );
		send_mplog( ch, "do_mpdelay: invalid (nonexistant?) argument" );
		return;
	}

	ndelay = atoi( arg );
	if ( ndelay < 1 || ndelay > 30 )
	{
		send_to_char( ch, "Argument out of range.\r\n" );
		send_mplog( ch, "do_mpdelay:  argument out of range (1 to 30)" );
		return;
	}

	WAIT_STATE( victim, ndelay * PULSE_IN_SECOND );
	send_to_char( ch, "do_mpdelay applied.\r\n" );
}


DO_RET do_mppeace( CHAR_DATA *ch, char *argument )
{
	char arg[MIL];
	CHAR_DATA *rch;
	CHAR_DATA *victim;

	if ( IS_PG(ch) || ch->desc || HAS_BIT(ch->affected_by, AFFECT_CHARM) )
	{
		send_to_char( ch, NOFOUND_MSG );
		return;
	}

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) )
	{
		send_to_char( ch, "Who do you want to mppeace?\r\n" );
		send_mplog( ch, "do_mppeace: invalid (nonexistent?) argument" );
		return;
	}

	if ( !str_cmp_all(arg, FALSE) )
	{
		for ( rch = ch->in_room->first_person;  rch;  rch=rch->next_in_room )
		{
			if ( rch->fighting )
			{
				stop_fighting( rch, TRUE );
				send_command( rch, "sit", CO );
			}
			stop_hating( rch );
			stop_hunting( rch );
			stop_fearing( rch );
		}
		send_to_char( ch, "Ok.\r\n" );
		return;
	}

	victim = get_char_room( ch, arg, TRUE );
	if ( !victim )
	{
		send_to_char( ch, "They must be in the room.n\r" );
		send_mplog( ch, "do_mppeace: target not in room" );
		return;
	}

	if ( victim->fighting )
		stop_fighting( victim, TRUE );

	stop_hating( ch );			stop_hunting( ch );			stop_fearing( ch );
	stop_hating( victim );		stop_hunting( victim );		stop_fearing( victim );
	send_to_char( ch, "Ok.\r\n" );
}
