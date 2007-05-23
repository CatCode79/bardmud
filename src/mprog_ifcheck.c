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
 > The MUDprograms are heavily based on the original MOBprogram code that	<
 >  was written by N'Atas-ha.												<
\****************************************************************************/

#include <ctype.h>
#include <stdarg.h>

#include "mud.h"
#include "affect.h"
#include "calendar.h"
#include "clan.h"
#include "command.h"
#include "db.h"
#include "gramm.h"
#include "interpret.h"
#include "movement.h"
#include "mprog.h"
#include "morph.h"
#include "room.h"
#include "timer.h"


/* Defines for new mudprog parsing, used as return values from mprog_do_command */
#define COMMANDOK		 1
#define IFTRUE			 2
#define IFFALSE			 3
#define ORTRUE			 4
#define ORFALSE			 5
#define FOUNDELSE		 6
#define FOUNDENDIF		 7
#define IFIGNORED		 8
#define ORIGNORED		 9


/* Ifstate defines, used to create and access ifstate array in mprog_driver. */
#define IN_IF			 0
#define IN_ELSE			 1
#define DO_IF			 2
#define DO_ELSE			 3

#define MAX_IFS			15
#define MAX_IF_ARGS		 6
#define MAX_PROG_NEST	15


/*
 * Recursive function used by the carryingvnum ifcheck.
 * It loops thru all objects belonging to a char (in nested containers)
 *  and returns TRUE if it finds a matching vnum.
 * I declared it static to limit its scope to this file.
 *
 * This recursive function works by using the following method for
 *  traversing the nodes in a binary tree:
 *
 * Start at the root node
 *   if there is a child then visit the child
 *     if there is a sibling then visit the sibling
 *   else
 *   if there is a sibling then visit the sibling
 */
static bool carryingvnum_visit( CHAR_DATA * ch, OBJ_DATA *obj, VNUM vnum )
{
/*	pager_printf( ch, "***obj= %s vnum= %d\r\n", obj->name, obj->vnum ); */

	if ( !ch )
	{
		send_log( NULL, LOG_MPROG, "carryingvnum_visit: ch è NULL" );
		return  BERR;
	}

	if ( !obj )
	{
		send_log( NULL, LOG_MPROG, "carryingvnum_visit: obj è NULL con ch: %s", ch->name );
		return BERR;
	}

	if ( vnum <= 0 || vnum > MAX_VNUM )
	{
		send_log( NULL, LOG_MPROG, "carryingvnum_visit: vnum passato errrato %d, con ch: %s, oggetto: %s (#%d)",
			vnum, ch->name, obj->short_descr, obj->vnum );
		return BERR;
	}

	if ( obj->wear_loc == -1 && obj->vnum == vnum )
		return TRUE;

	if ( obj->first_content )		/* node has a child? */
	{
		if ( carryingvnum_visit(ch, obj->first_content, vnum) )
			return TRUE;

		if ( obj->next_content )	/* node has a sibling? */
		{
			if ( carryingvnum_visit(ch, obj->next_content, vnum) )
				return TRUE;
		}
	}
	else if ( obj->next_content )	/* node has a sibling? */
	{
		if ( carryingvnum_visit(ch, obj->next_content, vnum) )
			return TRUE;
	}

	return FALSE;
}


/***************************************************************************
 * Local function code and brief comments.
 */

/* These two functions do the basic evaluation of ifcheck operators.
 * It is important to note that the string operations are not what
 *  you probably expect.  Equality is exact and division is substring.
 * Remember that lhs has been stripped of leading space, but can
 *  still have trailing spaces so be careful when editing since:
 *  "guard" and "guard " are not equal.
 */
bool mprog_seval( char *lhs, char *opr, char *rhs, CHAR_DATA *mob )
{
	if ( !VALID_STR(lhs) )
	{
		send_mplog( mob, "mprog_seval: lhs non è valido" );
		return BERR;
	}

	if ( !VALID_STR(opr) )
	{
		send_mplog( mob, "mprog_seval: opr non è valido" );
		return BERR;
	}

	if ( !VALID_STR(rhs) )
	{
		send_mplog( mob, "mprog_seval: rhs non è valido" );
		return BERR;
	}

	if ( !mob )
	{
		send_mplog( mob, "mprog_seval: mob è NULL" );
		return BERR;
	}

	if ( !str_cmp(opr, "=")
	  || !str_cmp(opr, "==") )		return ( bool )( !str_cmp  (lhs, rhs) );
	if ( !str_cmp(opr, "!=") )		return ( bool )(  str_cmp  (lhs, rhs) );
	if ( !str_cmp(opr, "/") )		return ( bool )(  str_prefix(rhs, lhs) );
	if ( !str_cmp(opr, "!/") )		return ( bool )(  str_prefix(rhs, lhs) );
	if ( !str_cmp(opr, "^" ) )		return ( bool )( !str_infix(rhs, lhs) );
	if ( !str_cmp(opr, "!^") )		return ( bool )(  str_infix(rhs, lhs) );
	if ( !str_cmp(opr, "\\") )		return ( bool )(  str_suffix(rhs, lhs) );
	if ( !str_cmp(opr, "!\\") )		return ( bool )(  str_suffix(rhs, lhs) );

	send_mplog( mob, "mprog_seval: operatore mprog errato '%s'", opr );

	return FALSE;
}


bool mprog_veval( int lhs, char *opr, int rhs, CHAR_DATA *mob )
{
	if ( !VALID_STR(opr) )
	{
		send_mplog( mob, "mprog_veval: opr stringa non valida" );
		return BERR;
	}

	if ( !mob )	
	{
		send_mplog( mob, "mprog_veval: mob è NULL" );
		return BERR;
	}

	if ( !str_cmp(opr, "=")
	  || !str_cmp(opr, "==") )		return ( lhs == rhs );
	if ( !str_cmp(opr, "!=") )		return ( lhs != rhs );
	if ( !str_cmp(opr, ">" ) )		return ( lhs >  rhs );
	if ( !str_cmp(opr, "<" ) )		return ( lhs <  rhs );
	if ( !str_cmp(opr, "<=") )		return ( lhs <= rhs );
	if ( !str_cmp(opr, ">=") )		return ( lhs >= rhs );
	if ( !str_cmp(opr, "&" ) )		return ( lhs &  rhs );
	if ( !str_cmp(opr, "|" ) )		return ( lhs |  rhs );

	send_mplog( mob, "mprog_veval: operatore mprog errato '%s'", opr );

	return FALSE;
}


#define isoperator(c) ( (c) == '='		\
					 || (c) == '<'		\
					 || (c) == '>'		\
					 || (c) == '!'		\
					 || (c) == '&'		\
					 || (c) == '|'		\
					 || (c) == '/'		\
					 || (c) == '^'		\
					 || (c) == '\\' )

/*
 * Value è l'int in cui viene salvato il valore convertito
 * to_cnv è la stringa che deve essere convertita in numero per poi essere salvata in value
 * negative se TRUE indica che accetta i numeri convertiti negativi, se è FALSE non li accetta
 * max indica quale valore massimo può essere value, se viene passato -1 indica che non ha un massimo preciso
 */
#define MP_ATOI( value, to_cnv, negative, max )														\
{																									\
	if ( !is_number((to_cnv)) )																		\
	{																								\
		send_mplog( mob, "mprog_do_ifcheck: %s: valore passato non numerico: %s", chck, (to_cnv) );	\
		return BERR;																				\
	}																								\
	value = atoi( (to_cnv) );																		\
	if ( (value) < 0 && (negative) == FALSE )														\
	{																								\
		send_mplog( mob, "mprog_do_ifcheck: %s: valore passato negativo: %s", chck, (to_cnv) );		\
		return BERR;																				\
	}																								\
	if ( (value) > (max) )																			\
	{																								\
		send_mplog( mob, "mprog_do_ifcheck: %s: valore passato troppo alto: %s", chck, (to_cnv) );	\
		return BERR;																				\
	}																								\
}

#define IN_ROOM		0
#define IN_AREA		1
#define IN_MUD		2

/*
 * This function performs the evaluation of the if checks.
 * It is here that you can add any ifchecks which you so desire.
 * Hopefully it is clear from what follows how one would go about adding your own.
 * The syntax for an if check is: ifcheck ( arg ) [opr val]
 *  where the parenthesis are required and the opr and val fields are
 *  optional but if one is there then both must be.
 * The spaces are all optional. The evaluation of the opr expressions is farmed out
 *  to reduce the redundancy of the mammoth if statement list.
 * If there are errors, then return BERR otherwise return boolean 1, 0
 */
int mprog_do_ifcheck( char *ifcheck, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
	CHAR_DATA	*chkchar = NULL;	/* mob o pg a seconda della variabile $ passata */
	OBJ_DATA	*chkobj  = NULL;	/* oggetto a seconda della variabile $ passata */
	char		*chck;				/* puntatore che contiene il nome dell'ifcheck */
	char		*cvar;				/* Tipo di variabile $ passata */
	char		*argv[MAX_IF_ARGS];
	char		*rval = "";
	char		*q;
	char		 buf[MSL];
	char		*p = buf;
	char		 opr[MIL];
	int			 argc = 0;
	int			 lhsvl = 0;
	int			 rhsvl = 0;

	if ( !VALID_STR(ifcheck) )
	{
		send_mplog( mob, "mprog_do_ifcheck: ifcheck è NULL" );
		return BERR;
	}

	/*
	 * New parsing to allow for multiple arguments inside the brackets
	 * ie: if leveldiff($n, $i) > 10
	 * It's also smaller, cleaner and probably faster
	 */
	strcpy( buf, ifcheck );
	opr[0] = '\0';

	while ( isspace(*p) )
		++p;

	argv[argc++] = p;
	while ( isalnum(*p) )
		++p;

	while ( isspace(*p) )
		*p++ = '\0';

	if ( *p != '(' )
	{
		send_mplog( mob, "mprog_do_ifcheck: errore di sintassi nell'ifcheck (parentesi sinistra mancante)" );
		return BERR;
	}

	*p++ = '\0';
	/* Need to check for spaces or if name( $n ) isn't legal */
	while ( isspace(*p) )
		*p++ = '\0';

	for ( ; ; )
	{
		argv[argc++] = p;

		while ( *p == '$' || isalnum(*p) )
			++p;

		while ( isspace(*p) )
			*p++ = '\0';

		switch ( *p )
		{
		  case ',':
			*p++ = '\0';

			while ( isspace(*p) )
				*p++ = '\0';

			if ( argc >= MAX_IF_ARGS )
			{
				while ( *p && *p != ')' )
					++p;

				if ( *p )
					*p++ = '\0';

				while ( isspace(*p) )
					*p++ = '\0';

				goto doneargs;
			}
			break;

		  case ')':
			*p++ = '\0';
			while ( isspace(*p) )
				*p++ = '\0';

			goto doneargs;
			break;

		  default:
			send_mplog( mob, "mprog_do_ifcheck: errore di sintassi nell'ifcheck (parentesi destra mancante)" );	/* (TT) */
			goto doneargs;
			break;
		}
	}


doneargs:
	q = p;
	while ( isoperator(*p) )
		++p;

	strncpy( opr, q, p-q );
	opr[p-q] = '\0';

	while ( isspace(*p) )
		*p++ = '\0';

	rval = p;

	while ( *p )
		++p;
	*p = '\0';

	chck = (argv[0]) ? argv[0] : "";
	cvar = (argv[1]) ? argv[1] : "";

	/*
	 * Chck contains check, cvar is the variable in the (), opr is the operator
	 *  if there is one, and rval is the value if there was an operator.
	 */
	if ( cvar[0] == '$' )
	{
		switch ( cvar[1] )
		{
		  case 'i':		chkchar = mob;				break;
		  case 'n':		chkchar = actor;			break;
		  case 't':		chkchar = (CHAR_DATA *)vo;	break;
		  case 'r':		chkchar = rndm;				break;
		  case 'o':		chkobj  = obj;				break;
		  case 'p':		chkobj  = (OBJ_DATA *)vo;	break;
		  default:
			send_mplog( mob, "mprog_do_ifcheck: bad argument '%c' to '%s'", cvar[0], chck );
			return BERR;
		}
		if ( !chkchar && !chkobj )
			return BERR;
	}


	/* Ifcheck per rendere casuale l'invio di comandi */
	if ( !str_cmp(chck, "Rand") )
	{
		int percent;

		MP_ATOI( percent, cvar, FALSE, 100 );

		if ( number_percent() <= percent )
			return TRUE;

		return FALSE;
	}

	/* Ifcheck che serve a manipolare l'economia dell'area */
	if ( !str_cmp(chck, "Economy") )
	{
		ROOM_DATA	*room;
		int			 money;
		VNUM		 vnum;

		MP_ATOI( vnum, cvar, FALSE, MAX_VNUM );

		if ( vnum == 0 )
		{
			if ( !mob->in_room )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: la stanza del mob è NULL", chck );
				return BERR;
			}
			room = mob->in_room;
		}
		else
		{
			room = get_room_index( NULL, vnum );
			if ( !room )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: vnum di stanza passato errato: %d room", chck, vnum );
				return BERR;
			}
		}

		if ( !room->area )
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: l'area della stanza #%d è NULL", chck, room->vnum );
			return BERR;
		}

		/* (TT) non sapendo bene come funzioni l'economia non so nè il massimo se se accettare numeri negativi */
		MP_ATOI( money, rval, FALSE, -1 );

		if ( room->area->economy_high > 0 )
			return mprog_veval( room->area->economy_low + 1000000000, opr, money, mob );
		else
			return mprog_veval( room->area->economy_low, opr, money, mob );
	}

	/* Ifcheck che serve a contare mob via vnum */
	if ( is_name(chck, "CountMobVnumRoom CountMobVnumArea CountMobVnumMud") )
	{
		MOB_PROTO_DATA *m_index;
		CHAR_DATA	   *tmob;
		int				found_count;
		int				in_type;
		VNUM			vnum;

		MP_ATOI( vnum, cvar, FALSE, MAX_VNUM );

		m_index = get_mob_index( NULL, vnum );
		if ( !m_index )
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: mob inesistente con vnum: #%d", chck, vnum );
			return BERR;
		}

		if		( !str_suffix("Room", chck) )	in_type = IN_ROOM;
		else if ( !str_suffix("Area", chck) )	in_type = IN_AREA;
		else if ( !str_suffix("Mud",  chck) )	in_type = IN_MUD;
		else
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: tipo di ifcheck in_type non esistente o errato: %s", chck, cvar );
			return BERR;
		}

		lhsvl = 0;
		found_count = 0;
		for ( tmob = first_char;  tmob;  tmob = tmob->next )
		{
			if ( IS_PG(tmob) )							continue;
			if ( tmob->pIndexData->vnum != vnum )		continue;

			/* Piccolo controllo sul conteggio del mob */
			if ( found_count > m_index->count )
			{
				send_mplog( mob, "mprog_if_check: %s: entità trovate maggiore di quelle ufficialmente esistenti", chck );
				break;
			}

			if ( in_type == IN_ROOM && tmob->in_room == mob->in_room )
				lhsvl++;
			else if ( in_type == IN_AREA && tmob->in_room->area == mob->in_room->area )
				lhsvl++;
			else if ( in_type == IN_MUD )
				lhsvl++;

			found_count++;
		}

		MP_ATOI( rhsvl, rval, FALSE, 100 );

		return mprog_veval( lhsvl, opr, rhsvl, mob );
	}

	/* Conta cercando per vnum i mob oppure per nome mob e pg */
	if ( !str_prefix(chck, "Count")
	  && (is_name(chck, "CountMobRoom CountMobArea CountMobMud")
	  ||  is_name(chck, "CountPgRoom CountPgArea CountPgMud")
	  ||  is_name(chck, "CountCharRoom CountCharArea CountCharMud")
	  ||  is_name(chck, "CountMobNameRoom CountMobNameArea CountMobNameMud")
	  ||  is_name(chck, "CountMobPrefixRoom CountMobPrefixArea CountMobPrefixMud")
	  ||  is_name(chck, "CountPgNameRoom CountPgNameArea CountPgNameMud")
	  ||  is_name(chck, "CountPgPrefixRoom CountPgPrefixArea CountPgPrefixMud")) )
	{
		CHAR_DATA	*tch;
		int			 in_type;
		bool		 search_pg	= TRUE;
		bool		 search_mob = TRUE;
		bool		 search_prefix = FALSE;

		if ( !str_prefix(chck, "CountPg") )
			search_mob = FALSE;
		else if ( !str_prefix(chck, "CountMob") )
			search_pg = FALSE;

		if ( !str_infix(chck, "Prefix") )
			search_prefix = TRUE;

		if		( !str_suffix("Room", chck) )	in_type = IN_ROOM;
		else if ( !str_suffix("Area", chck) )	in_type = IN_AREA;
		else if ( !str_suffix("Mud",  chck) )	in_type = IN_MUD;
		else
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: tipo di ifcheck in_type non esistente o errato: %s", chck, cvar );
			return BERR;
		}

		lhsvl = 0;
		for ( tch = first_char;  tch;  tch = tch->next )
		{
			if (  search_pg	&& IS_MOB(tch) && !search_mob )					continue;
			if (  search_mob && IS_PG (tch) && !search_pg  )				continue;
			if ( !search_prefix && !nifty_is_name(tch->name, cvar) )		continue;
			if (  search_prefix && !nifty_is_name_prefix(tch->name, cvar) )	continue;
			if ( IS_ADMIN(tch) && tch->pg->incognito != 0 )					continue;

			if ( in_type == IN_ROOM && tch->in_room == mob->in_room )
				lhsvl++;
			else if ( in_type == IN_AREA && tch->in_room->area == mob->in_room->area )
				lhsvl++;
			else if ( in_type == IN_MUD )
				lhsvl++;
		}

		MP_ATOI( rhsvl, rval, FALSE, -1 );

		return mprog_veval( lhsvl, opr, rhsvl, mob );
	}

	/* CountObj conta gli oggetti nel mob, COuntObjPg conta gli oggetti dell'attore del mudprog
	 * Tipi di ricerca: all, vnum, type, name, prefix
	 * Locazioni: here, room, carry e wear */
	if ( !str_prefix(chck, "CountObj")
	  && (is_name(chck, "CountObjHere CountObjRoom CountObjCarry CountObjWear")
	  ||  is_name(chck, "CountObjVnumHere CountObjVnumRoom CountObjVnumCarry CountObjVnumWear")
	  ||  is_name(chck, "CountObjTypeHere CountObjTypeRoom CountObjTypeCarry CountObjTypeWear")
	  ||  is_name(chck, "CountObjNameHere CountObjNameRoom CountObjNameCarry CountObjNameWear")
	  ||  is_name(chck, "CountObjPrefixHere CountObjPrefixRoom CountObjPrefixCarry CountObjPrefixWear")
	  ||  is_name(chck, "CountObjPgHere CountObjPgRoom CountObjPgCarry CountObjPgWear")
	  ||  is_name(chck, "CountObjPgVnumHere CountObjPgVnumRoom CountObjPgVnumCarry CountObjPgVnumWear")
	  ||  is_name(chck, "CountObjPgTypeHere CountObjPgTypeRoom CountObjPgTypeCarry CountObjPgTypeWear")
	  ||  is_name(chck, "CountObjPgNameHere CountObjPgNameRoom CountObjPgNameCarry CountObjPgNameWear")
	  ||  is_name(chck, "CountObjPgPrefixHere CountObjPgPrefixRoom CountObjPgPrefixCarry CountObjPgPrefixWear")) )
	{
		OBJ_DATA *pObj;
		VNUM	  vnum = atoi( cvar );
		int		  loc_type;		/* 0 = Here, 1 = Room, 2 = Carry, 3 = Wear */
		int		  type;
		bool	  search_pg = FALSE;
		bool	  search_vnum = FALSE;
		bool	  search_type = FALSE;
		bool	  search_name = FALSE;
		bool	  search_prefix = FALSE;

		if		( !str_infix(chck, "Vnum") )	search_vnum = TRUE;
		else if ( !str_infix(chck, "Type") )	search_type = TRUE;
		else if ( !str_infix(chck, "Name") )	search_name = TRUE;
		else if ( !str_infix(chck, "Prefix") )	search_prefix = TRUE;

		if ( search_vnum && !is_number(cvar) )	/* (TT) dovrò fare il check su cvar o su lvar? */
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: cvar non è un numero: %s", chck, cvar );
			return BERR;
		}

		type = code_num( NULL, cvar, CODE_OBJTYPE );
		if ( search_type && type == -1 )
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: cvar non è un tipo di oggetto valido: %s", chck, cvar );	/* (TT) dovrò fare il check su cvar o su lvar? */
			return BERR;
		}

		if ( !str_prefix(chck, "CountObjPg") )
			search_pg = TRUE;

		if		( !str_suffix("Here", chck ) )		loc_type = 0;
		else if ( !str_suffix("Room", chck ) )		loc_type = 1;
		else if	( !str_suffix("Carry", chck) )		loc_type = 2;
		else if	( !str_suffix("Wear", chck ) )		loc_type = 3;
		else
		{
			send_mplog( mob, "mprog_do_ifcheck: %s: tipo di ifcheck loc_type non esistente o errato: %s", chck, cvar );
			return BERR;
		}

		lhsvl = 0;
		if ( loc_type == 0 || loc_type == 2 || loc_type == 3 )
		{
			for ( pObj = search_pg ? actor->first_carrying : mob->first_carrying;  pObj;  pObj = pObj->next_content )
			{
				if ( search_vnum && pObj->vnum != vnum )						continue;
				if ( search_type && pObj->type != type )						continue;
				if ( search_name && !nifty_is_name(obj->name, cvar) )			continue;
				if ( search_prefix && !nifty_is_name_prefix(obj->name, cvar) )	continue;

				if ( loc_type == 2 && pObj->wear_loc != WEARLOC_NONE )			continue;
				if ( loc_type == 3 && pObj->wear_loc == WEARLOC_NONE )			continue;

				lhsvl += pObj->count;
			}
		}

		if ( loc_type == 0 || loc_type == 1 )
		{
			for ( pObj = search_pg ? actor->in_room->first_content : mob->in_room->first_content;  pObj;  pObj = pObj->next_content )
			{
				if ( search_vnum && pObj->vnum != vnum )						continue;
				if ( search_type && pObj->type != type )						continue;
				if ( search_name && !nifty_is_name(obj->name, cvar) )			continue;
				if ( search_prefix && !nifty_is_name_prefix(obj->name, cvar) )	continue;

				lhsvl += pObj->count;
			}
		}

		MP_ATOI( rhsvl, rval, FALSE, -1 );

		return mprog_veval( lhsvl, opr, rhsvl, mob );
	}

	/* Ritorna quante volte il mob è stato ucciso */
	if ( !str_cmp(chck, "TimesKilled") )
	{
		MOB_PROTO_DATA *pMob;

		if ( chkchar )
		{
			pMob = chkchar->pIndexData;
			if ( !pMob )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: struttura mob_index nulla per il mob %s",
					chck, (chkchar->name) ? chkchar->name : "sconosciuto" );
				return BERR;
			}
		}
		else
		{
			int vnum;

			MP_ATOI( vnum, cvar, FALSE, MAX_VNUM );

			pMob = get_mob_index( NULL, vnum );
			if ( !pMob )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: vnum errato: %d", chck, vnum );
				return BERR;
			}
		}

		MP_ATOI( rhsvl, rval, FALSE, -1 );

		return mprog_veval( pMob->killed, opr, rhsvl, mob );
	}

	/* ifcheck utilizzati solo dai mob mudprog */
	if ( chkchar )
	{
		if ( !str_cmp(chck, "IsPacifist") )		return ( IS_MOB(chkchar) && HAS_BIT_ACT(chkchar, MOB_PACIFIST) );

		if ( !str_cmp(chck, "IsMobInvis") )		return ( IS_MOB(chkchar) && HAS_BIT_ACT(chkchar, MOB_INCOGNITO) );
		if ( !str_cmp(chck, "MobInvisLevel") )
		{
			int	level;

			if ( IS_PG(chkchar) )
				return FALSE;

			MP_ATOI( level, rval, FALSE, LVL_LEGEND );

			return mprog_veval( chkchar->mobinvis, opr, level, mob );
		}

		if ( !str_cmp(chck, "IsPg") )			return IS_MOB(chkchar) ? FALSE : TRUE;
		if ( !str_cmp(chck, "IsMob") )			return IS_MOB(chkchar) ? TRUE : FALSE;

		if ( !str_cmp(chck, "CanSee") )			return can_see( mob , chkchar );

		if ( is_name(chck, "IsPassage IsOpen IsClosed IsLocked") )
		{
			EXIT_DATA *pexit;

			if ( !check_direction_arg(rval) )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: direzione passata errata: %s", chck, rval );
				return BERR;
			}

			pexit = find_door( chkchar, rval , TRUE );
			if ( !pexit )
				return FALSE;

			if ( !str_cmp(chck, "IsOpen") )
			{
				if ( !HAS_BIT(pexit->flags, EXIT_CLOSED) )
					return TRUE;
				else
					return FALSE;
			}

			if ( !str_cmp(chck, "IsClosed") )
			{
				if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
					return TRUE;
				else
					return FALSE;
			}

			if ( !str_cmp(chck, "IsLocked") )
			{
				if ( !HAS_BIT(pexit->flags, EXIT_LOCKED) )
					return TRUE;
				else
					return FALSE;
			}

			return TRUE;	/* per IsPassage ritorna TRUE se ha tovato l'uscita */
		}

		if ( !str_cmp(chck, "CanPkill") )		return can_pkill(chkchar, NULL)  ?  TRUE  :  FALSE;	/* (FF) Magari farla più complessa con argomento opzionale per indicare che si vuole controllare un mob o pg */

		if ( !str_cmp(chck, "IsMounted") )		return (chkchar->position == POSITION_MOUNT) ? TRUE : FALSE;

		if ( !str_cmp(chck, "IsMorphed") )		return (chkchar->morph) ? TRUE : FALSE;

		if ( !str_cmp(chck, "IsGood") )			return is_align(chkchar, ALIGN_GOOD)	?  TRUE  :  FALSE;
		if ( !str_cmp(chck, "IsNeutral") )		return is_align(chkchar, ALIGN_NEUTRAL) ?  TRUE  :  FALSE;
		if ( !str_cmp(chck, "IsEvil") )			return is_align(chkchar, ALIGN_EVIL)	?  TRUE  :  FALSE;

		if ( !str_cmp(chck, "IsFight") )		return who_fighting(chkchar)  ?  TRUE  :  FALSE;

		if ( !str_cmp(chck, "IsAdmin") )		return ( IS_ADMIN(chkchar) )  ?  TRUE  :  FALSE;

		if ( !str_cmp(chck, "IsCharmed") )		return HAS_BIT(chkchar->affected_by, AFFECT_CHARM)  ?  TRUE  :  FALSE;

		if ( !str_cmp(chck, "IsFlying") )		return HAS_BIT(chkchar->affected_by, AFFECT_FLYING)  ?  TRUE  :  FALSE;

		if ( !str_cmp(chck, "IsThief") )		return ( IS_PG(chkchar) && HAS_BIT_PLR(chkchar, PLAYER_THIEF) );
		if ( !str_cmp(chck, "IsAttacker") )		return ( IS_PG(chkchar) && HAS_BIT_PLR(chkchar, PLAYER_ATTACKER) );
		if ( !str_cmp(chck, "IsKiller") )		return ( IS_PG(chkchar) && HAS_BIT_PLR(chkchar, PLAYER_KILLER) );

		if ( !str_cmp(chck, "IsFollow") )		return ( chkchar->master != NULL && chkchar->master->in_room == chkchar->in_room );

		if ( !str_cmp(chck, "IsAffected") )
		{
			int aff;

			aff = code_num( NULL, rval, CODE_AFFECT );
			if ( aff == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: affect errato passato: %s", chck, rval );
				return BERR;
			}

			if ( HAS_BIT(chkchar->affected_by, aff) )
				return FALSE;

			return TRUE;
		}

		if ( !str_cmp(chck, "NumFighting") )
		{
			int num;

			MP_ATOI( num, rval, FALSE, -1 );
			return mprog_veval( chkchar->num_fighting-1, opr, num, mob );
		}

		if ( !str_cmp(chck, "HitPrcnt") )
		{
			int percent;

			MP_ATOI( percent, rval, FALSE, -1 );
			return mprog_veval( chkchar->points[POINTS_LIFE]/chkchar->points_max[POINTS_LIFE], opr, percent, mob );
		}

		if ( !str_cmp(chck, "InRoom") )
		{
			VNUM vnum;

			MP_ATOI( vnum, rval, FALSE, MAX_VNUM );
			return mprog_veval( chkchar->in_room->vnum, opr, vnum, mob );
		}

		if ( !str_cmp(chck, "WasInRoom") )
		{
			VNUM vnum;

			if ( !chkchar->was_in_room )
				return FALSE;

			MP_ATOI( vnum, rval, FALSE, MAX_VNUM );
			return mprog_veval( chkchar->was_in_room->vnum, opr, vnum, mob );
		}

		if ( !str_cmp(chck, "NoRecall") )		return HAS_BIT( chkchar->in_room->flags, ROOM_NORECALL ) ? TRUE : FALSE;

		if ( !str_cmp(chck, "Sex") )
		{
			int sex;

			sex = code_code( NULL, rval, CODE_SEX );
			if ( sex == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: stringa di sesso passata errata: %s", chck, rval );
				return BERR;
			}

			return mprog_veval( chkchar->sex, opr, sex, mob );
		}

		if ( !str_cmp(chck, "Position") )
		{
			int pos;

			pos = code_code( NULL, rval, CODE_POSITION );
			if ( pos == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: stringa di posizione passata errata: %s", chck, rval );
				return BERR;
			}

			return mprog_veval( chkchar->position, opr, pos, mob );
		}

#ifdef T2_ALFA
		/* E' commentata perché quest_number è stata tolta come variabile, però il mprog è interessante è da tenere per qualcosa in futuro */
		if ( !str_cmp(chck, "DoingQuest") )
		{
			int quest;

			if ( IS_MOB(chkchar) )
				return FALSE;

			MP_ATOI( quest, rval, FALSE, -1 );	/* (FF) non avendo ancora quest, non saprei che massimo dargli */
			mprog_veval( chkchar->pg->quest_number, opr, quest, mob );
		}
#endif

		if ( !str_cmp(chck, "IsHelled") )
		{
			int date;

			if ( IS_MOB(chkchar) )
				return FALSE;

			MP_ATOI( date, rval, FALSE, -1 );
			mprog_veval( chkchar->pg->release_date, opr, date, mob );
		}

		if ( !str_cmp(chck, "Level") )
		{
			int level;

			MP_ATOI( level, rval, FALSE, LVL_LEGEND );
			return mprog_veval( chkchar->level, opr, level, mob );
		}

		if ( !str_cmp(chck, "Trust") )
		{
			int trust;

			trust = code_code( NULL, rval, CODE_TRUST );
			return mprog_veval( chkchar->level, opr, trust, mob );
		}

		if ( !str_cmp(chck, "GoldAmount") )
		{
			int gold;

			MP_ATOI( gold, rval, FALSE, -1 );
			return mprog_veval( chkchar->gold, opr, gold, mob );
		}

		if ( !str_cmp(chck, "IsClass") )
		{
			int class;

			class = code_code( NULL, rval, CODE_CLASS );
			return mprog_veval( chkchar->class, opr, class, mob );	/* (FF) sarebbe interessante rifare tutto il sistema in maniera tale da che abbia già il valore numerico della classe in memoria e lo debba solo confrontare */
		}

		if ( !str_cmp(chck, " CarryWeight") )
		{
			int weight;

			MP_ATOI( weight, rval, FALSE, -1 );
			return mprog_veval( chkchar->carry_weight, opr, weight, mob );
		}

		if ( !str_cmp(chck, "HostDesc") )
		{
			if ( IS_MOB(chkchar) )
				return FALSE;

			if ( !VALID_STR(chkchar->desc->host) )
				return FALSE;

#ifdef T2_ALFA
			/* (FF) fare una funzioncina che controlla se la stringa passata sia un indirizzo ip o no */
			if ( !check_string_ip(rval) )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: stringa passata non è un indirizzo ip: %s", chck, rval );
				return BERR;
			}
#endif

			return mprog_seval( chkchar->desc->host, opr, rval, mob );
		}

		if ( !str_cmp(chck, "Multi") )
		{
			CHAR_DATA *ch;

			lhsvl = 0;
			for ( ch = first_player;  ch;  ch = ch->next_player )
			{
				if ( !is_same_ip(ch, chkchar) )
					continue;
				lhsvl++;
			}

			MP_ATOI( rhsvl, rval, FALSE, -1 );
			return mprog_veval( lhsvl, opr, rhsvl, mob );
		}

		if ( !str_cmp(chck, "Race") )
		{
			int race;

			race = code_code( NULL, rval, CODE_RACE );
			if  ( race == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: stringa di razza passata errata: %s", chck, rval );
				return BERR;
			}

			return mprog_veval( chkchar->race, opr, race, mob );
		}

		if ( !str_cmp(chck, "Morph") )
		{
			VNUM vnum;

			if ( !chkchar->morph )
				return FALSE;

			if ( !chkchar->morph->morph )
				return FALSE;

			MP_ATOI( vnum, rval, FALSE, MAX_VNUM );
			return mprog_veval(chkchar->morph->morph->vnum, opr, vnum, mob );
		}

		if ( !str_cmp(chck, "Clan") )
		{
			if ( IS_MOB(chkchar) )
				return FALSE;

			if ( !chkchar->pg->clan || !chkchar->pg->member )
				return FALSE;

			return mprog_seval( chkchar->pg->clan->name, opr, rval, mob );
		}

		if ( !str_cmp(chck, "ClanType") )
		{
			int	type;

			if ( IS_MOB(chkchar) )
				return FALSE;

			if ( !chkchar->pg->clan || !chkchar->pg->member )
				return FALSE;

			MP_ATOI( type, rval, FALSE, MAX_CLANTYPE );
			return mprog_veval( chkchar->pg->clan->type, opr, type, mob );
		}

		/* Check added to see if the person isleader of a clan */
		if ( !str_cmp (chck, "IsLeader") )
		{
			CLAN_DATA *clan;

			if ( IS_MOB(chkchar) )
				return FALSE;

			if ( !chkchar->pg->clan || chkchar->pg->member )
				return FALSE;

			clan = get_clan( rval, TRUE );
			if ( !clan )
				return FALSE;

			if ( chkchar->pg->member->rank != clan->rank_limit-1 )
				return FALSE;

			return TRUE;
		}

		/* Is char wearing some eq on a specific wear loc? */
		if ( !str_cmp(chck, "Wearing") )
		{
			OBJ_DATA *obj_wear;
			int		  wear;

			wear = code_code( NULL, rval, CODE_WEARFLAG );
			if ( wear == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: codice wear flag passato errato: %s", chck, rval );
				return BERR;
			}

			for ( obj_wear = chkchar->first_carrying;  obj_wear;  obj_wear = obj_wear->next_content )
			{
				if ( chkchar != obj_wear->carried_by )	continue;
				if ( obj_wear->wear_loc == -1 )			continue;

				if ( obj_wear->wear_loc == wear )
					return TRUE;
			}

			return FALSE;
		}

		/* Is char wearing some specific vnum? */
		if ( !str_cmp (chck, "WearingVnum") )
		{
			OBJ_DATA *obj_wear;
			VNUM	  vnum;

			MP_ATOI( vnum, rval, FALSE, MAX_VNUM );

			for ( obj_wear = chkchar->first_carrying;  obj_wear;  obj_wear = obj_wear->next_content )
			{
				if ( chkchar != obj_wear->carried_by )	continue;
				if ( obj_wear->wear_loc == -1 )			continue;

				if ( obj_wear->vnum == vnum )
					return TRUE;
			}

			return FALSE;
		}

		/* Is char carrying a specific piece of eq? */
		if ( !str_cmp(chck, "CarryingVnum") )
		{
			VNUM vnum;

			if ( !chkchar->first_carrying )
				return FALSE;

			MP_ATOI( vnum, rval, FALSE, MAX_VNUM );
			return ( carryingvnum_visit(chkchar, chkchar->first_carrying, vnum) );
		}

		if ( !str_cmp(chck, "WaitState") )
		{
			int	wait;

			if ( IS_MOB(chkchar) )
				return FALSE;

			if ( chkchar->wait == 0 )
				return FALSE;

			MP_ATOI( wait, rval, FALSE, -1 );
			return mprog_veval( chkchar->wait, opr, wait, mob );
		}

		if ( !str_cmp(chck, "ASupressed") )
		{
			int	atime;

			MP_ATOI( atime,  rval, FALSE, -1 );
			return mprog_veval( get_timer(chkchar, TIMER_ASUPRESSED), opr, atime, mob );
		}

		if ( !str_cmp(chck, "Point") )
		{
			char	arg[MIL];
			int		point;
			int		value;

			rval = one_argument( rval, arg );
			point = code_code( NULL, arg, CODE_POINTS );		/* (TT) dovrò usare lval o cvar? */
			if ( point == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: tipo di punteggio passato errato: %s", chck, arg );
				return BERR;
			}

			MP_ATOI( value, rval, FALSE, 32700 );

			return mprog_veval( chkchar->points[point], opr, value, mob );
		}

		if ( !str_cmp(chck, "Attr") )
		{
			char	arg[MIL];
			int		attr;
			int		value;

			rval = one_argument( rval, arg );
			attr = code_code( NULL, arg, CODE_POINTS );		/* (TT) dovrò usare lval o cvar? */
			if ( attr == -1 )
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: tipo di attributo passato errato: %s", chck, arg );
				return BERR;
			}

			MP_ATOI( value, rval, FALSE, MAX_ATTR );
			return mprog_veval( get_curr_attr(chkchar, attr), opr, value, mob );
		}
	}

	/* ifchecks utilizzati dagli oggetti mudprog */
	if ( chkobj )
	{
		if ( !str_cmp(chck, "ObjType") )
		{
			int type;

			type = code_code( NULL, rval, CODE_OBJTYPE );
			return mprog_veval( chkobj->type, opr, type, mob );
		}

		if ( !str_cmp(chck, "LeverPos") )
		{
			bool	isup	= FALSE;
			bool	wantsup	= FALSE;

			if ( chkobj->type != OBJTYPE_SWITCH
			  || chkobj->type != OBJTYPE_LEVER
			  || chkobj->type != OBJTYPE_PULLCHAIN )
			{
				return FALSE;
			}

			if ( HAS_BITINT(obj->lever->flags, TRIG_UP) )
				isup = TRUE;

			if ( !str_cmp(rval, "up") )
				wantsup = TRUE;
			else if ( !str_cmp(rval, "down") )
				wantsup = FALSE;
			else
			{
				send_mplog( mob, "mprog_do_ifcheck: %s: tipo di posizione leva errato: %s", chck, rval );
				return BERR;
			}

			return mprog_veval( wantsup, opr, isup, mob );
		}

/*	(FF) troppo casino fare questo pezzo di codice per ora, da tenere per il futuro, magari usare quel codice con l'accesso alle variabile tramite .xxx
		if ( !str_cmp(chck, "objval0") )	return mprog_veval( chkobj->value[0], opr, atoi(rval), mob );
		if ( !str_cmp(chck, "objval1") )	return mprog_veval( chkobj->value[1], opr, atoi(rval), mob );
		if ( !str_cmp(chck, "objval2") )	return mprog_veval( chkobj->value[2], opr, atoi(rval), mob );
		if ( !str_cmp(chck, "objval3") )	return mprog_veval( chkobj->value[3], opr, atoi(rval), mob );
		if ( !str_cmp(chck, "objval4") )	return mprog_veval( chkobj->value[4], opr, atoi(rval), mob );
		if ( !str_cmp(chck, "objval5") )	return mprog_veval( chkobj->value[5], opr, atoi(rval), mob );
*/
	} /* chiude l'if chkobj */


	/*
	 * The following checks depend on the fact that cval[1] can only contain
	 * one character, and that NULL checks were made previously.
	 */

	if ( !str_cmp(chck, "Number") )
	{
		int num;

		if ( chkchar )
		{
			if ( IS_PG(chkchar) )
				return FALSE;

			if ( chkchar == mob )
			{
				lhsvl = chkchar->gold;
				MP_ATOI( num, rval, FALSE, -1 );
			}
			else
			{
				lhsvl = chkchar->pIndexData->vnum;
				MP_ATOI( num, rval, FALSE, MAX_VNUM );
			}

			return mprog_veval( lhsvl, opr, num, mob );
		}

		if ( chkobj )
		{
			MP_ATOI( num, rval, TRUE, -1 );	/* check negative a TRUE visto che non si può sapere a priori a cosa possa servire questo ifcheck Number */
			return mprog_veval( chkobj->vnum, opr, num, mob );
		}

		send_mplog( mob, "mprog_do_ifcheck: %s: chkchar e chkobj sono NULL", chck );
		return BERR;
	}

	if ( !str_cmp(chck, "Time") )
	{
		int hour;

		MP_ATOI( hour, rval, FALSE, -1 );
		return mprog_veval( calendar.hour, opr, hour, mob );
	}

	if ( !str_cmp(chck, "Name") )
	{
		if ( chkchar )
			return mprog_seval( chkchar->name, opr, rval, mob );

		if ( chkobj )
			return mprog_seval( chkobj->name, opr, rval, mob );

		send_mplog( mob, "mprog_do_ifcheck: %s: chkchar e chkobj sono NULL", chck );
		return BERR;
	}

	if ( !str_cmp(chck, "Rank") )
	{
		int rank;

		if ( !chkchar || IS_MOB(chkchar) )
			return FALSE;

		MP_ATOI( rank, rval, FALSE, MAX_RANK );
		return mprog_veval( chkchar->pg->member->rank, opr, rank, mob );
	}


	/* Ok... all the ifchecks are done, so if we didnt find ours then something
	 *	odd happened.  So report the bug and abort the MUDprogram (return error)
	 */
	send_mplog( mob, "mprog_do_ifcheck: ifcheck sconosciuto: %s", chck );
	return BERR;
}

#undef isoperator

#undef MP_ATOI

#undef INROOM
#undef IN_AREA
#undef IN_MUD

#undef MAX_IF_ARGS


/* This routine handles the variables for command expansion.
 * If you want to add any go right ahead, it should be fairly
 *  clear how it is done and they are quite easy to do, so you
 *  can be as creative as you want. The only catch is to check
 *  that your variables exist before you use them. At the moment,
 *  using $t when the secondary target refers to an object
 *  i.e. >prog_act drops~<nl>if ispg($t)<nl>sigh<nl>endif<nl>~<nl>
 *  probably makes the mud crash (vice versa as well) The cure
 *  would be to change act() so that vo becomes vict & v_obj.
 * But this would require a lot of small changes all over the code.
 */

/*
 * There's no reason to make the mud crash when a variable's
 *  fubared.  I added some ifs.  I'm willing to trade some
 *  performance for stability.
 *
 *  Added char_died and obj_extracted checks
 */
void mprog_translate( char ch, char *t, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo, CHAR_DATA *rndm )
{
	static char *he_she [] = { "egli", "egli", "ella" };
	static char *him_her[] = { "gli", "gli", "le" };
	static char *his_her[] = { "suo", "suo", "sua" };
	CHAR_DATA	*vict	   = (CHAR_DATA *) vo;
	OBJ_DATA	*v_obj	   = (OBJ_DATA  *) vo;

	if ( v_obj && v_obj->serial )
		vict = NULL;
	else
		v_obj = NULL;

	*t = '\0';
	switch ( ch )
	{
	  case 'i':
		if ( mob && !char_died(mob) )
		{
			if ( mob->name )
				one_argument( mob->name, t );
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'I':
		if ( mob && !char_died(mob) )
		{
			if ( mob->short_descr )
				strcpy( t, mob->short_descr );
			else
				strcpy( t, "someone" );
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'n':
		if ( actor && !char_died(actor) )
		{
			if ( can_see(mob,actor) )
				one_argument( actor->name, t );

			if ( IS_PG( actor ) )
				*t = toupper( *t );
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'N':
		if ( actor && !char_died(actor) )
		{
			if ( can_see(mob, actor) )
			{
				if ( IS_MOB(actor) )
					strcpy( t, actor->short_descr );
				else
					sprintf( t, "%s %s", actor->name, actor->pg->title );
			}
			else
			{
				strcpy( t, "someone" );
			}
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 't':
		if ( vict && !char_died(vict) )
		{
			if ( can_see(mob, vict) )
				one_argument( vict->name, t );
			if ( IS_PG(vict) )
				*t = toupper( *t );
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'T':
		if ( vict && !char_died(vict) )
		{
			if ( can_see(mob, vict) )
			{
				if ( IS_MOB(vict) )
					strcpy( t, vict->short_descr );
				else
					sprintf( t, "%s %s", vict->name, vict->pg->title );
			}
			else
			{
				strcpy( t, "someone" );
			}
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'r':
		if ( rndm && !char_died(rndm) )
		{
			if ( can_see(mob, rndm) )
			{
				one_argument( rndm->name, t );
			}
			if ( IS_PG(rndm) )
			{
				*t = toupper( *t );
			}
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'R':
		if ( rndm && !char_died(rndm) )
		{
			if ( can_see(mob, rndm) )
			{
				if ( IS_MOB(rndm) )
					strcpy( t,rndm->short_descr );
				else
					sprintf( t, "%s %s", rndm->name, rndm->pg->title );
			}
			else
			{
				strcpy( t, "someone" );
			}
		}
		else
		{
			strcpy( t, "someone" );
		}
		break;

	  case 'e':
		if ( actor && !char_died(actor) )
			can_see(mob, actor)  ?  strcpy( t, he_she[actor->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "he" );
		break;

	  case 'm':
		if ( actor && !char_died(actor) )
			can_see( mob, actor )  ?  strcpy( t, him_her[actor->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "him" );
		break;

	  case 'x':
		if ( actor && !char_died(actor) )
			( actor->sex == SEX_FEMALE )  ?  strcpy( t, "a" )  :  strcpy( t, "o" );
		else
			strcpy( t, "o" );
		break;

	  case 'E':
		if ( vict && !char_died(vict) )
			can_see( mob, vict )  ?  strcpy( t, he_she[vict->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "he" );
		break;

	  case 'M':
		if ( vict && !char_died(vict) )
			can_see( mob, vict )  ?  strcpy( t, him_her[vict->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "him" );
		break;

	  case 'X':
		if ( vict && !char_died(vict) )
			( actor->sex == SEX_FEMALE )  ?  strcpy( t, "a" )  :  strcpy( t, "o" );
		else
			strcpy( t, "o" );
		break;

	  case 'j':
		if ( mob && !char_died(mob) )
			strcpy( t, he_she[ mob->sex ] );
		else
			strcpy( t, "he" );
		break;

	  case 'k':
		if ( mob && !char_died(mob) )
			strcpy( t, him_her[ mob->sex ] );
		else
			strcpy( t, "him" );
		break;

	  case 'l':
		if ( mob && !char_died(mob) )
			strcpy( t, his_her[ mob->sex ] );
		else
			strcpy( t, "his" );
		break;

	  case 'J':
		if ( rndm && !char_died(rndm) )
			can_see( mob, rndm )  ?  strcpy( t, he_she[rndm->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "he" );
		break;

	  case 'K':
		if ( rndm && !char_died(rndm) )
			can_see( mob, rndm )  ?  strcpy( t, him_her[rndm->sex] )  :  strcpy( t, "someone's" );
		else
			strcpy( t, "him" );
		break;

	  case 'L':
		if ( rndm && !char_died(rndm) )
			can_see( mob, rndm )  ?  strcpy( t, his_her[rndm->sex] )  :  strcpy( t, "someone" );
		else
			strcpy( t, "his" );
		break;

	  /* equip a FALSE perché anche se addosso non vedono cosa sia esattamente */
	  case 'o':
		if ( obj && !obj_extracted(obj) )
			can_see_obj( mob, obj, FALSE )  ?  one_argument( obj->name, t )  :  strcpy( t, "something" );
		else
			strcpy( t, "something" );
		break;

	  case 'O':
		if ( obj && !obj_extracted(obj) )
			can_see_obj( mob, obj, FALSE )  ?  strcpy( t, obj->short_descr )  :  strcpy( t, "something" );
		else
			strcpy( t, "something" );
		break;

	  case 'p':
		if ( v_obj && !obj_extracted(v_obj) )
			can_see_obj( mob, v_obj, FALSE )  ?  one_argument( v_obj->name, t )  :  strcpy( t, "something" );
		else
			strcpy( t, "something" );
		break;

	  case 'P':
		if ( v_obj && !obj_extracted(v_obj) )
			can_see_obj( mob, v_obj, FALSE )  ?  strcpy( t, v_obj->short_descr )  :  strcpy( t, "something" );
		else
			strcpy( t, "something" );
		break;

	  case 'a':
		if ( obj && !obj_extracted(obj) )
		{
			strcpy( t, aoran(obj->name) );
/* (RR) da rivederlo con le funzioni gramm
			switch ( *(obj->name) )
			{
			case 'a':
			case 'e':
			case 'i':
			case 'o':
			case 'u':
				strcpy( t, "an" );
				break;
			default:
				strcpy( t, "a" );
			}
*/
		}
		else
		{
			strcpy( t, "a" );
		}
		break;

	  case 'A':
		if ( v_obj && !obj_extracted(v_obj) )
			strcpy( t, aoran(v_obj->name) );
		else
			strcpy( t, "a" );
		break;

	  case '$':
		strcpy( t, "$" );
		break;

	  default:
		send_mplog( mob, "mprog_translate: $var errata" );
		break;
	}
}


/* This function replaces mprog_process_cmnd.
 * It is called from mprog_driver, once for each line in a mud prog.
 * This function checks what the line is, executes if/or checks and calls send_command
 *  to perform the command
 */
int mprog_do_command( MPROG_COM_LIST *mpcom, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo,
	CHAR_DATA *rndm, bool ignore, bool ignore_ors )
{
	char	*ifcheck;
	char	*point;
	char	*str;
	char	*i;
	char	 firstword[MIL];
	char	 buf[MIL];
	char	 tmp[MIL];
	int		 validif;
	VNUM	 vnum;

	if ( !mpcom )
	{
		send_mplog( mob, "mprog_do_command: mpcom è NULL" );
		return BERR;
	}

	/* Isolate the first word of the line, it gives us a clue what
	 * we want to do. */
	ifcheck = one_argument( mpcom->com, firstword );

	/* Ok, we found an if.  According to the boolean 'ignore', either
	 * ignore the ifcheck and report that back to mprog_driver or do
	 * the ifcheck and report whether it was successful.
	 */
	if ( !str_cmp(firstword, "if") )
	{
		if ( ignore )
			return IFIGNORED;

		validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

		if ( validif == 1 )
			return IFTRUE;

		if ( validif == 0 )
			return IFFALSE;

		return BERR;
	}

	/* Same behavior as with ifs, but use the boolean 'ignore_ors' to
	 * decide which way to go
	 */
	if ( !str_cmp(firstword, "or") )
	{
		if ( ignore_ors )
			return ORIGNORED;

		validif = mprog_do_ifcheck( ifcheck, mob, actor, obj, vo, rndm );

		if ( validif == 1 )
			return ORTRUE;

		if ( validif == 0 )
			return ORFALSE;

		return BERR;
	}

	/* For else and endif, just report back what we found.
	 * Mprog_driver keeps track of logiclevels. */
	if ( !str_cmp(firstword, "else") )
		return FOUNDELSE;

	if ( !str_cmp(firstword, "endif") )
		return FOUNDENDIF;

	/* Ok, didn't find an if, an or, an else or an endif.
	 * If the command is in an if or else section that is not to be
	 * performed, the boolean 'ignore' is set to true and we just
	 * return.  If not, we try to execute the command.
	 */
	if ( ignore )
		return COMMANDOK;

	/* If the command is 'break', that's all folks. */
	if ( !str_cmp(firstword, "break") )
		return BERR;

#ifdef T2_ALFA
	if ( !str_cmp(firstword, "continue") )
		/* farlo andare al primo comando di mud prog */
#endif

	point = buf;
	str   = mpcom->com;

	while ( *str != '\0' )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}
		str++;
		mprog_translate( *str, tmp, mob, actor, obj, vo, rndm );
		i = tmp;
		++str;
		while ( (*point = *i) != '\0' )
			++point, ++i;
	}
	*point = '\0';

	/* Salva il vnum del mob che potrebbe morire e quindi non sarebbe più ricavabile */
	vnum  = mob->pIndexData->vnum;

	/* Invia il comando (FF) find_command che ritorna il comando qui, senza check sul livello, così da poter inviare anche comandi da admin, alcuni comandi di admin non possono venire inviati grazia ad una flag di CMDFLAG_NOMPROG, che funziona da trust */
	send_command( mob, buf, CO );

	/* Potrebbe capitare qualche mprog in cui il mob muoia quindi bisogna assicurarsi che sia ancora vivo */
	if ( char_died(mob) )
	{
		send_log( NULL, LOG_MPROG, "mprog_do_command: mob died while executing program, vnum %d", vnum );
		return BERR;
	}

	return COMMANDOK;
}


/*
 * The main focus of the MUDprograms.
 * This routine is called whenever a trigger is successful.
 * It is responsible for parsing the command list and figuring out what to do.
 * However, like all complex procedures, everything is farmed out to the other guys.
 * The max logiclevel (MAX_IFS) is defined at the beginning of this file,
 *  use it to increase/decrease max allowed nesting.
 */
void mprog_driver( MPROG_DATA *mprog, CHAR_DATA *mob, CHAR_DATA *actor, OBJ_DATA *obj, void *vo )
{
	CHAR_DATA		*rndm = NULL;
	CHAR_DATA		*vch  = NULL;
	MPROG_COM_LIST	*mpcom = NULL;	/* Puntatore all'attuale comando mudprog che sta inviando */
	static int		 prog_nest;
	int				 count = 0;
	int				 ignorelevel  = 0;
	int				 iflevel;
	int				 result;
	bool			 ifstate[MAX_IFS][DO_ELSE+1];

	if ( HAS_BIT(mob->affected_by, AFFECT_CHARM) )
		return;

	if ( !mprog->first_mpcom )
	{
		send_mplog( mob, "mprog_driver: mob senza neppure un comando mud prog" );
		return;
	}

	/* Next couple of checks stop program looping. */
	if ( mob == actor )
	{
		send_mplog( mob, "mprog_driver: triggering oneself" );
		return;
	}

	if ( ++prog_nest > MAX_PROG_NEST )
	{
		send_mplog( mob, "mprog_driver: superato MAX_PROG_NEST per %s",
			mprog->first_mpcom->com );
		--prog_nest;
		return;
	}

	/* Make sure all ifstate bools are set to FALSE */
	for ( iflevel = 0;  iflevel < MAX_IFS;  iflevel++ )
	{
		for ( count = 0;  count < DO_ELSE;  count++ )
			ifstate[iflevel][count] = FALSE;
	}

	iflevel = 0;

	/*
	 * get a random visible player who is in the room with the mob.
	 *
	 * If there isn't a random player in the room, rndm stays NULL.
	 * If you do a $r, $R, $j, or $k with rndm = NULL, you'll crash in mprog_translate.
	 *
	 * Adding appropriate error checking in mprog_translate.
	 */
	count = 0;
	for ( vch = mob->in_room->first_person;  vch;  vch = vch->next_in_room )
	{
		if ( IS_MOB(vch) )			continue;
		if ( !can_see(mob, vch) )	continue;

		if ( number_range(0, count) == 0 )
			rndm = vch;

		count++;
	}

	while ( TRUE )
	{
		/* Se è NULL allora passa il primo comando del mudprog, altrimenti passa al sucessivo */
		if ( !mpcom )
			mpcom = mprog->first_mpcom;
		else
			mpcom = mpcom->next;

		/* Controlla se siamo arrivati alla fine con l'ultimo comando di mudprog */
		if ( !mpcom )
		{
			/* Confrontando l'array della posizione per vedere se nel mprog manca un endif */
			if ( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] )
				send_mplog( mob, "mprog_driver: endif mancante per %s", mprog->first_mpcom->com );

			--prog_nest;
			return;
		}

		/* Esegue il comando e salva il risultato */
		result = mprog_do_command( mpcom, mob, actor, obj, vo, rndm,
			(ifstate[iflevel][IN_IF]   && !ifstate[iflevel][DO_IF]) ||
			(ifstate[iflevel][IN_ELSE] && !ifstate[iflevel][DO_ELSE]),
			(ignorelevel > 0) );

		/* (FF) qui c'era anche il check per l'uscita con il single_step */


		/* Ora controlla il risultato sulla base del suo stato logico attuale */
		switch ( result )
		{
		  case COMMANDOK:
#ifdef DEBUG_MPROG
			send_mplog( mob, "COMMANDOK" );
#endif
			/* Ok, this one's a no-brainer. */
			continue;
			break;

		  case IFTRUE:
#ifdef DEBUG_MPROG
			send_mplog( mob, "IFTRUE" );
#endif
			/* An if was evaluated and found true.  Note that we are in an
			 * if section and that we want to execute it. */

			iflevel++;
			if ( iflevel == MAX_IFS )
			{
				send_mplog( mob, "mprog_driver: Maximum nested ifs exceeded" );
				--prog_nest;
				return;
			}

			ifstate[iflevel][IN_IF] = TRUE;
			ifstate[iflevel][DO_IF] = TRUE;
			break;

		  case IFFALSE:
#ifdef DEBUG_MPROG
			send_mplog( mob, "IFFALSE" );
#endif
			/* An if was evaluated and found false.  Note that we are in an
			 * if section and that we don't want to execute it unless we find
			 * an or that evaluates to true. */
			iflevel++;
			if ( iflevel == MAX_IFS )
			{
				send_mplog( mob, "mprog_driver: Maximum nested ifs exceeded" );
				--prog_nest;
				return;
			}
			ifstate[iflevel][IN_IF] = TRUE;
			ifstate[iflevel][DO_IF] = FALSE;
			break;

		  case ORTRUE:
#ifdef DEBUG_MPROG
			send_mplog( mob, "ORTRUE" );
#endif
			/* An or was evaluated and found true.  We should already be in an
			 * if section, so note that we want to execute it. */
			if ( !ifstate[iflevel][IN_IF] )
			{
				send_mplog( mob, "mprog_driver: Unmatched or" );
				--prog_nest;
				return;
			}
			ifstate[iflevel][DO_IF] = TRUE;
			break;

		  case ORFALSE:
#ifdef DEBUG_MPROG
			send_mplog( mob, "ORFALSE" );
#endif
			/* An or was evaluated and found false.  We should already be in an
			 * if section, and we don't need to do much.  If the if was true or
			 * there were/will be other ors that evaluate(d) to true, they'll set
			 * do_if to true. */
			if ( !ifstate[iflevel][IN_IF] )
			{
				send_mplog( mob, "mprog_driver: Unmatched or" );
				--prog_nest;
				return;
			}
			continue;
			break;

		  case FOUNDELSE:
#ifdef DEBUG_MPROG
			send_mplog( mob, "FOUNDELSE" );
#endif
			/* Found an else.  Make sure we're in an if section, bug out if not.
			 * If this else is not one that we wish to ignore, note that we're now
			 * in an else section, and look at whether or not we executed the if
			 * section to decide whether to execute the else section.  Ca marche
			 * bien. */
			if ( ignorelevel > 0 )
				continue;

			if ( ifstate[iflevel][IN_ELSE] )
			{
				send_mplog( mob, "mprog_driver: Found else in an else section" );
				--prog_nest;
				return;
			}

			if ( !ifstate[iflevel][IN_IF] )
			{
				send_mplog( mob, "mprog_driver: Unmatched else" );
				--prog_nest;
				return;
			}

			ifstate[iflevel][IN_ELSE] = TRUE;
			ifstate[iflevel][DO_ELSE] = !ifstate[iflevel][DO_IF];
			ifstate[iflevel][IN_IF]   = FALSE;
			ifstate[iflevel][DO_IF]   = FALSE;

			break;

		  case FOUNDENDIF:
#ifdef DEBUG_MPROG
			send_mplog( mob, "FOUNDENDIF" );
#endif
			/* Hmm, let's see... FOUNDENDIF must mean that we found an endif.
			 * So let's make sure we were expecting one, return if not.  If this
			 * endif matches the if or else that we're executing, note that we are
			 * now no longer executing an if.  If not, keep track of what we're
			 * ignoring. */
			if ( !( ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE] ) )
			{
				send_mplog( mob, "mprog_driver: Unmatched endif" );
				--prog_nest;
				return;
			}

			if ( ignorelevel > 0 )
			{
				ignorelevel--;
				continue;
			}

			ifstate[iflevel][IN_IF]   = FALSE;
			ifstate[iflevel][DO_IF]   = FALSE;
			ifstate[iflevel][IN_ELSE] = FALSE;
			ifstate[iflevel][DO_ELSE] = FALSE;

			iflevel--;
			break;

		  case IFIGNORED:
#ifdef DEBUG_MPROG
			send_mplog( mob, "IFIGNORED" );
#endif
			if ( !(ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE]) )
			{
				send_mplog( mob, "mprog_driver: Parse error, ignoring if while not in if or else" );
				--prog_nest;
				return;
			}
			ignorelevel++;
			break;

		  case ORIGNORED:
#ifdef DEBUG_MPROG
			send_mplog( mob, "ORIGNORED" );
#endif
			if ( !(ifstate[iflevel][IN_IF] || ifstate[iflevel][IN_ELSE]) )
			{
				send_mplog( mob, "mprog_driver: Unmatched or" );
				--prog_nest;
				return;
			}

			if ( ignorelevel == 0 )
			{
				send_mplog( mob, "mprog_driver: Parse error, mistakenly ignoring or" );
				--prog_nest;
				return;
			}
			break;

		  case BERR:
#ifdef DEBUG_MPROG
			send_mplog( mob, "BERR" );
#endif
			--prog_nest;
			return;
			break;
		}
	}

	--prog_nest;
}
