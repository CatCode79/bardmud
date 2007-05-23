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
 >						Online Reset Editing Module							<
\****************************************************************************/


/*
 * This file relies heavily on the fact that your linked lists are correct,
 * and that pArea->reset_first is the first reset in pArea.  Likewise,
 * pArea->reset_last *MUST* be the last reset in pArea.  Weird and
 * wonderful things will happen if any of your lists are messed up, none
 * of them good.  The most important are your pRoom->contents,
 * pRoom->people, rch->carrying, obj->contains, and pArea->reset_first ..
 * pArea->reset_last.
 */

#include <ctype.h>

#include "mud.h"
#include "affect.h"
#include "build.h"
#include "calendar.h"
#include "command.h"
#include "db.h"
#include "economy.h"
#include "editor.h"
#include "fread.h"
#include "interpret.h"
#include "make_obj.h"
#include "movement.h"
#include "reset.h"
#include "room.h"
#include "utility.h"


/*
 * Variabili esterne
 */
int		top_reset;		/* Numero di reset totali nel Mud */


/*
 * Funzioni locali
 */
void edit_reset( CHAR_DATA *ch, char *argument, AREA_DATA *pArea, ROOM_DATA *aRoom );


/*
 * This is one loopy function.  Ugh.
 */
bool is_room_reset( RESET_DATA *pReset, ROOM_DATA *aRoom, AREA_DATA *pArea )
{
	ROOM_DATA	*pRoom;
	RESET_DATA	*reset;
	int			 pr;

	if ( !aRoom )
		return TRUE;

	switch ( pReset->command )
	{
	  case 'M':
	  case 'O':
		pRoom = get_room_index( NULL, pReset->arg3 );
		if ( !pRoom || pRoom != aRoom )
			return FALSE;
		return TRUE;

	  case 'P':
	  case 'T':
	  case 'H':
		if ( pReset->command == 'H' )
			pr = pReset->arg1;
		else
			pr = pReset->arg3;
		for ( reset = pReset->prev;  reset;  reset = reset->prev )
		{
			if ( (reset->command == 'O' || reset->command == 'P'
			  ||  reset->command == 'G' || reset->command == 'E')
			  && (!pr || pr == reset->arg1) && get_obj_proto(NULL, reset->arg1) )
			{
				break;
			}
		}
		if ( reset && is_room_reset(reset, aRoom, pArea) )
			return TRUE;
		return FALSE;

	  case 'B':
		switch ( pReset->arg2 & BIT_RESET_TYPE_MASK )
		{
		  case BIT_RESET_DOOR:
		  case BIT_RESET_ROOM:
			return ( aRoom->vnum == pReset->arg1 );

		  case BIT_RESET_MOBILE:
			for ( reset = pReset->prev;  reset;  reset = reset->prev )
			{
				if ( reset->command == 'M' && get_mob_index(NULL, reset->arg1) )
					break;
			}
			if ( reset && is_room_reset(reset, aRoom, pArea) )
				return TRUE;
			return FALSE;

		  case BIT_RESET_OBJECT:
			for ( reset = pReset->prev;  reset;  reset = reset->prev )
			{
				if ( (reset->command == 'O' || reset->command == 'P'
				  ||  reset->command == 'G' || reset->command == 'E')
				  && (!pReset->arg1 || pReset->arg1 == reset->arg1)
				  && get_obj_proto(NULL, reset->arg1) )
				{
					break;
				}
			}
			if ( reset && is_room_reset(reset, aRoom, pArea) )
				return TRUE;
			return FALSE;
		}
		return FALSE;

	  case 'G':
	  case 'E':
		for ( reset = pReset->prev;  reset;  reset = reset->prev )
		{
			if ( reset->command == 'M' && get_mob_index(NULL, reset->arg1) )
				break;
		}

		if ( reset && is_room_reset(reset, aRoom, pArea) )
			return TRUE;
		return FALSE;

	  case 'D':
	  case 'R':
		pRoom = get_room_index( NULL, pReset->arg1 );
		if ( !pRoom || pRoom->area != pArea || (aRoom && pRoom != aRoom) )
			return FALSE;
		return TRUE;

	  default:
		return FALSE;
	}

	return FALSE;
}


RESET_DATA *find_reset( AREA_DATA *pArea, ROOM_DATA *pRoom, int numb )
{
	RESET_DATA *pReset;
	int			num = 0;

	for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
	{
		if ( is_room_reset(pReset, pRoom, pArea) && ++num >= numb )
			return pReset;
	}

	return NULL;
}


ROOM_DATA *find_room( CHAR_DATA *ch, char *argument, ROOM_DATA *pRoom )
{
	char arg[MIL];

	if ( pRoom )
		return pRoom;
	one_argument( argument, arg );

	if ( !VALID_STR(arg) && !is_number(arg) )
	{
		send_to_char( ch, "Reset di quale room?\r\n" );
		return NULL;
	}

	if ( !VALID_STR(arg) )
		pRoom = ch->in_room;
	else
		pRoom = get_room_index( NULL, atoi(arg) );

	if ( !pRoom )
	{
		send_to_char( ch, "La stanza non esiste.\r\n" );
		return NULL;
	}

	return pRoom;
}


/*
 * Separate function for recursive purposes
 */
#define DEL_RESET( area, reset, rprev )	\
do										\
{										\
	rprev = reset->prev;				\
	delete_reset( area, reset );		\
	reset = rprev;						\
	continue;							\
} while( 0 )


void delete_reset( AREA_DATA *pArea, RESET_DATA *pReset )
{
	RESET_DATA *reset;
	RESET_DATA *reset_prev;

	if ( pReset->command == 'M' )
	{
		for ( reset = pReset->next;  reset;  reset = reset->next )
		{
			/* Break when a new mob found */
			if ( reset->command == 'M' )
				break;
			/* Delete anything mob is holding */
			if ( reset->command == 'G' || reset->command == 'E' )
				DEL_RESET( pArea, reset, reset_prev );
			if ( reset->command == 'B'
				&& (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_MOBILE
				&& (!reset->arg1 || reset->arg1 == pReset->arg1) )
			{
				DEL_RESET( pArea, reset, reset_prev );
			}
		}
	}
	else if ( pReset->command == 'O' || pReset->command == 'P'
	  || pReset->command == 'G' || pReset->command == 'E' )
	{
		for ( reset = pReset->next; reset; reset = reset->next )
		{
			if ( reset->command == 'T' && (!reset->arg3 || reset->arg3 == pReset->arg1) )
				DEL_RESET(pArea, reset, reset_prev);

			if ( reset->command == 'H' && (!reset->arg1 || reset->arg1 == pReset->arg1) )
				DEL_RESET(pArea, reset, reset_prev);

			/* Delete nested objects, even if they are the same object. */
			if ( reset->command == 'P' && (reset->arg3 > 0
			  || pReset->command != 'P' || reset->extra-1 == pReset->extra)
			  && (!reset->arg3 || reset->arg3 == pReset->arg1) )
			{
				DEL_RESET( pArea, reset, reset_prev );
			}

			if ( reset->command == 'B'
			  && (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT
			  && (!reset->arg1 || reset->arg1 == pReset->arg1) )
			{
				DEL_RESET( pArea, reset, reset_prev );
			}

			/* Break when a new object of same type is found */
			if ( (reset->command == 'O' || reset->command == 'P'
			  ||  reset->command == 'G' || reset->command == 'E')
			  &&  reset->arg1 == pReset->arg1 )
			{
				break;
			}
		}
	}

	if ( pReset == pArea->last_mob_reset )
		pArea->last_mob_reset = NULL;
	if ( pReset == pArea->last_obj_reset )
		pArea->last_obj_reset = NULL;

	UNLINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );
	DISPOSE( pReset );
}

#undef DEL_RESET


RESET_DATA *find_oreset( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_DATA *pRoom, char *name )
{
	RESET_DATA *reset;

	if ( !VALID_STR(name) )
	{
		for ( reset = pArea->last_reset;  reset;  reset = reset->prev )
		{
			if ( !is_room_reset(reset, pRoom, pArea) )
				continue;

			switch ( reset->command )
			{
			  default:
				continue;
			  case 'O':
			  case 'E':
			  case 'G':
			  case 'P':
				break;
			}
			break;
		}

		if ( !reset )
			send_to_char(ch, "No object resets in list.\r\n" );

		return reset;
	}
	else
	{
		OBJ_PROTO_DATA *pObjTo = NULL;
		char			arg[MIL];
		int				cnt = 0;
		int				num = number_argument( name, arg );

		for ( reset = pArea->first_reset;  reset;  reset = reset->next )
		{
			if ( !is_room_reset(reset, pRoom, pArea) )
				continue;

			switch ( reset->command )
			{
			  default:
				continue;
			  case 'O':
			  case 'E':
			  case 'G':
			  case 'P':
				break;
			}

			pObjTo = get_obj_proto( NULL, reset->arg1 );
			if ( pObjTo && is_name(arg, pObjTo->name) && ++cnt == num )
				break;
		}

		if ( !pObjTo || !reset )
		{
			send_to_char( ch, "To object not in reset list.\r\n" );
			return NULL;
		}
	}

	return reset;
}


RESET_DATA *find_mreset( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_DATA *pRoom, char *name )
{
	RESET_DATA *reset;

	if ( !VALID_STR(name) )
	{
		for ( reset = pArea->last_reset;  reset;  reset = reset->prev )
		{
			if ( !is_room_reset(reset, pRoom, pArea) )
				continue;

			switch ( reset->command )
			{
			  default:
				continue;
			  case 'M':
				break;
			}
			break;
		}

		if ( !reset )
			send_to_char( ch, "No mobile resets in list.\r\n" );

		return reset;
	}
	else
	{
		MOB_PROTO_DATA *pMob = NULL;
		char			arg[MIL];
		int				cnt = 0;
		int				num = number_argument( name, arg );

		for ( reset = pArea->first_reset;  reset;  reset = reset->next )
		{
			if ( !is_room_reset(reset, pRoom, pArea) )
				continue;

			switch ( reset->command )
			{
			  default:
				continue;
			  case 'M':
				break;
			}

			if ( (pMob = get_mob_index(NULL, reset->arg1))
			  && is_name(arg, pMob->name) && ++cnt == num )
			{
				break;
			}
		}

		if ( !pMob || !reset )
		{
			send_to_char( ch, "Mobile not in reset list.\r\n" );
			return NULL;
		}
	}

	return reset;
}


void list_resets( CHAR_DATA *ch, AREA_DATA *pArea, ROOM_DATA *pRoom, int start, int end )
{
	RESET_DATA		*pReset;
	ROOM_DATA		*room;
	MOB_PROTO_DATA	*mob;
	OBJ_PROTO_DATA	*obj;
	OBJ_PROTO_DATA	*obj2;
	OBJ_PROTO_DATA	*lastobj;
	RESET_DATA		*lo_reset;
	const char		*rname = "???";
	const char		*mname = "???";
	const char		*oname = "???";
	char			*pbuf;
	char			 buf[MIL];
	int				 num = 0;
	bool			 found;

	if ( !ch )
	{
		send_log( NULL, LOG_RESET, "list_resets: ch è NULL" );
		return;
	}

	if ( !pArea )
	{
		send_log( NULL, LOG_RESET, "list_resets: pArea è NULL" );
		return;
	}

	room = NULL;
	mob = NULL;
	obj = NULL;
	lastobj = NULL;
	lo_reset = NULL;
	found = FALSE;

	for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
	{
		if ( !is_room_reset(pReset, pRoom, pArea) )
			continue;

		++num;
		sprintf( buf, "%2d) ", num );
		pbuf = buf + strlen(buf);

		switch ( pReset->command )
		{
		  default:
			sprintf( pbuf, "*** BAD RESET: %c %d %d %d %d ***\r\n",
				pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
			break;

		  case 'M':
			mob = get_mob_index( NULL, pReset->arg1 );
			if ( !mob )
				mname = "Mobile: *BAD VNUM*";
			else
				mname = mob->name;

			room = get_room_index( NULL, pReset->arg3 );
			if ( !room )
				rname = "Room: *BAD VNUM*";
			else
				rname = room->name;

			sprintf( pbuf, "%s (%d) -> %s (%d) [%d]",
				mname,
				pReset->arg1,
				rname,
				pReset->arg3,
				pReset->arg2 );

			if ( !room )
				mob = NULL;

			room = get_room_index( NULL, pReset->arg3 - 1 );
			if ( room && HAS_BIT(room->flags, ROOM_MASCOTTESHOP) )
				strcat( buf, " (mascotte)\r\n" );
			else
				strcat( buf, "\r\n" );
			break;

		  case 'G':
		  case 'E':
			if ( !mob )
				mname = "* ERROR: NO MOBILE! *";

			obj = get_obj_proto( NULL, pReset->arg1 );
			if ( !obj )
				oname = "Object: *BAD VNUM*";
			else
				oname = obj->name;

			sprintf( pbuf, "%s (%d) -> %s (%s) [%d]", oname, pReset->arg1, mname,
				(pReset->command == 'G')  ?  "carry"  :  code_name(NULL, pReset->arg3, CODE_WEARLOC),
				pReset->arg2 );

			if ( mob && mob->pShop )
				strcat( buf, " (shop)\r\n" );
			else
				strcat( buf, "\r\n" );

			lastobj = obj;
			lo_reset = pReset;
			break;

		  case 'O':
			obj = get_obj_proto( NULL, pReset->arg1 );
			if ( !obj )
				oname = "Object: *BAD VNUM*";
			else
				oname = obj->name;

			room = get_room_index( NULL, pReset->arg3 );
			if ( !room )
				rname = "Room: *BAD VNUM*";
			else
				rname = room->name;

			sprintf( pbuf, "(object) %s (%d) -> %s (%d) [%d]\r\n",
				oname,
				pReset->arg1,
				rname,
				pReset->arg3,
				pReset->arg2 );

			if ( !room )
				obj = NULL;

			lastobj = obj;
			lo_reset = pReset;
			break;

		  case 'P':
			obj = get_obj_proto( NULL, pReset->arg1 );
			if ( !obj )
				oname = "Object1: *BAD VNUM*";
			else
				oname = obj->name;

			obj2 = NULL;
			if ( pReset->arg3 > 0 )
			{
				obj2 = get_obj_proto( NULL, pReset->arg3 );
				rname = (obj2 ? obj2->name : "Object2: *BAD VNUM*");
				lastobj = obj2;
			}
			else if ( !lastobj )
			{
				rname = "Object2: *NULL obj*";
			}
			else if ( pReset->extra == 0 )
			{
				rname = lastobj->name;
				obj2 = lastobj;
			}
			else
			{
				RESET_DATA *reset;
				int			iNest;

				reset = lo_reset->next;
				for ( iNest = 0;  iNest < pReset->extra;  iNest++ )
				{
					for ( ;  reset;  reset = reset->next )
					{
						if ( reset->command == 'O' )	break;
						if ( reset->command == 'G' )	break;
						if ( reset->command == 'E' )	break;

						if ( reset->command == 'P' && !reset->arg3 && reset->extra == iNest )
							break;
					}

					if ( !reset || reset->command != 'P' )
						break;
				}

				if ( !reset )
				{
					rname = "Object2: *BAD NESTING*";
				}
				else
				{
					obj2 = get_obj_proto( NULL, reset->arg1 );
					if ( !obj2 )
						rname = "Object2: *NESTED BAD VNUM*";
					else
						rname = obj2->name;
				}
			}
			sprintf( pbuf, "(Put) %s (%d) -> %s (%d) [%d] {nest %d}\r\n",
				oname,
				pReset->arg1,
				rname,
				(obj2)  ?  obj2->vnum  :  pReset->arg3,
				pReset->arg2,
				pReset->extra );
			break;

		  case 'T':
			sprintf( pbuf, "TRAP: %d %d %d %d (%s)\r\n",
				pReset->extra,
				pReset->arg1,
				pReset->arg2,
				pReset->arg3,
				code_bitint(NULL, pReset->extra, CODE_TRAPFLAG) );
		  break;

		  case 'H':
			if ( pReset->arg1 > 0 )
			{
				obj2 = get_obj_proto( NULL, pReset->arg1 );
				if ( !obj2 )
					rname = "Object: *BAD VNUM*";
				else
					rname = obj2->name;
			}
			else if ( !obj )
				rname = "Object: *NULL obj*";
			else
				rname = oname;

			sprintf( pbuf, "Hide %s (%d)\r\n",
				rname,
				(pReset->arg1 > 0)  ?  pReset->arg1  :  (obj) ? obj->vnum : 0 );
			break;

		  case 'B':
		  {
			int	codearray = -1;

			strcpy( pbuf, "BIT: " );
			pbuf += 5;
			if ( HAS_BITINT(pReset->arg2, BIT_RESET_SET) )
			{
				strcpy( pbuf, "Set: " );
				pbuf += 5;
			}
			else if ( HAS_BITINT(pReset->arg2, BIT_RESET_TOGGLE) )
			{
				strcpy( pbuf, "Toggle: " );
				pbuf += 8;
			}
			else
			{
				strcpy( pbuf, "Remove: " );
				pbuf += 8;
			}

			switch ( pReset->arg2 & BIT_RESET_TYPE_MASK )
			{
			  default:
				sprintf( pbuf, "bad type %d", pReset->arg2 & BIT_RESET_TYPE_MASK );
				codearray = -1;
				break;

			  case BIT_RESET_DOOR:
			  {
				int door;

				room = get_room_index( NULL, pReset->arg1 );
				if ( !room )
					rname = "Room: *BAD VNUM*";
				else
					rname = room->name;

				door = (pReset->arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;
				door = URANGE( 0, door, MAX_DIR-1 );
				sprintf( pbuf, "Exit %s%s (%d), Room %s (%d)",
					code_name(NULL, door, CODE_DIR),
					(room && get_exit(room, door))  ?  ""  :  " (NO EXIT!)",
					door,
					rname,
					pReset->arg1 );
			  }
			  codearray = CODE_EXIT;
			  break;

			  case BIT_RESET_ROOM:
				room = get_room_index( NULL, pReset->arg1 );
				if ( !room )
					rname = "Room: *BAD VNUM*";
				else
					rname = room->name;
				sprintf( pbuf, "Room %s (%d)", rname, pReset->arg1 );
				codearray = CODE_ROOM;
				break;

			  case BIT_RESET_OBJECT:
				if ( pReset->arg1 > 0 )
				{
					obj2 = get_obj_proto( NULL, pReset->arg1 );
					if ( !obj2 )
						rname = "Object: *BAD VNUM*";
					else
						rname = obj2->name;
				}
				else if ( !obj )
					rname = "Object: *NULL obj*";
				else
					rname = oname;

				sprintf( pbuf, "Object %s (%d)",
					rname,
					(pReset->arg1 > 0)  ?  pReset->arg1  :  (obj) ? obj->vnum : 0 );
				codearray = CODE_OBJFLAG;
				break;

			  case BIT_RESET_MOBILE:
				if ( pReset->arg1 > 0 )
				{
					MOB_PROTO_DATA *mob2;

					mob2 = get_mob_index( NULL, pReset->arg1 );
					if ( !mob2 )
						rname = "Mobile: *BAD VNUM*";
					else
						rname = mob2->name;
				}
				else if ( !mob )
					rname = "Mobile: *NULL mob*";
				else
					rname = mname;

				sprintf( pbuf, "Mobile %s (%d)",
					rname,
					(pReset->arg1 > 0)  ?  pReset->arg1  :  (mob) ? mob->vnum : 0 );

				codearray = CODE_AFFECT;
				break;
			} /* chiude lo switch */

			pbuf += strlen(pbuf);
			if ( codearray != -1 )
			{
					sprintf( pbuf, "; flags: %s [%d]\r\n",
						code_bitint(NULL, pReset->arg3, codearray),
						pReset->arg3 );
			}
			else
				sprintf( pbuf, "; flags %d\r\n", pReset->arg3 );
		  } /* chiude il case */
		  break;

		  case 'D':
		  {
			char *ef_name;

			pReset->arg2 = URANGE( 0, pReset->arg2, MAX_DIR-1 );

			room = get_room_index( NULL, pReset->arg1 );
			if ( !room )
				rname = "Room: *BAD VNUM*";
			else
				rname = room->name;

			switch ( pReset->arg3 )
			{
			  default:	ef_name = "(* ERROR *)";		break;
			  case 0:	ef_name = "Open";				break;
			  case 1:	ef_name = "Close";				break;
			  case 2:	ef_name = "Close and lock";		break;
			}

			sprintf(pbuf, "%s [%d] the %s%s [%d] door %s (%d)\r\n", ef_name,
				pReset->arg3,
				code_name(NULL, pReset->arg2, CODE_DIR),
				(room && get_exit(room, pReset->arg2)  ?  ""  :  " (NO EXIT!)"),
				pReset->arg2, rname, pReset->arg1);
		  } /* chiude il case */
		  break;

		  case 'R':
			room = get_room_index( NULL, pReset->arg1 );
			if ( !room )
				rname = "Room: *BAD VNUM*";
			else
				rname = room->name;

			sprintf( pbuf, "Randomize exits 0 to %d -> %s (%d)\r\n",
				pReset->arg2,
				rname,
				pReset->arg1 );
			break;
		} /* chiue lo switch */

		if ( start == -1 || num >= start )
			send_to_char( ch, buf );

		if ( end != -1 && num >= end )
			break;
	} /* chiude il for */

	if ( num == 0 )
		send_to_char( ch, "Non c'è nessun reset definito.\r\n" );
}


DO_RET do_rreset( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA *pRoom;

	if ( ch->substate == SUB_REPEAT_CMD )
	{
		pRoom = ch->dest_buf;
		if ( !pRoom )
		{
			send_to_char( ch, "Your room pointer got lost.  Reset mode off.\r\n" );
			send_log( NULL, LOG_RESET, "do_rreset: %s's dest_buf points to invalid room", ch->name );
		}
		ch->substate = SUB_NONE;
		ch->dest_buf = NULL;
		return;
	}
	else
	{
		pRoom = ch->in_room;
	}

	if ( !can_modify_room(ch, pRoom) )
		return;

	edit_reset( ch, argument, pRoom->area, pRoom );
}


DO_RET do_reset( CHAR_DATA *ch, char *argument )
{
	AREA_DATA *pArea = NULL;
	char	  *parg;
	char	   arg[MIL];

	/*
	 * Can't have NPC's doing this.
	 */
	if ( IS_MOB(ch) )
	{
		send_to_char( ch, "I mob non possono farlo.\r\n" );
		return;
	}

	parg = one_argument( argument, arg );
	if ( ch->substate == SUB_REPEAT_CMD )
	{
		pArea = ch->dest_buf;
		if ( pArea && pArea != ch->pg->build_area && pArea != ch->in_room->area )
		{
			AREA_DATA *tmp;

			for ( tmp = first_build;  tmp;  tmp = tmp->next )
			{
				if ( tmp == pArea )
					break;
			}

			if ( !tmp )
			{
				for ( tmp = first_area;  tmp;  tmp = tmp->next )
				{
					if ( tmp == pArea )
						break;
				}
			}

			if ( !tmp )
			{
				send_to_char( ch, "Your area pointer got lost.  Reset mode off.\r\n" );
				send_log( NULL, LOG_RESET, "do_reset: %s's dest_buf points to invalid area", ch->name);		/* why was this cast to an int? */
				ch->substate = SUB_NONE;
				ch->dest_buf = NULL;
				return;
			}
		}

		if ( !VALID_STR(arg) )
		{
			ch_printf( ch, "Editing resets for area: %s\r\n", pArea->name );
			return;
		}

		if ( is_name(arg, "fatto fine done off") )
		{
			send_to_char( ch, "Reset mode off.\r\n" );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			return;
		}
	}

	if ( !pArea && get_trust(ch) >= TRUST_BUILDER )
	{
		char fname[80];

		sprintf( fname, "%s.are", capitalize(arg) );
		for ( pArea = first_build;  pArea;  pArea = pArea->next )
		{
			if ( !str_cmp(fname, pArea->filename) )
			{
				argument = parg;
				break;
			}
		}

		if ( !pArea )
			pArea = ch->pg->build_area;

		if ( !pArea )
			pArea = ch->in_room->area;
	}
	else
	{
		pArea = ch->pg->build_area;
	}

	if ( !pArea )
	{
		send_to_char( ch, "You do not have an assigned area.\n\r" );
		return;
	}

	edit_reset( ch, argument, pArea, NULL );
}


/*
 * Create a new reset (for online building)
 */
RESET_DATA *make_reset( char letter, int extra, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;

	CREATE( pReset, RESET_DATA, 1 );

	pReset->command	= letter;
	pReset->extra	= extra;
	pReset->arg1	= arg1;
	pReset->arg2	= arg2;
	pReset->arg3	= arg3;

	top_reset++;

	return pReset;
}


/*
 * Place a reset into an area, insert sorting it
 */
RESET_DATA *place_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;
	RESET_DATA *tmp;
	RESET_DATA *tmp2;

	if ( !tarea )
	{
		send_log( NULL, LOG_RESET, "place_reset: tarea è NULL" );
		return NULL;
	}

	letter = toupper( letter );
	pReset = make_reset( letter, extra, arg1, arg2, arg3 );

	if ( letter == 'M' )
		tarea->last_mob_reset = pReset;

	if ( tarea->first_reset )
	{
		switch ( letter )
		{
		  default:
			send_log( NULL, LOG_RESET, "place_reset: Bad reset type %c", letter );
			return NULL;

		  case 'D':
		  case 'R':
			for ( tmp = tarea->last_reset;  tmp;  tmp = tmp->prev )
			{
				if ( tmp->command == letter )
					break;
			}

			if ( tmp )	/* organize by location */
			{
				for ( ;  tmp && tmp->command == letter && tmp->arg1 > arg1;  tmp = tmp->prev )
					;
			}

			if ( tmp )	/* organize by direction */
			{
				for ( ;  tmp && tmp->command == letter && tmp->arg1 == tmp->arg1 && tmp->arg2 > arg2;  tmp = tmp->prev )
					;
			}

			if ( tmp )
				INSERT( pReset, tmp, tarea->first_reset, next, prev );
			else
				LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );

			return pReset;

		  case 'M':
		  case 'O':
			/* find last reset of same type */
			for ( tmp = tarea->last_reset;  tmp;  tmp = tmp->prev )
			{
				if ( tmp->command == letter )
					break;
			}

			if ( tmp )
				tmp2 = tmp->next;
			else
				tmp2 = NULL;

			/* organize by location */
			for ( ;  tmp;  tmp = tmp->prev )
			{
				if ( tmp->command != letter )	continue;
				if ( tmp->arg3 > arg3 )			continue;

				tmp2 = tmp->next;

				/* organize by vnum */
				if ( tmp->arg3 != arg3 )
					continue;

				for ( ;  tmp;  tmp = tmp->prev )
				{
					if ( tmp->command != letter )	continue;
					if ( tmp->arg3 != tmp->arg3 )	continue;

					if ( tmp->arg1 <= arg1 )
						tmp2 = tmp->next;

					break;
				}
			}

			/* skip over E or G for that mob */
			if ( tmp2 && letter == 'M' )
			{
				for ( ;  tmp2;  tmp2 = tmp2->next )
				{
					if ( tmp2->command != 'E' && tmp2->command != 'G' )
						break;
				}
			}
			else if ( tmp2 && letter == 'O' )		/* skip over P, T or H for that obj */
			{
				for ( ;  tmp2;  tmp2 = tmp2->next )
				{
					if ( tmp2->command != 'P'
					  && tmp2->command != 'T'
					  && tmp2->command != 'H' )
					{
						break;
					}
				}
			}

			if ( tmp2 )
				INSERT( pReset, tmp2, tarea->first_reset, next, prev );
			else
				LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );

			return pReset;

		  case 'G':
		  case 'E':
			/* find the last mob */
			tmp = tarea->last_mob_reset;
			if ( tmp )
			{
				/*
				 * See if there are any resets for this mob yet,
				 * put E before G and organize by vnum
				 */
				if ( tmp->next )
				{
					tmp = tmp->next;
					if ( tmp && tmp->command == 'E' )
					{
						if ( letter == 'E' )
						{
							for ( ;  tmp && tmp->command == 'E' && tmp->arg1 < arg1;  tmp = tmp->next )
								;
						}
						else
						{
							for ( ;  tmp && tmp->command == 'E';  tmp = tmp->next )
								;
						}
					}
					else if ( tmp && tmp->command == 'G' && letter == 'G' )
					{
						for ( ;  tmp && tmp->command == 'G' && tmp->arg1 < arg1;  tmp = tmp->next )
							;
					}

					if ( tmp )
						INSERT( pReset, tmp, tarea->first_reset, next, prev );
					else
						LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
				}
				else
				{
					LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
				}

				return pReset;
			} /* chiude l'if */
			break;

		  case 'P':
		  case 'T':
		  case 'H':
			/* find the object in question */
			tmp = tarea->last_obj_reset;
			if ( tmp
			  && ((letter == 'P' && arg3 == 0)
			  ||  (letter == 'T' && arg1 == 0 && HAS_BITINT(extra, TRAPFLAG_OBJ) )
			  ||  (letter == 'H' && arg1 == 0)) )
			{
				tmp = tmp->next;
				if ( tmp )
					INSERT( pReset, tmp, tarea->first_reset, next, prev );
				else
					LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );

				return pReset;
			}

			for ( tmp = tarea->last_reset;  tmp;  tmp = tmp->prev )
			{
				if ( tmp->arg1 == arg3
				  && (tmp->command == 'O'
				  ||  tmp->command == 'G'
				  ||  tmp->command == 'E'
				  ||  tmp->command == 'P') )
				{
					/*
					 * See if there are any resets for this object yet,
					 * put P before H before T and organize by vnum
					 */
					if ( tmp->next )
					{
						tmp = tmp->next;
						if ( tmp && tmp->command == 'P' )
						{
							if ( letter == 'P' && tmp->arg3 == arg3 )
							{
								for ( ;  tmp && tmp->command == 'P' && tmp->arg3 == arg3 && tmp->arg1 < arg1;  tmp = tmp->next )
									;
							}
							else if ( letter != 'T' )
							{
								for ( ;  tmp && tmp->command == 'P' && tmp->arg3 == arg3;  tmp = tmp->next )
									;
							}
						}
						else if ( tmp && tmp->command == 'H' )
						{
							if ( letter == 'H' && tmp->arg3 == arg3 )
							{
								for ( ;  tmp && tmp->command == 'H' && tmp->arg3 == arg3 && tmp->arg1 < arg1;  tmp = tmp->next )
									;
							}
							else if ( letter != 'H' )
							{
								for ( ; tmp && tmp->command == 'H' && tmp->arg3 == arg3; tmp = tmp->next )
									;
							}
						}
						else if ( tmp && tmp->command == 'T' && letter == 'T' )
						{
							for ( ;  tmp && tmp->command == 'T' && tmp->arg3 == arg3 && tmp->arg1 < arg1;  tmp = tmp->next )
								;
						}

						if ( tmp )
							INSERT( pReset, tmp, tarea->first_reset, next, prev );
						else
							LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );

					} /* chiude l'if */
					else
					{
						LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
					}

					return pReset;
				} /* chiude l'if */
			} /* chiude il for */
			break;
		} /* chiude lo switch */

		/* likely a bad reset if we get here... add it anyways */

	} /* chiude l'if */

	LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
	return pReset;
}


void edit_reset( CHAR_DATA *ch, char *argument, AREA_DATA *pArea, ROOM_DATA *aRoom )
{
	RESET_DATA		*pReset = NULL;
	RESET_DATA		*reset	= NULL;
	MOB_PROTO_DATA	*pMob	= NULL;
	ROOM_DATA		*pRoom;
	OBJ_PROTO_DATA  *pObj;
	char			*origarg = argument;
	char			 arg[MIL];
	int				 num = 0;
	VNUM			 vnum;

	if ( !VALID_STR(argument) || !str_cmp(argument, "?") )
	{
		char *nm = (ch->substate == SUB_REPEAT_CMD)  ?  ""  :  (aRoom)  ?  "rreset "  :  "reset ";
		char *rn = ( aRoom  ?  ""  :  " [room#]" );

		ch_printf( ch, "&YSyntax&w: %s<list|edit|delete|add|insert|place%s>\r\n",					nm, (aRoom)  ?  ""  :  "|area" );
		ch_printf( ch, "      : %sremove <#>\r\n",													nm );
		ch_printf( ch, "      : %smobile <mob#> [limit]%s\r\n",										nm, rn );
		ch_printf( ch, "      : %sobject <obj#> [limit [room%s]]\r\n",								nm, rn );
		ch_printf( ch, "      : %sobject <obj#> give <mob name> [limit]\r\n",						nm );
		ch_printf( ch, "      : %sobject <obj#> equip <mob name> <location> [limit]\r\n",			nm );
		ch_printf( ch, "      : %sobject <obj#> put <to_obj name> [limit]\r\n",						nm );
		ch_printf( ch, "      : %shide <obj name>\r\n",												nm );
		ch_printf( ch, "      : %strap <obj name> <type> <charges> <flags>\r\n",					nm );
		ch_printf( ch, "      : %strap room <type> <charges> <flags>\r\n",							nm );
		ch_printf( ch, "      : %sbit <set|toggle|remove> door%s <dir> <exit flags>\r\n",			nm, rn );
		ch_printf( ch, "      : %sbit <set|toggle|remove> object <obj name> <extra flags>\r\n",		nm );
		ch_printf( ch, "      : %sbit <set|toggle|remove> mobile <mob name> <affect flags>\r\n",	nm );
		ch_printf( ch, "      : %sbit <set|toggle|remove> room%s <room flags>\r\n",					nm, rn );
		ch_printf( ch, "      : %srandom <last dir>%s\r\n",											nm, rn );

		if ( !aRoom )
			send_to_char( ch, "\r\n[room#] will default to the room you are in, if unspecified.\r\n" );

		return;
	}

	argument = one_argument( argument, arg );
	if ( is_name(arg, "si on") )
	{
		ch->substate = SUB_REPEAT_CMD;
		ch->dest_buf = ( aRoom  ?  (void *)aRoom  :  (void *)pArea );
		send_to_char( ch, "Reset mode on.\r\n" );
		return;
	}

	if ( !aRoom && !str_cmp(arg, "area") )
	{
		if ( !pArea->first_reset )
		{
			send_to_char( ch, "You don't have any resets defined.\r\n" );
			return;
		}

		num = pArea->nplayer;
		pArea->nplayer = 0;
		reset_area( pArea );
		pArea->nplayer = num;

		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( is_name(arg, "lista list") )
	{
		int	start;
		int	end;

		argument = one_argument( argument, arg );
		start = is_number( arg )  ?  atoi( arg )  :  -1;

		argument = one_argument( argument, arg );
		end   = is_number( arg )  ?  atoi( arg )  :  -1;

		list_resets( ch, pArea, aRoom, start, end );
		return;
	}

	if ( is_name(arg, "modifica edita edit") )
	{
		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) || !is_number(arg) )
		{
			send_to_char( ch, "Usage: reset edita <number> <command>\r\n" );
			return;
		}

		num = atoi( arg );

		pReset = find_reset( pArea, aRoom, num );
		if ( !pReset )
		{
			send_to_char( ch, "Reset not found.\r\n" );
			return;
		}

		reset = parse_reset( argument, ch );
		if ( !reset )
		{
			send_to_char( ch, "Error in reset.  Reset not changed.\r\n" );
			return;
		}

		reset->prev = pReset->prev;
		reset->next = pReset->next;

		if ( !pReset->prev )
			pArea->first_reset = reset;
		else
			pReset->prev->next = reset;

		if ( !pReset->next )
			pArea->last_reset  = reset;
		else
			pReset->next->prev = reset;

		DISPOSE( pReset );
		send_to_char( ch, "Fatto.\r\n" );
		return;
	} /* chiude l'if */

	if ( is_name(arg, "aggiungi add") )
	{
		pReset = parse_reset( argument, ch );
		if ( !pReset )
		{
			send_to_char( ch, "Error in reset.  Reset not added.\r\n" );
			return;
		}

		add_reset( pArea, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
		DISPOSE( pReset );

		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( is_name(arg, "metti piazza place") )
	{
		pReset = parse_reset( argument, ch );
		if ( !pReset )
		{
			send_to_char( ch, "Error in reset.  Reset not added.\r\n" );
			return;
		}

		place_reset( pArea, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
		DISPOSE( pReset );

		send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( is_name(arg, "inserisci insert") )
	{
		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) || !is_number(arg) )
		{
			send_to_char( ch, "Usage: reset insert <number> <command>\r\n" );
			return;
		}

		num = atoi(arg);

		reset = find_reset( pArea, aRoom, num );
		if ( !reset )
		{
			send_to_char( ch, "Reset not found.\r\n" );
			return;
		}

		pReset = parse_reset( argument, ch );
		if ( !pReset )
		{
			send_to_char( ch, "Error in reset.  Reset not inserted.\r\n" );
			return;
		}

		INSERT(pReset, reset, pArea->first_reset, next, prev);
		send_to_char( ch, "Fatto.\r\n" );

		return;
	}

	if ( is_name(arg, "cancella delete") )
	{
		int		start;
		int		end;
		bool	found;

		if ( !VALID_STR(argument) )
		{
			send_to_char( ch, "Usage: reset delete <start> [end]\r\n" );
			return;
		}
		argument = one_argument( argument, arg );
		start = is_number( arg )  ?  atoi( arg )  :  -1;
		end   = is_number( arg )  ?  atoi( arg )  :  -1;
		num	  = 0;
		found = FALSE;

		for ( pReset = pArea->first_reset;  pReset;  pReset = reset )
		{
			reset = pReset->next;

			if ( !is_room_reset(pReset, aRoom, pArea) )
				continue;
			if ( start > ++num )
				continue;
			if ( (end != -1 && num > end) || (end == -1 && found) )
				return;

			UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
			if ( pReset == pArea->last_mob_reset )
				pArea->last_mob_reset = NULL;
			DISPOSE( pReset );
			top_reset--;
			found = TRUE;
		}

		if ( !found )
			send_to_char( ch, "Reset not found.\r\n" );
		else
			send_to_char( ch, "Fatto.\r\n" );
		return;
	}

	if ( is_name(arg, "rimuovi remove") )
	{
		int iarg;

		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) || !is_number(arg) )
		{
			send_to_char( ch, "Delete which reset?\r\n" );
			return;
		}

		iarg = atoi( arg );
		for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
		{
			if ( is_room_reset(pReset, aRoom, pArea) && ++num == iarg )
				break;
		}

		if ( !pReset )
		{
			send_to_char( ch, "Reset does not exist.\r\n" );
			return;
		}

		delete_reset( pArea, pReset );
		send_to_char( ch, "Reset deleted.\r\n" );

		return;
	}

	if ( is_name(arg, "mob mobile") )
	{
		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) || !is_number(arg) )
		{
			send_to_char( ch, "Reset which mobile vnum?\r\n" );
			return;
		}

		pMob = get_mob_index( NULL, atoi(arg) );
		if ( !pMob )
		{
			send_to_char( ch, "Mobile does not exist.\r\n" );
			return;
		}

		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) )
		{
			num = 1;
		}
		else if ( !is_number(arg) )
		{
			send_to_char( ch, "Reset how many mobiles?\r\n" );
			return;
		}
		else
		{
			num = atoi( arg );
		}

		pRoom = find_room( ch, argument, aRoom );
		if ( !pRoom )
			return;

		pReset = make_reset( 'M', 0, pMob->vnum, num, pRoom->vnum );
		LINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );

		send_to_char( ch, "Mobile reset added.\r\n" );
		return;
	}

	if ( is_name(arg, "obj object") )
	{
		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) || !is_number(arg) )
		{
			send_to_char( ch, "Reset which object vnum?\r\n" );
			return;
		}

		pObj = get_obj_proto( NULL, atoi(arg) );
		if ( !pObj )
		{
			send_to_char( ch, "Object does not exist.\r\n" );
			return;
		}

		argument = one_argument( argument, arg );
		if ( !VALID_STR(arg) )
			strcpy( arg, "room" );

		if ( is_name(arg, "metti put") )
		{
			argument = one_argument( argument, arg );
			reset = find_oreset( ch, pArea, aRoom, arg );
			if ( !reset )
				return;

			pReset = reset;
			/* Put in_objects after hide and trap resets */
			while ( reset->next
			  && (reset->next->command == 'H'
			  ||  reset->next->command == 'T'
			  || (reset->next->command == 'B'
			  && (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT
			  && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))) )
			{
				reset = reset->next;
			}

			argument = one_argument( argument, arg );
			num = atoi( arg );
			if ( num < 1 )
				num = 1;

			argument = one_argument( argument, arg );
			vnum = atoi( arg );
			if ( vnum < 1 )
				vnum = 1;

			pReset = make_reset( 'P', reset->extra+1, pObj->vnum, num, vnum );
			/* Grumble.. insert puts pReset before reset, and we need it after,
			 so we make a hackup and reverse all the list params.. :P.. */
			INSERT( pReset, reset, pArea->last_reset, prev, next );

			send_to_char( ch, "Object reset in object created.\r\n" );
			return;
		} /* chiude l'if */

		if ( is_name(arg, "dai give") )
		{
			argument = one_argument( argument, arg );
			reset = find_mreset( ch, pArea, aRoom, arg );
			if ( !reset )
				return;
			pReset = reset;
			while ( reset->next && reset->next->command == 'B'
			  && (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT
			  && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
			{
				reset = reset->next;
			}

			argument = one_argument( argument, arg );
			vnum = atoi( arg );
			if ( vnum < 1 )
				vnum = 1;

			pReset = make_reset('G', 1, pObj->vnum, vnum, 0);
			INSERT(pReset, reset, pArea->last_reset, prev, next);

			send_to_char( ch, "Object reset to mobile created.\r\n" );
			return;
		}

		if ( is_name(arg, "euipaggia equip") )
		{
			argument = one_argument( argument, arg );
			reset = find_mreset( ch, pArea, aRoom, arg );
			if ( !reset )
				return;

			pReset = reset;
			while ( reset->next && reset->next->command == 'B'
			  && (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT
			  && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1) )
			{
				reset = reset->next;
			}

			num = code_num( NULL, argument, CODE_WEARLOC );
			if ( num < 0 )
			{
				send_to_char( ch, "Reset object to which location?\r\n" );
				return;
			}

			for ( pReset = reset->next;  pReset;  pReset = pReset->next )
			{
				if ( pReset->command == 'M' )
					break;
				if ( pReset->command == 'E' && pReset->arg3 == num )
				{
					send_to_char( ch, "Mobile already has an item equipped there.\r\n" );
					return;
				}
			}

			argument = one_argument( argument, arg );
			vnum = atoi( arg );
			if ( vnum < 1 )
				vnum = 1;

			pReset = make_reset( 'E', 1, pObj->vnum, vnum, num );
			INSERT( pReset, reset, pArea->last_reset, prev, next );

			send_to_char( ch, "Object reset equipped by mobile created.\r\n" );
			return;
		} /* chiude l'if */

		if ( !VALID_STR(arg) || !str_cmp(arg, "room") || is_number(arg) )
		{
			if ( !str_cmp(arg, "room") )
				argument = one_argument( argument, arg );

			pRoom = find_room( ch, argument, aRoom );
			if ( !pRoom )
				return;

			if ( pRoom->area != pArea )
			{
				send_to_char( ch, "Cannot reset objects to other areas.\r\n" );
				return;
			}

			vnum = atoi( arg );
			if ( vnum < 1 )
				vnum = 1;

			pReset = make_reset( 'O', 0, pObj->vnum, vnum, pRoom->vnum );
			LINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );

			send_to_char( ch, "Object reset added.\r\n" );
			return;
		}
		send_to_char( ch, "Reset object to where?\r\n" );
		return;
	}

	if ( is_name(arg, "casuale random") )
	{
		argument = one_argument( argument, arg );
		vnum = get_dir( arg );
		if ( vnum < 0 || vnum >= MAX_DIR )
		{
			send_to_char( ch, "Reset which random doors?\r\n" );
			return;
		}

		if ( vnum == 0 )
		{
			send_to_char( ch, "There is no point in randomizing one door.\r\n" );
			return;
		}

		pRoom = find_room( ch, argument, aRoom );
		if ( pRoom->area != pArea )
		{
			send_to_char( ch, "Cannot randomize doors in other areas.\r\n" );
			return;
		}

		pReset = make_reset('R', 0, pRoom->vnum, vnum, 0);
		LINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );

		send_to_char( ch, "Reset random doors created.\r\n" );
		return;
	}

	if ( is_name(arg, "trappola trap") )
	{
		char	oname[MIL];
		int		chrg;
		int		value;
		int		extra = 0;
		bool	isobj;

		argument = one_argument( argument, oname );
		argument = one_argument( argument, arg );
		num		 = is_number( arg )  ?  atoi( arg )  :  -1;
		argument = one_argument( argument, arg );
		chrg	 = is_number( arg )  ?  atoi( arg )  :  -1;
		isobj	 = is_name( argument, "obj" );

		if ( isobj == is_name(argument, "stanza room") )
		{
			send_to_char( ch, "Reset: TRAP: Must specify ROOM or OBJECT\r\n" );
			return;
		}

		if ( is_name(oname, "stanza room") && !isobj )
		{
			vnum = (aRoom) ? aRoom->vnum : ch->in_room->vnum;
		}
		else
		{
			if ( is_number(oname) && !isobj )
			{
				vnum = atoi( oname );
				if ( !get_room_index(NULL, vnum) )
				{
					send_to_char( ch, "Reset: TRAP: no such room\r\n" );
					return;
				}
				reset = NULL;
			}
			else
			{
				reset = find_oreset( ch, pArea, aRoom, oname );
				if ( !reset )
					return;
				vnum = 0;
			}
		} /* chiude l'else */

		if ( num < 1 || num >= MAX_TRAPTYPE )
		{
			send_to_char( ch, "Reset: TRAP: invalid trap type\r\n" );
			return;
		}

		if ( chrg < 0 || chrg > 10000 )
		{
			send_to_char( ch, "Reset: TRAP: invalid trap charges\r\n" );
			return;
		}

		while ( VALID_STR(argument) )
		{
			argument = one_argument( argument, arg );
			value = code_num( NULL, arg, CODE_TRAPFLAG );
			if ( value < 0 || value >= MAX_TRAPFLAG )
			{
				send_to_char( ch, "Reset: TRAP: bad flag\r\n" );
				return;
			}
			SET_BITINT( extra, value );
		}

		pReset = make_reset( 'T', extra, num, chrg, vnum );
		if ( reset )
			INSERT(pReset, reset, pArea->last_reset, prev, next);
		else
			LINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );

		send_to_char( ch, "Trap created.\r\n" );
		return;
	}

	if ( !str_cmp(arg, "bit") )
	{
		char   *parg;
		char	option[MIL];
		int		temp_parse;
		int		flags = 0;

		argument = one_argument( argument, option );
		if ( !VALID_STR(option) )
		{
			send_to_char( ch, "You must specify SET, REMOVE, or TOGGLE.\r\n" );
			return;
		}

		num = 0;
		if		(  is_name_prefix(option, "imposta setta") )
			SET_BITINT( num, BIT_RESET_SET );
		else if (  is_name_prefix(option, "inverti toggle") )
			SET_BITINT( num, BIT_RESET_TOGGLE );
		else if ( !is_name_prefix(option, "rimuovi remove") )
		{
			send_to_char( ch, "You must specify SET, REMOVE, or TOGGLE.\r\n" );
			return;
		}

		argument = one_argument( argument, option );
		parg = argument;
		argument = one_argument( argument, arg );

		if ( !VALID_STR(option) )
		{
			send_to_char( ch, "Must specify OBJECT, MOBILE, ROOM, or DOOR.\r\n" );
			return;
		}

		if ( is_name_prefix(option, "porta door") )
		{
			SET_BITINT( num, BIT_RESET_DOOR );
			if ( aRoom )
			{
				pRoom = aRoom;
				argument = parg;
			}
			else if ( !is_number(arg) )
			{
				pRoom = ch->in_room;
				argument = parg;
			}
			else if ( !(pRoom = find_room(ch, arg, aRoom)) )
				return;

			argument = one_argument( argument, arg );
			if ( !VALID_STR(arg) )
			{
				send_to_char( ch, "Must specify direction.\r\n" );
				return;
			}
			vnum = get_dir( arg );
			num |= vnum << BIT_RESET_DOOR_THRESHOLD;
			vnum = pRoom->vnum;
			temp_parse = CODE_EXIT;
			reset = NULL;
		}
		else if ( is_name_prefix(option, "oggetto object") )
		{
			SET_BITINT( num, BIT_RESET_OBJECT );
			vnum = 0;
			temp_parse = CODE_OBJFLAG;
			reset = find_oreset( ch, pArea, aRoom, arg );
			if ( !reset )
				return;
		}
		else if ( is_name_prefix(option, "mobile") )
		{
			SET_BITINT( num, BIT_RESET_MOBILE );
			vnum = 0;
			temp_parse = CODE_AFFECT;
			reset = find_mreset( ch, pArea, aRoom, arg );
			if ( !reset )
				return;
		}
		else if ( is_name_prefix(option, "stanza room") )
		{
			SET_BITINT( num, BIT_RESET_ROOM );
			if ( aRoom )
			{
				pRoom = aRoom;
				argument = parg;
			}
			else if ( !is_number(arg) )
			{
				pRoom = ch->in_room;
				argument = parg;
			}
			else if ( !(pRoom = find_room(ch, arg, aRoom)) )
			{
				return;
			}

			vnum = pRoom->vnum;
			temp_parse = CODE_ROOM;
			reset = NULL;
		}
		else
		{
			send_to_char( ch, "Must specify OBJECT, MOBILE, ROOM, or DOOR.\r\n" );
			return;
		}

		while ( VALID_STR(argument) )
		{
			int		value;

			argument = one_argument( argument, arg );
			value = code_num( NULL, arg, temp_parse );
			if ( value < 0 || value >= (sizeof(int)*8) )
			{
				send_to_char( ch, "Reset: BIT: bad flag\r\n" );
				return;
			}

			SET_BITINT( flags, value );
		}

		if ( !flags )
		{
			send_to_char( ch, "Set which flags?\r\n" );
			return;
		}

		pReset = make_reset( 'B', 1, vnum, num, flags );
		if ( reset )
			INSERT( pReset, reset, pArea->last_reset, prev, next );
		else
			LINK( pReset, pArea->first_reset, pArea->last_reset, next, prev );

		send_to_char( ch, "Bitvector reset created.\r\n" );
		return;
	}

	if ( is_name(arg, "nascondi hide") )
	{
		argument = one_argument( argument, arg );

		reset = find_oreset( ch, pArea, aRoom, arg );
		if ( !reset )
			return;

		pReset = make_reset('H', 1, 0, 0, 0);
		INSERT( pReset, reset, pArea->last_reset, prev, next );

		send_to_char( ch, "Object hide reset created.\r\n" );
		return;
	}

	if ( ch->substate == SUB_REPEAT_CMD )
	{
		ch->substate = SUB_NONE;
		/* Si suppone che stia stato inviato argument nella sua lingua quindi niente send_command() */
		interpret( ch, origarg );
		ch->substate = SUB_REPEAT_CMD;
		ch->last_cmd = ( aRoom  ?  do_rreset  :  do_reset );
	}
	else
	{
		edit_reset( ch, "?", pArea, aRoom );
	}
}


void add_obj_reset( AREA_DATA *pArea, char cm, OBJ_DATA *obj, int v2, int v3 )
{
	OBJ_DATA	*inobj;
	static int	 iNest;

	if ( (cm == 'O' || cm == 'P') && VNUM_OBJ_TRAP == obj->vnum )
	{
		if ( cm == 'O' )
			add_reset( pArea, 'T', obj->trap->flags, obj->trap->type, obj->trap->charges, v3 );

		return;
	}

	add_reset( pArea, cm, (cm == 'P')  ?  iNest  :  1, obj->vnum, v2, v3 );

	/* Only add hide for in-room objects that are hidden and cant be moved, as
	 *	hide is an update reset, not a load-only reset */
	if ( cm == 'O' && HAS_BIT(obj->extra_flags, OBJFLAG_HIDDEN) && !HAS_BIT(obj->wear_flags, OBJWEAR_TAKE) )
		add_reset( pArea, 'H', 1, 0, 0, 0 );

	for ( inobj = obj->first_content;  inobj;  inobj = inobj->next_content )
	{
		if ( inobj->vnum == VNUM_OBJ_TRAP )
			add_obj_reset( pArea, 'O', inobj, 0, 0 );
	}

	if ( cm == 'P' )
		iNest++;

	for ( inobj = obj->first_content;  inobj;  inobj = inobj->next_content )
		add_obj_reset( pArea, 'P', inobj, inobj->count, obj->vnum );

	if ( cm == 'P' )
		iNest--;
}


void instaroom( AREA_DATA *pArea, ROOM_DATA *pRoom, bool dodoors )
{
	CHAR_DATA	*rch;
	OBJ_DATA	*obj;

	for ( rch = pRoom->first_person;  rch;  rch = rch->next_in_room )
	{
		if ( IS_PG(rch) )
			continue;

		add_reset( pArea, 'M', 1, rch->pIndexData->vnum, rch->pIndexData->count, pRoom->vnum );

		for ( obj = rch->first_carrying;  obj;  obj = obj->next_content )
		{
			if ( obj->wear_loc == WEARLOC_NONE )
				add_obj_reset( pArea, 'G', obj, 1, 0 );
			else
				add_obj_reset( pArea, 'E', obj, 1, obj->wear_loc );
		}
	}

	for ( obj = pRoom->first_content;  obj;  obj = obj->next_content )
		add_obj_reset( pArea, 'O', obj, obj->count, pRoom->vnum );

	if ( dodoors )
	{
		EXIT_DATA *pexit;

		for ( pexit = pRoom->first_exit;  pexit;  pexit = pexit->next )
		{
			int state = 0;

			if ( !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
				continue;

			if ( HAS_BIT(pexit->flags, EXIT_CLOSED) )
			{
				if ( HAS_BIT( pexit->flags, EXIT_LOCKED ) )
					state = 2;
				else
					state = 1;
			}

			add_reset( pArea, 'D', 0, pRoom->vnum, pexit->vdir, state );
		}
	}
}


void wipe_resets( AREA_DATA *pArea, ROOM_DATA *pRoom )
{
	RESET_DATA *pReset;

	for ( pReset = pArea->first_reset;  pReset;  )
	{
		if ( pReset->command != 'R' && is_room_reset(pReset, pRoom, pArea) )
		{
			/* Resets always go forward, so we can safely use the previous reset,
			 *	providing it exists, or first_reset if it doesnt */
			RESET_DATA *prev = pReset->prev;

			delete_reset( pArea, pReset );
			pReset = (prev) ? prev->next : pArea->first_reset;
		}
		else
		{
			pReset = pReset->next;
		}
	}
}


DO_RET do_instaroom( CHAR_DATA *ch, char *argument )
{
	AREA_DATA	*pArea;
	ROOM_DATA	*pRoom;
	char		 arg[MIL];
	bool		 dodoors;

	if ( IS_MOB(ch) || !ch->pg || !ch->pg->build_area )
	{
		send_to_char( ch, "You don't have an assigned area to create resets for.\r\n" );
		return;
	}

	argument = one_argument( argument, arg );
	if ( is_name(arg, "noporte nodoors") )
		dodoors = FALSE;
	else
		dodoors = TRUE;

	pArea = ch->pg->build_area;
	pRoom = find_room( ch, arg, NULL );
	if ( !pRoom )
	{
		send_to_char( ch, "Room doesn't exist.\r\n" );
		return;
	}

	if ( !can_modify_room(ch, pRoom) )
		return;

	if ( pRoom->area != pArea && get_trust(ch) < TRUST_BUILDER )
	{
		send_to_char( ch, "You cannot reset that room.\r\n" );
		return;
	}

	if ( pArea->first_reset )
		wipe_resets( pArea, pRoom );

	instaroom( pArea, pRoom, dodoors );
	send_to_char( ch, "Room resets installed.\r\n" );
}


DO_RET do_instazone( CHAR_DATA *ch, char *argument )
{
	ROOM_DATA	*pRoom;
	AREA_DATA	*pArea;
	VNUM		 vnum;
	bool		 dodoors;

	if ( IS_MOB(ch) || !ch->pg || !ch->pg->build_area )
	{
		send_to_char( ch, "You don't have an assigned area to create resets for.\r\n" );
		return;
	}

	if ( is_name(argument, "noporte nodoors") )
		dodoors = FALSE;
	else
		dodoors = TRUE;

	pArea = ch->pg->build_area;
	if ( pArea->first_reset )
		wipe_resets(pArea, NULL);

	for ( vnum = pArea->vnum_low;  vnum <= pArea->vnum_high;  vnum++ )
	{
		pRoom = get_room_index( NULL, vnum );
		if ( !pRoom || pRoom->area != pArea )
			continue;

		instaroom( pArea, pRoom, dodoors );
	}

	send_to_char( ch, "Area resets installed.\r\n" );
}


int generate_itemlevel( AREA_DATA *pArea, OBJ_PROTO_DATA *pObj )
{
	int	olevel;
	int	min = UMAX( pArea->level_low, 1 );
	int	max = UMIN( pArea->level_high, min+15 );

	if ( pObj->level > 0 )
	{
		olevel = UMIN( pObj->level, LVL_LEGEND );
	}
	else
	{
		switch ( pObj->type )
		{
		  default:					olevel = 0;									break;
		  case OBJTYPE_PILL:		olevel = number_range( min,   max );		break;
		  case OBJTYPE_POTION:		olevel = number_range( min,   max );		break;
		  case OBJTYPE_SCROLL:		olevel = pObj->scroll->level;				break;
		  case OBJTYPE_WAND:		olevel = number_range( min+4, max+1 );		break;
		  case OBJTYPE_STAFF:		olevel = number_range( min+9, max+5 );		break;
		  case OBJTYPE_DRESS:
		  case OBJTYPE_ARMOR:		olevel = number_range( min+4, max+1 );		break;
		  case OBJTYPE_WEAPON:		olevel = number_range( min+4, max+1 );		break;
		}
	}

	return olevel;
}


/*
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea )
{
	RESET_DATA		*pReset;
	CHAR_DATA		*mob;
	OBJ_DATA		*obj;
	OBJ_DATA		*lastobj;
	ROOM_DATA		*pRoom;
	MOB_PROTO_DATA	*pMobIndex;
	OBJ_PROTO_DATA	*pObj;
	OBJ_PROTO_DATA	*pObjTo;
	EXIT_DATA		*pexit;
	OBJ_DATA		*to_obj;
	int				 level = 0;
	bool			 ext_bv = FALSE;

	if ( !pArea )
	{
		send_log( NULL, LOG_RESET, "reset_area: pArea è NULL." );
		return;
	}

	mob = NULL;
	obj = NULL;
	lastobj = NULL;

	if ( !pArea->first_reset )
	{
		send_log( NULL, LOG_RESET, "reset_area: nessun reset per l'area %s", pArea->name );
		return;
	}

	level = 0;
	for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
	{
		BITVECTOR *plc = NULL;

		switch ( pReset->command )
		{
		  default:
			send_log( NULL, LOG_RESET, "reset_area: bad command %c.", pReset->command );
			break;

		  case 'M':
			pMobIndex = get_mob_index( NULL, pReset->arg1 );
			if ( !pMobIndex )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'M': bad mob vnum %d.", pReset->arg1 );
				continue;
			}

			pRoom = get_room_index( NULL, pReset->arg3 );
			if ( !pRoom )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'M': bad room vnum %d", pReset->arg3 );
				continue;
			}

			if ( pMobIndex->count >= pReset->arg2 )
			{
				mob = NULL;
				break;
			}

			/** Prevent Day and Night mob resets ** Rythmic */
			if ( HAS_BIT(pMobIndex->act, MOB_DAY) && calendar.sunlight ==  SUN_DARK )
			{
				mob = NULL;
				break;
			}
			if ( HAS_BIT(pMobIndex->act, MOB_NIGHT) && calendar.sunlight == SUN_LIGHT )
			{
				mob = NULL;
				break;
			}

			mob = make_mobile( pMobIndex );

			{
				ROOM_DATA *pRoomPrev = get_room_index( NULL, pReset->arg3 - 1 );

				if ( pRoomPrev && HAS_BIT(pRoomPrev->flags, ROOM_MASCOTTESHOP) )
					SET_BIT( mob->act, MOB_MASCOTTE );
			}

			if ( room_is_dark(pRoom) )
				SET_BIT( mob->affected_by, AFFECT_INFRARED );

			char_to_room( mob, pRoom );
			economize_mobgold( mob );
			level = URANGE( 0, get_level(mob)-2, LVL_LEGEND );

			break;

		  case 'G':
		  case 'E':
			pObj = get_obj_proto( NULL, pReset->arg1 );
			if ( !pObj )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'E' or 'G': bad obj vnum %d.", pReset->arg1 );
				continue;
			}

			if ( !mob )
			{
				lastobj = NULL;
				break;
			}

			if ( mob->pIndexData->pShop )
			{
				int olevel = generate_itemlevel( pArea, pObj );

				obj = make_object( pObj, olevel );
				SET_BIT( obj->extra_flags, OBJFLAG_INVENTORY );
			}
			else
				obj = make_object( pObj, number_fuzzy(level) );

			obj->level = URANGE( 0, obj->level, LVL_LEGEND );
			obj = obj_to_char( obj, mob );
			if ( pReset->command == 'E' )
			{
				if ( obj->carried_by != mob )
				{
					send_log( NULL, LOG_RESET, "'E' reset: can't give object %d to mob %d.",
						obj->vnum, mob->pIndexData->vnum );
					break;
				}
				equip_char( mob, obj, pReset->arg3 );
			}
			lastobj = obj;
			break;

		  case 'O':
			pObj = get_obj_proto( NULL, pReset->arg1 );
			if ( !pObj )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'O': bad obj vnum %d.", pReset->arg1 );
				continue;
			}

			pRoom = get_room_index( NULL, pReset->arg3 );
			if ( !pRoom )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'O': bad room vnum %d.", pReset->arg3 );
				continue;
			}

			if ( pArea->nplayer > 0 || count_obj_list(pObj, pRoom->first_content) > 0 )
			{
				obj = NULL;
				lastobj = NULL;
				break;
			}
			obj = make_object( pObj, number_fuzzy(generate_itemlevel(pArea, pObj)) );
			obj->level = UMIN( obj->level, LVL_LEGEND );
			obj->count = pReset->arg2; /* Added to fix object counts */
			obj->cost = 0;
			obj_to_room( obj, pRoom );
			lastobj = obj;
			break;

		  case 'P':
			pObj = get_obj_proto( NULL, pReset->arg1 );
			if ( !pObj )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'P': bad obj vnum %d.", pReset->arg1 );
				continue;
			}

			if ( pReset->arg3 > 0 )
			{
				pObjTo = get_obj_proto( NULL, pReset->arg3 );
				if ( !pObjTo )
				{
					send_log( NULL, LOG_RESET, "reset_area: 'P': bad objto vnum %d.", pReset->arg3 );
					continue;
				}

				if ( pArea->nplayer > 0
					|| !(to_obj = get_obj_type(pObjTo))
					|| !to_obj->in_room
					|| count_obj_list(pObj, to_obj->first_content) > 0 )
				{
					obj = NULL;
					break;
				}
				lastobj = to_obj;
			}
			else
			{
				int iNest;

				if ( !lastobj )
					break;
				to_obj = lastobj;

				for ( iNest = 0;  iNest < pReset->extra;  iNest++ )
				{
					to_obj = to_obj->last_content;
					if ( !to_obj )
					{
						send_log( NULL, LOG_RESET, "reset_area: 'P': Invalid nesting obj %d.", pReset->arg1 );
						iNest = -1;
						break;
					}
				}
				if ( iNest < 0 )
					continue;
			}
			obj = make_object( pObj, number_fuzzy(UMAX(generate_itemlevel(pArea, pObj), to_obj->level)) );
			obj->level = UMIN( obj->level, LVL_LEGEND );
			obj->count = pReset->arg2; /* Added to fix object counts */
			obj_to_obj( obj, to_obj );
			break;

		  case 'T':
			if ( HAS_BITINT(pReset->extra, TRAPFLAG_OBJ) )
			{
				/* We need to preserve obj for future 'T' and 'H' checks */
				OBJ_DATA *pobj;

				if ( pReset->arg3 > 0 )
				{
					pObjTo = get_obj_proto( NULL, pReset->arg3 );
					if ( !pObjTo )
					{
						send_log( NULL, LOG_RESET, "reset_area: 'T': bad objto vnum %d.", pReset->arg3 );
						continue;
					}
					if ( pArea->nplayer > 0
						|| !(to_obj = get_obj_type(pObjTo))
						|| (to_obj->carried_by && IS_PG(to_obj->carried_by))
						|| is_trapped(to_obj) )
					{
						break;
					}
				}
				else
				{
					if ( !lastobj || !obj )
						break;
					to_obj = obj;
				}
				pobj = make_trap( pReset->arg2, pReset->arg1,
					number_fuzzy(to_obj->level), pReset->extra );
				obj_to_obj( pobj, to_obj );
			}
			else
			{
				pRoom = get_room_index( NULL, pReset->arg3 );
				if ( !pRoom )
				{
					send_log( NULL, LOG_RESET, "reset_area: 'T': bad room %d.", pReset->arg3 );
					continue;
				}
				if ( pArea->nplayer > 0
					|| count_obj_list(get_obj_proto(NULL, VNUM_OBJ_TRAP),
					pRoom->first_content) > 0 )
				{
					break;
				}
				to_obj = make_trap( pReset->arg1, pReset->arg1, 10, pReset->extra );
				obj_to_room( to_obj, pRoom );
			}
			break;

		  case 'H':
			if ( pReset->arg1 > 0 )
			{
				pObjTo = get_obj_proto( NULL, pReset->arg1 );
				if ( !pObjTo )
				{
					send_log( NULL, LOG_RESET, "reset_area: 'H': bad objto vnum %d.", pReset->arg1 );
					continue;
				}
				if ( pArea->nplayer > 0
					|| !(to_obj = get_obj_type(pObjTo))
					|| !to_obj->in_room
					|| to_obj->in_room->area != pArea
					|| HAS_BIT(to_obj->extra_flags, OBJFLAG_HIDDEN) )
				{
					break;
				}
			}
			else
			{
				if ( !lastobj || !obj )
					break;
				to_obj = obj;
			}
			SET_BIT( to_obj->extra_flags, OBJFLAG_HIDDEN );
			break;

		  case 'B':
			switch ( pReset->arg2 & BIT_RESET_TYPE_MASK )
			{
			  case BIT_RESET_DOOR:
				{
					int doornum;

					pRoom = get_room_index( NULL, pReset->arg1 );
					if ( !pRoom )
					{
						send_log( NULL, LOG_RESET, "reset_area: 'B': door: bad room vnum %d.", pReset->arg1 );
						continue;
					}

					doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;
					pexit = get_exit( pRoom, doornum );
					if ( !pexit )
						break;

					plc = pexit->flags;
				}
				break;

			  case BIT_RESET_ROOM:
				pRoom = get_room_index( NULL, pReset->arg1 );
				if ( !pRoom )
				{
					send_log( NULL, LOG_RESET, "reset_area: 'B': room: bad room vnum %d.", pReset->arg1 );
					continue;
				}
				plc = pRoom->flags;
				break;

			  case BIT_RESET_OBJECT:
				if ( pReset->arg1 > 0 )
				{
					pObjTo = get_obj_proto( NULL, pReset->arg1 );
					if ( !pObjTo )
					{
						send_log( NULL, LOG_RESET, "reset_area: 'B': object: bad objto vnum %d.", pReset->arg1 );
						continue;
					}

					to_obj = get_obj_type( pObjTo );
					if ( !to_obj )							continue;
					if ( !to_obj->in_room )					continue;
					if (  to_obj->in_room->area != pArea )	continue;
				}
				else
				{
					if ( !lastobj || !obj )
						continue;
					to_obj = obj;
				}
				plc = to_obj->extra_flags;
				ext_bv = TRUE;
				break;

			  case BIT_RESET_MOBILE:
				if ( !mob )
					continue;
				plc = mob->affected_by;
				ext_bv = TRUE;
				break;

			  default:
				send_log( NULL, LOG_RESET, "reset_area: 'B': bad options %d.", pReset->arg2 );
				continue;
			} /* chiude lo switch */

			if ( plc )
			{
				if		( HAS_BITINT(pReset->arg2, BIT_RESET_SET) )		SET_BIT( plc, pReset->arg3 );
				else if ( HAS_BITINT(pReset->arg2, BIT_RESET_TOGGLE) )	TOGGLE_BIT( plc, pReset->arg3 );
				else													REMOVE_BIT( plc, pReset->arg3 );
			}
			break;

		  case 'D':
			pRoom = get_room_index( NULL, pReset->arg1 );
			if ( !pRoom )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'D': bad room vnum %d.", pReset->arg1 );
				continue;
			}

			pexit = get_exit( pRoom, pReset->arg2 );
			if ( !pexit )
				break;

			switch ( pReset->arg3 )
			{
			  case 0:
				REMOVE_BIT( pexit->flags, EXIT_CLOSED );
				REMOVE_BIT( pexit->flags, EXIT_LOCKED );
				break;
			  case 1:
				SET_BIT( pexit->flags, EXIT_CLOSED );
				REMOVE_BIT( pexit->flags, EXIT_LOCKED );
				if ( HAS_BIT(pexit->flags, EXIT_xSEARCHABLE) )
					SET_BIT( pexit->flags, EXIT_SECRET );
				break;
			  case 2:
				SET_BIT( pexit->flags, EXIT_CLOSED );
				SET_BIT( pexit->flags, EXIT_LOCKED );
				if ( HAS_BIT(pexit->flags, EXIT_xSEARCHABLE) )
					SET_BIT( pexit->flags, EXIT_SECRET);
				break;
			}
			break;

		  case 'R':
			pRoom = get_room_index( NULL, pReset->arg1 );
			if ( !pRoom )
			{
				send_log( NULL, LOG_RESET, "reset_area: 'R': bad room vnum %d.", pReset->arg1 );
				continue;
			}

			randomize_exits( pRoom, pReset->arg2-1 );
			break;
		} /* chiude lo switch */
	} /* chiude il for */
}


/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets( AREA_DATA *pArea )
{
	RESET_DATA *pReset;
	RESET_DATA *lastobj = NULL;

	for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
	{
		switch ( pReset->command )
		{
		  default:
			break;

		  case 'G':
		  case 'E':
		  case 'O':
			lastobj = pReset;
			break;

		  case 'P':
			if ( pReset->arg3 == 0 )
			{
				if ( !lastobj )
					pReset->extra = 1000000;
				else if ( lastobj->command != 'P' || lastobj->arg3 > 0 )
					pReset->extra = 0;
				else
					pReset->extra = lastobj->extra+1;

				lastobj = pReset;
			}
		}
	}
}


/*
 * Add a reset to an area
 */
RESET_DATA *add_reset( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 )
{
	RESET_DATA *pReset;

	if ( !tarea )
	{
		send_log( NULL, LOG_RESET, "add_reset: tarea è NULL." );
		return NULL;
	}

	letter = toupper( letter );
	pReset = make_reset( letter, extra, arg1, arg2, arg3 );
	switch ( letter )
	{
	  case 'M':  tarea->last_mob_reset = pReset;	break;
	  case 'H':  if ( arg1 > 0 )					break;
	  case 'E':
	  case 'G':
	  case 'P':
	  case 'O':  tarea->last_obj_reset = pReset;	break;
	  case 'T':
		if ( HAS_BITINT(extra, TRAPFLAG_OBJ) && arg1 == 0 )
			tarea->last_obj_reset = pReset;
		break;
	}

	LINK( pReset, tarea->first_reset, tarea->last_reset, next, prev );
	return pReset;
}


/*
 * Rimuove tutti i reset da un'area
 */
void clean_resets( AREA_DATA *tarea )
{
	RESET_DATA *pReset;

	for ( pReset = tarea->first_reset;  pReset;  pReset = tarea->first_reset )
	{
		DISPOSE( pReset );
		top_reset--;
	}

	tarea->first_reset	= NULL;
	tarea->last_reset	= NULL;
}


/*
 * Carica la sezione dei reset
 */
void fread_reset( AREA_DATA *tarea, MUD_FILE *fp )
{
	bool	not01 = FALSE;

	if ( !tarea )
	{
		send_log( fp, LOG_FREAD, "fread_reset: sezione #AREA non trovata." );
		if ( fBootDb )
			exit( EXIT_FAILURE );
		return;
	}

	if ( tarea->first_reset )
	{
		if ( fBootDb )
			send_log( fp, LOG_FREAD, "fread_reset: WARNING: resets già esistono per questa area." );
		else
		{
			/* Clean out the old resets */
			send_log( fp, LOG_BUILD, "fread_reset: cleaning resets %s", tarea->name );
			clean_resets( tarea );
		}
	}

	for ( ; ; )
	{
		ROOM_DATA	*pRoom;
		EXIT_DATA	*pexit;
		char		 letter;
		int			 extra;
		int			 arg1;
		int			 arg2;
		int			 arg3;

		letter = fread_letter( fp );

		if ( letter == EOF )
		{
			send_log( fp, LOG_FREAD, "fread_reset: fine prematura del file in lettura" );
			if ( fBootDb )
				exit( EXIT_FAILURE );
			return;
		}

		/* Se incontra il carattere S esce dalla lettura reset */
		if ( letter == 'S' )
			break;

		if ( letter == '*' )
		{
			fread_to_eol( fp );
			continue;
		}

		extra	= fread_number( fp );
		arg1	= fread_number( fp );
		arg2	= fread_number( fp );
		arg3	= ( letter == 'G' || letter == 'R' )  ?  0  :  fread_number( fp );
		fread_to_eol( fp );

		/* Validate parameters.
		 * We're calling the index functions for the side effect.
		 */
		switch ( letter )
		{
		  default:
			send_log( fp, LOG_FREAD, "fread_reset: comando errato '%c'.", letter );
			return;

		  case 'M':
			if ( get_mob_index(fp, arg1) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: 'M': mobile %d doesn't exist.", arg1 );
			if ( get_room_index(fp, arg3) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: 'M': room %d doesn't exist.", arg3 );
			break;

		  case 'O':
			if ( get_obj_proto(fp, arg1) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: '%c': object %d doesn't exist.", letter, arg1 );
			if ( get_room_index(fp, arg3) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: '%c': room %d doesn't exist.", letter, arg3 );
			break;

		  case 'P':
			if ( get_obj_proto(fp, arg1) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: '%c': object %d doesn't exist.", letter, arg1 );

			if ( arg3 > 0 )
			{
				if ( get_obj_proto(fp, arg3) == NULL && fBootDb )
					send_log( fp, LOG_BUILD, "fread_reset: 'P': destination object %d doesn't exist.", arg3 );
			}
			else if ( extra > 1 )
				not01 = TRUE;

			break;

		  case 'G':
		  case 'E':
			if ( get_obj_proto(fp, arg1) == NULL && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: '%c': object %d doesn't exist.", letter, arg1 );
			break;

		  case 'T':
			break;

		  case 'H':
			if ( arg1 > 0 )
			{
				if ( get_obj_proto(fp, arg1) == NULL && fBootDb )
					send_log( fp, LOG_BUILD, "fread_reset: 'H': object %d doesn't exist.", arg1 );
			}
			break;

		  case 'B':
			switch ( arg2 & BIT_RESET_TYPE_MASK )
			{
			  case BIT_RESET_DOOR:
				{
					int door;

					pRoom = get_room_index( fp, arg1 );
					if ( !pRoom )
					{
						send_log( fp, LOG_FREAD, "fread_reset: 'B': stanza %d inesistente.  Reset: %c %d %d %d %d",
							arg1, letter, extra, arg1, arg2, arg3 );
					}

					door = ( arg2 & BIT_RESET_DOOR_MASK ) >> BIT_RESET_DOOR_THRESHOLD;

					if ( !(pexit = get_exit(pRoom, door)) )
					{
						send_log( fp, LOG_FREAD, "fread_reset: 'B': l'uscita %d non è una porta.  Reset: %c %d %d %d %d",
							door, letter, extra, arg1, arg2, arg3 );
					}
				}
				break;

			  case BIT_RESET_ROOM:
				if ( get_room_index(fp, arg1) == NULL )
				{
					send_log( fp, LOG_FREAD, "fread_reset: 'B': stanza %d inesistente.  Reset: %c %d %d %d %d",
						arg1, letter, extra, arg1, arg2, arg3 );
				}
				break;

			  case BIT_RESET_OBJECT:
				if ( arg1 > 0 )
				{
					if ( get_obj_proto(fp, arg1) == NULL && fBootDb )
						send_log( fp, LOG_BUILD, "fread_reset: 'B': object %d doesn't exist.", arg1 );
				}
				break;

			  case BIT_RESET_MOBILE:
				if ( arg1 > 0 )
				{
					if ( get_mob_index(fp, arg1) == NULL && fBootDb )
						send_log( fp, LOG_BUILD, "fread_reset: 'B': mobile %d doesn't exist.", arg1 );
				}
				break;

			  default:
				send_log( fp, LOG_BUILD, "fread_reset: 'B': bad type flag (%d).", arg2 & BIT_RESET_TYPE_MASK );
				break;
			} /* chiude lo switch */
			break;

		  case 'D':
			pRoom = get_room_index( fp, arg1 );
			if ( !pRoom )
			{
				send_log( fp, LOG_FREAD, "fread_reset: 'D': stanza %d inesistente.  Reset: %c %d %d %d %d",
					arg1, letter, extra, arg1, arg2, arg3 );

				break;
			}

			if ( arg2 < 0
			  || arg2 >= MAX_DIR
			  || (pexit = get_exit(pRoom, arg2)) == NULL
			  || !HAS_BIT(pexit->flags, EXIT_ISDOOR) )
			{
				send_log( fp, LOG_FREAD, "fread_reset: 'D': l'uscita %d non è una porta.  Reset: %c %d %d %d %d",
					arg2, letter, extra, arg1, arg2, arg3 );
			}

			if ( arg3 < 0 || arg3 > 2 )
				send_log( fp, LOG_FREAD, "fread_reset: 'D': 'locks' errato: %d.", arg3 );
			break;

		  case 'R':
			pRoom = get_room_index( fp, arg1 );

			if ( !pRoom && fBootDb )
				send_log( fp, LOG_BUILD, "fread_reset: 'R': la stanza %d non esiste.", arg1 );

			if ( arg2 < 0 || arg2 >= MAX_DIR )
			{
				send_log( fp, LOG_FREAD, "fread_reset: 'R': uscita errata %d.", arg2 );
				break;
			}
			break;
		} /* chiude lo switch */

		/* finally, add the reset */
		add_reset( tarea, letter, extra, arg1, arg2, arg3 );
	} /* chiude il for */

	if ( !not01 )
		renumber_put_resets( tarea );
}


/*
 * This command allows an Immortal to seek out a vnum reference in all the resets in the world.
 * Handy for identifying what area a specific mob/item/other reset is coming from when one of
 *	your builders purposefully or accidentally adds the reset to an unrelated area.
 */
DO_RET do_rseek( CHAR_DATA *ch, char *argument )
{
	RESET_DATA	*pReset;
	AREA_DATA	*pArea;
	char		 arg[MSL];
	int			 x;
	int			 counter = 1;
	bool		 Room = FALSE;
	bool		 Obj = FALSE;
	bool		 Mob = FALSE;

	argument = one_argument( argument, arg );
	if ( !VALID_STR(arg) || !is_number(arg) )
	{
		send_to_char( ch, "Is not a number, and only vnums can be searched for in resets.\r\n" );
		return;
	}

	x = atoi( arg );
	if ( !str_cmp(argument, "all") || !VALID_STR(argument) )
	{
		Room = TRUE;
		Obj = TRUE;
		Mob = TRUE;
	}

	if ( is_name("room", argument) )
		Room = TRUE;

	if ( is_name("obj", argument) )
		Obj = TRUE;

	if ( is_name("mob", argument) )
		Mob = TRUE;

	for ( pArea = first_area;  pArea;  pArea = pArea->next )
	{
		for ( pReset = pArea->first_reset;  pReset;  pReset = pReset->next )
		{
			switch ( toupper(pReset->command) )
			{
			  default:
				counter++;
				break;

			  case 'M':
				if ( Mob && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CMob&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				if ( Room && x == pReset->arg3 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CRoom&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'G':
				if ( Obj && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CObj&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'E':
				if ( Obj && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CObj&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'O':
				if ( Obj && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CObj&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				if ( Room && x == pReset->arg3 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CRoom&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'P':
				if ( Obj && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CObj&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				if ( Obj && x == pReset->arg3 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CContainer&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'H':		/* Hides an item. */
				if ( Obj && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CHide Obj&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;

			  case 'D':		/* For doors */
				if ( Room && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CRoom&W]\n\r", x, pArea->filename, counter );
					list_resets(ch, pArea, NULL, counter, counter);
				}
				counter++;
				break;

			  case 'R':
				if ( Room && x == pReset->arg1 )
				{
					pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&CRoom&W]\n\r", x, pArea->filename, counter );
					list_resets( ch, pArea, NULL, counter, counter );
				}
				counter++;
				break;
			}

/*			if ( pReset->arg1 == x || pReset->arg2 == x || (pReset->arg3 == x  && Rooms) )
			{
				pager_printf( ch, "&W[&C%-6d&W] %-20s Reset Number: %10d [&C%s&W]\n\r", x, pArea->filename, counter, pReset->command == 'M' ? "Mobile" : pReset->command == 'O' ? "Object" : "Special" );
				list_resets( ch, pArea, NULL, counter, counter );
			}
			counter++;
*/
		}
		counter = 1;
	}
}
